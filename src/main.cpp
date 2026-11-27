#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "internet.h"
#include <dht.h>
// #include <ezTime.h>

#define LEDvm 2
#define pindht 5
int ldr = 32;
#define tipodht DHT22
unsigned long marcoZero = 0;
bool piscando;
bool estadoLed;

DHT sensor(pindht, tipodht);
// Timezone brasil;

//=====================================================MQTT===========================================================
const char *mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char *mqtt_client_id = "senai134_esp_lara";
const char *mqtt_topic_sub = "lara_tablet";
const char *mqtt_topic_pub = "lara_pc";

void conectaMqtt();
void retornoMqtt(char *topic, byte *payload, unsigned int length);

WiFiClient espClient;
PubSubClient client(espClient);

//=================================================SETUP==============================================================

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(5000); // espera eu digitar
  conexaoWifi();
  sensor.begin();
  pinMode(LEDvm, OUTPUT);

  // Serial.println("Sincronizando relógio via NTP...");
  // waitForSync();                           // Aguarda obter o horário da internet
  // brasil.setLocation("America/Sao_Paulo"); // Aplica o fuso de São Paulo / Brasil
  // Serial.println("Relógio sincronizado!");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(retornoMqtt); // quando recebe mensagem de volta, executa a funcao retorno
}

//=====================================================LOOP=========================================================

void loop()
{
  reconexaoWifi();

  // events();

  client.loop(); // fica verificando se teve retorno de algum canal
  if (!client.connected())
  {
    conectaMqtt();
  }

  if (piscando)
  {
    unsigned long marcoAtual1 = millis();
    if (marcoAtual1 - marcoZero >= 500)
    {
      estadoLed = !estadoLed;
      digitalWrite(LEDvm, estadoLed);
      marcoZero = marcoAtual1;
    }

    //==============================================LDR e DHT=========================================================

    // int valorLdr;

    // unsigned long marcoAtual = millis();

    // if (marcoAtual - marcoZero >= 3000)
    // {
    //   valorLdr = analogRead(ldr);

    //   float tempC = sensor.readTemperature();
    //   float umidade = sensor.readHumidity();

    //   JsonDocument pasta;
    //   pasta["disp"] = "Esp-Lara";
    //   pasta["temperatura"] = tempC;
    //   pasta["umidade"] = umidade;
    //   pasta["luz"] = valorLdr;

    //   String stringJson;
    //   serializeJson(pasta, stringJson);

    //   client.publish(mqtt_topic_pub, stringJson.c_str()); // publica conteúdo presente na stringjson

    //   Serial.print("Enviado: ");
    //   Serial.println(stringJson);

    //   marcoZero = marcoAtual;
    // }
    // //}
  }
}

//=============================================FUNCOES=========================================================

// PARA CONECTAR ----------------------------------------------------------------------------------------------

void conectaMqtt()
{
  while (!client.connected()) // enquanto não estiver conectado no mqtt, vai mandar conectar
  {
    Serial.println("Conectando ao MQTT");
    if (client.connect(mqtt_client_id))
    {
      Serial.println("Conectado ");
      client.subscribe(mqtt_topic_sub); // se inscreve no canal
    }
    else
    {
      Serial.println("Falha ao se conectar ");
      Serial.println(client.state()); // mostra o erro que deu
      Serial.println("Tentando novamente em 5 segundos");

      delay(5000);
    }
  }
}

// RECEBIMENTO MQTT --------------------------------------------------------------------------------------------

void retornoMqtt(char *topic, byte *payload, unsigned int length)
{
  // Serial.print("Mensagem recebida em ");
  // Serial.print(topic);
  // Serial.print(": ");

  String msgRecebida = "";
  for (int i = 0; i < length; i++)
  {
    msgRecebida += (char)payload[i]; // corpo da mensagem não vem no padrão string, mas sim como padrao de caracteres, entao estou salvando em string
  }

  Serial.println(msgRecebida);

  JsonDocument pasta;

  DeserializationError erro = deserializeJson(pasta, msgRecebida); // transforma msgRecebida em Json, se der erro salvo aqui

  if (erro)
  {
    Serial.print("Erro ao verificar o JSON: ");
    Serial.println(erro.c_str());
    return; // se der erro ele executa esse if, mostra o erro e sai da função
  }

  const char *msg = pasta["msg"];

  if (msg != nullptr)
  {
    String comando = String(msg);

    if (comando.indexOf("Piscar") != -1)
    {
      piscando = true;
    }
    else if (comando.indexOf("Ligar") != -1)
    {
      piscando = false; // Para de piscar para manter aceso fixo
      estadoLed = HIGH;
      digitalWrite(LEDvm, HIGH);
    }
    else if (comando.indexOf("Desligar") != -1)
    {
      piscando = false; // Para de piscar para manter apagado
      estadoLed = LOW;
      digitalWrite(LEDvm, LOW);
    }
  }

  // Acessar campos do JSON--------------------------------------------------------------------------------------
  // const char *dispositivo = pasta["disp"];
  // const char *msg = pasta["msg"];
  // // Serial.println("Dados decodificados");
  // // Serial.print("Esp: ");
  // // Serial.println(dispositivo);
  // Serial.print("Mensagem: ");
  // Serial.println(msg);
  // Serial.print("-------------------------");
}
