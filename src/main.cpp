//头文件
#include <Thread.h>
#include "Freenove_WS2812_Lib_for_ESP32.h"
#include "SimpleBLE.h"
#include <U8g2lib.h>
#include <Wire.h>
#include <_sd.h>
#include <WiFi.h>
#include "time.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
//---------------宏定义
#define LEDS_COUNT 1
#define LEDS_PIN 16
#define CHANNEL 0
#define font1 u8g2_font_amstrad_cpc_extended_8r
#define font2 u8g2_font_inb38_mn
#define font3 u8g_font_courR14
#define UP_BTN 35
#define DOWN_BTN 34
#define BACK_BTN 27
#define ENTER_BTN 26
#define MYVERSION "2.0210811"
//---------------声明
SimpleBLE ble;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/22, /* data=*/21);
Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);
//Tfilter["now"][0]["text"] = true;
//Tfilter["now"][0]["temperature"] = true;
StaticJsonDocument<400> doc;

//---------------------------线程声明
Thread LED = Thread();
Thread RGBLight = Thread();
Thread cleaner = Thread();
Thread BAT = Thread();
void clearOLED(); //清除OLED所有内容
void led();
void printStr(int line, String s);
void printStr(int line, int num);
void printStr(int line, int v, String s);
void printStr(int line, String s, int offset);
void printStr(int line, int num, int offset = 0);
void batteryCheck(); //检查电池
void clearHalf();
void autoreturn();
void close();
//---------------全局变量
u8 m_color[5][3] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 255}, {0, 0, 0}};
int j = 0;
const char *ssid = "doudoudoudou"; //wifi账号密码
const char *password = "zkzs1101";
const char *ntpServer = "ntp.aliyun.com"; //时间服务器
const long gmtOffset_sec = 28800;         //时间偏移
const int daylightOffset_sec = 0;         //夏令时
int sleepcount = 0;                       //睡眠计时器达到后睡眠
int rgbflag = 0;                          //rgb位置指示器

//显示时间
void printLocalTime()
{
  struct tm timeinfo;
  int fontwid = 14;
  int begi = 7;
  int heigh = 20;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  //u8g2.clear();
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 0, 127, 30);
  u8g2.setDrawColor(1);
  u8g2.setFont(font3);
  u8g2.setCursor(begi, heigh);
  u8g2.print(timeinfo.tm_hour);
  u8g2.setCursor(begi + fontwid * 2, heigh);
  u8g2.print(":");

  //---minutes
  u8g2.setCursor(begi + fontwid * 3, heigh);
  if (timeinfo.tm_min < 10)
  {
    u8g2.print("0");
    u8g2.setCursor(begi + fontwid * 4, heigh);
    u8g2.print(timeinfo.tm_min);
  }
  else
  {
    u8g2.setCursor(begi + fontwid * 3, heigh);
    u8g2.print(timeinfo.tm_min);
  }

  u8g2.setCursor(begi + fontwid * 5, heigh);
  u8g2.print(":");

  //--seconds
  u8g2.setCursor(begi + fontwid * 6, heigh);
  if (timeinfo.tm_sec < 10)
  {
    u8g2.setCursor(begi + fontwid * 6, heigh);
    u8g2.print("0");
    u8g2.setCursor(begi + fontwid * 7, heigh);
    u8g2.print(timeinfo.tm_sec);
  }
  else
  {
    u8g2.print(timeinfo.tm_sec);
  }

  u8g2.sendBuffer();
  if (Serial.available())
    Serial.println(&timeinfo, "%H:%M:%S");
}

