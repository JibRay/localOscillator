// localOscillator.ino
// Started by installing Adafruit Si5351 library. The following is a
// modification of the si5351 demo code.

#include <Adafruit_SI5351.h>

#define BUFFER_SIZE    80
#define S_IDLE          0
#define S_COMMAND       1
#define C_IDLE          0
#define C_VERSION       1
#define C_HELP          2
#define C_SET_FREQUENCY 3

//============================================================================
// Globals

const int version = 1;
Adafruit_SI5351 clockgen = Adafruit_SI5351();
char lineBuffer[BUFFER_SIZE];
int bufferIndex, mainState, commandState;
uint32_t frequency;

//============================================================================
// Global functions

void clearBuffer() {
  for (int i = 0; i < BUFFER_SIZE; ++i)
    lineBuffer[i] = '\0';
  bufferIndex = 0;
}

void printVersion() {
  Serial.print(F("localOscillator version "));
  Serial.println(version);
}

void setFrequency(float f) {
//  float f = atof(lineBuffer);
  frequency = (uint32_t)(f * 1e6);
  if ((frequency >= 3500000) && (frequency <= 4000000)) {
    Serial.print(F("Frequency set to "));
    Serial.println(frequency);
    // Set PLL_B to 900 MHz.
    clockgen.setupPLL(SI5351_PLL_B, 36, 0, 1);
    float denominator = 25000000.0 * 36.0 / frequency;
    uint32_t m1 = (uint32_t)denominator;
    uint32_t m2 = 1000.0 * (denominator - (float)m1);
    clockgen.setupMultisynth(1, SI5351_PLL_B, m1, m2, 1000);
  } else {
    Serial.println(F("Error: invalid fequency"));
  }
}

void setup(void)
{
  Serial.begin(19200);
  printVersion();
  clearBuffer();

  /* Initialise the sensor */
  if (clockgen.begin() != ERROR_NONE)
  {
    Serial.println(F("Connection to Si5351 clock generator failed"));
    while (1);
  }
  mainState = S_IDLE;
  commandState = C_IDLE;

  // INTEGER ONLY MODE --> most accurate output
  // Setup PLLA to integer only mode @ 900MHz (must be 600..900MHz)
  // Set Multisynth 0 to 112.5MHz using integer only mode (div by 4/6/8)
  // 25MHz * 36 = 900 MHz, then 900 MHz / 8 = 112.5 MHz
  Serial.println("Set PLLA to 900MHz");
  clockgen.setupPLLInt(SI5351_PLL_A, 36);
  Serial.println("Set Output #0 to 112.5MHz");
  clockgen.setupMultisynthInt(0, SI5351_PLL_A, SI5351_MULTISYNTH_DIV_8);

  // My code:
  // Serial.println("Set PLLA to 600MHz");
  // clockgen.setupPLLInt(SI5351_PLL_A, 24);
  // Serial.println("Set Output #0 to 75MHz");
  // clockgen.setupMultisynthInt(0, SI5351_PLL_A, SI5351_MULTISYNTH_DIV_8);

  // FRACTIONAL MODE --> More flexible but introduce clock jitter
  // Setup PLLB to fractional mode @616.66667MHz (XTAL * (24 + 2/3))
  // Setup Multisynth 1 to 13.55311MHz (PLLB/45.5)
  // clockgen.setupPLL(SI5351_PLL_B, 24, 2, 3);
  // Serial.println("Set Output #1 to 13.553115MHz");
  // clockgen.setupMultisynth(1, SI5351_PLL_B, 45, 1, 2);

  // My code:
  // clockgen.setupPLL(SI5351_PLL_B, p1, p2, p3);
  // clockgen.setupMultisynth(1, SI5351_PLL_B, m1, m2, m3);
  // f = (25e6 * (p1 + p2/p3)) / (m1 + (m2/m3))
  // clockgen.setupPLL(SI5351_PLL_B, 36, 0, 1);
  // Serial.println("Set Output #1 to 350.0 MHz");
  // clockgen.setupMultisynth(1, SI5351_PLL_B, 257, 1, 7);


  // Multisynth 2 is not yet used and won't be enabled, but can be
  // Use PLLB @ 616.66667MHz, then divide by 900 -> 685.185 KHz
  // then divide by 64 for 10.706 KHz
  // configured using either PLL in either integer or fractional mode

  Serial.println("Set Output #2 to 10.706 KHz");
  clockgen.setupMultisynth(2, SI5351_PLL_B, 900, 0, 1);
  clockgen.setupRdiv(2, SI5351_R_DIV_64);

  setFrequency(3.500000);
  // Enable the clocks
  clockgen.enableOutputs(true);
}

void printHelp() {
  Serial.println();
  Serial.println(F("Commands are 'Norm' or 'Imed'. Norm commands require an"));
  Serial.println(F("Enter character following input. Imed commands are"));
  Serial.println(F("executed when the command letter is received."));
  Serial.println(F("Command  Argument    Type  Description"));
  Serial.println(F("--------+-----------+-----+----------------------------"));
  Serial.println(F("  v                  Norm  Print version."));
  Serial.println(F("  h                  Norm  Print this help."));
  Serial.println(F("  f      frequency   Norm  Set fequency in megahertz."));
  Serial.println(F("                           Must be between 3.5 and 4.0."));
}

//============================================================================
// Main loop

void loop(void)
{
  if (Serial.available() > 0) {
    char c = Serial.read();
    switch (mainState) {
      case S_IDLE:
        switch (c) {
          case 'v':
            commandState = C_VERSION;
            mainState = S_COMMAND;
            clearBuffer();
            break;
          case 'h':
            commandState = C_HELP;
            mainState = S_COMMAND;
            clearBuffer();
            break;
      case 'f':
        commandState = C_SET_FREQUENCY;
        mainState = S_COMMAND;
        clearBuffer();
        }
        break;
      case S_COMMAND:
        switch (commandState) {
          case C_VERSION:
            if (c == '\n' || c == '\r') {
              printVersion();
              commandState = C_IDLE;
              mainState = S_IDLE;
            }
            break;
          case C_HELP:
            if (c == '\n' || c == '\r') {
              printHelp();
              commandState = C_IDLE;
              mainState = S_IDLE;
            }
            break;
          case C_SET_FREQUENCY:
            if (c == '\n' || c == '\r') {
              setFrequency(atof(lineBuffer));
              commandState = C_IDLE;
              mainState = S_IDLE;
            } else {
              lineBuffer[bufferIndex++] = c;
            }
            break;
        }
        break;
    }
  }
}
