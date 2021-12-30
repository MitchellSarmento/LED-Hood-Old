#include <PixelStrip.h>
#include <bluefruit.h>
#include <SPI.h>

// Pin connections
#define DATAPIN SCK
#define CLOCKPIN MOSI

// BLE Service
BLEDfu bledfu;
BLEDis bledis;
BLEUart bleuart;

// Effects control vars
byte effectIndex = 0;
#define numEffects 7

// Brightness vars
byte brightnessIndex = 0;
const byte brightnessLevels[] = {10, 30, 140, 230};
const byte numBrightnessLevels = SIZE(brightnessLevels);

// Speed vars
byte pulseSpeedIndex = 0;
const byte pulseSpeeds[] = {1, 1.5, 2};
const byte numPulseSpeeds = SIZE(pulseSpeeds);

// Strip definitions
const uint16_t stripLength = 20;
const uint8_t stripType = NEO_RGB + NEO_KHZ800;

// Patterns
enum pattern {
  RAINBOW_CYCLE,
  PULSE,
  RAINBOW_PULSE,
  PHOTON,
  RAINBOW_PHOTON,
  ROLLING,
  RAINBOW_ROLLING,
  SPARKLE
};

enum direction { FORWARD, REVERSE };

class PixelStripPatterns : public PixelStrip
{
  public:
    //-------------COLORS-----------------------------------------------COLORS
    //Define some colors we'll use frequently
    const uint32_t white =    this->Color(255, 255, 255);
    const uint32_t UCLAGold = this->Color(254, 187, 54);
    const uint32_t UCLABlue = this->Color(83, 104, 149);
    const uint32_t off =      this->Color( 0, 0, 0 );
    const uint32_t red =      this->Color(255, 0, 0);
    const uint32_t orange =   this->Color(255, 43, 0);
    const uint32_t ltOrange = this->Color(255, 143, 0);
    const uint32_t yellow =   this->Color(255, 255, 0);
    const uint32_t ltYellow = this->Color(255, 255, 100);
    const uint32_t green =    this->Color(0, 128, 0);
    const uint32_t blue =     this->Color(0, 0, 255);
    const uint32_t indigo =   this->Color( 75, 0, 130);
    const uint32_t violet =   this->Color(238, 130, 238);
    const uint32_t purple =   this->Color(123, 7, 197);
    const uint32_t pink =     this->Color(225, 0, 127);
  
    const uint32_t pastelRainbow = this->Color(130, 185, 226); //178,231,254,
    const uint32_t pastelRainbow1 = this->Color(110, 46, 145); //purple
    const uint32_t pastelRainbow2 = this->Color(54, 174, 218); //teal
    const uint32_t pastelRainbow3 = this->Color(120, 212, 96); //green
    const uint32_t pastelRainbow4 = this->Color(255, 254, 188); //yellow
    const uint32_t pastelRainbow5 = this->Color(236, 116, 70); //orange
    const uint32_t pastelRainbow6 = this->Color(229, 61, 84); //pink red
    //-------------COLORS-----------------------------------------------COLORS
  
    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern

    bool ReverseWhenComplete;
    unsigned long PulseSpeed = pulseSpeeds[pulseSpeedIndex];
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    PixelStripPatterns(uint16_t pixels, uint8_t datapin, uint8_t clockpin, uint8_t type, void (*callback)())
    :PixelStrip(pixels, datapin, clockpin, type)
    {
        OnComplete = callback;
    }

