/*
 * arduino_camera_classification.ino
 * Capture d'images via la caméra OV7670 et classification
 * avec le modèle Edge Impulse.
 * Résultats envoyés via Serial vers Node-RED.
 * Format de sortie : "classe,confiance"  ex: "capacitor,0.87"
 */

/* Librairies nécessaires */
#include <Classification_Composants_Electroniques_inferencing.h> // Modèle Edge Impulse
#include <Arduino_OV767X.h>                                       // Pilote caméra OV7670
#include <stdint.h>
#include <stdlib.h>

/* Dimensions brutes de la caméra OV7670 en mode QQVGA */
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS     160
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS     120

/* Macro pour aligner un pointeur sur 4 octets (requis pour certaines opérations mémoire) */
#define DWORD_ALIGN_PTR(a)   ((a & 0x3) ?(((uintptr_t)a + 0x4) & ~(uintptr_t)0x3) : a)

/* Seuil de confiance minimum pour envoyer une détection à Node-RED */
#define CONFIDENCE_THRESHOLD  0.6

/*
 * Classe OV7675 : étend OV767X pour lire les frames de la caméra
 * et les redimensionner pour Edge Impulse (96x96)
 */
class OV7675 : public OV767X {
    public:
        int begin(int resolution, int format, int fps);
        void readFrame(void* buffer);
    private:
        int vsyncPin, hrefPin, pclkPin, xclkPin;
        volatile uint32_t* vsyncPort;
        uint32_t vsyncMask;
        volatile uint32_t* hrefPort;
        uint32_t hrefMask;
        volatile uint32_t* pclkPort;
        uint32_t pclkMask;
        uint16_t width, height;
        uint8_t bytes_per_pixel;
        uint16_t bytes_per_row;
        uint8_t buf_rows;
        uint16_t buf_size;
        uint8_t resize_height;
        uint8_t *raw_buf;
        void *buf_mem;
        uint8_t *intrp_buf;
        uint8_t *buf_limit;
        void readBuf();
        int allocate_scratch_buffs();
        int deallocate_scratch_buffs();
};

/* Structure pour stocker les résolutions de redimensionnement disponibles */
typedef struct {
    size_t width;
    size_t height;
} ei_device_resize_resolutions_t;

/* Fonctions utilitaires pour lire le port Serial (utilisées par Edge Impulse) */
int ei_get_serial_available(void) { return Serial.available(); }
char ei_get_serial_byte(void) { return Serial.read(); }

/* Variables globales */
static OV7675 Cam;                          // Instance de la caméra
static bool is_initialised = false;         // Etat d'initialisation de la caméra
static uint8_t *ei_camera_capture_out = NULL; // Pointeur vers le buffer de capture
uint32_t resize_col_sz;                     // Largeur après redimensionnement
uint32_t resize_row_sz;                     // Hauteur après redimensionnement
bool do_resize = false;                     // Indique si un redimensionnement est nécessaire
bool do_crop = false;                       // Indique si un recadrage est nécessaire
static bool debug_nn = false;               // Mode debug du réseau de neurones (désactivé)

/* Déclarations des fonctions */
bool ei_camera_init(void);
void ei_camera_deinit(void);
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf);
int calculate_resize_dimensions(uint32_t out_width, uint32_t out_height, uint32_t *resize_col_sz, uint32_t *resize_row_sz, bool *do_resize);
void resizeImage(int srcWidth, int srcHeight, uint8_t *srcImage, int dstWidth, int dstHeight, uint8_t *dstImage, int iBpp);
void cropImage(int srcWidth, int srcHeight, uint8_t *srcImage, int startX, int startY, int dstWidth, int dstHeight, uint8_t *dstImage, int iBpp);

/* ----------------------------------------------------------------
 * setup() : initialisation du port Serial et affichage des
 * paramètres du modèle Edge Impulse
 * ---------------------------------------------------------------- */
void setup()
{
    Serial.begin(115200);
    while (!Serial);
    Serial.println("================================================");
    Serial.println("  TinyML - Classification de composants");
    Serial.println("  Arduino Nano 33 BLE Sense + OV7670");
    Serial.println("================================================");
    ei_printf("Inferencing settings:\n");
    ei_printf("\tImage resolution: %dx%d\n", EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT);
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));
    Serial.println("Format sortie : classe,confiance  ex: capacitor,0.87");
    Serial.println("------------------------------------------------");
}