void onButton()
{
  String out = "BLE32 name: ";
  out += String(millis() / 1000);
  Serial.println(out);
  ble.begin(out);
}
//清空OLED
void clearOLED()
{
  u8g2.clear();
  u8g2.clearBuffer();
}
void printStr(int line, String s)
{
  u8g2.setCursor(0, line * 10);
  u8g2.print(s);
}
void printStr(int line, String s, int offset = 0)
{
  u8g2.setCursor(0, line * 10 + offset);
  u8g2.print(s);
}
void printStr(int line, int v, String s)
{
  u8g2.setCursor(v * 6, line * 10);
  u8g2.print(s);
}
void printStr(int line, int num)
{

  u8g2.setCursor(0, line * 10);
  u8g2.print(num);
}
void printStr(int line, int num, int offset)
{

  u8g2.setCursor(0, line * 10 + offset);
  u8g2.print(num);
}
String tagethttp = "https://api.seniverse.com/v3/weather/now.json?key=SHt7w7B2FdDE7apnN&location=Xiamen&language=en";
void led()
{
  strip.setLedColorData(0, m_color[j][0], m_color[j][1], m_color[j][2]);
  strip.show();
  j++;
  if (j == 5)
    j = 0;
}
int temperature = 0;
const char *weather;
void getHttp() //TODO 获取http网页
{
  DynamicJsonDocument doc(2048);
  HTTPClient https;
  https.begin(tagethttp);
  int httpCode = https.GET();
  u8g2.setFont(font1);
  u8g2.clear();
  u8g2.setCursor(3, 50);
  if (httpCode > 0)
  {

    u8g2.sendBuffer();
    String payload = https.getString();
    deserializeJson(doc, payload);
    JsonObject results_0 = doc["results"][0];
    JsonObject results_0_now = results_0["now"];
    weather = results_0_now["text"];
    temperature = results_0_now["temperature"];
  }
  else
  {

    u8g2.print("Get http Failed");
    u8g2.sendBuffer();
    delay(1000);
  }
}

