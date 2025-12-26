#ifndef UILABEL_H
#define UILABEL_H

#include "UIComponent.h"

class UILabel : public Component {
public:
  String text; // The text to display
  int16_t x, y; // The x and y position of the text
  uint8_t textSize; // The size of the text
  uint16_t textColor; // The color of the text

  UILabel(String t, int16_t xPos, int16_t yPos, uint8_t size = 1, uint16_t color = WHITE)
    : text(t), x(xPos), y(yPos), textSize(size), textColor(color) {}

  void render(Adafruit_SSD1306 &display) override {
    display.setTextSize(textSize);
    display.setTextColor(textColor);
    display.setCursor(x, y);
    display.println(text);
  }
};

#endif
