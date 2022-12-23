#include <LiquidCrystal.h>
#include "LedControl.h"
#include <EEPROM.h>

const byte RS = 9;
const byte enable = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
const byte contrast = 3;

LiquidCrystal lcd(RS, enable, d4, d5, d6, d7);

const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

int matrixBrightness = 2;
int lcdContrast = 10;
int lcdBrightness = 10;
int row = 0;
int col = 0;
byte ledState = true;

String mainMenuTitles[5] = {
  "Start Game",
  "High Score",
  "Settings",
  "About",
  "How to play"
};

String settings[] = {
  "Button for <-",
  "Start Level: ",
  "Start Segment: ",
  "LCD Contrast: ",
  "Matrix Brightness: "
};

String highScore[] = {
  "Button for <-",
  "1: ",
  "2: ",
  "3: ",
  "4: ",
  "5: ",
  "Reset Highscore"
};

String about = "Memory Pattern, Created by: Cosali Aleina, GitHub: CAleina        ";

String howToPlay[5] = {
  "<-",
  "Level 1: Recreate the shape shown      ",
  "Level 2: Recreate the shapes shown together     ",
  "Level 3: Recreate the shape with the dots in order      ",
  "Level 4: Recreate the shape given      "
};

const byte arrow[8] = {
  0b11000,
  0b11100,
  0b11110,
  0b11111,
  0b11111,
  0b11110,
  0b11100,
  0b11000
};

const byte arrowUP[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b01110,
  0b11111,
  0b00000,
  0b00000,
  0b00000
};

const byte arrowDown[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};

byte heart[] = {
  0b11111,
  0b11111,
  0b10101,
  0b00000,
  0b10001,
  0b11011,
  0b11111,
  0b11111
};


const int joyXPin = A0;
const int joyYPin = A1;
const int buttonPin = 2;
long stateChangeTime = 0;
int stateChangeDelay = 100;

const int joyMinThreshold = 400;
const int joyMaxThreshold = 600;

bool joyIsNeutral = true;

byte buttonState = LOW;
byte buttonReading = LOW;
byte buttonLastReading = LOW;

byte menuActive = 0;
int mainMenuSegment = 0;
int subMenuActive = 0;
int subMenuSegment = 0;
long menuInterval = 0;
int scrollSegment = 0;
int settingActive = 0;

byte gameActive = 0;
int gameMatrix[8][8];
long gameInterval = 0;
int score = 0;
long introTime = 0;
byte gameLevel = 4;
byte gameSegment = 2;
byte order = 1;
int gameDots = 0;
int currentGameDot = 0;
byte gameIntroActive = 1;
int levelPoints = 10;
int gameLives = 5;

long level2Time = 0;
long level3Time = 0;

long lastDebounceTime;
int debounceDelay = 100;

