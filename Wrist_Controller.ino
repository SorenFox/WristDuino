#include <SPI.h>
#include <Wire.h>
#include <Servo.h>
#include <string.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define WINDOW_RADIUS 4 // Radius of displayed windows' corners
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char string0[] PROGMEM = "";

const char main1[] PROGMEM = "run";
const char main2[] PROGMEM = "settings";
const char main3[] PROGMEM = "about";
const char main4[] PROGMEM = "power off";

const char run1[] PROGMEM = "2servo";
const char run2[] PROGMEM = "randServo";
const char run3[] PROGMEM = "stopwatch";
const char run4[] PROGMEM = "calculator";

const char set1[] PROGMEM = "deadzone";
const char set2[] PROGMEM = "calibrate";
const char set3[] PROGMEM = "-";
const char set4[] PROGMEM = "-";

const char *const main[] PROGMEM = {string0,main1,main2,main3,main4,string0};
const char *const runProgs[] PROGMEM = {string0,run1,run2,run3,run4,string0};
const char *const settings[] PROGMEM = {string0,set1,set2,set3,set4,string0};

int leftPot = 0, rightPot = 1, buttonPin = 2;
int val, maxVal, minVal, maxSel, minSel, choice;
bool deadzone = true;
char buffer[6][10];

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    for(;;); // Don't proceed, loop forever
  }
  calibrate();
}

void calibrate() {

  display.clearDisplay();
  drawWindow(16,16,96,48,F("Calibration"),F("Move sliders to their maximum"));
  display.display();
  delay(2000);
  maxVal = analogRead(leftPot);
  maxSel = analogRead(rightPot);
  display.clearDisplay();
  drawWindow(16,16,96,48,F("Calibration"),F("Move sliders to their minimum"));
  display.display();
  delay(2000);
  minVal = analogRead(leftPot);
  minSel = analogRead(rightPot);
}

int drawWindow(int x, int y, int sizex, int sizey, String title, String dialog) {
  int cursorY, tempSize = sizex/6 - 1;
  char temp[tempSize + 1];
  
  display.drawRoundRect(x, y, sizex, sizey, WINDOW_RADIUS, WHITE);
  display.setCursor(x+4,y+4);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(title);
  display.drawLine(x,y+14,x+sizex-1,y+14,WHITE);
  
  display.setTextSize(1);
  cursorY = y+16;
  for (int i = 0; i < ceil((float)dialog.length()/(float)tempSize); i++) {
    display.setCursor(x+4,cursorY);
    temp[tempSize] = '\n';
    for (int j = i * tempSize; j < (i + 1) * tempSize; j++) {
      if (j < dialog.length()) {
        temp[j % tempSize] = dialog[j];
      } else {
        temp[j % tempSize] = ' ';
      }
    }
    display.print(temp);
    cursorY += 8;
  }

  return 0;
}

int drawSelection(char above[10], char middle[10], char below[10]) {
  display.setCursor(64 - strlen(above)*3, 8);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(above);

  display.drawLine(16,20,112,20,WHITE);

  display.setCursor(64 - strlen(middle)*6, 24);
  display.setTextSize(2);
  display.println(middle);

  display.drawLine(16,42,112,42,WHITE);

  display.setCursor(64 - strlen(below)*3, 46);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(below);
}

bool getDialog(char text1[10], int x1, int y1, char text2[10], int x2, int y2) {
  bool selection, button, reset = false;
  int val, sel;

  display.setTextSize(1);
  
  do {
    button = digitalRead(buttonPin);
    val = analogRead(leftPot);
    sel = analogRead(rightPot);
    val = map(val, minVal, maxVal, 0, 63);
    sel = map(sel, minSel, maxSel, 0, 63);

    if (sel < 50) {
      reset = true;
    }
    
    if (val <= 31) {
      display.setCursor(x1,y1);
      display.setTextColor(BLACK,WHITE);
      display.print(text1);
      display.setCursor(x2,y2);
      display.setTextColor(WHITE,BLACK);
      display.print(text2);
      selection = false;
    } else {
      display.setCursor(x1,y1);
      display.setTextColor(WHITE,BLACK);
      display.print(text1);
      display.setCursor(x2,y2);
      display.setTextColor(BLACK,WHITE);
      display.print(text2);
      selection = true;
    }

    display.drawLine(127,0,127,sel,WHITE);
    display.drawLine(127,63,127,sel,BLACK);
    display.display();
  } while ((sel < 60) || !reset);
  return selection;
}

int getSelection(char items[6][10]) {
  bool button, reset = false;
  int val, sel;

  do {
    button = digitalRead(buttonPin);
    val = analogRead(leftPot);
    sel = analogRead(rightPot);
    val = constrain(map(val, minVal, maxVal, 1, 4), 1, 4);
    sel = map(sel, minSel, maxSel, 0, 63);

    if ((sel < 50) && button) {
      reset = true; // make sure we aren't flicking to next menu
    }
    
    display.clearDisplay();
    drawSelection(items[val-1], items[val], items[val+1]);
    display.drawLine(0,0,0,val*15+3,WHITE);
    display.drawLine(127,0,127,sel,WHITE);
    display.display();
  } while ((sel < 60) && button || !reset);
  
  if (!button) {
    val = 0;
  }

  return val;
}

