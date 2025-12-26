#ifndef UITOGGLE_SWITCH_H
#define UITOGGLE_SWITCH_H

#include "UIComponent.h"

class UIToggleSwitch : public Component {
public:
  int16_t x, y, width, height;
  bool state;
  uint16_t onColor, offColor;
  void (*onToggle)(bool);

  UIToggleSwitch(int16_t xPos, int16_t yPos, int16_t w = 30, int16_t h = 15,
                 bool initialState = false, uint16_t onC = WHITE, uint16_t offC = BLACK, void (*toggleHandler)(bool) = nullptr)
    : x(xPos), y(yPos), width(w), height(h), state(initialState), onColor(onC), offColor(offC), onToggle(toggleHandler) {}

  void render(Adafruit_SSD1306 &display) override {
    // Draw switch background
    display.drawRect(x, y, width, height, WHITE);
    // Draw switch knob
    int16_t knobX = state ? x + width - height : x;
    display.fillRect(knobX, y, height, height, state ? onColor : offColor);
  }

  void toggle() {
    state = !state;
    if (onToggle) onToggle(state);
  }
};

#endif
/*
#include "UIToggleSwitch.h"

void onToggleSwitch(bool newState) {
  Serial.print("Toggle Switch is now ");
  Serial.println(newState ? "ON" : "OFF");
}

UIToggleSwitch* toggleSwitch = new UIToggleSwitch(50, 40, 30, 15, false, WHITE, BLACK, onToggleSwitch);

void setup() {
  // ... existing setup code ...
  ui.addComponent(toggleSwitch);
}

void loop() {
  // Simulate toggle
  delay(2000);
  toggleSwitch->toggle();
  ui.render();
}

*/