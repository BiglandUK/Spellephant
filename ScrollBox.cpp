// ScrollBox.cpp

#include "ScrollBox.h"
#include "BackBuffer.h"
#include <algorithm>
#include <math.h>

using namespace Gdiplus;
using namespace std;

Column::Column()
: justification_(Column::Left)
, columnWidth_(0)
{}

Column::Column(Column::Justify j, int columnWidth)
: justification_(j)
, columnWidth_(columnWidth)
{}

Column::~Column(){}

int Column::ColumnWidth() const { return columnWidth_; }

Gdiplus::StringAlignment Column::GetJustification(){
    switch(justification_){
        case Column::Left:{
            return Gdiplus::StringAlignmentNear;
        }
        case Column::Centre:{
            return Gdiplus::StringAlignmentCenter;
        }
        case Column::Right:{
            return Gdiplus::StringAlignmentFar;
        }
        default:
            return Gdiplus::StringAlignmentNear;
    }
}

TextColumn::TextColumn()
: Column()
, font_(0)
{}

TextColumn::TextColumn(Column::Justify j, int columnWidth, Gdiplus::Font* font)
: Column(j, columnWidth)
, font_(font)
{}

TextColumn::~TextColumn(){}

void TextColumn::Print(BackBuffer& bb, const std::wstring &data,
                        const Gdiplus::RectF &rec, const Gdiplus::Color &col,
                        bool active){
 Graphics graphics(bb.getDC());
 
 Gdiplus::StringFormat format;
 format.SetAlignment( GetJustification() );
 Gdiplus::RectF recCopy(rec.X+bb.mOffX, rec.Y+bb.mOffY, rec.Width, rec.Height);

 // Vertical pad test
 // This centres the text vertically
 Gdiplus::RectF recTest;
 graphics.MeasureString(data.c_str(), -1, font_, recCopy, &format, &recTest);
 if( recTest.Height < recCopy.Height ) { // Need to pad vertically
    recCopy.Y += (recCopy.Height - recTest.Height) / 2;    
 }
 graphics.DrawString(data.c_str(), -1, font_, recCopy, &format, &SolidBrush(col));

}

IconColumn::IconColumn()
: Column()
{}

IconColumn::IconColumn(Column::Justify j, int columnWidth,
                         Gdiplus::Image* pBitmap,
                         IconColumn::ValueList vl)
: Column(j, columnWidth)
, values_(vl)
, pBitmap_(pBitmap)
, imageWidth_(pBitmap ? pBitmap->GetWidth() / 2 : 0)
, imageHeight_( ( pBitmap && !values_.empty() ) ? 
                    pBitmap->GetHeight() / values_.size() : 0 )
{}

IconColumn::~IconColumn(){
    values_.clear();
}

void IconColumn::Print(BackBuffer& bb, const std::wstring& data,
                        const Gdiplus::RectF& rec, const Gdiplus::Color& col,
                        bool active){
     Graphics graphics(bb.getDC());
     
     // Calculate adjustments for justification, if necessary.
     REAL justifyAdjustment = 0.0f;
     if( imageWidth_ < columnWidth_ && justification_ != Column::Left ){
        if( justification_ == Column::Right )
            justifyAdjustment = static_cast<float>( columnWidth_ - imageWidth_ );
        if( justification_ == Column::Centre )
            justifyAdjustment = static_cast<float>( (columnWidth_ - imageWidth_) ) / 2.0f;
     }
     // Create actual cell rectangle.
     Gdiplus::RectF recCopy(rec.X+static_cast<float>( bb.mOffX )+ justifyAdjustment, rec.Y+static_cast<float>( bb.mOffY ), static_cast<float>( imageWidth_ ), static_cast<float>( imageHeight_ ));
     
     // Adjustments to centre vertically, if necessary
      if( imageHeight_ < recCopy.Height ) {
        recCopy.Y += (recCopy.Height - imageHeight_) / 2;    
      }
     
     // Select Bitmap
     // First, active or disabled image
     REAL x = 0.0f;
     if( !active ) x = static_cast<float>( imageWidth_ );
     // Second, image that matches value
     REAL y = 0.0f;
     for(int i = 0; i < static_cast<int>( values_.size() ); ++i){
        if( data == L"Off"){
            int g  = 0;
        }
        if( values_[i].compare( data ) == 0 ){
            if( i == 0 ){
                int g = 0;
            }
            y = static_cast<float>( i * imageHeight_ );
            break;
        }
     }
     graphics.DrawImage(pBitmap_, recCopy, x, y, static_cast<float>( imageWidth_ ),
                                                 static_cast<float>( imageHeight_ ), UnitPixel);

}

