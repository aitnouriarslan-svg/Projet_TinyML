/*
 * 1-DataGeneration_Arduino/data_generation.ino
 * Version interactive pour PuTTY :
 * 1. Attend la touche 'S' pour démarrer.
 * 2. Enregistre pendant 60 secondes exactement.
 * 3. S'arrête et attend de nouveau 'S'.
 */

#include <Arduino_LSM9DS1.h>

const unsigned long DUREE_CAPTURE = 120000; // 60 secondes en millisecondes
bool acquisitionActive = false;            // État du système
unsigned long tempsDebut = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial); // Attente de la connexion série

  if (!IMU.begin()) {
    Serial.println("Erreur critique : Impossible d'initialiser l'IMU !");
    while (1);
  }

  // Message d'accueil pour PuTTY
  Serial.println("---------------------------------------------");
  Serial.println("SYSTEME PRET.");
  Serial.println("Tapez 'S' (ou 's') pour lancer une capture de 60 secondes.");
  Serial.println("---------------------------------------------");
}

void loop() {
  // 1. GESTION DES COMMANDES UTILISATEUR
  if (Serial.available() > 0) {
    char commande = Serial.read();
    
    // Si l'utilisateur tape S et qu'on n'est pas déjà en train d'enregistrer
    if ((commande == 'S' || commande == 's') && !acquisitionActive) {
      acquisitionActive = true;
      tempsDebut = millis();
      
      Serial.println("DEMARRAGE DE L'ACQUISITION...");
      Serial.println("aX,aY,aZ"); // On imprime l'en-tête CSV juste avant les données
    }
  }

  // 2. LOGIQUE D'ACQUISITION
  if (acquisitionActive) {
    
    // Vérifier si les 60 secondes sont écoulées
    if (millis() - tempsDebut >= DUREE_CAPTURE) {
      acquisitionActive = false;
      Serial.println("---------------------------------------------");
      Serial.println("FIN DE LA CAPTURE (60s ecoulees).");
      Serial.println("Copiez vos donnees maintenant.");
      Serial.println("Tapez 'S' pour relancer une nouvelle capture.");
      Serial.println("---------------------------------------------");
    } 
    else {
      // Si on est encore dans les temps, on lit et on envoie
      if (IMU.accelerationAvailable()) {
        float x, y, z;
        IMU.readAcceleration(x, y, z);

        Serial.print(x);
        Serial.print(',');
        Serial.print(y);
        Serial.print(',');
        Serial.println(z); // Retour à la ligne pour la prochaine donnée
        
        delay(20); // Environ 50Hz
      }
    }
  }
}