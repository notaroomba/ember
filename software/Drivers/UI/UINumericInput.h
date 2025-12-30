#ifndef UINUMERICINPUT_H
#define UINUMERICINPUT_H

#include "UIComponent.h"

class UINumericInput : public Component {
public:
  int16_t x, y;
  int32_t value, minValue, maxValue;
  uint8_t textSize;
  uint16_t textColor;
  void (*onValueChange)(int32_t);

  UINumericInput(int16_t xPos, int16_t yPos, int32_t minVal, int32_t maxVal, int32_t initVal = 0,
                 uint8_t size = 1, uint16_t color = WHITE, void (*valueChangeHandler)(int32_t) = nullptr)
    : x(xPos), y(yPos), minValue(minVal), maxValue(maxVal), value(initVal),
      textSize(size), textColor(color), onValueChange(valueChangeHandler) {}

  void render(Adafruit_SSD1306 &display) override {
    display.setTextSize(textSize);
    display.setTextColor(textColor);
    display.setCursor(x, y);
    display.println(value);
  }

  void increment() {
    if (value < maxValue) {
      value++;
      if (onValueChange) onValueChange(value);
    }
  }

  void decrement() {
    if (value > minValue) {
      value--;
      if (onValueChange) onValueChange(value);
    }
  }
};

#endif
/*
#include "UINumericInput.h"

void onNumericValueChange(int32_t newValue) {
  Serial.print("Numeric value changed to: ");
  Serial.println(newValue);
}

UINumericInput* numericInput = new UINumericInput(60, 30, 0, 100, 50, 2, WHITE, onNumericValueChange);

void setup() {
  // ... existing setup code ...
  ui.addComponent(numericInput);
}

void loop() {
  // Simulate value change
  delay(1000);
  numericInput->increment();
  ui.render();
}

*/