//---------打印当前的天气到屏幕上面
const char* lastweather;
int firstprint=0;
void printWeather()
{
  int offset = 11;
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 33, 127, 32);
  u8g2.setDrawColor(1);
  u8g2.setCursor(35 - offset, 42);
  u8g2.print(temperature);
  //绘制温度符号
  u8g2.drawCircle(58 - offset + 4, 32, 2, U8G2_DRAW_ALL);
  u8g2.setCursor(63 - offset + 4, 42);
  u8g2.print("C");
  //绘制天气
  u8g2.setFont(font1);
  u8g2.setCursor(5, 57);
  if(firstprint == 0)
  {
u8g2.print(weather);
firstprint =1;
lastweather = weather;
  }
   else{
     u8g2.print(lastweather);
   }
  u8g2.setDrawColor(0);
  u8g2.drawBox(5 + 9 * 8, 57 - 7, 5 * 8, 7);
  u8g2.setDrawColor(1);
  batteryCheck();
  // u8g2.sendBuffer();
}
void animateG(int x, int y, int r)
{

  int rd1 = random(2);
  int rd2 = random(3);
  int rd3 = random(3);
  int rd4 = random(3);
  int rd5 = random(3);
  int rd6 = random(2);
  u8g2.drawLine(x * rd1, (y - r * 2 / 3) * rd2, (x + r / sqrt(3)) * rd3, (y + r / 3) * rd4);
  u8g2.drawLine((x + r / sqrt(3)) * rd3, (y + r / 3) * rd4, (x - r / sqrt(3)) * rd5, (y + r / 3) * rd6);
  u8g2.drawLine((x - r / sqrt(3)) * rd5, (y + r / 3) * rd6, x * rd1, (y - r * 2 / 3) * rd2);
}
//显示错误提示信息
void DisError()
{
  u8g2.clear();
  int counter = 0;
  while (1)
  {

    u8g2.setFont(font3);
    printStr(2, "ERROR", 0);
    u8g2.setFont(font1);
    printStr(4, "Connect failure", 5);
    printStr(5, "Check setting", 5);
    printStr(6, MYVERSION, 5);
    animateG(127 / 2, 63 / 2 + 3, 40);
    delay(100);
    u8g2.sendBuffer();
    if(counter++ == 100)
    close();
  }
}
//获取网上的时间
void getNetTime()
{
  int i = 0;
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(50);
    Serial.print(".");
    //u8g2.setCursor(2 + (i++), );
    u8g2.drawLine(0, 0, (i++), 0);
    u8g2.sendBuffer();
    if (i == 127)
    {
      DisError();
    }
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  u8g2.setDrawColor(1);
  u8g2.setFont(font1);
  u8g2.setCursor(10, 48);
  clearHalf();
  u8g2.setDrawColor(1);
  u8g2.print("Get Time!"); //TODO 获取时间的提示
  // u8g2.sendBuffer();

  printLocalTime();
  Serial.println(" CONNECTED");
  getHttp();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

//动画数字效果
void dynamicNum(int x, int y, int num)
{
  int i;
  for (i = 0; i < num; i++)
  {
    u8g2.setDrawColor(0);
    u8g2.drawBox(x, 10, 60, 60);
    u8g2.setCursor(x, y);
    u8g2.setDrawColor(1);
    u8g2.print(i);
    u8g2.drawBox(0, 63 - 3, (127 / 100) * i, 3);
    u8g2.sendBuffer();
    delay(2);
  }
}

//---------SD卡检查的功能
void page2_sdCheck()
{
  float size, used;
  double percent;
  u8g2.clear();
  u8g2.setFont(font1);                        // choose a suitable font
  u8g2.drawStr(128 / 9, 10, "SD RAIDER VA1"); // write something to the internal memory
  //u8g2.drawStr(128 / 9, 22, "Loading.."); // write something to the internal memory
  u8g2.sendBuffer();
  size = getSDSize();
  used = getSDUsed();
  if (size > 0)
  {
    printStr(3, "Total", -5);
    printStr(4, size, -5);
    printStr(5, "Used", -5);
    printStr(6, used, -5);
  }

  percent = used / size;
  u8g2.setFont(font2);
  u8g2.setCursor(50, 48);
  if (percent > 0.01)
  {

    dynamicNum(50, 60, (int)(percent * 100));
    strip.setLedColorData(0, 256);
    strip.show();
  }
  else
  {
    u8g2.print(1);
  }
  Serial.printf("Used Percent:%1f %%\r\n", percent * 100.0);
  u8g2.sendBuffer();
  sdend();
  autoreturn();
}
//---------自动延迟返回主界面函数
void autoreturn()
{
  //127*delaytime = 超时返回主界面的时间
  int i = 0;
  while (i < 127)
  {
    u8g2.drawLine(0, 0, i++, 0);
    delay(50);
    u8g2.sendBuffer();
    if (digitalRead(BACK_BTN) == LOW)
    {
      delay(20);
      if (digitalRead(BACK_BTN) == LOW)
      {
        return;
      }
    }
  }
  u8g2.clear();
  printWeather();
  return;
}
///自动变换RGB灯的程序
void RGBchg()
{

  strip.setLedColorData(0, strip.Wheel((0 * 256 / LEDS_COUNT + rgbflag) & 255));
  strip.show();
  rgbflag += 1;
  if (rgbflag == 256)
  {
    rgbflag = 0;
  }
}
//-----------屏幕关闭特效
void close()
{
  u8g2.clear();
  u8g2.drawBox(0, 63 / 2 - 3, 127, 6);
  u8g2.drawBox(127 / 2 - 3, 0, 6, 63);
  u8g2.sendBuffer();
  delay(3);
  u8g2.clear();
  u8g2.drawBox(0, 63 / 2 - 2, 127, 4);
  u8g2.drawBox(127 / 2 - 2, 0, 4, 63);
  u8g2.sendBuffer();
  delay(3);
  u8g2.clear();
  u8g2.drawBox(0, 63 / 2 - 1, 127, 2);
  u8g2.drawBox(127 / 2 - 1, 0, 2, 63);
  u8g2.sendBuffer();
  delay(3);
  u8g2.clear();
  u8g2.setPowerSave(1);
  strip.setLedColorData(0, strip.Wheel(0));
  strip.setAllLedsColor(0, 0, 0);
  esp_deep_sleep_start();
}

void clearHalf()
{
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 33, 127, 32);
}
//TODO ----绘制电池图标和检查电源
int lastbat = 0;
void batteryCheck()
{

  int x = 100;
  int y = 50;
  int offset = 5;
  float vol = 0;
  int volint = 0;

  u8g2.setCursor(x, y);
  u8g2.drawFrame(x, y, 20, 12);

  //1.获取电池电量的数值

  vol = (analogRead(32) / 4096.0) * 100;
  volint = (int)vol + offset;

  if (volint >= 100)
  {
    //1.1绘制电池的正极
    u8g2.drawFrame(x - 3, y + 3, 3, 6);
    u8g2.setCursor(x - 4, y - 5);
    volint = 100;
    u8g2.print("BAT");
  }
  else
  {

    u8g2.setCursor(x - 4, y - 5);
    u8g2.setDrawColor(1);
    u8g2.setFont(font1);
    //防抖

    if (abs(lastbat - (volint)) > 2)
    {
      u8g2.print(volint);
      lastbat = volint;
    }
    else
    {
      u8g2.print(lastbat);
    }
    u8g2.setCursor(x - 4 + 17, y - 5);
    u8g2.print("%");
  }

  //u8g2.drawBox(x+(analogRead(32) / 4096.0)*20, y+1, 20-(analogRead(32) / 4096.0)*20-1, 12-2);//绘制电池图标的电量
  //u8g2.sendBuffer();
}
//----------------启动运行一次
void setup()
{
  strip.begin();
  strip.setBrightness(10);
  Serial.begin(921600);
  Serial.println("ESP32 Init,serial test");
  //ble.begin("ESP32 SimpleBLE");
  pinMode(UP_BTN, INPUT);
  pinMode(DOWN_BTN, INPUT);
  pinMode(BACK_BTN, INPUT);
  pinMode(ENTER_BTN, INPUT);
  //多线程设置
  LED.onRun(led);
  LED.setInterval(120);
  RGBLight.onRun(RGBchg);
  RGBLight.setInterval(20);
  BAT.onRun(batteryCheck);
  BAT.setInterval(50);
  // cleaner.onRun(clearHalf);
  // RGBLight.setInterval(500);
  //TODO 1.开场文字
  u8g2.begin();
  u8g2.setFont(font3);                 // choose a suitable font
  u8g2.drawStr(4, 24, "Connecting.."); // write something to the internal memory
  u8g2.setFont(font1);
  u8g2.drawStr(128 / 9, 30 + 16, ssid);
  u8g2.drawStr(128 / 9, 30 + 30, password);
  u8g2.sendBuffer(); // transfer internal memory to the display
                     //TODO 2. 获取网络时间
  getNetTime();

  // clearOLED();
}

