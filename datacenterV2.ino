/**
 * Monitor de Temperatura e Umidade com ESP8266 + DHT11
 *
 * Conecta a placa a uma rede Wi-Fi, lê temperatura e umidade do sensor
 * DHT11 e envia os valores por HTTP (POST em JSON) para o backend,
 * que grava as leituras no banco de dados. Os valores também são
 * mostrados na porta serial para depuração.
 *
 * Baseado no exemplo "DHT11 Temperature Reader" da biblioteca DHT11
 * (Dhruba Saha, v2.1.0, MIT) e no exemplo "HelloServer" do ESP8266WiFi.
 */

#include <DHT11.h> // Biblioteca do sensor DHT11
#include <ESP8266WiFi.h> // Conexão Wi-Fi do ESP8266
#include <ESP8266HTTPClient.h> // Cliente HTTP, usado para enviar as leituras ao backend
#include <WiFiClient.h>

// Credenciais da rede Wi-Fi (ancoragem do celular, em 2.4 GHz)
#ifndef STASSID
#define STASSID "Varduga"
#define STAPSK "12345678"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

// Endereço do backend: IP do PC na rede da ancoragem (veja com ipconfig),
// porta 3000 e rota /leituras. O IP pode mudar a cada reconexão.
const char* SERVER_URL = "http://192.168.43.100:3000/leituras";

// Nome que identifica esta placa nos registros do banco
const char* DISPOSITIVO = "wemos-1";

// Sensor DHT11 com o pino de dados no GPIO13 (pino D7 no NodeMCU/Wemos)
DHT11 dht11(13);

void setup() {
    // Inicia a comunicação serial a 9600 bps para depuração
    Serial.begin(9600);

    // Conecta à rede Wi-Fi em modo estação (cliente)
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    // Aguarda a conexão, imprimindo um ponto a cada 0,5 s
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    // Conectado: mostra a rede e o IP recebido
    Serial.println("");
    Serial.print("Conectado a ");
    Serial.println(ssid);
    Serial.print("Endereco IP: ");
    Serial.println(WiFi.localIP());
}

// Envia uma leitura ao backend em formato JSON, por exemplo:
// {"dispositivo":"wemos-1","temperatura":25,"umidade":60}
void enviarLeitura(int temperatura, int umidade) {
    // Não tenta enviar se a conexão Wi-Fi caiu
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi desconectado");
        return;
    }

    WiFiClient client;
    HTTPClient http;

    // Abre a conexão com o backend e informa que o corpo é JSON
    http.begin(client, SERVER_URL);
    http.addHeader("Content-Type", "application/json");

    // Monta o corpo da requisição com os valores lidos
    String body = "{\"dispositivo\":\"" + String(DISPOSITIVO) +
                  "\",\"temperatura\":" + String(temperatura) +
                  ",\"umidade\":" + String(umidade) + "}";

    // Envia o POST e mostra o código de resposta (201 = gravado com sucesso)
    int code = http.POST(body);
    Serial.print("POST -> codigo HTTP: ");
    Serial.println(code);

    // Libera a conexão
    http.end();
}

void loop() {
    // Lê temperatura e umidade do sensor.
    // Em caso de falha, a biblioteca devolve um código de erro
    // (ERROR_CHECKSUM ou ERROR_TIMEOUT) no lugar do valor.
    int temperature = dht11.readTemperature();
    int humidity = dht11.readHumidity();

    // Verifica se cada leitura é válida (diferente dos códigos de erro)
    bool tempOk = temperature != DHT11::ERROR_CHECKSUM && temperature != DHT11::ERROR_TIMEOUT;
    bool humOk = humidity != DHT11::ERROR_CHECKSUM && humidity != DHT11::ERROR_TIMEOUT;

    if (tempOk && humOk) {
        // Leituras válidas: mostra na serial e envia ao backend
        Serial.print("Temperatura: ");
        Serial.print(temperature);
        Serial.print(" °C | Umidade: ");
        Serial.print(humidity);
        Serial.println(" %");
        enviarLeitura(temperature, humidity);
    } else {
        // Falha em alguma leitura: mostra o erro e não envia nada ao backend
        if (!humOk) {
            Serial.println(DHT11::getErrorString(humidity));
        }
        if (!tempOk) {
            Serial.println(DHT11::getErrorString(temperature));
        }
    }

    // O DHT11 só suporta uma leitura por segundo; aqui enviamos a cada 5 s
    delay(5000);
}