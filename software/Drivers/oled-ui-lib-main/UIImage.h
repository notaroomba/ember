#ifndef UIIMAGE_H
#define UIIMAGE_H

#include "UIComponent.h"

class UIImage : public Component {
public:
  const uint8_t* bitmap;
  int16_t x, y;
  uint8_t w, h;
  uint16_t color;

  UIImage(const uint8_t* bmp, int16_t xPos, int16_t yPos, uint8_t width, uint8_t height, uint16_t c = WHITE)
    : bitmap(bmp), x(xPos), y(yPos), w(width), h(height), color(c) {}

  void render(Adafruit_SSD1306 &display) override {
    display.drawBitmap(x, y, bitmap, w, h, color);
  }
};

#endif
/*
static const unsigned char PROGMEM myBitmap[] = {
  // Bitmap data
};

UIImage* icon = new UIImage(myBitmap, 56, 24, 16, 16);
ui.addComponent(icon);

*/