void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin, INPUT_PULLUP);

  analogWrite(contrast, 150);
  lcd.begin(16, 2);

  lcd.createChar(0, arrow);
  lcd.createChar(1, arrowUP);
  lcd.createChar(2, arrowDown);
  lcd.createChar(3, heart);

  lcd.setCursor(0, 0);

  lcd.print("Memory Pattern");

  lcd.setCursor(0, 1);

  lcd.write(byte(0));
  lcd.print("Press for Menu");

  gameLevel = EEPROM[0];
  gameSegment = EEPROM[1];
  lcdContrast = EEPROM[2];
  analogWrite(contrast, EEPROM[2]);
  matrixBrightness = EEPROM[3];
  lc.setIntensity(0, EEPROM[3]);

  if (gameLevel == 1) {
    levelPoints == 15;
  } else if (gameLevel == 2) {
    levelPoints == 20;
  } else if (gameLevel == 3) {
    levelPoints == 25;
  } else if (gameLevel == 4) {
    levelPoints == 2;
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println(settingActive);

  int joyX = analogRead(joyXPin);  //reading the joystick movements on the X and Y axes
  int joyY = analogRead(joyYPin);
  buttonReading = digitalRead(buttonPin);


  if (!gameActive) {
    if (menuActive) {                                                                                 //makes sure there is no recorded movement on the Y ax during state 2
      if (joyX < joyMinThreshold && joyIsNeutral && millis() - stateChangeTime > stateChangeDelay) {  //checking if the joystick moved left on the Y ax and checking not to take a double movement
        stateChangeTime = millis();
        lcd.clear();
        if (!subMenuActive) {
          mainMenuSegment++;
          mainMenu();
        } else if (settingActive) {
          if (subMenuSegment == 1) {
            gameLevel = gameLevel - 1;
          } else if (subMenuSegment == 2) {
            gameSegment = gameSegment - 1;
          } else if (subMenuSegment == 3) {
            lcdContrast = lcdContrast - 30;
          } else if (subMenuSegment == 4) {
            matrixBrightness = matrixBrightness - 3;
          }
        } else if (subMenuActive && (mainMenuSegment == 2 || mainMenuSegment == 4 || mainMenuSegment == 1)) {
          subMenuSegment++;
          scrollSegment = 0;
          if (mainMenuSegment == 2) {
            settingsMenu();
          } else if (mainMenuSegment == 4) {
            howToPlayMenu();
          } else {
            highScoreMenu();
          }
        }
        joyIsNeutral = false;
      }

      if (joyX > joyMaxThreshold && joyIsNeutral && millis() - stateChangeTime > stateChangeDelay) {  //checking if the joystick moved right on the Y ax and checking not to take a double movement
        stateChangeTime = millis();
        lcd.clear();
        if (!subMenuActive) {
          mainMenuSegment--;
          mainMenu();
        } else if (settingActive) {
          if (subMenuSegment == 1) {
            gameLevel = gameLevel + 1;
          } else if (subMenuSegment == 2) {
            gameSegment = gameSegment + 1;
          } else if (subMenuSegment == 3) {
            lcdContrast = lcdContrast + 30;
          } else if (subMenuSegment == 4) {
            matrixBrightness = matrixBrightness + 3;
          }
        } else if (subMenuActive && (mainMenuSegment == 2 || mainMenuSegment == 4 || mainMenuSegment == 1)) {
          subMenuSegment--;
          scrollSegment = 0;
          if (mainMenuSegment == 2) {
            settingsMenu();
          } else if (mainMenuSegment == 4) {
            howToPlayMenu();
          } else {
            highScoreMenu();
          }
        }
        joyIsNeutral = false;
      }

      if (joyY > joyMaxThreshold && joyIsNeutral && millis() - stateChangeTime > stateChangeDelay) {  //checking if the joystick moved upwards on the X ax and checking not to take a double movement
        stateChangeTime = millis();
        joyIsNeutral = false;
        if (mainMenuSegment == 2) {
          if (subMenuSegment != 0) {
            settingActive = 1;
            if (subMenuSegment == 4) {
              lc.shutdown(0, false);
              for (int i = 0; i < matrixSize; i++) {
                for (int j = 0; j < matrixSize; j++) {
                  lc.setLed(0, i, j, true);
                }
              }
            }
          }
        }
      }

      if (joyY < joyMinThreshold && joyIsNeutral && millis() - stateChangeTime > stateChangeDelay) {  //checking if the joystick moved downwards on the X ax and checking not to take a double movement
        stateChangeTime = millis();
        joyIsNeutral = false;
        if (settingActive == 1) {
          settingActive = 0;
          lcd.clear();
          if (subMenuSegment == 1) {
            EEPROM.update(0, gameLevel);
          } else if (subMenuSegment == 2) {
            EEPROM.update(1, gameSegment);
          } else if (subMenuSegment == 3) {
            EEPROM.update(2, lcdContrast);
          } else if (subMenuSegment == 4) {
            EEPROM.update(2, matrixBrightness);
            lc.clearDisplay(0);
            lc.shutdown(0, true);
          }
          subMenuSegment = 0;
          settingsMenu();
        }
      }

      if (joyY <= joyMaxThreshold && joyY >= joyMinThreshold && joyX <= joyMaxThreshold && joyX >= joyMinThreshold) {  //setting the joystick move back to neutral waiting for the next move
        joyIsNeutral = true;
      }
    }

    if (!settingActive) {
      if (buttonReading != buttonLastReading) {  //checking if the button state changed
        lastDebounceTime = millis();             //starting the timer for the debounce
      }

      if (millis() - lastDebounceTime > debounceDelay) {  //checking the state of the button again after debounce time passed
        if (buttonReading != buttonState) {               //if the previous button state is different than current button state, button state is changed to current
          buttonState = buttonReading;

          if (buttonState == HIGH) {  //state 2 is activated once button is back to original state
            if (menuActive == 0) {
              lc.shutdown(0, true);
              lc.clearDisplay(0);
              deleteGameMatrix();
              menuActive = 1;
              lcd.clear();
              mainMenu();
            } else if (menuActive == 1) {
              if (subMenuActive == 0) {
                lcd.clear();
                subMenuActive = 1;
                if (mainMenuSegment == 0) {
                  lc.shutdown(0, false);
                  lc.clearDisplay(0);
                }
              } else {
                if (mainMenuSegment == 1 && subMenuSegment == 6) {
                  for (int i = 4; i <= 8; i++) {
                    EEPROM.update(i, 0);
                  }
                  highScoreMenu();
                } else {
                  lcd.clear();
                  subMenuSegment = 0;
                  subMenuActive = 0;
                  scrollSegment = 0;
                  mainMenu();
                }
              }
            }
          }
        }
      }
    }

    if (millis() - menuInterval > 300 && subMenuActive) {
      menuInterval = millis();
      if (mainMenuSegment == 0) {
        gameActive = 1;
        menuActive = 0;
        deleteGameMatrix();
        subMenuActive = 0;
        subMenuSegment = 0;
        scrollSegment = 0;
        gameInterval = 0;
        score = 0;
        introTime = 0;
        gameLevel = EEPROM[0];
        gameSegment = EEPROM[1];
        order = 1;
        gameDots = 0;
        currentGameDot = 0;
        gameIntroActive = 1;
        gameLives = 5;
        if (gameLevel == 1) {
          levelPoints == 15;
        } else if (gameLevel == 2) {
          levelPoints == 20;
        } else if (gameLevel == 3) {
          levelPoints == 25;
        } else if (gameLevel == 4) {
          levelPoints == 2;
        }
      } else if (mainMenuSegment == 1) {
        highScoreMenu();
      } else if (mainMenuSegment == 2) {
        if (!settingActive) {
          settingsMenu();
        } else {
          if (subMenuSegment == 1) {
            setStartLevel();
          } else if (subMenuSegment == 2) {
            setGameSegment();
          } else if (subMenuSegment == 3) {
            setLCDContrast();
            analogWrite(contrast, lcdContrast);
          } else if (subMenuSegment == 4) {
            setMatrixBrightness();
            lc.setIntensity(0, matrixBrightness);
          }
        }
      } else if (mainMenuSegment == 3) {
        aboutMenu();
      } else if (mainMenuSegment == 4) {
        howToPlayMenu();
      }
    };
  }

  if (gameActive == 1) {

    if (!gameIntroActive && ((gameLevel == 2 && millis() - level2Time > 3000) || (gameLevel != 2 && millis() - introTime > 7000))) {
      if (subMenuSegment == 1) {
        lc.clearDisplay(0);
        subMenuSegment = 2;
      }
      if (joyY > joyMaxThreshold && millis() - stateChangeTime > stateChangeDelay) {
        stateChangeTime = millis();
        if (gameMatrix[row][col] != -1) {
          lc.setLed(0, row, col, 0);
        } else {
          lc.setLed(0, row, col, 1);
        }
        row--;
        if (row < 0) {
          row = matrixSize - 1;
        }
        lc.setLed(0, row, col, 1);
      }

      if (joyY < joyMinThreshold && millis() - stateChangeTime > stateChangeDelay) {
        stateChangeTime = millis();
        if (gameMatrix[row][col] != -1) {
          lc.setLed(0, row, col, 0);
        } else {
          lc.setLed(0, row, col, 1);
        }
        row++;
        if (row == matrixSize) {
          row = 0;
        }
        lc.setLed(0, row, col, 1);
      }

      if (joyX < joyMinThreshold && millis() - stateChangeTime > stateChangeDelay) {
        stateChangeTime = millis();
        if (gameMatrix[row][col] != -1) {
          lc.setLed(0, row, col, 0);
        } else {
          lc.setLed(0, row, col, 1);
        }
        col++;
        if (col == matrixSize) {
          col = 0;
        }
        lc.setLed(0, row, col, 1);
      }

      if (joyX > joyMaxThreshold && millis() - stateChangeTime > stateChangeDelay) {
        stateChangeTime = millis();
        if (gameMatrix[row][col] != -1) {
          lc.setLed(0, row, col, 0);
        } else {
          lc.setLed(0, row, col, 1);
        }
        col--;
        if (col < 0) {
          col = matrixSize - 1;
        }
        lc.setLed(0, row, col, 1);
      }

      if (millis() - stateChangeTime > stateChangeDelay) {
        stateChangeTime = millis();
        ledState = !ledState;
        lc.setLed(0, row, col, ledState);
      }

      if (buttonReading != buttonLastReading) {
        lastDebounceTime = millis();
      }

      if (millis() - lastDebounceTime > debounceDelay) {
        if (buttonReading != buttonState) {
          buttonState = buttonReading;

          if (buttonState == HIGH) {
            if (gameMatrix[row][col] == order) {
              score = score + levelPoints;
              lc.setLed(0, row, col, 1);
              gameMatrix[row][col] = -1;
              currentGameDot++;
              if (gameLevel == 3 || (gameLevel == 4 && gameSegment == 1)) {
                order++;
              }
              if (currentGameDot == gameDots) {
                gameIntroActive = 1;
                introTime = millis();
                deleteGameMatrix();
                lc.clearDisplay(0);
                subMenuSegment = 0;
              }
            } else if (gameMatrix[row][col] == 0) {
              score = score - levelPoints * 2;
              gameLives--;
            }
            if (gameLives == 0) {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Game Over!");
              lcd.setCursor(0, 1);
              lcd.print("Score: ");
              lcd.print(score);
              delay(4000);
              lcd.clear();
              int newScore = newHighScore(score);
              if (newScore > 0) {
                lcd.setCursor(0, 0);
                lcd.print("You Are Now ");
                lcd.setCursor(0, 1);
                lcd.print("#");
                lcd.print(newScore);
                lcd.print("In HighScore");
                delay(4000);
              }

              gameActive = 0;
              subMenuSegment = 0;
              subMenuActive = 0;
              mainMenuSegment = 1;
              menuActive = 1;
              deleteGameMatrix();
              lc.clearDisplay(0);
              lcd.clear();
              mainMenu();

            } else {
              lcd.setCursor(3, 1);
              lcd.print(score);
              lcd.setCursor(13, 1);
              lcd.print(gameLives);
            }
          }
        }
      }
    }

    else if (gameIntroActive) {
      if (subMenuSegment == 0) {
        lcd.setCursor(0, 0);
        lcd.print("Level ");
        lcd.print(gameLevel);
        lcd.setCursor(0, 1);
        lcd.print("S: ");
        lcd.print(score);
        lcd.setCursor(10, 1);
        lcd.write(byte(3));
        lcd.print(": ");
        lcd.print(gameLives);
        introTime = millis();
        subMenuSegment = 1;
        ledState = 0;
      } else {
        if (millis() - introTime > 4000 && subMenuSegment != 0) {
          if (gameLevel == 3 || (gameLevel == 4 && gameSegment == 1)) {
            introTime = millis();
          }
          if (subMenuSegment == 1) {
            currentGameDot = 0;
            subMenuSegment = 2;
          }
          if (gameSegment == 1) {
            if (gameLevel == 1) {
              gameDots = 2;
            } else if (gameLevel == 2) {
              gameDots = 4;
            } else if (gameLevel == 3) {
              gameDots = 4;
            } else if (gameLevel == 4) {
              gameDots = 8;
            }
          } else if (gameSegment == 2) {
            if (gameLevel == 1) {
              gameDots = 3;
            } else if (gameLevel == 2) {
              gameDots = 5;
            } else if (gameLevel == 3) {
              gameDots = 5;
            } else if (gameLevel == 4) {
              gameDots = 10;
            }
          } else if (gameSegment == 3) {
            if (gameLevel == 1) {
              gameDots = 4;
            } else if (gameLevel == 2) {
              gameDots = 6;
            } else if (gameLevel == 3) {
              gameDots = 6;
            } else {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Congratulations!");
              lcd.setCursor(0, 1);
              lcd.print("Score: ");
              lcd.print(score);
              delay(4000);
              lcd.clear();
              int newScore = newHighScore(score);
              if (newScore > 0) {
                lcd.setCursor(0, 0);
                lcd.print("You Are Now ");
                lcd.setCursor(0, 1);
                lcd.print("#");
                lcd.print(newScore);
                lcd.print("In HighScore");
                delay(4000);
              }

              gameActive = 0;
              subMenuSegment = 0;
              subMenuActive = 0;
              mainMenuSegment = 1;
              menuActive = 1;
              deleteGameMatrix();
              lc.clearDisplay(0);
              lcd.clear();
              mainMenu();
            }
          } else {
            gameLevel++;
            levelPoints = levelPoints + 5;
            gameSegment = 0;
            subMenuSegment = 0;
            deleteGameMatrix();
          }
          if (gameSegment == 0) {
            gameSegment++;
          } else {
            if (currentGameDot < gameDots) {
              if (gameLevel != 2) {
                if (gameLevel == 3 || (gameLevel == 4 && gameSegment == 1)) {
                  lc.setLed(0, row, col, 0);
                  currentGameDot++;
                } else {
                  currentGameDot++;
                }
                row = random(7);
                col = random(7);
                if (gameMatrix[row][col] == 0) {
                  gameMatrix[row][col] = order;
                  lc.setLed(0, row, col, 1);
                  if (gameLevel == 3 || (gameLevel == 4 && gameSegment == 1)) {
                    order++;
                  }
                }
              } else if (gameLevel == 2) {
                row = random(7);
                col = random(7);
                if (gameMatrix[row][col] == 0) {
                  if (currentGameDot == gameDots / 2 && millis() - level2Time > 10000) {
                    level2Time = millis();
                    lc.clearDisplay(0);
                    currentGameDot++;
                    gameMatrix[row][col] = order;
                    lc.setLed(0, row, col, 1);
                  } else if (currentGameDot != gameDots / 2) {
                    level2Time = millis();
                    currentGameDot++;
                    gameMatrix[row][col] = order;
                    lc.setLed(0, row, col, 1);
                  }
                }
              }
            } else if (currentGameDot == gameDots) {
              gameSegment++;
              subMenuSegment = 1;
              gameIntroActive = 0;
              order = 1;
              row = 0;
              col = 0;
              currentGameDot = 0;
              if (gameLevel == 1 || (gameLevel == 4 && gameSegment == 2)) {
                introTime == millis();
              }
            }
          }
        }
      }
    }
  }

  buttonLastReading = buttonReading;
}

