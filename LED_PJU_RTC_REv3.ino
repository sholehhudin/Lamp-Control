/* LED PJU
 * s.alayubi
 * sholehhudin.alayubi@gmail.com
 * last edited : 13 April 16
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include "RTClib.h"

#define Device "Lamp" 
#define PIN_PWM 16
#define PIN_Relay 14
#define PERIODE_PROSES 1 // *10detik
#define DELTA_PERIODE_PROSES 0 //*10detik
#define DEBUG true

//manipulating type of chip ID
static char Chip_ID[20];
String chip = String(ESP.getChipId());

// for wifi network
const char* ssid ="Lab Iot";
const char* password ="labiot2014";

//credential for MQTT
const char* usernameMQTT ="";
const char* passwordMQTT ="";
const char* broker = "192.168.1.55";
unsigned int port = 1883;
char topik[40];
char topik_SUB[40];
char topik_SUB_COMP1[40];
String topik_SUB_COMPARASI_Nilai="";
String topik_SUB_COMPARASI_Waktu="";
String topik_SUB_COMPARASI_Stat = "";
String topik_SUB_COMPARASI__Info_Stat = "";
String topik_SUB_COMPARASI__Info_ADC = "";
String topik_SUB_COMPARASI__Info_Waktu = "";
String topik_SUB_COMPARASI__Info_Inteligent = "";
String topik_SUB_COMPARASI_Webserver = "";
String topik_SUB_COMPARASI_Restart = "";
String topik_SUB_COMPARASI_Force = "";
String topik_SUB_COMPARASI_Rssi = "";
String topik_SUB_COMPARASI_Inteligent = "";
unsigned int Status_Kontrol = 0; // 0 artinya manual
char message_buff[256];
char message_sub[256];

int Banyak_Array = 0;
int T1 = 0;
int T2 = 0;
int T3 = 0;
int T4 = 0;
int T5 = 0;
int T6 = 0;
int T7 = 0;
int T8 = 0;
int T9 = 0;
int T10 = 0;
int T11 = 0;
int T12 = 0;
int T13 = 0;
int T14 = 0;

int I1 = 0;
int I2 = 0;
int I3 = 0;
int I4 = 0;
int I5 = 0;
int I6 = 0;
int I7 = 0;

long rssi  = 0;
unsigned int lock =0;
int connect_mqtt = 0;
unsigned int Nilai = 0;
unsigned int Nilai_Akhir = 0;
unsigned long w_awal=0;
unsigned long w_akhir=0;
unsigned long detik = 0;
unsigned long waktu_proses = 0;
unsigned long waktu_cek = 0;
unsigned long publish_RSSI = 1;

char rssid[20], rpassword[20], rbroker[20],rNilai[10];
String st;
int _YY = 0;
int _MO = 0;
int _DD = 0;
int _HH = 0;
int _MI = 0;
int _SS = 0;

WiFiServer server(80);
WiFiClient client;
RTC_DS1307 rtc;
PubSubClient mqtt(client);

void setup() {
  delay(1000);
  Serial.begin(9600);
  EEPROM.begin(512);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  bacadataeeprom();
  mqtt.setServer(broker,port);
  mqtt.setCallback(callback);
  pinMode(PIN_PWM, OUTPUT);
  pinMode(PIN_Relay, OUTPUT);
  Kontrol_LED(Nilai);
  chip.toCharArray(Chip_ID, chip.length()+1);
  Serial.print("ID : ");
  Serial.println(Chip_ID);
  
  sprintf(topik_SUB,"%s/%s/C/#",Device,Chip_ID);
  topik_SUB_COMPARASI_Waktu= String(Device)+"/"+String(Chip_ID)+"/C/T";
  topik_SUB_COMPARASI_Stat= String(Device)+"/"+String(Chip_ID)+"/C/S";
  topik_SUB_COMPARASI_Webserver= String(Device)+"/"+String(Chip_ID)+"/C/W";
  topik_SUB_COMPARASI_Restart= String(Device)+"/"+String(Chip_ID)+"/C/R";
  topik_SUB_COMPARASI_Force= String(Device)+"/"+String(Chip_ID)+"/C/F";
  topik_SUB_COMPARASI_Inteligent= String(Device)+"/"+String(Chip_ID)+"/C/I";
  topik_SUB_COMPARASI_Nilai= String(Device)+"/"+String(Chip_ID)+"/C/level";
  topik_SUB_COMPARASI_Rssi= String(Device)+"/"+String(Chip_ID)+"/C/rssi";
  topik_SUB_COMPARASI__Info_Stat= String(Device)+"/"+String(Chip_ID)+"/C/G/S";
  topik_SUB_COMPARASI__Info_Waktu= String(Device)+"/"+String(Chip_ID)+"/C/G/T";
  topik_SUB_COMPARASI__Info_Inteligent= String(Device)+"/"+String(Chip_ID)+"/C/G/I";
  topik_SUB_COMPARASI__Info_ADC= String(Device)+"/"+String(Chip_ID)+"/C/G/A";
  
  setup_wifi();
}


void loop() {
  if(!mqtt.connected()){
    reconnect();
  }
  mqtt.loop();
  pewaktu();
}

void pewaktu(){
  if(millis()>=detik*10000){
    detik++;
    
  }
  if (detik>publish_RSSI)
  {
   publish_RSSI = publish_RSSI+6;
   rssi = WiFi.RSSI();
    Serial.print("RSSI:");
    Serial.println(rssi);
    sprintf(topik,"%s/%s/RSSI",Device,Chip_ID);
    sprintf(message_buff,"%d",rssi);
    mqtt.publish(topik,message_buff); 

  }
  if (detik>waktu_proses*PERIODE_PROSES+DELTA_PERIODE_PROSES){
    waktu_proses++;
    DateTime now = rtc.now();
    _YY = now.year();
    _MO = now.month();
    _DD = now.day();
    _HH = now.hour();
    _MI = now.minute();
    _SS = now.second();

    if (DEBUG)
    {
      Serial.print("lock : ");
      Serial.println(lock);
      // Serial.println(T1*60+T2);
      // Serial.println(T3*60+T4);
      // Serial.println(T5*60+T6);
      // Serial.println(T7*60+T8);
      // Serial.println(T9*60+T10);
      // Serial.println(T11*60+T12);
      // Serial.println(T13*60+T14);
      Serial.print("Status_Kontrol : ");
      Serial.println(Status_Kontrol);
      Serial.print("Nilai : ");
      Serial.println(Nilai);
      //sprintf(topik,"%s/%s/test",Device,Chip_ID);
      //mqtt.publish(topik,"amanda");
      Serial.print("message_buff:");
      sprintf(message_buff,"%d-%02d-%02d %02d:%02d:%02d",_YY,_MO,_DD,_HH,_MI,_SS);
      Serial.println(message_buff);
    }
    //++++++++++++++++++++++++++++++++++++ inteligent ++++++++++++++++++++++++++++++++++++++++++
      inteligent();
    //++++++++++++++++++++++++++++++++++++ inteligent ++++++++++++++++++++++++++++++++++++++++++
    Serial.println("");
    Serial.println("");
  }
}


void setup_wifi() {
  delay(10);
  int count = 0;
  WiFi.softAPdisconnect(true);
  WiFi.begin(ssid, password);
  delay(5000);
  while(WiFi.status() != WL_CONNECTED){
    count++;
    WiFi.begin(ssid, password);
    delay(2000);
    if (count>=5)Creat_AP();
    }
}


void reconnect() {
  if (!mqtt.connected()) {
    if (mqtt.connect(Chip_ID,usernameMQTT,passwordMQTT))    
    {
      mqtt.subscribe(topik_SUB);
      connect_mqtt = 0;
    } 
    delay(1000);
    connect_mqtt++;
    Serial.print("connect_mqtt :");
    Serial.println(connect_mqtt);
    if (connect_mqtt>=20)
    {
      Status_Kontrol = 1;
      Serial.println("status kontrol menjadi full auto");
      connect_mqtt = 0;
    }
  }
}

void Kontrol_LED(unsigned int _Nilai){
  if (_Nilai>100)_Nilai = 100;
  
  Nilai = _Nilai;
  if (Nilai>Nilai_Akhir)
  {
    while(Nilai>Nilai_Akhir)
    {
      Nilai_Akhir++;
      analogWrite(PIN_PWM, Nilai_Akhir*10);
      delay(45);
    }
  }
  else if (Nilai<Nilai_Akhir)
  {
    while(Nilai<Nilai_Akhir)
    {
      Nilai_Akhir--;
      analogWrite(PIN_PWM, Nilai_Akhir*10);
      delay(45);
    }
  }
  else
  analogWrite(PIN_PWM, Nilai_Akhir*10);

  // if (Nilai==0){
  //   digitalWrite(PIN_Relay,LOW);
  // }
  // else{
  //   digitalWrite(PIN_Relay,HIGH);
  // }

  String SNilai = String(Nilai);
  char CNilai[SNilai.length()+1];
  SNilai.toCharArray(CNilai, SNilai.length()+1);

  for(int i = 0; i<=SNilai.length(); i++){
        if(i == (SNilai.length())){
          EEPROM.write((i+200), '=');
        }
        else
          EEPROM.write((i+200), CNilai[i]);
        delay(10);
  }
  EEPROM.commit();
  Serial.println("Nilai telah disimpan");
  sprintf(topik,"%s/%s/Dim",Device,Chip_ID);
  sprintf(message_buff,"%d",Nilai);
  mqtt.publish(topik,message_buff);
}

void Creat_AP()
{
  if(DEBUG)Serial.println("masuk Creat_AP");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.softAP(Chip_ID, Chip_ID);
  if (DEBUG){
    Serial.print("Wifi status : ");
    Serial.println(WiFi.status());
  }
  server.begin();
  w_awal = millis();
  while(1){
        w_akhir = millis();
        input();
        if(w_akhir - w_awal >= 300000){
            delay(200);
            ESP.restart();
            delay(200);
        }
  }
}

void Wifi_Scan(){
  if(DEBUG)Serial.println("Wifi_Scan awal");
  int n = WiFi.scanNetworks();
  st = "<p><b><h4>Data SSID dan BROKER sekarang : </h4><b></p>\r\n";
  st += "<p style=\"color:black;\"><h4><pre>SSID\t: <i>";
  st += ssid;
  st += "</i><h4>BROKER\t: <i>";
  st += broker;
  st += "</i></h4></p>";
  st += "<p><b><h4>Wifi Network yang terdeteksi : </h4><b></p>";
  st += "<ul style=\"color:black;\">";
  if(n == 0){
    st += "Tidak Ada WiFi yang Terdeteksi";
  }
  for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      st += "<li>";
      st +=i + 1;
      st += ": ";
      st += WiFi.SSID(i);
      st += "\t";
      st += "Strength : ";
      st += (2*(WiFi.RSSI(i)+100));
      st += " %";
      st += "</li>";
    }
  st += "</ul>";
  st += "<p><b><h4>Masukkan SSID dan password : </h4><b></p>";
  delay(100);
  if(DEBUG)Serial.println("Wifi_Scan Akhir");
}

void input() 
{
    //if(DEBUG)Serial.println("input awal");
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client) {
      return;
    } 
    Wifi_Scan();
    
    String req = client.readStringUntil('\r');
    
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);
  
    req = req.substring(addr_start + 1, addr_end);
    urlDecode(req);
    if(req.startsWith("/?ssid=")){
       String qssid; 
       qssid = req.substring(7,req.indexOf("&p"));
       
       String q0password;
       q0password = req.substring(req.indexOf("password="),req.indexOf("broker"));
       String qpassword;
       qpassword = q0password.substring(9,req.lastIndexOf('&'));
       
       String q0broker;
       q0broker = req.substring(req.indexOf("broker="),req.indexOf("waktu"));
       String qbroker;
       qbroker = q0broker.substring(7,req.lastIndexOf("&"));


       char as[qssid.length()+1], ap[qpassword.length()+1], ab[qbroker.length()+1];
      
       qssid.toCharArray(as, qssid.length()+1);
       qpassword.toCharArray(ap, qpassword.length()+1);
       qbroker.toCharArray(ab, qbroker.length()+1);

       if(DEBUG){
        Serial.println(qpassword.length());
        Serial.print("req = ");
        Serial.println(req);
        Serial.print("ssid = ");
        Serial.println(qssid);
        Serial.print("qpassword = ");
        Serial.println(qpassword);
        Serial.print("broker = ");
        Serial.println(qbroker);
        Serial.print("ap = ");
        Serial.println(ap);
        Serial.print("ab = ");
        Serial.println(ab);      
       }
       
       for(int i = 0; i<=qssid.length(); i++){
        EEPROM.write(i, as[i]);
        delay(10);
       }
       EEPROM.write(qssid.length(), '=');
       
       for(int i = 0; i<qpassword.length(); i++){
        if(i == (qpassword.length()-1)){
          EEPROM.write((i+20), '=');
        }
        else
          EEPROM.write((i+20), ap[i]);
        delay(10);
       }
       for(int i = 0; i<=qbroker.length(); i++){
        if(i == (qbroker.length())){
          EEPROM.write((i+40), '=');
        }
        else
          EEPROM.write((i+40), ab[i]);
        delay(10);
       }
       EEPROM.commit();

       for(int i = 0; i <= 120; i++){
        Serial.print((char)EEPROM.read(i));
       }
       
      
       
       client.flush();
       String s = "HTTP/1.1 200 OK\r\n";
       s += "Content-Type: text/html\r\n\r\n";
       s += "<!DOCTYPE HTML>\r\n<html>\r\n<body style=\"background-color:White;\">\r\n";
       s += "<h2>Terima Kasih</h2></body>";
       client.print(s);
       delay(200);
       ESP.restart();
       delay(200);
    }
    client.flush();
    // Prepare the response. Start with the common header:
    String s = "HTTP/1.1 200 OK\r\n";
    s += "Content-Type: text/html\r\n\r\n";
    s += "<!DOCTYPE HTML>\r\n<html>\r\n<head><title>\""+String(Device)+"\"</title><style>div { background-color: lightgrey; width: 600px; padding: 25px; border: 25px solid navy; margin: 25px; }</style></head>\r\n";
    s += "<body><div>";
    s += st;
    s += "<form method=get>";
    s += "SSID                : <input type=\"text\" name=\"ssid\"><br>\r\n";
    s += "Password            : <input type=\"password\" name=\"password\"><br>\r\n";
    s += "Broker (Server)     : <input type=\"text\" name=\"broker\"><br>\r\n";
    s += "<p><b><h4>Pilih Data yang ingin Anda Tampilkan : </h4><b></p>\r\n";
    s += "<br><br><input type=\"submit\" value=\"submit\">";
    s += "\r\n";      
    s += "</form>\r\n";
    s += "</div></body>\r\n</html>\r\n";

    client.print(s);
    
    delay(10);
}

void urlDecode(String &input)
{
  input.replace("%20", " ");
  input.replace("+", " ");
  input.replace("%21", "!");
  input.replace("%22", "\"");
  input.replace("%23", "#");
  input.replace("%24", "$");
  input.replace("%25", "%");
  input.replace("%26", "&");
  input.replace("%27", "\'");
  input.replace("%28", "(");
  input.replace("%29", ")");
  input.replace("%30", "*");
  input.replace("%31", "+");
  input.replace("%2C", ",");
  input.replace("%2E", ".");
  input.replace("%2F", "/");
  input.replace("%2C", ",");
  input.replace("%3A", ":");
  input.replace("%3A", ";");
  input.replace("%3C", "<");
  input.replace("%3D", "=");
  input.replace("%3E", ">");
  input.replace("%3F", "?");
  input.replace("%40", "@");
  input.replace("%5B", "[");
  input.replace("%5C", "\\");
  input.replace("%5D", "]");
  input.replace("%5E", "^");
  input.replace("%5F", "-");
  input.replace("%60", "`");
}

void bacadataeeprom(){

    T1 = EEPROM.read(100);
    T2 = EEPROM.read(101);
    I1 = EEPROM.read(102);
    T3 = EEPROM.read(103);
    T4 = EEPROM.read(104);
    I2 = EEPROM.read(105);
    T5 = EEPROM.read(106);
    T6 = EEPROM.read(107);
    I3 = EEPROM.read(108);
    T7 = EEPROM.read(109);
    T8 = EEPROM.read(110);
    I4 = EEPROM.read(111);
    T9 = EEPROM.read(112);
    T10 = EEPROM.read(113);
    I5 = EEPROM.read(114);
    T11 = EEPROM.read(115);
    T12 = EEPROM.read(116);
    I6 = EEPROM.read(117);
    T13 = EEPROM.read(118);
    T14 = EEPROM.read(119);
    I7 = EEPROM.read(120);
    Banyak_Array = EEPROM.read(121);

  for(int i=0; i<=5; i++){
    if((char)EEPROM.read(i+200) == '='){
      String SNilai = String(rNilai);
      Nilai = SNilai.toInt();
      Serial.print("Nilai :");
      Serial.println(Nilai);
    }
    else{
      rNilai[i] = (char)EEPROM.read(i+200);
    }
    delay(10);
  }
  Nilai_Akhir = Nilai;

  for(int i=0; i<=20; i++){
    if((char)EEPROM.read(i) == '='){
      ssid = rssid;
      Serial.print("SSID :");
      Serial.println(rssid);
      i = 50;
    }
    rssid[i] = (char)EEPROM.read(i);
    delay(10);
  }
  for(int i=0; i<=20; i++){
    if((char)EEPROM.read(i+20) == '='){
      password = rpassword;
      Serial.print("Password :");
      Serial.println(rpassword);
      i = 50;
    }
    else{
      rpassword[i] = (char)EEPROM.read(i+20);
    }
    delay(10);
  }
  for(int i=0; i<=20; i++){
    if((char)EEPROM.read(i+40) == '='){
      broker = rbroker;
      Serial.print("Broker :");
      Serial.println(rbroker);
      i = 50;
    }
    else{
      rbroker[i] = (char)EEPROM.read(i+40);
    }
    delay(20);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  delay(10);
  int i = 0;
  String Stopic = String(topic);
  for (; i < length; i++){message_sub[i] = payload[i];}
  message_sub[i] = '\0';
  String MessageSUB=String(message_sub);
if (DEBUG)
{
  Serial.println("GET Message :");
  Serial.print("Topik: ");
  Serial.println(topic);
  Serial.print("Message :");
  Serial.println(message_sub);
}
  if (Stopic==topik_SUB_COMPARASI_Nilai)
  {
    if (Status_Kontrol==0)
    {
      Serial.println("Ubah nilai PWM");
      Kontrol_LED(MessageSUB.toInt());  
    }
  }
  else if (Stopic==topik_SUB_COMPARASI_Waktu)
  {
    Serial.println("Ubah Waktu");
    String Tahun;
    Tahun = MessageSUB.substring(MessageSUB.indexOf("YY=")+3,MessageSUB.indexOf(",MO="));
    int _Tahun = Tahun.toInt();
    String Bulan;
    Bulan = MessageSUB.substring(MessageSUB.indexOf("MO=")+3,MessageSUB.indexOf(",DD="));
    int _Bulan = Bulan.toInt();
    String Hari;
    Hari = MessageSUB.substring(MessageSUB.indexOf("DD=")+3,MessageSUB.indexOf(",HH="));
    int _Hari = Hari.toInt();
    String Jam;
    Jam = MessageSUB.substring(MessageSUB.indexOf("HH=")+3,MessageSUB.indexOf(",MI="));
    int _Jam = Jam.toInt();
    String Menit;
    Menit = MessageSUB.substring(MessageSUB.indexOf("MI=")+3,MessageSUB.indexOf(",SS="));
    int _Menit = Menit.toInt();
    String Detik;
    Detik = MessageSUB.substring(MessageSUB.indexOf("SS=")+3,MessageSUB.length());
    int _Detik = Detik.toInt();

    if (DEBUG)
    {
      Serial.print("MessageSUB :");
      Serial.println(MessageSUB);
      Serial.print("Tahun :");
      Serial.println(Tahun);
      Serial.print("Bulan :");
      Serial.println(Bulan);
      Serial.print("Hari :");
      Serial.println(Hari);
      Serial.print("Jam :");
      Serial.println(Jam);
      Serial.print("Menit :");
      Serial.println(Menit);
      Serial.print("Detik :");
      Serial.println(Detik);
    }
    rtc.adjust(DateTime(_Tahun,_Bulan,_Hari,_Jam,_Menit,_Detik));

    delay(1000);
    DateTime now = rtc.now();
    _YY = now.year();
    _MO = now.month();
    _DD = now.day();
    _HH = now.hour();
    _MI = now.minute();
    _SS = now.second();

    sprintf(topik,"%s/%s/time",Device,Chip_ID);
    sprintf(message_buff,"%d-%02d-%02d %02d:%02d:%02d",_YY,_MO,_DD,_HH,_MI,_SS);
    mqtt.publish(topik,message_buff);
    lock = 0;
  }
  else if (Stopic==topik_SUB_COMPARASI_Stat)
  {
    Serial.println("Ubah Status kendali");
    if (MessageSUB=="auto")
    {
      Serial.println("Masuk mode auto");
      Status_Kontrol = 1;
    }
    else if (MessageSUB=="manual")
    {
      Serial.println("masuk mode manual");
      Status_Kontrol = 0;
      lock = 0;
    }

    sprintf(topik,"%s/%s/stat",Device,Chip_ID);
    if (Status_Kontrol)sprintf(message_buff,"auto");
    else sprintf(message_buff,"manual");
    mqtt.publish(topik,message_buff);
  }
  else if (Stopic==topik_SUB_COMPARASI_Webserver)
  {
    if (MessageSUB=="webserver")
    {
      Serial.println("Ubah webserver");
      sprintf(topik,"%s/%s/stat",Device,Chip_ID);
      mqtt.publish(topik,"webserver");
      Creat_AP();
    }
    
  }
  else if (Stopic==topik_SUB_COMPARASI_Force)
  {
    if (MessageSUB=="ON"){
      Status_Kontrol = 0;
      Kontrol_LED(100);
      lock = 0;
    }
    else if (MessageSUB=="OFF"){
      Status_Kontrol = 0;
      Kontrol_LED(0);
      lock = 0;
    }
    
  }
  else if (Stopic==topik_SUB_COMPARASI_Restart)
  {
    if (MessageSUB=="restart")
    {
      Serial.println("restart");
      sprintf(topik,"%s/%s/stat",Device,Chip_ID);
      mqtt.publish(topik,"restart");
      delay(200);
      ESP.restart();
      delay(200);
    }
    
  }
  else if (Stopic==topik_SUB_COMPARASI_Rssi)
  {
    rssi = WiFi.RSSI();
    Serial.print("RSSI:");
    Serial.println(rssi);
    sprintf(topik,"%s/%s/RSSI",Device,Chip_ID);
    sprintf(message_buff,"%d",rssi);
    mqtt.publish(topik,message_buff);   
  }
  else if (Stopic==topik_SUB_COMPARASI__Info_Stat)
  {
    Serial.println("Get stat");
    sprintf(topik,"%s/%s/stat",Device,Chip_ID);
    if (Status_Kontrol)sprintf(message_buff,"auto");
    else sprintf(message_buff,"manual");
    mqtt.publish(topik,message_buff);   
    sprintf(topik,"%s/%s/Dim",Device,Chip_ID);
    sprintf(message_buff,"%d",Nilai);
    mqtt.publish(topik,message_buff);
  }
  else if (Stopic==topik_SUB_COMPARASI__Info_Waktu)
  {
    Serial.println("Get time");
    DateTime now = rtc.now();
    _YY = now.year();
    _MO = now.month();
    _DD = now.day();
    _HH = now.hour();
    _MI = now.minute();
    _SS = now.second();

    sprintf(topik,"%s/%s/time",Device,Chip_ID);
    sprintf(message_buff,"%d-%02d-%02d %02d:%02d:%02d",_YY,_MO,_DD,_HH,_MI,_SS);
    mqtt.publish(topik,message_buff);  
  }
  else if (Stopic==topik_SUB_COMPARASI__Info_Inteligent)
  {
    Serial.println("Info inteligent"); 
    sprintf(topik,"%s/%s/intel",Device,Chip_ID);
    if (Banyak_Array == 1)sprintf(message_buff,"{\"%02d:%02d\":%d}",T1,T2,I1);
    else if(Banyak_Array == 2)sprintf(message_buff,"{\"%02d:%02d\":%d,\"%02d:%02d\":%d}",T1,T2,I1,T3,T4,I2);
    else if(Banyak_Array == 3)sprintf(message_buff,"{\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d}",T1,T2,I1,T3,T4,I2,T5,T6,I3);
    else if(Banyak_Array == 4)sprintf(message_buff,"{\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d}",T1,T2,I1,T3,T4,I2,T5,T6,I3,T7,T8,I4);
    else if(Banyak_Array == 5)sprintf(message_buff,"{\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d}",T1,T2,I1,T3,T4,I2,T5,T6,I3,T7,T8,I4,T9,T10,I5);
    else if(Banyak_Array == 6)sprintf(message_buff,"{\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d}",T1,T2,I1,T3,T4,I2,T5,T6,I3,T7,T8,I4,T9,T10,I5,T11,T12,I6);
    else if(Banyak_Array == 7)sprintf(message_buff,"{\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d,\"%02d:%02d\":%d}",T1,T2,I1,T3,T4,I2,T5,T6,I3,T7,T8,I4,T9,T10,I5,T11,T12,I6,T13,T14,I7);
    mqtt.publish(topik,message_buff);


  }
  else if (Stopic==topik_SUB_COMPARASI__Info_ADC)
  {
    Serial.println("Info ADC"); 
    sprintf(topik,"%s/%s/ADC",Device,Chip_ID);
    unsigned int _ADC =  analogRead(A0);
    sprintf(message_buff,"%d",_ADC);
    mqtt.publish(topik,message_buff);
  }
  else if (Stopic==topik_SUB_COMPARASI_Inteligent)
  {
    Serial.println("inteligent"); 
    DynamicJsonBuffer jsonBuffer;

    JsonArray& array = jsonBuffer.parseArray(MessageSUB);
    Banyak_Array =  array.size();
    if (Banyak_Array%3==0)
    {
      Banyak_Array =  Banyak_Array /3;
    }
    else Banyak_Array = 1;
    
    T1 = array[0];
    T2 = array[1];
    I1 = array[2];

    T3 = array[3];
    T4 = array[4];
    I2 = array[5];

    T5 = array[6];
    T6 = array[7];
    I3 = array[8];

    T7 = array[9];
    T8 = array[10];
    I4 = array[11];

    T9 = array[12];
    T10 = array[13];
    I5 = array[14];

    T11 = array[15];
    T12 = array[16];
    I6 = array[17];

    T13 = array[18];
    T14 = array[19];
    I7 = array[20];

    for (int i = 0; i < 21; i++)
    {
      EEPROM.write((i+100),array[i]);
      delay(10);
    }
    EEPROM.write((121),Banyak_Array);
    delay(10);
    EEPROM.commit();
    Serial.print("Banyak_Array : ");
    Serial.println(Banyak_Array);
    Serial.print(" : ");
    char dummy[256];
    sprintf(dummy,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",T1,T2,I1,T3,T4,I2,T5,T6,I3,T7,T8,I4,T9,T10,I5,T11,T12,I6,T13,T14,I7);
    Serial.println(dummy);
    lock = 0;
  }

  for (; i < 256; i++){message_sub[i] = '\0';}
}

void inteligent(){
  if (Status_Kontrol)
      {
        Serial.println("Masuk inteligent");

          if (T1*60+T2 > _HH*60+_MI && _HH*60+_MI >= 0 && lock != 1){
            Kontrol_LED(I1);
            lock = 1;
            Serial.println("1 1");
          }
          else if (T3*60+T4 > _HH*60+_MI && _HH*60+_MI >= T1*60+T2 && lock != 2){
            Kontrol_LED(I2);
            lock = 2;
            Serial.println("2 1");
          }
          else if (T5*60+T6 > _HH*60+_MI && _HH*60+_MI >= T3*60+T4 && lock != 3){
            Kontrol_LED(I3);
            lock = 3;
            Serial.println("3 1");
          }
          else if (T7*60+T8 > _HH*60+_MI && _HH*60+_MI >= T5*60+T6 && lock != 4){
            Kontrol_LED(I4);
            lock = 4;
            Serial.println("4 1");
          }
          else if (T9*60+T10 > _HH*60+_MI && _HH*60+_MI >= T7*60+T8 && lock != 5){
            Kontrol_LED(I5);
            lock = 5;
            Serial.println("5 1");
          }
          else if (T11*60+T12 > _HH*60+_MI && _HH*60+_MI >= T9*60+T10 && lock != 6){
            Kontrol_LED(I6);
            lock = 6;
            Serial.println("6 1");
          }
          else if (T13*60+T14 > _HH*60+_MI && _HH*60+_MI >= T11*60+T12 && lock != 7){
            Kontrol_LED(I7);
            lock = 7;
            Serial.println("7 1");
          }
      }

  if (!Status_Kontrol)
  {
    lock = 0;
  }
}
