/*
 * inference_vibrations.ino
 * VERSION FINALE : "SIMPLE & STABLE"
 * 1. Supprime la gravité (marche dans tous les sens)
 * 2. Lisse l'affichage (ne clignote pas)
 */

#include <Arduino_LSM9DS1.h>
#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/micro/micro_error_reporter.h> 

#include "model.h" 

// --- PARAMETRES ---
const int SEQ_LENGTH = 50;  // Nombre d'échantillons (1 seconde)
const int AXIS = 3;         // X, Y, Z

// --- VARIABLES MEMOIRE IA ---
const int kTensorArenaSize = 8 * 1024; 
alignas(16) uint8_t tensor_arena[kTensorArenaSize];

// --- VARIABLES DE LISSAGE (Pour éviter que ça clignote) ---
float smooth_repos = 0.0;
float smooth_faible = 0.0;
float smooth_forte = 0.0;
// Facteur de lissage (0.2 = fluide, 0.1 = très lent/stable)
const float ALPHA = 0.2; 

// --- POINTEURS TENSORFLOW ---
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // 1. Démarrage du capteur
  if (!IMU.begin()) {
    Serial.println("Erreur IMU !");
    while (1);
  }

  // 2. Configuration TinyML
  static tflite::MicroErrorReporter micro_error_reporter;
  const tflite::Model* model = tflite::GetModel(model_data);
  static tflite::AllOpsResolver resolver;
  
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, &micro_error_reporter);
  interpreter = &static_interpreter;

  // 3. Allocation Mémoire
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("Erreur Memoire !");
    while(1);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("Systeme pret ! (Mode Anti-Gravite)");
}

void loop() {
  float buffer_fenetre[SEQ_LENGTH * AXIS];
  
  // --- ETOPE 1 : ACQUISITION (1 seconde) ---
  for (int i = 0; i < SEQ_LENGTH; i++) {
    // Attente active pour respecter les 50Hz environ
    while (!IMU.accelerationAvailable());
    
    float x, y, z;
    IMU.readAcceleration(x, y, z);
    
    buffer_fenetre[i * 3 + 0] = x;
    buffer_fenetre[i * 3 + 1] = y;
    buffer_fenetre[i * 3 + 2] = z;
    
    delay(20); 
  }

  // --- ETAPE 2 : SUPPRESSION DE LA GRAVITE (Centrage) ---
  // C'est ici que la magie opère : on calcule la moyenne pour la retirer.
  float moyX = 0, moyY = 0, moyZ = 0;

  for (int i = 0; i < SEQ_LENGTH; i++) {
    moyX += buffer_fenetre[i * 3 + 0];
    moyY += buffer_fenetre[i * 3 + 1];
    moyZ += buffer_fenetre[i * 3 + 2];
  }
  moyX /= SEQ_LENGTH;
  moyY /= SEQ_LENGTH;
  moyZ /= SEQ_LENGTH;

  // Remplissage de l'IA en soustrayant la moyenne
  for (int i = 0; i < SEQ_LENGTH; i++) {
    input->data.f[i * 3 + 0] = buffer_fenetre[i * 3 + 0] - moyX;
    input->data.f[i * 3 + 1] = buffer_fenetre[i * 3 + 1] - moyY;
    input->data.f[i * 3 + 2] = buffer_fenetre[i * 3 + 2] - moyZ;
  }

  // --- ETAPE 3 : INFERENCE ---
  interpreter->Invoke();

  // --- ETAPE 4 : LISSAGE DES RESULTATS ---
  // On mélange le nouveau résultat (20%) avec l'ancien (80%)
  float new_repos = output->data.f[0];
  float new_faible = output->data.f[1];
  float new_forte = output->data.f[2];

  smooth_repos = (1.0 - ALPHA) * smooth_repos + ALPHA * new_repos;
  smooth_faible = (1.0 - ALPHA) * smooth_faible + ALPHA * new_faible;
  smooth_forte = (1.0 - ALPHA) * smooth_forte + ALPHA * new_forte;

  // --- ETAPE 5 : AFFICHAGE ET DECISION ---
  Serial.print("Repos:"); Serial.print(smooth_repos);
  Serial.print(" Faible:"); Serial.print(smooth_faible);
  Serial.print(" Forte:"); Serial.print(smooth_forte);
  Serial.print("\t >>> ");

  // La logique du "Qui est le plus fort ?"
  if (smooth_repos > smooth_faible && smooth_repos > smooth_forte) {
    Serial.println("REPOS");
  } 
  else if (smooth_forte > smooth_repos && smooth_forte > smooth_faible) {
    Serial.println("ALERTE FORTE !");
  } 
  else {
    Serial.println("Normal (Faible)");
  }
}