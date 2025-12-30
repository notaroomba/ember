#ifndef UIPROGRESSBAR_H
#define UIPROGRESSBAR_H

#include "UIComponent.h"

class UIProgressBar : public Component {
public:
  int16_t x, y, width, height;
  uint8_t progress; // 0 to 100
  uint16_t bgColor, barColor;

  UIProgressBar(int16_t xPos, int16_t yPos, int16_t w, int16_t h,
                uint16_t bg = BLACK, uint16_t bar = WHITE)
    : x(xPos), y(yPos), width(w), height(h), progress(0), bgColor(bg), barColor(bar) {}

  void render(Adafruit_SSD1306 &display) override {
    // Draw background
    display.fillRect(x, y, width, height, bgColor);
    // Draw progress bar
    int16_t barWidth = (width * progress) / 100;
    display.fillRect(x, y, barWidth, height, barColor);
  }

  void setProgress(uint8_t value) {
    progress = constrain(value, 0, 100);
  }
};

#endif

/*
#include "UIProgressBar.h"

UIProgressBar* progressBar = new UIProgressBar(10, 50, 100, 10);

void setup() {
  // ... existing setup code ...
  ui.addComponent(progressBar);
}

void loop() {
  // Simulate progress
  for (uint8_t i = 0; i <= 100; i++) {
    progressBar->setProgress(i);
    ui.render();
    delay(50);
  }
}

*/