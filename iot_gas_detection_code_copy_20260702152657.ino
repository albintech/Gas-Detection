#include <WiFi.h>
#include <FirebaseESP32.h>  
#include "time.h"

// 1. YOUR NETWORK CREDENTIALS
const char* ssid = "IQOO";          
const char* password = "123465zx";  

// 2. YOUR FIREBASE CREDENTIALS
#define API_KEY "AIzaSyCYfgEse120pRKj1AKS0zUNbpcrNxl_vb8" 
#define DATABASE_URL "https://gas-leak-detection-9e32e-default-rtdb.asia-southeast1.firebasedatabase.app/"

// 3. HARDWARE PIN CONFIGURATION
#define MQ2_PIN 34       
#define BUZZER_PIN 25    // Low-level trigger buzzer module
#define VALVE_PIN 26     // Low-level trigger relay module

// 4. SAFETY THRESHOLD & TIMING
const int THRESHOLD = 500; 
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 30000; 

// Track previous state to trigger instant alerts without spamming entries
bool leakAlreadyAlerted = false; 

// 5. INTERNET TIME (NTP) SETTINGS
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;  // India Standard Time (GMT+5:30)
const int daylightOffset_sec = 0;   

// Firebase Connection Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String getRealTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Time Sync Error";
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

// Unified function to handle exact database overrides
void uploadLogToFirebase(int currentGasLevel, String systemStatus) {
  String currentRealTime = getRealTime();

  FirebaseJson json;
  json.add("gas_level", currentGasLevel);
  json.add("timestamp", currentRealTime); 
  json.add("status", systemStatus);

  if (Firebase.setJSON(fbdo, "/Gas_Detection_Logs", json)) {
    Serial.println("-> Firebase Sync: Exact Data Overwritten Successfully.");
  } else {
    Serial.print("-> Firebase Sync Failed. Reason: ");
    Serial.println(fbdo.errorReason());
  }
}

void setup() {
  Serial.begin(115200);

  // Configure Pin Modes
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VALVE_PIN, OUTPUT);
  
  // Set Safe Initial State (HIGH = Deactivated for Low-Trigger Modules)
  digitalWrite(BUZZER_PIN, HIGH);  // Buzzer SILENT
  digitalWrite(VALVE_PIN, HIGH);   // Relay DE-ENERGIZED (Valve Open)

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[Connected to Wi-Fi Network]");

  Serial.println("Synchronizing real-time clock via NTP...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  struct tm timeinfo;
  while(!getLocalTime(&timeinfo)){
     delay(500);
     Serial.print(".");
  }
  Serial.println("\n[Clock Sync Successful!]");

  // Regional Firebase Host Route Configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.host = "gas-leak-detection-9e32e-default-rtdb.asia-southeast1.firebasedatabase.app"; 

  Serial.println("Signing up anonymously to Firebase...");
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("[Firebase Signup Token Created Successfully!]");
  } else {
    Serial.printf("Token creation error: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  int sensorValue = analogRead(MQ2_PIN);
  bool isLeakDetected = (sensorValue >= THRESHOLD);

  // --- CRITICAL SAFETY & INSTANT FIREBASE OVERRIDE ---
  if (isLeakDetected) {
    digitalWrite(BUZZER_PIN, LOW);   // LOW activates the low-level buzzer alarm
    digitalWrite(VALVE_PIN, LOW);    // LOW triggers relay to close gas valve

    // If a leak was just detected, force an immediate print to Serial and Firebase
    if (!leakAlreadyAlerted) {
      Serial.println("\n=============================================");
      Serial.println("CRITICAL ALERT: LEAKAGE IS DETECTED! (VALVE CLOSED)");
      Serial.print("Leakage Concentration Level: "); Serial.println(sensorValue);
      Serial.print("Timestamp: "); Serial.println(getRealTime());
      Serial.println("=============================================");
      
      // Force instant upload
      uploadLogToFirebase(sensorValue, "LEAKAGE DETECTED (VALVE CLOSED)");
      leakAlreadyAlerted = true; 
    }
  } else {
    digitalWrite(BUZZER_PIN, HIGH);  // HIGH silences the low-level buzzer
    digitalWrite(VALVE_PIN, HIGH);   // HIGH returns relay to open state
    
    // Reset leak state flag if concentrations drop to safe levels
    if (leakAlreadyAlerted) {
      Serial.println("\n[Environment Safe: Gas Leakage Cleared. Valve Re-opened.]");
      uploadLogToFirebase(sensorValue, "SAFE (VALVE OPEN)");
      leakAlreadyAlerted = false;
    }
  }

  // --- STANDARD 24x7 COMPLIANCE UPDATE (EVERY 30 SECONDS) ---
  if (millis() - lastSendTime >= sendInterval) {
    lastSendTime = millis();
    
    String systemStatus = isLeakDetected ? "LEAKAGE DETECTED (VALVE CLOSED)" : "SAFE (VALVE OPEN)";

    Serial.println("\n====== 24/7 AIR MONITORING UPDATE ======");
    Serial.print("Real-Time Clock           : "); Serial.println(getRealTime());
    Serial.print("Gas Concentration Level   : "); Serial.println(sensorValue);
    Serial.print("Current System Status     : "); Serial.println(systemStatus);
    Serial.println("========================================");

    // Standard routine update
    uploadLogToFirebase(sensorValue, systemStatus);
  }
  
  delay(100); 
}