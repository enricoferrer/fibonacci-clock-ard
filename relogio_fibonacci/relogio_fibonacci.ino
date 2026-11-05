// Relogio Fibonacci para a placa ESP32-2432S028 (tela de 2,8")
#include <SPI.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Nome e senha do Wi-Fi ficam na aba config_wifi.h (fora do Git)
#include "config_wifi.h"

// ============ CONFIGURACAO: edite so esta parte ============
// Fuso horario. Brasilia = "<-03>3". Manaus = "<-04>4". Acre = "<-05>5".
const char* FUSO_HORARIO = "<-03>3";

const bool INVERTER_CORES = true;         // MODO A = false, MODO B = true
const bool TROCAR_VERMELHO_AZUL = false;  // true se vermelho e azul vierem trocados
const uint8_t ROTACAO_TELA = 1;           // 1 ou 3 (3 vira a imagem de ponta-cabeca)
const bool MOSTRAR_HORA_DIGITAL = true;   // false esconde a hora em texto
// ===========================================================

// Pinos da placa
const int PINO_LUZ_TELA = 21;
const int PINO_LED_VERMELHO = 4;
const int PINO_LED_VERDE = 16;
const int PINO_LED_AZUL = 17;

Adafruit_ILI9341 tft(&SPI, 2, 15, -1);  // DC = 2, CS = 15, sem pino de reset

// O que cada quadrado representa
enum Funcao : uint8_t { NENHUM = 0, HORA = 1, MINUTO = 2, AMBOS = 3 };

const int QUANTIDADE = 5;
const int VALORES[QUANTIDADE] = {1, 1, 2, 3, 5};

// Layout em retangulo de 8 x 5 unidades: coluna, linha e lado de cada quadrado
const int UNIDADE = 40;      // 40 pixels: o relogio ocupa 320 x 200
const int MARGEM_TOPO = 20;  // centraliza os 200 pixels na tela de 240
const int ESPACO = 2;        // separacao entre os quadrados
const int LAYOUT[QUANTIDADE][3] = {
  {2, 0, 1},  // quadrado de valor 1
  {2, 1, 1},  // quadrado de valor 1
  {0, 0, 2},  // quadrado de valor 2
  {0, 2, 3},  // quadrado de valor 3
  {3, 0, 5},  // quadrado de valor 5
};

// Guarda o que ja esta na tela, para redesenhar so o que mudou (evita piscar)
uint8_t naTela[QUANTIDADE] = {255, 255, 255, 255, 255};
int ultimoMinuto = -1;

// ---------- Logica do relogio ----------

// Testa as 4^5 = 1024 combinacoes e fica com a que usa menos quadrados azuis.
// A hora 12 e mostrada como soma 12. Os minutos sao truncados: 7:24 vira 7:20.
void calcularFuncoes(int hora24, int minuto, Funcao resultado[QUANTIDADE]) {
  int horaAlvo = hora24 % 12;
  if (horaAlvo == 0) horaAlvo = 12;
  int minutoAlvo = minuto / 5;

  int menosAzuis = QUANTIDADE + 1;
  for (int combinacao = 0; combinacao < 1024; combinacao++) {
    int somaHora = 0;
    int somaMinuto = 0;
    int azuis = 0;
    for (int i = 0; i < QUANTIDADE; i++) {
      int funcao = (combinacao >> (2 * i)) & 3;
      if (funcao & HORA) somaHora += VALORES[i];
      if (funcao & MINUTO) somaMinuto += VALORES[i];
      if (funcao == AMBOS) azuis++;
    }
    if (somaHora == horaAlvo && somaMinuto == minutoAlvo && azuis < menosAzuis) {
      menosAzuis = azuis;
      for (int i = 0; i < QUANTIDADE; i++) {
        resultado[i] = static_cast<Funcao>((combinacao >> (2 * i)) & 3);
      }
    }
  }
}

// ---------- Desenho na tela ----------

