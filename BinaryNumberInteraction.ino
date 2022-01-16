#define Dig1 13
#define Dig2 10
#define Dig3 9
#define Dig4 2
#define DIGIT_ON LOW
#define DIGIT_OFF HIGH
#define DISPLAY_BRIGHTNESS 1
boolean duiz = false;
boolean hon = false;
#define segA 12
#define segB 8
#define segC 4
#define segD 6
#define segE 7
#define segF 11
#define segG 3
#define segPD = 5;

byte const numBits = 5;
uint8_t numberToConvertToBinary;
byte switches[numBits] = {A4, A3, A0, A1, A2}; //From most significant to least significat bit
byte fourDigitSevenSegmentDisplay[12] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
byte binaryNumber[numBits];

bool INTERACTION_SOLVED, INTERACTION_RUNNING;

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A7));
  for (byte i = 0; i < numBits; i++) {
    pinMode(switches[i], INPUT);
  }
  pinMode(segA, OUTPUT);
  pinMode(segB, OUTPUT);
  pinMode(segC, OUTPUT);
  pinMode(segD, OUTPUT);
  pinMode(segE, OUTPUT);
  pinMode(segF, OUTPUT);
  pinMode(segG, OUTPUT);
  pinMode(Dig1, OUTPUT);
  pinMode(Dig2, OUTPUT);
  pinMode(Dig3, OUTPUT);
  pinMode(Dig4, OUTPUT);
}

void loop() {
  if (!Serial) {
    Serial.begin(9600);
  }
  if (Serial.available()) {
    processSerialMessage();
  }
  if (INTERACTION_SOLVED == false && INTERACTION_RUNNING == true) {
    gameLoop();
  }
}

void gameLoop() {
  displayNumber(numberToConvertToBinary);
  for (byte i = 0; i < numBits; i++) {
    if ((!digitalRead(switches[i])) == HIGH) {
      //Print the number corresponding to the current combination of switches maybe;
    }
  }
  if (checkWinning()) {
    Serial.println("COM:INTERACTION_SOLVED;MSG:User Introduced Correct Binary Number;PNT:2000");
    Serial.flush();
    uint8_t times = 0;
    while (times < 5) {
      digitalWrite(Dig1, DIGIT_ON);
      lightNumber(8);
      digitalWrite(Dig2, DIGIT_ON);
      lightNumber(8);
      digitalWrite(Dig3, DIGIT_ON);
      lightNumber(8);
      digitalWrite(Dig4, DIGIT_ON);
      lightNumber(8);
      delay(500);
      digitalWrite(Dig4, DIGIT_OFF);
      digitalWrite(Dig3, DIGIT_OFF);
      digitalWrite(Dig2, DIGIT_OFF);
      digitalWrite(Dig1, DIGIT_OFF);
      delay(500);
      times++;
    }
  }
}

void processSerialMessage() {
  const int BUFF_SIZE = 64; // make it big enough to hold your longest command
  static char buffer[BUFF_SIZE + 1]; // +1 allows space for the null terminator
  static int length = 0; // number of characters currently in the buffer

  char c = Serial.read();
  if ((c == '\r') || (c == '\n')) {
    // end-of-line received
    if (length > 0) {
      tokenizeReceivedMessage(buffer);
    }
    length = 0;
  } else {
    if (length < BUFF_SIZE) {
      buffer[length++] = c; // append the received character to the array
      buffer[length] = 0; // append the null terminator
    }
  }
}

void tokenizeReceivedMessage(char *msg) {
  const int COMMAND_PAIRS = 10;
  char* tokenizedString[COMMAND_PAIRS + 1];
  int index = 0;

  char* command = strtok(msg, ";");
  while (command != 0) {
    char* separator = strchr(command, ':');
    if (separator != 0) {
      *separator = 0;
      tokenizedString[index++] = command;
      ++separator;
      tokenizedString[index++] = separator;
    }
    command = strtok(0, ";");
  }
  tokenizedString[index] = 0;

  processReceivedMessage(tokenizedString);
}

void processReceivedMessage(char** command) {
  if (strcmp(command[1], "START") == 0) {
    startSequence(command[3]);
  } else if (strcmp(command[1], "PAUSE") == 0) {
    pauseSequence(command[3]);
  } else if (strcmp(command[1], "STOP") == 0) {
    stopSequence(command[3]);
  } else if (strcmp(command[1], "INTERACTION_SOLVED_ACK") == 0) {
    setInteractionSolved();
  } else if (strcmp(command[1], "PING") == 0) {
    ping(command[3]);
  } else if (strcmp(command[1], "BAUD") == 0) {
    setBaudRate(atoi(command[3]), command[5]);
  } else if (strcmp(command[1], "SETUP") == 0) {
    Serial.println("COM:SETUP;INT_NAME:Binary Number Interaction;BAUD:9600");
    Serial.flush();
  }
}

