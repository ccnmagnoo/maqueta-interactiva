//Este código fue desarrollado en Abril de 2022 en el marco de la licitación de maquetas interactivas del sistema eléctrico nacional para el Ministerio de energía de Chile. 
//V02


#include "FastLED.h"

#define DATA_PIN    6
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS   197
CRGB leds[NUM_LEDS];

#define BRIGHTNESS         96
#define FRAMES_PER_SECOND  42

uint8_t gHue = 0;
int Boton1 = 3;
int Boton2 = 4;
int Boton3 = 5;
const int relePin =  2;

CRGB colorGeneracion  = CRGB(255,0,0);
CRGB colorTransmision  = CRGB(0,255,0);
CRGB colorDistribucion  = CRGB(0,0,255);

//-------------------TRANSMISION verde_________________________________
int HIDROLINERA[] = {10, 3};
int SUBESTACIONBIOMASA[] = {79, 8};
int SUBESTACIONGEO[] = {164, 8};

int AQ[] = {36,11,0};
int PQ[] = {53,4,0};
int QU[] = {87,2,0};
int QT[] = {92,6,0};
int HI[] = {139,3,0};
int GH[] = {161,3,0};
int LPOS[] = {188,3,0};

//-------------------GENERACION rojo__________________________________

int AB[] = {30,3,0};
int EOLICO[] = {33,3};
int HIDROGENOVERDE[] = {27,3};
int BIOMASA[] = {47,6};
int BOYA[] = {6,1};
int MINIHIDRO[] = {89,3};
int TERMOSOLAR[] = {98,5};
int SOLAR[] = {142,13};
int GEOTERMICA[] = {155,6};
int AUTOGENERACION[] = {191,2};

//------------------DISTRIBUCION azul_________________________________

int AUTOSUSTENTABLE[] = {0, 1};
int Petroleo[] = {1, 5};
int GNL[] = {17, 8};
int DS[] = {7,3,0};
int CS[] = {13,4,0};
int BC[] = {25,2,0};
int POBLADO[] = {57, 6};
int OQ[] = {63,3,0};
int FABRICA[] = {66, 7};
int QR[] = {73,6,0};
int EDIFICIO3[] = {103, 8};
int LUMINARIA1[] = {111, 1};
int EDIFICIO2[] = {112, 4};
int CIRCULOCIUDAD[] = {116, 10};
int LUMINARIA2[] = {126, 1};
int HR[] = {127,12,0};
int FH[] = {172,4,0};
int ZONAURBANA[] = {176,9};
int FPOS[] = {185,3,0};
int LUMINARIA3[] = {193,1};
int LUMINARIA4[] = {196,1};
int AUTOELECTRICO[] = {194,2};

unsigned long Timer = 0;
unsigned long currentTime = 0;
int intervalo = 201;

void setup() {
  delay(2000);
  
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);

  pinMode(Boton1, INPUT_PULLUP);
  pinMode(Boton2, INPUT_PULLUP);
  pinMode(Boton3, INPUT_PULLUP);
  pinMode(relePin, OUTPUT);
  
}

