/*
 * Capteur Ultrasonique Grove - Seeed Studios
 * STM32L476RG Nucleo
 * Header file
 */

#ifndef ULTRASONIC_GROVE_H
#define ULTRASONIC_GROVE_H

#include "stm32l4xx_hal.h"
#include <stdbool.h>

/* Fonctions publiques */
void Ultrasonic_Init(void);
void Ultrasonic_Trigger(void);
uint32_t Ultrasonic_GetDistance(void);
bool Ultrasonic_IsMeasurementDone(void);
uint32_t Ultrasonic_Measure(uint32_t timeout_ms);

/* Fonctions utilitaires */
void HAL_Delay_us(uint32_t us);
void DWT_Init(void);

#endif /* ULTRASONIC_GROVE_H */