/* ----------------------------------------------------------------
 * loop() : boucle principale
 * - Capture une image toutes les 2 secondes
 * - Lance l'inférence Edge Impulse
 * - Envoie le résultat via Serial si confiance >= 60%
 * ---------------------------------------------------------------- */
void loop()
{
    bool stop_inferencing = false;

    while(stop_inferencing == false) {

        ei_printf("\nCapture dans 2 secondes...\n");
        if (ei_sleep(2000) != EI_IMPULSE_OK) { break; }

        ei_printf("Capture en cours...\n");

        // Initialiser la caméra si ce n'est pas déjà fait
        if (ei_camera_init() == false) {
            ei_printf("ERR: Echec initialisation camera\r\n");
            break;
        }

        // Calculer les dimensions de redimensionnement nécessaires
        uint32_t resize_col_sz;
        uint32_t resize_row_sz;
        bool do_resize = false;
        int res = calculate_resize_dimensions(EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, &resize_col_sz, &resize_row_sz, &do_resize);
        if (res) { ei_printf("ERR: Echec calcul dimensions (%d)\r\n", res); break; }

        // Allouer la mémoire pour le buffer de capture
        void *snapshot_mem = NULL;
        uint8_t *snapshot_buf = NULL;
        snapshot_mem = ei_malloc(resize_col_sz * resize_row_sz * 2);
        if(snapshot_mem == NULL) { ei_printf("ERR: Echec allocation memoire\r\n"); break; }
        snapshot_buf = (uint8_t *)DWORD_ALIGN_PTR((uintptr_t)snapshot_mem);

        // Capturer l'image depuis la caméra
        if (ei_camera_capture(EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf) == false) {
            ei_printf("ERR: Echec capture image\r\n");
            if (snapshot_mem) ei_free(snapshot_mem);
            break;
        }

        // Préparer le signal d'entrée pour Edge Impulse
        ei::signal_t signal;
        signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
        signal.get_data = &ei_camera_cutout_get_data;

        // Lancer l'inférence
        ei_impulse_result_t result = { 0 };
        EI_IMPULSE_ERROR ei_error = run_classifier(&signal, &result, debug_nn);
        if (ei_error != EI_IMPULSE_OK) {
            ei_printf("ERR: Echec inférence (%d)\n", ei_error);
            ei_free(snapshot_mem);
            break;
        }

        // Afficher les scores de toutes les classes (debug)
        ei_printf("Predictions (DSP: %d ms, Classification: %d ms):\n",
                  result.timing.dsp, result.timing.classification);
        for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
            ei_printf("  %s: %.5f\r\n",
                      ei_classifier_inferencing_categories[i],
                      result.classification[i].value);
        }

        // Trouver la classe avec le score le plus élevé
        float max_score = 0.0;
        int   max_index = 0;
        for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
            if (result.classification[i].value > max_score) {
                max_score = result.classification[i].value;
                max_index = i;
            }
        }

        String classe = String(ei_classifier_inferencing_categories[max_index]);

        // Envoyer à Node-RED uniquement si :
        // - La classe n'est pas "unknown"
        // - La confiance est supérieure au seuil (60%)
        if (classe != "unknown" && max_score >= CONFIDENCE_THRESHOLD) {
            Serial.print(classe);
            Serial.print(",");
            Serial.println(max_score);
            ei_printf(">>> Envoyé à Node-RED : %s (%.2f)\n", classe.c_str(), max_score);
        } else {
            ei_printf(">>> Ignoré : %s (%.2f)\n", classe.c_str(), max_score);
        }

        // Vérifier si l'utilisateur envoie 'b' pour arrêter l'inférence
        while (ei_get_serial_available() > 0) {
            if (ei_get_serial_byte() == 'b') {
                ei_printf("Inférence arrêtée\r\n");
                stop_inferencing = true;
            }
        }

        // Libérer la mémoire allouée pour cette frame
        if (snapshot_mem) ei_free(snapshot_mem);
    }

    // Désactiver la caméra en fin de boucle
    ei_camera_deinit();
}

/* ----------------------------------------------------------------
 * calculate_resize_dimensions()
 * Détermine les dimensions intermédiaires de redimensionnement
 * en fonction de la résolution cible demandée par Edge Impulse
 * ---------------------------------------------------------------- */