    // Update the pattern
    void Update()
    {
      if ((millis() - lastUpdate) > Interval) // time to update
      {
        lastUpdate = millis();
        switch(ActivePattern)
        {
          case RAINBOW_CYCLE:
            RainbowCycleUpdate();
            break;
          case PULSE:
            PulsePatternUpdate();
            break;
          case RAINBOW_PULSE:
            RainbowPulsePatternUpdate();
            break;
          case PHOTON:
            PhotonPatternUpdate();
            break;
          case RAINBOW_PHOTON:
            RainbowPhotonUpdate();
            break;
          case ROLLING:
            RollingUpdate();
            break;
          case RAINBOW_ROLLING:
            RainbowRollingUpdate();
            break;
          case SPARKLE:
            SparkleUpdate();
            break;
          default:
            break;
        }
      }
    }

    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = REVERSE)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 240;
        Index = 0;
        Direction = dir;
        ReverseWhenComplete = false;
    }
    
    // Update the Rainbow Cycle Pattern.
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }

    // Initialize Pulse Pattern.
    void PulsePattern(uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = PULSE;
      Interval = interval;
      TotalSteps = 20;
      Index = 0;
      Direction = dir;
      ReverseWhenComplete = false;
      Color1 = yellow;
    }

    // Update Pulse Pattern.
    void PulsePatternUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        if (i == Index || i == Index + 1) {
          setPixelColor(i, Color1);
        } else {
          setPixelColor(i, DimColor(DimColor(Color1)));
        }
        delay(PulseSpeed);
      }
      show();
      Increment();
    }

    // Initialize Rainbow Pulse Pattern.
    void RainbowPulsePattern(uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = RAINBOW_PULSE;
      Interval = interval;
      TotalSteps = 240;
      Index = 0;
      Direction = dir;
      ReverseWhenComplete = false;
    }

    // Update Rainbow Pulse Pattern.
    void RainbowPulsePatternUpdate()
    {
      int pixels = numPixels();
      
      for (int i = 0; i < pixels; i++)
      {
        uint32_t color = Wheel(((i * 256 / numPixels()) + Index) & 255);
        
        if (i == Index % pixels || i == (Index + 1) % pixels) {
          setPixelColor(i, color);
        } else {
          setPixelColor(i, DimColor(DimColor(color)));
        }
        delay(PulseSpeed);
      }
      show();
      Increment();
    }

    // Initialize Photon Pattern.
    void PhotonPattern(uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = PHOTON;
      Interval = interval;
      TotalSteps = 20;
      Index = 0;
      Direction = dir;
      ReverseWhenComplete = false;
      Color1 = yellow;
    }

    // Update Photon Pattern.
    void PhotonPatternUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        if (i == Index) {
          setPixelColor(i, Color1);
        } else if (i == Index + 1) {
          setPixelColor(i, DimColor(Color1));
        } else if (i == Index + 2) {
          setPixelColor(i, DimColor(DimColor(Color1)));
        } else if (i == Index + 3) {
          setPixelColor(i, DimColor(DimColor(DimColor(Color1))));
        } else if (i == Index + 4) {
          setPixelColor(i, DimColor(DimColor(DimColor(DimColor(Color1)))));
        } else {
          setPixelColor(i, 0);
        }
        delay(PulseSpeed);
      }
      show();
      Increment();
    }

    // Initialize Rainbow Photon Pattern.
    void RainbowPhotonPattern(uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = RAINBOW_PHOTON;
      Interval = interval;
      TotalSteps = 240;
      Index = 0;
      Direction = dir;
      ReverseWhenComplete = false;
    }

    // Update the Rainbow Photon Pattern.
    void RainbowPhotonUpdate()
    {
      int pixels = numPixels();
      
      for (int i = 0; i < pixels; i++)
      {
        uint32_t color = Wheel(((i * 256 / numPixels()) + Index) & 255);
        
        if (i == Index % pixels) {
          setPixelColor(i, color);
        } else if (i == (Index + 1) % pixels) {
          setPixelColor(i, DimColor(color));
        } else if (i == (Index + 2) % pixels) {
          setPixelColor(i, DimColor(DimColor(color)));
        } else if (i == (Index + 3) % pixels) {
          setPixelColor(i, DimColor(DimColor(DimColor(color))));
        } else if (i == (Index + 4) % pixels) {
          setPixelColor(i, DimColor(DimColor(DimColor(DimColor(color)))));
        } else {
          setPixelColor(i, 0);
        }
        delay(PulseSpeed);
      }
      show();
      Increment();
    }

    // Initialize Rolling Pattern.
    void RollingPattern(uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = ROLLING;
      Interval = interval;
      TotalSteps = 20;
      Index = 0;
      Direction = dir;
      ReverseWhenComplete = false;
      Color1 = yellow;

      fill(0, 0, numPixels());
    }

    // Update the Rolling Pattern.
    void RollingUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        if (Index % 2 == 0) {
          setPixelColor(0, Color1);
          setPixelColor(6, Color1);
          setPixelColor(12, Color1);
          setPixelColor(18, Color1);
          setPixelColor(1, 0);
          setPixelColor(7, 0);
          setPixelColor(13, 0);
          setPixelColor(19, 0);
        } else {
          setPixelColor(0, 0);
          setPixelColor(6, 0);
          setPixelColor(12, 0);
          setPixelColor(18, 0);
          setPixelColor(1, Color1);
          setPixelColor(7, Color1);
          setPixelColor(13, Color1);
          setPixelColor(19, Color1);
        }
        delay(PulseSpeed);
      }
      
      show();
      Increment();
    }

    // Initialize Rainbow Rolling Pattern.
    void RainbowRollingPattern(uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = RAINBOW_ROLLING;
      Interval = interval;
      TotalSteps = 240;
      Index = 0;
      Direction = dir;
      ReverseWhenComplete = false;

      fill(0, 0, numPixels());
    }

    // Update the Rainbow Rolling Pattern.
    void RainbowRollingUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        uint32_t color = Wheel(((i * 256 / numPixels()) + Index) & 255);
        
        if (Index % 2 == 0) {
          setPixelColor(0, color);
          setPixelColor(6, color);
          setPixelColor(12, color);
          setPixelColor(18, color);
          setPixelColor(1, 0);
          setPixelColor(7, 0);
          setPixelColor(13, 0);
          setPixelColor(19, 0);
        } else {
          setPixelColor(0, 0);
          setPixelColor(6, 0);
          setPixelColor(12, 0);
          setPixelColor(18, 0);
          setPixelColor(1, color);
          setPixelColor(7, color);
          setPixelColor(13, color);
          setPixelColor(19, color);
        }
        delay(PulseSpeed);
      }
      
      show();
      Increment();
    }

    // Initialize Sparkle Pattern.
    void SparklePattern(uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = SPARKLE;
      Interval = interval;
      TotalSteps = 10922;
      Index = 0;
      Direction = dir;
      ReverseWhenComplete = false;
      Color1 = yellow;

      fill(0, 0, numPixels());
    }

    // Update the Sparkle Pattern.
    void SparkleUpdate()
    {
      uint32_t firePallet[3] = {red, ltOrange, ltYellow};
      int sizeOfPallet = SIZE(firePallet);
      
      /*for(int i=0; i< numPixels(); i++)
      {
        //setPixelColor(i, firePallet[i % sizeOfPallet]);
        //setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        //setPixelColor(i, firePallet[(i + Index) % sizeOfPallet]);
        //setPixelColor(i, Red(Wheel(((i * 256 / numPixels()) + Index))));
        //setPixelColor(i, ColorHSV(
      }*/
      /*for (int i = 0; i < 10922; i++) {
        setPixelColor(
      }*/
      /*for (int i = 0; i < numPixels(); i++) {
        //setPixelColor(i, ColorHSV(i + Index, 255, brightnessLevels[brightnessIndex]));
        //setPixelColor(i, ColorHSV(i + Index, 255, 255));
        setPixelColor(i, ColorHSV(((i * 256 / numPixels()) + (Index * 100)) & 255));
        show();
      }*/
      /*for (int i = 0; i < 10922; i++)
      {
        for (int j = 0; j < 20; j++)
        {
          setPixelColor(j, ColorHSV(i + (j * 10)));
        }
        show();
      }*/
      //show();
      //Increment();
      //delay(100);
    }

    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};

