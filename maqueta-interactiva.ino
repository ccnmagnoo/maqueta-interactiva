//Este código fue desarrollado en Abril de 2022 en el marco de la licitación de maquetas interactivas del sistema eléctrico nacional para el Ministerio de energía de Chile.
//V02
//Los caminos se animan como un fluido: cabeza brillante, estela y onda de fondo, con velocidad variable.

#include "FastLED.h"

#define DATA_PIN    6
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS   197
CRGB leds[NUM_LEDS];

#define BRIGHTNESS         96
#define FRAMES_PER_SECOND  60

int Boton1 = 3;
int Boton2 = 4;
int Boton3 = 5;
const int relePin =  2;

CRGB colorGeneracion  = CRGB(255,0,0);
CRGB colorTransmision  = CRGB(0,255,0);
CRGB colorDistribucion  = CRGB(0,0,255);

//Cada zona es {primer LED, cantidad de LEDs} sobre la maqueta física.
//Los tramos que se animan se declaran además en la tabla "caminos".

//-------------------TRANSMISION verde_________________________________
int HIDROLINERA[] = {10, 3};
int SUBESTACIONBIOMASA[] = {79, 8};
int SUBESTACIONGEO[] = {164, 8};

int AQ[] = {36,11};
int PQ[] = {53,4};
int QU[] = {87,2};
int QT[] = {92,6};
int HI[] = {139,3};
int GH[] = {161,3};
int LPOS[] = {188,3};

//-------------------GENERACION rojo__________________________________

int AB[] = {30,3};
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
int DS[] = {7,3};
int CS[] = {13,4};
int BC[] = {25,2};
int POBLADO[] = {57, 6};
int OQ[] = {63,3};
int FABRICA[] = {66, 7};
int QR[] = {73,6};
int EDIFICIO3[] = {103, 8};
int LUMINARIA1[] = {111, 1};
int EDIFICIO2[] = {112, 4};
int CIRCULOCIUDAD[] = {116, 10};
int LUMINARIA2[] = {126, 1};
int HR[] = {127,12};
int FH[] = {172,4};
int ZONAURBANA[] = {176,9};
int FPOS[] = {185,3};
int LUMINARIA3[] = {193,1};
int LUMINARIA4[] = {196,1};
int AUTOELECTRICO[] = {194,2};

//-------------------ANIMACION DE CAMINOS (fluido)--------------------

#define GRUPO_GENERACION    0
#define GRUPO_TRANSMISION   1
#define GRUPO_DISTRIBUCION  2

#define TRAIL_LEN     12    // LEDs que ocupa la estela
#define FADE_CAMINO   26    // desvanecido por cuadro mientras el tramo está activo
#define FADE_APAGADO  60    // desvanecido por cuadro al soltar el botón
#define AMPLITUD_ONDA 56    // brillo máximo de la onda de fondo (0-255)
#define TEMPO_MIN     176   // respiración global lenta: 0.69x la velocidad base
#define TEMPO_MAX     255   // 1.0x
#define VEL_MIN       10    // velocidad base mínima, décimas de LED/s por LED de largo
#define VEL_MAX       20    // velocidad base máxima

//Brillo de la estela según la distancia a la cabeza. Cambiar aquí para colas más largas.
static const uint8_t TRAIL[TRAIL_LEN] = {255, 190, 140, 102, 74, 52, 36, 24, 15, 9, 5, 2};

struct Camino {
  uint8_t  grupo;   // etapa a la que pertenece
  int*     seg;     // {primer LED, cantidad}
  int8_t   dir;     // +1 avanza hacia índices mayores, -1 retrocede
  CRGB*    color;
  uint8_t  fase;    // desfase de onda y tempo, para que no vayan sincronizados
  uint16_t pos;     // posición 8.8 (1/256 de LED)
  uint16_t base;    // velocidad base 8.8 LED/s
};

