# Maqueta interactiva del Sistema Eléctrico Nacional

Código de Arduino para una maqueta física educativa del sistema eléctrico chileno. La maqueta
representa, con una tira de LEDs y un pequeño motor, el recorrido de la energía desde las fuentes de
**generación** hasta los consumidores, pasando por la **transmisión** y la **distribución**.

Tres botones permiten encender cada etapa por separado (o combinarlas), y la maqueta responde
iluminando las zonas correspondientes con su color y animando el flujo de energía a lo largo de los
caminos y líneas del modelo, como un fluido que recorre cada tramo.

- **Generación** → rojo
- **Transmisión** → verde
- **Distribución** → azul

El objetivo es que estudiantes y público general puedan *ver* de dónde viene la electricidad, cómo
viaja por el país y cómo llega a las ciudades, industrias y hogares.

## ¿Qué hace el programa?

| Control | Pin | Efecto en la maqueta |
| --- | --- | --- |
| Botón 1 (generación) | 3 | Ilumina en rojo las centrales y fuentes (eólica, biomasa, solar, termosolar, geotérmica, mini-hidro, hidrógeno verde, autogeneración, boya) y anima el camino hacia la red. Además activa el relé del motor del molino eólico. |
| Botón 2 (transmisión) | 4 | Ilumina en verde las hidrolineras, subestaciones y líneas de transmisión, con pulsos con estela que recorren cada tramo. |
| Botón 3 (distribución) | 5 | Ilumina en azul la ciudad: poblados, fábricas, edificios, luminarias, circulación y autos eléctricos, con pulsos recorriendo la red de distribución. |

Los botones son independientes: se puede mostrar una sola etapa o las tres a la vez. Al soltar un
botón, su etapa se apaga progresivamente.

La maqueta usa **197 LEDs WS2812B** direccionables, por lo que cada central, línea, edificio o
luminaria es un rango de píxeles con posición y tamaño propios.

## Hardware

| Elemento | Detalle |
| --- | --- |
| Placa | Arduino de 5 V (AVR). El repositorio modelo NANO MEGA329P; definir la placa al compilar. |
| Tira de LEDs | 197 × WS2812B, orden de color `GRB`, conectada al pin 6. |
| Botones | 3 pulsadores con `INPUT_PULLUP` en los pines 3, 4 y 5 (presionado = `LOW`). |
| Relé | Módulo de relé en el pin 2, **activo en bajo**, para el motor del molino eólico. |
| Motores | El sketch maneja un motor mediante el relé (molino). Otros movimientos, si existen, no están en este código. |

Notas de montaje:

- Alimentar la tira con una fuente externa de 5 V y unir su tierra con la del Arduino; 197 LEDs
  superan lo que conviene tomar del regulador de la placa. Con estelas y ondas hay más LEDs encendidos
  a la vez que con la versión anterior, así que conviene una fuente de al menos 3 A.
- El relé se activa con `LOW` (coincide con muchos módulos de relé chinos) y se desactiva con `HIGH`.
- El programa espera 2 segundos al arrancar (`delay(2000)` en `setup()`), útil para que la fuente
  estabilice antes de encender los LEDs.

## Cómo está organizado el código

Todo vive en `maqueta-interactiva.ino`:

1. **Configuración**: pines, cantidad de LEDs, brillo (`96`), colores de cada etapa y los arreglos que
   describen cada zona de la maqueta.
2. **Tabla `caminos[]`**: los 16 tramos animados, con etapa, sentido, color, fase, posición y
   velocidad propia.
3. **`setup()`**: inicializa FastLED, los botones, el relé y la posición/velocidad inicial de los
   caminos.
4. **`loop()`**: corre a `FRAMES_PER_SECOND` (60), calcula el tiempo real transcurrido y, para cada
   etapa:
   - si el botón está presionado, dibuja sus zonas fijas y actualiza sus caminos;
   - si no, las va apagando con un desvanecido.
5. **Funciones de dibujo**:
   - `drawGeneracion()`, `drawTransmision()` y `drawDistribucion()` encienden un LED al azar dentro de
     una zona y aplican un desvanecido al resto, produciendo un parpadeo tipo "energía fluyendo".
   - `actualizarGrupo()` avanza la posición de cada camino y llama a `dibujarCamino()`, que pinta la
     cabeza, la estela y la onda de fondo.
   - `clean()` apaga un LED al azar de una zona en cada cuadro, de modo que las zonas se apagan
     gradualmente en lugar de golpe.

