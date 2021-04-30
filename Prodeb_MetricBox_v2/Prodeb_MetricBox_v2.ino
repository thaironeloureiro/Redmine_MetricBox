/*

  The minimum screen size is 320 x 240 as that is the keypad size.

  PINOUT no arquivo Setup21_ILI9488.h
  #define TFT_MISO 19 // (leave TFT SDO disconnected if other SPI devices share MISO)
  #define TFT_MOSI 2
  #define TFT_SCLK 18
  #define TFT_CS   15  // Chip select control pin
  #define TFT_DC   14  // Data Command control pin
  #define TFT_RST  33  // Reset pin (could connect to RST pin)
  #define TOUCH_CS 13     // Chip select pin (T_CS) of touch screen

  TOUCH_CS and SPI_TOUCH_FREQUENCY must be defined in the User_Setup.h file
  for the touch functions to do anything.
*/

#include "FS.h"
#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library
#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_wpa2.h"
#include <JPEGDecoder.h>
#include "logoProdeb_Traiket.h"
#include "avatar_alex.h"
#include "avatar_raimundo.h"
#include "avatar_tarcisio.h"
#include "avatar_thairone.h"
#include "avatar_emerson.h"
#include "avatar_leonardo.h"

// Count how many times the image is drawn for test purposes
//uint32_t icount = 0;
//para tratar caracteres especiais e acentos
#define ARDUINOJSON_DECODE_UNICODE 1
#include <ArduinoJson.h>

bool SerialDebug = false;

// SIM card PIN (leave empty, if not defined)
const char simPIN[]   = "";
#define SMS_TARGET  "123456789"
bool smsSent = false;
// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb
#include <Wire.h>
#include <TinyGsmClient.h>

// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

#define SerialAT  Serial1
//#define DUMP_AT_COMMANDS
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

//retorna o minino de dois valores (a,b)
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

//Credenciais de acesso wifi segura
#define EAP_IDENTITY "fulano.silva@empresa.com.br"
#define EAP_PASSWORD "123456798*$#"
static const char* ssid = "SSID_empresa";

int counter = 0;
const char* test_root_ca = \
                           "-----BEGIN CERTIFICATE-----\n" \
                           "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
                           "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
                           "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
                           "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
                           "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
                           "b20xIFAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
                           "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
                           "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
                           "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
                           "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
                           "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
                           "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
                           "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/CAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
                           "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
                           "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
                           "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
                           "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
                           "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
                           "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
                           "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
                           "-----END CERTIFICATE-----\n";

//host dos servicos
const char* host = "10.1.100.100";
const int httpsPort = 443;

unsigned long TempoUltimaAtualizacao;

TFT_eSPI tft = TFT_eSPI(); // biblioteca personalizada
uint16_t t_x = 0, t_y = 0; // Para armazenar as coordenadas de toque

//constantes utilizadas para a construção do teclado de consulta de chamados
#define KEY_X 40 // Centre of key
#define KEY_Y 96
#define KEY_W 62 // Width and height
#define KEY_H 30
#define KEY_SPACING_X 18 // X and Y gap
#define KEY_SPACING_Y 20
#define KEY_TEXTSIZE 1   // Font size multiplier

// Usando duas fontes, pois os números são bons quando estão em negrito
#define LABEL1_FONT &FreeSansOblique12pt7b // Key label font 1
#define LABEL2_FONT &FreeSansBold12pt7b    // Key label font 2

#define DISP_X 1
#define DISP_Y 10
#define DISP_W 238
#define DISP_H 50
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN

// Comprimento de número, buffer para armazená-lo e índice de caracteres
#define NUM_LEN 6
char numberBuffer[NUM_LEN + 1] = "";
uint8_t numberIndex = 0;

boolean pressed; //se há tecla precionada

// Define meter size as multiplier of 240 pixels wide 1.0 and 1.3333 work OK
#define M_SIZE 1.3333
#define TFT_GREY 0x5AEB

float ltx = 0;    // Saved x coord of bottom of needle
uint16_t osx = M_SIZE * 120, osy = M_SIZE * 120; // Saved x & y coords
uint32_t updateTime = 0;       // time for next update
int old_analog =  -999; // Value last displayed
int valuebar[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int cfd_fechado[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int cfd_avaliar_producao[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int cfd_homologacao[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int cfd_desenvolvimento[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int cfd_backlog[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int chart_leadtime[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


int d = 0;
int wip = 0;
int wip_perc = 0;
int qtd_chamados_corretivos = 0;
int qtd_chamados_adaptativos = 0;
int qtd_chamados_evolutivos = 0;
int qtd_chamados_consultas = 0;
int qtd_total_chamados_aberto = 0;
int leadtime_medio_qtd_chamados = 0;
float leadtime_medio = '0';
float throughput = '0';
float flow_efficiency_numeric = '0';

String leadtime_txt = "";
String waitingtime_txt = "";
String touchtime_txt = "";
String flow_efficiency_txt = "";

int tempocarrossel = 3000;
int telacarrossel = 2;

float taxa_rejeicao = '0';
int qtd_chamados_rejeitados = 0;
int qtd_chamados_rejeitados_Indevidos = 0;
int qtd_chamados_rejeitados_Real = 0;
int tem_rejeicao = 0;
int tem_cancelamento = 0;
int tem_impedimento = 0;
int tem_Liberado = 0;
int qtd_chamados_leadtime = 0;

int qtd_chamados_rejeitados_atual = 0;
int id_chamado_rejeitado = 0;
int id_chamado_rejeitado_sms = 0;

String DiaDev_DataIni = "";
String DiaDev_DataFim = "";
int DiaDev_DiaDev = 0;
float DiaDev_TotalHoras = 0;
String Avatar_username;

int Avatar_QtdChamadosAndamento;
int Avatar_id_chamado_andamento;

//para composição do graico de leadtimes
uint16_t ox = 0, oy = 0;
int ave[5] = {0, 0, 0, 0, 0};
int avec[5] = {0, 0, 0, 0, 0};
int avet[5] = {0, 0, 0, 0, 0};


#define DEG2RAD 0.0174532925

// Create 15 keys for the keypad
char keyLabel[15][5] = {"New", "Del", "Send", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "0", "#" };
uint16_t keyColor[15] = {TFT_RED, TFT_DARKGREY, TFT_DARKGREEN,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE
                        };

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[15];
TFT_eSPI_Button HomeBtn[4];

int Tela = 0;
int TelaAnt = 9;

// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5
#define TFT_GREY 0x2104 // Dark grey 16 bit colour
#include "Alert.h" // Out of range alert icon
uint32_t runTime = -99999;       // time for next update
float reading = '0'; // Value to be displayed
boolean range_error = 0;
int8_t ramp = 1;

bool setPowerBoostKeepOn(int en) {
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en) {
    Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  } else {
    Wire.write(0x35); // 0x37 is default reg value
  }
  return Wire.endTransmission() == 0;
}

//------------------------------------------------------------------------------------------
void setup() {
  // porta serial para monitoramento
  Serial.begin(115200);

  // Inicialização do display TFT
  tft.init();
  tft.setRotation(3);

  // Valores obtidos por outra aplicação de calibração
  //usadas para calibrar a tela
  uint16_t calData[5] = { 263, 3626, 321, 3431, 1 };
  tft.setTouch(calData);

  // Limpa Tela
  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("M E T R I C  B O X - Prodeb");
  tft.println("");
  tft.println("developed by Thairone Loureiro");
  tft.println("");
  tft.println("thairone.loureiro@gmail.com");
  tft.println("");
  tft.println("2019");
  tft.println("");
  tft.print("."); tft.print("."); tft.print(".");
  tft.println("");
  tft.println("Inicializando modem GSM...");

  //---------------- modem gsm
  // Mantem a energia quando estiver usando a bateria
  Wire.begin(I2C_SDA, I2C_SCL);
  bool isOk = setPowerBoostKeepOn(1);

  Serial.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));
  tft.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));
  //  Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Set GSM module baud rate and UART pins
  SerialAT.begin(74880, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);
  // Restart SIM800 module, it takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();
  // use modem.init() if you don't need the complete restart

  // Unlock your SIM card with a PIN if needed
  if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
    modem.simUnlock(simPIN);
  }
  //------------------------------------------


  Serial.print("connecting to ");
  Serial.println(ssid);
  //WiFi.begin(ssid, password);
  tft.println("WIfi ");
  tft.print("connecting to ");
  tft.println(ssid);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA); //init wifi mode
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide identity
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide username --> identity and username is same
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD)); //provide password
  esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT(); //set config settings to default
  esp_wifi_sta_wpa2_ent_enable(&config); //set config settings to enable function
  WiFi.begin(ssid); //connect to wifi

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address set: ");
  Serial.println(WiFi.localIP()); //print LAN IP

  tft.println("");
  tft.println("WiFi connected");
  tft.println("IP address set: ");
  tft.println(WiFi.localIP()); //print LAN IP

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Wifi station connect status:");
    tft.println("Wifi status:");
    tft.println(WiFi.status());
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000)
    {
      Serial.print(".");
      tft.print(".");
      delay(500);
    }

    if (WiFi.status() != WL_CONNECTED)
      ESP.restart();

    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

  }
  delay(1000);

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());


    tft.fillScreen(TFT_BLACK);
    tft.println("WiFi connected");
    tft.println("IP address:");
    tft.println(WiFi.localIP());
  }
  else
  {
    /*
      String smsMessage = String("Falha de conexão WIFI");
      if (modem.sendSMS(SMS_TARGET, smsMessage)) {
      Serial.println(smsMessage);
      //smsSent = true;
      }
      else {
      Serial.println("SMS failed to send");
      }
    */
  }


  TempoUltimaAtualizacao = millis();
  tft.fillScreen(TFT_BLACK);

  int ave[5], avec[5], avet[5];

  Tela = 0;
  drawScreenLogo(0, 0, 0);
}

//------------------------------------------------------------------------------------------

