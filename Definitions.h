//Definitions.h
// Contains typedefs
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <list>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <windows.h>
#include <gdiplus.h>

#define MAPVK_VK_TO_CHAR 2 // Need this for KeyStroke function

typedef unsigned int uint;
class Tag;
class Word;

typedef std::vector<Tag> TagList;
typedef std::map<unsigned int, Word> WordBank;
typedef std::list<Gdiplus::Image*> ImageList;
typedef std::set<int> IDList;

typedef std::list<std::wstring> StringList;
typedef std::vector<std::wstring> StringVec;
typedef std::list<std::wstring> FileNameList;

const unsigned int WORD_LENGTH_LIMIT = 20; // Max characters any word can be. TODO: Enforce throughout.
const unsigned int TOP_LEVEL_RANK_1 = 4;
const unsigned int TOP_LEVEL_RANK_2 = 9;
const unsigned int TOP_LEVEL_RANK_3 = 14;
const unsigned int TOP_LEVEL_RANK_4 = 15;

struct RowData; //forward declaration
typedef std::vector<RowData*> TableData;    // Used by ScrollBox.

enum ProgModes{
    TITLE=1,
    MENU,
    QUIT,
    NEWSPELLER,
    SELECTSPELLER,
    WORDLISTOPTIONS,
    LOADSPELLER,
    SPELLERMENU,
    QUICKSPELL,
    WORDWORKOUT,
    SPELLINGSPOTTING
};

enum Keys{
    DEL = 8,
    TAB = 9,
    ENTER = 13,
    SHIFT = 16,
    CAPSLOCK = 20,
    SPACE = 32
};


#endif // DEFINITIONS_H