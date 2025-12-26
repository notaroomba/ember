#ifndef UILIST_H
#define UILIST_H

#include "UIComponent.h"

class UIList : public Component {
public:
  String* items;
  uint8_t itemCount;
  int16_t x, y;
  uint8_t textSize;
  uint16_t textColor;
  uint8_t selectedIndex;
  void (*onItemSelect)(uint8_t);

  UIList(String itemArray[], uint8_t count, int16_t xPos, int16_t yPos, uint8_t size = 1, uint16_t color = WHITE, void (*itemSelectHandler)(uint8_t) = nullptr)
    : items(itemArray), itemCount(count), x(xPos), y(yPos), textSize(size), textColor(color), selectedIndex(0), onItemSelect(itemSelectHandler) {}

  void render(Adafruit_SSD1306 &display) override {
    display.setTextSize(textSize);
    int16_t yOffset = y;
    for (uint8_t i = 0; i < itemCount; i++) {
      if (i == selectedIndex) {
        display.fillRect(x - 2, yOffset - 2, display.width(), textSize * 8, WHITE);
        display.setTextColor(BLACK);
      } else {
        display.setTextColor(textColor);
      }
      display.setCursor(x, yOffset);
      display.println(items[i]);
      yOffset += textSize * 8;
    }
  }

  void nextItem() {
    selectedIndex = (selectedIndex + 1) % itemCount;
    if (onItemSelect) onItemSelect(selectedIndex);
  }

  void previousItem() {
    selectedIndex = (selectedIndex == 0) ? itemCount - 1 : selectedIndex - 1;
    if (onItemSelect) onItemSelect(selectedIndex);
  }
};

#endif
/*
String menuItems[] = {"Option 1", "Option 2", "Option 3"};

void onMenuSelect(uint8_t index) {
  Serial.print("Selected: ");
  Serial.println(menuItems[index]);
}

UIList* menuList = new UIList(menuItems, 3, 0, 20, 1, WHITE, onMenuSelect);
ui.addComponent(menuList);

*/