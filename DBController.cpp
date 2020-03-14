// DBController.cpp

#include "DBController.h"
#include <string>
#include "ScrollBox.h"
#include "Speller.h"
#include "Range.h"
#include <algorithm>
#include "Convert.h"
//#include "Word.h"

using namespace std;

DBController::DBController()
: pDatabase_(0), dbLocation_("Data/spellephant.db"), dbStatus_(dbCLOSED) {
    
    // This is dangerous - potential infinite loop!
    while( dbStatus_ != dbOPEN ){
        OpenConnection();
    }
}

DBController::~DBController(){
    CloseConnection();
}

int DBController::GetStatus() const {
    return dbStatus_;
}

bool DBController::IsOpen() const {
    return dbStatus_ == dbOPEN;
}

void DBController::OpenConnection(){
    if( !pDatabase_ ){
        int result = sqlite3_open_v2(dbLocation_, &pDatabase_, SQLITE_OPEN_READWRITE,0);
        if( result ){
            // TODO: Need to process the error code.
            dbStatus_ = dbERROR;
            CloseConnection();
        }
        dbStatus_ = dbOPEN;
    }
}

void DBController::CloseConnection(){
    sqlite3_close(pDatabase_);
    dbStatus_ = dbCLOSED;
}

inline
wstring DBController::GetWString(sqlite3_stmt *sql, int col){
    return std::wstring(reinterpret_cast<const wchar_t*>(sqlite3_column_text16(sql,col)));
}

inline
int DBController::GetInt(sqlite3_stmt* sql, int col){
    return sqlite3_column_int(sql,col);
}

inline
unsigned int DBController::GetUInt(sqlite3_stmt* sql, int col){
    return static_cast<unsigned int> ( sqlite3_column_int(sql,col) );
}

inline
bool DBController::GetBool(sqlite3_stmt* sql, int col){
    return sqlite3_column_int(sql, col) != 0; // this avoids the C4800 error, caused by static_cast<bool>
}

inline
double DBController::GetDouble(sqlite3_stmt* sql, int col){
    return sqlite3_column_double(sql, col);
}

// Getting data
int DBController::GetNumSpellers(){    
    sqlite3_stmt* sql;
    wstring cmd = L"Select count(*) From Spellers;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_step(sql);
    int numSpellers = 0;
    while( result == SQLITE_ROW ){
        numSpellers = GetInt(sql,0);
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);

    return numSpellers;
}

void DBController::GetSpellerNames(StringList& nameList){
    sqlite3_stmt* sql;
    wstring cmd = L"Select Name From Spellers;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_step(sql);
    while( result == SQLITE_ROW ){
        nameList.push_back( GetWString(sql, 0) );
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);

}

void DBController::GetAvatarIDList(IDList& idList){
    sqlite3_stmt* sql;
    wstring cmd = L"Select ID From Avatars;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_step(sql);
    while( result == SQLITE_ROW ){
        idList.insert( GetInt(sql, 0) );
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);
}

std::wstring DBController::GetAvatarFilenameFromID(int id){
    sqlite3_stmt* sql;
    wstring cmd = L"Select Filename From Avatars Where ID=@id;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_bind_int(sql, 1, id);
    result = sqlite3_step(sql);
    wstring fileName = L"";
    while( result == SQLITE_ROW ){
        fileName = GetWString(sql, 0);
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);

    return fileName;
}

void DBController::GetSpellersAndAvatars(TableData& data){
    sqlite3_stmt* sql;
    wstring cmd = L"SELECT Spellers.ID, Filename, Name FROM Spellers JOIN Avatars ON AvatarID = Avatars.ID;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_step(sql);
    while( result == SQLITE_ROW ){
        RowData rd;
        rd.dataID_ = GetInt(sql, 0);
        rd.data_.push_back(L"Images/avatars/" + GetWString(sql, 1));
        rd.data_.push_back(GetWString(sql,2));
        data.push_back(new RowData(rd.dataID_, rd.data_));  //Here's where it will go belly up...
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);
}

Speller* DBController::LoadSpeller(int id){

    IDList stars = GetStars(id);        // Get Stars for this Speller
    IDList tags = GetSpellerTags(id);   // Get Tags for this Speller
    
    // Get rest of speller info
    sqlite3_stmt* sql;
    wstring cmd = L"SELECT * FROM Spellers JOIN Avatars ON AvatarID = Avatars.ID WHERE Spellers.ID = @id;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_bind_int(sql, 1, id);
    result = sqlite3_step(sql);
    // Setup variables
    wstring name = L"";
    wstring avatarFilename = L"";
    Range difficulty(1,9); 
    
    // Process
    while( result == SQLITE_ROW ){
        name = GetWString(sql, 1);
        difficulty.mLow = GetInt(sql, 3);
        difficulty.mHigh = GetInt(sql, 4);
        avatarFilename = GetWString(sql, 20);
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);
    
    Speller* sp = new Speller(id, name, difficulty, avatarFilename, stars, tags);
    GetSpellerRecords( *sp );
    GetWrongSpellings( *sp );
    return sp;
}

