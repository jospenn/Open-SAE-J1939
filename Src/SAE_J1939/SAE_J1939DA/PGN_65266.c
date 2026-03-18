/*
 * Auxiliary_Valve_Measured_Position.c
 *
 *  Created on: 16 juli 2021
 *      Author: Daniel Mårtensson
 */

#include "SAE_J1939DA.h"

/* Layers */
#include "../../SAE_J1939/SAE_J1939-21_Transport_Layer/Transport_Layer.h"
#include "Structs.h"


void SAE_J1939DA_init_Request_Fuel_Economy(J1939 *j1939) 
{
	for (uint8_t i=0; i<MAX_PGN_65266_DEVICES; i++)
	{
		j1939->from_other_ecu_fuel_economy[i].from_ecu_address = INVALID_INDEX;
	}

	j1939->number_of_known_engines_fuel_economy = 0;
}


/*
 * Request Fuel Economy (Liquid) to all ECU
 * PGN: 65266
 */
ENUM_J1939_STATUS_CODES SAE_J1939DA_Send_Request_Fuel_Economy(J1939 *j1939, uint8_t DA, uint8_t valve_number) {
	return SAE_J1939_Send_Request(j1939, DA, PGN_FUEL_ECONOMY_65266); 
}

/*
 * Response the request Fuel Economy (Liquid) to all ECU
 * PGN: 65266
 */
ENUM_J1939_STATUS_CODES SAE_J1939DA_Response_Request_Fuel_Economy(J1939 *j1939, uint8_t valve_number) {
	return STATUS_SEND_OK;
}

static uint8_t J1939_GetEngineIndex(J1939 *j1939, uint8_t SA)
{
    /* ---- search existing ---- */
    for(uint8_t i = 0; i < j1939->number_of_known_engines_fuel_economy; i++)
    {
        if(j1939->from_other_ecu_fuel_economy[i].from_ecu_address == SA)
            return i;
    }

    /* ---- allocate new ---- */
    if(j1939->number_of_known_engines_fuel_economy >= MAX_PGN_65266_DEVICES)
	{
        return INVALID_INDEX;
	}

    uint8_t index = j1939->number_of_known_engines_fuel_economy++;
    return index;
}

/*
 * Read a response request Fuel Economy (Liquid) from any ECU - Broadcast in other words
 * PGN: 65266
 */
void SAE_J1939DA_Read_Response_Request_Fuel_Economy(J1939 *j1939, uint8_t SA, uint8_t data[]) 
{

	uint8_t index = J1939_GetEngineIndex(j1939, SA);
    
	if(index == INVALID_INDEX)
	{
        SAE_J1939_logger("PGN65266 table full");
		return;     /* table full */
	}

	j1939->from_other_ecu_fuel_economy[index].from_ecu_address = SA;
	j1939->from_other_ecu_fuel_economy[index].engine_fuel_rate = (data[1] << 8) | data[0];
	j1939->from_other_ecu_fuel_economy[index].engine_instantaneous_fuel_economy = (data[3] << 8) | data[2];
	j1939->from_other_ecu_fuel_economy[index].engine_average_fuel_economy = (data[5] << 8) | data[4];
	j1939->from_other_ecu_fuel_economy[index].engine_throttle_valve_1_Position_1 = data[6];
	j1939->from_other_ecu_fuel_economy[index].engine_throttle_valve_2_Position = data[7];
	j1939->from_other_ecu_fuel_economy[index].updateCounter++;
}

void SAE_J1939DA_Send_Periodic_Fuel_Economy(J1939 *j1939)
{
    uint8_t data[8];
	
	uint16_t raw = rand() % 64256;   // 0..64255
	float fuel_rate = raw * 0.05f;
    float inst_fe   = 4.5f;       // km/L
    float avg_fe    = 4.2f;       // km/L
    float throttle1 = 40.0f;      // %
    float throttle2 = 38.0f;      // %

    uint16_t fuel_rate_raw = (uint16_t)(fuel_rate / 0.05f);
    uint16_t inst_fe_raw   = (uint16_t)(inst_fe * 512.0f);
    uint16_t avg_fe_raw    = (uint16_t)(avg_fe * 512.0f);
    uint8_t  throttle1_raw = (uint8_t)(throttle1 / 0.4f);
    uint8_t  throttle2_raw = (uint8_t)(throttle2 / 0.4f);

    // Little endian packing
    data[0] = fuel_rate_raw & 0xFF;
    data[1] = fuel_rate_raw >> 8;

    data[2] = inst_fe_raw & 0xFF;
    data[3] = inst_fe_raw >> 8;

    data[4] = avg_fe_raw & 0xFF;
    data[5] = avg_fe_raw >> 8;

    data[6] = throttle1_raw;
    data[7] = throttle2_raw;

    SAE_J1939_Send_Periodic_Message(j1939, 6, 0xFEF2, data, 8);
}
