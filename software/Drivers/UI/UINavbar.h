#ifndef UINAVBAR_H
#define UINAVBAR_H

#include "UIComponent.h"
#define MAX_NAV_ITEMS 5

class UINavbarItem {
public:
  String label;
  void (*onSelect)();

  UINavbarItem(String lbl, void (*selectHandler)() = nullptr)
    : label(lbl), onSelect(selectHandler) {}
};

class UINavbar : public Component {
public:
  UINavbarItem* items[MAX_NAV_ITEMS];
  uint8_t itemCount;
  int16_t x, y, width, height;
  uint8_t selectedIndex;
  uint16_t bgColor, textColor, selectedColor;

  UINavbar(int16_t xPos, int16_t yPos, int16_t w, int16_t h,
           uint16_t bg = BLACK, uint16_t textC = WHITE, uint16_t selC = WHITE)
    : x(xPos), y(yPos), width(w), height(h),
      bgColor(bg), textColor(textC), selectedColor(selC),
      itemCount(0), selectedIndex(0) {}

  void addItem(UINavbarItem* item) {
    if (itemCount < MAX_NAV_ITEMS) {
      items[itemCount++] = item;
    }
  }

  void render(Adafruit_SSD1306 &display) override {
    // Draw navbar background
    display.fillRect(x, y, width, height, bgColor);

    // Calculate width per item
    int16_t itemWidth = width / itemCount;

    // Render items
    for (uint8_t i = 0; i < itemCount; i++) {
      int16_t itemX = x + i * itemWidth;
      if (i == selectedIndex) {
        // Highlight selected item
        display.fillRect(itemX, y, itemWidth, height, selectedColor);
        display.setTextColor(bgColor);
      } else {
        display.setTextColor(textColor);
      }
      display.setTextSize(1);
      display.setCursor(itemX + (itemWidth - items[i]->label.length() * 6) / 2, y + (height - 8) / 2);
      display.println(items[i]->label);
    }
  }

  void nextItem() {
    selectedIndex = (selectedIndex + 1) % itemCount;
  }

  void previousItem() {
    selectedIndex = (selectedIndex == 0) ? itemCount - 1 : selectedIndex - 1;
  }

  void selectItem() {
    if (items[selectedIndex]->onSelect) {
      items[selectedIndex]->onSelect();
    }
  }
};

#endif
/*
#include "UINavbar.h"

void homeSelected() {
  Serial.println("Home selected");
}

void settingsSelected() {
  Serial.println("Settings selected");
}

UINavbar* navbar = new UINavbar(0, 0, SCREEN_WIDTH, 12);
navbar->addItem(new UINavbarItem("Home", homeSelected));
navbar->addItem(new UINavbarItem("Settings", settingsSelected));
ui.addComponent(navbar);

*/