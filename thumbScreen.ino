// -----------------------------------------------------------------------------
// File:   thumbScreen.ino
// Desc:   a simple game
// Source: https://github.com/DidymusRex/thumbScreen
// License: This work is licensed under a Creative Commons 
//          Attribution-ShareAlike 4.0 International License.
//          https://creativecommons.org/licenses/by-sa/4.0/legalcode
// -----------------------------------------------------------------------------
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BLUPIN 13 // Blue LED
#define BTNPIN 12 // Trigger button
#define GRNPIN 11 // Green LED
#define REDPIN 10 // Red LED
#define RNDPIN A2 // Leave this pin floating for randomness

#define THUMB_X A0
#define THUMB_Y A1
#define THUMB_MIN_X 0
#define THUMB_MAX_X 1024
#define THUMB_MIN_Y 0
#define THUMB_MAX_Y 1024

#define TIME_LIMIT_US 5000

// player and target radii
#define PLAYER_R 4
#define TARGET_R 4

// -------------------------------------
// global variables
// -------------------------------------
// count micro-seconds
long us, init_us;

// player xy, target xy, max xy,player wins, player losses
unsigned int px,py,tx,ty,mx,my,pw,pl;

// effective zero distance from x,y 0,0
// keep player and target from going off the edge
const unsigned int ZP=PLAYER_R;
const unsigned int ZT=TARGET_R;

// victory distance, smaller is harder.
// const unsigned int VD=PLAYER_R+TARGET_R; // edges touch
// const unsigned int VD=TARGET_R;          // player edge touches target center
   const unsigned int VD=PLAYER_R;          // target edge touched player center

// define display. Why 4? 'Cause Lady Ada said so.
#define OLED_RESET 4
Adafruit_SSD1306 oled(OLED_RESET);

// -----------------------------------------------------------------------------
// begin functions
// -----------------------------------------------------------------------------

// -------------------------------------
// update display during game
// -------------------------------------
void draw_field(){
  oled.clearDisplay();
  oled.drawCircle(tx,ty,TARGET_R,WHITE);
  oled.fillCircle(px,py,PLAYER_R,WHITE);

  // show the time-bar on the display
  // time elapsed 3px high across bottom of display
  int w=map(millis()-init_us,0,TIME_LIMIT_US,0,128);
  oled.writeFillRect(0,62,w,3,WHITE);

  oled.display();
}

// -------------------------------------
// initialize the game
// -------------------------------------
void init_game(){

  // set the LEDs
  digitalWrite(GRNPIN, LOW);
  digitalWrite(REDPIN, LOW);
  digitalWrite(BLUPIN, HIGH);

  oled.clearDisplay();
  oled.setTextColor(WHITE);

  //score
  oled.setTextSize(1);
  oled.setCursor(10,0);
  oled.print("Arduino ");
  oled.print(pl;
  oled.print(" Human ");
  oled.print(pw);
  
  // Go!
  oled.setTextSize(2);
  oled.setCursor(50,20);
  oled.print("GO!");
  
  // prompt
  oled.setTextSize(1);
  oled.setCursor(25,50);
  oled.print("press to start");
  
  oled.display();

  // wait for the button push
  while ( digitalRead(BTNPIN)){ delay(10); }
  digitalWrite(BLUPIN, LOW);

  // initialize the target and player positions
  // note: We may need to loop-proof place_player. it is possible to set 
  //       PLAYER_R and TARGET_R relative to the size of the display such
  //       that place_player will loop infinitely. Don't do that.

  // place_target
  tx=random(ZT,mx);
  ty=random(ZT,my);

  // place_player
  do{
    px=random(ZP,mx);
    py=random(ZP,my);
  }while(get_distance()<VD);
  
  draw_field();
  
  // start the timer
  init_us=us=millis();
}

#ifdef DEBUG
// -------------------------------------
// show info on serial output
// -------------------------------------
void syrial(){
  Serial.print("px: ");
  Serial.print(px);
  Serial.print(" py: ");
  Serial.print(py);
  Serial.print(" tx: ");
  Serial.print(tx);
  Serial.print(" ty: ");
  Serial.print(ty);
  Serial.print(" d: ");
  Serial.println(get_distance());
}
#endif

// -------------------------------------
// calculate distance from center of player to center of target
// -------------------------------------
unsigned int get_distance(){
  // pythagoras said a*a + b*b = c*c
  int a=px-tx;
  int b=py-ty;
  unsigned int c2=(a*a)+(b*b);
  return round(sqrt(c2));
}

// -------------------------------------
// game over, dude!
// -------------------------------------
void game_over(int result){
  int ex, ey;
  char cry[9];

  if (result == HIGH){ // Victory!
      digitalWrite(GRNPIN, HIGH);
      sprintf(cry, "VICTORY!");
      ex=tx; ey=ty; pw++;
  } else {          // Defeat!
      digitalWrite(REDPIN, HIGH);
      sprintf(cry, "*DEFEAT*");
      ex=px; ey=py; pl++;
  }

#ifdef DEBUG
  // for debugging
  Serial.print(cry);
  Serial.print(" in ");
  Serial.print(round((millis()-us)/1000));
  Serial.println(" seconds");
#endif

  // notify display (explode target)
  for (int i=0; i<30; i+=3){
      oled.drawCircle(ex, ey, TARGET_R+i, WHITE);
      oled.display();
      delay(33);
  }

  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(20,25);
  oled.print(cry);
  oled.display();

  // wait 2 seconds and restart
  delay(2000);
  init_game();
}

// -----------------------------------------------------------------------------
// SETUP
// -----------------------------------------------------------------------------
void setup() {
  // reset score
  pw=pl=0;
  
  // thumb stick conected here
  pinMode(THUMB_X, INPUT);
  pinMode(THUMB_Y, INPUT);

  // LED connected here
  pinMode(REDPIN, OUTPUT);
  pinMode(GRNPIN, OUTPUT);
  pinMode(BLUPIN, OUTPUT);
  
  // seed the random generator from a floating analog pin
  randomSeed(analogRead(RNDPIN));

#ifdef DEBUG 
  // for debugging
  Serial.begin(9600);
#endif

  // set up display
  oled.begin();
  
  // max x and y values, leave three pixels for the time-bar
  mx=oled.width()-TARGET_R;
  my=oled.height()-(TARGET_R+3);
  
  // initialize the game
  init_game();
}

// -----------------------------------------------------------------------------
// LOOP
// -----------------------------------------------------------------------------
void loop() {
  // get thumbstick vector
  int ix = map(analogRead(THUMB_X),THUMB_MIN_X,THUMB_MAX_X,3,-3);
  int iy = map(analogRead(THUMB_Y),THUMB_MIN_Y,THUMB_MAX_Y,-3,3);
  
  // adjust and constrain player position
  px+=ix; py+=iy;
  px=px<ZP?ZP:px; px=px>mx?mx:px;
  py=py<ZP?ZP:py; py=py>my?my:py;

#ifdef DEBUG
  // update serial display every 0.1 seconds or so
  if (millis()>us+100){
    syrial();
    us=millis();
  }
#endif
  
  int g=-1;                                       // Undecided
  if (get_distance() < VD){ g=HIGH; }             // Victory
  if (millis()-init_us > TIME_LIMIT_US){ g=LOW; } // Defeat
  (g>-1)?game_over(g):draw_field();               // Win, lose or draw
}
// -----------------------------------------------------------------------------

