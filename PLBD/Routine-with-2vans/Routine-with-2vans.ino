#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Adam";
const char* password = "adam1234";

const int van1ControlPin = 14;
const int van2ControlPin = 15;
bool van1Status = false;
bool van2Status = false;

unsigned long van1StartTime = 0;
unsigned long van1Duration = 120000; // 2 minutes
bool van1AutoControl = true;

unsigned long van2StartTime = 0;
unsigned long van2Duration = 180000; // 3 minutes
bool van2AutoControl = true;

AsyncWebServer server(80);

void setup() {
  pinMode(van1ControlPin, OUTPUT);
  pinMode(van2ControlPin, OUTPUT);
  
  digitalWrite(van1ControlPin, HIGH);
  digitalWrite(van2ControlPin, HIGH);

  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body>";
    html += "<h1>Contrôle des ventilateurs</h1>";
    html += "<h2>Van 1 Control:</h2>";
    html += "<p>Van 1 Auto Control: ";
    html += van1AutoControl ? "ON" : "OFF";
    html += "</p>";
    html += "<p>Van 1 Duration: ";
    html += van1Duration / 60000;
    html += " minutes</p>";
    html += "<a href=\"/toggle-van1-auto\"><button>Toggle Van 1 Auto Control</button></a>";
    html += "<a href=\"/increment-van1-duration\"><button>Increment Van 1 Duration</button></a>";
    html += "<a href=\"/decrement-van1-duration\"><button>Decrement Van 1 Duration</button></a>";

    html += "<h2>Van 2 Control:</h2>";
    html += "<p>Van 2 Auto Control: ";
    html += van2AutoControl ? "ON" : "OFF";
    html += "</p>";
    html += "<p>Van 2 Duration: ";
    html += van2Duration / 60000;
    html += " minutes</p>";
    html += "<a href=\"/toggle-van2-auto\"><button>Toggle Van 2 Auto Control</button></a>";
    html += "<a href=\"/increment-van2-duration\"><button>Increment Van 2 Duration</button></a>";
    html += "<a href=\"/decrement-van2-duration\"><button>Decrement Van 2 Duration</button></a>";

    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/toggle-van1-auto", HTTP_GET, [](AsyncWebServerRequest *request){
    van1AutoControl = !van1AutoControl;
    request->redirect("/");
  });

  server.on("/increment-van1-duration", HTTP_GET, [](AsyncWebServerRequest *request){
    van1Duration += 60000;  // Increase duration by 1 minute
    request->redirect("/");
  });

  server.on("/decrement-van1-duration", HTTP_GET, [](AsyncWebServerRequest *request){
    if (van1Duration >= 120000) { // Minimum duration is 2 minutes
      van1Duration -= 60000;  // Decrease duration by 1 minute
    }
    request->redirect("/");
  });

  server.on("/toggle-van2-auto", HTTP_GET, [](AsyncWebServerRequest *request){
    van2AutoControl = !van2AutoControl;
    request->redirect("/");
  });

  server.on("/increment-van2-duration", HTTP_GET, [](AsyncWebServerRequest *request){
    van2Duration += 60000;  // Increase duration by 1 minute
    request->redirect("/");
  });

  server.on("/decrement-van2-duration", HTTP_GET, [](AsyncWebServerRequest *request){
    if (van2Duration >= 120000) { // Minimum duration is 2 minutes
      van2Duration -= 60000;  // Decrease duration by 1 minute
    }
    request->redirect("/");
  });

  server.begin();

  // Initialize start times
  van1StartTime = millis();
  van2StartTime = millis();
}

void loop() {
  if (van1AutoControl) {
    if (!van1Status && (millis() - van1StartTime >= van1Duration)) {
      digitalWrite(van1ControlPin, HIGH);
      van1Status = true;
    } else if (van1Status && (millis() - van1StartTime >= van1Duration)) {
      digitalWrite(van1ControlPin, LOW);
      van1Status = false;
      van1StartTime = millis();
    }
  }

  if (van2AutoControl) {
    if (!van2Status && (millis() - van2StartTime >= van2Duration)) {
      digitalWrite(van2ControlPin, HIGH);
      van2Status = true;
    } else if (van2Status && (millis() - van2StartTime >= van2Duration)) {
      digitalWrite(van2ControlPin, LOW);
      van2Status = false;
      van2StartTime = millis();
    }
  }
}
