#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include "UIRenderer.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UISeparator.h"
#include "UISpinner.h"
#include "UINavbar.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

UIRenderer ui(display);

// Global variables
UISpinner* spinner;
UINavbar* navbar;

void homeSelected() {
  Serial.println("Home selected");
}

void settingsSelected() {
  Serial.println("Settings selected");
}

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Create UI components
  UILabel* titleLabel = new UILabel("My App", 0, 16, 2);
  spinner = new UISpinner(SCREEN_WIDTH / 1.5, SCREEN_HEIGHT / 1.5, 10);

  // Navbar
  navbar = new UINavbar(0, 0, SCREEN_WIDTH, 12);
  navbar->addItem(new UINavbarItem("Home", homeSelected));
  navbar->addItem(new UINavbarItem("Settings", settingsSelected));

  // Separator
  UISeparator* separator = new UISeparator(0, 14, SCREEN_WIDTH);

  // Add components to UI renderer
  ui.addComponent(navbar);
  ui.addComponent(separator);
  ui.addComponent(titleLabel);
  ui.addComponent(spinner);

  // Initial render
  ui.render();
}

void loop() {
  // Update spinner animation
  spinner->update();

  // Render UI
  ui.render();

  // Simulate navbar navigation (for demonstration purposes)
  delay(2000);
  navbar->nextItem();
  ui.render();

  // Simulate item selection
  delay(2000);
  navbar->selectItem();
}