void drawNums() {
  display.setCursor(64,40);
  display.setTextSize(1);
  display.print(F("7 8 9 D"));
  display.setCursor(64,48);
  display.print(F("4 5 6 ."));
  display.setCursor(64,56);
  display.print(F("1 2 3 0"));
}

void drawOperators() {
  display.setCursor(64,40);
  display.setTextSize(1);
  display.print(F("+ -"));
  display.setCursor(64,48);
  display.print(F("* /"));
  display.setCursor(64,56);
  display.print(F("^ l"));
}

char getNum() {
  int val, sel, prevX = 63, prevY = 39;
  int cycles = 0, result;
  bool button, reset = false;

  do {
    button = digitalRead(buttonPin);
    val = analogRead(leftPot);
    sel = analogRead(rightPot);
    val = constrain(map(val, minVal, maxVal, 0, 3), 0, 3);
    sel = constrain(map(sel, minSel, maxSel, 0, 2), 0, 2);

    if (button) {
      reset = true;
    }

    if ((prevX != val*12 + 63) || (prevY != sel*8 + 39)) {
      display.drawRect(prevX, prevY, 10, 8, BLACK); // blank off the last drawn box
    }

    prevX = val*12 + 63;
    prevY = sel*8 + 39;
    display.drawRect(prevX, prevY, 10, 8, WHITE); // draw the new box
    display.setTextColor(WHITE);
    drawNums();

    display.display();
  } while (button || !reset);

  while (!button && cycles < 30000) { // set delay for button held, signal end of number
    button = digitalRead(buttonPin);
    cycles++;
  }

  if (cycles < 30000) {
    if (val < 3) {
      result = 55 - sel*3 + val; // 30 + number logic (for ASCII char)
    } else if (sel == 2) {
      result = 48;
    } else if (sel == 1) {
      result = '.';
    } else {
      result = '<';
    }
  } else {
    result = '\0'; // indicate the end of a number
  }

  display.drawRect(prevX, prevY, 10, 8, BLACK); // blank off the last drawn box
  display.setTextColor(BLACK); // clean up keypad
  drawNums();

  return result;
}

char getOperation() {
  int val, sel, prevX = 63, prevY = 39;
  int cycles = 0, result;
  bool button, reset = false;

  do {
    button = digitalRead(buttonPin);
    val = analogRead(leftPot);
    sel = analogRead(rightPot);
    val = constrain(map(val, minVal, maxVal, 0, 1), 0, 1);
    sel = constrain(map(sel, minSel, maxSel, 0, 2), 0, 2);

    if (button) {
      reset = true;
    }

    if ((prevX != val*12 + 63) || (prevY != sel*8 + 39)) {
      display.drawRect(prevX, prevY, 10, 8, BLACK);
    }

    prevX = val*12 + 63;
    prevY = sel*8 + 39;
    display.drawRect(prevX, prevY, 10, 8, WHITE);
    drawOperators();

    display.display();
  } while (button || !reset);

  while (!button && cycles < 30000) { // set delay for button held, signal end of number
    button = digitalRead(buttonPin);
    cycles++;
  }

  if (cycles < 30000) {
    if (val == 0) {
      if (sel == 0) {
        result = '+';
      } else if (sel == 1) {
        result = '*';
      } else {
        result = '^';
      }
    } else {
      if (sel == 0) {
        result = '-';
      } else if (sel == 1) {
        result = '/';
      } else {
        result = 'l';
      }
    }
  } else {
    result = 'e'; // indicate program exit 
  }

  display.drawRect(prevX, prevY, 10, 8, BLACK); // blank off the last drawn box
  display.setTextColor(BLACK); // clean up keypad
  drawOperators();

  return result;
}

double parseDouble(char inChars[10]) { // parse a string of chars to a double
  int power = 1;
  double result = 0.0;

  for (int i = 9; i >=0; i--) {
    if (inChars[i] == '.') {
      result /= power;
      power = 1;
    } else if (inChars[i] != '\0') {
      result += power * (double)(inChars[i] - 48);
      power *= 10;
    }
  }

  return result;
}

double calculate(double num1, double num2, char operation) {
  double result = 1;

  if (operation == '+') {
    result = num1 + num2;
  } else if (operation == '-') {
    result = num1 - num2;
  } else if (operation == '*') {
    result = num1 * num2;
  } else if (operation == '/') {
    result = num1 / num2;
  } else if (operation == '^') {
    result = pow(num1,num2);
  } else if (operation == 'l') {
    result = log10(num1) / log10(num2);
  }

  return result;
}

//---[programs]---------------------------------------------------