int calculate_resize_dimensions(uint32_t out_width, uint32_t out_height, uint32_t *resize_col_sz, uint32_t *resize_row_sz, bool *do_resize)
{
    size_t list_size = 2;
    // Résolutions intermédiaires disponibles
    const ei_device_resize_resolutions_t list[list_size] = { {42,32}, {128,96} };
    *resize_col_sz = EI_CAMERA_RAW_FRAME_BUFFER_COLS;
    *resize_row_sz = EI_CAMERA_RAW_FRAME_BUFFER_ROWS;
    *do_resize = false;
    for (size_t ix = 0; ix < list_size; ix++) {
        if ((out_width <= list[ix].width) && (out_height <= list[ix].height)) {
            *resize_col_sz = list[ix].width;
            *resize_row_sz = list[ix].height;
            *do_resize = true;
            break;
        }
    }
    return 0;
}

/* ----------------------------------------------------------------
 * ei_camera_init()
 * Initialise la caméra OV7670 en mode QQVGA RGB565
 * ---------------------------------------------------------------- */
bool ei_camera_init(void) {
    if (is_initialised) return true; // Déjà initialisée
    if (!Cam.begin(QQVGA, RGB565, 1)) {
        ei_printf("ERR: Echec initialisation camera\r\n");
        return false;
    }
    is_initialised = true;
    return true;
}

/* ----------------------------------------------------------------
 * ei_camera_deinit()
 * Libère la caméra
 * ---------------------------------------------------------------- */
void ei_camera_deinit(void) {
    if (is_initialised) { Cam.end(); is_initialised = false; }
}

/* ----------------------------------------------------------------
 * ei_camera_capture()
 * Capture une image et applique redimensionnement et recadrage
 * pour obtenir la résolution attendue par Edge Impulse (96x96)
 * ---------------------------------------------------------------- */
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf)
{
    if (!is_initialised) { ei_printf("ERR: Camera non initialisée\r\n"); return false; }
    if (!out_buf) { ei_printf("ERR: Paramètres invalides\r\n"); return false; }

    int res = calculate_resize_dimensions(img_width, img_height, &resize_col_sz, &resize_row_sz, &do_resize);
    if (res) { ei_printf("ERR: Echec calcul dimensions (%d)\r\n", res); return false; }

    // Vérifier si un recadrage est nécessaire après redimensionnement
    if ((img_width != resize_col_sz) || (img_height != resize_row_sz)) { do_crop = true; }

    // Lire la frame depuis la caméra
    Cam.readFrame(out_buf);

    // Recadrer l'image au centre si nécessaire
    if (do_crop) {
        uint32_t crop_col_sz, crop_row_sz, crop_col_start, crop_row_start;
        crop_row_start = (resize_row_sz - img_height) / 2;
        crop_col_start = (resize_col_sz - img_width) / 2;
        crop_col_sz = img_width;
        crop_row_sz = img_height;
        cropImage(resize_col_sz, resize_row_sz, out_buf,
                  crop_col_start, crop_row_start,
                  crop_col_sz, crop_row_sz, out_buf, 16);
    }
    ei_camera_capture_out = out_buf;
    return true;
}

/* ----------------------------------------------------------------
 * ei_camera_cutout_get_data()
 * Convertit les pixels RGB565 en valeurs float RGB888
 * requis par Edge Impulse pour l'inférence
 * ---------------------------------------------------------------- */
int ei_camera_cutout_get_data(size_t offset, size_t length, float *out_ptr) {
    size_t pixel_ix = offset * 2;
    size_t bytes_left = length;
    size_t out_ptr_ix = 0;
    while (bytes_left != 0) {
        // Lire un pixel RGB565 (2 octets)
        uint16_t pixel = (ei_camera_capture_out[pixel_ix] << 8) | ei_camera_capture_out[pixel_ix+1];
        // Extraire les composantes R, G, B
        uint8_t r = ((pixel >> 11) & 0x1f) << 3;
        uint8_t g = ((pixel >> 5) & 0x3f) << 2;
        uint8_t b = (pixel & 0x1f) << 3;
        // Encoder en RGB888 sous forme float
        out_ptr[out_ptr_ix] = (float)((r << 16) + (g << 8) + b);
        out_ptr_ix++; pixel_ix += 2; bytes_left--;
    }
    return 0;
}