ImageColumn::ImageColumn()
: Column()
, imageWidth_(0), imageHeight_(0)
, justifyAdjustment_(0.0f)
{}

ImageColumn::ImageColumn(Column::Justify j, int columnWidth, int imageWidth, int imageHeight )
: Column(j, columnWidth)
, imageWidth_(imageWidth), imageHeight_(imageHeight), justifyAdjustment_(0.0f)
{
     
     // Calculate adjustments for horizontal justification, if necessary.
     if( imageWidth_ < columnWidth_ && justification_ != Column::Left ){
        if( justification_ == Column::Right )
            justifyAdjustment_ = static_cast<float>( columnWidth_ - imageWidth_ );
        if( justification_ == Column::Centre )
            justifyAdjustment_ = (columnWidth_ - imageWidth_) / 2.0f;
     }
}

ImageColumn::~ImageColumn(){}

void ImageColumn::Print(BackBuffer& bb, const std::wstring &data,
                        const Gdiplus::RectF &rec, const Gdiplus::Color &col,
                        bool active){
     Graphics graphics(bb.getDC());
     
     // Create actual cell rectangle.
     Gdiplus::RectF recCopy(rec.X+static_cast<float>( bb.mOffX ) + justifyAdjustment_, rec.Y+static_cast<float>( bb.mOffY ), static_cast<float>( imageWidth_ ), static_cast<float>( imageHeight_ ));

     // Check vertical adjustment (rec.Height is the row height of the scroll box)
     // Basically centres image vertically
     if( imageHeight_ < rec.Height ){
        recCopy.Y += (rec.Height - imageHeight_)/2;  
     }
     
     // Create Image
     Gdiplus::Image* pImage = new Image( data.c_str() );
     graphics.DrawImage(pImage, recCopy);

}

// ROWDATA
RowData::RowData()
    : dataID_(-1), active_(true), visible_(true)
{}

RowData::RowData(int dataID, Data data, bool active, bool visible)
    : dataID_(dataID), data_(data), active_(active)
{}

bool RowData::IsVisible(){
    return visible_;
}

void RowData::ToggleActive(){
    active_ = !active_;
}

void RowData::Activate(){
    active_ = true;
}
void RowData::Deactivate(){
    active_ = false;
}

void RowData::VisToggleActive(){
    if( visible_ )
        active_ = !active_;
}

void RowData::VisActivate(){
    if( visible_ )
        active_ = true;
}
void RowData::VisDeactivate(){
    if( visible_ )
        active_ = false;
}

void RowData::ToggleVisibility(){
    visible_ = !visible_;
}

void RowData::Show(){
    visible_ = true;
}

void RowData::Hide(){
    visible_ = false;
}

bool RowData::EqualToID( const unsigned int id ) const {
    return dataID_ == id;
}


// SCROLLBOX

ScrollBox::ScrollBox(TableData* data, Gdiplus::PointF pos, int rowHeight, int numRows)
: data_(data)
, position_(pos)
, rowHeight_(rowHeight)
, totalWidth_(0)
, rowColour1_(50,0,0,255)
, rowColour2_(100,0,0,255)
, numRows_(numRows)
, currentTopRow_(0)
, selectable_(true)
, selectedRow_(-1)
, selectionColour_(255,50,0)
, upImage_(new Bitmap(L"Images/upArrow.bmp"))
, downImage_(new Bitmap(L"Images/downArrow.bmp"))
, up_(upImage_, PointF(0.0, 0.0),0.0f, 0.0f, Button::Normal, Button::Normal, Color(0,0,0,0))
, down_(downImage_, PointF(0.0, 0.0),0.0f, 0.0f, Button::Normal, Button::Normal, Color(0,0,0,0))
, barGrabbed_(false)
, barClicked_(0)
, scrollBarDisabled_(false)
, timer_(0.0)
, numVisibleData_(data_->size())
{}

ScrollBox::~ScrollBox(){
    for(list<Column*>::iterator iter = columns_.begin(); iter != columns_.end(); ++iter) {
        delete *iter;
        *iter = 0;
    }
    columns_.clear();
    
    delete upImage_;
    upImage_ = 0;
    
    delete downImage_;
    downImage_ = 0;
}

void ScrollBox::AddData(RowData* data){
    data_->push_back(data);
}

void ScrollBox::AddColumn(Column::Justify j, int columnWidth,
                            Gdiplus::Font *font){
    columns_.push_back(new TextColumn(j, columnWidth, font));
    UpdateAfterNewColumn(columnWidth);
}

