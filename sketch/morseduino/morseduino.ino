/*
  Morseduino
  by Baldini Giulia, Nardini Matteo, Scolati Remo,
  faculty of Computer Science, Free University of Bolzano.

  This sketch reads the resistance variation of a fotoresistor and interprets it as morse code,
  displaying the received signal as standard text on a LCD display.
  This sketch was created as part of the 2016 Arduino Workshop at the Free University of Bolzano.
*/

/*
  This sketch is distributed with the license CC-BY-SA [Creative Commons-Attribution-Share Alike] 
  (http://creativecommons.org/licenses/by-sa/4.0/).
*/

#include <LiquidCrystal.h>    // Library used to handle the LCD screen

/*
  ========================  SKETCH'S CONSTANTS  ========================
*/
const int MORSE_UNIT = 700;   // The base unit of the morse comunication in milliseconds

//#define IS_DEBUG ;          // If the current execution should print debug information or not.
//#define AUTO_SPACE_WORDS ;  // If the current execution should put an automatic space between the words.


/*
  ======================== PIN LAYOUT DEFINITION ========================
*/

int fotoresistor_input_pin = A0;  // The analogue input pin to which the fotoresistor is connected

int dot_pluse_output_pin = 9;     // The digital output pin to pulse when a "dot" is received
int dash_pluse_output_pin = 6;    // The digital output pin to pulse when a "dash" is received



/*
  ========================  SKETCH  ========================
*/

int DELTA_MIN = 65;     // The minimum variation of resistance to be considered a "state change"

int sensorValue = 0;                    // The current value of the sensor
int sensorValueOld = 0;                 // The previous value of the sensor
int delta = 0;                          // The delta of resistance between this reading of the sensor and the previous one

unsigned long timeLastTurnedOn = 0;     // The number of milliseconds from the last reset at which the fotoresistor detected the last "ON" signal
unsigned long timeLastTurnedOff = 0;    // The number of milliseconds from the last reset at which the fotoresistor detected the last "OFF" signal
unsigned long interval = 0;             // Used as temporary variable

String currLetter = "";                 // The current letter being read. Here the letter is stored as morse, a character "." means that a dot was received, a character "-" means that a dash was received.
String currString = "";                 // The current string. It contains all the letters received up to a certain moment, converted in plain text.
bool letterHasBegun = false;            // If a letter is being received or not. 

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  // The variable holding the reference to the LCD screen used to display the output.

bool isSignalOn = false;                // If the received "signal" is on
  
void setup() {
    //sets up the LCD and the two LEDs

    lcd.begin(16, 2);  
    pinMode(dot_pluse_output_pin, OUTPUT);
    pinMode(dash_pluse_output_pin, OUTPUT);

    //If the Debug Mode is activated, it will print the message in the LCD
    #if defined(IS_DEBUG)     
      Serial.begin(9600);
      lcd.print("LCD test: if you can read this, it works");
      delay(1000);
      lcd.clear();
    #endif

    //Calibration of the light.
    lcd.setCursor(0, 0);
    lcd.print("Calibration...");
    lcd.setCursor(0, 1);
    lcd.print("Turn on led"); //The light has to be turned on.
    delay(3000);
    int onLevel = analogRead(fotoresistor_input_pin);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Calibration...");
    lcd.setCursor(0, 1);
    lcd.print("Turn off led"); //The light has to be turned off.
    delay(3000);
    int offLevel = analogRead(fotoresistor_input_pin);

    lcd.clear();
    DELTA_MIN = (onLevel - offLevel) / 2; //It calculates the difference of light value between the light turned off and on.

    sensorValueOld = analogRead(fotoresistor_input_pin); //sensorValueOld now contains the value of the sensor when the process starts

}

