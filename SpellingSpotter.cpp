// SpellingSpotting.cpp
#include "SpellingSpotter.h"
#include "BackBuffer.h"
#include <algorithm>
#include "Random.h"
#include "Utility.h"
#include "Speller.h"
#include "ScreenPrinter.h"

using namespace std;
using namespace Gdiplus;

// SSWord
SSWord::SSWord(std::wstring text, bool correct)
    : text_(text), correct_(correct), selected_(false)
{}


// SSRow
SSRow::SSRow(float width)
    : horizontalOffset_(0.0f), emptyWidth_(width)
{}

bool SSRow::AddWord(SSWord *word){
    if( emptyWidth_ < word->width_ ){
        return false;
    }
    emptyWidth_ -= word->width_;
    words_.push_back( word );
    horizontalOffset_ = 0.5f * emptyWidth_; // update horizontal Offset
    return true;
}

void SSRow::RandomiseWordOrder() {
    if( words_.size() < 2 )
        return;
    random_shuffle(words_.begin(), words_.end() );
}

bool SSRow::IsEmpty() const{
    return words_.empty();
}

// SSRegion
SSRegion::SSRegion(std::wstring correctSpelling, StringVec wrongSpellings,
                   Gdiplus::PointF pos, BackBuffer *bb, Font* font, Speller& speller)
    : position_(pos), bb_(bb),
        height_(500.0f), width_(1000.0f), hPad_(10.0f), vPad_(10.0f), hMargin_(20.0f), vMargin_(30.0f), // these should become constants
        highlightWidth_(5.0f), highlightColour_(Color(255,255,0)),
        maxRows_(0), selected_(false), timer_(0.0), pFont_(font), FADESPEED(1.0)
{
    paper_      = speller.GetColour( Speller::PAPER );
    correct_    = speller.GetColour( Speller::CORRECT );
    wrong_      = speller.GetColour( Speller::WRONG );
    pen_ = correctFade_ = wrongFade_ = speller.GetColour( Speller::PEN );
    
    SetUp(correctSpelling, wrongSpellings, speller);
}

SSRegion::~SSRegion(){
    wordList_.erase( remove_if( wordList_.begin(), wordList_.end(), deleteAll<SSWord>),
                     wordList_.end() );
}

void SSRegion::SetUp(std::wstring& correctSpelling, StringVec& wrongSpellings, Speller& speller){
    
    // Add correct word to beginning of vector
    wordList_.push_back( new SSWord(correctSpelling, true) );
    SSRegion::HeightAndWidth dim = MeasureWord( correctSpelling );
    wordList_[0]->width_ = dim.second;
    float maxHeight = dim.first; // Store height for comparison (TODO: Might not be needed - just a single measurement.)
    
    // Shuffle up the wrong spellings
    random_shuffle(wrongSpellings.begin(), wrongSpellings.end() );
    
    // TODO: Is the measured height going to be the same for all strings?
    // Add the wrong spellings to the vector
    for( StringVec::iterator iter = wrongSpellings.begin();
         iter != wrongSpellings.end();
         ++iter ){
        SSWord* word = new SSWord( *iter );
        SSRegion::HeightAndWidth dim = MeasureWord( *iter );
        word->width_ = dim.second;
        maxHeight = (dim.first > maxHeight) ? dim.first : maxHeight;
        wordList_.push_back( word );
    }
    maxRows_ = static_cast<unsigned int> (height_ / maxHeight);
    rowHeight_ = height_ / static_cast<float>( maxRows_ );
    // Make the rows
    rows_.resize(maxRows_, SSRow(width_));
    
    // Populate the rows
    // Correct word first
    int index = GetRandomRowIndex();
    rows_[index].AddWord( wordList_[0] ); // TODO: word should fit - what if it doesn't?
    
    // TEST: For now, fit every wrong word possible. Potential for infinite loop.
    for(SSWordList::iterator iter = wordList_.begin()+1;
        iter != wordList_.end();
        ++iter ){
            do{
                index = GetRandomRowIndex();
            } while( !(rows_[index].AddWord( *iter ) ) );
    }
    
    // Delete any empty rows.
    rows_.erase(remove_if( rows_.begin(), rows_.end(), mem_fun_ref( &SSRow::IsEmpty ) ),rows_.end() );
    
    // TODO: Sort out sparse rows here.
    
    // Calculate vertical offset
    verticalOffset_ = 0.5f * ( height_ - (rows_.size() * rowHeight_ ) );
    
    // Randomise words in each row
    for_each(rows_.begin(), rows_.end(), mem_fun_ref( &SSRow::RandomiseWordOrder ) );
    
}

SSRegion::HeightAndWidth SSRegion::MeasureWord( std::wstring& word ){
    HDC hdc = bb_->getDC();
    Gdiplus::Graphics graphics(hdc);
    PointF p;
    //Need the following StringFormat for accurate measurement.
    StringFormat* sf = (StringFormat::GenericTypographic())->Clone();
    sf->SetFormatFlags( StringFormatFlags::StringFormatFlagsMeasureTrailingSpaces );
    Gdiplus::RectF rec;
    graphics.MeasureString(word.c_str(), -1, pFont_, p, sf, &rec);
    HeightAndWidth dim(rec.Height + 2.0f*vPad_ + 2.0*vMargin_ , rec.Width + 2.0f*hPad_ + 2.0f*hMargin_ );
    return dim;
}