void servo2(bool deadzone) {
  Servo left, right;
  int val, sel, leftServo = 0, rightServo = 0, deadCycle;
  bool button;
  
  display.clearDisplay();
  display.display();

  do {
    button = digitalRead(buttonPin);
    val = analogRead(leftPot);
    sel = analogRead(rightPot);
    val = constrain(map(val, minVal, maxVal, 80, 180), 80, 180);
    sel = constrain(map(sel, minSel, maxSel, 80, 180), 80, 180);

    if (deadzone) {
      left.detach();
      right.detach();
    } else {
      deadCycle = 0;
    }
    
    if ((abs(leftServo - val) > 30) || (abs(rightServo - sel) > 30)) {
      deadCycle = 0;
      Serial.println("attached");
      left.attach(9);
      right.attach(10);
    }
    
    while ((deadCycle < 5000) && button) {
      button = digitalRead(buttonPin);
      val = analogRead(leftPot);
      sel = analogRead(rightPot);
      val = constrain(map(val, minVal, maxVal, 80, 180), 80, 180);
      sel = constrain(map(sel, minSel, maxSel, 80, 180), 80, 180);

      if ((abs(leftServo - val) > 4) || (abs(rightServo - sel) > 4)) {
        deadCycle = 0;
        leftServo = val;
        rightServo = sel;
      } else {
        deadCycle++;
      }

      left.write(val);
      right.write(sel);
    }

  } while (button);

  left.detach(); // makes sure servos detach when finished.
  right.detach();
}

void calculator() {
  double num1 = 0, num2 = 0, result = 0;
  char operation = '+';
  char sequence[10];
  int cycles = 0, val, sel, length = 0, power;
  bool button;
  char inputMode = 'a'; // a = input num1, b = input num2, c = input operation, d = answer input mode
  char operationMode = 'a'; // a = inputting, b = exiting

  display.clearDisplay();

  do { // loop over the input modes
    button = digitalRead(buttonPin);

    for (int i = 0; i < 10; i++) {
      sequence[i] = '\0';
    }

    do {
      if (inputMode == 'a' || inputMode == 'b') { // check that we're inputting a number
        sequence[length] = getNum();
        if (sequence[length] == '<') {
          sequence[length] = '\0';
          sequence[length - 1] = '\0';
          length--;
        } else if (sequence[length] == '\0') {
          if (inputMode == 'a') {
            for (int i = 0; i < 10; i++) { // clear sequence for input num2
              sequence[i] = '\0';
            }
            length = 0;
            operationMode = 'b'; // kick out of the number input loop
          } else if (inputMode == 'b') {
            length = 0;
            operationMode = 'b';
          }
        } else {
          length++;
        }
      }

      if (inputMode == 'a') {
        num1 = parseDouble(sequence);
        display.setCursor(0,0);
        display.print("->");
        display.setCursor(16,0);
      } else if (inputMode == 'b') {
        num2 = parseDouble(sequence);
        display.setCursor(0,10);
        display.print("->");
        display.setCursor(16,10);
      }
      
      display.print(sequence);
      display.display();
    } while (operationMode == 'a' && length < 10); // exit number input after user holds button

    if (inputMode == 'a') {
      String(num1,10).toCharArray(sequence, 10); // read the existing num1 into sequence
      
      inputMode = 'b';
      operationMode = 'a';
    } else if (inputMode == 'b') {
      String(num2,10).toCharArray(sequence, 10); // read the existing num2 into sequence
      
      inputMode = 'c';
      operation = getOperation();
      operationMode = 'a';
    } else if (inputMode == 'c') {
      result = calculate(num1,num2,operation); // calculate the actual result
      display.setCursor(0,20);
      display.print(result);
    }

  } while (operation != 'e');

}

void loop() {
  
  do {
    for (int i = 0; i < 6; i++) {
      strcpy_P(buffer[i], (char *)pgm_read_word(&(main[i])));
    }
    choice = getSelection(buffer);
    
    switch (choice) {
      case 1:
        for (int i=0; i < 6; i++) {
          strcpy_P(buffer[i], (char *)pgm_read_word(&(runProgs[i])));
        }
        choice = getSelection(buffer);
        switch (choice) {
          case 1:
            servo2(deadzone);
            break;

          case 4:
            calculator();
            break;

          default:
            break;
        }
        break;
        
      case 2:
        for (int i=0; i < 6; i++) {
          strcpy_P(buffer[i], (char *)pgm_read_word(&(settings[i])));
        }
        choice = getSelection(buffer);

        switch (choice) {
          case 1:
            display.clearDisplay();
            drawWindow(16,4,96,56,"Settings","Enable servo deadzone?");
            deadzone = getDialog("disable",18,40,"enable",64,40);
            break;

          case 2:
            calibrate();
            break;

          default:
            break;
        }
        break;
      case 3:
        display.clearDisplay();
        drawWindow(8,8,112,48,F("About"),F("V0.2   Developed by Joseph Loveday"));
        display.display();
        delay(3000);
        break;
        
      case 4:
        break;

      default:
        calibrate();
        break;
    }
  } while (choice != 4);
}