void StripComplete();

PixelStripPatterns strip(stripLength, DATAPIN, CLOCKPIN, stripType, &StripComplete);

void brightnessAdjust() {
  brightnessIndex = (brightnessIndex + 1) % numBrightnessLevels;
  strip.setBrightness( brightnessLevels[brightnessIndex] );
  sendResponse("Changed brightness.");
}

void pulseSpeedAdjust() {
  pulseSpeedIndex = (pulseSpeedIndex + 1) % numPulseSpeeds;
  strip.PulseSpeed = pulseSpeeds[pulseSpeedIndex];
  sendResponse("Changed pulse speed.");
}

//-------------SETUP-----------------------------------------------SETUP
void setup() {
  delay(3000); // power-up safety delay
  
  //initalize the led strip, and set the starting brightness
  strip.begin();

  strip.setBrightness( brightnessLevels[brightnessIndex] );
  strip.show();

  // Start a pattern
  strip.RainbowCycle(3);

  // Bluefruit setup
  Bluefruit.begin();
  Bluefruit.setName("Mitch_LED_Hood");
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  Bluefruit.Periph.setConnectCallback(connect_callback);
  
  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();  

  // Configure and start BLE UART service
  bleuart.begin();

  // Set up and start advertising
  startAdv();
}
//-------END SETUP---------------------------------------------END SETUP

void startAdv(void)
{  
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  
  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));
}

uint8_t mode = 'a';
bool adjustingColor = false;
bool adjustingMainColor = false;
char const* response;

