#include <DS1302.h>
#include <LiquidCrystal_I2C.h>
#include "EEPROM.h"



#define ag_ismi "Animeto"
#define ag_sifresi "2150215021"
#define debug true
#define M1 4 //motor
#define M2 3
#define kSclkPin 7  // Serial Clock
#define kIoPin   6  // Input/Output
#define kCePin   5  // Chip Enable

char prevChar = "";

typedef void (*WebServerMessagesFunction) (String message);
typedef void (*ClockMessageFunction) (String message);

//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
//LiquidCrystal_I2C lcd = new LiquidCrystal_I2C(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address


//*********      **  *******       **      ***  ***  ****************************
//*********  ******  *******  ***  **  *******    *******************************
//*********  ******  *******  ***  **  *******  **  *****************************
//*********      **       **       **      ***  ****  ***************************
class Clock
{
  private:
    long alarm_hour = 0;
    long alarm_min = 0;
    bool alarm_fired = false;
    bool showdot = true;
    long previousMillis = 0;
    int clockinterval = 1000;
    byte mode = 0;

    String dayAsString(const Time::Day day) {
      switch (day) {
        case Time::kSunday: return "Sunday";
        case Time::kMonday: return "Monday";
        case Time::kTuesday: return "Tuesday";
        case Time::kWednesday: return "Wednesday";
        case Time::kThursday: return "Thursday";
        case Time::kFriday: return "Friday";
        case Time::kSaturday: return "Saturday";
      }
      return "(unknown day)";
    }

    void Mode()
    {
      //for harware button setup. 
      //mode 0: saat gösterim
      //mode 1: saat ayarlama
      //mode 2: dakika ayarlama
      //mode 3: mama 1 saat ayar
      //mode 4: mama 1 dakika ayar
      //mode 5: mama 2 saat ayar
      //mode 6: mama 2 dakika ayar
      mode += 1;
      if (mode == 3)
      {
        mode = 0;
      }
      Serial.println(" Mode= " + String(mode));
    }
    
    void SetHourMode()
    {
      Time t = rtc->time();
      lcd.home ();
      lcd.print("Ayr SA  ");
      lcd.setCursor ( 0, 1 );
      lcd.print(SaatFormat(t));
    }

    void IncHour()
    {
      Time t = rtc->time();
      int hour = t.hr + 1;
      SetTime(t.yr, t.mon, t.date, hour, t.min);
      PrintTime();
    }

    void DecHour()
    {
      Time t = rtc->time();
      int hour = t.hr - 1;
      SetTime(t.yr, t.mon, t.date, hour, t.min);
      PrintTime();
    }

    void IncMin()
    {
      Time t = rtc->time();
      int min = t.min + 1;
      SetTime(t.yr, t.mon, t.date, t.hr, min);
      PrintTime();
    }

    void DecMin()
    {
      Time t = rtc->time();
      int min = t.min - 1;
      SetTime(t.yr, t.mon, t.date, t.hr, min);
      PrintTime();
    }

    void SetMinMode()
    {
      lcd.home ();
      lcd.print("Ayr DA  ");
      Time t = rtc->time();
      lcd.print(SaatFormat(t));
    }

  public:
    DS1302* rtc = new DS1302(kCePin, kIoPin, kSclkPin);
    ClockMessageFunction clockCallbackfunction;

    Clock(ClockMessageFunction clockMessageCallback)
    {
      Serial.println("Clock");
      clockCallbackfunction = clockMessageCallback;
      Time t(2017, 10, 7, 11, 45, 00, Time::kSaturday);
      rtc->time(t);
    }
  
    void SetAlarm(int hour, int minute)
    {
      Serial.println("SetAlarm");
      alarm_hour = hour;
      alarm_min = minute;
      Serial.println(alarm_min);
    }

    void Button1Press()
    {
      Serial.println("Button1Press");
      Mode();
    }

    void Button2Press()
    {
      Serial.println("Button2Press");
      if (mode == 1)
      {
        IncHour();
      }

      if (mode == 2)
      {
        IncMin();
      }
    }

    void Button3Press()
    {
      Serial.println("Button3Press");
      if (mode == 1)
      {
        DecHour();
      }

      if (mode == 2)
      {
        DecMin();
      }
    }

