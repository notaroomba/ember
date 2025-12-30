#ifndef UILINECHART_H
#define UILINECHART_H

#include "UIComponent.h"
#define MAX_DATA_POINTS 100

class UILineChart : public Component {
public:
  int16_t x, y, width, height;
  uint16_t data[MAX_DATA_POINTS];
  uint8_t dataCount;
  uint16_t lineColor;

  UILineChart(int16_t xPos, int16_t yPos, int16_t w, int16_t h, uint16_t color = WHITE)
    : x(xPos), y(yPos), width(w), height(h), dataCount(0), lineColor(color) {}

  void render(Adafruit_SSD1306 &display) override {
    // Draw axes
    display.drawRect(x, y, width, height, WHITE);
    // Plot data
    if (dataCount > 1) {
      for (uint8_t i = 0; i < dataCount - 1; i++) {
        int16_t x0 = x + (i * (width - 2)) / (MAX_DATA_POINTS - 1) + 1;
        int16_t y0 = y + height - 1 - (data[i] * (height - 2)) / 100;
        int16_t x1 = x + ((i + 1) * (width - 2)) / (MAX_DATA_POINTS - 1) + 1;
        int16_t y1 = y + height - 1 - (data[i + 1] * (height - 2)) / 100;
        display.drawLine(x0, y0, x1, y1, lineColor);
      }
    }
  }

  void addDataPoint(uint16_t value) {
    if (dataCount < MAX_DATA_POINTS) {
      data[dataCount++] = constrain(value, 0, 100);
    } else {
      // Shift data left and add new value at the end
      memmove(data, data + 1, (MAX_DATA_POINTS - 1) * sizeof(uint16_t));
      data[MAX_DATA_POINTS - 1] = constrain(value, 0, 100);
    }
  }
};

#endif
/*
#include "UILineChart.h"

UILineChart* lineChart = new UILineChart(0, 20, SCREEN_WIDTH, 40);

void setup() {
  // ... existing setup code ...
  ui.addComponent(lineChart);
}

void loop() {
  // Simulate data
  uint16_t sensorValue = analogRead(A0) / 10;
  lineChart->addDataPoint(sensorValue);
  ui.render();
  delay(500);
}

*/