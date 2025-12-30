#ifndef UIBUTTON_H
#define UIBUTTON_H

#include "UIComponent.h"

class UIButton : public Component {
public:
  String label;
  int16_t x, y, width, height;
  uint16_t bgColor, textColor;
  void (*onClick)();

  UIButton(String lbl, int16_t xPos, int16_t yPos, int16_t w, int16_t h,
           uint16_t bg = WHITE, uint16_t textC = BLACK, void (*clickHandler)() = nullptr)
    : label(lbl), x(xPos), y(yPos), width(w), height(h),
      bgColor(bg), textColor(textC), onClick(clickHandler) {}

  void render(Adafruit_SSD1306 &display) override {
    display.fillRect(x, y, width, height, bgColor);
    display.setTextSize(1);
    display.setTextColor(textColor);
    int16_t xLabel = x + (width - (label.length() * 6)) / 2;
    int16_t yLabel = y + (height - 8) / 2;
    display.setCursor(xLabel, yLabel);
    display.println(label);
  }

  void checkPress(int16_t pressX, int16_t pressY) {
    if (pressX >= x && pressX <= x + width && pressY >= y && pressY <= y + height) {
      if (onClick) onClick();
    }
  }
};

#endif
