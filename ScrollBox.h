// ScrollBox.h
#ifndef SCROLLBOX_H
#define SCROLLBOX_H

#include <windows.h>
#include <gdiplus.h>
#include <list>
#include <vector>
#include <map>
#include "Button.h"
#include <cstdlib>

class BackBuffer;

class Column{
public:
    enum Justify{Left, Centre, Right};
    
    Column();
    Column(Justify j, int columnWidth);
    
    virtual ~Column();
    
    Gdiplus::StringAlignment GetJustification();
    
    virtual void Print(BackBuffer& bb, const std::wstring& data, const Gdiplus::RectF& rec, const Gdiplus::Color& col,
                        bool active) = 0;
    virtual int  ColumnWidth() const;
    
protected:
    int justification_;
    int columnWidth_;
};

class TextColumn : public Column {
public:
    TextColumn();
    TextColumn(Justify j, int columnWidth, Gdiplus::Font* font);
    
    virtual ~TextColumn();
    
    virtual void Print(BackBuffer& bb, const std::wstring& data, const Gdiplus::RectF& rec, const Gdiplus::Color& col,
                        bool active);
    
protected:
    Gdiplus::Font* font_;
};

/* -------Difference between IconColumn and ImageColumn--------
   The IconColumn uses the same image file for every row, but cuts out a particular part of that image for display.
   Hence, different rows can feature different icons - useful for active / inactive type symbols.
   The ImageColumn displays a different image file for every row, but displays the entire image.  Useful for lists
   of avatars.
*/

// Shows icons cut out from a single image file.
class IconColumn : public Column {
public:
    typedef std::vector<std::wstring> ValueList;

    IconColumn();
    IconColumn(Column::Justify j, int columnWidth,
                         Gdiplus::Image* pBitmap,
                         IconColumn::ValueList vl);
                         
    ~IconColumn();
    
    virtual void Print(BackBuffer& bb, const std::wstring& data, const Gdiplus::RectF& rec, const Gdiplus::Color& col,
                        bool active);

protected:
    Gdiplus::Image* pBitmap_;
    ValueList values_;
    int imageWidth_, imageHeight_;
    
    
};

// Shows images specified by filename data.
class ImageColumn : public Column {
public:
    ImageColumn();
    ImageColumn(Column::Justify j, int columnWidth, int imageWidth, int imageHeight );
                         
    ~ImageColumn();
    
    virtual void Print(BackBuffer& bb, const std::wstring& data, const Gdiplus::RectF& rec, const Gdiplus::Color& col,
                        bool active);

protected:
    int imageWidth_, imageHeight_;
    Gdiplus::REAL justifyAdjustment_;
    
    
};

/*  RowData struct (improvement over previous RowData, which was simply a vector<wstring>, with the first two
    entries string versions of active state and row ID).
    
    Includes bool for active state
    Includes integer for row reference
    Includes vector of wstrings for other data
    THOUGHTS:- add bools for visibility, editability?
*/
struct RowData{
    typedef std::vector<std::wstring> Data;
    RowData();
    RowData(int dataID, Data data, bool active = true, bool visible = true );
    bool IsVisible();
    
    // These three functions alter the active_ member, regardless of visibility.
    void ToggleActive();
    void Activate();
    void Deactivate();
    //These three functions check the data is visible first; only visible data is altered.
    void VisToggleActive();
    void VisActivate();
    void VisDeactivate();
    void ToggleVisibility();
    void Show();
    void Hide();
    
    bool EqualToID( const unsigned int id ) const; // Function to see if id passed in is equal.  Used as predicate.
    bool active_;
    bool visible_;
    int  dataID_;
    Data data_;
};

// COLUMN SORTING TEMPLATES
// Used to sort alphabetically by a specified column in the ScrollBox.
// Undefined if out of bounds value given for N.
// Case insensitive, and converts all diacritics to "normal" letters.
template <int N>
bool ColumnAlphaSort(const RowData* l, const RowData* r){
	return ToLower(l->data_[N], true) < ToLower(r->data_[N], true);
}
// Used to sort numerically using a specified column in the ScrollBox.
// Undefined if out of bounds or the conversion fails.
template <int N>
bool ColumnNumericSort(const RowData* l, const RowData* r){
    return _wtoi(l->data_[N].c_str()) < _wtoi(r->data_[N].c_str());
}

