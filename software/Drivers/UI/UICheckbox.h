#ifndef UICHECKBOX_H
#define UICHECKBOX_H

#include "UIComponent.h"

class UICheckbox : public Component {
public:
  String label;
  int16_t x, y;
  bool checked;
  uint16_t boxColor, checkColor, textColor;
  void (*onToggle)(bool);

  UICheckbox(String lbl, int16_t xPos, int16_t yPos, bool initChecked = false,
             uint16_t boxC = WHITE, uint16_t checkC = BLACK, uint16_t textC = WHITE,
             void (*toggleHandler)(bool) = nullptr)
    : label(lbl), x(xPos), y(yPos), checked(initChecked),
      boxColor(boxC), checkColor(checkC), textColor(textC), onToggle(toggleHandler) {}

  void render(Adafruit_SSD1306 &display) override {
    display.drawRect(x, y, 10, 10, boxColor);
    if (checked) {
      display.drawLine(x, y, x + 10, y + 10, checkColor);
      display.drawLine(x + 10, y, x, y + 10, checkColor);
    }
    display.setTextSize(1);
    display.setTextColor(textColor);
    display.setCursor(x + 14, y + 1);
    display.println(label);
  }

  void toggle() {
    checked = !checked;
    if (onToggle) onToggle(checked);
  }
};

#endif
/*
void onCheckboxToggle(bool isChecked) {
  Serial.print("Checkbox is ");
  Serial.println(isChecked ? "checked" : "unchecked");
}

UICheckbox* agreeCheckbox = new UICheckbox("Agree", 0, 40, false, WHITE, BLACK, WHITE, onCheckboxToggle);
ui.addComponent(agreeCheckbox);

*/