void mainMenu() {
  if (mainMenuSegment > 4) {
    mainMenuSegment = 0;
  } else if (mainMenuSegment < 0) {
    mainMenuSegment = 4;
  }
  if (mainMenuSegment == 4) {
    lcd.setCursor(1, 0);
    lcd.print(mainMenuTitles[mainMenuSegment - 1]);
    lcd.setCursor(0, 1);
    lcd.write(byte(0));
    lcd.print(mainMenuTitles[mainMenuSegment]);
  } else {
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    lcd.print(mainMenuTitles[mainMenuSegment]);
    lcd.setCursor(1, 1);
    lcd.print(mainMenuTitles[mainMenuSegment + 1]);
  }
}

void settingsMenu() {
  if (subMenuSegment > 4) {
    subMenuSegment = 0;
  } else if (subMenuSegment < 0) {
    subMenuSegment = 4;
  }
  if (subMenuSegment == 4) {
    lcd.setCursor(1, 0);
    lcd.print(settings[subMenuSegment - 1]);
    lcd.setCursor(0, 1);
    lcd.write(byte(0));
    lcd.print(settings[subMenuSegment]);
  } else {
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    lcd.print(settings[subMenuSegment]);
    lcd.setCursor(1, 1);
    lcd.print(settings[subMenuSegment + 1]);
  }
}

