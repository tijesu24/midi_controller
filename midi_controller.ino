#include <Bounce.h>

// Map MIDI CC channels to knobs numbered left to right.
#define CC01  102
#define CC02  103
#define CC03  104

#define CC04  105
#define CC05  106
#define CC06  107



// Map the TeensyLC pins to each potentiometer numbered left to right.
#define POT01 0
a
#define POT02 1



// Use this MIDI channel.
#define MIDI_CHANNEL 1


// the MIDI channel number to send messages
const int channel = 1;

// Create Bounce objects for each button.  The Bounce object
// automatically deals with contact chatter or "bounce", and
// it makes detecting changes very simple.
Bounce button0 = Bounce(3, 5);
Bounce button1 = Bounce(4, 5);  // 5 = 5 ms debounce time
Bounce button2 = Bounce(5, 5);

// Send MIDI CC messages for all 17 knobs after the main loop runs this many times.
// This prevents having to twiddle the knobs to update the receiving end.
// 10,000 loops is roughly 10 seconds.
#define LOOPS_PER_REFRESH 10000

// potentiometer read parameters
#define POT_BIT_RES         10 // 10 works, 7-16 is valid
#define POT_NUM_READS       32 // 32 works

// Track the knob state.
uint16_t prev_pot_val[2] = {0xffff,
                             0xffff};

// an array of potentiometer pins
uint8_t pot[2] = {POT01,
                   POT02};

// an array of CC numbers
uint8_t cc[5] = {CC01,CC02, CC03, CC04, CC05};

// Prevent jitter when reading the potentiometers.
// Higher value is less chance of jitter but also less precision.
const uint8_t nbrhd = 5;

// Count the number of main loops.
uint16_t loop_count = 0;

void setup() {
  // serial monitoring for debugging
  Serial.begin(38400);

  // potentiometers
  analogReadResolution(POT_BIT_RES);
  analogReadAveraging(POT_NUM_READS);
}

void loop() {

  button0.update();
  button1.update();
  button2.update();

  // Check each button for "falling" edge.
  // Send a MIDI Note On message when each button presses
  // Update the Joystick buttons only upon changes.
  // falling = high (not pressed - voltage from pullup resistor)
  //           to low (pressed - button connects pin to ground)
  if (button0.fallingEdge()) {
    usbMIDI.sendNoteOn(cc[3], 99, channel); 
  }
  if (button1.fallingEdge()) {
    usbMIDI.sendNoteOn(cc[4], 99, channel); 
  }
  if (button2.fallingEdge()) {
    usbMIDI.sendNoteOn(cc[5], 99, channel); 
  }

  // Check each button for "rising" edge
  // Send a MIDI Note Off message when each button releases
  // For many types of projects, you only care when the button
  // is pressed and the release isn't needed.
  // rising = low (pressed - button connects pin to ground)
  //          to high (not pressed - voltage from pullup resistor)
  if (button0.risingEdge()) {
    usbMIDI.sendNoteOff(cc[3], 0, channel); 
  }
  if (button1.risingEdge()) {
    usbMIDI.sendNoteOff(cc[4], 0, channel);  
  }
  if (button2.risingEdge()) {
    usbMIDI.sendNoteOff(cc[5], 0, channel);  
  }

  
  // Read each pot knob, and send MIDI CC only if the value changed.
  for (uint8_t i = 0; i < 2; i++) {
    uint16_t pot_val = analogRead(pot[i]);
    if ((pot_val < prev_pot_val[i] - nbrhd) ||
        (pot_val > prev_pot_val[i] + nbrhd)) {
      usbMIDI.sendControlChange(cc[i], pot_val >> (POT_BIT_RES - 7), MIDI_CHANNEL);
      prev_pot_val[i] = pot_val;
    }
  }

  // Periodically send MIDI CC for every knob so that the receiving end matches the knobs
  // even when changing pure data patches.
  if (loop_count > LOOPS_PER_REFRESH) {
    //Pot
    for (uint8_t i = 0; i < 2; i++) {
      usbMIDI.sendControlChange(cc[i], analogRead(pot[i]) >> (POT_BIT_RES - 7), MIDI_CHANNEL);
    }
    for (uint8_t i = 0; i < 2; i++) {
      usbMIDI.sendControlChange(cc[i], analogRead(pot[i]) >> (POT_BIT_RES - 7), MIDI_CHANNEL);
    }
    loop_count = 0;
  }
  loop_count++;
}
