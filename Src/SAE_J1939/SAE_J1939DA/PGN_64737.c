/*
 * Auxiliary_Valve_Measured_Position.c
 *
 *  Created on: 16 juli 2021
 *      Author: Daniel Mårtensson
 */

#include "SAE_J1939DA.h"

/* Layers */
#include "../../SAE_J1939/SAE_J1939-21_Transport_Layer/Transport_Layer.h"
#include "../../Hardware/Hardware.h"
#include "Structs.h"

#define INVALID_INDEX 0xFF

void SAE_J1939DA_init_Request_Fuel_Economy2(J1939 *j1939) 
{
	for (uint8_t i=0; i<MAX_PGN_64737_DEVICES; i++)
	{
		j1939->from_other_ecu_fuel_economy2[i].from_ecu_address = 255;
		j1939->from_other_ecu_fuel_economy2[i].engine_fuel_rate_high_resolution=0;
		j1939->from_other_ecu_fuel_economy2[i].engine_diesel_fuel_demand_rate=0;
	}
	j1939->number_of_known_engines_fuel_economy2 = 0;

}


/*
 * Request Fuel Economy 2 (Liquid) to all ECU
 * PGN: 64737
 */
ENUM_J1939_STATUS_CODES SAE_J1939DA_Send_Request_Fuel_Economy2(J1939 *j1939, uint8_t DA, uint8_t valve_number) {
	return SAE_J1939_Send_Request(j1939, DA, PGN_FUEL_ECONOMY_65266); 
}

/*
 * Response the request Fuel Economy 2 (Liquid) to all ECU
 * PGN: 64737
 */
ENUM_J1939_STATUS_CODES SAE_J1939DA_Response_Request_Fuel_Economy2(J1939 *j1939, uint8_t valve_number) {
	return STATUS_SEND_OK;
}

static uint8_t J1939_GetEngineIndex(J1939 *j1939, uint8_t SA)
{
    /* ---- search existing ---- */
    for(uint8_t i = 0; i < j1939->number_of_known_engines_fuel_economy2; i++)
    {
        if(j1939->from_other_ecu_fuel_economy2[i].from_ecu_address == SA)
            return i;
    }

    /* ---- allocate new ---- */
    if(j1939->number_of_known_engines_fuel_economy2 >= MAX_PGN_65266_DEVICES)
	{
        return INVALID_INDEX;
	}

    uint8_t index = j1939->number_of_known_engines_fuel_economy2++;
    return index;
}

/*
 * Read a response request Fuel Economy 2(Liquid) from any ECU - Broadcast in other words
 * PGN: 64737
 */
void SAE_J1939DA_Read_Response_Request_Fuel_Economy2(J1939 *j1939, uint8_t SA, uint8_t data[]) 
{
	uint8_t index = J1939_GetEngineIndex(j1939, SA);
    
	if(index == INVALID_INDEX)
	{
        SAE_J1939_logger("PGN65266 table full");
		return;     /* table full */
	}

	j1939->from_other_ecu_fuel_economy2[index].from_ecu_address = SA;
	j1939->from_other_ecu_fuel_economy2[index].engine_fuel_rate_high_resolution = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
	j1939->from_other_ecu_fuel_economy2[index].engine_diesel_fuel_demand_rate = (data[5] << 8) | data[4];
	j1939->from_other_ecu_fuel_economy2[index].updateCounter++;
}

void SAE_J1939DA_Send_Periodic_Fuel_Economy2_Liquid(J1939 *j1939)
{
    uint8_t data[8];

    /* -------- SPN 1600 ----------
       Engine Fuel Rate High Resolution
       0.001 l/h per bit
    */
    uint32_t fuel_rate_raw = rand() % 400000;   // 0..399.999 l/h

    /* -------- SPN 7596 ----------
       Diesel Fuel Demand Rate
       0.05 l/h per bit
    */
    uint16_t diesel_demand_raw = rand() % 6000;     // 0..300 l/h

    /* Little endian packing */
    data[0] = fuel_rate_raw & 0xFF;
    data[1] = (fuel_rate_raw >> 8) & 0xFF;
    data[2] = (fuel_rate_raw >> 16) & 0xFF;
    data[3] = (fuel_rate_raw >> 24) & 0xFF;

    data[4] = diesel_demand_raw & 0xFF;
    data[5] = diesel_demand_raw >> 8;

    /* Reserved */
    data[6] = 0xFF;
    data[7] = 0xFF;

    SAE_J1939_Send_Periodic_Message(
        j1939,
        6,
        0xFCE1,
        data,
        8
    );
}