//---------MAIN LOOP-------------------------------------------MAIN LOOP
void loop() {
  if ( Bluefruit.connected() && bleuart.notifyEnabled() )
  {
    int command = bleuart.read();

    if (adjustingColor) {
      switch (command) {
        case 'r': { // Set color red
          if (adjustingMainColor) {
            strip.Color1 = strip.red;
          } else {
            strip.Color2 = strip.red;
          }
          adjustingColor = false;
          sendResponse("Set color red.");
          break;
        }
        case 'o': { // Set color orange
          if (adjustingMainColor) {
            strip.Color1 = strip.orange;
          } else {
            strip.Color2 = strip.orange;
          }
          adjustingColor = false;
          sendResponse("Set color orange.");
          break;
        }
        case 'y': { // Set color yellow
          if (adjustingMainColor) {
            strip.Color1 = strip.yellow;
          } else {
            strip.Color2 = strip.yellow;
          }
          adjustingColor = false;
          sendResponse("Set color yellow.");
          break;
        }
        case 'g': { // Set color green
          if (adjustingMainColor) {
            strip.Color1 = strip.green;
          } else {
            strip.Color2 = strip.green;
          }
          adjustingColor = false;
          sendResponse("Set color green.");
          break;
        }
        case 'b': { // Set color blue
          if (adjustingMainColor) {
            strip.Color1 = strip.blue;
          } else {
            strip.Color2 = strip.blue;
          }
          adjustingColor = false;
          sendResponse("Set color blue.");
          break;
        }
        case 'i': { // Set color indigo
          if (adjustingMainColor) {
            strip.Color1 = strip.indigo;
          } else {
            strip.Color2 = strip.indigo;
          }
          adjustingColor = false;
          sendResponse("Set color indigo.");
          break;
        }
        case 'v': { // Set color violet
          if (adjustingMainColor) {
            strip.Color1 = strip.violet;
          } else {
            strip.Color2 = strip.violet;
          }
          adjustingColor = false;
          sendResponse("Set color violet.");
          break;
        }
        case 'x': { // Cancel color change
          adjustingColor = false;
          sendResponse("Cancelled.");
          break;
        }
      }
    } else {
      switch (command) {
        case 'x': { // Brightness
          brightnessAdjust();
          break;
        }
        case 'p': { // Pulse speed
          pulseSpeedAdjust();
          break;
        }
        case 'm': { // Main color (Color1)
          adjustingColor = true;
          adjustingMainColor = true;
          sendResponse("Adj Color1: ROYGBIV.");
          break;
        }
        case 's': { // Secondary color (Color2)
          adjustingColor = true;
          adjustingMainColor = false;
          sendResponse("Adj Color2: ROYGBIV.");
          break;
        }
        case 'a': { // Change mode to a
          mode = 'a';
          sendResponse("Changed to mode A.");
          break;
        }
        case 'b': { // Change mode to b
          mode = 'b';
          sendResponse("Changed to mode B.");
          break;
        }
        case 'c': { // Change mode to c
          mode = 'c';
          sendResponse("Changed to mode C.");
          break;
        }
        case 'd': { // Change mode to d
          mode = 'd';
          sendResponse("Changed to mode D.");
          break;
        }
        case 'e': { // Change mode to e
          mode = 'e';
          sendResponse("Changed to mode E.");
          break;
        }
        case '0': {
          uint8_t n;
          switch (mode) {
            case 'a': { n = 0; strip.RainbowCycle(3); sendResponse("Changed to pattern 0."); break; }
            case 'b': { n = 10; sendResponse("Changed to pattern 10."); break; }
            case 'c': { n = 20; sendResponse("Changed to pattern 20."); break; }
            case 'd': { n = 30; sendResponse("Changed to pattern 30."); break; }
            case 'e': { n = 40; sendResponse("Changed to pattern 40."); break; }
          }
          strip.ActivePattern = (pattern)n;
          break;
        }
        case '1': {
          uint8_t n;
          switch (mode) {
            case 'a': { n = 1; strip.PulsePattern(3); sendResponse("Changed to pattern 1."); break; }
            case 'b': { n = 11; sendResponse("Changed to pattern 11."); break; }
            case 'c': { n = 21; sendResponse("Changed to pattern 21."); break; }
            case 'd': { n = 31; sendResponse("Changed to pattern 31."); break; }
          }
          strip.ActivePattern = (pattern)n;
          break;
        }
        case '2': {
          uint8_t n;
          switch (mode) {
            case 'a': { n = 2; strip.RainbowPulsePattern(3); sendResponse("Changed to pattern 2."); break; }
            case 'b': { n = 12; sendResponse("Changed to pattern 12."); break; }
            case 'c': { n = 22; sendResponse("Changed to pattern 22."); break; }
            case 'd': { n = 32; sendResponse("Changed to pattern 32."); break; }
          }
          strip.ActivePattern = (pattern)n;
          break;
        }
        case '3': {
          uint8_t n;
          switch (mode) {
            case 'a': { n = 3; strip.PhotonPattern(3); sendResponse("Changed to pattern 3."); break; }
            case 'b': { n = 13; sendResponse("Changed to pattern 13."); break; }
            case 'c': { n = 23; sendResponse("Changed to pattern 23."); break; }
            case 'd': { n = 33; sendResponse("Changed to pattern 33."); break; }
          }
          strip.ActivePattern = (pattern)n;
          break;
        }
        case '4': {
          uint8_t n;
          switch (mode) {
            case 'a': { n = 4; strip.RainbowPhotonPattern(3); sendResponse("Changed to pattern 4."); break; }
            case 'b': { n = 14; sendResponse("Changed to pattern 14."); break; }
            case 'c': { n = 24; sendResponse("Changed to pattern 24."); break; }
            case 'd': { n = 34; sendResponse("Changed to pattern 34."); break; }
          }
          strip.ActivePattern = (pattern)n;
          break;
        }
        case '5': {
          uint8_t n;
          switch (mode) {
            case 'a': { n = 5; strip.RollingPattern(3); sendResponse("Changed to pattern 5."); break; }
            case 'b': { n = 15; sendResponse("Changed to pattern 15."); break; }
            case 'c': { n = 25; sendResponse("Changed to pattern 25."); break; }
            case 'd': { n = 35; sendResponse("Changed to pattern 35."); break; }
          }
          strip.ActivePattern = (pattern)n;
          break;
        }
        case '6': {
          uint8_t n;
          switch (mode) {
            case 'a': { n = 6; strip.RainbowRollingPattern(3); sendResponse("Changed to pattern 6."); break; }
            case 'b': { n = 16; sendResponse("Changed to pattern 16."); break; }
            case 'c': { n = 26; sendResponse("Changed to pattern 26."); break; }
            case 'd': { n = 36; sendResponse("Changed to pattern 36."); break; }
          }
          strip.ActivePattern = (pattern)n;
          break;
        }
        case '7': {
          uint8_t n;
          switch (mode) {
            case 'a': { n = 7; strip.SparklePattern(3); sendResponse("Changed to pattern 7."); break; }
            case 'b': { n = 17; sendResponse("Changed to pattern 17."); break; }
            case 'c': { n = 27; sendResponse("Changed to pattern 27."); break; }
            case 'd': { n = 37; sendResponse("Changed to pattern 37."); break; }
          }
          strip.ActivePattern = (pattern)n;
          break;
        }
        case '8': {
          uint8_t n;
          switch (mode) {
            case 'a': { n = 8; sendResponse("Changed to pattern 8."); break; }
            case 'b': { n = 18; sendResponse("Changed to pattern 18."); break; }
            case 'c': { n = 28; sendResponse("Changed to pattern 28."); break; }
            case 'd': { n = 38; sendResponse("Changed to pattern 38."); break; }
          }
          strip.ActivePattern = (pattern)n;
          break;
        }
        case '9': {
          uint8_t n;
          switch (mode) {
            case 'a': { n = 9; sendResponse("Changed to pattern 9."); break; }
            case 'b': { n = 19; sendResponse("Changed to pattern 19."); break; }
            case 'c': { n = 29; sendResponse("Changed to pattern 29."); break; }
            case 'd': { n = 39; sendResponse("Changed to pattern 39."); break; }
          }
          strip.ActivePattern = (pattern)n;
          break;
        }
      }
    } 
  }
  
  // Update the strip
  strip.Update();
}
//------END MAIN LOOP-----------------------------------END MAIN LOOP

//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// Strip completion callback
void StripComplete()
{
  if (strip.ReverseWhenComplete) {
    strip.Reverse();
  }
}

//a quick shortening of the random color function, just to reduce the pattern function calls more readable
uint32_t RC() {
  return strip.randColor();
}

void sendResponse(char const *response) {
    bleuart.write(response, strlen(response)*sizeof(char));
    bleuart.write("\n", strlen("\n")*sizeof(char));
}
