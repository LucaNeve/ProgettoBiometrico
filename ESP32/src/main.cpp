#include <Arduino.h>
#include <Adafruit_Fingerprint.h>

#include <WiFi.h>
#include <HTTPClient.h>

// Collegamenti hardware
#define RX_PIN 16  // RX del sensore al pin 16 dell'ESP32
#define TX_PIN 17  // TX del sensore al pin 17 dell'ESP32
#define TOUCH_PIN 18  // Pin collegato al pin TOUCH del sensore


// Configurazione WiFi
const char* ssid = "A25";
const char* password = "biancaneve";

String serverBase = "http://10.157.62.245/backend/";

// Creazione dell'oggetto sensore fingerprint
HardwareSerial fingerSerial(1);  // HardwareSerial su UART1
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

// Dichiarazione funzioni
void eseguiAccesso();
void aggiornaUltimoIngresso(int id);

void registraImpronta(String nome, String cognome, String stanza);
void registraUtente(String nome, String cognome, String stanza, int id_template);

void clearAll();
void eliminaTuttiUtenti();

void eliminaImpronta(int id);
void eliminaUtente(int id);

int cercaIdDisponibile();

void setup(){
  // Impostazione delle comunicazioni seriali
  Serial.begin(9600);   // Debug seriale
  fingerSerial.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN);  // Serial per il sensore

  // Connessione WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    delay(500);

  // Inizializzazione sensore
  Serial.println("Inizializzazione sensore di impronte digitali...");
    finger.begin(57600);
    if (finger.verifyPassword()) {
        Serial.println("Sensore di impronte inizializzato correttamente!");
    } else {
        Serial.println("Errore nell'inizializzazione del sensore.");
        while (true);
    }
}

void loop() {
  if (Serial.available()) {  // Controlla se Qt ha inviato un comando
    String comando = Serial.readStringUntil('\n');  // Legge il comando
    comando.trim();  // Rimuove spazi vuoti e caratteri extra

    if (comando.startsWith("REGISTRA")) {
      int firstSep = comando.indexOf(';');
      int secondSep = comando.indexOf(';', firstSep + 1);
      int thirdSep = comando.indexOf(';', secondSep + 1);

      if (firstSep > 0 && secondSep > firstSep && thirdSep > secondSep) {
        String nome = comando.substring(firstSep + 1, secondSep);
        String cognome = comando.substring(secondSep + 1, thirdSep);
        String stanza = comando.substring(thirdSep + 1);

        registraImpronta(nome, cognome, stanza); // Avvia la registrazione con i dati
      } else {
        Serial.println("ERRORE: Formato comando REGISTRA errato!");
      }

    }else if(comando.startsWith("ELIMINA")){
      int firstSep = comando.indexOf(';');  // Trova il primo separatore
    if (firstSep != -1) { // Controlla se il separatore esiste
        String idString = comando.substring(firstSep + 1); // Prendi tutto dopo ";"
        idString.trim();  // Rimuove spazi o caratteri extra

        int id = idString.toInt(); // Converte in intero
        if (id > 0) { // Controlla che sia un ID valido
            eliminaImpronta(id);
        }
    }
    }else if (comando == "PULISCI") {
      clearAll();
    }else if (comando == "ACCEDI") {
      eseguiAccesso();
    }/*else {
      Serial.println("ERRORE: Comando non riconosciuto.");
    }*/
  }
}


// Confronto/Accesso
void eseguiAccesso(){
  Serial.println("Tocca il sensore per verificare l'identità.");
  delay(1000);

  Serial.println("Poggia il dito sul sensore...");
  while (finger.getImage() != FINGERPRINT_OK) {
    delay(1000);
    if (Serial.available()) {  // Controlla se Qt ha inviato un comando
      String comando = Serial.readStringUntil('\n');  // Legge il comando
      comando.trim();  // Rimuove spazi vuoti e caratteri extra
      if(comando == "INDIETRO")
        return;
    }
  }

  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Errore nella conversione in template.");
    delay(1000);
    return;
  }

  if (finger.fingerSearch() != FINGERPRINT_OK) {
    Serial.println("Impronta non riconosciuta.");
    delay(1000);
    return;
  }

  Serial.println("Accesso consentito! ID trovato: " + String(finger.fingerID));
  delay(2000);

  aggiornaUltimoIngresso(finger.fingerID);
}

void aggiornaUltimoIngresso(int id) {
  if (WiFi.status() == WL_CONNECTED) {
    String serverName = serverBase + "aggiorna_accesso.php";
    //String serverName = serverBase + "update_access";
    HTTPClient http;
    String payload = "{\"id_template\":" + String(id) + "}";

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.print("Risposta dal server: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Errore HTTP: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi non connesso, impossibile inviare dati.");
  }
}


