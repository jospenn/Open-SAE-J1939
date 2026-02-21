#ifndef __SAE_J1939DA_H__
#define __SAE_J1939DA_H__

#include "Structs.h"

void SAE_J1939DA_init_Request_Fuel_Economy(J1939 *j1939);
void SAE_J1939DA_Read_Response_Request_Fuel_Economy(J1939 *j1939, uint8_t SA, uint8_t motor_number, uint8_t data[]);

#endif // __SAE_J1939DA_H__