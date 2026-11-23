#include <Arduino.h>
#include <WiFi.h>
#include "internet.h" //quando adiciono bibliotecas minhas, devo colocar em aspas
#include "senha.h"
#include <ezTime.h>

const unsigned long tempoEsperaConexao = 10000;  // variável pra controle de tempo de conexao e reconexao
const unsigned long tempoEsperaReconexao = 5000; // a cada 5 seg verifica se wifi ta conectado

//=======================================FUNCÕES=============================================================

// FUNÇÃO DE CONEXÃO ---------------------------------------------------

void conexaoWifi()
{
  Serial.printf("Conectando ao Wifi: %s", SSID);

  WiFi.begin(SSID, SENHA);

  unsigned long marcoZeroWifi = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - marcoZeroWifi < tempoEsperaConexao) // enquanto não estiver conectado e estiver esperando os 10 seg, vai executar oq está dentro
  {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Wifi conectado com sucesso!");
    Serial.print("Endereço de IP: ");
    Serial.println(WiFi.localIP());

    // SINCRONIZAÇÃO DO EZTIME (É feita logo após conectar no Wi-Fi)
    
  }
  else
  {
    Serial.println("Falha ao conectar");
  }
}

// FUNÇÃO RECONEXÃO WIFI -------------------------------------------------------

void reconexaoWifi()
{
  unsigned long marcoAtualWifi = millis();
  unsigned long ultimaConexao = 0;

  if (marcoAtualWifi - ultimaConexao > tempoEsperaConexao)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Conexao Wifi perdida");
      conexaoWifi(); // mandando executar de novo a conexao
    }
    ultimaConexao = marcoAtualWifi;
  }
}