void loop(void) {

  int TeclaHome = touchRead(12);

  tempocarrossel++;
  //Serial.print("tempocarrossel:"); Serial.println(tempocarrossel);

  // obtem as coordenadas do ponto da tela que foi tocado
  pressed = tft.getTouch(&t_x, &t_y);
  // delay(10); // UI debouncing

  if (TeclaHome < 20) { //menu desenho de tela apenas
    // delay(10);
    if (TeclaHome = touchRead(12) < 20) {
      tft.init();
      tempocarrossel = 0;
      drawScreen1(-1, t_x, t_y);
    }
    TelaAnt = Tela;
  }

  if (Tela == 1) {//menu
    drawScreen1(Tela, t_x, t_y);
    TelaAnt = Tela;
  }

  if (Tela == 2) //Tela consulta chamado
  {
    drawScreen2(Tela, t_x, t_y);
    TelaAnt = Tela;
  }

  if (Tela == 3) //Tela que exibe metricas de chamados abertos
  {
    drawScreen3(3, t_x, t_y);
    TelaAnt = Tela;
  }

  if (Tela == 4) //Tela que exibe metricas 2
  {
    drawScreen4(Tela, t_x, t_y);
    TelaAnt = Tela;
  }
  if (Tela == 5) //Tela que exibe taxa de rejeição
  {
    drawScreen5(5, t_x, t_y);
    TelaAnt = Tela;
  }
  /*
    if (Tela == 6) //Tela que exibe CFD
    {
    drawScreen6(6, t_x, t_y);
    // tft.invertDisplay(false);
    TelaAnt = Tela;
    }
  */
  if (Tela == 6) //Tela que exibe Throughput
  {
    drawScreen11(11, t_x, t_y);
    TelaAnt = Tela;
  }
  if (Tela == 7) //Tela que exibe Leadtime
  {
    drawScreen7(7, t_x, t_y);
    // tft.invertDisplay(false);
    TelaAnt = Tela;
  }
  if (Tela == 8) //Tela que exibe Throughput
  {
    drawScreen12(12, t_x, t_y);
    TelaAnt = Tela;
  }
  if (tempocarrossel == 4000) {
    drawScreenLogo(0, 0, 0);
  }
  if (tempocarrossel == 4250) {
    drawScreen3(-1, t_x, t_y);// drawScreen3 - Tela que exibe metricas de chamados abertos
  }
  if (tempocarrossel == 4300) {
    drawScreen4(-1, t_x, t_y);// drawScreen4
  }
  if (tempocarrossel == 5000) {
    drawScreen5(-1, t_x, t_y);    // drawScreen5 - taxa de rejeição
  }
  /*
    if (tempocarrossel == 6000) {
    drawScreen6(-1, t_x, t_y); // drawScreen6 - exibe CFD
    }*/
  if (tempocarrossel == 5700) {
    drawScreen11(-1, t_x, t_y); // drawScreen6 - exibe Throughput
  }
  if (tempocarrossel == 6500) {
    drawScreen8(-1, t_x, t_y); // drawScreen8 - DIADEV
  }
  if (tempocarrossel == 7200) {
    drawScreen7(-1, t_x, t_y); // drawScreen7 - leadtime
  }
  if (tempocarrossel == 7900) {
    drawScreen12(-1, t_x, t_y); // drawScreen7 - leadtime
  }
  if (tempocarrossel == 8600) {
    tft.init();
    drawScreen10(-1, 1, t_x, t_y);
    tempocarrossel = 3500;
  }

}

//------------------------------------------------------------------------------------------
void drawKeypad()
{
  uint8_t row;
  uint8_t col;
  uint8_t b;

  numberIndex = 0;
  numberBuffer[numberIndex] = 0;

  // Draw the keys
  for (row = 0; row < 5; row++) {
    for (col = 0; col < 3; col++) {
      b = col + row * 3;
      if (b < 3) tft.setFreeFont(LABEL1_FONT);
      else tft.setFreeFont(LABEL2_FONT);

      key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                        KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
                        keyLabel[b], KEY_TEXTSIZE);
      key[b].drawButton();
    }
  }
}
//------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------
void drawCard(const char *numChamado)
{
  uint32_t colorCard;

  if (WiFi.status() == WL_CONNECTED) { //if we are connected to Eduroam network
    counter = 0; //reset counter
    if (SerialDebug) {
      Serial.println("Wifi is still connected with IP: ");
      Serial.println(WiFi.localIP());   //inform user about his IP address
    }
  } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
    WiFi.begin(ssid);
  }
  while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
    delay(500);
    if (SerialDebug) Serial.print(".");
    counter++;
    if (counter >= 60) { //30 seconds timeout - reset board
      ESP.restart();
    }
  }

  String url = "http://10.1.100.96/api.redmine/getstatus.php?issueId=" + String(numChamado);
  //StaticJsonDocument<1024> jsonBuffer;

  if (SerialDebug) {
    Serial.print("requesting URL: ");
    Serial.println(url);
  }

  HTTPClient http;
  http.setTimeout(190000);
  http.begin(url);
  http.setConnectTimeout(190000);
  int start = millis();
  int httpCode = http.GET();
  int end = millis();
  if (SerialDebug) Serial.println(String("Effective timeout: ") + (end - start));
  if (httpCode > 0) {
    if (SerialDebug) Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    //file found at server --> on unsucessful connection code will be -1
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      if (SerialDebug) Serial.println(payload);
      // Use arduinojson.org/assistant to compute the capacity.
      //const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
      DynamicJsonDocument jsonBuffer(1024);
      DeserializationError error = deserializeJson(jsonBuffer, payload);
      if (error) {
        if (SerialDebug) {
          Serial.println(F("Parsing failed!"));
          Serial.println(error.c_str());
        }
        return;
      }

      if (jsonBuffer["categoria"] == "Evolutiva")
        colorCard = TFT_YELLOW;
      if (jsonBuffer["categoria"] == "Adaptativa")
        colorCard = TFT_ORANGE;
      if (jsonBuffer["categoria"] == "Corretiva")
        colorCard = TFT_GREEN;
      if (jsonBuffer["categoria"] == "Consulta")
        colorCard = TFT_WHITE;

      tft.fillRoundRect(242, 0, 239, 320, 20, colorCard),
                        tft.setTextFont(0);
      tft.setTextDatum(TC_DATUM);
      tft.setTextColor(TFT_BLACK, colorCard);
      tft.setTextSize(2);

      String projeto = jsonBuffer["projeto"];
      projeto = projeto.substring(0, 17);  projeto.trim();

      tft.setCursor(247, 5, 2);
      if (SerialDebug) Serial.println(projeto);
      tft.println(projeto);

      tft.setCursor(244, 45, 2);
      if (SerialDebug) Serial.println(jsonBuffer["chamado"].as<char*>());
      tft.println(jsonBuffer["chamado"].as<char*>());

      tft.setCursor(244, 85, 2);
      if (SerialDebug) Serial.println(jsonBuffer["categoria"].as<char*>());
      tft.println(jsonBuffer["categoria"].as<char*>());

      tft.setCursor(244, 125, 2);
      if (SerialDebug) Serial.println(jsonBuffer["prioridade"].as<char*>());
      tft.println(jsonBuffer["prioridade"].as<char*>());

      String sit_parte1 = jsonBuffer["situcao"];
      String sit_parte2 = sit_parte1;
      sit_parte1 = sit_parte1.substring(0, 17);  sit_parte1.trim();
      sit_parte2 = sit_parte2.substring(17, 34); sit_parte2.trim();

      tft.setCursor(244, 165, 2);
      if (SerialDebug) Serial.println(sit_parte1);
      tft.println(sit_parte1);
      tft.setCursor(244, 195, 2);
      if (SerialDebug) Serial.println(sit_parte2);
      tft.println(sit_parte2);

      String tag_parte1 = jsonBuffer["tags"];
      String tag_parte2 = tag_parte1;
      tag_parte1 = tag_parte1.substring(0, 17); tag_parte1.trim();
      tag_parte2 = tag_parte2.substring(17, 34); tag_parte2.trim();

      tft.setTextColor(TFT_BLACK, TFT_RED);
      tft.setCursor(244, 235, 2);
      if (SerialDebug) Serial.println(tag_parte1);
      tft.println(tag_parte1);
      tft.setCursor(244, 265, 2);
      if (SerialDebug) Serial.println(tag_parte2);
      tft.println(tag_parte2);
      tft.setTextColor(TFT_BLACK, colorCard);

      tft.setCursor(300, 304, 1);
      if (SerialDebug) Serial.println(jsonBuffer["versao"].as<char*>());
      tft.println(jsonBuffer["versao"].as<char*>());

      tft.setTextSize(1);
    }
  } else {
    if (SerialDebug) Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

}

//------------------------------------------------------------------------------------------

// #########################################################################
//  Draw the analogue meter on the screen
// #########################################################################
void analogMeter(char *label)
{
  // Meter outline
  tft.fillRect(0, 0, 239, 126, TFT_GREY);
  tft.fillRect(5, 3, 230, 119, TFT_WHITE);

  tft.setTextColor(TFT_BLACK);  // Text colour

  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -50; i < 51; i += 5) {
    // Long scale tick length
    int tl = 15;

    // Coodinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (100 + tl) + 120;
    uint16_t y0 = sy * (100 + tl) + 140;
    uint16_t x1 = sx * 100 + 120;
    uint16_t y1 = sy * 100 + 140;

    // Coordinates of next tick for zone fill
    float sx2 = cos((i + 5 - 90) * 0.0174532925);
    float sy2 = sin((i + 5 - 90) * 0.0174532925);
    int x2 = sx2 * (100 + tl) + 120;
    int y2 = sy2 * (100 + tl) + 140;
    int x3 = sx2 * 100 + 120;
    int y3 = sy2 * 100 + 140;

    // Yellow zone limits
    //if (i >= -50 && i < 0) {
    //  tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_YELLOW);
    //  tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
    //}

    // Green zone limits
    if (i >= 0 && i < 25) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_ORANGE);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_ORANGE);
    }

    // Orange zone limits
    if (i >= 25 && i < 50) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_RED);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_RED);
    }

    // Short scale tick length
    if (i % 25 != 0) tl = 8;

    // Recalculate coords incase tick lenght changed
    x0 = sx * (100 + tl) + 120;
    y0 = sy * (100 + tl) + 140;
    x1 = sx * 100 + 120;
    y1 = sy * 100 + 140;

    // Draw tick
    tft.drawLine(x0, y0, x1, y1, TFT_BLACK);

    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0) {
      // Calculate label positions
      x0 = sx * (100 + tl + 10) + 120;
      y0 = sy * (100 + tl + 10) + 140;
      switch (i / 25) {
        case -2: tft.drawCentreString("0", x0, y0 - 12, 2); break;
        case -1: tft.drawCentreString("25", x0, y0 - 9, 2); break;
        case 0: tft.drawCentreString("50", x0, y0 - 6, 2); break;
        case 1: tft.drawCentreString("75", x0, y0 - 9, 2); break;
        case 2: tft.drawCentreString("100", x0, y0 - 12, 2); break;
      }
    }

    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);
    x0 = sx * 100 + 120;
    y0 = sy * 100 + 140;
    // Draw scale arc, don't draw the last part
    if (i < 50) tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
  }

  //tft.drawString("%WIP", 5 + 230 - 40, 119 - 20, 2); // Units at bottom right
  //tft.drawCentreString("%WIP", 120, 70, 4); // Comment out to avoid font 4
  tft.drawString(label, 5 + 230 - 40, 119 - 20, 2); // Units at bottom right
  tft.drawCentreString(label, 120, 70, 4); // Comment out to avoid font 4
  tft.drawRect(5, 3, 230, 119, TFT_BLACK); // Draw bezel line

}


// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void plotNeedle(char *label, int value, byte ms_delay)
{
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  char buf[8]; dtostrf(value, 4, 0, buf);
  tft.drawRightString(buf, 40, 119 - 20, 2);

  if (value < -10) value = -10; // Limit value to emulate needle end stops
  if (value > 110) value = 110;

  if (ms_delay == 0) old_analog = value; // Update immediately id delay is 0

  float sdeg = map(old_analog, -10, 110, -150, -30); // Map value to angle
  // Calcualte tip of needle coords
  float sx = cos(sdeg * 0.0174532925);
  float sy = sin(sdeg * 0.0174532925);

  // Calculate x delta of needle start (does not start at pivot point)
  float tx = tan((sdeg + 90) * 0.0174532925);

  // Re-plot text under needle
  tft.setTextColor(TFT_BLACK);
  //tft.drawCentreString("%WIP", 120, 70, 4); // // Comment out to avoid font 4
  tft.drawCentreString(label, 120, 70, 4); // // Comment out to avoid font 4


  // Store new needle end coords for next erase
  ltx = tx;
  osx = sx * 98 + 120;
  osy = sy * 98 + 140;

  // Draw the needle in the new postion, magenta makes needle a bit bolder
  // draws 3 lines to thicken needle
  tft.drawLine(120 + 20 * ltx - 1, 140 - 20, osx - 1, osy, TFT_RED);
  tft.drawLine(120 + 20 * ltx, 140 - 20, osx, osy, TFT_MAGENTA);
  tft.drawLine(120 + 20 * ltx + 1, 140 - 20, osx + 1, osy, TFT_RED);

}

// #########################################################################
//  Draw a linear meter on the screen
// #########################################################################
void plotLinear(char *label, int x, int y)
{
  int w = 36;
  tft.drawRect(x, y, w, 155, TFT_GREY);
  tft.fillRect(x + 2, y + 19, w - 3, 155 - 38, TFT_WHITE);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawCentreString(label, x + w / 2, y + 2, 2);

  for (int i = 0; i < 110; i += 10)
  {
    tft.drawFastHLine(x + 20, y + 27 + i, 6, TFT_BLACK);
  }

  for (int i = 0; i < 110; i += 50)
  {
    tft.drawFastHLine(x + 20, y + 27 + i, 9, TFT_BLACK);
  }


  tft.drawCentreString("---", x + w / 2, y + 155 - 18, 2);
}

// #########################################################################
//  Adjust 11 linear meter pointer positions
// #########################################################################
void plotPointer(void)
{
  int dy = 187;
  //byte pw = 16;

  tft.setTextColor(TFT_GREEN, TFT_BLACK);


  //if (TempoUltimaAtualizacao>1 && millis() - TempoUltimaAtualizacao > 5000) {
  //TempoUltimaAtualizacao = millis();

  // Move the 9 pointers one pixel towards new value
  for (int i = 0; i < 12; i++)
  {
    char buf[8];
    //delay(500);
    dtostrf(valuebar[i], 4, 0, buf);
    tft.drawRightString(buf, i * 40 + 36 - 5, 187 - 27 + 155 - 18, 2);

    int dx = 3 + 40 * i;
    if (valuebar[i] < 0) valuebar[i] = 0; // Limit value to emulate needle end stops
    if (valuebar[i] > 100) valuebar[i] = 100;

    dy = 187 + 100 - valuebar[i];

    tft.fillTriangle(dx , dy, dx + 3 + 16, dy , dx , dy  - 5, TFT_RED);
    tft.fillTriangle(dx , dy, dx + 3 + 16, dy , dx , dy  + 5, TFT_RED);
  }
}

//------------------------------------------------------
// #########################################################################
// Draw circle segments
// #########################################################################

// x,y == coords of centre of circle
// start_angle = 0 - 359
// sub_angle   = 0 - 360 = subtended angle
// r = radius
// colour = 16 bit colour value

int fillSegment(int x, int y, int start_angle, int sub_angle, int r, unsigned int colour)
{
  // Calculate first pair of coordinates for segment start
  float sx = cos((start_angle - 90) * DEG2RAD);
  float sy = sin((start_angle - 90) * DEG2RAD);
  uint16_t x1 = sx * r + x;
  uint16_t y1 = sy * r + y;

  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + sub_angle; i++) {

    // Calculate pair of coordinates for segment end
    int x2 = cos((i + 1 - 90) * DEG2RAD) * r + x;
    int y2 = sin((i + 1 - 90) * DEG2RAD) * r + y;

    tft.fillTriangle(x1, y1, x2, y2, x, y, colour);

    // Copy segment end to sgement start for next segment
    x1 = x2;
    y1 = y2;
  }
}
//-------------------------------------------------------------------------------------------
//####################################################################################################
// Draw a JPEG on the TFT pulled from a program memory array
//####################################################################################################
void drawArrayJpeg(const uint8_t arrayname[], uint32_t array_size, int xpos, int ypos) {

  int x = xpos;
  int y = ypos;

  JpegDec.decodeArray(arrayname, array_size);
  jpegInfo(); // Print information from the JPEG file (could comment this line out)
  renderJPEG(x, y);
  if (SerialDebug) Serial.println("#########################");
}
//-------------------------------------------------------------------------------------------
//####################################################################################################
// Print image information to the serial port (optional)
//####################################################################################################
void jpegInfo() {
  if (SerialDebug) {
    Serial.println(F("==============="));
    Serial.println(F("JPEG image info"));
    Serial.println(F("==============="));
    Serial.print(F(  "Width      :")); Serial.println(JpegDec.width);
    Serial.print(F(  "Height     :")); Serial.println(JpegDec.height);
    Serial.print(F(  "Components :")); Serial.println(JpegDec.comps);
    Serial.print(F(  "MCU / row  :")); Serial.println(JpegDec.MCUSPerRow);
    Serial.print(F(  "MCU / col  :")); Serial.println(JpegDec.MCUSPerCol);
    Serial.print(F(  "Scan type  :")); Serial.println(JpegDec.scanType);
    Serial.print(F(  "MCU width  :")); Serial.println(JpegDec.MCUWidth);
    Serial.print(F(  "MCU height :")); Serial.println(JpegDec.MCUHeight);
    Serial.println(F("==============="));
  }
}
//-------------------------------------------------------------------------------------------
//####################################################################################################
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
//####################################################################################################
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void renderJPEG(int xpos, int ypos) {

  // retrieve infomration about the image
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while (JpegDec.read()) {

    // save a pointer to the image block
    pImg = JpegDec.pImage ;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;  // Calculate coordinates of top left corner of current MCU
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // calculate how many pixels must be drawn
    uint32_t mcu_pixels = win_w * win_h;

    tft.startWrite();

    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= tft.width() && ( mcu_y + win_h ) <= tft.height())
    {

      // Now set a MCU bounding window on the TFT to push pixels into (x, y, x + width - 1, y + height - 1)
      tft.setAddrWindow(mcu_x, mcu_y, win_w, win_h);

      // Write all MCU pixels to the TFT window
      while (mcu_pixels--) {
        // Push each pixel to the TFT MCU area
        tft.pushColor(*pImg++);
      }

    }
    else if ( (mcu_y + win_h) >= tft.height()) JpegDec.abort(); // Image has run off bottom of screen so abort decoding

    tft.endWrite();
  }

  // calculate how long it took to draw the image
  drawTime = millis() - drawTime;

  // print the results to the serial port
  if (SerialDebug) {
    Serial.print(F(  "Total render time was    : ")); Serial.print(drawTime); Serial.println(F(" ms"));
    Serial.println(F(""));
  }
}



//--------------------------------------------------------------------------------------------
void GetDadosTela3()
{
  if (TempoUltimaAtualizacao > 1 && millis() - TempoUltimaAtualizacao > 3000) {
    TempoUltimaAtualizacao = millis();

    if (WiFi.status() == WL_CONNECTED) { //if we are connected to Eduroam network
      counter = 0; //reset counter
      if (SerialDebug) {
        Serial.println("Wifi is still connected with IP: ");
        Serial.println(WiFi.localIP());   //inform user about his IP address
      }
    } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
      WiFi.begin(ssid);
    }
    while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
      delay(500);
      if (SerialDebug) Serial.print(".");
      counter++;
      if (counter >= 60) { //30 seconds timeout - reset board
        ESP.restart();
      }
    }

    String url = "http://10.1.100.96/api.redmine/metricas_esp32_GSM.php";
    StaticJsonDocument<1024> jsonBuffer;

    if (SerialDebug) {
      Serial.print("requesting URL: ");
      Serial.println(url);
    }

    HTTPClient http;
    http.setTimeout(190000);
    http.begin(url);
    http.setConnectTimeout(190000);
    int start = millis();
    int httpCode = http.GET();
    int end = millis();
    if (SerialDebug) Serial.println(String("Effective timeout: ") + (end - start));
    if (httpCode > 0) {
      if (SerialDebug) Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      //file found at server --> on unsucessful connection code will be -1
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        if (SerialDebug) Serial.println(payload);
        // Use arduinojson.org/assistant to compute the capacity.
        //const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
        DynamicJsonDocument jsonBuffer(1024);
        DeserializationError error = deserializeJson(jsonBuffer, payload);
        if (error) {
          if (SerialDebug) {
            Serial.println(F("Parsing failed!"));
            Serial.println(error.c_str());
          }
          return;
        }

        if (SerialDebug) Serial.println(jsonBuffer["qtd_chamados_Aberto"].as<char*>());

        valuebar[0] = jsonBuffer["qtd_chamados_Novo"].as<int>();
        valuebar[1] = jsonBuffer["qtd_chamados_Aberto"].as<int>();
        valuebar[2] = jsonBuffer["qtd_chamados_Desev_Andamento"].as<int>();
        valuebar[3] = jsonBuffer["qtd_chamados_Desev_Resolvido"].as<int>();
        valuebar[4] = jsonBuffer["qtd_chamados_PreHmlFazer"].as<int>();
        valuebar[5] = jsonBuffer["qtd_chamados_PreHmlFazendo"].as<int>();
        valuebar[6] = jsonBuffer["qtd_chamados_PreHmlFeito"].as<int>();
        valuebar[7] = jsonBuffer["qtd_chamados_Hml_Andamento"].as<int>();
        valuebar[8] = jsonBuffer["qtd_chamados_Hml_Validado"].as<int>();
        valuebar[9] = jsonBuffer["qtd_chamados_AvaliarProducao"].as<int>();
        valuebar[10] = jsonBuffer["qtd_tag_rejeicao"].as<int>();
        valuebar[11] = jsonBuffer["qtd_tag_Liberado"].as<int>();

        old_analog = 0;

        qtd_total_chamados_aberto = jsonBuffer["qtd_total_chamados"].as<int>();
        wip = jsonBuffer["wip"].as<int>();
        wip_perc = jsonBuffer["wip_perc"].as<int>();

        qtd_chamados_corretivos = jsonBuffer["qtd_chamados_corretivos"].as<int>();
        qtd_chamados_adaptativos = jsonBuffer["qtd_chamados_adaptativos"].as<int>();
        qtd_chamados_evolutivos = jsonBuffer["qtd_chamados_evolutivos"].as<int>();
        qtd_chamados_consultas = jsonBuffer["qtd_chamados_consultas"].as<int>();

      }
    } else {
      if (SerialDebug) Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  }
}