void loop() {

    // first the two LEDs are set as off
    digitalWrite(dot_pluse_output_pin, LOW);
    digitalWrite(dash_pluse_output_pin, LOW);
    

    sensorValue = analogRead(fotoresistor_input_pin); //takes in the value of current light from the sensor
    delta = sensorValue - sensorValueOld;             //the difference between the value that was calculated before and the value just calculated is defined.
                                                      //delta determines if there is any difference in the light caught before and after.
      

    if (delta > DELTA_MIN) {
        //There is a signal strong enough
        timeLastTurnedOn = millis(); 
        isSignalOn = true;
    }
      

    if (delta < -DELTA_MIN) {
        //There is not a signal strong enough
        timeLastTurnedOff = millis();
        isSignalOn = false;
        
        // Calculating the number of millisecond from the last time it turned on.
        interval =  timeLastTurnedOff - timeLastTurnedOn;

        //If the Debug Mode is activated, it will print the message on the serial monitor.
        #if defined(IS_DEBUG)
          Serial.print("Detected from HIGH to LOW. Interval: ");
          Serial.println(interval);
        #endif
        
        if (interval < MORSE_UNIT){
            //If the flash was short, so the sign detected is a dot.
            currLetter += ".";
            digitalWrite(dot_pluse_output_pin, HIGH);
            letterHasBegun = true;

            //If the Debug Mode is activated, it will print the message on the serial monitor.
            #if defined(IS_DEBUG)
                Serial.println("Detected .");
            #endif

        }else{
            //If the flash was long, so the sign detected is a dash.
            currLetter += "-";
            digitalWrite(dash_pluse_output_pin, HIGH);
            letterHasBegun = true;

            //If the Debug Mode is activated, it will print the message on the serial monitor.
            #if defined(IS_DEBUG)
                Serial.println("Detected -");
            #endif

        }
        
        
      }



    // If a letter should be added
    if (letterHasBegun && (millis() - timeLastTurnedOff) > (MORSE_UNIT * 2) && (!isSignalOn)){
        // Handling some special letters:
        //  -> "----" => Delete the message and reset the display
        //  -> "..--" => Delete the last letter from the message

          
        lcd.setCursor(0,0);
        lcd.clear(); // the LED is cleared and reset every time a letter is added.
        letterHasBegun = false;

        if (currLetter == "----"){
            // Reset
            currString = "";
            timeLastTurnedOff = millis();

            //If the Debug Mode is activated, it will print the message on the serial monitor.
            #if defined(IS_DEBUG)
                Serial.println("System resetted");
            #endif
                
        }else if (currLetter == "..--"){
            // Delete last letter
            currString = currString.substring(0, currString.length() - 1);
            lcd.print(currString); //Printing on the screen the new String

            //If the Debug Mode is activated, it will print the message on the serial monitor.
            #if defined(IS_DEBUG)
                Serial.print("Deleted last letter inserted. Current string: ");
                Serial.println(currString);
            #endif
            
        }else{
                
            //Adding current letter to current word
            currString += morseDictionary(currLetter);
                
            lcd.print(currString); //Printing on the screen the new String

            //If the Debug Mode is activated, it will print the message on the serial monitor.
            #if defined(IS_DEBUG)
                Serial.print("Letter added to string. Current string: ");
                Serial.println(currString);
            #endif
                
        }

            currLetter = ""; // currLetter is now resetted-
    }

    #if defined(AUTO_SPACE_WORDS)
        //If the auto-space mode is activated, if there has been a pause of more than 7 Morse Units, it puts a space.
        if (currString.charAt(currString.length() - 1) != 32 && (!isSignalOn) && (millis() - timeLastTurnedOff) > (MORSE_UNIT * 7) && currString != ""){
            currString += " ";
        }
    #endif

    // Delay that allows the board to process everything avoing double readings and similar.
    delay(90);

    //Now the value of sensorOld becames the value of
    sensorValueOld = sensorValue;   
      
}


//The following method takes in a String of dots and dashes and returns the corresponding letter.
//If the input String does not correspond to any letter, it returns an empty String.
String morseDictionary(String d){

    if(d == ".-") return "A";
    if(d == "-...") return "B";
    if(d == "-.-.") return "C";
    if(d == "-..")  return "D";
    if(d == ".") return "E";
    if(d == "..-.") return "F";
    if(d == "--.") return "G";
    if(d == "....") return "H";
    if(d == "..") return "I";
    if(d == ".---") return "J";
    if(d == "-.-") return "K";
    if(d == ".-..") return "L";
    if(d == "--") return "M";    
    if(d == "-.") return "N";
    if(d == "---") return "O";
    if(d == ".--.") return "P";
    if(d == "--.-") return "Q";
    if(d == ".-.") return "R";
    if(d == "...") return "S";
    if(d == "-") return "T";    
    if(d == "..-") return "U";
    if(d == "...-") return "V";    
    if(d == ".--") return "W";
    if(d == "-..-") return "X";    
    if(d == "-.--") return "Y";
    if(d == "--..") return "Z";

    return "";
}
