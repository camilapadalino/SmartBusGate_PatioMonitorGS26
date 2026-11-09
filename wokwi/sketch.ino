
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include "secrets.h"

// =====================================
// SMART BUS GATE 2026
// WOKWI + PYTHON + MACHINE LEARNING
// =====================================

// PINOS
#define TRIG_PIN 5
#define ECHO_PIN 18
#define SERVO_PIN 19
#define LED_VERDE 26
#define LED_VERMELHO 27
#define PIR_PIN 25

// REGRAS DA CANCELA
#define DISTANCIA_ABERTURA 20
#define DISTANCIA_LIBERACAO 30
#define TEMPO_SEGURANCA 3000
#define TEMPO_ALERTA_OBSTRUCAO 30

// INTERVALOS DA IA (milissegundos)
#define INTERVALO_IA_NORMAL 5000
#define INTERVALO_IA_FALHA 30000

// WIFI
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// ENDPOINT N8N
const char* WEBHOOK_URL =
  "https://globalsolutionrpa26.app.n8n.cloud/webhook/smart-bus-gate";

// ENDPOINT DA IA
const char* URL_IA =
  "https://occurring-largely-congress-consolidation.trycloudflare.com/prever";

// VARIAVEIS
Servo cancela;

bool cancelaAberta = false;
bool alertaObstrucaoEnviado = false;

unsigned long inicioAreaLivre = 0;
unsigned long inicioProximidade = 0;
unsigned long tempoParado = 0;

unsigned long ultimaConsultaIA = 0;
unsigned long intervaloAtualIA = INTERVALO_IA_NORMAL;

String ultimaClassificacaoIA = "NAO_CONSULTADA";

// =====================================
// CONEXAO WIFI
// =====================================

void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);

  Serial.print("Conectando ao WiFi");

  unsigned long inicio = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - inicio < 30000
  ) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Falha na conexao WiFi.");
  }
}

// =====================================
// ENVIO DE EVENTOS AO N8N
// =====================================

void registrarEvento(
  String evento,
  float distancia,
  bool movimento
) {
  JsonDocument documento;

  documento["dispositivo"] = "SMART_BUS_GATE_01";
  documento["evento"] = evento;
  documento["distancia_cm"] = distancia;
  documento["tempo_parado"] = tempoParado;
  documento["movimento"] = movimento;
  documento["cancela"] =
    cancelaAberta ? "ABERTA" : "FECHADA";

  String json;
  serializeJson(documento, json);

  Serial.println("================================");
  Serial.println("EVENTO N8N: " + evento);
  Serial.println(json);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("N8N: WiFi desconectado.");
    return;
  }

  WiFiClientSecure cliente;
  cliente.setInsecure(); // Apenas Wokwi
  cliente.setHandshakeTimeout(15);

  HTTPClient http;
  http.setTimeout(5000);
  http.setReuse(false);

  if (!http.begin(cliente, WEBHOOK_URL)) {
    Serial.println("N8N: Erro ao iniciar HTTP.");
    return;
  }

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  int codigo = http.POST(json);

  Serial.print("HTTP N8N: ");
  Serial.println(codigo);

  if (codigo >= 200 && codigo < 300) {
    Serial.println("N8N: Evento enviado!");
  } else {
    Serial.println("N8N: Falha no envio.");

    if (codigo < 0) {
      Serial.println(http.errorToString(codigo));
    }
  }

  http.end();
}

// =====================================
// CONSULTA A IA PYTHON
// =====================================

void consultarIA(
  float distancia,
  bool movimento,
  unsigned long tempo
) {
  Serial.println("================================");
  Serial.println("ENVIANDO DADOS PARA IA PYTHON");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("IA: WiFi desconectado.");
    intervaloAtualIA = INTERVALO_IA_FALHA;
    ultimaClassificacaoIA = "INDISPONIVEL";
    return;
  }

  JsonDocument entrada;

  entrada["distancia_cm"] = distancia;
  entrada["movimento"] = movimento;
  entrada["tempo_parado"] = tempo;

  String json;
  serializeJson(entrada, json);

  Serial.println(json);

  WiFiClientSecure cliente;
  cliente.setInsecure(); // Apenas Wokwi
  cliente.setHandshakeTimeout(15);

  HTTPClient http;
  http.setTimeout(5000);
  http.setReuse(false);

  if (!http.begin(cliente, URL_IA)) {
    Serial.println("IA: Falha ao iniciar HTTP.");
    intervaloAtualIA = INTERVALO_IA_FALHA;
    ultimaClassificacaoIA = "INDISPONIVEL";
    return;
  }

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  http.addHeader("x-api-key", API_KEY_IA);

  int codigo = http.POST(json);

  Serial.print("HTTP IA: ");
  Serial.println(codigo);

  if (codigo == 200) {
    String resposta = http.getString();

    JsonDocument resultado;

    DeserializationError erro =
      deserializeJson(resultado, resposta);

    if (!erro && resultado["classificacao"].is<String>()) {
      ultimaClassificacaoIA =
        resultado["classificacao"].as<String>();

      intervaloAtualIA = INTERVALO_IA_NORMAL;

      Serial.print("CLASSIFICACAO DA IA: ");
      Serial.println(ultimaClassificacaoIA);

      if (
        ultimaClassificacaoIA ==
        "POSSIVEL_OBSTRUCAO"
      ) {
        Serial.println(
          "ALERTA IA: POSSIVEL OBSTRUCAO!"
        );
      }

      if (ultimaClassificacaoIA == "ANOMALIA") {
        Serial.println(
          "ALERTA IA: ANOMALIA DETECTADA!"
        );
      }

    } else {
      Serial.println("IA: Resposta JSON invalida.");
      intervaloAtualIA = INTERVALO_IA_FALHA;
      ultimaClassificacaoIA = "INDISPONIVEL";
    }

  } else {
    Serial.println("IA: Falha na consulta.");

    intervaloAtualIA = INTERVALO_IA_FALHA;
    ultimaClassificacaoIA = "INDISPONIVEL";

    if (codigo < 0) {
      Serial.println(http.errorToString(codigo));
    } else {
      Serial.println(http.getString());
    }

    Serial.println(
      "IA indisponivel. Controle local mantido."
    );
  }

  http.end();

  Serial.println("================================");
}

