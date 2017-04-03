// -----------------------------------------------------------------------------
// Game 1. touch the circle.
// -----------------------------------------------------------------------------
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BLUPIN 13
#define BTNPIN 12
#define GRNPIN 11
#define REDPIN 10

#define THUMB_MIN_X 0
#define THUMB_MAX_X 1024
#define THUMB_MIN_Y 0
#define THUMB_MAX_Y 1024

// player and target radii
#define PLAYER_R 4
#define TARGET_R 4

// -------------------------------------
// global variables
// -------------------------------------
// count micro-seconds
long us, init_us;

// player xy, target xy, max xy
unsigned int px,py,tx,ty,mx,my,v,d;

// effective zero distance from x,y 0,0
const unsigned int ZP=PLAYER_R;
const unsigned int ZT=TARGET_R;

// victory distance
// const unsigned int VD=PLAYER_R+TARGET_R;
// const unsigned int VD=TARGET_R;
   const unsigned int VD=PLAYER_R;

// define display. Why 4? 'Cause Lady Ada said so.
#define OLED_RESET 4
Adafruit_SSD1306 oled(OLED_RESET);

// -----------------------------------------------------------------------------
// begin functions
// -----------------------------------------------------------------------------

// -------------------------------------
// place the target circle at a random position
// -------------------------------------
void place_target(){
  tx=random(ZT,mx);
  ty=random(ZT,my);
}

// -------------------------------------
// start the player at a random position
// -------------------------------------
void place_player(){
  do{
    px=random(ZP,mx);
    py=random(ZP,my);
  }while(get_distance()<VD);
}

// -------------------------------------
// show target and player on display
// -------------------------------------
void draw_field(){
  oled.clearDisplay();
  oled.drawCircle(tx,ty,TARGET_R,WHITE);
  oled.fillCircle(px,py,PLAYER_R,WHITE);
  
  int w=map(millis()-init_us,0,10000,0,128);
  oled.writeFillRect(0, 62, w, 3, WHITE);
  //oled.drawFastHLine(127,61,t,WHITE);
  oled.display();
}

// -------------------------------------
// initialize the game
// -------------------------------------
void init_game(){
  // get millis at start
  us=millis();
  init_us=us;
 
  // seed the random generator from a floating analog pin
  randomSeed(analogRead(2));

  digitalWrite(GRNPIN, LOW);
  digitalWrite(REDPIN, LOW);
  digitalWrite(BLUPIN, HIGH);

  // prompt to start the game
  oled.clearDisplay();
  oled.setTextSize(4);
  oled.setTextColor(WHITE);
  oled.setCursor(35,18);
  oled.print("GO!");
  oled.setTextSize(1);
  oled.setCursor(25,50);
  oled.print("press to start");
  oled.display();

  // wait for the button push
  while ( digitalRead(BTNPIN)){ delay(10); }
  digitalWrite(BLUPIN, LOW);

  // initialize the target and player
  // note: it is possible to set PLAYER_R and TARGET_R relative to the
  //       size of the display that place_target or place_player will loop
  //       infinitely. Don't do that.
  place_target();
  place_player();
  draw_field();
}

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
  Serial.println(d);
}

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
  
  if (result == 1){ // Victory!
      digitalWrite(GRNPIN, HIGH);
      sprintf(cry, "VICTORY!");
      ex=tx; ey=ty;
  } else {          // Defeat!
      digitalWrite(REDPIN, HIGH);
      sprintf(cry, "*DEFEAT*");
      ex=px; ey=py;
  }
 
  Serial.print(cry);
  Serial.print(d);
  Serial.print(" vs. ");
  Serial.print(VD);
  Serial.print(" in ");
  Serial.print(round((millis()-us)/1000));
  Serial.println(" seconds");

  // notify display (explode target)
  for (int i=0; i<30; i+=3){
      oled.drawCircle(ex, ey, TARGET_R+i, WHITE);
      oled.display();
      delay(66);
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
  // thumb stick conected here
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  // LED connected here
  pinMode(GRNPIN, OUTPUT);
  pinMode(REDPIN, OUTPUT);

  // good for debugging
  Serial.begin(9600);

  // set up display
  oled.begin();
  
  // max x and y values, leave hree pixels for the status bar
  mx=oled.width()-TARGET_R;
  my=oled.height()-(TARGET_R+3);
  
  // init the game
  init_game();
}

// -----------------------------------------------------------------------------
// LOOP
// -----------------------------------------------------------------------------
void loop() {
  // get tumbstick vector
  int ix = map(analogRead(A0),THUMB_MIN_X,THUMB_MAX_X,3,-3);
  int iy = map(analogRead(A1),THUMB_MIN_Y,THUMB_MAX_Y,-3,3);
  
  // adjust and constrain position
  px+=ix; py+=iy;
  px=px<ZP?ZP:px; px=px>mx?mx:px;
  py=py<ZP?ZP:py; py=py>my?my:py;

  // update status bar every 0.1 seconds or so
  if (millis()>us+100){
    syrial();
    us=millis();
  }
  
  int game=-1;
  if (get_distance() < VD){ game=1; }      // Victory
  if (millis()-init_us > 10000){ game=0; } // Defeat

  if (game > -1) {
    game_over(game);
  }else{
    draw_field();
  }
}
// -----------------------------------------------------------------------------

