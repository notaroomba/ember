#ifndef UIDROPDOWNMENU_H
#define UIDROPDOWNMENU_H

#include "UIComponent.h"
#define MAX_DROPDOWN_ITEMS 5

class UIDropdownMenu : public Component {
public:
  String items[MAX_DROPDOWN_ITEMS];
  uint8_t itemCount;
  int16_t x, y, width, height;
  uint8_t selectedIndex;
  bool expanded;
  uint16_t bgColor, textColor, selectedColor;
  void (*onItemSelect)(uint8_t);

  UIDropdownMenu(int16_t xPos, int16_t yPos, int16_t w, int16_t h,
                 uint16_t bg = BLACK, uint16_t textC = WHITE, uint16_t selC = WHITE, void (*itemSelectHandler)(uint8_t) = nullptr)
    : x(xPos), y(yPos), width(w), height(h), bgColor(bg), textColor(textC), selectedColor(selC),
      itemCount(0), selectedIndex(0), expanded(false), onItemSelect(itemSelectHandler) {}

  void addItem(String item) {
    if (itemCount < MAX_DROPDOWN_ITEMS) {
      items[itemCount++] = item;
    }
  }

  void render(Adafruit_SSD1306 &display) override {
    // Draw dropdown box
    display.drawRect(x, y, width, height, bgColor);
    // Display selected item
    display.setTextSize(1);
    display.setTextColor(textColor);
    display.setCursor(x + 2, y + (height - 8) / 2);
    display.println(items[selectedIndex]);

    if (expanded) {
      // Display all items
      for (uint8_t i = 0; i < itemCount; i++) {
        int16_t itemY = y + height * (i + 1);
        display.fillRect(x, itemY, width, height, (i == selectedIndex) ? selectedColor : bgColor);
        display.setTextColor((i == selectedIndex) ? bgColor : textColor);
        display.setCursor(x + 2, itemY + (height - 8) / 2);
        display.println(items[i]);
      }
    }
  }

  void toggle() {
    expanded = !expanded;
  }

  void selectNext() {
    if (expanded) {
      selectedIndex = (selectedIndex + 1) % itemCount;
    }
  }

  void selectPrevious() {
    if (expanded) {
      selectedIndex = (selectedIndex == 0) ? itemCount - 1 : selectedIndex - 1;
    }
  }

  void confirmSelection() {
    if (onItemSelect) onItemSelect(selectedIndex);
    expanded = false;
  }
};

#endif

/*
#include "UIDropdownMenu.h"

void onDropdownSelect(uint8_t index) {
  Serial.print("Dropdown selected: ");
  Serial.println(index);
}

UIDropdownMenu* dropdownMenu = new UIDropdownMenu(10, 20, 100, 12, BLACK, WHITE, WHITE, onDropdownSelect);

void setup() {
  // ... existing setup code ...
  dropdownMenu->addItem("Option 1");
  dropdownMenu->addItem("Option 2");
  dropdownMenu->addItem("Option 3");
  ui.addComponent(dropdownMenu);
}

void loop() {
  // Simulate interaction
  delay(1000);
  dropdownMenu->toggle(); // Open dropdown
  ui.render();
  delay(1000);
  dropdownMenu->selectNext();
  ui.render();
  delay(1000);
  dropdownMenu->confirmSelection();
  ui.render();
}

*/