#include <EEPROM.h>

#define Clockspeed 16000000
#define Prescaler_Timer1 8 
#define Prescaler_Timer0 1024 
#define Button_Down 2 //S1
#define Button_Up 4 //S2
#define Button_Menu 8 //S3
#define LED_RED 1 
#define LED_GREEN 0
#define Timer1_Out 10 // 16bit LED
#define Timer0_Out 6 // 8bit Coil
#define EEPROM_Set_Bit 0
#define EEPROM_Freq_LED_Value 1
#define EEPROM_On_LED_Value 2
#define EEPROM_Freq_Coil_Value 3

int Button_State[10];
//Todo eeprom und failsafe bei timer overflow

int Timer0_FreqInHz , Timer1_FreqInHz, Timer1_OnInProzent;
int Timer1_FreqInBit, Timer1_OnTimeInBit, Timer0_FreqInBit;
int Menu=0; // 0 Coil Freq; 1 LED Freq; 2 LED Pulsewidth

void setup() {
  if(EEPROM.read(EEPROM_Set_Bit) == 0)
  {
    EEPROM.write(EEPROM_Set_Bit,1);
    EEPROM.write(EEPROM_Freq_Coil_Value,100);
    EEPROM.write(EEPROM_Freq_LED_Value,100);
    EEPROM.write(EEPROM_On_LED_Value,50);
  }
  
  Timer0_FreqInHz = EEPROM.read(EEPROM_Freq_Coil_Value);
  Timer1_FreqInHz = EEPROM.read(EEPROM_Freq_LED_Value);
  Timer1_OnInProzent = EEPROM.read(EEPROM_On_LED_Value);
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(Timer1_Out, OUTPUT);
  pinMode(Timer0_Out, OUTPUT);
  pinMode(Button_Menu, INPUT);
  pinMode(Button_Up, INPUT);
  pinMode(Button_Down, INPUT);
  

  
  
  TCCR1A = _BV(COM1A0) | _BV(COM1B1) | _BV(WGM10) | _BV(WGM11); //nicht invertiert
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); //8 prescaler, Fast PWM OCR1A Top OC1B Output


  //Setup Timer0 8bit Coil Setup
  TCCR0A = _BV(WGM01)| _BV(WGM00)| _BV(COM0A0)| _BV(COM0B0); //Fast PWM mit OCRA TOP mit WGM02=1 Pegelwechsel an OC0A bei Compare-Match // OC0B kann in diesem modus nicht verwendet werden 

  //Prescaler
  TCCR0B = _BV(CS00)| _BV(CS02)| _BV(WGM02); //1024 prescaler

  Set_Timer();
  LED(Menu);
}

void loop() {
  if (Button_Klick(Button_Menu) == 1){
    if (Menu < 2)
    {
      Menu++;
      LED(Menu); // Menu 0 = Red/Coil Freq ; Menu 1 = Green/Led Freq; Menu 2 = Orange/LED On
    }
    else
    {
      Menu = 0;
      LED(Menu);
    } 
  }
  if (Button_Klick(Button_Up) == 1){
    switch (Menu){
      case 0:
      Timer0_FreqInHz++;
      //EEPROM.update(EEPROM_Freq_Coil_Value, Timer0_FreqInHz);
      break;
      case 1:
      Timer1_FreqInHz++;
      //EEPROM.update(EEPROM_Freq_LED_Value, Timer1_FreqInHz);
      break;
      case 2:
      Timer1_OnInProzent++;
      //EEPROM.update(EEPROM_On_LED_Value, Timer1_OnInProzent);
    }
    Set_Timer();
  }
  if (Button_Klick(Button_Down) == 1){
    switch (Menu){
      case 0:
      Timer0_FreqInHz--;
      //EEPROM.update(EEPROM_Freq_Coil_Value, Timer0_FreqInHz);
      break;
      case 1:
      Timer1_FreqInHz--;
      //EEPROM.update(EEPROM_Freq_LED_Value, Timer1_FreqInHz);
      break;
      case 2:
      Timer1_OnInProzent--;
      //EEPROM.update(EEPROM_On_LED_Value, Timer1_OnInProzent);
    }
    Set_Timer();
  }   
} 

int Button_Klick(int Button){
  int Button_Now = digitalRead(Button);
  if(Button_Now != Button_State[Button])
  {
    Button_State[Button]=Button_Now;
    return 1;
  }
  else
  {
    Button_State[Button]=Button_Now;
    return 0;
  }
 
}

void LED(int Number){
  switch (Number){
    case 0:
    digitalWrite(LED_RED,1);
    digitalWrite(LED_GREEN,0);
    break;
    case 1:
    digitalWrite(LED_RED,0);
    digitalWrite(LED_GREEN,1);
    break;
    case 2:
    digitalWrite(LED_RED,1);
    digitalWrite(LED_GREEN,1);
    break;
  }
}

void Set_Timer (void)
{
    Timer1_FreqInBit = (Clockspeed / (Prescaler_Timer1 * Timer1_FreqInHz))-1 ;
    Timer1_OnTimeInBit = Timer1_FreqInBit /(100/Timer1_OnInProzent);
    Timer0_FreqInBit = ((Clockspeed / Prescaler_Timer0)/(Timer0_FreqInHz*2))-1;
    OCR0A = Timer0_FreqInBit;
    OCR1A = Timer1_FreqInBit;
    OCR1B = Timer1_OnTimeInBit;
}