### Animación de los caminos

Cada tramo es un pulso con estela que recorre la tira:

- **Cabeza**: el LED más cercano a la posición del pulso se mezcla con blanco (`nblend(..., 96)`).
- **Estela**: tabla `TRAIL[]` con 12 niveles, interpolada con `lerp8by8` para que el brillo cambie de
  forma continua aunque la cabeza esté entre dos LEDs. En tramos cortos la estela se comprime para que
  el movimiento se note igual.
- **Onda de fondo**: un `sin8()` que viaja por el tramo (`faseOnda`) y mantiene viva la zona entre
  pulsos; su amplitud es `AMPLITUD_ONDA`.
- **Velocidad variable**: un tempo global (`beatsin8`, un ciclo cada ~12 s) multiplica la velocidad
  base de cada tramo, que además oscila con su propia fase. Al completar una vuelta, cada tramo sortea
  una velocidad base nueva.
- **Sentido**: `dir = +1` avanza hacia índices mayores y `dir = -1` al revés, conservando el sentido
  físico original de cada línea.

### Formato de los arreglos de LEDs

Cada zona se declara con los píxeles que ocupa:

```cpp
int SOLAR[] = {142, 13};   // inicia en el LED 142 y ocupa 13 LEDs
int AQ[]    = {36, 11};    // tramo animado: inicia en 36 y ocupa 11
```

- Todos los arreglos usan `{primer LED, cantidad de LEDs}`; los tramos animados se declaran además en
  la tabla `caminos[]`.
- El estado de la animación (posición, velocidad, fase) vive en `struct Camino`, así que estos arreglos
  son solo datos y no se modifican en tiempo de ejecución.
- Los índices corresponden a la posición física sobre la maqueta impresa. Si se cambia el orden o el
  largo de un arreglo, la animación deja de coincidir con el modelo físico.

## Instalación y carga

1. Instalar la biblioteca **FastLED** (Gestor de bibliotecas del IDE de Arduino, o
   `arduino-cli lib install FastLED`). No está incluida en el repositorio.
2. Arduino exige que el archivo principal `.ino` esté dentro de una carpeta con su mismo nombre. En
   este repositorio la carpeta es `maqueta-energia` y el archivo `maqueta_interactiva.ino`, así que hay
   que copiar o enlazar el archivo a una carpeta llamada `maqueta_interactiva/` antes de abrirlo o
   compilarlo.
3. Seleccionar la placa correspondiente al hardware real (no viene definida en el repositorio) y el
   puerto, y cargar el sketch.

## Personalización

- **Colores**: `colorGeneracion`, `colorTransmision`, `colorDistribucion` al inicio del archivo.
- **Brillo**: `BRIGHTNESS` (0–255).
- **Velocidad del flujo**: `VEL_MIN` y `VEL_MAX` (velocidad base de cada tramo, en décimas de LED por
  segundo por LED de largo), `TEMPO_MIN`/`TEMPO_MAX` (respiración global) y `FRAMES_PER_SECOND`
  (suavidad general).
- **Estela y onda**: `TRAIL[]`, `TRAIL_LEN` (largo de la cola), `AMPLITUD_ONDA` (brillo de la onda de
  fondo) y `FADE_CAMINO`/`FADE_APAGADO` (qué tan rápido se desvanece el tramo).
- **Parpadeo de las zonas fijas**: el desvanecido en `fadeToBlackBy(..., 15)` de `drawGeneracion()`,
  `drawTransmision()` y `drawDistribucion()`.
- **Cantidad de LEDs**: `NUM_LEDS`; si se reduce, hay que revisar que ningún arreglo apunte más allá
  del último LED.
- **Pines**: `DATA_PIN`, `Boton1..3` y `relePin`.
- El relé está invertido respecto del botón 1: se activa mientras la generación está encendida y se
  apaga en caso contrario.

## Limitaciones conocidas

- No hay pruebas automatizadas ni compilación en CI: la verificación es compilar y probar en la
  maqueta.
- El movimiento se calcula con `millis()`, no con interrupciones de timer: si una etapa se siente
  demasiado rápida o lenta, ajustar `VEL_*`/`TEMPO_*` antes que tocar la lógica.
- Los comentarios, nombres y mensajes de commit están en español; conviene mantener esa convención.

## Créditos

Desarrollado en abril de 2022, versión V02, en el marco de la licitación de maquetas interactivas del
Sistema Eléctrico Nacional para el **Ministerio de Energía de Chile**.