Camino caminos[] = {
  //Generación (rojo)
  { GRUPO_GENERACION,   AB,  -1, &colorGeneracion,   17, 0, 0 },

  //Transmisión (verde)
  { GRUPO_TRANSMISION,  AQ,  +1, &colorTransmision,  31, 0, 0 },
  { GRUPO_TRANSMISION,  PQ,  +1, &colorTransmision,  74, 0, 0 },
  { GRUPO_TRANSMISION,  QU,  -1, &colorTransmision, 111, 0, 0 },
  { GRUPO_TRANSMISION,  QT,  -1, &colorTransmision, 148, 0, 0 },
  { GRUPO_TRANSMISION,  HI,  -1, &colorTransmision, 185, 0, 0 },
  { GRUPO_TRANSMISION,  GH,  +1, &colorTransmision, 222, 0, 0 },
  { GRUPO_TRANSMISION,  LPOS, -1, &colorTransmision,  3, 0, 0 },

  //Distribución (azul)
  { GRUPO_DISTRIBUCION, DS,  +1, &colorDistribucion,  41, 0, 0 },
  { GRUPO_DISTRIBUCION, CS,  -1, &colorDistribucion,  79, 0, 0 },
  { GRUPO_DISTRIBUCION, BC,  +1, &colorDistribucion, 117, 0, 0 },
  { GRUPO_DISTRIBUCION, OQ,  -1, &colorDistribucion, 155, 0, 0 },
  { GRUPO_DISTRIBUCION, QR,  -1, &colorDistribucion, 193, 0, 0 },
  { GRUPO_DISTRIBUCION, HR,  -1, &colorDistribucion, 231, 0, 0 },
  { GRUPO_DISTRIBUCION, FH,  +1, &colorDistribucion,  13, 0, 0 },
  { GRUPO_DISTRIBUCION, FPOS, -1, &colorDistribucion,  51, 0, 0 },
};

const uint8_t NUM_CAMINOS = sizeof(caminos) / sizeof(caminos[0]);

void dibujarCamino(Camino &c, uint8_t faseOnda);
void actualizarGrupo(uint8_t grupo, bool activo, uint8_t dtMs, uint8_t faseOnda, uint16_t tempo);

void iniciarCaminos() {
  for (uint8_t k = 0; k < NUM_CAMINOS; k++) {
    Camino &c = caminos[k];
    c.pos = 0;
    c.base = (uint16_t)c.seg[1] * random8(VEL_MIN, VEL_MAX) * 256UL / 10;
  }
}

void setup() {
  delay(2000);

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);

  pinMode(Boton1, INPUT_PULLUP);
  pinMode(Boton2, INPUT_PULLUP);
  pinMode(Boton3, INPUT_PULLUP);
  pinMode(relePin, OUTPUT);

  iniciarCaminos();
}

