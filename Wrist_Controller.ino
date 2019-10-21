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
const char set3[] PROGMEM = "three";
const char set4[] PROGMEM = "four";

const char dialog0[] PROGMEM = "Maximum";
const char dialog1[] PROGMEM = "Minimum";
const char dialog2[] PROGMEM = "V1.0\nDeveloped by Joseph Loveday";


const char *const main[] PROGMEM = {string0,main1,main2,main3,main4,string0};
const char *const runProgs[] PROGMEM = {string0,run1,run2,run3,run4,string0};
const char *const settings[] PROGMEM = {string0,set1,set2,set3,set4,string0};
const char *const dialog[] PROGMEM = {dialog0,dialog1,dialog2};

int leftPot = 0, rightPot = 1, buttonPin = 2;
int val, maxVal, minVal, maxSel, minSel, choice;
bool deadzone = true;
char buffer[6][10];
char dialogBuffer[24];

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
  strcpy(dialogBuffer, (char *)pgm_read_word(&(dialog[0])));
  drawWindow(16,16,96,48,"Calibration",dialogBuffer);
  display.display();
  delay(2000);
  maxVal = analogRead(leftPot);
  maxSel = analogRead(rightPot);
  display.clearDisplay();
  strcpy(dialogBuffer, (char *)pgm_read_word(&(dialog[1])));
  drawWindow(16,16,96,48,"Calibration",dialogBuffer);
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
  display.setCursor(56,32);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("7 8 9 + -");
  display.setCursor(56,40);
  display.print("4 5 6 * / ");
  display.setCursor(56,48);
  display.print("1 2 3 0 . =");
}

char getNum() {
  int val, sel, prevX = 63, prevY = 31;
  int result;
  bool button, reset = false;

  do {
    button = digitalRead(buttonPin);
    val = analogRead(leftPot);
    sel = analogRead(rightPot);
    val = constrain(map(val, minVal, maxVal, 0, 5), 0, 5);
    sel = constrain(map(sel, minSel, maxSel, 0, 2), 0, 2);

    if (button) {
      reset = true;
    }

    if ((prevX != val*12 + 55) || (prevY != sel*8 + 31)) {
      display.drawRect(prevX, prevY, 10, 8, BLACK);
    }

    prevX = val*12 + 55;
    prevY = sel*8 + 31;
    display.drawRect(prevX, prevY, 10, 8, WHITE);
    drawNums();

    display.display();
  } while (button || !reset);

  if (val < 3) {
    result = 55 - sel*3 + val; // 30 + number logic (for ASCII char)
  } else if ((val == 3) && (sel == 2)) {
    result = 48;
  } else {
    result = '.';
  }

  return result;
}

double parseDouble(char inChars[10]) {
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
  double num1 = 0, num2 = 0;
  char sequence[10];
  int cycles = 0, val, sel, length = 0, power;
  bool button;

  for (int i = 0; i < 10; i++) {
    sequence[i] = '\0';
  }

  do {
    button = digitalRead(buttonPin);
    
    if (button) {
      cycles = 0;
    } else {
      cycles++;
    }

    display.clearDisplay();

    do {
      sequence[length] = getNum();
      length++;

      display.clearDisplay();
      display.setCursor(0,0);
      display.print(parseDouble(sequence));
      display.display();
    } while (length < 6);

  } while (cycles < 20);

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
        for (int i = 0; i < 3; i++) {
          strcpy(buffer[i],(char *)pgm_read_word(&(dialog[i])));
        }
        display.clearDisplay();
        drawWindow(8,8,112,48,"About","V1.0   Developed by Joseph Loveday");
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
