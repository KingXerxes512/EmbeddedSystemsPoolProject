#include <GFX4d.h>
#include <Wire.h>

//void setup(){
//  Serial.begin(115200); 
//  while(!Serial){} // Waiting for serial connection
// 
//  Serial.println();
//  Serial.println("Start I2C scanner ...");
//  Serial.print("\r\n");
//  byte count = 0;
//  
//  Wire.begin();
//  for (byte i = 8; i < 120; i++)
//  {
//    Wire.beginTransmission(i);
//    if (Wire.endTransmission() == 0)
//      {
//      Serial.print("Found I2C Device: ");
//      Serial.print(" (0x");
//      Serial.print(i, HEX);
//      Serial.println(")");
//      count++;
//      delay(1);
//      }
//  }
//  Serial.print("\r\n");
//  Serial.println("Finish I2C scanner");
//  Serial.print("Found ");
//  Serial.print(count, HEX);
//  Serial.println(" Device(s).");
//}
//
//void loop() {}

GFX4d gfx = GFX4d();

float count = 0.0;

void setup() {
  // put your setup code here, to run once:
  gfx.begin();
  gfx.Cls();
  gfx.ScrollEnable(true);
  gfx.BacklightOn(true);
  gfx.Orientation(PORTRAIT);
  gfx.SmoothScrollSpeed(5);
  gfx.TextColor(WHITE); gfx.Font(1);  gfx.TextSize(5);
  gfx.MoveTo(0,0);
  gfx.TextColor(WHITE); gfx.Font(2);  gfx.TextSize(1);
  gfx.print("Style DOT MATRIX");
  gfx.MoveTo(0,70);
  gfx.print("Device Statuses:");
//  gfx.MoveTo(0,140);
//  gfx.print("Style DOT MATRIX LED");
//  gfx.MoveTo(0,222);
//  gfx.print("Style DOT MATRIX FADE");
//  gfx.TextColor(WHITE); gfx.Font(1);  gfx.TextSize(6);
}

void loop() {
  // put your main code here, to run repeatedly:
  gfx.TextSize(3);
  gfx.FontStyle(DOTMATRIXROUND);
  gfx.TextColor(ORANGE, BLACK);
  gfx.MoveTo(0, 20);
  gfx.print(count);
  gfx.MoveTo(0,90);
  gfx.print("ON");
  //gfx.print("");
  count += 0.1;
  if(count > 900) count = 0.0;
  
}
