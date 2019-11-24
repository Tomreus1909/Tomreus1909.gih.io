//10/07/2019-----
#include <EEPROM.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <TimerOne.h>
static volatile unsigned long khoang_cach;
const int trig = 8; //chân trig của HC-SR04
const int echo = 7; //chân echo của HC-SR04

byte ngon_tay_cai_6 = 6; //ngón tay cái - thumb
byte ngon_tay_ut_12 = 12; //ngón tay út - little finger
//byte relay = 4; //relay 3v3a
byte coi_chip_10 = 10; //buzzer
byte led_11 = 11; //led
byte relay = 4; //relay 3v3a
byte opto_volt_pin_5 = 5; //opto pc817
byte do_volt_out_A1 = A1; //Check whether the charger is charging or not
byte do_volt_pin_A2 = A2; //Measure the battery voltage
byte led_mode = 13;

byte rung_cai = 0;
byte rung_ut = 0;
byte rung_cai_1 = 0;
byte rung_cai_2 = 0;
byte rung_ut_1 = 0;
byte rung_ut_2 = 0;
bool che_do_rung = 0;
byte loa1 = 0;
byte loa2 = 0;
byte den1 = 0;
byte den2 = 0;
char mode = '0';

int dovolt_out = 0;
float volt_out = 0;
unsigned long pe_rung = 0;
unsigned long pe_rung_cai_1 = 0;
unsigned long pe_rung_cai_2 = 0;
unsigned long pe_rung_ut_1 = 0;
unsigned long pe_rung_ut_2 = 0;
unsigned long pe_loa1 = 0;
unsigned long pe_loa2 = 0;
unsigned long pe11 = 0;
unsigned long pe12 = 0;

static const uint32_t GPSBaud = 9600;
static const int RXPin = 13, TXPin = 12;

void getDistance(){
  digitalWrite(trig, LOW);
  delayMicroseconds(5);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  unsigned long tgian = pulseIn(echo,HIGH,30000);
  khoang_cach =  0.0343 * tgian / 2;
  Serial.print("Khoang cach hien tai: ");
  Serial.println(khoang_cach);
}

void setup()
{   

    attachInterrupt(digitalPinToInterrupt(3),ngat1,FALLING);
    Timer1.initialize(300000);  //0.3s
    Timer1.attachInterrupt(getDistance);
    Serial.begin(9600);     // giao tiếp Serial với baudrate 9600
    //ss.begin(GPSBaud);
    pinMode(trig, OUTPUT); // chân trig sẽ phát tín hiệu
    pinMode(coi_chip_10, OUTPUT);
    digitalWrite(coi_chip_10, HIGH);
    delay(500);
    digitalWrite(coi_chip_10, LOW);
    pinMode(ngon_tay_cai_6, OUTPUT);
    pinMode(ngon_tay_ut_12, OUTPUT);
    pinMode(echo, INPUT); // chân echo sẽ nhận tín hiệu
    pinMode(3,INPUT_PULLUP);
    delay(600);
    che_do_rung = EEPROM.read(13);
    delay(200);
    if ((che_do_rung != 1) && (che_do_rung != 0)) {
        delay(1000);
        EEPROM.write(13, 0);
        delay(1000);
        che_do_rung = EEPROM.read(13);
    } //quan trọng, chỉ dùng 1 lần - Only run once
//
}
void ngat()
{
  digitalWrite(ngon_tay_cai_6, 0);
  digitalWrite(ngon_tay_ut_12, 0);
  digitalWrite(coi_chip_10, LOW);
  
}
void ngat1()
{
  che_do_rung=!che_do_rung;
  EEPROM.write(13, che_do_rung);
  che_do_rung=EEPROM.read(13);
  //Chế độ rung
  if(che_do_rung==0){
    Serial.print('X');
    digitalWrite(led_mode,HIGH);
  }
  //Chế độ âm thanh
  else{
    Serial.print('Y');
    digitalWrite(led_mode,LOW);
  }
}


void sacPin(unsigned long ms)
{
    unsigned long start_sac = millis();// thời gian tính từ lúc bật máy đến khi cắm sạc(ms)
    while ((millis() - start_sac <= ms) && (volt_out >= 3.5)) // khi thời gian sạc chưa đủ và volt_out > = 3.5V thì vẫn sạc
    {
        //đo volt ngoài
        dovolt_out = analogRead(do_volt_out_A1);
        volt_out = dovolt_out / 1023.0 * 5.0;
        //nháy đèn
        if ((den1 == 0) && (millis() - pe12 >= 1000)) {
            pe11 = millis();
            digitalWrite(led_11, HIGH);
            den1 = 1;
            den2 = 0;
        }
        if ((den2 == 0) && (millis() - pe11 >= 1000)) {
            pe12 = millis();
            digitalWrite(led_11, LOW);
            den2 = 1;
            den1 = 0;
        }
            digitalWrite(led_11,1);
            delay(1000);
            digitalWrite(11,0);
            delay(1000);
    }
    digitalWrite(led_11, HIGH);
}

