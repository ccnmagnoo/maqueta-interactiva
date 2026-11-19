#include <FastLED.h>

//-------------------HARDWARE-----------------------------------------
#define DATA_PIN    6
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS   197
#define BRIGHTNESS 96
#define FRAMES_PER_SECOND  42

//-------------------ENTRADAS-----------------------------------------
const int relePin   = 2;   // activo en bajo: LOW enciende el molino eólico
const int buttonGx  = 3;   // generación (naranja)
const int buttonTx  = 4;   // transmisión (verde)
const int buttonDx  = 5;   // distribución (lavanda)

//-------------------COLORES------------------------------------------
CRGB leds[NUM_LEDS];
CRGB naranja = CRGB(255, 154, 0);
CRGB lavanda = CRGB(96, 97, 182);
CRGB verde   = CRGB(183, 216, 148);

//-------------------ANIMACION (fluido)-------------------------------
#define TRAIL_LEN       12     // LEDs que ocupa la estela
#define AMPLITUD_ONDA   48     // brillo de la onda de fondo (0-255)
#define TEMPO_MIN      176     // respiración global lenta: 0.69x la velocidad base
#define TEMPO_MAX      255     // 1.0x
#define VEL_MIN         10     // velocidad base: px/s por cada 10 px de largo
#define VEL_MAX         20
#define RAMP_MS        250     // rampa de encendido/apagado de cada zona

//Brillo de la estela según la distancia a la cabeza.
static const uint8_t TRAIL[TRAIL_LEN] = {255, 190, 140, 102, 74, 52, 36, 24, 15, 9, 5, 2};

//-------------------COMPONENTES--------------------------------------
enum EnergySystem { Gx, Tx, Dx };
enum Direction { forward, backward };

class Component {
  public:
    int start;              // primer LED de la zona en la maqueta
    int count;              // cantidad de LEDs de la zona
    EnergySystem category;
    uint8_t brillo;         // 0..255, envolvente de encendido/apagado
    uint8_t fase;           // desfase propio, para que no vayan sincronizados

    //Rampa suave: la zona se enciende y se apaga en RAMP_MS, sin saltos.
    void subirBrillo(bool activo, uint8_t dtMs) {
      uint8_t paso = (uint8_t)((uint16_t)255 * dtMs / RAMP_MS);
      if (paso == 0) paso = 1;
      brillo = activo ? qadd8(brillo, paso) : qsub8(brillo, paso);
    }

    virtual void draw(CRGB color, uint8_t dtMs, uint8_t faseOnda, uint16_t tempo, bool activo) = 0;
    virtual void iniciar() = 0;
    virtual ~Component() {}
};

//Zona que respira: central, subestación, edificio o luminaria.
class Station : public Component {
  public:
    Station(int startAt, int pixels, EnergySystem type) {
      start = startAt;
      count = pixels;
      category = type;
      brillo = 0;
      fase = 0;
    }

    void iniciar() override {
      fase = random8();
      brillo = 0;
    }

    void draw(CRGB color, uint8_t dtMs, uint8_t faseOnda, uint16_t tempo, bool activo) override {
      subirBrillo(activo, dtMs);
      if (count <= 0) return;

      //Respiración lenta de toda la zona, compartiendo el tempo global.
      uint8_t respira = scale8(96 + (sin8((uint8_t)((faseOnda >> 2) + fase)) >> 1), tempo);
      respira = scale8(respira, brillo);

      for (int i = 0; i < count; i++) {
        //La zona queda encendida con un brillo que ondula suavemente, sin apagarse.
        uint8_t nivel = 128 + (sin8((uint8_t)(i * 32 + faseOnda + fase)) >> 1);
        nivel = scale8(nivel, respira);
        leds[start + i] = color;
        leds[start + i].nscale8_video(nivel);
      }

      if (brillo > 100 && random8() < 12) {   // destello ocasional sobre la zona
        leds[start + random16(count)] = color;
      }
    }
};

//Tramo por el que viaja un pulso: cabeza brillante, estela y onda de fondo.
//La posición vive aquí, nunca se tocan start/count: son posiciones físicas.
class PowerLine : public Component {
  private:
    uint16_t pos;                   // posición 8.8 dentro del tramo (1/256 de LED)
    uint16_t base;                  // velocidad base 8.8, en LED/s
    Direction direction;

  public:
    PowerLine(int startAt, int pixels, EnergySystem type, Direction dir) {
      start = startAt;
      count = pixels;
      category = type;
      direction = dir;
      pos = 0;
      base = 0;
      brillo = 0;
      fase = 0;
    }

