#include <FastLED.h>
#include <NewPing.h>


// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN0 3
#define DATA_PIN1 5 
#define DATA_PIN2 6
#define DATA_PIN3 9
#define DATA_PIN4 10

// Define the array of leds
#define NUM_LEDS 40 //number of leds per branch
#define NUM_BRANCH 5 //number of branches
CRGB leds[NUM_BRANCH][NUM_LEDS];

#define TRIGGER_PIN  11  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     11  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 100 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-NUM_BRANCH00cm.
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

unsigned int pingSpeed = 5; // How frequently are we going to send out a ping (in milliseconds). NUM_BRANCH0ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.

unsigned int glitchSpeed = 2000; // How frequently are we going to send a "glitch" (in ms)
unsigned long glitchTimer;

unsigned int sensorReading = 0; //Hold sensor value

void echoCheck();

void setup() { 
  FastLED.addLeds<NEOPIXEL, DATA_PIN0>(leds[0], NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN1>(leds[1], NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN2>(leds[2], NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN3>(leds[3], NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN4>(leds[4], NUM_LEDS);
  randomSeed(analogRead(0));
  Serial.begin(115200);
  pingTimer = millis(); // Start now.
  glitchTimer = millis();
}

void loop() {
  /**************************
   * Some parameters or restrictions to apply...
   * if you ever feel the need to apply restrictions.
   **************************/

  const char pathway = 1;
  const int phase = 0; //this is the phase delta between each branch
  const int startHue = 160;
  const int endHue = 250;
  const int startVal = 10;
  const int endVal = 100;

  /****************************
   * Magic happens here...
   *****************************/
  static unsigned char readings[NUM_LEDS];
  static int j=0;
  
  unsigned long glitchTimer, glitchLength, glitchI;

  int hue; //set the color in HSV color-space
  int val; //set the brightness
  int lastSensorReading; //holds last sample

  if (millis() >= pingTimer) {   // pingSpeed milliseconds since last ping, do another ping.
    pingTimer += pingSpeed;      // Set the next ping time.
    sonar.ping_timer(echoCheck); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status.

    if(lastSensorReading == sensorReading && j) {
      readings[j] = readings[(j-1)%NUM_LEDS] - 1;
    }

    else {
      readings[j] = map(sensorReading,0,MAX_DISTANCE,0,255);
    }

    lastSensorReading = sensorReading;
    j = (j+1) % NUM_LEDS; //so that the circular datastore only moves when a readings have been done :)
    Serial.println(readings[j], DEC); //display newly registered reading
  }

  if (millis() >= glitchTimer) {
    glitchTimer += glitchSpeed;
    glitchSpeed = random(5000,10000);
    glitchLength = random(NUM_LEDS/20);
    for(int glitchI = 0; glitchI <= glitchLength; glitchI++) {
      readings[j-(pathway*glitchI)] = readings[j-pathway] + 128 - map(glitchI,0,glitchLength,255,0);
    }
  }


  for(int i = 0; i < NUM_LEDS; i++) {
    for(int y = 0; y < NUM_BRANCH; y++) {
      hue = startHue + readings[(j+i*pathway+y*phase)%NUM_LEDS] % (endHue - startHue);
      val = map(readings[(j+i*pathway+y*phase)%NUM_LEDS],0,255,startVal,endVal);
      leds[y][i].setHSV(hue,255,100);//val);
    }
  }
  FastLED.show();
}

void echoCheck() { // Timer2 interrupt calls this function every 24uS where you can check the ping status.
  // Don't do anything here!
  if (sonar.check_timer()) { // This is how you check to see if the ping was received.
    // Here's where you can add code.
    sensorReading = sonar.ping_result / US_ROUNDTRIP_CM; // Ping returned, uS result in ping_result, convert to cm with US_ROUNDTRIP_CM.
  }
  // Don't do anything here!
}

