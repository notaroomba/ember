#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include "UIRenderer.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UISlider.h"
#include "UIList.h"
#include "UICheckbox.h"
#include "UITextInput.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

UIRenderer ui(display);

int counter = 0;
UILabel* counterLabel;
UISlider* brightnessSlider;
UICheckbox* agreeCheckbox;
UITextInput* nameInput;

void incrementCounter() {
  counter++;
  counterLabel->text = "Count: " + String(counter);
  ui.render();
}

void onBrightnessChange(int16_t newValue) {
  Serial.print("Brightness: ");
  Serial.println(newValue);
}

void onCheckboxToggle(bool isChecked) {
  Serial.print("Checkbox is ");
  Serial.println(isChecked ? "checked" : "unchecked");
}

void onTextInputChange(String newText) {
  Serial.print("Name input: ");
  Serial.println(newText);
}

void setup() {
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  UILabel* titleLabel = new UILabel("My App", 0, 0, 2);
  counterLabel = new UILabel("Count: 0", 0, 16, 1);

  UIButton* incrementButton = new UIButton("Increment", 0, 50, 60, 14, WHITE, BLACK, incrementCounter);

  brightnessSlider = new UISlider(0, 32, 100, 10, 0, 100, 50, WHITE, BLACK, onBrightnessChange);

  agreeCheckbox = new UICheckbox("I agree", 0, 44, false, WHITE, BLACK, WHITE, onCheckboxToggle);

  nameInput = new UITextInput(0, 56, 1, WHITE, onTextInputChange);

  ui.addComponent(titleLabel);
  ui.addComponent(counterLabel);
  ui.addComponent(brightnessSlider);
  ui.addComponent(agreeCheckbox);
  ui.addComponent(nameInput);
  ui.addComponent(incrementButton);

  ui.render();
}

void loop() {
  // Simulated interactions for demonstration purposes
  delay(3000);
  incrementCounter();
  brightnessSlider->increase();
  ui.render();

  agreeCheckbox->toggle();
  ui.render();

  nameInput->appendChar('A');
  ui.render();
}