    char* SaatFormat(Time t)
    {
      char saatformat[16];
      snprintf(saatformat, sizeof(saatformat), "%02d:%02d:%02d",
               t.hr, t.min, t.sec);
      return  saatformat;
    }

    String formatDigit(int number)
    {
      String r="";
      if (number<10)
      {
        r = "0"+number;
      }
      else
      { 
        r= number;
      }
      return r;
    }

    void PrintTime()
    {
      //Serial.println("PrintTime");
      Time t = rtc->time();
      // Name the day of the week.
      //const String day = dayAsString(t.day);
      // Format the time and date and insert into the temporary buffer.
      // lazım olursa debug olarak serial basma işi için kullanılabilir.
      //char buffull[50];
      //snprintf(buffull, sizeof(buffull), "%s %04d-%02d-%02d %02d:%02d:%02d",
      //         day.c_str(),
      //         t.yr, t.mon, t.date,
      //         t.hr, t.min, t.sec);
      //Serial.println(buffull);
      lcd.home ();
      lcd.print("A1 " + formatDigit(alarm_hour)+":"+formatDigit(alarm_min));      
      lcd.setCursor ( 0, 1 );
      lcd.print(SaatFormat(t));
      //if (showdot)
      //{
        //lcd.print(t.hr);
      //}
      //else
      //{
        //lcd.print(t.hr);
      //}
      //display.showNumberDecEx(alarm_hour, (0x80 >> 1), true, 2, 0);
      //showdot = !showdot;
      //Serial.println(t.sec);
      //Serial.println("PrintTime end");
    }

    void SetTime(int year, int month, int day, int hour, int min)
    {
      //rtc->writeProtect(false);//bunu yeni cihazda bir kez çağırmak yetiyor.
      Time t(year, month, day, hour, min, 00, 6);
      rtc->time(t);
    }

    void Update(int statusByte)
    {
      mode = 0;
      if (mode == 0)
      {
        //esas rutindeki loop içinde çağrılır. Saat ve display ile ilgili gösterim
        //işlerinin tümünü halleder.
        unsigned long currentMillis = millis();
        // How much time has passed, accounting for rollover with subtraction!
        if ((unsigned long)(currentMillis - previousMillis) >= clockinterval)
        {
          lcd.home ();
          if (statusByte == 8)
          {
            //lcd.print("Onln 1.0");
            //Serial.println(alarm_min);
          }
          else
          {
            lcd.print("NO CONN.");
          }
          // It's time to do something!
          this->PrintTime();

          //Time t = rtc->time();
         //alarm kontrolü yapmak için burası uygun bir yer olabilir.
         Time t = rtc->time();
         if(t.hr==alarm_hour && t.min == alarm_min)
         {
          if(!alarm_fired)
          {
            alarm_fired = true; //sadece bir kez alarm sinyali vermesi için.
            clockCallbackfunction("Alarm");
          }
         }
         else
         {
            alarm_fired = false;
         }
 
          // Use the snapshot to set track time until next event
          previousMillis = currentMillis;
        }
      }

      if (mode == 1)
      {
        //esas rutindeki loop içinde çağrılır. Saat ve display ile ilgili gösterim
        //işlerinin tümünü halleder.
        unsigned long currentMillis = millis();
        // How much time has passed, accounting for rollover with subtraction!
        if ((unsigned long)(currentMillis - previousMillis) >= 500)
        {
          // It's time to do something!
          this->SetHourMode();
          // Use the snapshot to set track time until next event
          previousMillis = currentMillis;
        }
      }

      if (mode == 2)
      {
        //esas rutindeki loop içinde çağrılır. Saat ve display ile ilgili gösterim
        //işlerinin tümünü halleder.
        unsigned long currentMillis = millis();
        // How much time has passed, accounting for rollover with subtraction!
        if ((unsigned long)(currentMillis - previousMillis) >= 500)
        {
          // It's time to do something!
          this->SetMinMode();
          // Use the snapshot to set track time until next event
          previousMillis = currentMillis;
        }
      }
    }
};
///******************************************************************************
//*******************************************************************************
//*******************************************************************************




class Webserver
{

  private:
    int timeout =  12000;
    int timeout1 = 500;

  public:

    WebServerMessagesFunction callBackServerState;
    WebServerMessagesFunction callBackClientResponse;

