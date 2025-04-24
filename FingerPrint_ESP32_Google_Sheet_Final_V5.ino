#include <LiquidCrystal_I2C.h>
#include "WiFi.h"
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
//----------------------------------------

// Defines the button PIN.
#define BTN_PIN 33

//******************** ADDED *************************************************************
HardwareSerial mySerial(2);  // Use UART2 on ESP32 (TX=17, RX=16)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

//----------------------------------------SSID and PASSWORD of your WiFi network.
const char* ssid = "CYBOX_2.4";  //--> Your wifi name
const char* password = "CYBOX117"; //--> Your wifi password
//const char* ssid = "TP-Link_A126";  //--> Your wifi name
//const char* password = "My050380"; //--> Your wifi password
//----------------------------------------

// Google script Web_App_URL.
String Web_App_URL = "https://script.google.com/macros/s/AKfycbxOa4hFLmPtNuXyO8c5n1q3uZesu4KWJH_DZuaD7sAaTu3jWy89C96n8xbqSvHqZ7QDjA/exec";

String reg_Info = "";

String atc_Info = "";
String atc_Name = "";
String atc_Date = "";
String atc_Time_In = "";
String atc_Time_Out = "";

// Variables for the number of columns and rows on the LCD.
int lcdColumns = 20;
int lcdRows = 4;
int fingerID;
String fingerID_string;
String modes = "atc";

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  // (lcd_address, lcd_Columns, lcd_Rows)

