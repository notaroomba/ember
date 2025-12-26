#ifndef UIRENDERER_H
#define UIRENDERER_H

#include <Adafruit_SSD1306.h>
#include "UIComponent.h"

#define MAX_COMPONENTS 10

class UIRenderer {
public:
  Component* components[MAX_COMPONENTS];
  uint8_t componentCount = 0;
  Adafruit_SSD1306 &display;

  UIRenderer(Adafruit_SSD1306 &disp) : display(disp) {}

  void addComponent(Component* component) {
    if (componentCount < MAX_COMPONENTS) {
      components[componentCount++] = component;
    }
  }

  void render() {
    display.clearDisplay();
    for (uint8_t i = 0; i < componentCount; i++) {
      components[i]->render(display);
    }
    display.display();
  }
};

#endif
