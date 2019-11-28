// CC-by-NC www.Electrosmash.com/rights
// Based on OpenMusicLabs previous works.
// pedalshield_mega_monitor.ino reads the hardware resources and send it to the serial port.

#include "U8glib.h"
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Display which does not send ACK

//defining hardware resources.
#define LED 13
#define FOOTSWITCH 12
#define TOGGLE 2
#define PUSHBUTTON_1 A5
#define PUSHBUTTON_2 A4

//defining the output PWM parameters
#define PWM_FREQ 0x00FF // pwm frequency - 31.3KHz
#define PWM_MODE 0 // Fast (1) or Phase Correct (0)
#define PWM_QTY 2 // 2 PWMs in parallel

//other variables
int input, vol_variable=512;
int counter=0;
byte ADC_low, ADC_high;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  //setup IO
  pinMode(FOOTSWITCH, INPUT_PULLUP);
  pinMode(TOGGLE, INPUT_PULLUP);
  pinMode(PUSHBUTTON_1, INPUT_PULLUP);
  pinMode(PUSHBUTTON_2, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(6, OUTPUT); //PWM0 as output
  pinMode(7, OUTPUT); //PWM1 as output
  
  // setup ADC
  ADMUX = 0x60; // left adjust, adc0, internal vcc
  ADCSRA = 0xe5; // turn on adc, ck/32, auto trigger
  ADCSRB = 0x00; // freeruning mode
  DIDR0 = 0x01; // turn off digital inputs for adc0

  // setup PWM
  TCCR4A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1)); //
  TCCR4B = ((PWM_MODE << 3) | 0x11); // ck/1
  TIMSK4 = 0x20; // interrupt on capture interrupt
  ICR4H = (PWM_FREQ >> 8);
  ICR4L = (PWM_FREQ & 0xff);
  DDRB |= ((PWM_QTY << 1) | 0x02); // turn on outputs
  sei(); // turn on interrupts - not really necessary with arduino
  }

void loop() 
{
  //Turn on the LED and write the OLED if the effect is ON.
  if (digitalRead(FOOTSWITCH)) 
  {
  digitalWrite(LED, HIGH); //light the LED
    u8g.firstPage(); 
 do {
   u8g.setFont(u8g_font_helvR10r);
   u8g.drawStr( 0, 12, "ADC: "); 
   u8g.setPrintPos(40, 12); u8g.print(input); 
   u8g.drawStr( 0, 24, "PUSH1: "); 
   u8g.setPrintPos(55, 24);
   u8g.print(digitalRead(PUSHBUTTON_1)); 
   u8g.drawStr( 0, 36, "PUSH2: "); 
   u8g.setPrintPos(55, 36);
   u8g.print(digitalRead(PUSHBUTTON_2)); 
   u8g.drawStr( 0, 48, "TOOGLE-SW: "); 
   u8g.setPrintPos(90, 48);
   u8g.print(digitalRead(TOGGLE)); 
   u8g.drawStr( 0, 60, "FOOT-SW: "); 
   u8g.setPrintPos(70, 60);
   u8g.print(digitalRead(FOOTSWITCH)); 
  } while( u8g.nextPage() );
  }
    else  
  {  
    digitalWrite(LED, LOW); // switch-off the LED
   }
}

ISR(TIMER4_CAPT_vect) 
{
  // get ADC data
  ADC_low = ADCL; // you need to fetch the low byte first
  ADC_high = ADCH;
  //construct the input sumple summing the ADC low and high byte.
  input = ((ADC_high << 8) | ADC_low) + 0x8000; // make a signed 16b value

//Using the OLED screen and the  serial port is not compatible, it will make the OLED refresh rate too slow.
//If you want to try the serial port, just uncomment the code below (but the screen wont work)

  //print on the serial port all the interesting variables:
  /*
  Serial.print(" ADCL: ");Serial.println(ADC_low);
  Serial.print(" ADCH: ");Serial.println(ADC_high);
  Serial.print(" guitar signal: ");Serial.println(input);
  Serial.print(" push-button_1: ");Serial.println(digitalRead(PUSHBUTTON_1));
  Serial.print(" push-button_2: ");Serial.println(digitalRead(PUSHBUTTON_2));
  Serial.print(" toggle switch: ");Serial.println(digitalRead(TOGGLE));
  Serial.print(" foot-switch  : ");Serial.println(digitalRead(FOOTSWITCH));
  */ 
  //print blank spaces to read the values better.
 /*  
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();  
  */
  
  //write the PWM signal
  OCR4AL = ((input + 0x8000) >> 8); // convert to unsigned, send out high byte
  OCR4BL = input; // send out low byte
}