// =====================================
// CONTROLE DA CANCELA
// =====================================

void abrirCancela() {
  cancela.write(90);
  cancelaAberta = true;

  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_VERMELHO, LOW);

  inicioAreaLivre = 0;

  Serial.println("EVENTO: CANCELA_ABERTA");
}

void fecharCancela() {
  cancela.write(0);
  cancelaAberta = false;

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);

  inicioAreaLivre = 0;

  Serial.println("EVENTO: CANCELA_FECHADA");
}

// =====================================
// CONFIGURACAO INICIAL
// =====================================

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  cancela.setPeriodHertz(50);
  cancela.attach(SERVO_PIN, 500, 2400);

  cancela.write(0);

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);

  Serial.println("================================");
  Serial.println("SMART BUS GATE 2026");
  Serial.println("WOKWI + PYTHON + MACHINE LEARNING");
  Serial.println("================================");

  conectarWiFi();

  Serial.println("Sistema iniciado!");
  Serial.println("Cancela inicialmente FECHADA.");
}

// =====================================
// LOOP PRINCIPAL
// =====================================

void loop() {

  // 1. LEITURA DO ULTRASSONICO
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracao = pulseIn(
    ECHO_PIN,
    HIGH,
    30000
  );

  // 2. LEITURA DO PIR
  bool movimento =
    digitalRead(PIR_PIN) == HIGH;

  // 3. FALHA DO SENSOR
  if (duracao == 0) {
    inicioAreaLivre = 0;
    inicioProximidade = 0;
    tempoParado = 0;
    alertaObstrucaoEnviado = false;

    Serial.println("ALERTA: FALHA_SENSOR");

    delay(500);
    return;
  }

  // 4. DISTANCIA
  float distancia =
    duracao * 0.0343 / 2.0;

  // 5. TEMPO DE PERMANENCIA
  if (distancia <= DISTANCIA_ABERTURA) {

    if (inicioProximidade == 0) {
      inicioProximidade = millis();
    }

    tempoParado =
      (millis() - inicioProximidade) / 1000;

  } else {
    inicioProximidade = 0;
    tempoParado = 0;
    alertaObstrucaoEnviado = false;
  }

  // 6. MONITORAMENTO
  Serial.println("--------------------");

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  Serial.print("PIR: ");
  Serial.println(
    movimento ? "MOVIMENTO" : "LIVRE"
  );

  Serial.print("Tempo de permanencia: ");
  Serial.print(tempoParado);
  Serial.println(" segundos");

  // 7. CONTROLE DA CANCELA
  if (!cancelaAberta) {

    if (distancia <= DISTANCIA_ABERTURA) {
      abrirCancela();

      registrarEvento(
        "CANCELA_ABERTA",
        distancia,
        movimento
      );
    }

  } else {

    bool areaLivre =
      distancia > DISTANCIA_LIBERACAO &&
      !movimento;

    if (areaLivre) {

      if (inicioAreaLivre == 0) {
        inicioAreaLivre = millis();

        Serial.println(
          "Aguardando area livre..."
        );
      }

      if (
        millis() - inicioAreaLivre >=
        TEMPO_SEGURANCA
      ) {
        fecharCancela();

        registrarEvento(
          "CANCELA_FECHADA",
          distancia,
          movimento
        );
      }

    } else {
      inicioAreaLivre = 0;

      Serial.println(
        "Cancela mantida aberta por seguranca."
      );
    }
  }

  // 8. ALERTA LOCAL DE OBSTRUCAO
  if (
    distancia <= DISTANCIA_ABERTURA &&
    tempoParado >= TEMPO_ALERTA_OBSTRUCAO &&
    !alertaObstrucaoEnviado
  ) {
    Serial.println(
      "ALERTA LOCAL: POSSIVEL_OBSTRUCAO"
    );

    registrarEvento(
      "POSSIVEL_OBSTRUCAO",
      distancia,
      movimento
    );

    alertaObstrucaoEnviado = true;
  }

  // 9. CONSULTA PERIODICA A IA
  if (
    millis() - ultimaConsultaIA >=
    intervaloAtualIA
  ) {
    ultimaConsultaIA = millis();

    consultarIA(
      distancia,
      movimento,
      tempoParado
    );
  }

  delay(500);
}
