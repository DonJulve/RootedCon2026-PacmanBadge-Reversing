#include "ui.h"
#include "sdl.h"

#define BG_COLOR            0x0000
#define BAR_COLOR           0x001F // Blueish
#define TEXT_COLOR          0xFFFF
#define TEXT2_COLOR         0xF800 // Redish
#define SELECTED_TEXT_COLOR 0xFFE0 // Yellowish
#define SELECTED_BG_COLOR   0x03E0 // Greenish

UI::UI(Arduino_GFX* screen) {
    this->screen = screen;
}

UI::~UI() {}

const char* UI::selectGame() {
    screen->fillScreen(0x0000);

    const char* title = "Julve Gameboy";
    int16_t title_w = 13 * 6; // Approx width for standard font
    int16_t title_x = (screen->width() - title_w) / 2;
    screen->setTextColor(0xFFFF, 0x0000);
    screen->setCursor(title_x, 10);
    screen->print(title);

    unsigned int last_input_time = 0;
    constexpr unsigned int delay = 250;
    max_items = (screen->height() - 32) / ITEM_HEIGHT;

    getGBFiles();
    drawFileList();

    const int size = files.size();
    while (true) {
        bool game_chosen = false;
        unsigned int now = millis();

        if (now - last_input_time > delay) {
            if (sdl_read_button(1)) { // Up (Physical Down)
                selected--;
                if (selected < 0) {
                    selected = (size - 1);
                    scroll_offset = selected - max_items + 1;
                } else if (selected < scroll_offset) scroll_offset = selected;
                if (scroll_offset < 0) scroll_offset = 0;
                if (scroll_offset > size - 1) scroll_offset = size - 1;
                drawFileList();
                last_input_time = now;
            }

            if (sdl_read_button(0)) { // Down (Physical Up)
                selected++;
                if (selected > (size - 1)) {
                    selected = 0;
                    scroll_offset = selected;
                } else if (selected >= scroll_offset + max_items)
                    scroll_offset = selected - max_items + 1;
                if (scroll_offset < 0) scroll_offset = 0;
                if (scroll_offset > size - 1) scroll_offset = size - 1;
                drawFileList();
                last_input_time = now;
            }

            if (sdl_read_button(3)) { // Left (Physical Right)
                int screen_pos = selected - scroll_offset;
                selected -= max_items;
                if (selected < 0) selected = 0;
                scroll_offset = selected - screen_pos;
                if (scroll_offset < 0) scroll_offset = 0;
                drawFileList();
                last_input_time = now;
            }

            if (sdl_read_button(2)) { // Right (Physical Left)
                int screen_pos = selected - scroll_offset;
                selected += max_items;
                if (selected > size - 1) selected = size - 1;
                scroll_offset = selected - screen_pos;
                if (scroll_offset < 0) scroll_offset = 0;
                if (scroll_offset > size - 1) scroll_offset = size - 1;
                drawFileList();
                last_input_time = now;
            }

            if (sdl_read_button(5) && size > 0 && selected >= 0 && selected < size) { // A (Physical B)
                game_chosen = true;
            }
        }

        if (game_chosen && size > 0) {
            static std::string game;
            game = files[selected];
            if (game.length() > 0 && game[0] != '/') game = "/" + game;
            
            screen->fillScreen(0x0000);
            return game.c_str();
        }
    }
}

void UI::getGBFiles() {
    File root = LittleFS.open("/");
    if (!root) {
        Serial.println("Failed to open LittleFS directory");
        return;
    }
    
    Serial.println("Loading ROMs from LittleFS...");
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            std::string filename = file.name();
            if (filename.length() >= 3 && filename.rfind(".gb") == filename.size() - 3) {
                files.push_back(filename);
            }
        }
        file = root.openNextFile();
    }
    root.close();
    Serial.printf("Total ROMs loaded: %d\n", files.size());
}

void UI::drawFileList() {
    if (prev_selected != selected) {
        screen->fillRect(0, 32, screen->width(), screen->height() - 32, BG_COLOR);
    }

    const int size = files.size();
    for (int i = 0; i < max_items; i++) {
        int item = i + scroll_offset;
        if (item >= size) break;

        std::string file = files[item];
        if (file.length() > 20) file = file.substr(0, 20);

        int y = i * ITEM_HEIGHT + 32;
        screen->setCursor(4, y);
        if (item == selected) {
            screen->setTextColor(SELECTED_TEXT_COLOR, BG_COLOR);
            std::string cursor_text = "> " + file;
            screen->print(cursor_text.c_str());
        } else {
            screen->setTextColor(TEXT_COLOR, BG_COLOR);
            std::string normal_text = "  " + file;
            screen->print(normal_text.c_str());
        }
    }

    prev_selected = selected;
}
