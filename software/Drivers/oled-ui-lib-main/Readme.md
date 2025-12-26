# Arduino Oled UI Components 
A component-based UI library for SSD1306 oled screens, tested on 128x64 version.

## Prerequisites
- **Adafruit SSD1306 Library**: Install via Arduino Library Manager.

## Usage
Include relevant headrs in your sketch.
```cpp
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "UIRenderer.h"
#include "UILabel.h"
#include "UIButton.h"
// Include other components as needed
```

### Basic Example
This example displayes a label and a button. See examples/ folder for further usage examples.
```cpp
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "UIRenderer.h"
#include "UILabel.h"
#include "UIButton.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Instantiate the UI renderer
UIRenderer ui(display);

// Global variables
UILabel* helloLabel;
UIButton* clickButton;

void onButtonClick() {
  Serial.println("Button clicked!");
}

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  // Create UI components
  helloLabel = new UILabel("Hello, World!", 0, 0, 2);
  clickButton = new UIButton("Click Me", 0, 30, 80, 16, WHITE, BLACK, onButtonClick);

  // Add components to UI renderer
  ui.addComponent(helloLabel);
  ui.addComponent(clickButton);

  // Initial render
  ui.render();
}

void loop() {
  // Simulate button press (for demonstration purposes)
  delay(2000);
  onButtonClick();
}
```

### Components Reference

#### UILabel

Displays text on the screen.

**Usage Example**:

```
UILabel* myLabel = new UILabel("Sample Text", x, y, textSize, textColor);
ui.addComponent(myLabel);
```

- **Parameters**:
  - `text`: The text to display.
  - `x`, `y`: Position on the screen.
  - `textSize`: Size of the text (default is `1`).
  - `textColor`: Color of the text (default is `WHITE`).

#### UIButton

A clickable button with a label.

**Usage Example**:

```
void onButtonClick() {
  // Handle button click
}

UIButton* myButton = new UIButton("Press", x, y, width, height, bgColor, textColor, onButtonClick);
ui.addComponent(myButton);
```

- **Parameters**:
  - `label`: Button label text.
  - `x`, `y`: Position on the screen.
  - `width`, `height`: Size of the button.
  - `bgColor`: Background color (default is `WHITE`).
  - `textColor`: Text color (default is `BLACK`).
  - `onClick`: Callback function when the button is clicked.

#### UISeparator

Draws a horizontal line to separate UI sections.

**Usage Example**:

```
UISeparator* separator = new UISeparator(x, y, width, color);
ui.addComponent(separator);
```

- **Parameters**:
  - `x`, `y`: Starting position.
  - `width`: Length of the separator.
  - `color`: Line color (default is `WHITE`).

#### UISpinner

An animated spinner to indicate processing.

**Usage Example**:

```
UISpinner* spinner = new UISpinner(centerX, centerY, radius, color);
ui.addComponent(spinner);
```

- **Parameters**:
  - `centerX`, `centerY`: Center position.
  - `radius`: Size of the spinner.
  - `color`: Spinner color (default is `WHITE`).

**Note**: Call `spinner->update()` in the loop to animate.

#### UINavbar

A navigation bar with menu items.

**Usage Example**:

```
void onHomeSelected() {
  // Handle home selection
}

UINavbar* navbar = new UINavbar(x, y, width, height);
navbar->addItem(new UINavbarItem("Home", onHomeSelected));
ui.addComponent(navbar);
```

- **Parameters**:
  - `x`, `y`: Position on the screen.
  - `width`, `height`: Size of the navbar.
  - `bgColor`, `textColor`, `selectedColor`: Colors for various states.

#### UIProgressBar

Displays progress for operations.

**Usage Example**:

```
UIProgressBar* progressBar = new UIProgressBar(x, y, width, height);
progressBar->setProgress(50); // Set progress to 50%
ui.addComponent(progressBar);
```

- **Parameters**:
  - `x`, `y`: Position on the screen.
  - `width`, `height`: Size of the progress bar.
  - `bgColor`, `barColor`: Colors for background and progress.

#### UIRadioButton

Allows selection from multiple options.

**Usage Example**:

```
void onRadioSelection(uint8_t index) {
  // Handle selection
}

UIRadioButton* radioGroup = new UIRadioButton(x, y, textSize, textColor, onRadioSelection);
radioGroup->addOption(new UIRadioOption("Option 1"));
radioGroup->addOption(new UIRadioOption("Option 2"));
ui.addComponent(radioGroup);
```

- **Parameters**:
  - `x`, `y`: Position on the screen.
  - `textSize`: Text size (default is `1`).
  - `textColor`: Text color (default is `WHITE`).
  - `onSelectionChange`: Callback function when the selection changes.

#### UIToggleSwitch

A binary on/off switch.

**Usage Example**:

```
void onToggle(bool state) {
  // Handle toggle
}

UIToggleSwitch* toggleSwitch = new UIToggleSwitch(x, y, width, height, initialState, onColor, offColor, onToggle);
ui.addComponent(toggleSwitch);
```

- **Parameters**:
  - `x`, `y`: Position on the screen.
  - `width`, `height`: Size of the switch.
  - `initialState`: Starting state (`true` for on, `false` for off).
  - `onColor`, `offColor`: Colors for the on and off states.
  - `onToggle`: Callback function when the state changes.

#### UINumericInput

Adjust numerical values.

**Usage Example**:

```
void onValueChange(int32_t newValue) {
  // Handle value change
}

UINumericInput* numericInput = new UINumericInput(x, y, minValue, maxValue, initialValue, textSize, textColor, onValueChange);
ui.addComponent(numericInput);
```

- **Parameters**:
  - `x`, `y`: Position on the screen.
  - `minValue`, `maxValue`: Minimum and maximum values.
  - `initialValue`: Starting value.
  - `textSize`: Text size (default is `1`).
  - `textColor`: Text color (default is `WHITE`).
  - `onValueChange`: Callback function when the value changes.

#### UIMessageBox

Displays messages or alerts.

**Usage Example**:

```
UIMessageBox* messageBox = new UIMessageBox("Alert!", x, y, width, height);
messageBox->show(); // To display
messageBox->hide(); // To hide
ui.addComponent(messageBox);
```

- **Parameters**:
  - `message`: The message to display.
  - `x`, `y`: Position on the screen.
  - `width`, `height`: Size of the message box.
  - `bgColor`, `textColor`, `borderColor`: Colors for background, text, and border.

#### UILineChart

Visualizes data trends.

**Usage Example**:

```
UILineChart* lineChart = new UILineChart(x, y, width, height);
lineChart->addDataPoint(sensorValue);
ui.addComponent(lineChart);
```

- **Parameters**:
  - `x`, `y`: Position on the screen.
  - `width`, `height`: Size of the chart.
  - `lineColor`: Color of the line (default is `WHITE`).

#### UIDropdownMenu

Selects from a list of options.

**Usage Example**:

```
void onDropdownSelect(uint8_t index) {
  // Handle selection
}

UIDropdownMenu* dropdownMenu = new UIDropdownMenu(x, y, width, height, bgColor, textColor, selectedColor, onDropdownSelect);
dropdownMenu->addItem("Option A");
dropdownMenu->addItem("Option B");
ui.addComponent(dropdownMenu);
```

- **Parameters**:
  - `x`, `y`: Position on the screen.
  - `width`, `height`: Size of the dropdown menu.
  - `bgColor`, `textColor`, `selectedColor`: Colors for background, text, and selected item.
  - `onItemSelect`: Callback function when an item is selected.
