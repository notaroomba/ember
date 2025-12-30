#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include "UIRenderer.h"
#include "UILabel.h"
#include "UIButton.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Instantiate the UI renderer
UIRenderer ui(display);

// Global variables
int counter = 0;
UILabel* counterLabel;

// Button click handler
void incrementCounter() {
  counter++;
  counterLabel->text = "Count: " + String(counter);
  ui.render();
}

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Create UI components
  UILabel* titleLabel = new UILabel("CounterApp", 0, 0, 2);
  counterLabel = new UILabel("Count: 0", 0, 20, 1);

  UIButton* incrementButton = new UIButton("Increment", 0, 40, 80, 20, WHITE, BLACK, incrementCounter);

  // Add components to UI renderer
  ui.addComponent(titleLabel);
  ui.addComponent(counterLabel);
  ui.addComponent(incrementButton);

  // Initial render
  ui.render();
}

void loop() {
  // Simulate button press (for example purposes)
  delay(2000);
  incrementCounter();
}
