#include <FastLED.h>
#include <string>
//Environment variables
#define DATA_PIN    6
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS   197
#define BRIGHTNESS 96
#define FRAMES_PER_SECOND  42

//board inputs
uint8_t gHue = 0;
const int relePin =  2;
const int buttonGx = 3;
const int buttonTx = 4;
const int buttonDx = 5;

//Board Color
CRGB leds[NUM_LEDS];
CRGB red  = CRGB(255,0,0);
CRGB green  = CRGB(0,255,0);
CRGB blue  = CRGB(0,0,255);

//Objects
enum EnergySystem {Gx,Tx,Dx};
class Component{
  public:
    int start;
    int count;
    EnergySystem category;
    void draw(CRGB color){

    };

};
class Station:public Component{
  public:
    Station(int startAt, int pixels, EnergySystem type){
      start = startAt;
      count = pixels;
      category = type;
    };
    void draw(CRGB color){
      fadeToBlackBy( leds + start, count, 15);
      int pos = random16( count); //start at random
      leds[pos + start]=color;
    };

    void clean() {
    int pos = random16(count);
    leds[pos + start].setRGB(0,0,0);
    };
};

enum Direction{forward,backward};
class PowerLine:public Component{
  public:
    int end;
    Direction direction;
    PowerLine(int startAt, int pixels, int endAt,EnergySystem type, Direction dir,){
      start = startAt;
      count = pixels;
      end = endAt;
      category = type;
      direction = dir;


    };

    void draw(CRGB color){
      //init led section
      leds[end+start].setRGB(0,0,0);
      if(direction==forward){
        //if appoint forward
        if(end+start== start){  
          end += 1; 
        }   else if(end+start<start+count-1){
          end += 1;   
          }
        else{
          end = 0;       
          }
      }

      if(direction==backward){
        //apoint backward
        if(count-1+start> start){
          count -= 1;
          end += 1;   
        }else if (count-1+start== start){
          count = end+1;
          end = 0;    
        }
      }
      //set color
      leds[end+start]=color;
    }
};

class System{
  private:
    CRGB color;
  public:
    Station stations;
    PowerLine roads;
    System(Station setStations[],PowerLine setRoads[],CRGB setColor){
      stations = setStations;
      roads = setRoads;
      color = setColor;
    };

    void on(){
      for (Station item: stations) item.draw(color);
      for (Station item: roads) item.draw(color);
      digitalWrite(relePin, LOW);
    };
    void off(){
      for (Station item: stations) item.clean();
      for (Station item: roads) item.clean(); 
      digitalWrite(relePin, HIGH); // Molino eolico
    };
};



//Transmission system

Station txStation[] = {
  Station hydroline(10,3,Tx),
  Station seBiomass(79,8,Tx),
  Station seGeotherm(164,8,Tx)
};

PowerLine txRoad[] = {
  PowerLine aq(36,11,0,Tx,forward),
  PowerLine pq(53,4,0,Tx,forward),
  PowerLine gh(161,3,0,Tx,forward),
  PowerLine qt(92,6,0,Tx,backward),
  PowerLine qu(87,2,0,Tx,backward),
  PowerLine hi(139,3,0,Tx,backward),
  PowerLine lpos(188,3,0,Tx,backward)
};

System transmission(txStation,txRoad,blue);

//Generation system
Station gxStation[]={
  Station eolic(33,3,Gx),
  Station hidrogen(27,3,Gx),
  Station biomass(47,3,Gx),
  Station tidal(6,1,Gx),
  Station minihydro(89,3,Gx),
  Station thermo(98,5,Gx),
  Station fotovoltaic(142,13,Gx),
  Station geothermic(155,6,Gx),
  Station netbilling(192,2,Gx)
};

PowerLine gxRoad[] = {
  PowerLine ab(30,3,0,Gx,backward)
};

System generation(gxStation,gxRoad,red);

//Distribution system 
Station dxStation[] = {
    Station car(0,1,Dx),
    Station petrol(1,5,Dx),
    Station town(57,6,Dx),
    Station industry(66,7,Dx),
    Station urban(176,9,Dx),
    Station buildA(112,4,Dx),
    Station buildB(103,8,Dx),
    Station city(116,10,Dx),
    Station lumA(111,1,Dx),
    Station lumB(126,1,Dx),
    Station lumC(193,1,Dx),
    Station lumD(196,1,Dx),
    Station tesla(194,2,Dx),
    Station gnl(17,8,Dx)
};

PowerLine dxRoad[] ={
  PowerLine ds(7,3,0,Dx,forward),
  PowerLine bc(25,2,0,Dx,forward),
  PowerLine fh(172,4,0,Dx,forward),
  PowerLine cs(13,4,0,Dx,backward),
  PowerLine oq(63,3,0,Dx,backward),
  PowerLine qr(73,6,0,Dx,backward),
  PowerLine hr(127,12,0,Dx,backward),
  PowerLine fpos(185,3,0,Dx,backward),
};

System distribution(dxStation,dxRoad,blue);



unsigned long Timer = 0;
unsigned long currentTime = 0;
int intervale = 201;

//Arduino functions

void setup() {
  //Main function
  delay(2000);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  //Test init
  pinMode(buttonGx, INPUT_PULLUP);
  pinMode(buttonTx, INPUT_PULLUP);
  pinMode(buttonDx, INPUT_PULLUP);
  pinMode(relePin, OUTPUT);
  
}

void loop(){
//repeat
  currentTime = millis();
  int generationSwitch = digitalRead(buttonGx); //red switch
  int transimissionSwitch = digitalRead(buttonTx); //green switch
  int distributionSwitch = digitalRead(buttonDx); //blue switch
  
//Generation Energy 
  if(generationSwitch == LOW){
    generation.on()
  }else{
    //off
    generation.off()
  
    }
    
//Transimission Energy
  if(transimissionSwitch == LOW){
    transmission.on()
  }
  else{
    transmission.off()
    }  

//---------------DISTRIBUCION-AZUL----------------  
  if(distributionSwitch == LOW){
    distribution.on()
  }
  else{ 
    distribution.off()
    }  

  if(currentTime-Timer >intervale){
   Timer=currentTime;}

  FastLED.show();  
}