void GetTaxaRejeicao()
{
  if (TempoUltimaAtualizacao > 1 && millis() - TempoUltimaAtualizacao > 1000) {
    TempoUltimaAtualizacao = millis();

    if (WiFi.status() == WL_CONNECTED) { //if we are connected to Eduroam network
      counter = 0; //reset counter
      if (SerialDebug) {
        Serial.println("Wifi is still connected with IP: ");
        Serial.println(WiFi.localIP());   //inform user about his IP address
      }
    } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
      WiFi.begin(ssid);
    }
    while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
      delay(500);
      if (SerialDebug) Serial.print(".");
      counter++;
      if (counter >= 60) { //30 seconds timeout - reset board
        ESP.restart();
      }
    }

    String url = "http://10.1.100.96/api.redmine/gettaxarejeicao.php";
    StaticJsonDocument<1024> jsonBuffer;

    if (SerialDebug) {
      Serial.print("requesting URL: ");
      Serial.println(url);
    }

    HTTPClient http;
    http.setTimeout(190000);
    http.begin(url);
    http.setConnectTimeout(190000);
    int start = millis();
    int httpCode = http.GET();
    int end = millis();
    if (SerialDebug) Serial.println(String("Effective timeout: ") + (end - start));
    if (httpCode > 0) {
      if (SerialDebug) Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      //file found at server --> on unsucessful connection code will be -1
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        if (SerialDebug) Serial.println(payload);
        // Use arduinojson.org/assistant to compute the capacity.
        //const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
        DynamicJsonDocument jsonBuffer(1024);
        DeserializationError error = deserializeJson(jsonBuffer, payload);
        if (error) {
          if (SerialDebug) {
            Serial.println(F("Parsing failed!"));
            Serial.println(error.c_str());
          }
          return;
        }

        if (SerialDebug) Serial.println(jsonBuffer["taxa_rejeicao"].as<char*>());

        taxa_rejeicao = jsonBuffer["taxa_rejeicao"].as<float>();
        qtd_chamados_rejeitados = jsonBuffer["qtd_chamados_rejeitados"].as<int>();
        qtd_chamados_rejeitados_Indevidos = jsonBuffer["qtd_chamados_rejeitados_Indevidos"].as<int>();
        qtd_chamados_rejeitados_Real = jsonBuffer["qtd_chamados_rejeitados_Real"].as<int>();
        tem_rejeicao = jsonBuffer["tem_rejeicao"].as<int>();
        tem_cancelamento = jsonBuffer["tem_cancelamento"].as<int>();
        tem_impedimento = jsonBuffer["tem_impedimento"].as<int>();
        tem_Liberado = jsonBuffer["tem_Liberado"].as<int>();
        qtd_chamados_leadtime = jsonBuffer["qtd_chamados_leadtime"].as<int>();

        old_analog = 0;
      }
    } else {
      if (SerialDebug) Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  }
}

void GetChamadoRejeitado()
{
  if (TempoUltimaAtualizacao > 1 && millis() - TempoUltimaAtualizacao > 1000) {
    TempoUltimaAtualizacao = millis();

    if (WiFi.status() == WL_CONNECTED) { //if we are connected to Eduroam network
      counter = 0; //reset counter
      if (SerialDebug) {
        Serial.println("Wifi is still connected with IP: ");
        Serial.println(WiFi.localIP());   //inform user about his IP address
      }
    } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
      WiFi.begin(ssid);
    }
    while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
      delay(500);
      if (SerialDebug) Serial.print(".");
      counter++;
      if (counter >= 60) { //30 seconds timeout - reset board
        ESP.restart();
      }
    }

    String url = "http://10.1.100.96/api.redmine/getchamadorejeitado.php";
    StaticJsonDocument<1024> jsonBuffer;

    if (SerialDebug) {
      Serial.print("requesting URL: ");
      Serial.println(url);
    }

    HTTPClient http;
    http.setTimeout(190000);
    http.begin(url);
    http.setConnectTimeout(190000);
    int start = millis();
    int httpCode = http.GET();
    int end = millis();
    if (SerialDebug) Serial.println(String("Effective timeout: ") + (end - start));
    if (httpCode > 0) {
      if (SerialDebug) Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      //file found at server --> on unsucessful connection code will be -1
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        if (SerialDebug) Serial.println(payload);
        // Use arduinojson.org/assistant to compute the capacity.
        //const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
        DynamicJsonDocument jsonBuffer(1024);
        DeserializationError error = deserializeJson(jsonBuffer, payload);
        if (error) {
          if (SerialDebug) {
            Serial.println(F("Parsing failed!"));
            Serial.println(error.c_str());
          }
          return;
        }

        if (SerialDebug) {
          Serial.println("### id_chamado_rejeitado ###");
          Serial.println(jsonBuffer["id_chamado_rejeitado"].as<int>());
        }

        qtd_chamados_rejeitados_atual = jsonBuffer["qtd_chamados_rejeitados"].as<int>();
        id_chamado_rejeitado = jsonBuffer["id_chamado_rejeitado"].as<int>();
        old_analog = 0;
      }
    } else {
      if (SerialDebug) Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  }
}


void GetLeadtime()
{
  if (TempoUltimaAtualizacao > 1 && millis() - TempoUltimaAtualizacao > 3000) {
    TempoUltimaAtualizacao = millis();

    if (WiFi.status() == WL_CONNECTED) { //if we are connected to Eduroam network
      counter = 0; //reset counter
      if (SerialDebug) {
        Serial.println("Wifi is still connected with IP: ");
        Serial.println(WiFi.localIP());   //inform user about his IP address
      }
    } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
      WiFi.begin(ssid);
    }
    while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
      delay(500);
      if (SerialDebug) Serial.print(".");
      counter++;
      if (counter >= 60) { //30 seconds timeout - reset board
        ESP.restart();
      }
    }

    String url = "http://10.1.100.96/api.redmine/leadtime_esp32.php";
    StaticJsonDocument<1024> jsonBuffer;

    if (SerialDebug) {
      Serial.print("requesting URL: ");
      Serial.println(url);
    }

    HTTPClient http;
    http.setTimeout(190000);
    http.begin(url);
    http.setConnectTimeout(190000);
    int start = millis();
    int httpCode = http.GET();
    int end = millis();
    if (SerialDebug) Serial.println(String("Effective timeout: ") + (end - start));
    if (httpCode > 0) {
      if (SerialDebug) Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      //file found at server --> on unsucessful connection code will be -1
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        if (SerialDebug) Serial.println(payload);
        // Use arduinojson.org/assistant to compute the capacity.
        //const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
        DynamicJsonDocument jsonBuffer(1024);
        DeserializationError error = deserializeJson(jsonBuffer, payload);
        if (error) {
          if (SerialDebug) {
            Serial.println(F("Parsing failed!"));
            Serial.println(error.c_str());
          }
          return;
        }

        if (SerialDebug) Serial.println(jsonBuffer["qtd_chamados_Aberto"].as<char*>());

        // Create a Sine wave for testing
        leadtime_medio = jsonBuffer["leadtime_medio"].as<float>();
        leadtime_medio_qtd_chamados = jsonBuffer["qtd_chamados"].as<int>();
      }
    } else {
      if (SerialDebug) Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  }
}


void GetEficiencia()
{
  if (TempoUltimaAtualizacao > 1 && millis() - TempoUltimaAtualizacao > 3000) {
    TempoUltimaAtualizacao = millis();

    if (WiFi.status() == WL_CONNECTED) { //if we are connected to Eduroam network
      counter = 0; //reset counter
      if (SerialDebug) {
        Serial.println("Wifi is still connected with IP: ");
        Serial.println(WiFi.localIP());   //inform user about his IP address
      }
    } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
      WiFi.begin(ssid);
    }
    while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
      delay(500);
      if (SerialDebug) Serial.print(".");
      counter++;
      if (counter >= 60) { //30 seconds timeout - reset board
        ESP.restart();
      }
    }

    String url = "http://10.1.100.96/api.redmine/geteficiencia_dinamico.php";
    StaticJsonDocument<1024> jsonBuffer;

    if (SerialDebug) {
      Serial.print("requesting URL: ");
      Serial.println(url);
    }

    HTTPClient http;
    http.setTimeout(190000);
    http.begin(url);
    http.setConnectTimeout(190000);
    int start = millis();
    int httpCode = http.GET();
    int end = millis();
    if (SerialDebug) Serial.println(String("Effective timeout: ") + (end - start));
    if (httpCode > 0) {
      if (SerialDebug) Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      //file found at server --> on unsucessful connection code will be -1
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        if (SerialDebug) Serial.println(payload);
        // Use arduinojson.org/assistant to compute the capacity.
        //const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
        DynamicJsonDocument jsonBuffer(1024);
        DeserializationError error = deserializeJson(jsonBuffer, payload);
        if (error) {
          if (SerialDebug) {
            Serial.println(F("Parsing failed!"));
            Serial.println(error.c_str());
          }
          return;
        }

        if (SerialDebug) Serial.println(jsonBuffer["qtd_chamados_leadtime"].as<char*>());

        // Create a Sine wave for testing
        flow_efficiency_numeric = jsonBuffer["flow_efficiency_numeric"].as<float>();
        flow_efficiency_txt = jsonBuffer["flow_efficiency"].as<char*>();
        leadtime_txt = jsonBuffer["leadtime"].as<char*>();
        waitingtime_txt = jsonBuffer["waitingtime"].as<char*>();
        touchtime_txt = jsonBuffer["touchtime"].as<char*>();

      }
    } else {
      if (SerialDebug) Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  }
}

