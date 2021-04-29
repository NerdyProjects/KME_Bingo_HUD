#include <U8x8lib.h>
#include <avr/sleep.h>
#include <avr/power.h>


U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
unsigned long nextCycle;
uint8_t ignition;

void setup() {
  u8x8.begin();
  u8x8.setBusClock(400000);
  u8x8.setContrast(2);
  Serial.begin(9600);
  Serial.setTimeout(100);
  nextCycle = millis();
}

void updateLambda(uint8_t voltage)
{
  static uint8_t last;
  uint8_t symbol = 157;
  uint8_t v;
  if (voltage > 46) {
    symbol = 141;
    v = 15;
  } else {
    v = voltage / 3;
  }
    
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  u8x8.drawGlyph(v, 6, symbol);
  if(last != v) {
    u8x8.drawGlyph(last, 6, ' ');
  }
  u8x8.setCursor(4, 7);
  u8x8.print(voltage * 5.1f/255.0f); // 0.75
  last = v;
}

void updateRpm(uint16_t rpm_val)
{
  uint16_t rpm = 0;
  if (rpm_val != 0) {
    rpm = 15000000 / rpm_val;
  }
  u8x8.setFont(u8x8_font_inb33_3x6_n);
  u8x8.setCursor(0, 0);
  if (rpm < 100) {
    u8x8.print("000");
  } else if (rpm < 1000) {
    u8x8.print ("0");
  }
  u8x8.print(rpm);
}

void drawUintPadding(uint8_t x, uint8_t y, uint8_t v)
{
  u8x8.drawGlyph(x, y, v / 100);
  u8x8.drawGlyph(x, y, (v % 100) / 10);
  u8x8.drawGlyph(x, y, v % 10);
}

void updateStepper(uint8_t v)
{
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  drawUintPadding(0, 7, v);
}

void updateIgnitionState(uint8_t v)
{
  ignition = v & (1 << 3);
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  if(ignition) {
    u8x8.drawGlyph(9, 7, 'I');
  } else {
    u8x8.drawGlyph(9, 7, 'O');
  }
}

void updateLpgState(uint8_t v)
{
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  if(v & (1)) {
    u8x8.drawGlyph(11, 7, 'L');
  } else {
    u8x8.drawGlyph(11, 7, 'P');
  }
}

uint8_t requestStatus()
{
  const uint8_t request[] =  { 0x65, 0x02, 0x02, 0x69 };
  uint8_t response[11];
  uint8_t responseBytes;
  Serial.write(request, 4);
  responseBytes = Serial.readBytes(response, 11);
  if (responseBytes != 11) {
    u8x8.clear();
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
    u8x8.print("Comm Err L:");
    u8x8.print(responseBytes, DEC);
  } else {
    uint8_t checksum = 0;
    for(uint8_t i = 0; i < 10; ++i) {
      checksum += response[i];
    }
    if (response[0] != 0x65) {
      u8x8.clear();
      u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
      u8x8.print("Comm Err 0:");
      u8x8.print(response[0], HEX);
    } else if (checksum != response[10]) {
      u8x8.clear();
      u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
      u8x8.print("Comm Err C:");
      u8x8.print(response[10], HEX);
      u8x8.setCursor(0, 1);
      u8x8.print(response[10], HEX);
    } else {
      // Data okay.
      // TPS voltage, Lambda voltage, Stepper pos, IAP, RPM, RPM, lambda/TPS/Ignition (3) state, LPG (0)/cutoff(3) state, engine temp
      updateRpm((response[5] | response[6] << 8));
      updateLambda(response[2]);
      updateStepper(response[3]);
      updateIgnitionState(response[7]);
      updateLpgState(response[8]);
      return 0;
    }
  }  
}

void loop() {
  uint8_t c;
  if(c = requestStatus()) {
    
  }
  
  /*u8x8.setCursor(0, 5);
  u8x8.print("AP: 230, IGN");
  u8x8.setCursor(0, 6);
  u8x8.clearLine(6);
  u8x8.print("DBG blabla");
  u8x8.setCursor(0, 7);
  u8x8.print("DBG blabla");
  //u8x8.setCursor(0, 4);
  //u8x8.print("(Bat: ");
  /* 910k to Input, 91k to GND. So Uraw=11*U; Uref=1.1V */
  //u8x8.print(analogRead(A0) * 11 * 1100 / 1024);
  //u8x8.print(" mV)    ");  
  nextCycle += 200UL;
  if (!ignition) {
    nextCycle += 4800UL;
  }
  while(millis() < nextCycle) {
    enter_sleep();
  }
  u8x8.setPowerSave(!ignition);  
}

void enter_sleep(void)  
  {  
  set_sleep_mode(SLEEP_MODE_IDLE);  
  sleep_enable();
  power_adc_disable();    /* Analog-Eingaenge abschalten */
  power_spi_disable();    /* SPI abschalten */
  power_timer1_disable(); /* Timer0 abschalten */
  power_timer2_disable(); /* Timer0 abschalten */
  power_twi_disable();    /* TWI abschalten */
  sleep_mode();  
  /** Das Programm läuft ab hier nach dem Aufwachen weiter. **/  
  /** Es wird immer zuerst der Schlafmodus disabled.        **/  
  sleep_disable();   
  power_all_enable();     /* Komponenten wieder aktivieren */
  }  
