int LED_RED = A0;
int LED_BLUE = A1;
int LED_GRN = A2;

bool ON = LOW;
bool OFF = HIGH;

#include <Keypad.h>
#include <SPI.h>
#include <SD.h>
#include <Keyboard.h> // The main library for sending keystrokes.

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
const int NORMAL = 0;
const int CHOOSE_FILE = 1;
const int READ_SERIAL = 2;
const int WRITE_FILE = 3;
int mode = NORMAL;
char fileNr = "\0";
String filename = "";
String serial_txt = "";

void setup() {
   pinMode(LED_BUILTIN, OUTPUT);
   pinMode(LED_RED, OUTPUT);
   pinMode(LED_GRN, OUTPUT);
   pinMode(LED_BLUE, OUTPUT);
   led_green();
   Serial.begin(9600);
   if (!SD.begin(10)) {
    digitalWrite(LED_RED, ON);
    while (1);
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
   led_green();
  if (key >= '0' && key <= '9'){
     filename = "";
     filename.concat(key);
     filename.concat(".txt");
     myFile = SD.open(filename);  
      if (myFile) {
         led_blue();
         // read from the file until there's nothing else in it:
        String s;
        while (myFile.available()) {
          char c = myFile.read();
          s += c;
        }
        myFile.close();
        Keyboard.print(s);                //  Send as keyboard chars
        Keyboard.press(KEY_RETURN);       // Press the Enter key.
        delay(100);                       // Wait for the computer to register the press.
        Keyboard.releaseAll();
        led_green();
      }
  }
  else {
    if (key == '*') {
       led_blue();
       mode = CHOOSE_FILE;
    }
    else {      
      digitalWrite(LED_RED, ON);
    }
  }
}


void read_serial() {
  char inchar = '\0';
  
  if (Serial.available()) { 
    delay(100);
    serial_txt = Serial.readString();
  }
  if (serial_txt.length() > 0) {
    mode = WRITE_FILE;
  }
}

bool writefile() {
    bool retval = false;
    filename = "";
    filename.concat(fileNr);
    filename.concat(".txt");
    
    if (SD.exists(filename)) {
       SD.remove(filename);
    }
    myFile = SD.open(filename, FILE_WRITE);  
    if (myFile) {
      myFile.println(serial_txt);
      myFile.close();  
    }
    if ( SD.exists(filename) ) {
       retval = true;
    }
    return retval;
}

void loop() {
  char key = keypad.getKey();

  if (key != '\0') {
      switch (mode) {
      case NORMAL : 
             normal_mode(key);
             break;
      case CHOOSE_FILE :  
             fileNr = key; 
             mode = READ_SERIAL;
             serial_txt = "";
             Serial.print("Send contents for file '");  Serial.print(fileNr); Serial.println(".TXT' via serial port");
             break;
       default :
           led_red(); 
           break;
       }
    }
    else {
      switch (mode) {
         case READ_SERIAL :
              read_serial(); 
              break;
         case WRITE_FILE :
              Serial.print("Writing contents to file '");  Serial.print(fileNr); Serial.println(".TXT'");
              if ( writefile() ) {
                 mode = NORMAL;
              }
              else {
                led_red();
              }   
              mode = NORMAL; 
              break;
      }
    }   
}