// #########################################################################
// Tela logo
// #########################################################################
void drawScreenLogo(int TelaAtual, int t_x, int t_y) {
  if (TelaAtual == 0)
  {
    tft.fillScreen(TFT_BLACK);
    tft.invertDisplay(false);
    int x = (tft.width()  - 478) / 2 - 1;
    int y = (tft.height() - 300) / 2 - 1;

    drawArrayJpeg(Prodeb, sizeof(Prodeb), x, y); // Draw a jpeg image stored in memory at x,y
    delay(2000);
    Tela = -1;
  }
}
// #########################################################################
// Tela 1 - Tela Home de Menus
// #########################################################################
void drawScreen1(int TelaAtual, int t_x, int t_y) {

  if (TelaAtual != 1)
  {

    Tela = 1;
    // TelaAtual = 1;
    tft.fillScreen(TFT_BLACK);
    tft.invertDisplay(false);

    // Draw keypad background
    tft.fillRect(0, 0, 480, 320, TFT_DARKGREY);

    // Draw number display area and frame
    tft.fillRect(1, 5, 472, 317, TFT_BLACK);
    tft.drawRect(1, 5, 472, 317, TFT_WHITE);


    tft.setFreeFont(LABEL2_FONT);

    HomeBtn[0].initButton(&tft, 120, 45, // x, y, w, h, outline, fill, text
                          200, 55, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                          "Issue", 1);

    HomeBtn[1].initButton(&tft, 340, 45, // x, y, w, h, outline, fill, text
                          200, 55, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                          "Kanban", 1);

    HomeBtn[2].initButton(&tft, 120, 120, // x, y, w, h, outline, fill, text
                          200, 55, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                          "Leadtime", 1);

    HomeBtn[3].initButton(&tft, 340, 120, // x, y, w, h, outline, fill, text
                          200, 55, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                          "Reject", 1);

    HomeBtn[5].initButton(&tft, 120, 195, // x, y, w, h, outline, fill, text
                          200, 55, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                          "Leadtimes", 1);
    /*
      HomeBtn[4].initButton(&tft, 340, 195, // x, y, w, h, outline, fill, text
                           200, 55, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                           "CFD", 1);                          */

    /*HomeBtn[4].initButton(&tft, 340, 195, // x, y, w, h, outline, fill, text
                          200, 55, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                          "Throughput", 1);                           */

    HomeBtn[4].initButton(&tft, 340, 195, // x, y, w, h, outline, fill, text
                          200, 55, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                          "Efficiency", 1);

    HomeBtn[6].initButton(&tft, 120, 270, // x, y, w, h, outline, fill, text
                          200, 55, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                          "DiaDev", 1);

    HomeBtn[7].initButton(&tft, 340, 270, // x, y, w, h, outline, fill, text
                          200, 55, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                          "Andamento", 1);

    HomeBtn[0].press(false);
    HomeBtn[1].press(false);
    HomeBtn[2].press(false);
    HomeBtn[3].press(false);
    HomeBtn[4].press(false);
    HomeBtn[5].press(false);
    HomeBtn[6].press(false);
    HomeBtn[7].press(false);

    HomeBtn[0].drawButton();
    HomeBtn[1].drawButton();
    HomeBtn[2].drawButton();
    HomeBtn[3].drawButton();
    HomeBtn[4].drawButton();
    HomeBtn[5].drawButton();
    HomeBtn[6].drawButton();
    HomeBtn[7].drawButton();
  }

  //Tela Home já desenhada
  if (TelaAtual == 1) {

    // / Check if any key coordinate boxes contain the touch coordinates
    for (uint8_t h = 0; h < 8; h++) {
      if (pressed && HomeBtn[h].contains(t_x, t_y)) {
        HomeBtn[h].press(true);
      } else {
        HomeBtn[h].press(false);
      }
    }
    // Check if any key has changed state
    for (uint8_t h = 0; h < 8; h++) {
      if (HomeBtn[h].justReleased()) HomeBtn[h].drawButton();     // draw normal
      if (HomeBtn[h].justPressed()) {
        HomeBtn[h].drawButton(true);  // draw invert
        if (h == 0) { //Tela que checa chamados
          tempocarrossel = 0;
          drawScreen2(-1, t_x, t_y);
          if (SerialDebug) Serial.println("Abrir tela de consulta de chamados");
        }
        if (h == 1) {  //Indicador 1
          tempocarrossel = 0;
          drawScreen3(-1, t_x, t_y);
          if (SerialDebug) Serial.println("Abrir tela de Indicadores1");
        }
        if (h == 2) {
          tempocarrossel = 0;
          if (SerialDebug) Serial.println("Leadtime");
          drawScreen4(-1, t_x, t_y);
        }
        if (h == 3) {
          tempocarrossel = 0;
          if (SerialDebug) Serial.println("Taxa de Rejeição");
          drawScreen5(-1, t_x, t_y);
        }
        if (h == 4) {
          tempocarrossel = 0;
          //Serial.println("CFD");
          //drawScreen6(-1, t_x, t_y);
          //if(SerialDebug) Serial.println("Throughput");
          //drawScreen11(-1, t_x, t_y);
          if (SerialDebug) Serial.println("Flow Efficiency");
          drawScreen12(-1, t_x, t_y);
        }
        if (h == 5) {
          tempocarrossel = 0;
          if (SerialDebug) Serial.println("LeadTimes");
          drawScreen7(-1, t_x, t_y);
          // tft.invertDisplay(false);
        }
        if (h == 6) {
          tempocarrossel = 0;
          if (SerialDebug) Serial.println("DiaDev");
          drawScreen8(-1, t_x, t_y);
          // tft.invertDisplay(false);
        }
        if (h == 7) {
          tempocarrossel = 0;
          if (SerialDebug) Serial.println("DEV Chamado atual");
          drawScreen10(-1, 1, t_x, t_y);
          // tft.invertDisplay(false);
        }
        HomeBtn[h].press(false);

        // delay(10);
      }
    }

  }
}


// #########################################################################
// Tela 2 - chamados
// #########################################################################
void drawScreen2(int TelaAtual, int t_x, int t_y) {

  if (TelaAtual != 2) {
    Tela = 2;
    // TelaAtual = 2;
    // Clear the screen
    tft.fillScreen(TFT_BLACK);
    tft.invertDisplay(false);

    // Draw keypad background
    tft.fillRect(0, 0, 240, 320, TFT_DARKGREY);
    // Draw number display area and frame
    tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
    tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);

    numberIndex = 0;
    numberBuffer[numberIndex] = 0;
    //delay(100); // UI debouncing
    drawKeypad();
  }
  if (TelaAtual == 2) {        //Tela consulta chamado

    // / Check if any key coordinate boxes contain the touch coordinates
    uint8_t b;
    for (b = 0; b < 15; b++) {
      if (pressed && key[b].contains(t_x, t_y)) {
        key[b].press(true);
      } else {
        key[b].press(false);
      }
    }

    // Check if any key has changed state
    for (b = 0; b < 15; b++) {
      if (b < 3) tft.setFreeFont(LABEL1_FONT);
      else tft.setFreeFont(LABEL2_FONT);

      if (key[b].justReleased()) key[b].drawButton();     // draw normal

      if (key[b].justPressed()) {
        key[b].drawButton(true);  // draw invert

        if (b >= 3) {
          if (numberIndex < NUM_LEN) {
            numberBuffer[numberIndex] = keyLabel[b][0];
            numberIndex++;
            numberBuffer[numberIndex] = 0; // zero terminate
          }
        }

        // Del button, so delete last char
        if (b == 1) {
          numberBuffer[numberIndex] = 0;
          if (numberIndex > 0) {
            numberIndex--;
            numberBuffer[numberIndex] = 0;//' ';
          }
        }

        if (b == 2) {
          if (SerialDebug) Serial.println(numberBuffer);
          drawCard(numberBuffer);
        }
        // we dont really check that the text field makes sense
        // just try to call
        if (b == 0) {
          numberIndex = 0; // Reset index to 0
          numberBuffer[numberIndex] = 0; // Place null in buffer
        }

        // Update the number display field
        tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
        tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
        tft.setTextColor(DISP_TCOLOR);     // Set the font colour

        // Draw the string, the value returned is the width in pixels
        int xwidth = tft.drawString(numberBuffer, DISP_X + 4, DISP_Y + 12);

        // Now cover up the rest of the line up by drawing a black rectangle.  No flicker this way
        // but it will not work with italic or oblique fonts due to character overlap.
        tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);

        delay(10); // UI debouncing
      }
    }
  }

}

// #########################################################################
// Tela 3 - Indicadores Kanban - abertos
// #########################################################################
void drawScreen3(int TelaAtual, int t_x, int t_y) {
  if (TelaAtual != 3) {
    Tela = 3;
    // TelaAtual=3;

    if (SerialDebug) {
      Serial.println("Acesso a drawScreen3");
      Serial.print("TelaAtual: "); Serial.println(TelaAtual);
    }

    tft.setFreeFont(LABEL2_FONT);

    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.invertDisplay(false);
    analogMeter("%WIP"); // Draw analogue meter

    // Draw 6 linear meters
    byte d = 40;
    plotLinear("NO", 0, 160);
    plotLinear("PB", 1 * d, 160);
    plotLinear("DA", 2 * d, 160);
    plotLinear("DR", 3 * d, 160);
    plotLinear("P1", 4 * d, 160);
    plotLinear("P2", 5 * d, 160);
    plotLinear("P3", 6 * d, 160);
    plotLinear("HA", 7 * d, 160);
    plotLinear("HV", 8 * d, 160);
    plotLinear("AP", 9 * d, 160);
    plotLinear("Re", 10 * d, 160);
    plotLinear("Le", 11 * d, 160);

    GetDadosTela3();

  }
  if (TelaAtual == 3) {
    plotPointer(); // It takes aout 3.5ms to plot each gauge for a 1 pixel move, 21ms for 6 gauges

    //plota o indicador do wip
    plotNeedle("%WIP", wip_perc, 0); // It takes between 2 and 12ms to replot the needle with zero delay


    int fatia1 = map(qtd_chamados_corretivos, 0, qtd_total_chamados_aberto, 0, 360);
    int fatia2 = map(qtd_chamados_adaptativos, 0, qtd_total_chamados_aberto, 0, 360);
    int fatia3 = map(qtd_chamados_evolutivos, 0, qtd_total_chamados_aberto, 0, 360);
    int fatia4 = map(qtd_chamados_consultas, 0, qtd_total_chamados_aberto, 0, 360);

    // Draw 4 pie chart segments
    fillSegment(330, 80, 0, fatia1, 75, TFT_GREEN);
    fillSegment(330, 80, fatia1, fatia2, 75, TFT_ORANGE);
    fillSegment(330, 80, fatia1 + fatia2, fatia3, 75, TFT_YELLOW);
    fillSegment(330, 80, fatia1 + fatia2 + fatia3, 360 - (fatia1 + fatia2 + fatia3), 75, TFT_WHITE);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    //tft.setCursor(470, 20, 2);

    char buf[8];
    dtostrf(qtd_chamados_corretivos, 4, 0, buf);
    tft.drawRightString(buf, 465, 32, 2);

    dtostrf(qtd_chamados_adaptativos, 4, 0, buf);
    tft.drawRightString(buf, 465, 62, 2);

    dtostrf(qtd_chamados_evolutivos, 4, 0, buf);
    tft.drawRightString(buf, 465, 93, 2);

    dtostrf(qtd_chamados_consultas, 4, 0, buf);
    tft.drawRightString(buf, 465, 123, 2);

    tft.fillRect(415, 30, 20, 20, TFT_GREEN);
    tft.fillRect(415, 60, 20, 20, TFT_ORANGE);
    tft.fillRect(415, 90, 20, 20, TFT_YELLOW);
    tft.fillRect(415, 120, 20, 20, TFT_WHITE);
    //delay(500);
  }
}


// #########################################################################
// Tela 4 - Leadtime Médio
// #########################################################################
void drawScreen4(int TelaAtual, int t_x, int t_y) {

  if (TelaAtual != 4) {
    Tela = 4;
    TelaAtual = 4;

    tft.setFreeFont(LABEL2_FONT);
    if (SerialDebug) Serial.println("Indicador 2");
    tft.fillScreen(TFT_BLACK);
    tft.invertDisplay(false);

    if (millis() - runTime >= 0L) { // Execute every TBD ms
      runTime = millis();

      // Set the the position, gap between meters, and inner radius of the meters
      int xpos = 0, ypos = 5, gap = 4, radius = 52;

      // Draw a large meter
      xpos = 480 / 2 - 160, ypos = 0, gap = 15, radius = 170;
      reading += (ramp);
      if (reading > 98) ramp = -1;
      if (reading < 0) ramp = 1;

      GetLeadtime();
      reading = leadtime_medio;
      // Comment out above meters, then uncomment the next line to show large meter
      ringMeter(reading, 0, 30, xpos, ypos, radius, " Leadtime ", GREEN2RED); // Draw analogue meter
    }
  }
}


