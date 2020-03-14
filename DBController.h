// DBController.h
/*
This class deals with all the interactions with the SQLite3 database.
*/
#ifndef DBCONTROLLER_H
#define DBCONTROLLER_H

#include "sqlite3.h"
#include "Definitions.h" //Contains typedefs
#include "Word.h"
#include <string>
#include "Speller.h"

class DBController{
public:
    enum{dbERROR,dbCLOSED,dbOPEN};
    DBController();
    ~DBController();

    // Status checks
    int GetStatus() const;  // See enum for return possible status values.
    bool IsOpen() const;    // Returns true if status is open.
    
    // Getting Data
    int GetNumSpellers();   // Gets number of spellers recorded in the database. Returns -1 if error.
    void GetSpellerNames(StringList& nameList);
    void GetAvatarList(ImageList& imageList); 
    void GetAvatarIDList(IDList& idList);
    std::wstring GetAvatarFilenameFromID(int id);
    void GetSpellersAndAvatars(TableData& data);
    Speller* LoadSpeller(int id);
    void GetTagList(TagList& tagList); // tagList will be cleared first
    void GetWordBank(WordBank& wordBank); // wordList will be cleared first
    void GetBreakdowns(WordBank& wordBank); // get breakdowns from DB.
    void GetWordToTag(WordBank& wordBank, TagList& tagList); // get links between words and tags
    IDList GetStars(unsigned int spellerID);
    IDList GetSpellerTags(unsigned int spellerID);
    
    //Test - trying with an actual speller object
    void GetSpellerRecords(Speller& speller); // get all records
    void GetWrongSpellings(Speller& speller); // get wrong spellings
    
    // Inputting Data
    unsigned int AddSpeller(std::wstring spellerName, int avatarID); // returns ID of inserted speller
    void AddStars(unsigned int spellerID, IDList& stars);
    void AddSpellerTags(unsigned int spellerID, IDList& tags);
    void AddSpellerRecord( unsigned int spellerID, unsigned int wordID,
                           unsigned int attempts,  int level);
    void AddWrongSpelling( unsigned int spellerID, unsigned int wordID, WrongSpelling& ws);
    
    // Updating Data
    void UpdateDifficulty(unsigned int spellerID, int low, int high);
    void UpdateSpellerRecord( Speller& speller, unsigned int wordID );
    
    // Deleting Data
    void DeleteStars(unsigned int spellerID, IDList& stars);
    void DeleteSpellerTags( unsigned int spellerID, IDList& tags );
    void DeleteWrongSpelling( unsigned int spellerID, unsigned int wordID, std::wstring spelling );

private:
    void OpenConnection();
    void CloseConnection();
    
    std::wstring GetWString(sqlite3_stmt* sql, int col);
    int          GetInt    (sqlite3_stmt* sql, int col);
    unsigned int GetUInt   (sqlite3_stmt* sql, int col);
    bool         GetBool   (sqlite3_stmt* sql, int col);
    double       GetDouble (sqlite3_stmt* sql, int col);

private:
    sqlite3* pDatabase_;         // set by call to sqlite3_open_v2
    const char* dbLocation_;   // stores location of database file
    int dbStatus_;
    

};

#endif // DBCONTROLLER_H