    Webserver(WebServerMessagesFunction serverState, WebServerMessagesFunction clientResponse)
    {
      callBackServerState = serverState;
      callBackClientResponse = clientResponse;
    }

    void Create()
    {
      String baglantiKomutu = String("AT+CWJAP=\"") + ag_ismi + "\",\"" + ag_sifresi + "\"";
      callBackServerState("BUSY");
      PrintDebug("");
      PrintDebug("Creating a web server...");
      int repeat = 0;
      bool webServerCreated = false;
      while (repeat < 4)
      {
        repeat++;
        PrintDebug("Try:" + String(repeat));
        if (!ESPCommand("AT+RST", "ready")) {
          continue;
        }  ;//eğer komut hatalı dönerse en baştan komutları oluşturma işine girişir.
        if (!ESPCommand("AT+CWMODE=1", "OK")) {
          continue;
        }  ;//eğer komut hatalı dönerse en baştan komutları oluşturma işine girişir.
        if (!ESPCommand(baglantiKomutu, "OK")) {
          continue;
        } ;
        if (!ESPCommand("AT+CWJAP?", "OK")) {
          continue;
        } ;
        if (!ESPCommand("AT+CWJAP?", "OK")) {
          continue;
        };
        if (!ESPCommand("AT+CIPMUX=1", "OK")) {
          continue;
        };
        if (!ESPCommand("AT+CIPSERVER=1,80", "OK")) {
          continue;
        };
        //ESPCommand("AT+CIFSR", "OK");
        PrintDebug("Web server created.");
        callBackServerState("READY");
        PrintDebug("");
        webServerCreated = true;
        break;
      }
      if (!webServerCreated)
      {
        callBackServerState("ERROR");
        PrintDebug("Web server can not be created. Try reset!");
      }
    }

    bool ESPCommand(String command, String SearchFor)
    {
      long int time = millis();
      String response = "";
      PrintDebug("-------------------");
      PrintDebug("Command: " + command);
      Serial1.print(command + "\r\n");
      //delay(20);
      while ( (time + timeout) > millis())
      {
        while (Serial1.available() > 0)
        {
          char c = Serial1.read(); // read the next character.
          response += String(c);
          //Serial.print("r:" + response);
          if (response.indexOf(SearchFor) > 0)
          {
            Serial1.flush();
            PrintDebug("Response:" + response);
            return true;
          }

          if (response.indexOf("ERROR") > 0)
          {
            Serial1.flush();
            PrintDebug("* ERROR *");
            PrintDebug("Response:" + response);
            return false;
          }
        }
      }
      Serial1.flush();
      Serial.print("TIMEOUT" + response + ":\r\n");
      PrintDebug("* TIMEOUT *");
      return false;
    }

    bool SendDataCommand(int connection, String text, bool closeConnection)
    {
      //bunu ayrı yaptım çünkü gönderilen veri içindeki OK yüzünden.
      long int time = millis();
      String response = "";
      String command = "AT+CIPSEND=" + String(connection) + "," + String(text.length()); // + "\r\n" + text ;
      ESPCommand(command, ">");
      ESPCommand(text, "SEND OK");

      if (closeConnection)
      {
        CloseConnection(connection);  // (a) http://192.168.2.40/?pin=12 şeklinde çağrıldığında, bu komut (kapatma) olursa browser dönmeyi bırakıyor.
      }                               // file:///C:/Users/burak/OneDrive/kişisel/arduino/web/test.html şeklinde kullanıldığında ise
      // yukarıdaki komut işleri yavaşlatıyor. Hiç kapatılmaz ise daha iyi çalışıyor gibi.
      // cep telefonu üzerinden kullanıldığında (a) yöntemi daha iyi gibi




      //PrintDebug("Sending data...");
      //PrintDebug("Command:" + command);
      //Serial1.println(text);
      //delay(20);

      /*
        while( (time+timeout) > millis())
        {
        while(Serial1.available()>0)
        {
          char c = Serial1.read(); // read the next character.
          response+=String(c);
          if(response.indexOf("OK")>0)
          {
            PrintDebug("OK Response:" + response);
            PrintDebug("Data sended.");
            return true;
          }
          if(response.indexOf("ERROR")>0)
          {
            Serial1.flush();
            PrintDebug("ERROR Response:" + response + " Data can not be sended.");
            return false;
          }
        }
        }
        Serial1.flush();
        PrintDebug("TIMEOUT Response:" + response + " Data can not be sent.");
        return false;
      */
    }