void loop()
{
  currentTime = millis();
  int button1State = digitalRead(Boton1);
  int button2State = digitalRead(Boton2);
  int button3State = digitalRead(Boton3);
  
//---------------GENERACION-ROJO----------------  

  if(button1State == LOW){
    drawGeneracion( EOLICO[0], EOLICO[1]);
    drawGeneracion( BIOMASA[0], BIOMASA[1]);
    drawGeneracion( BOYA[0], BOYA[1]);
    drawGeneracion( HIDROGENOVERDE[0], HIDROGENOVERDE[1]);
    drawGeneracion( MINIHIDRO[0], MINIHIDRO[1]);
    drawGeneracion( TERMOSOLAR[0], TERMOSOLAR[1]);
    drawGeneracion( SOLAR[0], SOLAR[1]);
    drawGeneracion( GEOTERMICA[0], GEOTERMICA[1]);
    drawGeneracion( AUTOGENERACION[0], AUTOGENERACION[1]);

digitalWrite(relePin, LOW);
  }
  
//APAGAR
  
  else{
    clean( AB[0], AB[1]);
    clean( EOLICO[0], EOLICO[1]);
    clean( BOYA[0], BOYA[1]);
    clean( BIOMASA[0], BIOMASA[1]);
    clean(HIDROGENOVERDE[0], HIDROGENOVERDE [1]);
    clean( MINIHIDRO[0], MINIHIDRO[1]);
    clean(TERMOSOLAR[0], TERMOSOLAR[1]);
    clean(SOLAR[0],SOLAR[1]);
    clean(GEOTERMICA[0], GEOTERMICA[1]); 
    clean(AUTOGENERACION[0], AUTOGENERACION[1]);
    
    digitalWrite(relePin, HIGH); // Molino eolico
    }
    
//---------------TRANSMISION-VERDE----------------   
 
  if(button2State == LOW){
    drawTransmision( HIDROLINERA[0], HIDROLINERA[1]);
    drawTransmision( SUBESTACIONBIOMASA[0], SUBESTACIONBIOMASA[1]);
    drawTransmision( SUBESTACIONGEO[0], SUBESTACIONGEO[1]);
  }

//APAGAR
  else{
    clean( HIDROLINERA[0], HIDROLINERA[1]);
    clean(SUBESTACIONBIOMASA[0], SUBESTACIONBIOMASA[1]);
    clean(SUBESTACIONGEO[0], SUBESTACIONGEO[1]);
    clean(AQ[0],AQ[1]);
    clean(PQ[0],PQ[1]);
    clean(QU[0],QU[1]);
    clean(QT[0],QT[1]);
    clean(HI[0],HI[1]);
    clean(GH[0],GH[1]);
    clean(LPOS[0],LPOS[1]);
  
    }  

//---------------DISTRIBUCION-AZUL----------------  
  if(button3State == LOW){
  drawDistribucion(AUTOSUSTENTABLE[0],AUTOSUSTENTABLE[1]);
  drawDistribucion(Petroleo[0],Petroleo[1]);
  drawDistribucion(GNL[0],GNL[1]);
  drawDistribucion(POBLADO[0],POBLADO[1]);
  drawDistribucion(FABRICA[0],FABRICA[1]);
  drawDistribucion(EDIFICIO3[0],EDIFICIO3[1]);
  drawDistribucion(EDIFICIO2[0],EDIFICIO2[1]);
  drawDistribucion(LUMINARIA1[0],LUMINARIA1[1]);
  drawDistribucion(CIRCULOCIUDAD[0],CIRCULOCIUDAD[1]);
  drawDistribucion(LUMINARIA2[0],LUMINARIA2[1]);
  drawDistribucion(ZONAURBANA[0],ZONAURBANA[1]);
  drawDistribucion(LUMINARIA3[0],LUMINARIA3[1]);
  drawDistribucion(LUMINARIA4[0],LUMINARIA4[1]);
  drawDistribucion(AUTOELECTRICO[0],AUTOELECTRICO[1]);
  }

//APAGAR
  else{ 
   clean(AUTOSUSTENTABLE[0],AUTOSUSTENTABLE[1]);
   clean(Petroleo[0],Petroleo[1]);
   clean(GNL[0],GNL[1]);
   
   clean(DS[0],DS[1]);
   clean(CS[0],CS[1]);
   clean(BC[0],BC[1]);
   clean(POBLADO[0],POBLADO[1]);
   clean(OQ[0],OQ[1]); 
   clean(FABRICA[0],FABRICA[1]);
   clean(QR[0],QR[1]);
   clean(EDIFICIO3[0],EDIFICIO3[1]); 
   clean(EDIFICIO2[0],EDIFICIO2[1]);
   clean(LUMINARIA1[0],LUMINARIA1[1]);
   clean(CIRCULOCIUDAD[0],CIRCULOCIUDAD[1]);
   clean(LUMINARIA2[0],LUMINARIA2[1]);
   clean(HR[0],HR[1]);
   clean(FH[0],FH[1]);
   clean(ZONAURBANA[0],ZONAURBANA[1]);
   clean(FPOS[0],FPOS[1]);
   clean(LUMINARIA3[0],LUMINARIA3[1]);
   clean(LUMINARIA4[0],LUMINARIA4[1]);
   clean(AUTOELECTRICO[0],AUTOELECTRICO[1]);
    }  

   if(currentTime-Timer >intervalo){
   Timer=currentTime;

//---------------+GENERACION-ROJO----------------  
     if(button1State == LOW){  
       drawCaminosGeneracionI(AB);
       } 
//---------------TRANSMISION-VERDE----------------     
     if(button2State == LOW){  
       drawCaminosTransmision(AQ);
       drawCaminosTransmision(PQ);
       drawCaminosTransmisionI(QU);
       drawCaminosTransmisionI(QT);
       drawCaminosTransmisionI(HI);
       drawCaminosTransmision(GH);
       drawCaminosTransmisionI(LPOS);   
       } 

//---------------DISTRIBUCION-AZUL----------------  
     if(button3State == LOW){  
       drawCaminosDistribucion(DS);
       drawCaminosDistribucionI(CS);
       drawCaminosDistribucion(BC);
       drawCaminosDistribucionI(OQ);
       drawCaminosDistribucionI(QR);
       drawCaminosDistribucionI(HR); 
       drawCaminosDistribucion(FH); 
       drawCaminosDistribucionI(FPOS); 
            
       }     
   }
   
  FastLED.show();  
  
   

 // FastLED.delay(1000/FRAMES_PER_SECOND); 
}