//**************** http_Req() *********************
// Subroutine for sending HTTP requests to Google Sheets.
void http_Req(String str_modes, String str_uid) 
{
  if (WiFi.status() == WL_CONNECTED) 
  {
    String http_req_url = "";

    //----------------------------------------Create links to make HTTP requests to Google Sheets.
    if (str_modes == "atc") 
    {
      http_req_url  = Web_App_URL + "?sts=atc";
      http_req_url += "&uid=" + str_uid;
    }
    if (str_modes == "reg") 
    {
      http_req_url = Web_App_URL + "?sts=reg";
      http_req_url += "&uid=" + str_uid;
    }
    
    //----------------------------------------Sending HTTP requests to Google Sheets.
    Serial.println();
    Serial.println("-------------");
    Serial.println("Sending request to Google Sheets...");
    Serial.print("URL : ");
    Serial.println(http_req_url);
    
    // Create an HTTPClient object as "http".
    HTTPClient http;

    // HTTP GET Request.
    http.begin(http_req_url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    // Gets the HTTP status code.
    int httpCode = http.GET(); 
    Serial.print("HTTP Status Code : ");
    Serial.println(httpCode);

    // Getting response from google sheet.
    String payload;
    if (httpCode > 0) 
    {
      payload = http.getString();
      Serial.println("Payload : " + payload);  
    }
    
    Serial.println("-------------");
    http.end();
    //----------------------------------------
    
    String sts_Res = getValue(payload, ',', 0);

    //----------------------------------------Conditions that are executed are based on the payload response from Google Sheets (the payload response is set in Google Apps Script).
    if (sts_Res == "OK") 
    {
      //..................
      if (str_modes == "atc") 
      {
        atc_Info = getValue(payload, ',', 1);
        
        if (atc_Info == "TI_Successful") 
        {
          atc_Name = getValue(payload, ',', 2);
          atc_Date = getValue(payload, ',', 3);
          atc_Time_In = getValue(payload, ',', 4);

          //::::::::::::::::::Create a position value for displaying "Name" on the LCD so that it is centered.
          int name_Lenght = atc_Name.length();
          int pos = 0;
          if (name_Lenght > 0 && name_Lenght <= lcdColumns) 
          {
            pos = map(name_Lenght, 1, lcdColumns, 0, (lcdColumns / 2) - 1);
            pos = ((lcdColumns / 2) - 1) - pos;
          } 
          else if (name_Lenght > lcdColumns) 
          {
            atc_Name = atc_Name.substring(0, lcdColumns);
          }
          //::::::::::::::::::
       
          if (fingerID != -1) 
          {
            // Display name if it's not already displayed
            lcd.setCursor(0, fingerID - 1);
            lcd.print(atc_Name);
            delay(1000); // Display name for 2 seconds
          } 
        }  
        delay(500);
      }

      if (atc_Info == "TO_Successful") 
      {
        atc_Name = getValue(payload, ',', 2);
        atc_Date = getValue(payload, ',', 3);
        atc_Time_In = getValue(payload, ',', 4);
        atc_Time_Out = getValue(payload, ',', 5);

        //::::::::::::::::::Create a position value for displaying "Name" on the LCD so that it is centered.
        int name_Lenght = atc_Name.length();
        int pos = 0;
        if (name_Lenght > 0 && name_Lenght <= lcdColumns) 
        {
          pos = map(name_Lenght, 1, lcdColumns, 0, (lcdColumns / 2) - 1);
          pos = ((lcdColumns / 2) - 1) - pos;
        } 
        else if (name_Lenght > lcdColumns) 
        {
          atc_Name = atc_Name.substring(0, lcdColumns);
        }
        //::::::::::::::::::

        if (fingerID != -1) 
        {
          // Clear name if it's already displayed
          lcd.setCursor(0, fingerID - 1);
          lcd.print("                ");
          delay(1000); // Clear the name for 2 seconds
        }  
      }
      atc_Info = "";
      atc_Name = "";
      atc_Date = "";
      atc_Time_In = "";
      atc_Time_Out = "";
    }
      
    if (str_modes == "reg") 
    {
      reg_Info = getValue(payload, ',', 1);
      reg_Info = "";
    }
  }
} 

//*************************** getValue() *************************************
String getValue(String data, char separator, int index) 
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) 
  {
    if (data.charAt(i) == separator || i == maxIndex) 
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//************************** byteArray_to_string() ****************************
/*void byteArray_to_string(byte array[], unsigned int len, char buffer[]) 
{
  for (unsigned int i = 0; i < len; i++) 
  {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len*2] = '\0';
} */

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println();
  delay(1000);

  mySerial.begin(57600, SERIAL_8N1, 16, 17);
    
  // Initialize fingerprint sensor
  finger.begin(57600);

  if (finger.verifyPassword()) 
  {
    Serial.println("Found fingerprint sensor!");
  } 
  else 
  {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  pinMode(BTN_PIN, INPUT_PULLUP);
  
  lcd.init();
  lcd.backlight();
  
  lcd.clear();

  delay(500);

  lcd.setCursor(5,0);
  lcd.print("ESP32 RFID");
  lcd.setCursor(3,1);
  lcd.print("Google  Sheets");
  lcd.setCursor(1,2);
  lcd.print("Attendance  System");
  lcd.setCursor(0,3);
  lcd.print("***************");
  delay(3000);
  lcd.clear();

  //----------------------------------------Set Wifi to STA mode
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");
  //---------------------------------------- 

  //----------------------------------------Connect to Wi-Fi (STA).
  Serial.println();
  Serial.println("------------");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  //:::::::::::::::::: The process of connecting ESP32 with WiFi Hotspot / WiFi Router.
  // The process timeout of connecting ESP32 with WiFi Hotspot / WiFi Router is 20 seconds.
  // If within 20 seconds the ESP32 has not been successfully connected to WiFi, the ESP32 will restart.
  // I made this condition because on my ESP32, there are times when it seems like it can't connect to WiFi, so it needs to be restarted to be able to connect to WiFi.

  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");

    lcd.setCursor(0,0);
    lcd.print("Connecting to SSID");
    delay(250);

    lcd.clear();
    delay(250);
    
    if (connecting_process_timed_out > 0) connecting_process_timed_out--;
    if (connecting_process_timed_out == 0) 
    {
      delay(1000);
      ESP.restart();
    }
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("------------");

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("WiFi connected");
  delay(2000);
  //::::::::::::::::::
  //----------------------------------------

  lcd.clear();
  delay(500);
}

void loop()
{
  fingerID = getFingerprintID();

  //----------------------------------------Switches modes when the button is pressed.
  // modes = "reg" means the mode for new user registration.
  // modes = "atc" means the mode for filling in attendance (Time In and Time Out).

  int BTN_State = digitalRead(BTN_PIN);

  if (BTN_State == LOW) 
  {
    lcd.clear();
    
    if (modes == "atc") 
    {
      modes = "reg";
    } 
    else if (modes == "reg")
    {
      modes = "atc";
    }
    delay(500);
  }
  //----------------------------------------

  fingerID = getFingerprintID();
  fingerID_string = String(fingerID);

  //---Conditions that are executed if modes == "atc".
  if (modes == "atc") 
  {
    if(fingerID > 0)
    if (fingerID)
    {
     http_Req(modes, fingerID_string);
    }
  }
  //----------------------------------------

  //********* Conditions that are executed if modes == "reg".
  if (modes == "reg") 
  {
    if(fingerID > 0)
    if (fingerID)
    {
      http_Req(modes, fingerID_string);
    }
  }
  //----------------------------------------
  delay(10);
  Serial.print("fingerID: ");
  Serial.println(fingerID);
}

// Function to get fingerprint ID
int getFingerprintID() 
{
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) 
  {
    Serial.println("No finger detected.");
    return -1;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) 
  {
    Serial.println("Failed to convert image.");
    return -1;
  }

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK) 
  {
    Serial.println("No match found.");
    return -1;
  }
  return finger.fingerID; // Return the finger ID of the matched fingerprint
}
