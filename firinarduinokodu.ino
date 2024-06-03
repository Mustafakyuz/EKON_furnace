//Kütüphaneler ekleniyor:******************************
#include<Nextion.h>
#include "max6675.h"
//*****************************************************

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
//#define buton 8 //dijital çıkıştan deger okuma testi için
//Bu Arduino Uno için nextion serial haberleşmesi komutu
//#include<SoftwareSerial.h>
//SoftwareSerial HMISerial(10,11);
//******************************************************

// Tanımlamalar Yapılıyor:******************************
int x=0;
uint32_t dualstate=0;
int okunan_deger = 0;
float sicaklik_gerilim = 0;
int sicaklik = 0; 
float periyot=60;
uint32_t set_deger= 70;
float ust_band=30;
float alt_band=30;
float ust_seviye=set_deger+ust_band;
float alt_seviye=set_deger-alt_band;
float olc_deger=1;
float role_pasif, role_aktif;
//*******************************************************

//Fonksiyonlar ekleniyor:*****************************
NexDSButton bt4=NexDSButton(0, 24, "bt4"); 
NexNumber termoust1=NexNumber(0, 26, "termoust1");
NexNumber n1=NexNumber(0, 12, "n1");
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
NexTouch *nex_listen_list[] = 
{
   &termoust1,
   &bt4,
   &n1,
   NULL
};

void bt4PopCallback(void *ptr)
{
     uint32_t dualstate=0;
     Serial.println("buton");
    bt4.getValue(&dualstate);
     Serial.println("buton2");
    if(dualstate==1) {
      digitalWrite(48,HIGH);   
      delay(100);
  }
    else {
      digitalWrite(48,LOW);
      delay(200);
  }
    Serial.println(dualstate);
}
//*****************************************************


void setup() {
   //Nextion setup:****************************
   dbSerialPrintln("setup begin");
   Serial.begin(9600);
   nexInit();
   bt4.attachPop(bt4PopCallback);
   dbSerialPrintln("setup done");
   //*********************************
   pinMode(28,OUTPUT); //pinler setup
   pinMode(8,INPUT);

  
   //Timer setup: ******************************
   cli();
   TCCR1A =0;
   TCCR1B =0;
   TCNT1 = 0;
   OCR1A = 15624;
   TCCR1B |=(1<< WGM12);
   TCCR1B |=(1<< CS12)| (1<< CS10);
   TIMSK1 |=(1<<OCIE1A);
   sei();
   //********************************************* 
   olc_deger = thermocouple.readCelsius(); // İlk çalıştığı anda setup yapılırken thermocouple dan deger okuyor.  
   
}

//Timer Fonksiyonu
ISR(TIMER1_COMPA_vect)
{ 
      // Resistans rölelerinin çalışma sürelerinin hesaplanması: *******
      role_pasif=((olc_deger-alt_seviye)*periyot) / (ust_band+alt_band);
      role_aktif=periyot-role_pasif; 
      //****************************************************************
   bt4.getValue(&dualstate);
   x++; //60 saniyede bir sıfırlıcak şekilde her saniyede 1 artıyor.
   if(dualstate==1) {
       if(x<60) {
          if(olc_deger<alt_seviye) {
              digitalWrite(28,LOW);
              Serial.println("Olculen deger az ısınıyor.");
              Serial.println(olc_deger);
          }
          else {
              if(x<=role_aktif) {
                  digitalWrite(28,LOW);
                  Serial.println("Olculen deger yerinde ısınıyor.");
                  Serial.println(olc_deger);
              }
              else {
                  digitalWrite(28,HIGH);
                  Serial.println("Olculen deger yerinde ısınMIYOR.");
                  Serial.println(olc_deger);
              }   
          }
       }
       else if(x==60) {
          x=-1;
          olc_deger = thermocouple.readCelsius();
          Serial.println("--------Periyodik sicaklik--------");
          Serial.println(olc_deger);
          
       }
       else {
          x=-1;
       }
    }
    else
      digitalWrite(28,HIGH);
      
  }
//*************************************************************************

//Ana döngü loop fonksiyonu
void loop() {

  sicaklik=thermocouple.readCelsius();
  termoust1.setValue(sicaklik); 
//**************************************************** 
//    dualstate = digitalRead(buton);
// Dualstate buton için test  kodu 
    
    n1.getValue(&set_deger);
    Serial.println("Set deger:");
    Serial.println(set_deger);
   
    if(dualstate==1) {
        digitalWrite(48,HIGH);   
        delay(100);
    }
    else {
        digitalWrite(48,LOW);
       delay(200);
    }
  
    nexLoop(nex_listen_list); 
  
}
