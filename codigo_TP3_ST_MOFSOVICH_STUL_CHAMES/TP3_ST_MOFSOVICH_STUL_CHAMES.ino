//kevin mofsovich aaron stul y nico chames tp3, telegram con 2 loops
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
////////////////////////////////////////////////////////////////////
TaskHandle_t Task1;
TaskHandle_t Task2;

////////////////////////////////////////////////////////////////////
const char* ssid = "MECA-IoT";
const char* password = "IoT$2025";
#define BOTtoken "7565913149:AAHVOnKdXOd0jtXWCiurgOqT6DXENc9J19A"
#define CHAT_ID "7173083776"
#define CHAT_ID2 "7173083776"
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
// Telegram y WiFi
// Tiempos
unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 1000;
////////////////////////////////////////////////////////////////////
int pinbot1 = 34;
int pinbot2 = 35;
int led = 26;
unsigned long tiempo;
////////////////////////////////////////////////////////////////////
int limite = 26;
bool hacecalor = false;
////////////////////////////////////////////////////////////////////
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 21
#define DHTPIN 23
#define SCL_PIN 22
////////////////////////////////////////////////////////////////////
const long intervalo = 300;
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);
sensors_event_t event;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);//inicia la pantalla oled
////////////////////////////////////////////////////////////////////
int opcion = 0;
#define pantallaajuste 0
#define pantallamedicion 1
#define confirmacionmedicion 2
#define subir 3
#define bajar 4
#define confirmacionajuste 5
#define paso0 6
#define paso1 7
#define paso2 8

////////////////////////////////////////////////////////////////////
bool cuentaatras = false;
unsigned long tiempoInicio = 0;
int pasos = 0;
const unsigned long TIEMPO_CUENTA = 5000;  // 5 segundos
bool ledState = false;
bool calor_anterior = false;
////////////////////////////////////////////////////////////////////
void setup() {
  
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  pinMode(pinbot1, INPUT);
  pinMode(pinbot2, INPUT);

  dht.begin();

  sensor_t sensor;
  dht.temperature().getSensor(&sensor);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {//si falla la pantalla
    Serial.println(F("SSD1306 allocation failed"));
  }

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Lectura de temperatura:");
  display.display();



               // Conectar a Wi-Fi
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);//servidor
  while (WiFi.status() != WL_CONNECTED) {//si el status no se conecta bien
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  delay(100);
  bot.sendMessage(CHAT_ID, "Bot listo para recibir comandos", "");
    xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2code, /* Task function. */
    "Task2",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task2,    /* Task handle to keep track of created task */
    1);        /* pin task to core 1 */
}


//Task1code: blinks an LED every 1000 ms
void Task1code(void* pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    if (event.temperature > limite) {
      digitalWrite(2, HIGH);
hacecalor = true;
    } else if (event.temperature <limite)

     {
      digitalWrite(2, LOW);
    hacecalor = false;
    }
    unsigned long tiempoahora = millis();
    if (tiempoahora - tiempo >= intervalo) {
      tiempo = tiempoahora;
      dht.temperature().getEvent(&event);

      if (isnan(event.temperature)) {
        display.println("Error leyendo temp");
      }
    }

    int bot1 = digitalRead(pinbot1);
    int bot2 = digitalRead(pinbot2);
    switch (opcion) {
      case pantallaajuste:

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Limite: ");
        display.print(limite);
        display.println(" C");
        display.setCursor(0, 10);
        display.setCursor(0, 20);
        display.print("Bot1: +");
        display.setCursor(0, 30);
        display.print("Bot2: -");
        display.display();
        if (bot1 == LOW && bot2 == LOW) {

          opcion = confirmacionajuste;
        }
        if (bot1 == LOW) {

          opcion = subir;
        }
        if (bot2 == LOW) {

          opcion = bajar;
        }
        break;
      case pantallamedicion:

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Temp: ");
        display.print(event.temperature);
        display.println(" C");
        display.setCursor(0, 20);

        display.setCursor(0, 40);
        display.print("Limite: ");
        display.print(limite);
        display.println(" C");
        display.print("pasos");
        display.println(pasos);
        display.display();
        if (tiempoahora - tiempoInicio >= TIEMPO_CUENTA) {
          pasos = 0;
        }



        if (pasos == 0) {
          if (bot1 == LOW && bot2 == HIGH) {
            opcion = paso0;
          } else if (bot1 == HIGH && bot2 == LOW) {
            pasos = 0;
          }
        }

        if (pasos == 1) {
          if (bot1 == HIGH && bot2 == LOW) {
            opcion = paso1;
          } else if (bot1 == LOW && bot2 == HIGH) {
            pasos = 0;
          }
        }
        if (pasos == 2) {
          if (bot1 == LOW && bot2 == HIGH) {
            opcion = paso2;
          } else if (bot1 == HIGH && bot2 == LOW) {
            pasos = 0;
          }
        }
        if (pasos == 3) {
          opcion = confirmacionmedicion;
        }
        break;
      case confirmacionmedicion:
        if (bot1 == HIGH && bot2 == HIGH) {
          pasos = 0;
          opcion = pantallaajuste;
        }
        break;
      case confirmacionajuste:
        if (bot1 == HIGH && bot2 == HIGH) {

          opcion = pantallamedicion;
        }
        break;
      case paso0:
        if (bot1 == HIGH && bot2 == HIGH) {
          tiempoInicio = millis();
          pasos = 1;

          opcion = pantallamedicion;
        }
        break;
      case paso1:
        if (bot1 == HIGH && bot2 == HIGH) {
          tiempoInicio = millis();
          pasos = 2;
          opcion = pantallamedicion;
        }
        break;
      case paso2:
        if (bot1 == HIGH && bot2 == HIGH) {
          pasos = 3;
          opcion = pantallamedicion;
        }
        break;
      case subir:
        if (bot1 == HIGH) {
          limite++;
          opcion = pantallaajuste;
        }
        if (bot1 == LOW && bot2 == LOW) {

          opcion = confirmacionajuste;
        }
        break;
      case bajar:
        if (bot2 == HIGH) {
          limite--;
          opcion = pantallaajuste;
        }
        if (bot1 == LOW && bot2 == LOW) {

          opcion = confirmacionajuste;
        }
        break;
    }
  }
}

//Task2code: blinks an LED every 700 ms
void Task2code(void* pvParameters) {
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
  if (hacecalor && !calor_anterior) {
      bot.sendMessage(CHAT_ID, "Hace calor...", "");
      calor_anterior = true; 
    }
   if (!hacecalor) {
      calor_anterior = false;
    }

    if (millis() > lastCheckTime + checkInterval) {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while (numNewMessages != 0) {
        Serial.println("got response");
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
      lastCheckTime = millis();
    }
  }
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

  if (text == "/temp") {
       bot.sendMessage(chat_id, "hay ", "");
        bot.sendMessage(chat_id, String(event.temperature) , "");
 
 
      
    }
  }
}

void loop() {
}