    void MessageLoop()
    {
      //waits for the client messages
      long int time = millis();
      String response = "";
      String partialResponse = "";
      bool dataRecieved = false;

      if (Serial1.available() > 0)
      {
        Serial.println("m1");
        while ( (time + timeout1) > millis())
        {
          while (Serial1.available() > 0)
          {
            //Serial.println("m2:" + partialResponse);
            char c = Serial1.read(); // read the next character.
            response += c;
            partialResponse += String(c);
            if (c == 'K' && prevChar == 'O')
            {
              //process response
              Serial.println("****  process response start ***** ");
              //Serial.println("partial:" + partialResponse);
              //Serial.println("****  process response ***** ");
              callBackClientResponse(partialResponse);
              Serial.println("****  process response end ***** ");
              //int connection = ProcessData(partialResponse);
              //delay(200);
              partialResponse = "";
              dataRecieved = true;
              break;
            }
            prevChar = c;
          }
          if (dataRecieved)
          {
            break;
          }
        }
      }
    }

    void CloseConnection(int connection)
    {
      PrintDebug("Closing connection...");
      ESPCommand("AT+CIPCLOSE=" + String(connection), "OK");
      PrintDebug("Connection closed.");
    }

    void PrintDebug(String text)
    {
      if (debug)
      {
        Serial.print(text + "\r\n");
      }
    }

};
//--------------------------------------------------------------- web server end-----------------------------------


//main program start
bool armed = false;
long alarm_min;
long alarm_hour;
bool connectedDevice = false;
int deviceStatus = 8;

long previousMotorMillis = 0;
int motorForwardInterval = 1500;
int motorBackwardInterwal = 300;
int maxRepeatCount = 10;
int repeatCount = 0;
int motorMainMode = 0;
int motorSubMode = 0;

String ServerResponse(String PinValue)
{
  String content = "";
  content += "HTTP/1.1 200 OK\r\n";
  content += "Access-Control-Allow-Origin: *\r\n";
  content += "Content-Type: text/html\r\n";
  content += "\r\n"; // do not forget this one
  content += "Hello";
  /*content += "<!DOCTYPE HTML>\r\n";
  content += "<html>\r\n";
  content += "Hello";
  content += "</html>\r\n";
  */


  /*
  content += "<html>\r\n";
  content += "Led pin is now: ";
  content += PinValue + "\r\n";
  content += "Alarm Saati: ";
  content += String(alarm_hour) + ":" + String(alarm_min);
  content += "<br><br>\r\n";
  content += "Click <a href=\"/?pin=12OFF\">here</a> turn the LED Off<br>\r\n";
  content += "Click <a href=\"/?pin=13ON\">here</a> turn the LED On<br>\r\n";
  content += "Click <a href=\"/?pin=30ST\">here</a> START<br>\r\n";
  content += "Click <a href=\"/?pin=31ST\">here</a> STOP<br>\r\n";
  content += "Click <a href=\"/?pin=162000\">Set Alarm To 20:00</a> STOP<br>\r\n";
  content += "Click <a href=\"/?pin=14201901111400\">Set Alarm To 2019-01-11 14:00 yyyymmddhhmm  </a> STOP<br>\r\n";
  content += "</html>\r\n";
  */

/*
content += "<html><head><title>ESP8266 LED Control</title></head><body>";
content += "<button id=\"11\" class=\"led\" onclick=\"loadXMLDoc();\">Toggle</button>";
content += "<button id=\"12\" class=\"led\">Toggle Pin 12</button>";
content += "<button id=\"13\" class=\"led\">Toggle Pin 13</button>";
content += "  </body>";
content += "  <script type=\"text/javascript\">\r\n";
content += "    function loadXMLDoc() {";
content += "      var xmlhttp = new XMLHttpRequest();\r\n";
content += "      alert('test');\r\n";
content += "      xmlhttp.onreadystatechange = function() {\r\n";
content += "        if (xmlhttp.readyState == XMLHttpRequest.DONE) {\r\n";
content += "           if (xmlhttp.status == 200) {\r\n";
content += "             document.getElementById(\"myDiv\").innerHTML = xmlhttp.responseText;\r\n";
content += "           }\r\n";
content += "           else if (xmlhttp.status == 400) {\r\n";
content += "            alert('There was an error 400');\r\n";
content += "           }\r\n";
content += "           else {\r\n";
content += "        }\r\n";
content += "      };\r\n";
content += "    }\r\n";
content += "    </script>\r\n";
content += "</html>\r\n";
*/
  

  /*

    content +="<html>\r\n";
    content +="<body>\r\n";
    content +="<h1>Happy New Millennium!</h1>\r\n";
    content +="</body>\r\n";
    content +="</html>\r\n";
    content ="";


    String metin = "";
    metin += "HTTP/1.0 200 \r\n";
    metin += "Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n";
    metin += "Content-Type: text/html\r\n";
    metin += "Content-Length: " + String(content.length()) + "\r\n";
  */

  return  content;// + content;

}

