// localOscillator.ino
// Started by installing Adafruit Si5351 library. The following is a
// modification of the si5351 demo code.

#include <Adafruit_SI5351.h>

//============================================================================
// Globals

const int version = 1;

Adafruit_SI5351 clockgen = Adafruit_SI5351();

//============================================================================
// Global functions

void setup(void)
{
  Serial.begin(19200);
  Serial.println(F("localOscillator version "));
  Serial.println(version);

  /* Initialise the sensor */
  if (clockgen.begin() != ERROR_NONE)
  {
    Serial.println(F("Connection to Si5351 clock generator failed"));
    while(1);
  }

  // INTEGER ONLY MODE --> most accurate output
  // Setup PLLA to integer only mode @ 900MHz (must be 600..900MHz)
  // Set Multisynth 0 to 112.5MHz using integer only mode (div by 4/6/8)
  // 25MHz * 36 = 900 MHz, then 900 MHz / 8 = 112.5 MHz
  Serial.println("Set PLLA to 900MHz");
  clockgen.setupPLLInt(SI5351_PLL_A, 36);
  Serial.println("Set Output #0 to 112.5MHz");
  clockgen.setupMultisynthInt(0, SI5351_PLL_A, SI5351_MULTISYNTH_DIV_8);

  // FRACTIONAL MODE --> More flexible but introduce clock jitter
  // Setup PLLB to fractional mode @616.66667MHz (XTAL * 24 + 2/3)
  // Setup Multisynth 1 to 13.55311MHz (PLLB/45.5)
  clockgen.setupPLL(SI5351_PLL_B, 24, 2, 3);
  Serial.println("Set Output #1 to 13.553115MHz");
  clockgen.setupMultisynth(1, SI5351_PLL_B, 45, 1, 2);

  // Multisynth 2 is not yet used and won't be enabled, but can be
  // Use PLLB @ 616.66667MHz, then divide by 900 -> 685.185 KHz
  // then divide by 64 for 10.706 KHz
  // configured using either PLL in either integer or fractional mode

  Serial.println("Set Output #2 to 10.706 KHz");
  clockgen.setupMultisynth(2, SI5351_PLL_B, 900, 0, 1);
  clockgen.setupRdiv(2, SI5351_R_DIV_64);

  // Enable the clocks
  clockgen.enableOutputs(true);
}

//============================================================================
// Main loop

void loop(void)
{
}