/* Constantes pour l'interpolation bilinéaire (précision en virgule fixe) */
#ifdef __ARM_FEATURE_SIMD32
#include <device.h>
#endif
#define FRAC_BITS 14
#define FRAC_VAL (1<<FRAC_BITS)
#define FRAC_MASK (FRAC_VAL - 1)

/* ----------------------------------------------------------------
 * resizeImage()
 * Redimensionne une image par interpolation bilinéaire
 * Supporte les formats 8 bits (niveaux de gris) et 16 bits (RGB565)
 * ---------------------------------------------------------------- */
void resizeImage(int srcWidth, int srcHeight, uint8_t *srcImage, int dstWidth, int dstHeight, uint8_t *dstImage, int iBpp)
{
    uint32_t src_x_accum, src_y_accum, x_frac, nx_frac, y_frac, ny_frac;
    int x, y, ty;
    if (iBpp != 8 && iBpp != 16) return;
    src_y_accum = FRAC_VAL/2;
    const uint32_t src_x_frac = (srcWidth * FRAC_VAL) / dstWidth;
    const uint32_t src_y_frac = (srcHeight * FRAC_VAL) / dstHeight;
    const uint32_t r_mask = 0xf800f800;
    const uint32_t g_mask = 0x07e007e0;
    const uint32_t b_mask = 0x001f001f;
    uint8_t *s, *d;
    uint16_t *s16, *d16;
    uint32_t x_frac2, y_frac2;
    for (y=0; y < dstHeight; y++) {
        ty = src_y_accum >> FRAC_BITS;
        y_frac = src_y_accum & FRAC_MASK;
        src_y_accum += src_y_frac;
        ny_frac = FRAC_VAL - y_frac;
        y_frac2 = ny_frac | (y_frac << 16);
        s = &srcImage[ty * srcWidth];
        s16 = (uint16_t *)&srcImage[ty * srcWidth * 2];
        d = &dstImage[y * dstWidth];
        d16 = (uint16_t *)&dstImage[y * dstWidth * 2];
        src_x_accum = FRAC_VAL/2;
        if (iBpp == 8) {
            // Interpolation pour image en niveaux de gris (8 bits)
            for (x=0; x < dstWidth; x++) {
                uint32_t tx, p00, p01, p10, p11;
                tx = src_x_accum >> FRAC_BITS;
                x_frac = src_x_accum & FRAC_MASK;
                nx_frac = FRAC_VAL - x_frac;
                x_frac2 = nx_frac | (x_frac << 16);
                src_x_accum += src_x_frac;
                p00 = s[tx]; p10 = s[tx+1];
                p01 = s[tx+srcWidth]; p11 = s[tx+srcWidth+1];
                p00 = ((p00 * nx_frac) + (p10 * x_frac) + FRAC_VAL/2) >> FRAC_BITS;
                p01 = ((p01 * nx_frac) + (p11 * x_frac) + FRAC_VAL/2) >> FRAC_BITS;
                p00 = ((p00 * ny_frac) + (p01 * y_frac) + FRAC_VAL/2) >> FRAC_BITS;
                *d++ = (uint8_t)p00;
            }
        } else {
            // Interpolation pour image couleur RGB565 (16 bits)
            for (x=0; x < dstWidth; x++) {
                uint32_t tx, p00, p01, p10, p11;
                uint32_t r00,r01,r10,r11,g00,g01,g10,g11,b00,b01,b10,b11;
                tx = src_x_accum >> FRAC_BITS;
                x_frac = src_x_accum & FRAC_MASK;
                nx_frac = FRAC_VAL - x_frac;
                x_frac2 = nx_frac | (x_frac << 16);
                src_x_accum += src_x_frac;
                // Lire les 4 pixels voisins et corriger l'endianness
                p00 = __builtin_bswap16(s16[tx]); p10 = __builtin_bswap16(s16[tx+1]);
                p01 = __builtin_bswap16(s16[tx+srcWidth]); p11 = __builtin_bswap16(s16[tx+srcWidth+1]);
                // Extraire les canaux R, G, B de chaque pixel
                r00=(p00&r_mask)>>1; g00=p00&g_mask; b00=p00&b_mask;
                r10=(p10&r_mask)>>1; g10=p10&g_mask; b10=p10&b_mask;
                r01=(p01&r_mask)>>1; g01=p01&g_mask; b01=p01&b_mask;
                r11=(p11&r_mask)>>1; g11=p11&g_mask; b11=p11&b_mask;
                // Interpoler chaque canal séparément
                r00=((r00*nx_frac)+(r10*x_frac)+FRAC_VAL/2)>>FRAC_BITS;
                r01=((r01*nx_frac)+(r11*x_frac)+FRAC_VAL/2)>>FRAC_BITS;
                r00=((r00*ny_frac)+(r01*y_frac)+FRAC_VAL/2)>>FRAC_BITS;
                g00=((g00*nx_frac)+(g10*x_frac)+FRAC_VAL/2)>>FRAC_BITS;
                g01=((g01*nx_frac)+(g11*x_frac)+FRAC_VAL/2)>>FRAC_BITS;
                g00=((g00*ny_frac)+(g01*y_frac)+FRAC_VAL/2)>>FRAC_BITS;
                b00=((b00*nx_frac)+(b10*x_frac)+FRAC_VAL/2)>>FRAC_BITS;
                b01=((b01*nx_frac)+(b11*x_frac)+FRAC_VAL/2)>>FRAC_BITS;
                b00=((b00*ny_frac)+(b01*y_frac)+FRAC_VAL/2)>>FRAC_BITS;
                // Recombiner les canaux et corriger l'endianness
                p00=((r00<<1)&r_mask)|(g00&g_mask)|(b00&b_mask);
                *d16++ = (uint16_t)__builtin_bswap16(p00);
            }
        }
    }
}

