#ifndef UICOMPONENT_H
#define UICOMPONENT_H

#include <Adafruit_SSD1306.h>

class Component {
public:
  virtual void render(Adafruit_SSD1306 &display) = 0;
  virtual void update() {};
};

#endif
