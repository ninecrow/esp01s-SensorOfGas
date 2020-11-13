#include <Arduino.h>
#include <U8x8lib.h>
#include <SoftwareSerial.h>

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 10, /* data=*/ 9, /* reset=*/ U8X8_PIN_NONE); 

int gas_din=2;
int gas_ain=A0;
int ad_value;
#define DIS_BUFFER_SIZE  (10)
char dis[DIS_BUFFER_SIZE];
int speakerPin=3 ;      //蜂鸣器引脚定义

#define SERIAL_BUFFER_SIZE (128)
char  msg[SERIAL_BUFFER_SIZE];

// 11 rxPin , 12 txPin
SoftwareSerial mySerial(11,12);

/*
 * 初始各个引脚
 * arduino 默认方法
 */
void setup()
{
  init_gas();
  init_display();
  init_beep();
  Serial.begin(9600);
  //初始软串口
  mySerial.begin(9600);
}

/*
 * 主调用循环
 * arduino 默认方法
 */
void loop()
{
  ad_value=analogRead(gas_ain);
  snprintf (dis, DIS_BUFFER_SIZE, "GAS:%d", ad_value);
  
  
  Serial.println(dis);
  u8x8.setFont(u8x8_font_8x13B_1x2_f);
  u8x8.drawString(0,2,dis);

  memset(msg,0,SERIAL_BUFFER_SIZE);
  if(digitalRead(gas_din)==LOW)
  {
    //Serial.println("Gas leakage");
    u8x8.drawString(0,0,"Gas leakage");
    beep();

    snprintf (msg,SERIAL_BUFFER_SIZE, "Gas leakage|%d", ad_value);
    Serial.println(msg);
    Serial.print("buffer strlen:");
    Serial.println(strlen(msg));
    mySerial.write(msg,strlen(msg)+1);
  }
  else
  {
    //Serial.println("Gas not leak");
    u8x8.drawString(0,0,"Gas not leak");
 
    snprintf (msg,SERIAL_BUFFER_SIZE, "Gas not leak|%d", ad_value);
    Serial.println(msg);
    Serial.print("buffer strlen:");
    Serial.println(strlen(msg));
    //strlen 不包含\0空字符， 传输到esp8266 串口需要读取\0空字符作为 消息结尾
    mySerial.write(msg,strlen(msg)+1);
    //mySerial.print(msg);
  }
  
  // display 
  u8x8.refreshDisplay();

  //延迟秒刷新一次
  delay(10000);
}

/*
 * 气体检测引脚初始
 * 气体传感器可以调节灵敏度，用气体打火机测试
 */
void init_gas()
{
  pinMode(gas_din,INPUT);
  pinMode(gas_ain,INPUT);
}

/*
 * 显示屏引脚初始
 * 左上角是原点，向右是X轴，向下是Y轴
 * 128*64点阵
 */
void init_display()
{
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(10, 0);
  digitalWrite(9, 0);    
  
  u8x8.begin();
  u8x8.setPowerSave(0);
}

/*
 * 无源蜂鸣器初始
 */
void init_beep(){
  pinMode(speakerPin, OUTPUT);
}


void playTone(int tone, int duration) { 
  for (long i = 0; i < duration * 1000L; i += tone * 2) { 
    digitalWrite(speakerPin, HIGH); 
    delayMicroseconds(tone); 
    digitalWrite(speakerPin, LOW); 
    delayMicroseconds(tone); 
  } 
} 

void playNote(char note, int duration) { 
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' }; 
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 }; 
// play the tone corresponding to the note name 
  for (int i = 0; i < 8; i++) { 
    if (names[i] == note) { 
      playTone(tones[i], duration); 
    } 
  } 
}


/*
 * 无源蜂鸣器 演奏小星星
 */
void beep()
{
  int length = 15; // the number of notes 
  char notes[] = "ccggaagffeeddc "; // a space represents a rest 
  int tempo = 300;
  int beats[] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 }; 
  for (int i = 0; i < length; i++) { 
    if (notes[i] == ' ') { 
      delay(beats[i] * tempo); // rest 
    } else { 
      playNote(notes[i], beats[i] * tempo); 
    } 
    // pause between notes 
    delay(tempo / 2);  
  }
}