// Registra una nuova impronta
void registraImpronta(String nome, String cognome, String stanza) {
  
  Serial.println("Inizia la registrazione della nuova impronta.");
  delay(1000);
  int id = cercaIdDisponibile();
  if (id == -1) {
    Serial.print("Errore: database pieno.");
    return;
  }
  Serial.println("Utilizzerò l'ID: " + String(id));
  delay(2000);

  // Prima scansione
  Serial.println("Poggia il dito sul sensore...");
  while (finger.getImage() != FINGERPRINT_OK) {
    delay(1000);
    if (Serial.available()) {  // Controlla se Qt ha inviato un comando
      String comando = Serial.readStringUntil('\n');  // Legge il comando
      comando.trim();  // Rimuove spazi vuoti e caratteri extra
      if(comando == "INDIETRO")
        return;
    }
  }

  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Errore nella conversione in template.");
    return;
  }
  Serial.println("Rimuovi il dito e attendi...");

  delay(2000);  // Pausa per rimozione dito

  // Seconda scansione
  Serial.println("Poggia il dito sul sensore...");
  while (finger.getImage() != FINGERPRINT_OK) {
    delay(1000);
    if (Serial.available()) {  // Controlla se Qt ha inviato un comando
      String comando = Serial.readStringUntil('\n');  // Legge il comando
      comando.trim();  // Rimuove spazi vuoti e caratteri extra
      if(comando == "INDIETRO")
        return;
    }
  }

  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("Errore nella conversione in template.");
    return;
  }

  // Creazione e memorizzazione del modello
  if (finger.createModel() != FINGERPRINT_OK) {
    Serial.println("Errore nella creazione del modello.");
    return;
  }

  if (finger.storeModel(id) != FINGERPRINT_OK) {
    Serial.println("Errore nel salvataggio del modello.");
    return;
  }
  delay(1000);
  Serial.println("Impronta registrata correttamente con ID: " + String(id));

  registraUtente(nome, cognome, stanza, id);

}

void registraUtente(String nome, String cognome, String stanza, int id_template){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Configurazione URL del server
    String serverName = serverBase + "aggiungi_utente.php";
    //String serverName = serverBase + "register";

    // Costruzione del payload
    String payload = "Nome=" + nome + "&Cognome=" + cognome + "&Stanza=" + stanza + "&ID_Template=" + String(id_template);

    // Configurazione richiesta HTTP POST
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Invio della richiesta POST
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.print("Risposta del server: ");
      Serial.println(httpResponseCode);
      Serial.println(http.getString()); // Mostra la risposta del server
    } else {
      Serial.print("Errore nella connessione al server: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Errore: WiFi non connesso.");
  }
}


// Rimozione impronte
void clearAll(){
  Serial.println("Eliminazione di tutte le impronte...");
  for (int id = 1; id <= finger.capacity; id++) {
    if (finger.deleteModel(id) == FINGERPRINT_OK) {
      //Serial.print("Impronta ID "); Serial.print(id); Serial.println(" eliminata.");
    }
  }
  Serial.println("Tutte le impronte sono state eliminate.");
  delay(1000);

  eliminaTuttiUtenti();
}
// Funzione per eliminare tutti gli utenti nel server
void eliminaTuttiUtenti() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Configurazione URL del server
    String serverName = serverBase + "elimina_tutti_utenti.php";
    //String serverName = serverBase + "delete_all";


    // Configurazione richiesta HTTP POST
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Invio della richiesta POST senza payload
    int httpResponseCode = http.POST("");

    if (httpResponseCode > 0) {
      Serial.print("Risposta del server: ");
      Serial.println(httpResponseCode);
      Serial.println(http.getString()); // Mostra la risposta del server
    } else {
      Serial.print("Errore nella connessione al server: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Errore: WiFi non connesso.");
  }
}



void eliminaImpronta(int id){
  if (finger.loadModel(id) != FINGERPRINT_OK) {
    Serial.println("Nessuna impronta trovata con questo ID.");
    return;
  }

  if (finger.deleteModel(id) == FINGERPRINT_OK) {
    Serial.print("Impronta ID "); Serial.print(id); Serial.println(" eliminata con successo!");
    eliminaUtente(id);
  } else {
    Serial.println("Errore durante l'eliminazione dell'impronta.");
  }
}

void eliminaUtente(int id){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Configurazione URL del server
    String serverName = serverBase + "elimina_utente.php";

    // Costruzione del payload
    String payload = "ID_Template=" + String(id);

    // Configurazione richiesta HTTP POST
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Invio della richiesta POST
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.print("Risposta del server: ");
      Serial.println(httpResponseCode);
      Serial.println(http.getString()); // Mostra la risposta del server
    } else {
      Serial.print("Errore nella connessione al server: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Errore: WiFi non connesso.");
  }
}

// Trova il primo id disponibile
int cercaIdDisponibile() {
  for (int id = 1; id <= finger.capacity; id++) {
    if (finger.loadModel(id) != FINGERPRINT_OK) {
      return id;  // Ritorna il primo ID disponibile
    }
  }
  return -1;  // Nessun ID disponibile
}