#ifndef __SAE_J1939DA_H__
#define __SAE_J1939DA_H__

#include "Structs.h"

#define INVALID_INDEX 0xFF

void SAE_J1939_Send_Periodic_Message(J1939 *j1939, uint8_t priority, uint32_t PGN, uint8_t *data, uint8_t dataSize);

void SAE_J1939DA_init_Request_Fuel_Economy(J1939 *j1939);
void SAE_J1939DA_Read_Response_Request_Fuel_Economy(J1939 *j1939, uint8_t SA, uint8_t data[]);
void SAE_J1939DA_Send_Periodic_Fuel_Economy(J1939 *j1939);

void SAE_J1939DA_init_Request_Fuel_Economy2(J1939 *j1939);
void SAE_J1939DA_Read_Response_Request_Fuel_Economy2(J1939 *j1939, uint8_t SA, uint8_t data[]);
void SAE_J1939DA_Send_Periodic_Fuel_Economy2_Liquid(J1939 *j1939);

#endif // __SAE_J1939DA_H__