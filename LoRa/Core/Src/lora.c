/*
 * lora.c
 *
 *  Created on: Feb 19, 2026
 *      Author: G45p8
 */


#include "lora.h"
#include "lcd.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart4;
extern rgb_lcd lcd;

static uint8_t rx_char;
static char rx_line[200];
static uint16_t rx_index = 0;
static LoRa_Mode lora_mode_actuel;

/* ─────────────────────────────────────────
   Envoi brut AT
───────────────────────────────────────── */
void lora_envoiebrut(char *cmd)
{
    char buffer[255];
    snprintf(buffer, sizeof(buffer), "%s\r\n", cmd);
    HAL_UART_Transmit(&huart4, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    HAL_Delay(300); // laisser le module répondre
}

/* ─────────────────────────────────────────
   Init : même config RF sur les 2 modules
   mode = LORA_MODE_EMETTEUR ou RECEPTEUR
───────────────────────────────────────── */
void lora_init(LoRa_Mode mode)
{
    lora_mode_actuel = mode;

    HAL_Delay(2000); // attendre démarrage module

    lora_envoiebrut("AT");
    lora_envoiebrut("AT+MODE=TEST");

    // Config RF identique sur les 2 modules ← c'est ce qui les "lie"
    lora_envoiebrut("AT+TEST=RFCFG,868000000,SF7,125,12,15,14,ON,OFF,OFF");

    // Afficher le mode sur LCD
    clearlcd();
    lcd_position(&hi2c1, 0, 0);
    if (mode == LORA_MODE_EMETTEUR)
    {
        lcd_print(&hi2c1, "Mode: Emetteur");
        // l'emetteur n'écoute pas au démarrage
    }
    else
    {
        lcd_print(&hi2c1, "Mode: Recepteur");
        lora_startrecep(); // le récepteur écoute tout de suite
    }

    // Armer l'interruption UART dans tous les cas
    HAL_UART_Receive_IT(&huart4, &rx_char, 1);
}

/* ─────────────────────────────────────────
   Envoi d'un message
───────────────────────────────────────── */
void lora_envoie(char *msg)
{
    // Convertir le message en hex (format attendu par AT+TEST=TXLRPKT)
    char hex[128] = {0};
    for (int i = 0; i < strlen(msg) && i < 60; i++)
    {
        char octet[3];
        snprintf(octet, sizeof(octet), "%02X", (uint8_t)msg[i]);
        strcat(hex, octet);
    }

    char cmd[200];
    snprintf(cmd, sizeof(cmd), "AT+TEST=TXLRPKT,\"%s\"", hex);
    lora_envoiebrut(cmd);

    // Afficher confirmation sur LCD
    lcd_position(&hi2c1, 0, 1);
    lcd_print(&hi2c1, "Envoye!         ");
}

/* ─────────────────────────────────────────
   Passer en mode écoute
───────────────────────────────────────── */
void lora_startrecep(void)
{
    lora_envoiebrut("AT+TEST=RXLRPKT");
    lcd_position(&hi2c1, 0, 1);
    lcd_print(&hi2c1, "Ecoute...       ");
}

/* ─────────────────────────────────────────
   Traitement d'une ligne reçue
   Exemple de réponse module :
   +TEST: RX "48656C6C6F"
───────────────────────────────────────── */
static void hex_vers_texte(char *hex, char *out, int out_size)
{
    int len = strlen(hex);
    int j = 0;
    for (int i = 0; i + 1 < len && j < out_size - 1; i += 2)
    {
        char octet[3] = {hex[i], hex[i+1], '\0'};
        out[j++] = (char)strtol(octet, NULL, 16);
    }
    out[j] = '\0';
}


void lora_traiterecep(char *line)
{
    // Afficher TOUTE réponse du module pour déboguer
    clearlcd();
    lcd_position(&hi2c1, 0, 0);
    lcd_print(&hi2c1, "RX brut:");
    lcd_position(&hi2c1, 0, 1);
    char affichage[17] = {0};
    strncpy(affichage, line, 16);
    lcd_print(&hi2c1, affichage);
    HAL_Delay(1000);

    // La réponse ressemble à : +TEST: RX "48656C6C6F"
    //if (strstr(line, "+TEST: RX") == NULL) return;

    char *start = strchr(line, '"');
    char *end   = strrchr(line, '"');

    if (start && end && end > start)
    {
        char hex[128] = {0};
        int len = end - start - 1;

        if (len > 0 && len < (int)sizeof(hex))
        {
            strncpy(hex, start + 1, len);
            hex[len] = '\0';

            // Convertir hex en texte lisible
            char message[64] = {0};
            hex_vers_texte(hex, message, sizeof(message));

            // Affichage sur LCD
            clearlcd();

            // Ligne 0 : label
            lcd_position(&hi2c1, 0, 0);
            lcd_print(&hi2c1, "Recu:");

            // Ligne 1 : le message (16 chars max)
            lcd_position(&hi2c1, 0, 1);

            // Tronquer à 16 caractères si trop long
            char affichage[17] = {0};
            strncpy(affichage, message, 16);
            lcd_print(&hi2c1, affichage);

            HAL_Delay(2000);

            // Relancer l'écoute
            lora_startrecep();
        }
    }
}

/* ─────────────────────────────────────────
   À appeler depuis HAL_UART_RxCpltCallback
───────────────────────────────────────── */
void lora_uart_callback(void)
{
    if (rx_index < sizeof(rx_line) - 1)
    {
        rx_line[rx_index++] = rx_char;
    }

    if (rx_char == '\n')
    {
        rx_line[rx_index] = '\0';
        lora_traiterecep(rx_line);
        rx_index = 0;
    }

    HAL_UART_Receive_IT(&huart4, &rx_char, 1);
}
