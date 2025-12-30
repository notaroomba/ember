#ifndef UISPINNER_H
#define UISPINNER_H

#include "UIComponent.h"

class UISpinner : public Component {
public:
  int16_t x, y, radius;
  uint16_t color;
  uint8_t frame;

  UISpinner(int16_t xPos, int16_t yPos, int16_t r, uint16_t c = WHITE)
    : x(xPos), y(yPos), radius(r), color(c), frame(0) {}

  void render(Adafruit_SSD1306 &display) override {
    // Draw spinner frame
    display.drawCircle(x, y, radius, color);

    // Draw rotating line
    int16_t x2 = x + radius * cos(frame * PI / 4);
    int16_t y2 = y + radius * sin(frame * PI / 4);
    display.drawLine(x, y, x2, y2, color);
  }

  void update() override {
    // Update frame for animation
    frame = (frame + 1) % 8;
  }
};

#endif
/*
#include "UISpinner.h"

UISpinner* spinner = new UISpinner(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 10);
ui.addComponent(spinner);

*/