/* ----------------------------------------------------------------
 * cropImage()
 * Recadre une région de l'image source vers l'image destination
 * Supporte les formats 8 bits et 16 bits
 * ---------------------------------------------------------------- */
void cropImage(int srcWidth, int srcHeight, uint8_t *srcImage, int startX, int startY, int dstWidth, int dstHeight, uint8_t *dstImage, int iBpp)
{
    uint32_t *s32, *d32;
    int x, y;
    // Vérifier que les paramètres de recadrage sont valides
    if (startX<0||startX>=srcWidth||startY<0||startY>=srcHeight||(startX+dstWidth)>srcWidth||(startY+dstHeight)>srcHeight) return;
    if (iBpp != 8 && iBpp != 16) return;
    if (iBpp == 8) {
        uint8_t *s, *d;
        for (y=0; y<dstHeight; y++) {
            s = &srcImage[srcWidth*(y+startY)+startX];
            d = &dstImage[dstWidth*y];
            x = 0;
            // Copie optimisée par blocs de 4 octets si alignement correct
            if ((intptr_t)s&3||(intptr_t)d&3) { for(;x<dstWidth;x++){*d++=*s++;} }
            else {
                s32=(uint32_t*)s; d32=(uint32_t*)d;
                for(;x<dstWidth-3;x+=4){*d32++=*s32++;}
                s=(uint8_t*)s32; d=(uint8_t*)d32;
                for(;x<dstWidth;x++){*d++=*s++;}
            }
        }
    } else {
        uint16_t *s, *d;
        for (y=0; y<dstHeight; y++) {
            s=(uint16_t*)&srcImage[2*srcWidth*(y+startY)+startX*2];
            d=(uint16_t*)&dstImage[dstWidth*y*2];
            x=0;
            // Copie optimisée par blocs de 4 octets si alignement correct
            if((intptr_t)s&2||(intptr_t)d&2){for(;x<dstWidth;x++){*d++=*s++;}}
            else {
                s32=(uint32_t*)s; d32=(uint32_t*)d;
                for(;x<dstWidth-1;x+=2){*d32++=*s32++;}
                s=(uint16_t*)s32; d=(uint16_t*)d32;
                for(;x<dstWidth;x++){*d++=*s++;}
            }
        }
    }
}

/* Vérification que le modèle est bien compilé pour un capteur caméra */
#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "Invalid model for current sensor"
#endif

#include <Arduino.h>
#include <Wire.h>

/* Macros pour accéder aux registres GPIO du nRF52840 (Arduino Nano 33 BLE) */
#define digitalPinToBitMask(P) (1 << (digitalPinToPinName(P) % 32))
#define portInputRegister(P) ((P == 0) ? &NRF_P0->IN : &NRF_P1->IN)

/* ----------------------------------------------------------------
 * OV7675::begin()
 * Configure les broches GPIO de la caméra et initialise
 * le pilote OV767X en mode VGA
 * ---------------------------------------------------------------- */
