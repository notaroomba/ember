#ifndef UIMESSAGEBOX_H
#define UIMESSAGEBOX_H

#include "UIComponent.h"

class UIMessageBox : public Component {
public:
  String message;
  int16_t x, y, width, height;
  uint16_t bgColor, textColor, borderColor;
  bool visible;

  UIMessageBox(String msg, int16_t xPos, int16_t yPos, int16_t w, int16_t h,
               uint16_t bg = BLACK, uint16_t textC = WHITE, uint16_t borderC = WHITE)
    : message(msg), x(xPos), y(yPos), width(w), height(h),
      bgColor(bg), textColor(textC), borderColor(borderC), visible(false) {}

  void render(Adafruit_SSD1306 &display) override {
    if (visible) {
      // Draw background
      display.fillRect(x, y, width, height, bgColor);
      // Draw border
      display.drawRect(x, y, width, height, borderColor);
      // Draw message
      display.setTextSize(1);
      display.setTextColor(textColor);
      display.setCursor(x + 5, y + (height - 8) / 2);
      display.println(message);
    }
  }

  void show() {
    visible = true;
  }

  void hide() {
    visible = false;
  }
};

#endif
/*
#include "UIMessageBox.h"

UIMessageBox* messageBox = new UIMessageBox("Alert!", 20, 20, 88, 24);

void setup() {
  // ... existing setup code ...
  ui.addComponent(messageBox);
}

void loop() {
  // Show message box after 3 seconds
  delay(3000);
  messageBox->show();
  ui.render();
  // Hide message box after another 3 seconds
  delay(3000);
  messageBox->hide();
  ui.render();
}

*/