uint16_t corDaFuncao(uint8_t funcao) {
  uint16_t vermelho = TROCAR_VERMELHO_AZUL ? ILI9341_BLUE : ILI9341_RED;
  uint16_t azul = TROCAR_VERMELHO_AZUL ? ILI9341_RED : ILI9341_BLUE;
  switch (funcao) {
    case HORA: return vermelho;
    case MINUTO: return ILI9341_GREEN;
    case AMBOS: return azul;
    default: return ILI9341_WHITE;
  }
}

void desenharQuadrado(int i, uint8_t funcao) {
  int x = LAYOUT[i][0] * UNIDADE + ESPACO;
  int y = LAYOUT[i][1] * UNIDADE + MARGEM_TOPO + ESPACO;
  int lado = LAYOUT[i][2] * UNIDADE - 2 * ESPACO;
  tft.fillRect(x, y, lado, lado, corDaFuncao(funcao));
}

void mostrarMensagem(const char* linha1, const char* linha2) {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 90);
  tft.print(linha1);
  tft.setCursor(10, 120);
  tft.print(linha2);
}

void atualizarRelogio(const struct tm& agora) {
  Funcao funcoes[QUANTIDADE];
  calcularFuncoes(agora.tm_hour, agora.tm_min, funcoes);
  for (int i = 0; i < QUANTIDADE; i++) {
    if (naTela[i] != funcoes[i]) {
      desenharQuadrado(i, funcoes[i]);
      naTela[i] = funcoes[i];
    }
  }

  char texto[16];
  snprintf(texto, sizeof(texto), "%02d:%02d", agora.tm_hour, agora.tm_min);
  Serial.printf("Hora: %s\n", texto);
  if (MOSTRAR_HORA_DIGITAL) {
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // o fundo apaga o texto anterior
    tft.setTextSize(2);
    tft.setCursor(130, 222);
    tft.print(texto);
  }
}

// ---------- Wi-Fi e hora da internet ----------

void conectarWiFi() {
  mostrarMensagem("Conectando ao Wi-Fi...", "");
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_NOME, WIFI_SENHA);

  unsigned long inicio = millis();
  bool avisou = false;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (!avisou && millis() - inicio > 30000) {
      mostrarMensagem("Sem Wi-Fi. Confira nome,", "senha e rede de 2,4 GHz");
      Serial.println("Wi-Fi nao conectou: confira nome, senha e rede de 2,4 GHz");
      avisou = true;
    }
  }
  Serial.println("Wi-Fi conectado");
}

void sincronizarHora() {
  mostrarMensagem("Buscando a hora...", "");
  configTzTime(FUSO_HORARIO, "a.st1.ntp.br", "pool.ntp.org");
  struct tm agora;
  while (!getLocalTime(&agora, 1000)) {
    Serial.println("Aguardando a hora da internet...");
  }
  Serial.println("Hora sincronizada");
}

// ---------- Programa principal ----------

void setup() {
  Serial.begin(115200);

  // Apaga o LED colorido da parte de tras (ele acende com nivel baixo)
  pinMode(PINO_LED_VERMELHO, OUTPUT);
  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_AZUL, OUTPUT);
  digitalWrite(PINO_LED_VERMELHO, HIGH);
  digitalWrite(PINO_LED_VERDE, HIGH);
  digitalWrite(PINO_LED_AZUL, HIGH);

  pinMode(PINO_LUZ_TELA, OUTPUT);
  digitalWrite(PINO_LUZ_TELA, HIGH);

  SPI.begin(14, 12, 13, 15);  // SCK, MISO, MOSI, CS
  tft.begin(20000000);
  tft.setRotation(ROTACAO_TELA);
  tft.invertDisplay(INVERTER_CORES);

  conectarWiFi();
  sincronizarHora();
  tft.fillScreen(ILI9341_BLACK);
}

void loop() {
  struct tm agora;
  if (getLocalTime(&agora, 100) && agora.tm_min != ultimoMinuto) {
    ultimoMinuto = agora.tm_min;
    atualizarRelogio(agora);
  }
  delay(1000);
}
