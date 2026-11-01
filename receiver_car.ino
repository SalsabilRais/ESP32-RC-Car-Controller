#include <WiFi.h>
#include <WebServer.h>

const char* ssid     = "Rais";     
const char* password = "20262027";

// --- DÉFINITION DES BROCHES (PINS) ---
// Moteur Gauche (Driver L298N)
#define ENA 13  // PWM Vitesse Moteur Gauche
#define IN1 12  // Direction Moteur Gauche 1
#define IN2 14  // Direction Moteur Gauche 2

// Moteur Droit (Driver L298N)
#define ENB 25  // PWM Vitesse Moteur Droit
#define IN3 27  // Direction Moteur Droit 1
#define IN4 26  // Direction Moteur Droit 2

// Capteurs & Actuateurs
#define IR_SENSOR_PIN 34  // Broche de sortie du capteur IR d'obstacle
#define BUZZER_PIN    32  // Broche du Buzzer

// Serveur Web sur le port 80
WebServer server(80);

// --- CODE HTML DE LA MANETTE WEB ---
const char HTML_CONTROLLER[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <title>Manette Voiture ESP32</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background-color: #222; color: white; margin: 0; padding: 0; user-select: none; }
        h2 { margin-top: 20px; }
        .controller { display: grid; grid-template-columns: 80px 80px 80px; grid-template-rows: 80px 80px 80px; gap: 15px; justify-content: center; margin-top: 30px; }
        .btn { background: #ff9800; border: none; border-radius: 15px; color: white; font-size: 24px; font-weight: bold; cursor: pointer; box-shadow: 0 4px #b26a00; }
        .btn:active { background: #e68a00; box-shadow: 0 2px #b26a00; transform: translateY(2px); }
        .up { grid-column: 2; grid-row: 1; }
        .left { grid-column: 1; grid-row: 2; }
        .stop { grid-column: 2; grid-row: 2; background: #f44336; box-shadow: 0 4px #b71c1c; }
        .right { grid-column: 3; grid-row: 2; }
        .down { grid-column: 2; grid-row: 3; }
    </style>
</head>
<body>
    <h2>Manette Voiture ESP32</h2>
    <div class="controller">
        <button class="btn up" ontouchstart="sendCmd('F')" onmousedown="sendCmd('F')" ontouchend="sendCmd('S')" onmouseup="sendCmd('S')">&#9650;</button>
        <button class="btn left" ontouchstart="sendCmd('L')" onmousedown="sendCmd('L')" ontouchend="sendCmd('S')" onmouseup="sendCmd('S')">&#9664;</button>
        <button class="btn stop" onclick="sendCmd('S')">STOP</button>
        <button class="btn right" ontouchstart="sendCmd('R')" onmousedown="sendCmd('R')" ontouchend="sendCmd('S')" onmouseup="sendCmd('S')">&#9654;</button>
        <button class="btn down" ontouchstart="sendCmd('B')" onmousedown="sendCmd('B')" ontouchend="sendCmd('S')" onmouseup="sendCmd('S')">&#9660;</button>
    </div>

    <script>
        function sendCmd(cmd) {
            fetch('/move?dir=' + cmd);
        }
    </script>
</body>
</html>
)rawliteral";

// --- FONCTIONS DE CONTRÔLE DES MOTEURS ---
void avancer() {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    analogWrite(ENA, 200);   analogWrite(ENB, 200);
}

void reculer() {
    digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
    analogWrite(ENA, 200);   analogWrite(ENB, 200);
}

void tournerGauche() {
    digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    analogWrite(ENA, 180);   analogWrite(ENB, 180);
}

void tournerDroite() {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
    analogWrite(ENA, 180);   analogWrite(ENB, 180);
}

void stopper() {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    analogWrite(ENA, 0);    analogWrite(ENB, 0);
}

// --- GESTION DES REQUÊTES DU SERVEUR WEB ---
void handleRoot() {
    server.send(200, "text/html", HTML_CONTROLLER);
}

void handleMove() {
    if (server.hasArg("dir")) {
        String dir = server.arg("dir");
        if (dir == "F") avancer();
        else if (dir == "B") reculer();
        else if (dir == "L") tournerGauche();
        else if (dir == "R") tournerDroite();
        else if (dir == "S") stopper();
    }
    server.send(200, "text/plain", "OK");
}

// --- SETUP ---
void setup() {
    Serial.begin(115200);

    // Configuration des broches
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
    pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

    pinMode(IR_SENSOR_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    stopper(); // Arrêt initial des moteurs

    // Démarrage du mode Access Point (Wi-Fi émis par l'ESP32)
    WiFi.softAP(ssid, password);
    Serial.println("Point d'accès Wi-Fi démarré !");
    Serial.print("Adresse IP : ");
    Serial.println(WiFi.softAPIP());

    // Routes du serveur Web
    server.on("/", handleRoot);
    server.on("/move", handleMove);
    server.begin();
    Serial.println("Serveur Web démarré.");
}

// --- LOOP ---
void loop() {
    server.handleClient(); // Traitement des clics de la manette web

    // Lecture du capteur d'obstacle IR
    // La plupart des modules IR envoient un signal LOW (0) lorsqu'un obstacle est détecté
    int obstacleDetected = digitalRead(IR_SENSOR_PIN);

    if (obstacleDetected == LOW) { 
        digitalWrite(BUZZER_PIN, HIGH); // Activer le son du buzzer
        stopper();                      // Optionnel : Stopper la voiture automatiquement
    } else {
        digitalWrite(BUZZER_PIN, LOW);  // Désactiver le son
    }
} 
