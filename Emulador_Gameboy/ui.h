#ifndef UI_H
#define UI_H

#include <LittleFS.h>
#include <Arduino_GFX_Library.h>
#include <string>
#include <vector>

class UI {
public:
    UI(Arduino_GFX* screen);
    ~UI();
    const char* selectGame();

private:
    void getGBFiles();
    void drawFileList();
    
    Arduino_GFX* screen = nullptr;
    int selected = 0;
    int prev_selected = 0;
    int scroll_offset = 0;
    int max_items = 0;
    static constexpr int ITEM_HEIGHT = 12;
    std::vector<std::string> files;
};

#endif