void ScrollBox::AddColumn(Column::Justify j, int columnWidth,
                             Gdiplus::Image *image, IconColumn::ValueList values){
    columns_.push_back(new IconColumn(j, columnWidth, image, values));
    UpdateAfterNewColumn(columnWidth);
}

void ScrollBox::AddColumn(Column::Justify j, int columnWidth,
                            int imageWidth, int imageHeight ){
    columns_.push_back(new ImageColumn(j, columnWidth, imageWidth, imageHeight));
    UpdateAfterNewColumn(columnWidth);                           
}

void ScrollBox::SetRowColours(Gdiplus::Color& colour1, Gdiplus::Color& colour2){
    rowColour1_ = colour1;
    rowColour2_ = colour2;   
}

void ScrollBox::SetRowColours(Gdiplus::Color& colour){
    SetRowColours(colour, colour);
}

void ScrollBox::SetSelectedColour(Gdiplus::Color& colour) {
    selectionColour_ = colour;
}

void ScrollBox::UpdateAfterNewColumn(int columnWidth){
    totalWidth_ += columnWidth;
    
    UpdateScrollBar();
}

void ScrollBox::UpdateScrollBar(){
    int totalHeight = (numRows_) * rowHeight_;

    up_.SetPosition(PointF(position_.X + totalWidth_, position_.Y));
    down_.SetPosition(PointF(position_.X + totalWidth_,
                            position_.Y + totalHeight - down_.Height() ));
                            
    // Disable buttons if more rows visible than there is data.
    if( numRows_ >= static_cast<int>( numVisibleData_ ) ){
        up_.Disable();
        down_.Disable();
        scrollBarDisabled_ = true;
    }
    else{
        up_.ResetState();
        down_.ResetState();
        scrollBarDisabled_ = false;
    }
    
    scrollRectangle_.X = position_.X + totalWidth_;
    scrollRectangle_.Y = position_.Y + up_.Height();
    scrollRectangle_.Height = totalHeight - up_.Height() - down_.Height();
    scrollRectangle_.Width = up_.Width();
    
    scrollBar_.X = scrollRectangle_.X + 1;
    scrollBar_.Width = scrollRectangle_.Width - 2;
    
    UpdateScrollBarPosition();
    
}

void ScrollBox::UpdateScrollBarPosition(){
    if( data_->empty() )
        return;

    scrollBar_.Height = (numRows_ * scrollRectangle_.Height) / numVisibleData_ - 2.0f;
    scrollBar_.Y = 1.0f + (currentTopRow_ * scrollRectangle_.Height)/ numVisibleData_ + scrollRectangle_.Y;

    
    if( scrollBar_.Height < 10.0f ) // Ensure minimum height.
        scrollBar_.Height = 10.0f; 
        
    if( scrollBar_.Height + 2.0f > scrollRectangle_.Height ) // Ensure not larger than maximum
        scrollBar_.Height = scrollRectangle_.Height;
        
    if( scrollBar_.GetBottom() + 1.0f > scrollRectangle_.GetBottom() ) { // Check not hanging over bottom
        scrollBar_.Y = scrollRectangle_.GetBottom() - scrollBar_.Height - 1.0f;
    }
}


void ScrollBox::Update(double dt, const Gdiplus::PointF& mousePos ){
    timer_ += dt;
    if( timer_ >= 0.1 ){
        if( down_.GetState() == Button::Clicked ){
            Scroll(1);
        }
        if( up_.GetState() == Button::Clicked ) {
            Scroll(-1);
        }
        if( barClicked_ ){
            Scroll(numRows_ * barClicked_ );
        }
        timer_ = 0.0;
    }
    if( barGrabbed_ )
        DragBar(mousePos);

}