// #########################################################################
// Tela 5 - Taxa de Rejeição
// #########################################################################
void drawScreen5(int TelaAtual, int t_x, int t_y) {

  if (TelaAtual != 5) {
    Tela = 5;
    // TelaAtual=5;

    tft.setFreeFont(LABEL2_FONT);
    if (SerialDebug) {
      Serial.println("Acesso a drawScreen5 - taxa de Rejeição");
      Serial.print("TelaAtual: "); Serial.println(TelaAtual);
    }

    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.invertDisplay(false);
    analogMeter("Tx Rej"); // Draw analogue meter

    GetTaxaRejeicao();
    delay(500);
    GetChamadoRejeitado();
    delay(500);
    if (id_chamado_rejeitado > 0)
    {
      drawCard(String(id_chamado_rejeitado).c_str());

      //Envio de SMS para indicar chamado Rejeitado
      if (id_chamado_rejeitado != id_chamado_rejeitado_sms)
      {
        String smsMessage = String("Chamado Rejeitado: #" + String(id_chamado_rejeitado));
        if (modem.sendSMS(SMS_TARGET, smsMessage)) {
          if (SerialDebug) Serial.println(smsMessage);
          //smsSent = true;
          id_chamado_rejeitado_sms = id_chamado_rejeitado;
        }
        else {
          if (SerialDebug) Serial.println("SMS failed to send");
        }
      }

    }

    if (SerialDebug) {
      Serial.print("id_chamado_rejeitado "); Serial.println(id_chamado_rejeitado);
      Serial.print("Tx Rej"); Serial.println(taxa_rejeicao);
    }
    //}
    //if (TelaAtual == 5) {

    //plota o indicador do wip
    plotNeedle("Tx Rej", taxa_rejeicao, 0);

    /*
      Serial.print("id_chamado_rejeitado "); Serial.println(id_chamado_rejeitado);
      Serial.print("Tx Rej"); Serial.println(taxa_rejeicao);
    */

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    char buf[8];


    tft.drawString("Taxa de Rejeicao: ", 20, 140, 2);
    dtostrf(taxa_rejeicao, 4, 2, buf);
    tft.drawString(buf, 140, 140, 2);


    if (tem_rejeicao > 0) {
      tft.fillRect(10, 200, 20, 20, TFT_RED);
      tft.drawString("Chamado Rejeitado", 100, 200, 2);

    }

    if (tem_cancelamento > 0) {
      tft.drawString("Pedido de Cancelamento", 120, 230, 2);
      tft.fillRect(10, 230, 20, 20, TFT_RED);
    }

    if (tem_impedimento) {
      tft.drawString("Impedimento", 80, 260, 2);
      tft.fillRect(10, 260, 20, 20, TFT_RED);
    }

    if (tem_Liberado) {
      tft.drawString("Liberado em espera", 100, 290, 2);
      tft.fillRect(10, 290, 20, 20, TFT_RED);
    }
    //delay(500);
  }
}

// #########################################################################
// Tela 6 - CFD
// #########################################################################
void drawScreen6(int TelaAtual, int t_x, int t_y) {
  if (TelaAtual != 6) {
    Tela = 6;
    // TelaAtual=5;
    tft.setFreeFont(LABEL2_FONT);

    if (SerialDebug) {
      Serial.println("Acesso a drawScreen6 - CFD");
      Serial.print("TelaAtual: "); Serial.println(TelaAtual);
    }

    //tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.invertDisplay(false);

    GetDadosCFD();

    tft.setRotation(2);
    tft.fillScreen(0xffff);

    for (int Dtx = 0; Dtx < 11; Dtx++) {
      dotchart_10x10(4, cfd_backlog[Dtx], Dtx, 0x7800);
    }
    //delay(5);

    for (int Dtx = 0; Dtx < 11; Dtx++) {
      dotchart_10x10(3, cfd_desenvolvimento[Dtx], Dtx, 0xFDA0);
    }
    //delay(5);
    for (int Dtx = 0; Dtx < 11; Dtx++) {
      dotchart_10x10(2, cfd_homologacao[Dtx], Dtx, 0xF81F);
    }

    //delay(5);

    for (int Dtx = 0; Dtx < 11; Dtx++) {
      dotchart_10x10(1, cfd_avaliar_producao[Dtx], Dtx, 0x07E0);
    }

    //delay(5);

    for (int Dtx = 0; Dtx < 11; Dtx++) {
      dotchart_10x10(0, cfd_fechado[Dtx], Dtx, 0x780F);
    }

    //delay(5);

    tft.setRotation(3);
    tft.setTextSize(1);
    tft.setTextColor(0x01E8);
    tft.setCursor(20, 20);
    tft.print("CFD");

    /*
      int dl = 20;
      for (int i = 0; i < 6; i++) {
      for (uint16_t a = 0; a < 3; a++) {
        tft.drawLine(dl, 40 + a, dl + 10, 40 + a, 0xFB21);
      }
      dl += 16;
      }
    */

    tft.fillRect(10, 50, 20, 20, 0x7800);
    tft.drawString("Bckl", 40, 55, 2);

    tft.fillRect(10, 80, 20, 20, 0xFDA0);
    tft.drawString("Des", 35, 85, 2);

    tft.fillRect(10, 110, 20, 20, 0xF81F);
    tft.drawString("Hml", 40, 110, 2);

    tft.fillRect(10, 140, 20, 20, 0x07E0);
    tft.drawString("Aval", 40, 140, 2);

    tft.fillRect(10, 170, 20, 20, 0x780F);
    tft.drawString("Fec", 40, 175, 2);

    tft.setRotation(2);
    //aveg(1);
    tft.setRotation(3);
    tft.setTextSize(1);
    //tft.fillScreen(0xffff);
    delay(500);
  }
}


// #########################################################################
// Tela 7 - LEADTIMES
// #########################################################################
void drawScreen7(int TelaAtual, int t_x, int t_y) {
  if (TelaAtual != 7) {
    Tela = 7;
    // TelaAtual=5;
    //Serial.println("Acesso a drawScreen7 - Leadtimes");
    //Serial.print("TelaAtual: "); Serial.println(TelaAtual);

    //tft.setRotation(3);
    //tft.fillScreen(TFT_BLACK);
    tft.setFreeFont(LABEL2_FONT);
    tft.invertDisplay(true);
    delay(100);

    GetEficiencia();


    tft.setRotation(2);
    tft.fillScreen(0xffff);

    for (int Dtx = 0; Dtx < 11; Dtx++) {
      dotchart_10x10(1, chart_leadtime[Dtx], Dtx, 0xFDA0);
      dchart_10x10(1, chart_leadtime[Dtx], Dtx);
    }


    //GetLeadtime();
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.setTextColor(0x01E8);
    tft.setCursor(20, 20);
    tft.print("Lead Time: ");
    //tft.setCursor(20, 70);
    //tft.println(leadtime_medio);

    tft.println(leadtime_txt);
    tft.print("   Touch Time: ");
    tft.println(touchtime_txt);
    tft.print("   Waiting Time: ");
    tft.println(waitingtime_txt);
    tft.print("   Flow Efficiency: ");
    tft.println(flow_efficiency_txt);

    delay(100);
    GetDadosLeadtimes();

    /*
      int dl = 20;
      for (int i = 0; i < 6; i++) {
      for (uint16_t a = 0; a < 3; a++) {
        tft.drawLine(dl, 40 + a, dl + 10, 40 + a, 0xFB21);
      }
      dl += 16;
      }
    */


    tft.setRotation(2);
    aveg(1);
    tft.setRotation(3);
    tft.setTextSize(1);
    //tft.fillScreen(0xffff);
    tft.invertDisplay(false);
    delay(500);
  }

}

// #########################################################################
// Tela 8 - DIADEV
// #########################################################################
void drawScreen8(int TelaAtual, int t_x, int t_y) {
  if (TelaAtual != 8) {
    Tela = 8;
    // TelaAtual=5;
    //Serial.println("Acesso a drawScreen7 - Leadtimes");
    //Serial.print("TelaAtual: "); Serial.println(TelaAtual);

    //tft.setRotation(3);
    //tft.fillScreen(TFT_BLACK);
    //tft.invertDisplay(true);

    GetDadosDiaDev();


    tft.setFreeFont(LABEL2_FONT);
    tft.setRotation(3);
    //tft.invertDisplay(true);
    tft.fillScreen(TFT_BLACK);

    tft.setTextSize(2);
    tft.setTextColor(0x01E8);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(20, 50);
    tft.print("DiaDev:");

    tft.setCursor(200, 100);
    //tft.setTextSize(1);
    tft.setTextColor(TFT_YELLOW); //tft.setTextFont(7);
    tft.println(DiaDev_DiaDev);

    //tft.setTextSize(2);
    tft.setTextColor(0x01E8);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(20, 200);
    tft.print("Total de Horas:");

    tft.setCursor(240, 250);

    tft.setTextColor(TFT_YELLOW);// tft.setTextFont(7);
    tft.println(DiaDev_TotalHoras);


    tft.setTextSize(1);
    tft.setCursor(20, 310);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print(DiaDev_DataIni);
    tft.print("   -    ");
    tft.print(DiaDev_DataFim);

    delay(500);
  }

}



