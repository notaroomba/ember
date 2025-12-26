#ifndef UIRADIOBUTTON_H
#define UIRADIOBUTTON_H

#include "UIComponent.h"
#define MAX_RADIO_OPTIONS 5

class UIRadioOption {
public:
  String label;
  bool selected;

  UIRadioOption(String lbl, bool sel = false)
    : label(lbl), selected(sel) {}
};

class UIRadioButton : public Component {
public:
  UIRadioOption* options[MAX_RADIO_OPTIONS];
  uint8_t optionCount;
  int16_t x, y;
  uint8_t textSize;
  uint16_t textColor;
  void (*onSelectionChange)(uint8_t);

  UIRadioButton(int16_t xPos, int16_t yPos, uint8_t size = 1, uint16_t color = WHITE, void (*selectionHandler)(uint8_t) = nullptr)
    : x(xPos), y(yPos), textSize(size), textColor(color), optionCount(0), onSelectionChange(selectionHandler) {}

  void addOption(UIRadioOption* option) {
    if (optionCount < MAX_RADIO_OPTIONS) {
      options[optionCount++] = option;
    }
  }

  void render(Adafruit_SSD1306 &display) override {
    display.setTextSize(textSize);
    int16_t yOffset = y;
    for (uint8_t i = 0; i < optionCount; i++) {
      // Draw circle
      display.drawCircle(x, yOffset + 4, 4, WHITE);
      if (options[i]->selected) {
        display.fillCircle(x, yOffset + 4, 2, WHITE);
      }
      // Draw label
      display.setTextColor(textColor);
      display.setCursor(x + 10, yOffset);
      display.println(options[i]->label);
      yOffset += textSize * 10;
    }
  }

  void selectOption(uint8_t index) {
    if (index < optionCount) {
      for (uint8_t i = 0; i < optionCount; i++) {
        options[i]->selected = (i == index);
      }
      if (onSelectionChange) onSelectionChange(index);
    }
  }
};

#endif
/*
#include "UIRadioButton.h"

void onRadioSelection(uint8_t index) {
  Serial.print("Radio option selected: ");
  Serial.println(index);
}

UIRadioButton* radioGroup = new UIRadioButton(10, 20, 1, WHITE, onRadioSelection);

void setup() {
  // ... existing setup code ...
  radioGroup->addOption(new UIRadioOption("Option A"));
  radioGroup->addOption(new UIRadioOption("Option B"));
  radioGroup->addOption(new UIRadioOption("Option C"));
  ui.addComponent(radioGroup);
}

void loop() {
  // Simulate selection
  for (uint8_t i = 0; i < 3; i++) {
    radioGroup->selectOption(i);
    ui.render();
    delay(1000);
  }
}

*/