void startSequence(char* TIMESTAMP) {
  numberToConvertToBinary = random(1, 32);
  for (byte i = 0; i < numBits; i++) {
    byte state = bitRead(numberToConvertToBinary, i);
    binaryNumber[((numBits - 1) - i)] = state;
  }
  INTERACTION_SOLVED = false;
  INTERACTION_RUNNING = true;
  Serial.print("COM:START_ACK;MSG:");
  for (uint8_t i = 0; i < numBits; i++) {
    Serial.print(binaryNumber[i]);
  }
  Serial.print(";ID:");
  Serial.print(TIMESTAMP);
  Serial.print("\r\n");
  Serial.flush();
}

void pauseSequence(char* TIMESTAMP) {
  INTERACTION_RUNNING = !INTERACTION_RUNNING;
  if (INTERACTION_RUNNING) {
    Serial.print("COM:PAUSE_ACK;MSG:Device is now running;ID:");
  } else {
    Serial.print("COM:PAUSE_ACK;MSG:Device is now paused;ID:");
  }
  Serial.print(TIMESTAMP);
  Serial.print("\r\n");
  Serial.flush();
}

void stopSequence(char* TIMESTAMP) {
  INTERACTION_RUNNING = false;
  Serial.print("COM:STOP_ACK;MSG:Device is now stopped;ID:");
  Serial.print(TIMESTAMP);
  Serial.print("\r\n");
  Serial.flush();
}

void setInteractionSolved() {
  INTERACTION_SOLVED = true;
  INTERACTION_RUNNING = false;
}

void ping(char* TIMESTAMP) {
  Serial.print("COM:PING;MSG:PING;ID:");
  Serial.print(TIMESTAMP);
  Serial.print("\r\n");
  Serial.flush();
}

void setBaudRate(int baudRate, char* TIMESTAMP) {
  Serial.flush();
  Serial.begin(baudRate);
  Serial.print("COM:BAUD_ACK;MSG:The Baud Rate was set to ");
  Serial.print(baudRate);
  Serial.print(";ID:");
  Serial.print(TIMESTAMP);
  Serial.print("\r\n");
  Serial.flush();
}

bool checkWinning() {
  for (byte i = 0; i < numBits; i++) {
    if ((!digitalRead(switches[i])) == binaryNumber[i]) {
      continue;
    } else {
      return false;
    }
  }
  INTERACTION_SOLVED = true;
  return true;
}

void displayNumber(int figur) {
  duiz = false;
  hon = false;
  for (int k = 0; k < 50; k++) { // for loop to slow it down.
    for (int digit = 1 ; digit < 5 ; digit++) { //for loop to place the number in the right digit
      switch (digit) {
        case 1:
          if (figur > 999) {
            digitalWrite(Dig1, DIGIT_ON);
            lightNumber(figur / 1000); // for example 2511 / 1000 = 2
            figur %= 1000; // new value of figur = 511 figur = figur %1000

            delayMicroseconds(DISPLAY_BRIGHTNESS);
            if (figur < 100) {
              duiz = true;
              if (figur < 10) {
                hon = true;

              }

            } else duiz = false;
          }

          break;
        case 2:
          if (duiz == true) {
            digitalWrite(Dig2, DIGIT_ON);
            lightNumber(0);
            delayMicroseconds(DISPLAY_BRIGHTNESS);

          } if (hon == true) {
            break;
          }

          if (figur > 99 && figur < 1000) {
            digitalWrite(Dig2, DIGIT_ON);
            lightNumber(figur / 100);
            figur %= 100;
            delayMicroseconds(DISPLAY_BRIGHTNESS);
            if (figur < 10) {
              hon = true;

            } else hon = false;
          }
          break;
        case 3:
          if (hon == true) {
            digitalWrite(Dig3, DIGIT_ON);
            lightNumber(0);
            delayMicroseconds(DISPLAY_BRIGHTNESS);
            break;
          }

          if (figur > 9 && figur < 100) {
            digitalWrite(Dig3, DIGIT_ON);
            lightNumber(figur / 10);
            figur %= 10;
            delayMicroseconds(DISPLAY_BRIGHTNESS);
          }

          break;
        case 4:
          if (figur < 10) {
            digitalWrite(Dig4, DIGIT_ON);
            lightNumber(figur);
            delayMicroseconds(DISPLAY_BRIGHTNESS);

            break;
          }
      }
      if (digit != 4) {
        //Turn off all segments
        lightNumber(10);
        //Turn off all digits
        digitalWrite(Dig1, DIGIT_OFF);
        digitalWrite(Dig2, DIGIT_OFF);
        digitalWrite(Dig3, DIGIT_OFF);
        digitalWrite(Dig4, DIGIT_OFF);
      }
    }
  }
}

