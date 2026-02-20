/*
 * lora.h
 *
 *  Created on: Feb 19, 2026
 *      Author: G45p8
 */

#ifndef LORA_H
#define LORA_H

#include "main.h"

typedef enum {
    LORA_MODE_EMETTEUR,
    LORA_MODE_RECEPTEUR
} LoRa_Mode;

void lora_init(LoRa_Mode mode);
void lora_envoiebrut(char *cmd);
void lora_envoie(char *msg);
void lora_startrecep(void);
void lora_traiterecep(char *line);
void lora_uart_callback(void);  // à appeler dans HAL_UART_RxCpltCallback

#endif