void DBController::GetTagList(TagList& tagList){
    tagList.clear(); // Clear list
    //tagList.push_back(Tag(0, L"[untagged]", true));
    sqlite3_stmt* sql;
    wstring cmd = L"SELECT * FROM Tags ORDER BY Name;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_step(sql);
    // Setup variables
    unsigned int id = 0;
    wstring tagName = L"";    
    // Process
    while( result == SQLITE_ROW ){
        id = GetUInt(sql, 0);
        tagName = GetWString(sql, 1);
        result = sqlite3_step(sql);
        tagList.push_back(Tag(id, tagName));
    }
    sqlite3_finalize(sql);
}

void DBController::GetWordBank(WordBank& wordBank){
    wordBank.clear(); // Clear bank
    sqlite3_stmt* sql;
    wstring cmd = L"SELECT * FROM Words NATURAL JOIN Spellings ORDER BY difficulty, LOWER(spelling);";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_step(sql);
    // Setup variables
    int wordID = 0;
    int difficulty = 0;
    bool confusable = false;
    int mainSpellingID = 0;
    int spellingID = 0;
    wstring spelling = L"";
    
    // Process
    while( result == SQLITE_ROW ){
        wordID = GetUInt(sql, 0);
        difficulty = GetUInt(sql, 1);
        confusable = GetBool(sql, 2);
        mainSpellingID = GetUInt(sql, 3);
        spellingID = GetUInt(sql, 4);
        spelling = GetWString(sql, 5);
        result = sqlite3_step(sql);

        WordBank::iterator iter;
        iter = wordBank.insert( make_pair(wordID, Word(wordID, difficulty, confusable, mainSpellingID))).first;
        // iter now should point either to the newly inserted Word or the pre-existing Word (see P183 Josuttis)
        Spelling sp(spellingID, spelling); // Create spelling
        iter->second.AddSpelling(sp); // Add it to this Word's spelling list, if unique.
    }
    sqlite3_finalize(sql);
}

void DBController::GetBreakdowns(WordBank& wordBank){
    sqlite3_stmt* sql;
    wstring cmd = L"SELECT WordID, SpellingID, Position, Length, ColourNumber"
                  L" FROM Spellings NATURAL JOIN Breakdowns ORDER BY SpellingID, Position;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_step(sql);
    // Setup variables
    unsigned int wordID = 0;
    unsigned int spellingID = 0;
    unsigned int position =0;
    unsigned int length = 0;    
    unsigned int colourNum = 0;
    // Process
    while( result == SQLITE_ROW ){
        wordID     = GetUInt(sql, 0);
        spellingID = GetUInt(sql, 1);
        position   = GetUInt(sql, 2);
        length     = GetUInt(sql, 3);
        colourNum  = GetUInt(sql, 4);
        result = sqlite3_step(sql);
        WordBank::iterator iter = wordBank.find(wordID);
        if( iter != wordBank.end() ){
            iter->second.AddBreakdown(spellingID, position, length, colourNum);
        }
    }
    sqlite3_finalize(sql);
}

void DBController::GetWordToTag(WordBank &wordBank, TagList &tagList){
    sqlite3_stmt* sql;
    wstring cmd = L"SELECT * FROM WordToTag;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_step(sql);
    // Setup variables
    unsigned int wordID = 0;
    unsigned int tagID = 0;
    // Process
    while( result == SQLITE_ROW ){
        wordID = GetUInt(sql, 0);
        tagID = GetUInt(sql, 1);
        result = sqlite3_step(sql);
        WordBank::iterator wordIter = wordBank.find(wordID);
        TagList::iterator tagIter = find_if(tagList.begin(), tagList.end(), bind2nd(mem_fun_ref(&Tag::EqualToID),tagID));
        if( wordIter != wordBank.end() && tagIter != tagList.end() ){
            // add tag to word
            wordIter->second.AddTagID( tagID );
            // add word to tag
            tagIter->AddWordID( wordID );
        }
    }
    sqlite3_finalize(sql);
}

IDList DBController::GetStars(unsigned int spellerID){
    IDList stars;
    sqlite3_stmt* sql;
    wstring cmd = L"SELECT wordID FROM Stars WHERE spellerID = @id;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_bind_int(sql, 1, spellerID);
    result = sqlite3_step(sql);
    // Setup variable
    unsigned int wordId = 0;
    // Process
    while( result == SQLITE_ROW ){
        wordId = GetUInt(sql, 0);
        stars.insert(wordId);
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);
    return stars;
}