void lightNumber(int numberToDisplay) {
#define SEGMENT_ON HIGH
#define SEGMENT_OFF LOW
  switch (numberToDisplay) {
    case 0:
      digitalWrite(segA, SEGMENT_ON);
      digitalWrite(segB, SEGMENT_ON);
      digitalWrite(segC, SEGMENT_ON);
      digitalWrite(segD, SEGMENT_ON);
      digitalWrite(segE, SEGMENT_ON);
      digitalWrite(segF, SEGMENT_ON);
      digitalWrite(segG, SEGMENT_OFF);
      break;
    case 1:
      digitalWrite(segA, SEGMENT_OFF);
      digitalWrite(segB, SEGMENT_ON);
      digitalWrite(segC, SEGMENT_ON);
      digitalWrite(segD, SEGMENT_OFF);
      digitalWrite(segE, SEGMENT_OFF);
      digitalWrite(segF, SEGMENT_OFF);
      digitalWrite(segG, SEGMENT_OFF);
      break;
    case 2:
      digitalWrite(segA, SEGMENT_ON);
      digitalWrite(segB, SEGMENT_ON);
      digitalWrite(segC, SEGMENT_OFF);
      digitalWrite(segD, SEGMENT_ON);
      digitalWrite(segE, SEGMENT_ON);
      digitalWrite(segF, SEGMENT_OFF);
      digitalWrite(segG, SEGMENT_ON);
      break;
    case 3:
      digitalWrite(segA, SEGMENT_ON);
      digitalWrite(segB, SEGMENT_ON);
      digitalWrite(segC, SEGMENT_ON);
      digitalWrite(segD, SEGMENT_ON);
      digitalWrite(segE, SEGMENT_OFF);
      digitalWrite(segF, SEGMENT_OFF);
      digitalWrite(segG, SEGMENT_ON);
      break;
    case 4:
      digitalWrite(segA, SEGMENT_OFF);
      digitalWrite(segB, SEGMENT_ON);
      digitalWrite(segC, SEGMENT_ON);
      digitalWrite(segD, SEGMENT_OFF);
      digitalWrite(segE, SEGMENT_OFF);
      digitalWrite(segF, SEGMENT_ON);
      digitalWrite(segG, SEGMENT_ON);
      break;
    case 5:
      digitalWrite(segA, SEGMENT_ON);
      digitalWrite(segB, SEGMENT_OFF);
      digitalWrite(segC, SEGMENT_ON);
      digitalWrite(segD, SEGMENT_ON);
      digitalWrite(segE, SEGMENT_OFF);
      digitalWrite(segF, SEGMENT_ON);
      digitalWrite(segG, SEGMENT_ON);
      break;
    case 6:
      digitalWrite(segA, SEGMENT_ON);
      digitalWrite(segB, SEGMENT_OFF);
      digitalWrite(segC, SEGMENT_ON);
      digitalWrite(segD, SEGMENT_ON);
      digitalWrite(segE, SEGMENT_ON);
      digitalWrite(segF, SEGMENT_ON);
      digitalWrite(segG, SEGMENT_ON);
      break;
    case 7:
      digitalWrite(segA, SEGMENT_ON);
      digitalWrite(segB, SEGMENT_ON);
      digitalWrite(segC, SEGMENT_ON);
      digitalWrite(segD, SEGMENT_OFF);
      digitalWrite(segE, SEGMENT_OFF);
      digitalWrite(segF, SEGMENT_OFF);
      digitalWrite(segG, SEGMENT_OFF);
      break;
    case 8:
      digitalWrite(segA, SEGMENT_ON);
      digitalWrite(segB, SEGMENT_ON);
      digitalWrite(segC, SEGMENT_ON);
      digitalWrite(segD, SEGMENT_ON);
      digitalWrite(segE, SEGMENT_ON);
      digitalWrite(segF, SEGMENT_ON);
      digitalWrite(segG, SEGMENT_ON);
      break;
    case 9:
      digitalWrite(segA, SEGMENT_ON);
      digitalWrite(segB, SEGMENT_ON);
      digitalWrite(segC, SEGMENT_ON);
      digitalWrite(segD, SEGMENT_ON);
      digitalWrite(segE, SEGMENT_OFF);
      digitalWrite(segF, SEGMENT_ON);
      digitalWrite(segG, SEGMENT_ON);
      break;
    case 10:
      digitalWrite(segA, SEGMENT_OFF);
      digitalWrite(segB, SEGMENT_OFF);
      digitalWrite(segC, SEGMENT_OFF);
      digitalWrite(segD, SEGMENT_OFF);
      digitalWrite(segE, SEGMENT_OFF);
      digitalWrite(segF, SEGMENT_OFF);
      digitalWrite(segG, SEGMENT_OFF);
      break;
  }
}
