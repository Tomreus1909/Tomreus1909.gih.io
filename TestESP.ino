#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

#define GPS_UPDATE  600000
//#define GPS_UPDATE  180000
#define GPS_TIMEOUT 120000

static const int RXPin = 14, TXPin = 12;

// The TinyGPS++ object
TinyGPSPlus gps;

SoftwareSerial ss(D5,D6);

//Define FirebaseESP8266 data object
FirebaseData firebaseData;

int state;
int pre_state = 0;
uint32_t start_time = 0;
bool debug = false;

void setup() {
//  pinMode(D0,OUTPUT);
//  pinMode(D1,OUTPUT);
  Serial.begin(9600);
  ss.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
//  WiFi.begin("Co&Com T1","20172018");
  WiFi.begin("Tom Reus","123456789");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
  }
  // Khởi tạo liên kết với Firebase
  Serial.println("WiFi connected");
//Firebase.begin("espclient-60eeb.firebaseio.com","bzz8anfRXXuM2DsxsD7VZ4oh0SDu4dYQc3fbPc5v"); //account Firebase để kết nối
  Firebase.begin("thang-51cf7.firebaseio.com","wpVMfpZ9ybA4Jr22OstucY3o6Moqy7BnUiWGDwMA"); //account Firebase để kết nối
  Firebase.reconnectWiFi(true);
  if (Firebase.getInt(firebaseData,"/status/Mode")) {
    pre_state = firebaseData.intData();
  }
}

void loop() {
  //10' Đọc dữ liệu GPS và giải mã
  if(millis() - start_time > GPS_UPDATE){
    start_time = millis();
    bool GPS_done = false;
    while (GPS_done==false){
//      byte gpsData = ss.read();
//      if(debug) {
//        Serial.write(gpsData);
//      }
      if(ss.available()>0) gps.encode(ss.read());
      yield();
//      delay(0);
      if (gps.location.isUpdated()){
        Firebase.setFloat(firebaseData,"/data/Lat",gps.location.lat());
        Firebase.setFloat(firebaseData,"/data/Long",gps.location.lng());
        if(debug){
          Serial.println();
          Serial.println("Lat: "+String(gps.location.lat()));
          Serial.println("Long: "+String(gps.location.lng()));
        }
        GPS_done = true;
        break;
      }
      //Sau 3' k kết nối được thì thoát vòng lặp
      if((millis() - start_time > GPS_TIMEOUT) && (GPS_done == false)) break;
    }
  }
  else{
    //Kiểm tra việc chuyển chế độ
    if (Firebase.getInt(firebaseData,"/status/Mode")) {
      state = firebaseData.intData();
      //Serial.print(state);
    }
    if(pre_state==1 && state==0){
      Serial.write('0');
      //digitalWrite(D1,LOW);
      pre_state = 0;
    }
    else if(pre_state==0 && state==1){
      Serial.write('1');
      //digitalWrite(D1,HIGH);
      pre_state = 1; 
    }
  
    if(Serial.available()){
      char c = Serial.read();
      if(c == 'X'){
        Firebase.setInt(firebaseData,"/status/Mode",1);
        //digitalWrite(D0,HIGH);
      }
      if(c == 'Y'){
        Firebase.setInt(firebaseData,"/status/Mode",0);
        //digitalWrite(D0,LOW);
      }
    }
  }
}
