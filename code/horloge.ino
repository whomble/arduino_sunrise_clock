#include <Wire.h>
#include <RTClib.h>
RTC_DS1307 RTC;
#include <IRremote.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <Arduino.h>
#include <TM1637Display.h>
#define CLK 6
#define DIO 3
TM1637Display display(CLK, DIO);
int RECV_PIN = 8;
#define PIN            5
#define NUMPIXELS      16
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
IRrecv irrecv(RECV_PIN);
decode_results results;
#define buz 9
uint8_t hor, mins, Sec;
int light;
int bright;
int brightOn;
int cligno;
int power1;
int power2;
int power3;
int powerProg1;
int powerProg2;
int powerProg3;
int rate = 0;
int deboutMin;
int deboutHeure;
int bouton;
int color;
int change;
int reglage;
int changeH;
int reglageReveil;
int debout;
bool buzz;
bool night;
int reveilGO;
int argh = 0;
unsigned long currentMillis;
unsigned long previousMillis;
unsigned long interval = 350;
uint8_t segto;
unsigned char ClockPoint = 1;

//diplay time on the 4*7segment display
void affichage(int hor, int mins, int Sec, int brightOn, int bright) {
  display.setBrightness(bright, brightOn);
  display.showNumberDec(hor / 10, false, 1, 0);
  display.showNumberDec(hor % 10, false, 1, 1);
  display.showNumberDec(mins / 10, false, 1, 2);
  display.showNumberDec(mins % 10, false, 1, 3);
}

/* this part of the code increase gradually red green and blue intensity to simulate sunrise
 *  I've choose 3 factor to match the true colors so there is 5 times more green than blue and 2 times more red than blue */
void allumerLedsProg(double rate) {
  for (int i = 0; i < rate / 2; i++)
  {
    powerProg1 = 5 * rate / 2;
    powerProg2 = 2 * rate / 2;
    powerProg3 = 1 * rate / 2;
    pixels.setPixelColor(i, pixels.Color(powerProg1, powerProg2, powerProg3));
    pixels.show();
  }
}
int lumiereAmbiance(int bouton) {

// switching on and off the lights using ether the remote or the push button

  if (bouton == 42 && light == 0) {
    light = 1 ;
    bouton = 999;
  }
  if (bouton == 42 or buzz == 0 && light == 2) {
    light = 0;
    bouton = 999;
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show();
    }
  }
  if (light == 1) {

    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(power1, power2, power3));
      pixels.show();
    }
    light = 2;
  }
  
  //allow to increase or decrease gradually each color for 0 to 255 (0,1,3,7,15,31,63,127,255)
  
  if (light == 2 && bouton < 7 && bouton > 0) {
    switch (bouton) {
      case 1:
        power1 = ((power1 + 1) * 2) - 1;
        if (power1 > 255) {
          power1 = 255;
        }
        break;
      case 4:
        power1 = ((power1 + 1) / 2) - 1; /* mod 256, -= 5 */
        if (power1 < 0) {
          power1 = 0;
        }
        break;
      case 2:
        power2 = ((power2 + 1) * 2) - 1;
        if (power2 > 255) {
          power2 = 255;
        }
        break;
      case 5:
        power2 = ((power2 + 1) / 2) - 1;
        if (power2 < 0) {
          power2 = 0;
        }
        break;
      case 3:
        power3 = ((power3 + 1) * 2) - 1;
        if (power3 > 255) {
          power3 = 255;
        }
        break;
      case 6:
        power3 = ((power3 + 1) / 2) - 1;
        if (power3 < 0) {
          power3 = 0;
        }
        break;
      default:
        break;
    }
    light = 1;
    bouton = 999;
  }
  return bouton;
}

//if you want to add a buzzer to the device
void buzzer(int dring) {
}

// compare the current time with the alarm

void wakeup(int hor, int mins, int buzz) {
  if (debout == 1) {
    if (hor == deboutHeure && mins == deboutMin) {
      reveilGO = 1;
    }
    if (rate == 31 or buzz == 0) { // switch off the ligths after 30 mins or if the push button has been activated
      for (int i = 0; i < NUMPIXELS; i++)
      {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        pixels.show();
      }
      reveilGO = 0;
      rate = 0;
      buzzer(0);
    }
    if (reveilGO == 1) { // rate is use to define the sunrise speed 
      rate = mins - deboutMin;
      if (rate < 0) {
        rate = rate + 60;
      }
      allumerLedsProg(rate);//if you want the sun to rise in 15 mins instead of 30 you just have to add a *2 before rate
      if (rate == 30) {
        buzzer(1);
      }
    }
  }
}

//Recieve and harvest the data from IR sensor and associate each button to a value, to find your remote value, just use the IR Remote exemple and replace the hex data
int IR() {
  if (irrecv.decode(&results)) {
    irrecv.resume();
    switch (results.value % 0x10000) {
      case 0x6897:
        bouton = 0;
        break;
      case 0x30CF:
        bouton = 1;
        break;
      case 0x18E7:
        bouton = 2;
        break;
      case 0x7A85:
        bouton = 3;
        break;
      case 0x10EF:
        bouton = 4;
        break;
      case 0x38C7:
        bouton = 5;
        break;
      case 0x5AA5:
        bouton = 6;
        break;
      case 0x42BD:
        bouton = 7;
        break;
      case 0x4AB5:
        bouton = 8;
        break;
      case 0x52AD:
        bouton = 9;
        break;
      case 0xA25D:
        bouton = 42; //off
        break;
      case 0x629D:
        bouton = 74; //vol+
        break;
      case 0xA857:
        bouton = 38; //vol-
        break;
      case 0x9867:
        bouton = 714; //EQ
        break;
      case 0xC23D:
        reglage = 1; //>>
        break;
      case 0x22DD:
        reglage = 0; //<<
        break;
      case 0xB04F:
        bouton = 333; //st rept
        break;
      case 0x02FD:
        bouton = 222; //st rept
        break;
      default:
        break;
    }
  }
  return bouton;
}

