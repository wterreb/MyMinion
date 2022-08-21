int LED_RED = A0;
int LED_BLUE = A1;
int LED_GRN = A2;

bool ON = LOW;
bool OFF = HIGH;

#include <Keypad.h>
#include <SPI.h>
#include <SD.h>
#include <Keyboard.h> // The main library for sending keystrokes.

bool  debugMode = false;

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {6,5,4,3}; //connect to the row pinouts of the keypad
byte colPins[COLS] =  {7,8,9}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
File myFile;
const int MODE_NORMAL = 0;
const int MODE_CHOOSE_WRITE_FILENAME = 1;
const int MODE_READ_FILE = 2;
const int MODE_SHOW_SEND_SERIAL_DATA_MESSAGE = 3;
const int MODE_READ_SERIAL_DATA = 4;
const int MODE_ERROR = 5;
int mode = MODE_NORMAL;
char keyChar = "\0";
String fileNumber = "";
String filename = "";


void setup() {
   pinMode(LED_BUILTIN, OUTPUT);
   pinMode(LED_RED, OUTPUT);
   pinMode(LED_GRN, OUTPUT);
   pinMode(LED_BLUE, OUTPUT);
   all_leds_off();
   Serial.begin(9600);
   if (!SD.begin(10)) {
    led_red();
    while (1);
  }
  else {
    led_green();
  }
  Keyboard.begin();  // Initialise the library.
}

void all_leds_off()
{
   digitalWrite(LED_RED, OFF);
   digitalWrite(LED_BLUE, OFF);
   digitalWrite(LED_GRN, OFF);
}

void led_red()
{
   all_leds_off();
   digitalWrite(LED_RED,ON);
}

void led_blue()
{
   all_leds_off();
   digitalWrite(LED_BLUE,ON);
}

void led_green()
{
   all_leds_off();
   digitalWrite(LED_GRN,ON);
}


void set_filename(char key) {
  filename = key + ".txt";
}


void normal_mode(char key) {

  if (key >= '0' && key <= '9') {
     filename.concat(key);  
  }
  else {
    if (key == '*') {
       mode = MODE_CHOOSE_WRITE_FILENAME;
    }
    if (key == '#') { 
      filename.concat(".txt");
      switch (mode) {
          case MODE_NORMAL :      
             mode = MODE_READ_FILE;
             break;
          case MODE_CHOOSE_WRITE_FILENAME : 
              mode = MODE_SHOW_SEND_SERIAL_DATA_MESSAGE;
              break;
          default:
             mode = MODE_ERROR;   
             break;
      }
    }  
  }
}

void read_file_contents() {
  myFile = SD.open(filename);  
  String inStr = "";
  
  if (myFile) {
    if (debugMode) {
      Serial.print("Read file "); Serial.println(filename);
    }

    while (myFile.available()) {  // read from the file until there's nothing else in it:
     // char c = (char)myFile.read();
      inStr = myFile.readString();  //read until timeout
    }
    myFile.close();

    for(int i =0; i < inStr.length(); i++ ) {
       char c = inStr[i];
       if (debugMode) {
          if (c == '+') {
              Serial.print("\r\n");
          }
          else {
             Serial.print(c);
          }
       } else {    
          if (c == '+') {
               Keyboard.press(KEY_RETURN);       // Press the Enter key.
               delay(500);                       // Wait for the computer to register the press.
          }
          else {
            Keyboard.print(c);                //  Send as keyboard chars
          }
         
       }
    }
    if (debugMode) {
    Serial.println(" ");
    } else {
     //   Keyboard.press(KEY_RETURN);       // Press the Enter key.
     //   delay(100);                       // Wait for the computer to register the press.
        Keyboard.releaseAll();
    }
  } else {       
      led_red();
      delay(1000); 
  }
  filename = "";  
}


bool read_serial() {
  bool retval = false;
  if ( Serial.available() ) { 
      if (SD.exists(filename)) {
          SD.remove(filename);
      }
      Serial.print("Write file : "); Serial.println(filename);
      myFile = SD.open(filename, FILE_WRITE);
      while ( Serial.available() )
      {
        char rc = Serial.read();
        if (myFile) {
          myFile.print(rc);
        }
        Serial.print(rc);
      }
      if (myFile) {
          delay(100);
          myFile.close();  
      }
      if ( SD.exists(filename) ) {
          retval = true;
      }
  }
  return retval;
}



void loop() {
  char key = keypad.getKey();

  if (key != '\0') {
      switch (mode) {
      case MODE_NORMAL :               // Fall through
      case MODE_CHOOSE_WRITE_FILENAME:
             normal_mode(key);
             break;
       default :
           led_red(); 
           break;
       }
    }
    else {
      switch (mode) {
         case MODE_READ_FILE: 
             read_file_contents();
             mode = MODE_NORMAL;
             break;

         case MODE_SHOW_SEND_SERIAL_DATA_MESSAGE:
             Serial.println("");
             Serial.print("Send contents for file '");  Serial.print(filename); Serial.println("' via serial port"); 
             mode = MODE_READ_SERIAL_DATA;
             break;
             
         case MODE_READ_SERIAL_DATA :
              if ( read_serial() ) {
                 mode = MODE_NORMAL;
                 filename = "";
              }
              break;
      }
    }   
    if (mode == MODE_ERROR) {
      led_red();
      delay(1000); 
      mode = MODE_NORMAL;
    }
    if (mode == MODE_NORMAL || mode == MODE_CHOOSE_WRITE_FILENAME) {
      if (filename.length() == 0) {
         led_green();
      } else {
         led_blue();
      }
    }
}
