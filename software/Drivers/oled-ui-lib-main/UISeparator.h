#ifndef UISEPARATOR_H
#define UISEPARATOR_H

#include "UIComponent.h"

class UISeparator : public Component {
public:
  int16_t x, y, width;
  uint16_t color;

  UISeparator(int16_t xPos, int16_t yPos, int16_t w, uint16_t c = WHITE)
    : x(xPos), y(yPos), width(w), color(c) {}

  void render(Adafruit_SSD1306 &display) override {
    display.drawLine(x, y, x + width, y, color);
  }
};

#endif
/*
#include "UISeparator.h"

// Create a separator instance
UISeparator* separator = new UISeparator(0, 30, SCREEN_WIDTH);
ui.addComponent(separator);

*/