//Set time, the << and >> button on my remote alow to switch from min to hour and the +/- increment or decrement the value, feel free to just have a button for each function

void times(int hor, int mins, int changeH) {
  if (changeH == 1) {
    if (reglage == 0) {
      if (bouton == 74)
        hor++;
      else if (bouton == 38)
        hor += 23;
      hor %= 24;
    }
    else {
      if (bouton == 74)
        mins++;
      else if (bouton == 38)
        mins--;
      if (mins == 60) {
        hor++;
        hor %= 24;
        mins = 0;
      }
      else if (mins == -1) {
        hor += 23;
        hor %= 24;
        mins = 59;
      }
    }
    RTC.adjust(DateTime(2017, 9, 17, hor, mins, 0)); //Save the value to the RTC, since the seconds and the date isn't display on my clock I've set a random date
  }
}

// same as befor but for the alarm clock

int setReveil(int reglageReveil) {
  if (reglageReveil == 1) {
    if (reglage == 0) {
      if (bouton == 74)
        deboutHeure++;
      else if (bouton == 38)
        deboutHeure += 23;
      deboutHeure %= 24;
    }
    else {
      if (bouton == 74)
        deboutMin++;
      else if (bouton == 38)
        deboutMin--;
      if (deboutMin == 60) {
        deboutHeure++;
        deboutHeure %= 24;
        deboutMin = 0;
      }
      else if (deboutMin == -1) {
        deboutHeure += 23;
        deboutHeure %= 24;
        deboutMin = 59;
      }
    }
  }
  return (999);
}

//Switch of the display at night
int nuit (int hor, int brightOn) {
  if ((22 <= hor or hor <= 10) && night == 0) {
    brightOn = 0;
    night = 1;
  }
  else if ((10 < hor and hor < 22) && night == 1)
  {
    brightOn = 1;
    night = 0;
  }
  return  brightOn;
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");
  Wire.begin();
  RTC.begin();
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  if (!RTC.isrunning())
  {
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  RTC.adjust(DateTime(__DATE__, __TIME__));
  for (int i = 0; i < NUMPIXELS; i++)

  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.show();
  }
  power1 = 0;
  power2 = 0;
  power3 = 0;
  powerProg1 = 0;
  powerProg2 = 0;
  powerProg3 = 0;
  light = 0;
  change = 0;
  color = 1;
  reglage = 0;
  changeH = 0;
  deboutHeure = 0;
  deboutMin = 0;
  reglageReveil = 0;
  reveilGO = 0;
  debout = 0;
  pinMode (7, INPUT_PULLUP);
  pinMode(9, OUTPUT);
}
void loop()
{
  // Read time from the real time clock
  DateTime now = RTC.now();
  Sec = now.second();
  hor = now.hour();
  mins = now.minute();
  buzz = digitalRead (7);
  
  bouton = IR();// detect if the remote is used and read the value and call the function to set time
  if (bouton == 714) {
    if (changeH == 0) {
      reglageReveil = 0;
      changeH = 1;
    }
    else if (changeH == 1) {
      changeH = 0;
    }
  }
  times(hor, mins, changeH);
  if (bouton == 333) {
    if (reglageReveil == 0) {
      reglageReveil = 1;
      changeH = 0;
    }
    else if (reglageReveil == 1) {
      reglageReveil = 0;
    }
  }
  // if we change the time, the display blink
  if (reglageReveil == 0) {
    if (changeH == 1) {
      currentMillis = millis();
      if ( abs(currentMillis - previousMillis) >= interval) {
        previousMillis = currentMillis;
        if (cligno == 1) {
          affichage(hor, mins, Sec, 1, 1);
          cligno = 0;
          interval = 500;
        }
        else {
          affichage(hor, mins, Sec, 1, bright);
          cligno = 1;
          interval = 500;
        }
      }
    }
    else if (changeH == 0) {

      affichage(hor, mins, Sec, brightOn, bright);
    }
  }
  //same for the alarm clock
  else if (reglageReveil == 1) {
    currentMillis = millis();
    if (abs(currentMillis - previousMillis) >= interval) {
      previousMillis = currentMillis;
      if (cligno == 1) {
        affichage(deboutHeure, deboutMin, 1, 1, 1);
        cligno = 0;
        interval = 500;
      }
      else {
        affichage(deboutHeure, deboutMin, 1, 1, bright);
        cligno = 1;
        interval = 500;
      }
    }
  }
  // enable/disable the alarm clock ( the clock blink one time in red when off and in green when on)
  if (bouton == 222) {
    if (debout == 0) {
      debout = 1;

      for (int i = 0; i < NUMPIXELS; i++)
      {
        pixels.setPixelColor(i, pixels.Color(0, 50, 0));
        pixels.show();
      }
    }
    else if (debout == 1) {
      debout = 0;

      for (int i = 0; i < NUMPIXELS; i++)
      {
        pixels.setPixelColor(i, pixels.Color(50, 0, 0));
        pixels.show();
      }

    }
    delay(200);

    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show();
    }
  }

  // decrease brightness during night
  if (22 <= hor or hor <= 10) {
    bright = 1;
  }
  else if (10 < hor < 22)
  {
    bright = 7;
  }
  if (buzz == 0) {
    brightOn = 1 - brightOn;
    delay(100);
  }
  bouton = setReveil(reglageReveil);
  bouton = IR();
  bouton = lumiereAmbiance(bouton);
  brightOn = nuit (hor, brightOn);
  wakeup(hor, mins, buzz);
}
