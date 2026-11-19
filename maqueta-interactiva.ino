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
const int buttonGx  = 3;   // generación (rojo)
const int buttonTx  = 4;   // transmisión (verde)
const int buttonDx  = 5;   // distribución (azul)

//-------------------COLORES------------------------------------------
CRGB leds[NUM_LEDS];
CRGB red  = CRGB(255, 154, 0);
CRGB blue = CRGB(96, 97, 182);
CRGB green = CRGB(183, 216, 148);

//-------------------COMPONENTES--------------------------------------
enum EnergySystem { Gx, Tx, Dx };
enum Direction { forward, backward };

class Component {
  public:
    int start;              // primer LED de la zona en la maqueta
    int count;              // cantidad de LEDs de la zona
    EnergySystem category;

    virtual void draw(CRGB color) = 0;
    virtual void clean() = 0;
    virtual ~Component() {}
};

//Zona que se ilumina con destellos aleatorios (central, subestación, edificio).
class Station : public Component {
  public:
    Station(int startAt, int pixels, EnergySystem type) {
      start = startAt;
      count = pixels;
      category = type;
    }

    void draw(CRGB color) override {
      if (count <= 0) return;
      fadeToBlackBy(leds + start, count, 15);
      int pos = random16(count);      // se enciende un punto al azar
      leds[start + pos] = color;
    }

    void clean() override {
      if (count <= 0) return;
      int pos = random16(count);
      leds[start + pos] = CRGB::Black;
    }
};

//Tramo por el que viaja un pulso. La posición vive aquí, nunca se tocan
//start/count: son posiciones físicas de la maqueta.
class PowerLine : public Component {
  private:
    int pos;                        // cabeza del pulso, 0..count-1
    Direction direction;
  public:
    PowerLine(int startAt, int pixels, EnergySystem type, Direction dir) {
      start = startAt;
      count = pixels;
      category = type;
      direction = dir;
      pos = (dir == forward) ? 0 : count - 1;   // el pulso nace en su extremo
    }

    void draw(CRGB color) override {
      if (count <= 0) return;
      leds[start + pos] = CRGB::Black;            // borra la cabeza anterior
      pos += (direction == forward) ? 1 : -1;
      if (pos >= count) pos = 0;
      if (pos < 0) pos = count - 1;
      leds[start + pos] = color;
    }

    void clean() override {
      if (count <= 0) return;
      leds[start + pos] = CRGB::Black;
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

    void on() {
      for (uint8_t i = 0; i < stationCount; i++) stations[i].draw(color);
      for (uint8_t i = 0; i < roadCount; i++) roads[i].draw(color);
    }

    void off() {
      for (uint8_t i = 0; i < stationCount; i++) stations[i].clean();
      for (uint8_t i = 0; i < roadCount; i++) roads[i].clean();
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

System transmission(txStation, txRoad, blue);

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

System generation(gxStation, gxRoad, red);

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

System distribution(dxStation, dxRoad, green);

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
}

void loop() {
  uint32_t ahora = millis();
  if (ahora - Timer < (1000UL / FRAMES_PER_SECOND)) return;
  Timer = ahora;

  bool generacion = (digitalRead(buttonGx) == LOW);
  bool transmision = (digitalRead(buttonTx) == LOW);
  bool distribucion = (digitalRead(buttonDx) == LOW);

  if (generacion) generation.on();
  else generation.off();

  if (transmision) transmission.on();
  else transmission.off();

  if (distribucion) distribution.on();
  else distribution.off();

  //El relé solo depende del botón de generación.
  digitalWrite(relePin, generacion ? LOW : HIGH);

  FastLED.show();
}
