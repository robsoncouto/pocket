/*
 Robson Couto sept/2016 
 
 This code is based on ShendoXT implementation, check it out 
 https://github.com/ShendoXT/memcarduino
  _____________
 /             \       Pin connections:
|               |         1 - Data        -> Connect to pin 12 on Arduino (MISO)
|               |         2 - Command     -> Pin 11 of Arduino (MOSI)
|  MEMORY CARD  |         3 - 7.6V        -> 5V of Arduino
|               |         4 - GND         -> Gnd of Arduino
|               |         5 - 3.6V        -> 3.3 of Arduino
|_______________|         6 - Atention    -> Pin 10 of Arduino (CS)
|1 2|3 4 5|6 7 8|         7 - Clock       -> Pin 13 of Arduino (SCL) 
                          8 - Acknowledge -> Pin 2 of Arduino (INT0)

The acknowledge pin must be connected to a interrupt pin, so it is connected to pin 2.
The other pins use the SPI interface, if you have a different board than UNO,Nano,Mini or any 
other board that uses Atmega328, you may have to find the correct SPI pins for your board.
*/

#include<SPI.h>

int CS = 10;
byte adrHigh, adrLow;
int ACKpin = 2;

int volatile response = HIGH;
void ACK() {
  response = LOW;
}
byte transfer(byte data) {
  byte rx;
  response = HIGH;
  rx = SPI.transfer(data);
  int timeout = 2000;
  // if(rx==0x5C)timeout=2000;
  while (response == HIGH) {
    delayMicroseconds(1);
    timeout--;
    if (timeout == 0) return 0; //try again
  }
  return rx;
}
void readSector(unsigned int Adr) {
  digitalWrite(CS, LOW);
  transfer(0x81);
  transfer(0x52);
  transfer(0x00);
  transfer(0x00);
  transfer(Adr >> 8);
  transfer(Adr & 0xFF);
  transfer(0x00);
  transfer(0x00);
  transfer(0x00);
  transfer(0x00);
  for (int i = 0; i < 128; i++) {
    Serial.write(transfer(0x00));
  }
  transfer(0x00);
  SPI.transfer(0x00);
  digitalWrite(CS, HIGH);
  delayMicroseconds(500);
}

void writeSector(unsigned char MSB, unsigned char LSB){
  byte dataBuffer[128];
  byte response;
  for(int i=0;i<128;i++){
        while(Serial.available()==0);
        dataBuffer[i]=Serial.read();
  }
  byte CHK = MSB;
  CHK^=LSB;
  digitalWrite(CS, LOW);
  transfer(0x81);
  transfer(0x57);
  transfer(0x00);
  transfer(0x00);
  transfer(MSB);
  transfer(LSB);
  for (int i = 0; i < 128; i++)
  {
    transfer(dataBuffer[i]);
    CHK ^= dataBuffer[i];
  }
  transfer(CHK);
  transfer(0x00);
  transfer(0x00);
  response = SPI.transfer(0x00);//At least for the Pocket Station, the last byte never gives a ACK, so the tranfer method is not used here to agilise.
  digitalWrite(CS, HIGH);
  delayMicroseconds(500);
  if(response!='G')CHK=~CHK;//problem at writing sector, forces the PC to resend the sector by chaging the CHK
  Serial.write(CHK);
}

void setup() {

  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  pinMode(12, INPUT);
  digitalWrite(12, HIGH);
  //The memory card needs this SPI configuration: 
  SPI.setClockDivider(SPI_CLOCK_DIV64);
  SPI.setBitOrder(LSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.begin();
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH); 
  //delay(1000);
  attachInterrupt(digitalPinToInterrupt(ACKpin), ACK, FALLING);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0)
  {

    switch (Serial.read())
    {
      default:
        Serial.write('E');
        break;

      case 'R':
        delay(5);

        for (int j = 0; j < 1024; j++) { //read sectors
          readSector(j);
        }

        break;

      case 'W':
        while(Serial.available()<2);
        adrHigh=Serial.read();
        adrLow=Serial.read();
        writeSector(adrHigh,adrLow);
        break;
    }
  }
}