void loop()
{
  //đo volt ngoài - Check whether the charger is charging or not
     dovolt_out = analogRead(do_volt_out_A1);
     volt_out = dovolt_out / 1023.0 * 5.0;
     digitalWrite(opto_volt_pin_5, LOW);
    
    //chế độ sạc pin - Battery charging mode
     while ((volt_out >= 3.5) ) {
        digitalWrite(coi_chip_10, LOW);
        analogWrite(ngon_tay_cai_6, 0);
        analogWrite(ngon_tay_ut_12, 0);
        sacPin(360000);

        //đo volt ngoài - Check whether the charger is charging or not
        dovolt_out = analogRead(do_volt_out_A1);
        volt_out = dovolt_out / 1023.0 * 5.0;
        digitalWrite(relay, HIGH);
        digitalWrite(opto_volt_pin_5, HIGH);
        delay(50);
        //đo volt pin - Measure the battery voltage
        int dovolt_pin = analogRead(do_volt_pin_A2);
        float volt_pin = dovolt_pin / 1023.0 * 5.0;
        if (volt_pin <= 3.3) {
            digitalWrite(relay, HIGH);// pin được nối với chân chờ hở của adapter để chuẩn bị sạc
            sacPin(3600000);
        }
        if(volt_pin >=4){
            digitalWrite(led_11, HIGH);
            digitalWrite(relay,LOW);
        }//full battery
    }
    //chế độ không sạc pin - No battery charging mode
    if (volt_out < 3.5) {
        //Kiểm tra có nhận được dữ liệu điều khiển từ ESP hay k
        if(Serial.available()){  
          char mode = Serial.read();
          if(mode=='0') che_do_rung = 0;
          else if(mode=='1') che_do_rung = 1;
        }
        delay(10);
        if (che_do_rung == 1) {
            //delay(300);
            EEPROM.write(13, 1);
            delay(300);
        }
        else {
            //delay(300);
            EEPROM.write(13, 0);
            delay(300);
        }
        che_do_rung = EEPROM.read(13);
     
      
        if (khoang_cach <= 20) {
            rung_cai = 0;
            rung_ut = 1;
            pe_rung = millis();
        }
        if ((khoang_cach > 20) && (khoang_cach <= 50)) {
            rung_cai = 1;
            rung_ut = 0;
            pe_rung = millis();
        }
        if(khoang_cach>50) {
            if (millis() - pe_rung >= 1000) {
                rung_cai = 0;
                rung_ut = 0;
            }
        }

        //chế độ rung ngón tay - finger vibration mode
        if (che_do_rung == 0) {
            
                digitalWrite(coi_chip_10, LOW);
            
            // rung nhẹ ngón tay cái - Motor vibration on the thumb
            if (rung_cai == 1) {
                if ((rung_cai_1 == 0) && (millis() - pe_rung_cai_2 >= 100)) {
                    pe_rung_cai_1 = millis();
                    analogWrite(ngon_tay_cai_6, 200);
                    rung_cai_1 = 1;
                    rung_cai_2 = 0;
                } //rung có nhịp
                if ((rung_cai_2 == 0) && (millis() - pe_rung_cai_1 >= 300)) {
                    pe_rung_cai_2 = millis();
                    analogWrite(ngon_tay_cai_6, 0);
                    rung_cai_2 = 1;
                    rung_cai_1 = 0;
                }
            }
            else {
                analogWrite(ngon_tay_cai_6, 0);
            }
            // rung nhẹ ngón tay út - Vibration motor on the little finger
            if (rung_ut == 1) {
                if ((rung_ut_1 == 0) && (millis() - pe_rung_ut_2 >= 100)) {
                    pe_rung_ut_1 = millis();
                    analogWrite(ngon_tay_ut_12, 200);
                    rung_ut_1 = 1;
                    rung_ut_2 = 0;
                } //rung có nhịp - Vibration motor feel comfortable
                if ((rung_ut_2 == 0) && (millis() - pe_rung_ut_1 >= 300)) {
                    pe_rung_ut_2 = millis();
                    analogWrite(ngon_tay_ut_12, 0);
                    rung_ut_2 = 1;
                    rung_ut_1 = 0;
                }
            }
              else {
                analogWrite(ngon_tay_ut_12, 0);
            }
        }
        //chế độ phát loa, nháy loa nhanh dần khi khoảng cách giảm dần - Sound mode
        else {
            
            analogWrite(ngon_tay_cai_6, 0);
            analogWrite(ngon_tay_ut_12, 0);
        if (20< khoang_cach&&khoang_cach <= 50) {               
                    if ((loa1 == 0) && (millis() - pe_loa2 >= 500)) {
                    pe_loa1 = millis();
                    digitalWrite(coi_chip_10, HIGH);
                    loa1 = 1;
                    loa2 = 0;
                }
                if ((loa2 == 0) && (millis() - pe_loa1 >= 500)) {
                    pe_loa2 = millis();
                    digitalWrite(coi_chip_10, LOW);
                    loa2 = 1;
                    loa1 = 0;
                }
            }
         if (khoang_cach<=20){
             if ((loa1 == 0) && (millis() - pe_loa2 >= 100)) {
                pe_loa1 = millis();
                digitalWrite(coi_chip_10, HIGH);
                loa1 = 1;
                loa2 = 0;
            }
            if ((loa2 == 0) && (millis() - pe_loa1 >= 100)) {
                pe_loa2 = millis();
                digitalWrite(coi_chip_10, LOW);
                loa2 = 1;
                loa1 = 0;
            }
                  }
        if (khoang_cach >50)
        {
          digitalWrite(coi_chip_10, LOW);
        }
    }
    }
}
