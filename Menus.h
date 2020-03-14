// Menus.h
/*  TopMenu - the Main menu 
    NewSpeller - create a new speller
    SelectSpeller - select a speller from the existing accounts
    SpellerMenu - the Speller's activity menu
*/

#ifndef MENUS_H
#define MENUS_H

#include "Mode.h"
#include <vector>
#include "Keyboard.h"
#include "Definitions.h"


// Forward Declarations
class BackBuffer;
class Button;
class DBController;
class ScreenPrinter;
class ScrollBox;
class Speller;

class TopMenu : public Mode{
public:

    enum{SELECT,NEW,QUIT};

    TopMenu(unsigned int& nextMode, unsigned int previousMode, unsigned int id, BackBuffer* bb,
            const unsigned int numSpellers);
    virtual ~TopMenu();
    
    virtual void Update( double dt, const Gdiplus::PointF* cursorPos);
    virtual void Display(BackBuffer* bb);

    virtual void LMBUp(const Gdiplus::PointF* cursorPos);
    virtual void LMBDown(const Gdiplus::PointF* cursorPos, const double time=0.0);
    virtual void KeyDown(unsigned int key);
    virtual void KeyUp();
    virtual void RMBUp(const Gdiplus::PointF* cursorPos);
    virtual void RMBDown(const Gdiplus::PointF* cursorPos);
    virtual void Wheel(short zDelta, Gdiplus::PointF* mousePos);

private:
    Gdiplus::Image* pBackground_;   // Stores the background image
    std::vector<Button> buttons_;   // Stores the buttons.
    std::vector<Gdiplus::Bitmap*> pButtonImages_; // Stores button images
    const unsigned int numSpellers_; // How many user accounts are there?  Determines which buttons are active.
    

};


class NewSpeller: public Mode {
public:

    enum{OK,CANCEL,KEYBOARDS};

    NewSpeller(unsigned int& nextMode, unsigned int previousMode, unsigned int id, Gdiplus::Font* font,
               DBController* db, ScreenPrinter* sp, unsigned int& spellerID,
               TagList& tagList);
    virtual ~NewSpeller();
    
    virtual void Update( double dt, const Gdiplus::PointF* cursorPos);
    virtual void Display(BackBuffer* bb);

    virtual void LMBUp(const Gdiplus::PointF* cursorPos);
    virtual void LMBDown(const Gdiplus::PointF* cursorPos, const double time=0.0);
    virtual void KeyDown(unsigned int key);
    virtual void KeyUp();
    virtual void RMBUp(const Gdiplus::PointF* cursorPos);
    virtual void RMBDown(const Gdiplus::PointF* cursorPos);
    virtual void Wheel(short zDelta, Gdiplus::PointF* mousePos);
    
private:
    virtual void KeyboardSetup();
    virtual void SwitchKeyboard(int num);
    virtual void GetNextAvatar();
    virtual void EditSpellerName( wchar_t ch );
    virtual void CreateSpeller();
    
    void CreateSpellerTags(); // Sets up default entries in SpellerTags(DB)
    void SetDefaultDifficulty(); // Sets up default difficulty in DB

private:
    Gdiplus::Image* pBackground_;   // Stores the background image
    std::vector<Button*> buttons_;   // Stores the buttons.
    std::vector<Gdiplus::Bitmap*> pButtonImages_; // Stores button images
    Gdiplus::Image* pNameExists_;   // Name exists warning.
    
    // Avatar information
    IDList idList_;                     // IDs from database
    IDList::iterator displayedAvatar_;  // ID for current avatar
    Gdiplus::Image* pAvatar_;           // Image for current avatar
    const Gdiplus::RectF avatarZone_;   // Print location (Also sets size of avatar)
    

    
    // Keyboards
    Keyboard* keyboard_;    // Pointer to current keyboard
    Keyboard qwerty_;       // standard qwerty
    Keyboard abc_;          // alphabetical order
    Keyboard special_;      // special characters
    
    Gdiplus::Font* mpFont_;         
    DBController* pDB_;             // Database access
    ScreenPrinter* pScreenPrinter_; // Prints the speller's name
    std::wstring spellerName_;      // Speller's name
    const int maxCharacters_;       // limit for name.
    StringList existingNames_;      // List of names that already exist (speller name must be unique)
    bool showNameExistsWarning_;
    
    unsigned int& spellerID_;   // If a speller is created, it is stored here and passed back to cApp.cpp.
    TagList& tagList_;          // If a speller is created, the tag list is used to create the default entries in SpellerTags in the DB
};

class SelectSpeller : public Mode {

public:
    enum{OK,CANCEL};
    
    SelectSpeller(unsigned int& nextMode, unsigned int previousMode, unsigned int id, Gdiplus::Font* font,
               DBController* db, unsigned int& spellerID);
    virtual ~SelectSpeller();
    
    virtual void Update( double dt, const Gdiplus::PointF* cursorPos);
    virtual void Display(BackBuffer* bb);

    virtual void LMBUp(const Gdiplus::PointF* cursorPos);
    virtual void LMBDown(const Gdiplus::PointF* cursorPos, const double time=0.0);
    virtual void KeyDown(unsigned int key);
    virtual void KeyUp();
    virtual void RMBUp(const Gdiplus::PointF* cursorPos);
    virtual void RMBDown(const Gdiplus::PointF* cursorPos);
    virtual void Wheel(short zDelta, Gdiplus::PointF* mousePos);
    
private:
    void SetUpScrollBox();
    void GetData();
    void LoadSpeller(int rowID);
    
private:
    Gdiplus::Image* pBackground_;   // Stores the background image
    std::vector<Button*> buttons_;   // Stores the buttons.
    std::vector<Gdiplus::Bitmap*> pButtonImages_; // Stores button images

    // Avatar information
    IDList idList_;                     // IDs from database
    ImageList avatarList_;              // Images for avatars
    
    //ScrollBox
    ScrollBox* sbSpellerList_;          // Pointer to scrollbox for spellers
    TableData spellerData_;               // Speller data
    
    Gdiplus::Font* mpFont_;         
    DBController* pDB_;             // Database access
    unsigned int& spellerID_;
};

class SpellerMenu : public Mode {

public:
    enum{MAIN, WORDOPTIONS, QUICKSPELL};
    
    SpellerMenu(unsigned int& nextMode, unsigned int previousMode, unsigned int id, Gdiplus::Font* font,
               DBController* db);
    virtual ~SpellerMenu();
    
    virtual void Update( double dt, const Gdiplus::PointF* cursorPos);
    virtual void Display(BackBuffer* bb);

    virtual void LMBUp(const Gdiplus::PointF* cursorPos);
    virtual void LMBDown(const Gdiplus::PointF* cursorPos, const double time=0.0);
    virtual void KeyDown(unsigned int key);
    virtual void KeyUp();
    virtual void RMBUp(const Gdiplus::PointF* cursorPos);
    virtual void RMBDown(const Gdiplus::PointF* cursorPos);
    virtual void Wheel(short zDelta, Gdiplus::PointF* mousePos);
    
private:
    Gdiplus::Image* pBackground_;   // Stores the background image
    std::vector<Button*> buttons_;   // Stores the buttons.
    std::vector<Gdiplus::Bitmap*> pButtonImages_; // Stores button images
    
    Gdiplus::Font* mpFont_;         
    DBController* pDB_;             // Database access
    
    
};

#endif // MENUS_H