void ClockMessages(String message)
{
    Serial.println("Clock message" + message);
    if(message=="Alarm")
    {
      Start();
    }
}

void WebServerStateMessages(String message)
{
  if (message == "READY")
  {
    connectedDevice = true;
    deviceStatus = 8;
    LcdMessage("Ready");
  }

  if (message == "ERROR")
  {
    connectedDevice = false;
    deviceStatus = 0;
    LcdMessage("CON ERR");
  }

  if (message == "BUSY")
  {
    deviceStatus = 1;
    LcdMessage("BUSY");
  }

  Serial.println("-WebServerStateMessages: " + message);
}

void WebServerClientMessages(String message)
{
  Serial.println("WebServerClientMessages: " + message);
  ProcessData(message);
}


Clock c(ClockMessages);

Webserver ws(WebServerStateMessages, WebServerClientMessages); //WebServerMessages callback function. Sunucu oluşturma ve mesaj sırasındaki tüm iletişim buradan sağlanır.


void setup() {
  Serial.begin(9600); //serial monitor. For debug
  Serial1.begin(115200); // your esp's baud rate might be different
  lcd.begin(8, 2);
  pinMode(13, OUTPUT);
  pinMode(15, INPUT);
  digitalWrite(13, LOW);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  //LcdMessage("Init...");
  ReadAlarm();
  ws.Create();
  c.SetTime(2017, 12, 28, 9, 14);
  c.PrintTime();
  //lcd.home ();

}

void LcdMessage(String msg)
{
  lcd.home ();
  lcd.print("        ");
  lcd.print(msg);
}


void loop() {

  unsigned long currentMillis = millis();

  if (connectedDevice)
  {
    ws.MessageLoop();
  }
  c.Update(deviceStatus);

  MotorControl(currentMillis);

  ReadKeyboard();

}

void ReadKeyboard()
{
//bir sebeple düğmeler çalışıyor prototip cihazda
    int button=digitalRead(17);
    if(button==HIGH)
    {
      lcd.clear();
      LcdMessage(String(button));
    }
}

void MotorControl(unsigned long cm)
{
  if (motorMainMode == 1)
  {
    if (motorSubMode == 0)
    {
      previousMotorMillis = cm;
      motorSubMode = 1;
      Geri();
    }

    if (motorSubMode == 1)
    {
      if ((unsigned long)(cm - previousMotorMillis) >= motorBackwardInterwal) {
        ileri();
        repeatCount = repeatCount + 1;
        previousMotorMillis = cm;
        // Use the snapshot to set track time until next event
        //motorMainMode = 0;
        motorSubMode = 2;
      }
    }

    if (motorSubMode == 2)
    {
      if ((unsigned long)(cm - previousMotorMillis) >= motorForwardInterval) {
        Dur();
        // Use the snapshot to set track time until next event
        motorMainMode = 1;
        motorSubMode = 0;
      }
    }

  }

  if (motorMainMode == 0)
  {
    motorMainMode = 3;
    Dur();
  }

  if (repeatCount == maxRepeatCount + 1)
  {
    motorMainMode = 3;
    repeatCount = 0;
    Dur();
  }
}

void ileri()
{
  Serial.println("motor ileri");
  digitalWrite(M1, LOW);
  digitalWrite(M2, HIGH);
}

void Geri()
{
  Serial.println("motor geri");
  digitalWrite(M1, HIGH);
  digitalWrite(M2, LOW);
}