// #########################################################################
// Tela 10 - Dev - Chamado atual
// #########################################################################
void drawScreen10(int TelaAtual, int idDev, int t_x, int t_y) {
  if (TelaAtual != 10) {
    Tela = 10;

    tft.setFreeFont(LABEL2_FONT);
    tft.setRotation(3);
    //tft.invertDisplay(true);
    tft.fillScreen(TFT_BLACK);

    //tft.fillScreen(TFT_BLACK);
    //tft.invertDisplay(false);
    //int x = (tft.width()  - 478) / 2 - 1;
    //int y = (tft.height() - 300) / 2 - 1;
    int x = 20;
    int y = 20;

    /*
      #include "avatar_alex.h"
      #include "avatar_raimundo.h"
      #include "avatar_tarcisio.h"
      #include "avatar_thairone.h"
      #include "avatar_uhitlei.h"
      #include "avatar_victor.h"
      #include "avatar_vovo.h"
    */

    /*
      1 id: 939,name: "Alex"
      2 id: 631,name: "Raimundo"
      3 id: 328, name: "Tarcisio"
      4 id: 709, name: "Thairone"
      5 id: 1063,name: "Victor"
      6 id: 1105,name: "Emerson"
      7 id: 1537,name: "Leonardo"
      8 id: 570, name: "Uhitlei"
    */

    int idUserRedmine;

    if (idDev == 1) {
      drawArrayJpeg(avatar_939, sizeof(avatar_939), x, y); //1 id: 939,name: "Alex XXXXX"
      idUserRedmine = 939;
      Avatar_username = "Alex XXXXX";
    }
    if (idDev == 2) {
      drawArrayJpeg(avatar_631, sizeof(avatar_631), x, y); //2 id: 631,name: "Raimundo XXXXX"
      idUserRedmine = 631;
      Avatar_username = "Raimundo XXXXX";
    }
    if (idDev == 3) {
      drawArrayJpeg(avatar_328, sizeof(avatar_328), x, y); //3 id: 328, name: "Tarcisio XXXXX XXXXX"
      idUserRedmine = 328;
      Avatar_username = "Tarcisio XXXXX XXXXX";
    }

    if (idDev == 4) {
      drawArrayJpeg(avatar_1105, sizeof(avatar_1105), x, y); //1105,name: "Emerson XXXXX"
      idUserRedmine = 1105;
      Avatar_username = "Emerson XXXXX";
    }
    if (idDev == 5) {
      drawArrayJpeg(avatar_1537, sizeof(avatar_1537), x, y); //9 id: 1537,name: "Leonardo XXXXX"
      idUserRedmine = 1537;
      Avatar_username = "Leonardo XXXXX";
    }
    if (idDev == 6) {
      drawArrayJpeg(avatar_709, sizeof(avatar_709), x, y); //4 id: 709, name: "Thairone Simões Loureiro"
      idUserRedmine = 709;
      Avatar_username = "Thairone Simões Loureiro";
    }
    Avatar_id_chamado_andamento = 0;
    GetChamadoAvatar(idUserRedmine);
    delay(500);


    tft.setTextSize(1);
    tft.setTextColor(0x01E8);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(20, 300);  tft.print(Avatar_username);

    if (Avatar_id_chamado_andamento > 0) {
      drawCard(String(Avatar_id_chamado_andamento).c_str());
      delay(3500);
    }

    if (idDev < 6) {
      idDev++;
      drawScreen10(-1, idDev, t_x, t_y);
    }
  }

}


// #########################################################################
// Tela 11 - Throughput
// #########################################################################
void drawScreen11(int TelaAtual, int t_x, int t_y) {

  if (TelaAtual != 11) {
    Tela = 11;
    TelaAtual = 11;

    tft.setFreeFont(LABEL2_FONT);
    if (SerialDebug) Serial.println("Indicador 2");
    tft.fillScreen(TFT_BLACK);
    tft.invertDisplay(false);

    if (millis() - runTime >= 0L) { // Execute every TBD ms
      runTime = millis();

      // Set the the position, gap between meters, and inner radius of the meters
      int xpos = 0, ypos = 5, gap = 4, radius = 52;

      // Draw a large meter
      xpos = 480 / 2 - 160, ypos = 0, gap = 15, radius = 170;
      reading += (ramp);
      if (reading > 98) ramp = -1;
      if (reading < 0) ramp = 1;

      GetDadosTela3();
      delay(100);
      GetLeadtime();
      delay(100);
      throughput = wip / leadtime_medio;

      if (SerialDebug) {
        Serial.print("Wip: ");
        Serial.println(wip);
        Serial.print("leadtime_medio: ");
        Serial.println(leadtime_medio);
        Serial.print("throughput:");
        Serial.println(throughput);
      }

      reading = throughput;
      // Comment out above meters, then uncomment the next line to show large meter
      ringMeter(reading, 0, 15, xpos, ypos, radius, " Throughput ", GREEN2RED); // Draw analogue meter
    }
  }
}


// #########################################################################
// Tela 12 - Eficiencia
// #########################################################################
void drawScreen12(int TelaAtual, int t_x, int t_y) {

  if (TelaAtual != 12) {
    Tela = 12;
    TelaAtual = 12;

    tft.setFreeFont(LABEL2_FONT);
    if (SerialDebug) Serial.println("Flow Efficiency");
    tft.fillScreen(TFT_BLACK);
    tft.invertDisplay(false);

    if (millis() - runTime >= 0L) { // Execute every TBD ms
      runTime = millis();

      // Set the the position, gap between meters, and inner radius of the meters
      int xpos = 0, ypos = 5, gap = 4, radius = 52;

      // Draw a large meter
      xpos = 480 / 2 - 160, ypos = 0, gap = 15, radius = 170;
      reading += (ramp);
      if (reading > 98) ramp = -1;
      if (reading < 0) ramp = 1;


      GetEficiencia();
      delay(100);



      if (SerialDebug) {
        Serial.print("Wip: ");
        Serial.println(wip);
        Serial.print("leadtime_medio: ");
        Serial.println(leadtime_txt);
        Serial.print("Waitingtime:");
        Serial.println(waitingtime_txt);
        Serial.print("Touchtime:");
        Serial.println(touchtime_txt);
        Serial.print("Flow Efficiency:");
        Serial.println(flow_efficiency_txt);
      }


      // Comment out above meters, then uncomment the next line to show large meter
      ringMeter(flow_efficiency_numeric, 0, 100, xpos, ypos, radius, "Flow Efficiency", GREEN2RED); // Draw analogue meter
    }
  }
}

////////////////////////////////////////////////////////////////
void aveg(int fase)
{
  int z = 0;
  if (SerialDebug) {
    Serial.println(ave[fase]);
    Serial.println(avec[fase]);
  }
  avet[fase] = ave[fase] / avec[fase];
  if (SerialDebug) Serial.println(avet[fase]);
  avet[fase] = avet[fase] * 32;

  for (int i = 0; i < 24; i++) {
    for (uint16_t a = 0; a < 3; a++) {
      tft.drawLine(avet[fase] + a, z, avet[fase] + a, z + 10, 0xFB21);
    } // thick
    for (uint16_t a = 0; a < 2; a++) {
      tft.drawLine(avet[fase] - a, z, avet[fase] - a, z + 10, 0xFB21);
    }
    delay(100); z = z + 20;
  }
}
//////////////////////////////////////////////////////////////////

void dchart_10x10(int fase, uint16_t nx, uint16_t ny)
{
  ave[fase] += nx; avec[fase]++; nx = nx * 32; ny = ny * 48;
  tft.drawCircle(nx, ny, 10, 0x0517);
  tft.drawCircle(nx, ny, 9, 0x0517);
  tft.fillCircle(nx, ny, 7, 0x0517);
  delay (100); ox = nx; oy = ny;
}

///////////////////////////////////////////////////////////////////////
void dotchart_10x10(int fase, uint16_t nx, uint16_t ny, uint32_t cor) {
  ave[fase] += nx; avec[fase]++; nx = nx * 32;
  ny = ny * 48;
  int plus = 0;
  float fplus = 0; int sign = 0; int y = 0, x = 0; y = oy; x = ox;
  float xmines, ymines; xmines = nx - ox; ymines = ny - oy;
  if (ox > nx)
  { xmines = ox - nx;
    sign = 1;
  }
  else
    sign = 0;

  for (int a = 0; a < (ny - oy); a++)
  {
    fplus += xmines / ymines;
    plus = fplus;
    if (sign == 1)
      tft.drawFastHLine(0, y, x - plus, cor);
    else
      tft.drawFastHLine(0, y, x + plus, cor);
    y++;
    //delay(5);
  }

  ox = nx;
  oy = ny;
}

// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(float value, int vmin, int vmax, int x, int y, int r, const char *units, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option

  if (SerialDebug) {
    Serial.print("value: ");
    Serial.println(value);
  }

  x += r; y += r;   // Calculate coords of centre of ring

  int w = r / 3;    // Width of outer ring is 1/4 of radius

  int angle = 150;  // Half the sweep angle of meter (300 degrees)

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 3; // Segments are 3 degrees wide = 100 segments for 300 degrees
  byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Variable to save "value" text colour from scheme and set default
  int colour = TFT_BLUE;

  // Draw colour blocks every inc degrees
  for (int i = -angle + inc / 2; i < angle - inc / 2; i += inc) {
    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      switch (scheme) {
        case 0: colour = TFT_RED; break; // Fixed colour
        case 1: colour = TFT_GREEN; break; // Fixed colour
        case 2: colour = TFT_BLUE; break; // Fixed colour
        case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
        case 4: colour = rainbow(map(i, -angle, angle, 70, 127)); break; // Green to red (high temperature etc)
        case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
        default: colour = TFT_BLUE; break; // Fixed colour
      }
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      //text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREY);
    }
  }
  // Convert value to a string
  char buf[10];
  byte len = 4; if (value > 999) len = 5;
  dtostrf(value, len, 2, buf);
  buf[len] = ' '; buf[len + 1] = 0; // Add blanking space and terminator, helps to centre text too!
  // Set the text colour to default
  tft.setTextSize(1);

  if (SerialDebug) {
    Serial.print("value: ");
    Serial.println(value);
    Serial.print("buf: ");
    Serial.println(buf);
  }

  if (value > 15) {
    drawAlert(x, y + 90, 50, 1);
  }
  else {
    drawAlert(x, y + 90, 50, 0);
  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // Uncomment next line to set the text colour to the last segment value!
  tft.setTextColor(colour, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  // Print value, if the meter is large then use big font 8, othewise use 4
  if (r > 84) {
    tft.setTextPadding(55 * 3); // Allow for 3 digits each 55 pixels wide
    tft.drawString(buf, x, y, 8); // Value in middle
  }
  else {
    tft.setTextPadding(3 * 14); // Allow for 3 digits each 14 pixels wide
    tft.drawString(buf, x, y, 4); // Value in middle
  }
  tft.setTextSize(1);
  tft.setTextPadding(0);
  // Print units, if the meter is large then use big font 4, othewise use 2
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (r > 84) tft.drawString(units, x, y + 60, 4); // Units display
  else tft.drawString(units, x, y + 15, 2); // Units display

  // Calculate and return right hand side x coordinate
  return x + r;
}

void drawAlert(int x, int y , int side, boolean draw)
{
  if (draw && !range_error) {
    drawIcon(alert, x - alertWidth / 2, y - alertHeight / 2, alertWidth, alertHeight);
    range_error = 1;
  }
  else if (!draw) {
    tft.fillRect(x - alertWidth / 2, y - alertHeight / 2, alertWidth, alertHeight, TFT_BLACK);
    range_error = 0;
  }
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

//====================================================================================
// This is the function to draw the icon stored as an array in program memory (FLASH)
//====================================================================================
// To speed up rendering we use a 64 pixel buffer
#define BUFF_SIZE 64
// Draw array "icon" of defined width and height at coordinate x,y
// Maximum icon size is 255x255 pixels to avoid integer overflow
void drawIcon(const unsigned short* icon, int16_t x, int16_t y, int8_t width, int8_t height) {

  uint16_t  pix_buffer[BUFF_SIZE];   // Pixel buffer (16 bits per pixel)

  tft.startWrite();
  // Set up a window the right size to stream pixels into
  tft.setAddrWindow(x, y, width, height);
  // Work out the number whole buffers to send
  uint16_t nb = ((uint16_t)height * width) / BUFF_SIZE;
  // Fill and send "nb" buffers to TFT
  for (int i = 0; i < nb; i++) {
    for (int j = 0; j < BUFF_SIZE; j++) {
      pix_buffer[j] = pgm_read_word(&icon[i * BUFF_SIZE + j]);
    }
    tft.pushColors(pix_buffer, BUFF_SIZE);
  }

  // Work out number of pixels not yet sent
  uint16_t np = ((uint16_t)height * width) % BUFF_SIZE;

  // Send any partial buffer left over
  if (np) {
    for (int i = 0; i < np; i++) pix_buffer[i] = pgm_read_word(&icon[nb * BUFF_SIZE + i]);
    tft.pushColors(pix_buffer, np);
  }

  tft.endWrite();
}


void GetDadosCFD()
{
  if (TempoUltimaAtualizacao > 1 && millis() - TempoUltimaAtualizacao > 3000) {
    TempoUltimaAtualizacao = millis();

    if (WiFi.status() == WL_CONNECTED) { //if we are connected to Eduroam network
      counter = 0; //reset counter
      if (SerialDebug) {
        Serial.println("Wifi is still connected with IP: ");
        Serial.println(WiFi.localIP());   //inform user about his IP address
      }
    } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
      WiFi.begin(ssid);
    }
    while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
      delay(500);
      if (SerialDebug) Serial.print(".");
      counter++;
      if (counter >= 60) { //30 seconds timeout - reset board
        ESP.restart();
      }
    }

    String url = "http://10.1.100.96/api.redmine/getdadoscfd.php";
    // StaticJsonDocument<4096> jsonBuffer;

    if (SerialDebug) {
      Serial.print("requesting URL: ");
      Serial.println(url);
    }

    HTTPClient http;
    http.setTimeout(190000);
    http.begin(url);
    http.setConnectTimeout(190000);
    int start = millis();
    int httpCode = http.GET();
    int end = millis();
    if (SerialDebug) Serial.println(String("Effective timeout: ") + (end - start));
    if (httpCode > 0) {
      if (SerialDebug) Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      //file found at server --> on unsucessful connection code will be -1
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        if (SerialDebug) Serial.println(payload);
        // Use arduinojson.org/assistant to compute the capacity.
        //const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
        DynamicJsonDocument jsonBuffer(5120);
        DeserializationError error = deserializeJson(jsonBuffer, payload);
        if (error) {
          if (SerialDebug) {
            Serial.println(F("Parsing failed!"));
            Serial.println(error.c_str());
          }
          return;
        }



        if (SerialDebug) Serial.println(jsonBuffer["data_ini"].as<char*>());
        //String data_aux = jsonBuffer["data_ini"];
        String data_aux;
        //data_aux.replace("-","");
        int fatorDt = jsonBuffer["data_ini"].as<int>();
        int valor_max = jsonBuffer["maximo"].as<int>();
        for (int Dtx = 0; Dtx < 11; Dtx++) {
          data_aux = String(fatorDt);

          //cfd_fechado[Dtx]=map(jsonBuffer["fechado"][data_aux].as<int>(), 0, jsonBuffer["backlog"]["max"].as<int>(), 0, 10);
          cfd_fechado[Dtx] = map(jsonBuffer["fechado"][data_aux].as<int>(), 0, valor_max, 0, 10);
          cfd_avaliar_producao[Dtx] = map(jsonBuffer["avaliar_producao"][data_aux].as<int>(), 0, valor_max, 0, 10);
          cfd_homologacao[Dtx] = map(jsonBuffer["homologacao"][data_aux].as<int>(), 0, valor_max, 0, 10);
          cfd_desenvolvimento[Dtx] = map(jsonBuffer["desenvolvimento"][data_aux].as<int>(), 0, valor_max, 0, 10);
          cfd_backlog[Dtx] = map(jsonBuffer["backlog"][data_aux].as<int>(), 0, valor_max, 0, 10);

          fatorDt = fatorDt + 1;
        }
      }
    } else {
      if (SerialDebug) Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  }
}