void loop()
{
  static uint32_t lastFrame = 0;
  uint32_t now = millis();
  uint32_t dt = now - lastFrame;

  if (dt < (1000UL / FRAMES_PER_SECOND)) {
    return;
  }
  lastFrame = now;
  if (dt > 100) {
    dt = 100;
  }

  bool genActiva  = digitalRead(Boton1) == LOW;
  bool transActiva = digitalRead(Boton2) == LOW;
  bool distActiva = digitalRead(Boton3) == LOW;

  uint8_t faseOnda = (uint8_t)(millis() >> 3);
  uint16_t tempo = beatsin8(5, TEMPO_MIN, TEMPO_MAX);

//---------------GENERACION-ROJO----------------

  if(genActiva){
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
    clean(TERMOSOLAR[0],TERMOSOLAR[1]);
    clean(SOLAR[0],SOLAR[1]);
    clean(GEOTERMICA[0], GEOTERMICA[1]);
    clean(AUTOGENERACION[0], AUTOGENERACION[1]);

    digitalWrite(relePin, HIGH); // Molino eolico
  }

//---------------TRANSMISION-VERDE----------------

  if(transActiva){
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
  if(distActiva){
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

  actualizarGrupo(GRUPO_GENERACION, genActiva, (uint8_t)dt, faseOnda, tempo);
  actualizarGrupo(GRUPO_TRANSMISION, transActiva, (uint8_t)dt, faseOnda, tempo);
  actualizarGrupo(GRUPO_DISTRIBUCION, distActiva, (uint8_t)dt, faseOnda, tempo);

  FastLED.show();
}


void dibujarCamino(Camino &c, uint8_t faseOnda)
{
  int start = c.seg[0];
  int len   = c.seg[1];
  int span  = len << 8;

  fadeToBlackBy(leds + start, len, FADE_CAMINO);

  //Posición de la cabeza en 8.8, en el sentido de avance del tramo.
  int headFx = (c.dir < 0) ? ((len - 1) << 8) - (int)c.pos : (int)c.pos;

  //En tramos cortos la estela se comprime para que igual se note el movimiento.
  int trailSpan = (len < TRAIL_LEN) ? len : TRAIL_LEN;
  uint32_t trailRange = (uint32_t)trailSpan << 8;

  for (int i = 0; i < len; i++) {
    //Distancia circular (8.8) desde la cabeza hasta este LED, siempre hacia atrás.
    int dFx = (c.dir < 0) ? ((i << 8) - headFx) : (headFx - (i << 8));
    if (dFx < 0) {
      dFx += span;
    }

    //Estela suavizada: interpola la tabla para que el brillo no salte entre LEDs.
    uint8_t brillo = 0;
    if ((uint32_t)dFx < trailRange) {
      uint16_t idx = (uint32_t)dFx * (TRAIL_LEN - 1) * 256 / (trailRange - 1);
      uint8_t i0 = idx >> 8;
      uint8_t fr = idx & 0xFF;
      if (i0 >= TRAIL_LEN - 1) {
        brillo = TRAIL[TRAIL_LEN - 1];
      } else {
        brillo = lerp8by8(TRAIL[i0], TRAIL[i0 + 1], fr);
      }
    }

    int wavePos = (c.dir < 0) ? -i * 24 : i * 24;
    uint8_t onda = (sin8((uint8_t)(wavePos + faseOnda + c.fase)) * AMPLITUD_ONDA) >> 8;
    brillo = qadd8(brillo, onda);
    if (brillo == 0) {
      continue;
    }

    //El LED más cercano a la cabeza se mezcla con blanco para dar el núcleo brillante.
    CRGB col = (dFx < 160) ? nblend(*c.color, CRGB::White, 96) : *c.color;
    col.nscale8_video(brillo);
    leds[start + i] += col;
  }
}

void actualizarGrupo(uint8_t grupo, bool activo, uint8_t dtMs, uint8_t faseOnda, uint16_t tempo)
{
  for (uint8_t k = 0; k < NUM_CAMINOS; k++) {
    Camino &c = caminos[k];
    if (c.grupo != grupo) {
      continue;
    }

    if (!activo) {
      fadeToBlackBy(leds + c.seg[0], c.seg[1], FADE_APAGADO);
      continue;
    }

    //Variación suave de velocidad propia de cada tramo (0.88x - 1.0x).
    uint8_t local = 224 + (sin8((uint8_t)(millis() / 16) + c.fase) >> 3);

    uint32_t vel = (uint32_t)c.base * tempo >> 8;
    vel = vel * local >> 8;

    c.pos += (uint16_t)((vel * dtMs) / 1000);

    uint16_t span = (uint16_t)c.seg[1] << 8;
    if (c.pos >= span) {
      c.pos %= span;
      //Al cerrar cada vuelta se elige una velocidad base nueva para ese tramo.
      c.base = (uint16_t)c.seg[1] * random8(VEL_MIN, VEL_MAX) * 256UL / 10;
    }

    dibujarCamino(c, faseOnda);
  }
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

void clean(int startpixel, int pixelcount)
{
    int pos = random16( pixelcount);
    leds[pos + startpixel].setRGB(0,0,0);
}