void ScrollBox::Display(BackBuffer& bb){
    if( columns_.empty() || data_->empty() ) return;
        
    PointF pos = position_; // Make a working copy of the top left corner.
    
    /* Unless client has specified number of rows, calculate the number of rows
       that will fit on the screen.
       (based on initial position, row height and taking offset into account.)*/
    if( numRows_ <= 0 ){
        numRows_ = (bb.height() - (static_cast<int>(pos.Y) + bb.mOffY)) / rowHeight_;
        UpdateScrollBar();
    }
        
    Graphics graphics(bb.getDC());
    int rowCount = 0; // counts number of DISPLAYED rows, for use in determining banded row colour.
    for( int i = currentTopRow_; rowCount < numRows_; ++i) {
        // Draw background row colour
        Color rowColour;
        if( i == selectedRow_ )
            rowColour = selectionColour_;
        else
            rowColour = (rowCount + currentTopRow_ ) % 2 == 0 ? rowColour1_ : rowColour2_;
        
        graphics.FillRectangle(&SolidBrush(rowColour),
                               RectF(pos.X + static_cast<float>( bb.mOffX ), pos.Y + static_cast<float>( bb.mOffY ),
                                     static_cast<float>( totalWidth_ ), static_cast<float>( rowHeight_ )));
        
        // Get data, if this isn't an empty row
        if( i < static_cast<int>(numVisibleData_) ){
            // Get row of data
            RowData* data( (*data_)[i] );
            // Set ink colour
            Color dataInk(0,0,0);
            // If inactive, lighten ink colour TODO: CREATE NORMAL AND ACTIVE INK
            if( !(data->active_) ) {
                dataInk = Color(200,200,200);
            }
            
            list<Column*>::iterator iter;
            int index = 0;                                                                  
            for(iter = columns_.begin(); iter != columns_.end(); ++iter ){
                // Get next item of data
                wstring item = data->data_[index];
                // Determine rectangle
                RectF rec(pos.X, pos.Y, static_cast<float>( (**iter).ColumnWidth() ), static_cast<float>( rowHeight_ ) );

                // Print in the specified column
                (**iter).Print(bb, item, rec, dataInk, data->active_ );
                // Update column position
                pos.X += (**iter).ColumnWidth();
                // Increase index for next data
                index++;
                if( index > static_cast<int>( data->data_.size() ) )
                    break;
            
            } // End of row
        }
        pos.X = position_.X;// Reset column position
        pos.Y += rowHeight_;//Update row position
        ++rowCount;
    }
    
    // Display buttons
    up_.Display(&bb);
    down_.Display(&bb);
    
    // Display ScrollBar
    RectF tempScroll(scrollRectangle_.X + bb.mOffX, scrollRectangle_.Y + bb.mOffY,
                    scrollRectangle_.Width, scrollRectangle_.Height);
    graphics.FillRectangle(&SolidBrush(Color(200,200,200)), tempScroll);
    RectF tempBar(scrollBar_.X + bb.mOffX, scrollBar_.Y + bb.mOffY,
                scrollBar_.Width, scrollBar_.Height);
    graphics.FillRectangle( &SolidBrush(Color(100,100,100)), tempBar);
    
}

std::pair<int,int> ScrollBox::Click( const Gdiplus::PointF& mousePos, bool& dblclkd, double time ){
    std::pair<int, int> cell(-1,-1);
    
    // Check for click within the entire scrollbox - if none, return now.
    if( !(Region(RectF(position_, Gdiplus::SizeF(static_cast<float>(totalWidth_ + scrollRectangle_.Width),
                                                 static_cast<float>(numRows_ * rowHeight_))
         )).IsVisible(mousePos))){
            // Nothing recognisable clicked
            cell.first = -2; // Notify that nothing recognisable has been clicked.
            return cell;
    }
    
    // Check scrollbar controls
    if( !scrollBarDisabled_ ) {
        if( up_.Click(&mousePos) ) { // Click up button
           timer_ = 0.1;           // Set timer button to trigger immediate scroll
           return cell;
        }
        
        if( down_.Click(&mousePos) ){
            timer_ = 0.1;
            return cell;
        }
        
        Region sb(scrollBar_);  // Has the user grabbed the scroll bar?
        if(sb.IsVisible(mousePos) ){
            barGrabbed_ = true;
            grabPoint_ = mousePos.Y - scrollBar_.Y;
            return cell; // Check nothing else.
        }
        
        Region sr(scrollRectangle_); // Has the user clicked in the scroll rectangle?
        if( sr.IsVisible(mousePos) ) {
            timer_ = 0.1; // Set timer for repeated action
            if( mousePos.Y < scrollBar_.GetTop() ) { // Has the user clicked above the rectangle?
                //Scroll(-numRows_);
                barClicked_ = -1;
                return cell;
            }
            //Scroll(numRows_);
            barClicked_ = 1;
            return cell;
        }
    }
    
    // Has the user clicked on a row?
    Region sbx(RectF(position_.X, position_.Y, static_cast<float>(totalWidth_), static_cast<float>( numRows_ * rowHeight_ )));
    if( sbx.IsVisible(mousePos) ){
        int row, col;

        // Which row? - Specifically, the data row, not the visible row. (Zero-based)
        row = currentTopRow_ + static_cast<int>(mousePos.Y - position_.Y)/ rowHeight_;
        if( selectable_ ){ // Only do this is if scroll box allows row selection
            if( row < static_cast<int>( numVisibleData_ ) ) { // Don't continue if an empty row was selected.
                // This section checks for a double-click
                static double clickTimer = time;    // Set up timer
                if( selectedRow_ == row && time - clickTimer < 0.5 ) { // Only check if the selected row has been
                    dblclkd = true;                                         // clicked again within half a second.    
                }
                clickTimer = time;                  // Update timer with current time
                selectedRow_ = row;                 // Store selection for visible display.
                
            }
            else {
                selectedRow_ = -1; // Blank row selected, so clear selection.
            }
        }

        // Which column? - Specifically, the visible column. (NOT zero-based.)
        int x = static_cast<int>( mousePos.X - position_.X );
        col = 1;
        std::list<Column*>::iterator iter( columns_.begin() );
        for( ; iter != columns_.end(); ++iter ){
            if( (**iter).ColumnWidth() >= x ){
                break;
            }
            x -= (**iter).ColumnWidth();
            col++;
        }
        return std::pair<int,int>(row, col);
    }
    

    return cell;
}

