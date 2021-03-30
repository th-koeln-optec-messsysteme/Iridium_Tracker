#include <SoftwareSerial.h>

#include <TinyGPS.h>

#include <IridiumSBD.h>

SoftwareSerial IridiumSerial(18, 19);

#define DIAGNOSTICS false

IridiumSBD modem(IridiumSerial, 10);
void setup() {
  // put your setup code here, to run once:
  int signalQuality = -1;
  int err = 0;

  Serial.begin(19200);
  while (!Serial);

  IridiumSerial.begin(19200);

  Serial.println("Starte den Rockblock...");
  // Der folgende Befehl dauert ca. 4 Minuten!
  err = modem.begin();
  Serial.println(err);
  if (err != ISBD_SUCCESS) {
    Serial.println("Begin failed: error ");
    Serial.println(err);
    if (err == ISBD_NO_MODEM_DETECTED)
      Serial.println("Rockblock nicht erreicht.");
    return;
  }

  // Testen der Signalqualität, gibt einen Wert zw. 1 und 5 wieder
  // Sollte besser als 2 sein
  err = modem.getSignalQuality(signalQuality);
  if (err != ISBD_SUCCESS) {
    Serial.println("Signalqualität konnte nicht bestimmt werden.");
    Serial.println(err);
    return;
  }
  Serial.print("Auf einer Skala von 1 bis 5 liegt die Signalqualität bei ");
  Serial.print(signalQuality);
  Serial.print(".");

  // Senden der Nachricht
  Serial.println("Nachricht wird versucht zu senden. Dies kann ein paar Minuten dauern.");
  err = modem.sendSBDText("Hallo Welt!");
  if (err != ISBD_SUCCESS) {
    Serial.println("Nachricht konnte nicht gesendet werden.");
    Serial.println(err);
    if (err == ISBD_SENDRECEIVE_TIMEOUT) {
      Serial.println("Sendezeit wurde überschritten. Vermutlich kein Empfang der Antenne. Versuche es von woanders.");
    }
  }

  else {
    Serial.println("Super! Es hat funktioniert!");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Ich bin im Loop angekommen.");
  delay(10000);
}

#if DIAGNOSTICS
void ISBDConsoleCallback(IridiumSBD *device, char c){
  Serial.write(c);
}
void ISBDDiagsCallback(IridiumSBD *device, char c) {
  Serial.write(c);
}
#endif
