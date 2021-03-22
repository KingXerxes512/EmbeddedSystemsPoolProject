#include <GFX4d.h>

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
  gfx.print("Style DOT MATRIX SQUARE");
  gfx.MoveTo(0,140);
  gfx.print("Style DOT MATRIX LED");
  gfx.MoveTo(0,222);
  gfx.print("Style DOT MATRIX FADE");
  gfx.TextColor(WHITE); gfx.Font(1);  gfx.TextSize(6);
}

void loop() {
  // put your main code here, to run repeatedly:
  gfx.TextSize(5);
  gfx.FontStyle(DOTMATRIXROUND);
  gfx.TextColor(ORANGE, BLACK);
  gfx.MoveTo(0, 20);
  gfx.print(count);
  gfx.print("");
  count += 0.1;
  if(count > 900) count = 0.0;
  
}
