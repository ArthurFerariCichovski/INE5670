/**
 * Monitor de Temperatura e Umidade com ESP8266 + DHT11
 *
 * Conecta a placa a uma rede Wi-Fi, lê temperatura e umidade do sensor
 * DHT11 a cada segundo e envia os valores pela porta serial.
 *
 * Baseado no exemplo "DHT11 Temperature Reader" da biblioteca DHT11
 * (Dhruba Saha, v2.1.0, MIT) e no exemplo "HelloServer" do ESP8266WiFi.
 */

#include <DHT11.h> // Biblioteca do sensor DHT11
#include <ESP8266WiFi.h> // Conexão Wi-Fi do ESP8266
#include <WiFiClient.h>
#include <ESP8266WebServer.h> // Servidor HTTP (preparado para uso futuro)
#include <ESP8266mDNS.h>

// Credenciais da rede Wi-Fi
#ifndef STASSID
#define STASSID "Varduga"
#define STAPSK "12345678"
#endif


const char* ssid = STASSID;
const char* password = STAPSK;

// Servidor HTTP na porta 80 (ainda não iniciado; será usado para
// disponibilizar as leituras ao backend/app)
ESP8266WebServer server(80);

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

void loop() {
    // Lê temperatura e umidade do sensor.
    // Em caso de falha, a biblioteca devolve um código de erro
    // (ERROR_CHECKSUM ou ERROR_TIMEOUT) no lugar do valor.
    int temperature = dht11.readTemperature();
    int humidity = dht11.readHumidity();

    // Umidade: imprime o valor ou a mensagem de erro
    if (humidity != DHT11::ERROR_CHECKSUM && humidity != DHT11::ERROR_TIMEOUT) {
        Serial.print("Umidade: ");
        Serial.print(humidity);
        Serial.println(" %");
    } else {
        Serial.println(DHT11::getErrorString(humidity));
    }
    // Temperatura: imprime o valor ou a mensagem de erro
    if (temperature != DHT11::ERROR_CHECKSUM && temperature != DHT11::ERROR_TIMEOUT) {
        Serial.print("Temperatura: ");
        Serial.print(temperature);
        Serial.println(" °C");
    } else {
        Serial.println(DHT11::getErrorString(temperature));
    }
    
    // O DHT11 só suporta uma leitura por segundo
    delay(1000);
}