int OV7675::begin(int resolution, int format, int fps)
{
    // Configurer les broches de synchronisation en entrée et XCLK en sortie
    pinMode(OV7670_VSYNC, INPUT); pinMode(OV7670_HREF, INPUT);
    pinMode(OV7670_PLK, INPUT);   pinMode(OV7670_XCLK, OUTPUT);
    // Récupérer les registres et masques GPIO pour une lecture rapide
    vsyncPort = portInputRegister(digitalPinToPort(OV7670_VSYNC));
    vsyncMask = digitalPinToBitMask(OV7670_VSYNC);
    hrefPort  = portInputRegister(digitalPinToPort(OV7670_HREF));
    hrefMask  = digitalPinToBitMask(OV7670_HREF);
    pclkPort  = portInputRegister(digitalPinToPort(OV7670_PLK));
    pclkMask  = digitalPinToBitMask(OV7670_PLK);
    bool ret = OV767X::begin(VGA, format, fps);
    width = OV767X::width(); height = OV767X::height();
    bytes_per_pixel = OV767X::bytesPerPixel();
    bytes_per_row = width * bytes_per_pixel;
    resize_height = 2;
    buf_mem = NULL; raw_buf = NULL; intrp_buf = NULL;
    return ret;
}

/* ----------------------------------------------------------------
 * OV7675::allocate_scratch_buffs()
 * Alloue les buffers temporaires nécessaires à la lecture de frame
 * ---------------------------------------------------------------- */
int OV7675::allocate_scratch_buffs()
{
    buf_rows = height / resize_row_sz * resize_height;
    buf_size = bytes_per_row * buf_rows;
    buf_mem = ei_malloc(buf_size);
    if(buf_mem == NULL) { ei_printf("ERR: Echec allocation\r\n"); return false; }
    raw_buf = (uint8_t *)DWORD_ALIGN_PTR((uintptr_t)buf_mem);
    return 0;
}

/* ----------------------------------------------------------------
 * OV7675::deallocate_scratch_buffs()
 * Libère les buffers temporaires après lecture de la frame
 * ---------------------------------------------------------------- */
int OV7675::deallocate_scratch_buffs()
{
    ei_free(buf_mem); buf_mem = NULL; return 0;
}

/* ----------------------------------------------------------------
 * OV7675::readFrame()
 * Lit une frame complète depuis la caméra en désactivant
 * les interruptions, puis la redimensionne par blocs
 * ---------------------------------------------------------------- */
void OV7675::readFrame(void* buffer)
{
    allocate_scratch_buffs();
    uint8_t* out = (uint8_t*)buffer;
    noInterrupts(); // Désactiver les interruptions pour une lecture précise
    // Attendre le début d'une nouvelle frame (signal VSYNC)
    while ((*vsyncPort & vsyncMask) == 0);
    while ((*vsyncPort & vsyncMask) != 0);
    int out_row = 0;
    // Lire et redimensionner la frame par blocs de lignes
    for (int raw_height = 0; raw_height < height; raw_height += buf_rows) {
        readBuf();
        resizeImage(width, buf_rows, raw_buf, resize_col_sz, resize_height, &(out[out_row]), 16);
        out_row += resize_col_sz * resize_height * bytes_per_pixel;
    }
    interrupts(); // Réactiver les interruptions
    deallocate_scratch_buffs();
}

/* ----------------------------------------------------------------
 * OV7675::readBuf()
 * Lit les pixels bruts depuis le bus de données de la caméra
 * en synchronisation avec les signaux HREF et PCLK
 * ---------------------------------------------------------------- */
void OV7675::readBuf()
{
    int offset = 0;
    uint32_t ulPin = 33;
    NRF_GPIO_Type *port = nrf_gpio_pin_port_decode(&ulPin);
    for (int i = 0; i < buf_rows; i++) {
        while ((*hrefPort & hrefMask) == 0); // Attendre le début d'une ligne
        for (int col = 0; col < bytes_per_row; col++) {
            while ((*pclkPort & pclkMask) != 0); // Attendre front descendant PCLK
            uint32_t in = port->IN;
            // Extraire et réorganiser les bits du bus de données (D0-D7)
            in >>= 2; in &= 0x3f03; in |= (in >> 6);
            raw_buf[offset++] = in;
            while ((*pclkPort & pclkMask) == 0); // Attendre front montant PCLK
        }
        while ((*hrefPort & hrefMask) != 0); // Attendre fin de ligne
    }
}