    void iniciar() override {
      fase = random8();
      brillo = 0;
      pos = 0;
      base = (uint16_t)((uint32_t)count * random8(VEL_MIN, VEL_MAX) * 256UL / 10);
    }

    void draw(CRGB color, uint8_t dtMs, uint8_t faseOnda, uint16_t tempo, bool activo) override {
      subirBrillo(activo, dtMs);
      if (count <= 0) return;

      //Variación propia de velocidad (0.88x - 1.0x) y respiración global.
      uint8_t local = 224 + (sin8((uint8_t)((millis() >> 4) + fase)) >> 3);
      uint32_t vel = (uint32_t)base * tempo >> 8;
      vel = vel * local >> 8;

      //Avance en 8.8 según el tiempo real, no según cuadros.
      uint16_t span = (uint16_t)(count << 8);
      uint32_t destino = pos + vel * dtMs / 1000UL;
      if (destino >= span) {
        destino %= span;
        //Al cerrar cada vuelta se sortea una velocidad nueva para ese tramo.
        base = (uint16_t)((uint32_t)count * random8(VEL_MIN, VEL_MAX) * 256UL / 10);
      }
      pos = (uint16_t)destino;

      //Posición de la cabeza en 8.8, en el sentido de avance del tramo.
      int headFx = (direction == backward) ? ((count - 1) << 8) - (int)pos : (int)pos;
      int spanFx = count << 8;
      if (headFx < 0) headFx += spanFx;   // la cabeza da la vuelta al tramo

      //En tramos cortos la estela se comprime para que igual se note el movimiento.
      int trailSpan = (count < TRAIL_LEN) ? count : TRAIL_LEN;
      uint32_t trailRange = (uint32_t)trailSpan << 8;

      CRGB nucleo = nblend(color, CRGB::White, 96);   // corazón blanco del pulso

      for (int i = 0; i < count; i++) {
        //Distancia circular (8.8) desde la cabeza hasta este LED, siempre hacia atrás.
        int dFx = (direction == backward) ? ((i << 8) - headFx) : (headFx - (i << 8));
        if (dFx < 0) dFx += spanFx;

        //Estela suavizada: interpola la tabla para que el brillo no salte entre LEDs.
        uint8_t b = 0;
        if ((uint32_t)dFx < trailRange) {
          uint16_t idx = (uint32_t)dFx * (TRAIL_LEN - 1) * 256 / (trailRange - 1);
          uint8_t i0 = idx >> 8;
          uint8_t fr = idx & 0xFF;
          b = (i0 >= TRAIL_LEN - 1) ? TRAIL[TRAIL_LEN - 1] : lerp8by8(TRAIL[i0], TRAIL[i0 + 1], fr);
        }

        //Onda tenue de fondo, en contrafase con el sentido del tramo.
        int wavePos = (direction == backward) ? -i * 24 : i * 24;
        uint8_t onda = (sin8((uint8_t)(wavePos + faseOnda + fase)) * AMPLITUD_ONDA) >> 8;
        b = qadd8(b, onda);
        b = scale8(b, brillo);

        //Se reescribe el LED en cada cuadro: la estela no se acumula ni satura.
        leds[start + i] = (dFx < 160) ? nucleo : color;
        leds[start + i].nscale8_video(b);
      }
    }
};

//Agrupa las zonas y los tramos de una etapa. Guarda punteros a los arreglos
//globales para no copiarlos ni mutarlos.
class System {
  private:
    CRGB color;
    Station *stations;
    uint8_t stationCount;
    PowerLine *roads;
    uint8_t roadCount;

  public:
    template <size_t N, size_t M>
    System(Station (&setStations)[N], PowerLine (&setRoads)[M], CRGB setColor)
      : color(setColor),
        stations(setStations),
        stationCount(N),
        roads(setRoads),
        roadCount(M) {}

    void iniciar() {
      for (uint8_t i = 0; i < stationCount; i++) stations[i].iniciar();
      for (uint8_t i = 0; i < roadCount; i++) roads[i].iniciar();
    }

    void actualizar(bool activo, uint8_t dtMs, uint8_t faseOnda, uint16_t tempo) {
      for (uint8_t i = 0; i < stationCount; i++) stations[i].draw(color, dtMs, faseOnda, tempo, activo);
      for (uint8_t i = 0; i < roadCount; i++) roads[i].draw(color, dtMs, faseOnda, tempo, activo);
    }
};

