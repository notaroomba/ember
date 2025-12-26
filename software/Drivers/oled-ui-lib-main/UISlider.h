#ifndef UISLIDER_H
#define UISLIDER_H

#include "UIComponent.h"

class UISlider : public Component {
public:
  int16_t x, y, width, height;
  int16_t minValue, maxValue;
  int16_t value;
  uint16_t bgColor, knobColor;
  void (*onValueChange)(int16_t);

  UISlider(int16_t xPos, int16_t yPos, int16_t w, int16_t h,
           int16_t minVal, int16_t maxVal, int16_t initVal,
           uint16_t bg = WHITE, uint16_t knob = BLACK,
           void (*valueChangeHandler)(int16_t) = nullptr)
    : x(xPos), y(yPos), width(w), height(h),
      minValue(minVal), maxValue(maxVal), value(initVal),
      bgColor(bg), knobColor(knob), onValueChange(valueChangeHandler) {}

  void render(Adafruit_SSD1306 &display) override {
    display.drawRect(x, y, width, height, bgColor);
    int16_t knobX = x + ((value - minValue) * (width - 4)) / (maxValue - minValue);
    display.fillRect(knobX, y + 2, 4, height - 4, knobColor);
  }

  void setValue(int16_t newValue) {
    value = constrain(newValue, minValue, maxValue);
    if (onValueChange) onValueChange(value);
  }

  void increase() { setValue(value + 1); }
  void decrease() { setValue(value - 1); }
};

#endif
/*
int brightness = 50;

void onBrightnessChange(int16_t newValue) {
  brightness = newValue;
  Serial.print("Brightness: ");
  Serial.println(brightness);
}

UISlider* brightnessSlider = new UISlider(10, 40, 100, 10, 0, 100, brightness, WHITE, BLACK, onBrightnessChange);
ui.addComponent(brightnessSlider);

*/