void Dur()
{
  Serial.println("motor stop");
  digitalWrite(M1, LOW);
  digitalWrite(M2, LOW);
}


int ProcessData(String data)
{
  int pinNo = 0;
  int pinlocation = data.indexOf("pin=");
  int connectionlocation = data.indexOf("+IPD");
  if (connectionlocation > 0)
  {
    if (pinlocation > 0)
    {
      pinNo = data.substring(pinlocation + 4, pinlocation + 6).toInt();
      int connection = data.substring(connectionlocation + 5 , connectionlocation + 6).toInt();
      Serial.println("****  pin located ***** ");
      Serial.println("data: " + data);
      Serial.println("connection: " + String(connection));
      Serial.println("pin: " + String(pinNo));
      //komutlar pin ile gönderilir
      String PinData = "";
      switch (pinNo)
      {
        case 12:
          digitalWrite(13, LOW); // toggle pin
          PinData = data.substring(pinlocation + 7, pinlocation + 10).toInt(); //pin=12OFF
          LcdMessage(PinData);
          break;

        case 13:
          digitalWrite(13, HIGH); // toggle pin
          PinData = data.substring(pinlocation + 7, pinlocation + 9).toInt(); //pin=13ON
          LcdMessage(PinData);
          break;

        case 14:
          //set current time  yyyymmddhhmm
          PinData = data.substring(pinlocation + 6, pinlocation + 19); //pin=14yyyymmddhhmm
          SetCurrentTime(PinData);
          break;

        case 16:
          //set alarm ssmm   1345 13:45 alarm on.
          PinData = data.substring(pinlocation + 6, pinlocation + 11); //pin=162355
          SetAlarms(PinData);
          break;

       case 17:
          //set alarm off.
          SetAlarmOff();
          break;
          
       case 18:
          //set alarm on.
          SetAlarmOn();
          break;
                  

        case 20:
          //dönme süresi. saniye cinsinden 000-999  titresim param1 000-999 titresim param2 000-999
          PinData = data.substring(pinlocation + 7, pinlocation + 18).toInt(); //pin=12345678901
          SetMotorParameters(PinData);
          break;

        case 30:
          //start
          Start();
          break;

        case 31:
          //Stop
          Stop();
          break;

        case 40:
          //status. cihazın statü dönüşü yapması için kullanılır.
          PinData = data.substring(pinlocation + 7, pinlocation + 11).toInt(); //pin=40ST
          break;
      }

      String metin = ServerResponse(PinData);
      ws.SendDataCommand(connection, metin, true);

      return connection;
    }
    return -1;
  }
}

void SetMotorParameters(String motorParams)
{

}

void SetCurrentTime(String timeString)
{
  Serial.println("Set clock Message:" + timeString);
  int year   =  timeString.substring(0, 4).toInt();
  int month  =  timeString.substring(4, 6).toInt();
  int day    =  timeString.substring(6, 8).toInt();
  int hour   =  timeString.substring(8, 10).toInt();
  int minute =  timeString.substring(10, 12).toInt();
  Serial.println(year);
  Serial.println(month);
  Serial.println(day);
  Serial.println(hour);
  Serial.println(minute);
  c.SetTime(year, month, day, hour, minute);
  c.PrintTime();
}

void SetAlarms(String msg)
{ 
  Serial.println("Alarm Message:" + msg);
  int alarmInt = msg.toInt();
  int hour = alarmInt/100;
  int minute = alarmInt - (hour * 100);
  alarm_hour = hour;
  alarm_min = minute;
  //LcdMessage("A:" + String(hour) + ":" + String(minute));
  Serial.println(hour);
  Serial.println(minute);
  c.SetAlarm(hour, minute);
  //EEPROM.write(0, hour);
  //EEPROM.write(1, minute);
}

void SetAlarmOn()
{
  EEPROM.write(2, 1);
}

void SetAlarmOff()
{
  EEPROM.write(2, 0);
}

void Start()
{
  motorMainMode = 1;
}

void Stop()
{
  motorMainMode = 0;
}

String ReadAlarm()
{
  alarm_hour = EEPROM.read(0);
  alarm_min =  EEPROM.read(1);
  char buf[4] = "";
  sprintf(buf, "%02d%02d", alarm_hour, alarm_min);
  Serial.println("AlarmString" + String(buf));
   return String(buf);
}