std::pair<int,int> ScrollBox::Click( const Gdiplus::PointF& mousePos ){
    bool waste = false;
    return Click( mousePos, waste );
}

void ScrollBox::Release( const Gdiplus::PointF& mousePos ){
    up_.Release( &mousePos );
    down_.Release( &mousePos );
    barGrabbed_ = false;
    barClicked_ = 0;
}

void ScrollBox::Wheel(short zDelta, Gdiplus::PointF& mousePos) {
    if( static_cast<int>( numVisibleData_ ) <= numRows_ ) return; // no scroll if no rows to scroll!
    if( mousePos.X > position_.X && mousePos.X < (position_.X + totalWidth_) &&
        mousePos.Y > position_.Y && mousePos.Y < ( position_.Y + (numRows_ * rowHeight_ ))) {
            if( zDelta < 0 )
                Scroll(1);
            else
                Scroll(-1);
        }
}

int ScrollBox::SelectedRow(){
    return selectedRow_;
}

void ScrollBox::ClearSelectedRow(){
    selectedRow_ = -1;
}

void ScrollBox::Refresh(){
    // This is required on the off-chance some rows have been made "invisible".
    // the partition brings the visible rows to the front of the container, and
    // the printing process below stops when it gets to the first invisible row or
    // reaches the end of the data.
    numVisibleData_ = distance(data_->begin(), stable_partition(data_->begin(), data_->end(), mem_fun(&RowData::IsVisible)));
    // count how many data are actually visible
    ValidateTopRow();
    UpdateScrollBar();
}

void ScrollBox::Scroll(int rows){
    currentTopRow_ += rows;
    
    ValidateTopRow();
    
    UpdateScrollBarPosition();
}

void ScrollBox::ValidateTopRow(){
    // Keep these checks in the same order.
    
    // Basic check for invalid top row
    if( currentTopRow_ < 0 )
        currentTopRow_ = 0 ;
    
    // This ensures top row, where possible, avoids unnecessary blank rows.    
    // If more data is available than will fit in the table...    
    if( static_cast<int>( numVisibleData_ ) - numRows_ > 0 ) {
        if( currentTopRow_ > (static_cast<int>( numVisibleData_ ) - numRows_ ) ) { // ...and the top row exceeds the difference...
            currentTopRow_ = (numVisibleData_ - numRows_); // ...the top row is set to equal the difference.
        }
    }
    
    // If there are more rows than visible data...
    if( numRows_ - static_cast<int>( numVisibleData_ ) > 0 ) {
            currentTopRow_ = 0; //...reset top row to very top.
    }
}

void ScrollBox::DragBar(const Gdiplus::PointF &mousePos) {
    scrollBar_.Y = mousePos.Y - grabPoint_;
    // Restrict within limits
    if( scrollBar_.Y < (scrollRectangle_.Y + 1.0f) )
        scrollBar_.Y = scrollRectangle_.Y + 1.0f;
    
    REAL barBottom = scrollBar_.GetBottom();
    REAL boxBottom = scrollRectangle_.GetBottom();
        
    if( barBottom >= boxBottom ){
        scrollBar_.Y = scrollRectangle_.GetBottom() - scrollBar_.Height - 1.0f;
    }
    
    // Calculate new top row, based on scroll bar position
    float numPages = numVisibleData_ - numRows_; // How many "pages" of rows are possible.
    float numer = (scrollBar_.Y - scrollRectangle_.Y -1) * (numPages / (scrollRectangle_.Height - scrollBar_.Height - 2.0f));
    float temp = floor( numer + 0.5f);
    currentTopRow_ = static_cast<int> (temp);
    ValidateTopRow();
}

void ScrollBox::SwapData(TableData* data){
    data_ = data;
    ValidateTopRow();
    UpdateScrollBarPosition();
}