void highScoreMenu() {
  if (subMenuSegment > 6) {
    subMenuSegment = 0;
  } else if (subMenuSegment < 0) {
    subMenuSegment = 6;
  }
  if (subMenuSegment == 6) {
    lcd.setCursor(1, 0);
    lcd.print(highScore[subMenuSegment - 1]);
    lcd.print(EEPROM[8]);
    lcd.setCursor(0, 1);
    lcd.write(byte(0));
    lcd.print(highScore[subMenuSegment]);
  } else {
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    lcd.print(highScore[subMenuSegment]);
    if (subMenuSegment != 0) {
      lcd.print(EEPROM[subMenuSegment + 3]);
    }
    lcd.setCursor(1, 1);
    lcd.print(highScore[subMenuSegment + 1]);
    if (subMenuSegment != 5) {
      lcd.print(EEPROM[subMenuSegment + 3]);
    }
  }
}

void aboutMenu() {
  if (subMenuSegment == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Button for <-");
  }
  lcd.setCursor(0, 1);
  for (int i = subMenuSegment; i < subMenuSegment + 15; i++) {
    lcd.print(about[i % (about.length())]);
  }
  subMenuSegment++;
}

void howToPlayMenu() {
  if (subMenuSegment > 4) {
    subMenuSegment = 0;
  } else if (subMenuSegment < 0) {
    subMenuSegment = 4;
  }
  if (subMenuSegment == 4) {
    if (scrollSegment == 0) {
      lcd.setCursor(1, 0);
      lcd.print("Level ");
      lcd.print(subMenuSegment - 1);
    }
    lcd.setCursor(0, 1);
    lcd.write(byte(0));
    for (int i = scrollSegment; i < scrollSegment + 15; i++) {
      lcd.print(howToPlay[subMenuSegment][i % (howToPlay[subMenuSegment].length())]);
    }
    scrollSegment++;
  } else if (subMenuSegment == 0) {
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    lcd.print(howToPlay[subMenuSegment]);
    lcd.setCursor(1, 1);
    lcd.print("Level ");
    lcd.print(subMenuSegment + 1);
  } else {
    if (scrollSegment == 0) {
      lcd.setCursor(1, 1);
      lcd.print("Level ");
      lcd.print(subMenuSegment + 1);
    }
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    for (int i = scrollSegment; i < scrollSegment + 15; i++) {
      lcd.print(howToPlay[subMenuSegment][i % (howToPlay[subMenuSegment].length())]);
    }
    scrollSegment++;
  }
}

void deleteGameMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      gameMatrix[row][col] = 0;
    }
  }
}

void setStartLevel() {
  lcd.clear();
  if (gameLevel < 1) {
    gameLevel = 1;
  } else if (gameLevel > 4) {
    gameLevel = 4;
  }
  lcd.setCursor(0, 1);
  lcd.print("Value: ");
  lcd.print(gameLevel);
  if (gameLevel != 4) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
  if (gameLevel != 1) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  }
}

void setLCDContrast() {
  lcd.clear();
  if (lcdContrast < 0) {
    lcdContrast = 0;
  } else if (lcdContrast > 150) {
    lcdContrast = 150;
  }
  lcd.setCursor(0, 1);
  lcd.print("Value: ");
  lcd.print(lcdContrast);
  if (lcdContrast != 150) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
  if (gameLevel != 0) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  }
}

void setGameSegment() {
  lcd.clear();
  if (gameSegment < 1) {
    gameSegment = 1;
  } else {
    if (gameLevel != 4) {
      if (gameSegment > 3) {
        gameSegment = 3;
      }
    } else {
      if (gameSegment > 2) {
        gameSegment = 2;
      }
    }
  }

  lcd.setCursor(0, 1);
  lcd.print("Value: ");
  lcd.print(gameSegment);
  if (gameLevel != 4) {
    if (gameSegment != 3) {
      lcd.setCursor(15, 0);
      lcd.write(byte(1));
    }
  } else {
    if (gameSegment != 2) {
      lcd.setCursor(15, 0);
      lcd.write(byte(1));
    }
  }

  if (gameSegment != 0) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  }
}

void setMatrixBrightness() {
  lcd.clear();
  if (matrixBrightness < 1) {
    matrixBrightness = 1;
  } else if (matrixBrightness > 13) {
    matrixBrightness = 13;
  }
  lcd.setCursor(0, 1);
  lcd.print("Value: ");
  lcd.print(matrixBrightness);
  if (matrixBrightness != 13) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
  if (matrixBrightness != 1) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  }
}

int newHighScore(int newScore) {
  int firstPosition;
  int ok = 0;
  for (int j = 4; j <= 8; j++) {
    if (newScore > EEPROM[j]) {
      firstPosition = j;
      for (int i = 8; i > firstPosition; i--) {
        EEPROM.update(i, EEPROM[i - 1]);
      }
      EEPROM.update(j, newScore);
      ok = j;
      break;
    }
  }
  return ok;
}