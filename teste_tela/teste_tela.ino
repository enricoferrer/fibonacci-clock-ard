// Teste da tela: alterna entre MODO A e MODO B a cada 5 segundos.
// Anote em qual modo os nomes combinam com as cores.
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

Adafruit_ILI9341 tft(&SPI, 2, 15, -1);  // DC = 2, CS = 15, sem pino de reset

void faixa(int y, uint16_t cor, uint16_t corTexto, const char* nome) {
  tft.fillRect(0, y, 320, 50, cor);
  tft.setTextColor(corTexto);
  tft.setTextSize(3);
  tft.setCursor(10, y + 13);
  tft.print(nome);
}

void desenhar(const char* modo) {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(10, 8);
  tft.print(modo);
  faixa(40, ILI9341_RED, ILI9341_WHITE, "VERMELHO");
  faixa(90, ILI9341_GREEN, ILI9341_BLACK, "VERDE");
  faixa(140, ILI9341_BLUE, ILI9341_WHITE, "AZUL");
  faixa(190, ILI9341_WHITE, ILI9341_BLACK, "BRANCO");
}

void setup() {
  Serial.begin(115200);
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);  // liga a luz de fundo da tela
  SPI.begin(14, 12, 13, 15);  // SCK, MISO, MOSI, CS
  tft.begin(20000000);
  tft.setRotation(1);  // tela deitada
  Serial.println("Teste da tela iniciado");
}

void loop() {
  tft.invertDisplay(false);
  desenhar("MODO A");
  Serial.println("MODO A (INVERTER_CORES = false)");
  delay(5000);

  tft.invertDisplay(true);
  desenhar("MODO B");
  Serial.println("MODO B (INVERTER_CORES = true)");
  delay(5000);
}