IDList DBController::GetSpellerTags(unsigned int spellerID){
    IDList tags;
    sqlite3_stmt* sql;
    wstring cmd = L"SELECT tagID FROM SpellerTags WHERE spellerID = @id;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_bind_int(sql, 1, spellerID);
    result = sqlite3_step(sql);
    // Setup variable
    unsigned int tagId = 0;
    // Process
    while( result == SQLITE_ROW ){
        tagId = GetUInt(sql, 0);
        tags.insert(tagId);
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);
    return tags;
}

void DBController::GetSpellerRecords(Speller &speller){
    // Get all records from SpellerRecords
    sqlite3_stmt* sql;
    wstring cmd = L"SELECT * FROM SpellerRecords WHERE spellerID = @id;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_bind_int(sql, 1, speller.GetID());
    result = sqlite3_step(sql);

    // Process
    while( result == SQLITE_ROW ){
        unsigned int wordID = GetUInt(sql, 1);
        unsigned int attempts = GetUInt(sql, 2);
        int level = GetInt(sql, 3);
        speller.AddRecord( wordID, attempts, level );
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);
}

void DBController::GetWrongSpellings( Speller& speller ){
    // Get all wrong spellings for speller's records
    sqlite3_stmt* sql;
    wstring cmd = L"SELECT WordID, Spelling, Score, AverageLinkLength, LongestLink, LengthDifference FROM WrongSpellings WHERE spellerID = @id;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_bind_int(sql, 1, speller.GetID());
    result = sqlite3_step(sql);
    // Process
    while( result == SQLITE_ROW ){
        unsigned int wordID = GetUInt(sql, 0);
        wstring spelling    = GetWString(sql, 1);
        int score           = GetInt(sql, 2);
        double aveLinkLength= GetDouble(sql, 3);
        unsigned int longestLink = GetUInt( sql, 4);
        unsigned int lengthDiff  = GetUInt( sql, 5);
        WrongSpelling ws( spelling, score, aveLinkLength, longestLink, lengthDiff);
        speller.AddWrongSpelling( wordID, ws );
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);    
}

/* Inserting Data */

unsigned int DBController::AddSpeller(std::wstring spellerName, int avatarID){
    sqlite3_stmt* sql;
    wstring cmd = L"Insert Into Spellers (Name, AvatarID) VALUES (@name, @id);";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_bind_text16(sql, 1, spellerName.c_str(), -1, SQLITE_STATIC);
    result = sqlite3_bind_int(sql, 2, avatarID);
    result = sqlite3_step(sql);
    while( result == SQLITE_ROW ){
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);
    return static_cast<unsigned int>( sqlite3_last_insert_rowid( pDatabase_ ) );
}

void DBController::AddStars( unsigned int spellerID, IDList& stars) {
    sqlite3_stmt* sql;
    wstring cmd = L"INSERT INTO Stars VALUES (@spellerId, @wordId);";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1, &sql, 0);
    result = sqlite3_bind_int(sql, 1, spellerID);
    for( IDList::iterator iter = stars.begin(); iter != stars.end(); ++iter ){
        result = sqlite3_bind_int(sql, 2, *iter);
        result = sqlite3_step(sql);
        result = sqlite3_reset(sql);
        if( result != SQLITE_OK ){ // TODO: check I can do this.
            break;
        }
    }
    sqlite3_finalize(sql);
}

void DBController::AddSpellerTags( unsigned int spellerID, IDList& tags) {
    sqlite3_stmt* sql;
    wstring cmd = L"INSERT INTO SpellerTags VALUES (@spellerId, @tagId);";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1, &sql, 0);
    result = sqlite3_bind_int(sql, 1, spellerID);
    for( IDList::iterator iter = tags.begin(); iter != tags.end(); ++iter ){
        result = sqlite3_bind_int(sql, 2, *iter);
        result = sqlite3_step(sql);
        result = sqlite3_reset(sql);
        if( result != SQLITE_OK ){ // TODO: check I can do this.
            break;
        }
    }
    sqlite3_finalize(sql);
}

void DBController::AddSpellerRecord( unsigned int spellerID,
                                     unsigned int wordID,
                                     unsigned int attempts,
                                     int level){
    sqlite3_stmt* sql;
    wstring cmd = L"INSERT INTO SpellerRecords VALUES (@spellerId, @wordId, @attempts, @level);";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1, &sql, 0);
    result = sqlite3_bind_int(sql, 1, spellerID);
    result = sqlite3_bind_int(sql, 2, wordID);
    result = sqlite3_bind_int(sql, 3, attempts);
    result = sqlite3_bind_int(sql, 4, level);
    result = sqlite3_step(sql);
    while( result == SQLITE_ROW ){
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);
}

