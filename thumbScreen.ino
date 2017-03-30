// -----------------------------------------------------------------------------
// Game 1. touch the circle.
// -----------------------------------------------------------------------------
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define LEDPIN 11

// these values determined by experimentation, your mileage may vary.
#define THUMB_MIN_X 30
#define THUMB_MAX_X 630
#define THUMB_MIN_Y 330
#define THUMB_MAX_Y 990

// player and target radii
#define PLAYER_R 4
#define TARGET_R 4

// -------------------------------------
// global variables
// -------------------------------------
// count micro-seconds
long us;

// player xy, target xy, max xy
unsigned int px,py,tx,ty,mx,my,v,d;

// effective zero distance from x,y 0,0
const unsigned int ZP=PLAYER_R;
const unsigned int ZT=TARGET_R;

// victory distance
const unsigned int VD=PLAYER_R+TARGET_R;

// define display. Why 4? 'Cause Lady Ada said so.
#define OLED_RESET 4
Adafruit_SSD1306 oled(OLED_RESET);

// -------------------------------------
// begin functions
// -------------------------------------
// place the target circle at a random position
void place_target(){tx=random(ZT,mx);ty=random(ZT,my);}

// start the player at a random position
void place_player(){
  do{
    px=random(ZP,mx);
    py=random(ZP,my);
  }while(get_distance()<VD);}

// show target and player on display
void draw_field(){
  oled.clearDisplay();
  oled.drawCircle(tx,ty,TARGET_R,WHITE);
  oled.fillCircle(px,py,PLAYER_R,WHITE);
  oled.display();
}

// initialize the game
void init_game(){
  // get millis at start
  us=millis();

  // seed the random generator from a floating analog pin
  randomSeed(analogRead(2));

  // prompt
  oled.clearDisplay();
  oled.setTextSize(4);
  oled.setTextColor(WHITE);
  oled.setCursor(10,0);
  oled.print("GO!");
  oled.display();
  delay(3000);

  // initialize the target and player
  // note: it is possible to set PLAYER_R and TARGET_R relative to the
  //       size of the display that place_target or place_player will loop
  //       infinitely. Don't do that.
  place_target();
  place_player();
  draw_field();
}

// Blink LED and show info on serial output
void blynk(){
  digitalWrite(LEDPIN,HIGH);
  delay(1);
  digitalWrite(LEDPIN,LOW);

  Serial.print("px: ");
  Serial.print(px);
  Serial.print(" py: ");
  Serial.print(py);
  Serial.print(" tx: ");
  Serial.print(tx);
  Serial.print(" ty: ");
  Serial.print(ty);
  Serial.print(" d: ");
  Serial.println(d);
}

// calculate distance from center of player to center of target
unsigned int get_distance(){
  // pythagoras said a*a + b*b = c*c
  int a=px-tx;
  int b=py-ty;
  unsigned int c2=(a*a)+(b*b);
  return round(sqrt(c2));
}

// what if you win?
void victory(){
  // notify serial
  blynk();
  Serial.print("Victory! ");
  Serial.print(d);
  Serial.print(" vs. ");
  Serial.print(VD);
  Serial.print(" in ");
  Serial.print(round((millis()-us)/1000));
  Serial.println(" seconds");

  // notify display
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(10,0);
  oled.print("Victory! ");
  oled.display();

  // wait 5 seconds and restart
  delay(5000);
  init_game();
}

// -----------------------------------------------------------------------------
// SETUP
// -----------------------------------------------------------------------------
void setup() {
  // thumb stick conected here
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  // LED connected here
  pinMode(LEDPIN, OUTPUT);

  // good for debugging
  Serial.begin(9600);

  // set up display
  oled.begin();
  
  // max x and y values
  mx=oled.width()-TARGET_R;
  my=oled.height()-TARGET_R;
  
  // init the game
  init_game();
}

// -----------------------------------------------------------------------------
// LOOP
// -----------------------------------------------------------------------------
void loop() {
  // get tumbstick vector
  int ix = map(analogRead(A0),THUMB_MIN_X,THUMB_MAX_X,-3,3);
  int iy = map(analogRead(A1),THUMB_MIN_Y,THUMB_MAX_Y,-3,3);

  // adjust and constrain position
  px+=ix; py+=iy;
  px=px<ZP?ZP:px; px=px>mx?mx:px;
  py=py<ZP?ZP:py; py=py>my?my:py;

  d=get_distance();
  if (d<VD){victory();}else{draw_field();}

  // blink LED and output to serial once per second
  if (millis()>us+5000){blynk();us=millis();}
}
// -----------------------------------------------------------------------------