int SSRegion::GetRandomRowIndex(){
    return Random(0, static_cast<int>( rows_.size() - 1 ) );
}

void SSRegion::Display(){
    Graphics graphics(bb_->getDC());
    ScreenPrinter* sp = new ScreenPrinter( bb_ );
    
    PointF storePos = position_;
    // Adjust for offsets.
    storePos.X += bb_->mOffX;
    storePos.Y += ( bb_->mOffY + verticalOffset_ );
    PointF pos = storePos;
    // Cycle through rows
    for( int row = 0; row < rows_.size(); ++row ){
        
        pos.X += rows_[row].horizontalOffset_;
        // Cycle through words in row
        for( int word = 0; word < rows_[row].words_.size(); ++word ){
            SSWord* pWord = rows_[row].words_[word];
            
            pos.X += hMargin_;
            pos.Y += vMargin_;
            
            // Draw highlight border, if needed
            if( !selected_ && highlightWord_.first == row && highlightWord_.second == word ){
                DrawHighlight(graphics, pos);
            }
                        
            // Display paper
            graphics.FillRectangle( &SolidBrush( Color( paper_ ) ),
                                    pos.X, pos.Y,
                                    pWord->width_ - (2.0f * hMargin_), rowHeight_ - (2.0f * vMargin_) );
            
            Gdiplus::Color colour = pen_;
            if( pWord->correct_ ){ colour = correctFade_; }
            if( pWord->selected_ && !(pWord->correct_) ){ colour = wrongFade_; }
            
            PointF wordPos = pos;
            // Adjust for padding, BUT REPLACE screen offsets - ScreenPrinter takes them off again.
            wordPos.X += hPad_ - bb_->mOffX;
            wordPos.Y += vPad_ - bb_->mOffY;
            for( wstring::iterator letter = rows_[row].words_[word]->text_.begin();
                 letter != pWord->text_.end();
                 ++letter ){
                wordPos = sp->PrintLetter( *letter, wordPos, *pFont_, colour );
            } //End Letter For
            
            // Update position for next word.
            pos.X += pWord->width_ - hMargin_;
            pos.Y -= vMargin_;
        } // End Word For
        pos.Y += rowHeight_;
        pos.X = storePos.X;
    } //End Row For
    delete sp; // delete screenprinter object
    
    
}

void SSRegion::Update(double dt, const Gdiplus::PointF *cursorPos){
    int g = 0;
    if( selected_ ) {
        timer_ += dt;
        double ratio = timer_ / FADESPEED;
        correctFade_ = GetFadeColour( pen_, correct_, ratio );
        wrongFade_   = GetFadeColour( pen_, wrong_, ratio );
        return;
    }
    
    IsInWord( cursorPos );

}

bool SSRegion::IsInWord( const Gdiplus::PointF* cursorPos ){
    // Determine border highlight
    highlightWord_.first = highlightWord_.second = -1;
    PointF start = position_;
    start.Y += verticalOffset_;
    for( int row = 0; row < rows_.size(); ++row ){
        start.X += rows_[row].horizontalOffset_; // Adjust for h offset
        start.Y += vMargin_;
        for( int word = 0; word < rows_[row].words_.size(); ++word ){
            start.X += hMargin_; // adjust for margins
            
            float width = rows_[row].words_[word]->width_ - (2.0f * hMargin_);
            float height = rowHeight_ - ( 2.0f * vMargin_ );
            if( ClickInRegion(cursorPos, &start, width, height) ){
                highlightWord_.first = row;
                highlightWord_.second = word;
                return true;
            }
            // Not found, so advance to next word position
            start.X += width + hMargin_;
        }
        // Not found in this row, so reset horizontal position and move vertical position.
        // TODO: consider breaking out of here if highlightWord_ is set
        start.X = position_.X;
        start.Y += rowHeight_ - vMargin_;
    }
    return false;
}

bool SSRegion::Click(const Gdiplus::PointF* cursorPos){
    if( IsInWord( cursorPos ) && !selected_ ){
        selected_ = true;
        rows_[highlightWord_.first].words_[highlightWord_.second]->selected_ = true;
        return true;
    }
    return false;
}

void SSRegion::DrawHighlight( Graphics& graphics, const PointF& pos ){
    float width = rows_[highlightWord_.first].words_[highlightWord_.second]->width_ - ( 2.0f * hMargin_ );
    float height = rowHeight_ - ( 2.0f * vMargin_ );
    Gdiplus::RectF rectangles[] = { RectF(pos.X - highlightWidth_, pos.Y - highlightWidth_, width + highlightWidth_, highlightWidth_ ), // Top
                                    RectF(pos.X, pos.Y + height, width + highlightWidth_, highlightWidth_), // Bottom
                                    RectF(pos.X - highlightWidth_, pos.Y, highlightWidth_, height + highlightWidth_), // Left
                                    RectF(pos.X + width, pos.Y - highlightWidth_, highlightWidth_, height + highlightWidth_)}; // Right
    graphics.FillRectangles( &SolidBrush( highlightColour_ ), rectangles, 4 ); 
                            
}