void DBController::AddWrongSpelling( unsigned int spellerID,
                                     unsigned int wordID,
                                     WrongSpelling &ws){
    sqlite3_stmt* sql;
    wstring cmd = L"INSERT INTO WrongSpellings (SpellerID, WordID, Spelling, Score, AverageLinkLength, LongestLink, LengthDifference) VALUES (@spellerId, @wordId, @spelling, @score, @averageLinkLength, @longestLink, @lengthDifference);";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1, &sql, 0);
    result = sqlite3_bind_int(sql, 1, spellerID);
    result = sqlite3_bind_int(sql, 2, wordID);
    result = sqlite3_bind_text16(sql, 3, ws.spelling_.c_str(), -1, SQLITE_STATIC);
    result = sqlite3_bind_int(sql, 4, ws.score_);
    result = sqlite3_bind_double(sql, 5, ws.aveLinkLength_);
    result = sqlite3_bind_int(sql, 6, ws.longestLink_);
    result = sqlite3_bind_int(sql, 7, ws.lengthDifference_);
    result = sqlite3_step(sql);
    while( result == SQLITE_ROW ){
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);

}

/* Updating Data */
void DBController::UpdateDifficulty(unsigned int spellerID, int low, int high){
    // Check inputs
    // Check limits
    if( low < 1 ) low = 1;
    if( high > 10 ) high = 10;
    // Swaps low and high if wrong way round
    if( low > high )
        swap(low, high);

    
    sqlite3_stmt* sql;
    wstring cmd = L"UPDATE Spellers SET MinDifficulty = @low, MaxDifficulty = @high WHERE Id = @id;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_bind_int(sql, 1, low);
    result = sqlite3_bind_int(sql, 2, high);
    result = sqlite3_bind_int(sql, 3, spellerID);
    result = sqlite3_step(sql);
    while( result == SQLITE_ROW ){
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);
}

void DBController::UpdateSpellerRecord(Speller& speller, unsigned int wordID ){
    sqlite3_stmt* sql;
    wstring cmd = L"UPDATE SpellerRecords SET Attempts = @attempts, Level = @level WHERE SpellerID = @spellerid AND WordID = @wordid;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_bind_int(sql, 1, speller.GetWordAttempts( wordID ));
    result = sqlite3_bind_int(sql, 2, speller.GetWordLevel( wordID ));
    result = sqlite3_bind_int(sql, 3, speller.GetID() );
    result = sqlite3_bind_int(sql, 4, wordID );
    result = sqlite3_step(sql);
    while( result == SQLITE_ROW ){
        result = sqlite3_step(sql);
    }
    sqlite3_finalize(sql);
}

/* Deleting Data */
void DBController::DeleteStars(unsigned int spellerID, IDList &stars){
    
    // Don't waste time if empty.
    if( stars.empty() ) return;
    
    sqlite3_stmt* sql;
    wstring cmd = L"DELETE FROM Stars WHERE SpellerID = @spellerId AND WordID = @wordId;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_bind_int(sql, 1, spellerID);
    for( IDList::iterator iter = stars.begin(); iter != stars.end(); ++iter ){
        result = sqlite3_bind_int(sql, 2, *iter);
        result = sqlite3_step(sql);
        result = sqlite3_reset(sql);
        if( result != SQLITE_OK ){ // TODO: check I can do this.
            break;
        }
    }
    sqlite3_finalize(sql);
}

void DBController::DeleteSpellerTags(unsigned int spellerID, IDList &tags){
    
    // Don't waste time if empty.
    if( tags.empty() ) return;
    
    sqlite3_stmt* sql;
    wstring cmd = L"DELETE FROM SpellerTags WHERE SpellerID = @spellerId AND TagID = @tagId;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_bind_int(sql, 1, spellerID);
    for( IDList::iterator iter = tags.begin(); iter != tags.end(); ++iter ){
        result = sqlite3_bind_int(sql, 2, *iter);
        result = sqlite3_step(sql);
        result = sqlite3_reset(sql);
        if( result != SQLITE_OK ){ // TODO: check I can do this.
            break;
        }
    }
    sqlite3_finalize(sql);
}

void DBController::DeleteWrongSpelling(unsigned int spellerID, unsigned int wordID, std::wstring spelling){
    sqlite3_stmt* sql;
    wstring cmd = L"DELETE FROM WrongSpellings WHERE SpellerID = @spellerId AND WordID = @wordId AND Spelling = @spelling;";
    int result = sqlite3_prepare16_v2(pDatabase_, cmd.c_str(), -1,&sql,0);
    result = sqlite3_bind_int(sql, 1, spellerID);
    result = sqlite3_bind_int(sql, 2, wordID);
    result = sqlite3_bind_text16(sql, 3, spelling.c_str(), -1, SQLITE_TRANSIENT);
    result = sqlite3_step(sql);
    sqlite3_finalize(sql);
}