//-------------------------------------------启动持续运行
void Btn()
{
  if (digitalRead(UP_BTN) == LOW) //SD卡检查功能
  {
    delay(20);
    if (digitalRead(UP_BTN) == LOW)
    {
      u8g2.setFont(font1);
      u8g2.setDrawColor(1);
      u8g2.setCursor(12, 50);
      page2_sdCheck();
    }
  }
  // if (digitalRead(DOWN_BTN) == LOW)
  // {
  //   delay(20);
  //   if (digitalRead(DOWN_BTN) == LOW)
  //   {
  //     u8g2.setFont(font1);
  //     u8g2.setDrawColor(1);
  //     u8g2.setCursor(12, 50);
  //     clearHalf();
  //     u8g2.print("Down pressed");
  //     u8g2.sendBuffer();
  //     delay(500);
  //     while (digitalRead(DOWN_BTN) == LOW)
  //       ;
  //   }
  // }
  if (digitalRead(BACK_BTN) == LOW)
  {
    delay(20);
    if (digitalRead(BACK_BTN) == LOW)
    {
      u8g2.setFont(font1);
      u8g2.setDrawColor(1);
      u8g2.setCursor(12, 50);
      //clearHalf();
      //u8g2.print("Back pressed");
      u8g2.sendBuffer();
      delay(500);
      while (digitalRead(BACK_BTN) == LOW) ;
      u8g2.clear();
      //getHttp();
    }
  }
  if (digitalRead(ENTER_BTN) == LOW)
  {
    delay(20);
    if (digitalRead(ENTER_BTN) == LOW)
    {
      u8g2.clear();
      u8g2.setFont(font1);
      u8g2.setDrawColor(1);
      u8g2.setCursor(12, 20);
      clearHalf();
      u8g2.print(MYVERSION);
      u8g2.setCursor(12, 20 + 10);
      u8g2.print("Created by");
      u8g2.setCursor(12, 20 + 20);
      u8g2.print("Sinzo");
      u8g2.sendBuffer();
      delay(500);
      while (digitalRead(ENTER_BTN) == LOW)
        ;
    }
  }
}
void loop()
{
  //Serial.println("System run"); // if(LED.shouldRun())
  // LED.run();
  if (RGBLight.shouldRun())
    RGBLight.run();
  //if(BAT.shouldRun())
  //   BAT.run();

  Btn();
  printLocalTime();
  printWeather();
  u8g2.sendBuffer();
}