void drawGeneracion(int startpixel, int pixelcount) 
{
    fadeToBlackBy( leds + startpixel, pixelcount, 15);
    int pos = random16( pixelcount);
    leds[pos + startpixel]=colorGeneracion;
    }
    
void drawTransmision(int startpixel, int pixelcount) 
{
    fadeToBlackBy( leds + startpixel, pixelcount, 15);
    int pos = random16( pixelcount);
    leds[pos + startpixel]=colorTransmision;
    }

void drawDistribucion(int startpixel, int pixelcount) 
{
    fadeToBlackBy( leds + startpixel, pixelcount, 15);
    int pos = random16( pixelcount);
    leds[pos + startpixel]=colorDistribucion;
    }

void drawCaminosDistribucion(int tramo [2]) 
{
   leds[tramo[2]+tramo[0]].setRGB(0,0,0);
      if(tramo[2]+tramo[0]== tramo[0]){  
     tramo[2] += 1; 
      }   
      else if(tramo[2]+tramo[0]< tramo[0]+tramo[1]-1){
        tramo[2] += 1;   
        }
      else{
        tramo[2] = 0;       
        }
    leds[tramo[2]+tramo[0]]=colorDistribucion;
    }

void drawCaminosDistribucionI(int tramo [2]) 
{
   leds[tramo[1]-1+tramo[0]].setRGB(0,0,0);
     if(tramo[1]-1+tramo[0]> tramo[0]){
        tramo[1] = tramo[1]-1;
        tramo[2] += 1;   
        }
      else if (tramo[1]-1+tramo[0]== tramo[0]){
        tramo[1] = tramo[2]+1;
        tramo[2] = 0;    
        }
    leds[tramo[1]-1+tramo[0]]=colorDistribucion;
    }
void drawCaminosTransmision(int tramo [2]) 
{
   leds[tramo[2]+tramo[0]].setRGB(0,0,0);
      if(tramo[2]+tramo[0]== tramo[0]){  
     tramo[2] += 1; 
      }   
      else if(tramo[2]+tramo[0]< tramo[0]+tramo[1]-1){
        tramo[2] += 1;   
        }
      else{
        tramo[2] = 0;       
        }
    leds[tramo[2]+tramo[0]]=colorTransmision;
    }

void drawCaminosTransmisionI(int tramo [2]) 
{
   leds[tramo[1]-1+tramo[0]].setRGB(0,0,0);
     if(tramo[1]-1+tramo[0]> tramo[0]){
        tramo[1] = tramo[1]-1;
        tramo[2] += 1;   
        }
      else if (tramo[1]-1+tramo[0]== tramo[0]){
        tramo[1] = tramo[2]+1;
        tramo[2] = 0;    
        }
    leds[tramo[1]-1+tramo[0]]=colorTransmision;
    }
void drawCaminosGeneracion(int tramo [2]) 
{
   leds[tramo[2]+tramo[0]].setRGB(0,0,0);
      if(tramo[2]+tramo[0]== tramo[0]){  
     tramo[2] += 1; 
      }   
      else if(tramo[2]+tramo[0]< tramo[0]+tramo[1]-1){
        tramo[2] += 1;   
        }
      else{
        tramo[2] = 0;       
        }
    leds[tramo[2]+tramo[0]]=colorGeneracion;
    }

void drawCaminosGeneracionI(int tramo [2]) 
{
   leds[tramo[1]-1+tramo[0]].setRGB(0,0,0);
     if(tramo[1]-1+tramo[0]> tramo[0]){
        tramo[1] = tramo[1]-1;
        tramo[2] += 1;   
        }
      else if (tramo[1]-1+tramo[0]== tramo[0]){
        tramo[1] = tramo[2]+1;
        tramo[2] = 0;    
        }
    leds[tramo[1]-1+tramo[0]]=colorGeneracion;
    }    
void clean(int startpixel, int pixelcount) 
{
    int pos = random16( pixelcount);
    leds[pos + startpixel].setRGB(0,0,0);
}