//-------------------TRANSMISION--------------------------------------
Station txStation[] = {
  Station(10, 3, Tx),   // hydroline
  Station(79, 8, Tx),   // seBiomass
  Station(164, 8, Tx)   // seGeotherm
};

PowerLine txRoad[] = {
  PowerLine(36, 11, Tx, forward),   // aq
  PowerLine(53, 4, Tx, forward),   // pq
  PowerLine(161, 3, Tx, forward),   // gh
  PowerLine(92, 6, Tx, backward),   // qt
  PowerLine(87, 2, Tx, backward),   // qu
  PowerLine(139, 3, Tx, backward),   // hi
  PowerLine(188, 3, Tx, backward)   // lpos
};

System transmission(txStation, txRoad, verde);

//-------------------GENERACION---------------------------------------
Station gxStation[] = {
  Station(33, 3, Gx),   // eolic
  Station(27, 3, Gx),   // hidrogen
  Station(47, 3, Gx),   // biomass
  Station(6, 1, Gx),   // tidal
  Station(89, 3, Gx),   // minihydro
  Station(98, 5, Gx),   // thermo
  Station(142, 13, Gx),   // fotovoltaic
  Station(155, 6, Gx),   // geothermic
  Station(192, 2, Gx)   // netbilling
};

PowerLine gxRoad[] = {
  PowerLine(30, 3, Gx, backward)   // ab
};

System generation(gxStation, gxRoad, naranja);

//-------------------DISTRIBUCION-------------------------------------
Station dxStation[] = {
  Station(0, 1, Dx),   // car
  Station(1, 5, Dx),   // petrol
  Station(57, 6, Dx),   // town
  Station(66, 7, Dx),   // industry
  Station(176, 9, Dx),   // urban
  Station(112, 4, Dx),   // buildA
  Station(103, 8, Dx),   // buildB
  Station(116, 10, Dx),   // city
  Station(111, 1, Dx),   // lumA
  Station(126, 1, Dx),   // lumB
  Station(193, 1, Dx),   // lumC
  Station(196, 1, Dx),   // lumD
  Station(194, 2, Dx),   // tesla
  Station(17, 8, Dx)   // gnl
};

PowerLine dxRoad[] = {
  PowerLine(7, 3, Dx, forward),   // ds
  PowerLine(25, 2, Dx, forward),   // bc
  PowerLine(172, 4, Dx, forward),   // fh
  PowerLine(13, 4, Dx, backward),   // cs
  PowerLine(63, 3, Dx, backward),   // oq
  PowerLine(73, 6, Dx, backward),   // qr
  PowerLine(127, 12, Dx, backward),   // hr
  PowerLine(185, 3, Dx, backward)   // fpos
};

System distribution(dxStation, dxRoad, lavanda);

//-------------------CUADRO-------------------------------------------
uint32_t Timer = 0;

void setup() {
  delay(2000);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(buttonGx, INPUT_PULLUP);
  pinMode(buttonTx, INPUT_PULLUP);
  pinMode(buttonDx, INPUT_PULLUP);
  pinMode(relePin, OUTPUT);
  digitalWrite(relePin, HIGH);   // molino apagado al arrancar

  //Fases y velocidades distintas en cada arranque.
  randomSeed(analogRead(A0));
  generation.iniciar();
  transmission.iniciar();
  distribution.iniciar();
}

void loop() {
  uint32_t ahora = millis();
  uint32_t delta = ahora - Timer;
  if (delta < (1000UL / FRAMES_PER_SECOND)) return;
  Timer = ahora;
  if (delta > 100) delta = 100;          // tras un atasco no se salta media animación

  bool generacion = (digitalRead(buttonGx) == LOW);
  bool transmision = (digitalRead(buttonTx) == LOW);
  bool distribucion = (digitalRead(buttonDx) == LOW);

  uint8_t dt = (uint8_t)delta;
  uint8_t faseOnda = (uint8_t)(ahora >> 3);            // onda viajera (~2 s por ciclo)
  uint16_t tempo = beatsin8(5, TEMPO_MIN, TEMPO_MAX);  // respiración global (~12 s)

  generation.actualizar(generacion, dt, faseOnda, tempo);
  transmission.actualizar(transmision, dt, faseOnda, tempo);
  distribution.actualizar(distribucion, dt, faseOnda, tempo);

  //El relé solo depende del botón de generación.
  digitalWrite(relePin, generacion ? LOW : HIGH);

  FastLED.show();
}
