#ifndef UITEXTINPUT_H
#define UITEXTINPUT_H

#include "UIComponent.h"

/**
  void onTextInputChange(String newText) {
  Serial.print("Input text: ");
  Serial.println(newText);
}

UITextInput* nameInput = new UITextInput(0, 56, 1, WHITE, onTextInputChange);
ui.addComponent(nameInput);
*/

class UITextInput : public Component {
public:
  String text;
  int16_t x, y;
  uint8_t textSize;
  uint16_t textColor;
  void (*onTextChange)(String);

  UITextInput(int16_t xPos, int16_t yPos, uint8_t size = 1, uint16_t color = WHITE, void (*textChangeHandler)(String) = nullptr)
    : text(""), x(xPos), y(yPos), textSize(size), textColor(color), onTextChange(textChangeHandler) {}

  void render(Adafruit_SSD1306 &display) override {
    display.setTextSize(textSize);
    display.setTextColor(textColor);
    display.setCursor(x, y);
    display.println(text);
    display.drawLine(x + text.length() * 6 * textSize, y, x + text.length() * 6 * textSize, y + 8 * textSize, textColor);
  }

  void appendChar(char c) {
    text += c;
    if (onTextChange) onTextChange(text);
  }

  void backspace() {
    if (text.length() > 0) {
      text.remove(text.length() - 1);
      if (onTextChange) onTextChange(text);
    }
  }
};

#endif
/*
void onTextInputChange(String newText) {
  Serial.print("Input text: ");
  Serial.println(newText);
}

UITextInput* nameInput = new UITextInput(0, 56, 1, WHITE, onTextInputChange);
ui.addComponent(nameInput);

*/