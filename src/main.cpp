#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FS.h>
#include "LittleFS.h"


#include "./WiFiStarter/WiFiStarter.cpp"
#include "./bancoDeDados/bancoDeDados.cpp"

Banco_de_Dados banco_de_dados;
WiFibro wifi;

int ultima_conexao_hora;
int ultimo_save_contagem_min;
int ultimo_get_time_seg = 0;
String registro_loja = "defin5";
struct tm timeinfo;
bool pegou_data = false;

#define INTERVALO_REGISTROS_HORA 1
#define INTERVALO_SALVAR_CONTAGEM_MIN 2
#define INTERVALO_GET_TIME_SEG 30

#define DOC_SIZE 5000
#define FORMAT_LITTLEFS_IF_FAILED true
#define JSON_SAVE_FILE "/saves.json"

int entraram = 7;

DynamicJsonDocument readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
    }
    DynamicJsonDocument json(5000);
    DeserializationError error = deserializeJson(json, file);
    serializeJsonPretty(json, Serial);
    file.close();
    return json;
}

void Showone(int wichone, String* registro, String* data, String* contagem){
  DynamicJsonDocument doc(DOC_SIZE);

  File credFile = LittleFS.open(JSON_SAVE_FILE, FILE_READ);// open file for reading
  DeserializationError error = deserializeJson(doc, credFile);// deserialize json
  if( !credFile ){ Serial.println("Failed to open file"); return; }// if file doesn't exist, return 
   
  const char* registro_salvo = doc[wichone]["codigo_loja"]; //
  const char* data_salvo = doc[wichone]["data"]; // 
  const char* contagem_salvo = doc[wichone]["contagem"]; //
  credFile.close();
  
  *registro = registro_salvo;
  *data = data_salvo;
  *contagem = contagem_salvo;
}

void writeFile(fs::FS &fs, const char * path, DynamicJsonDocument message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
  }
  serializeJson(message, file);
  file.close();
}

void addCredentials(const char * registro, const char * data,const char * contagem){
  DynamicJsonDocument doc(DOC_SIZE);// create json doc
  String Serialized; // create string to store serialized json

  File credFile = LittleFS.open(JSON_SAVE_FILE, FILE_READ);// open file for reading

if( !credFile ){ Serial.println("Failed to open file"); return; }// if file doesn't exist, return
    
DeserializationError error = deserializeJson(doc, credFile);// deserialize json
if( error ){ Serial.printf("Error on deserialization: %s\n", error.c_str() );} //error when spiff is empty or not formatted correctly

JsonArray inforArr = doc["information"].as<JsonArray>();// get array from json


JsonObject newCred = doc.createNestedObject();// create new object in json
newCred["codigo_loja"] = registro;
newCred["data"] = data;
newCred["contagem"] = contagem;

 
serializeJsonPretty(doc, Serialized);

File credFile2 = LittleFS.open(JSON_SAVE_FILE, FILE_WRITE);// open file for writing

credFile2.print(Serialized);

credFile2.close();
credFile.close();
Serialized = "";      

}

int hora;
int minuto;
char data[21];
ulong ultima_atualizacao_datahora_millis;

void atualizar_e_formatar_data_antiga(int minuto, int hora);
const char* ntpServer = "pool.ntp.org"; // Servidor que vai nos fornecer a data
const long gmtOffset_sec = -3600 * 3; // Quantas horas estamos atrasados em relação a GMT
const int daylightOffset_sec = 3600; // Offset do horário de verão

void setup() 
{
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.begin(115200);
  WiFi.begin("JUIZADODAINF", "100juizo2018");
  while (WiFi.status() == WL_CONNECTED) {delay(50);}
  getLocalTime(&timeinfo);
  ultima_atualizacao_datahora_millis = millis();
}

void loop() 
{
    int minutos_a_adicionar = int(((millis() - ultima_atualizacao_datahora_millis) / (1000*60)) % 60);
    int horas_a_adicionar   = int(((millis() - ultima_atualizacao_datahora_millis) / (1000*60*60)) % 24);
    hora += horas_a_adicionar;
    minuto += minutos_a_adicionar;
    atualizar_e_formatar_data_antiga(minuto,hora);
    Serial.println(data);
    delay(1000);
}

void atualizar_e_formatar_data_antiga(int minuto, int hora)
{
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  char timeMonth[10];
  strftime(timeMonth,10, "%B", &timeinfo);
  char timeDay[3];
  strftime(timeDay,3, "%d", &timeinfo);
  char timeYear[5];
  strftime(timeYear,5, "%Y", &timeinfo);
  // Atualizando a variável data
  strcat(data, timeDay);
  strcat(data,"/");
  strcat(data,timeMonth);
  strcat(data,"/");
  strcat(data,timeYear);
  strcat(data," ");
  String hora_str = String(hora);
  strcat(data, hora_str.c_str());
  String minuto_str = ":" + String(minuto);
  strcat(data,minuto_str.c_str());
}
/*
resposta = WiFiStarter.httpGETRequest(serverNameLoja);
  DeserializationError error = deserializeJson(doc, resposta);
  const char* lojas = doc[0]["codigo_loja"];
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  else
  {
    Serial.println(lojas);
  }
    delay(3000);
*/