// TODO: Make many of the settings customisable by client.
class ScrollBox{
public:
    //typedef std::vector<std::wstring> RowData; // Single row of data
    typedef std::vector<RowData*> TableData;    // Collection of all rows 
    ScrollBox(TableData* data, Gdiplus::PointF pos, int rowHeight, int numRows = 0);
    
    ~ScrollBox();
    void AddData( RowData* data );
    
    // For Text columns
    void AddColumn( Column::Justify j, int columnWidth,
                    Gdiplus::Font* font );
    // For Icon columns
    void AddColumn( Column::Justify j, int columnWidth,
                    Gdiplus::Image* image, IconColumn::ValueList values );
    // For Image columns
    void AddColumn( Column::Justify j, int columnWidth,
                    int imageWidth, int imageHeight );
                    
    void SetRowColours( Gdiplus::Color& colour1, Gdiplus::Color& colour2 );
    void SetRowColours( Gdiplus::Color& colour); //sets both rows to the same colour
    void SetSelectedColour( Gdiplus::Color& colour );
    //TODO: ink colour / active ink colour
    
    void Update( double dt, const Gdiplus::PointF& mousePos );
    void Display( BackBuffer& bb );
    
    // Clicking on a row
    // If a cell is clicked, this function returns the row and col (as pair<int,int>) of the table.
    // If nothing recognisable has been clicked, it returns -2 in pair.first
    // If something else is clicked, such as the scroll bar, up/down buttons, it returns -1,-1 for the row,col.
    // In addition, this sets a passed in bool to true if a double click is detected.
    std::pair<int,int> Click( const Gdiplus::PointF& mousePos,
                              bool& dblclkd, double time=0.0 );
                              
    std::pair<int,int> Click( const Gdiplus::PointF& mousePos ); // As above, but without double click aspect.
                              
    void Release( const Gdiplus::PointF& mousePos );
    
    void Wheel(short zDelta, Gdiplus::PointF& mousePos);
    
    int SelectedRow(); // return selected row.  -1 means no row selected.
    void ClearSelectedRow(); // clear selection.
    
    void Refresh(); // Sets box to show top row; recalculates scrollbar; resorts visibility.
    
    void SwapData( TableData* data ); // point to a different data set - this is experimental.  New data must have same data structure.
    

private:
    void UpdateAfterNewColumn(int columnWidth);
    void UpdateScrollBar();
    void UpdateScrollBarPosition();
    void Scroll(int rows);
    void DragBar(const Gdiplus::PointF& mousePos);
    void ValidateTopRow(); // Checks that currentTopRow_ is valid.

private:
    TableData* data_;// Note: POINTER data member - data is created by the client, but can be manipulated by ScrollBox.
                       
    std::list<Column*> columns_;
    Gdiplus::PointF position_;
    int rowHeight_; // How many pixels high each row is - set by client
    int totalWidth_; // How many pixels wide the table is, excluding the scroll bar - calculated.
    Gdiplus::Color rowColour1_;     // This colour, and the one below, make the banded row colours.
    Gdiplus::Color rowColour2_;     // 
    int numRows_;                   // How many rows to be displayed.  Calculated if not set by client.
    int currentTopRow_;             // The item in the RowData displayed on the top (visible) row.
    bool selectable_;               // Whether a row in the scrollbox can be visibly selected.
    int selectedRow_;               // The currently selected row.  -1 if none selected.
    unsigned int numVisibleData_;   // How many rows of data are "visible".
    TableData::iterator cutOffPoint; // Position in TableData to stop printing - this is where the invisible data starts.
    
    Gdiplus::Color selectionColour_; // Background colour for a selected row.
    
    // The following are the various objects for the controls for scrolling the box.
    Gdiplus::Bitmap* upImage_;
    Gdiplus::Bitmap* downImage_;
    Button up_, down_;
    Gdiplus::RectF scrollRectangle_; // The slide area of the scroll bar.
    Gdiplus::RectF scrollBar_;       // The sliding dragbar.
    bool barGrabbed_;           // True if bar is clicked on.  Cleared when button released.
    int barClicked_;            // -1=bar area clicked above bar; +1=bar area clicked below. Set to 0 at release.
    Gdiplus::REAL grabPoint_;   // Difference between top of bar and point grabbed on bar.
    bool scrollBarDisabled_;    // Disable controls if scrollbar not needed.
    double timer_;              // Controls repeat effect of holding down scroll buttons
    
};

#endif; //SCROLLBOX_H