void GetDadosLeadtimes()
{
  if (TempoUltimaAtualizacao > 1 && millis() - TempoUltimaAtualizacao > 3000) {
    TempoUltimaAtualizacao = millis();

    if (WiFi.status() == WL_CONNECTED) { //if we are connected to Eduroam network
      counter = 0; //reset counter
      if (SerialDebug) {
        Serial.println("Wifi is still connected with IP: ");
        Serial.println(WiFi.localIP());   //inform user about his IP address
      }
    } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
      WiFi.begin(ssid);
    }
    while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
      delay(500);
      if (SerialDebug) Serial.print(".");
      counter++;
      if (counter >= 60) { //30 seconds timeout - reset board
        ESP.restart();
      }
    }

    String url = "http://10.1.100.96/api.redmine/getdadosleadtime.php";
    //StaticJsonDocument<4096> jsonBuffer;

    if (SerialDebug) {
      Serial.print("requesting URL: ");
      Serial.println(url);
    }

    HTTPClient http;
    http.setTimeout(190000);
    http.begin(url);
    http.setConnectTimeout(190000);
    int start = millis();
    int httpCode = http.GET();
    int end = millis();
    if (SerialDebug) Serial.println(String("Effective timeout: ") + (end - start));
    if (httpCode > 0) {
      if (SerialDebug) Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      //file found at server --> on unsucessful connection code will be -1
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        if (SerialDebug) Serial.println(payload);
        // Use arduinojson.org/assistant to compute the capacity.
        //const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
        DynamicJsonDocument jsonBuffer(5120);
        DeserializationError error = deserializeJson(jsonBuffer, payload);
        if (error) {
          if (SerialDebug) {
            Serial.println(F("Parsing failed!"));
            Serial.println(error.c_str());
          }
          return;
        }



        if (SerialDebug) Serial.println(jsonBuffer["data_ini"].as<char*>());
        //String data_aux = jsonBuffer["data_ini"];
        String data_aux;
        int fatorDt = jsonBuffer["data_ini"].as<int>();
        int valor_max = 40;
        for (int Dtx = 0; Dtx < 11; Dtx++) {
          data_aux = String(fatorDt);
          chart_leadtime[Dtx] = map(jsonBuffer["leadtime"][data_aux]["leadtime_dias"].as<int>(), 0, valor_max, 0, 10);

          if (SerialDebug) {
            Serial.println("Leadtimes");
            Serial.println(chart_leadtime[Dtx]);
          }
          fatorDt = fatorDt + 1;
        }
      }
    } else {
      if (SerialDebug) Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  }
}


void GetDadosDiaDev()
{
  if (TempoUltimaAtualizacao > 1 && millis() - TempoUltimaAtualizacao > 3000) {
    TempoUltimaAtualizacao = millis();

    if (WiFi.status() == WL_CONNECTED) { //if we are connected to Eduroam network
      counter = 0; //reset counter
      if (SerialDebug) {
        Serial.println("Wifi is still connected with IP: ");
        Serial.println(WiFi.localIP());   //inform user about his IP address
      }
    } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
      WiFi.begin(ssid);
    }
    while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
      delay(500);
      if (SerialDebug) Serial.print(".");
      counter++;
      if (counter >= 60) { //30 seconds timeout - reset board
        ESP.restart();
      }
    }

    String url = "http://10.1.100.96/api.redmine/getdiadev.php";
    //StaticJsonDocument<4096> jsonBuffer;

    if (SerialDebug) {
      Serial.print("requesting URL: ");
      Serial.println(url);
    }

    HTTPClient http;
    http.setTimeout(190000);
    http.begin(url);
    http.setConnectTimeout(190000);
    int start = millis();
    int httpCode = http.GET();
    int end = millis();
    if (SerialDebug) Serial.println(String("Effective timeout: ") + (end - start));
    if (httpCode > 0) {
      if (SerialDebug) Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      //file found at server --> on unsucessful connection code will be -1
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        if (SerialDebug) Serial.println(payload);
        // Use arduinojson.org/assistant to compute the capacity.
        //const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
        DynamicJsonDocument jsonBuffer(4096);
        DeserializationError error = deserializeJson(jsonBuffer, payload);
        if (error) {
          if (SerialDebug) {
            Serial.println(F("Parsing failed!"));
            Serial.println(error.c_str());
          }
          return;
        }

        //String sit_parte1 = jsonBuffer["DataIni"];
        DiaDev_DataIni = jsonBuffer["DataIni"].as<char*>();
        DiaDev_DataFim = jsonBuffer["DataFim"].as<char*>();

        //Serial.println(jsonBuffer["chamado"].as<char*>());
        //tft.println(jsonBuffer["chamado"].as<char*>());

        DiaDev_DiaDev = jsonBuffer["DiaDev"].as<int>();
        DiaDev_TotalHoras = jsonBuffer["TotalHoras"].as<float>();

        if (SerialDebug) Serial.println(DiaDev_DiaDev);

      }
    } else {
      if (SerialDebug) Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  }
}

void GetChamadoAvatar(int idUser)
{
  if (TempoUltimaAtualizacao > 1 && millis() - TempoUltimaAtualizacao > 3000) {
    TempoUltimaAtualizacao = millis();

    if (WiFi.status() == WL_CONNECTED) { //if we are connected to Eduroam network
      counter = 0; //reset counter
      if (SerialDebug) {
        Serial.println("Wifi is still connected with IP: ");
        Serial.println(WiFi.localIP());   //inform user about his IP address
      }
    } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
      WiFi.begin(ssid);
    }
    while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
      delay(500);
      if (SerialDebug) Serial.print(".");
      counter++;
      if (counter >= 60) { //30 seconds timeout - reset board
        ESP.restart();
      }
    }

    String url = "http://10.1.100.96/api.redmine/getchamadoavatar_dinamico.php?user=" + String(idUser);
    //StaticJsonDocument<4096> jsonBuffer;

    if (SerialDebug) {
      Serial.print("requesting URL: ");
      Serial.println(url);
    }

    HTTPClient http;
    http.setTimeout(190000);
    http.begin(url);
    http.setConnectTimeout(190000);
    int start = millis();
    int httpCode = http.GET();
    int end = millis();
    if (SerialDebug) Serial.println(String("Effective timeout: ") + (end - start));
    if (httpCode > 0) {
      if (SerialDebug) Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      //file found at server --> on unsucessful connection code will be -1
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        if (SerialDebug) Serial.println(payload);
        // Use arduinojson.org/assistant to compute the capacity.
        //const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
        DynamicJsonDocument jsonBuffer(4096);
        DeserializationError error = deserializeJson(jsonBuffer, payload);
        if (error) {
          if (SerialDebug) {
            Serial.println(F("Parsing failed!"));
            Serial.println(error.c_str());
          }
          return;
        }

        Avatar_QtdChamadosAndamento = 0;
        Avatar_id_chamado_andamento = 0;
        Avatar_QtdChamadosAndamento = jsonBuffer["qtd_chamados_andamento"].as<int>();
        Avatar_id_chamado_andamento = jsonBuffer["id_chamado_andamento"].as<int>();

        if (SerialDebug) {
          Serial.println("### Avatar_id_chamado_andamento ###");
          Serial.println(Avatar_id_chamado_andamento);
        }

      }
    } else {
      if (SerialDebug) Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  }
}
