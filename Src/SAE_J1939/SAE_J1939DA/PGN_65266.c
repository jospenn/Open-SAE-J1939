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
#include "esp_log.h"

void SAE_J1939DA_init_Request_Fuel_Economy(J1939 *j1939) 
{
	for (uint8_t i=0; i<MAX_PGN_65266_DEVICES; i++)
	{
		j1939->from_other_ecu_fuel_economy[i].from_ecu_address = 255;
	}
}


/*
 * Request Fuel Economy (Liquid) to all ECU
 * PGN: 65266
 */
ENUM_J1939_STATUS_CODES SAE_J1939DA_Send_Request_Fuel_Economy(J1939 *j1939, uint8_t DA, uint8_t valve_number) {
	return SAE_J1939_Send_Request(j1939, DA, PGN_AUXILIARY_VALVE_MEASURED_POSITION_0 + valve_number); /* valve_number can be 0 to 15 */
}

/*
 * Response the request Fuel Economy (Liquid) to all ECU
 * PGN: 65266
 */
ENUM_J1939_STATUS_CODES SAE_J1939DA_Response_Request_Fuel_Economy(J1939 *j1939, uint8_t valve_number) {
	uint32_t ID = (0x0CFF << 16) | ((0x20 + valve_number) << 8) | j1939->information_this_ECU.this_ECU_address;
	uint8_t data[8];
	data[0] = j1939->this_auxiliary_valve_measured_position[valve_number].measured_position_percent;
	data[1] = j1939->this_auxiliary_valve_measured_position[valve_number].measured_position_percent >> 8;
	data[2] = 0b11110000 | j1939->this_auxiliary_valve_measured_position[valve_number].valve_state;
	data[3] = j1939->this_auxiliary_valve_measured_position[valve_number].measured_position_micrometer;
	data[4] = j1939->this_auxiliary_valve_measured_position[valve_number].measured_position_micrometer >> 8;
	data[5] = data[6] = data[7] = 0xFF;								/* All reserved */
	return CAN_Send_Message(ID, data);
}

/*
 * Read a response request Fuel Economy (Liquid) from any ECU - Broadcast in other words
 * PGN: 65266
 */
void SAE_J1939DA_Read_Response_Request_Fuel_Economy(J1939 *j1939, uint8_t SA, uint8_t motor_number, uint8_t data[]) 
{
	// j1939->fuel_rate_lph = ((uint16_t)data[0] | ((uint16_t)data[1] << 8)) * 0.05f;
	// j1939->inst_fuel_economy = ((uint16_t)data[2] | ((uint16_t)data[3] << 8)) / 512.0f;
	// j1939->avg_fuel_economy  = ((uint16_t)data[4] | ((uint16_t)data[5] << 8)) / 512.0f;
	// j1939->throttle1_pos      = data[6] * 0.4f;
	// j1939->throttle2_pos      = data[7] * 0.4f;
	j1939->from_other_ecu_fuel_economy[motor_number].from_ecu_address = SA;
	j1939->from_other_ecu_fuel_economy[motor_number].engine_fuel_rate = (data[1] << 8) | data[0];
	j1939->from_other_ecu_fuel_economy[motor_number].engine_instantaneous_fuel_economy = (data[3] << 8) | data[2];
	j1939->from_other_ecu_fuel_economy[motor_number].engine_average_fuel_economy = (data[5] << 8) | data[4];
	j1939->from_other_ecu_fuel_economy[motor_number].engine_throttle_valve_1_Position_1 = data[6];
	j1939->from_other_ecu_fuel_economy[motor_number].engine_throttle_valve_2_Position = data[7];
	j1939->from_other_ecu_fuel_economy[motor_number].updateCounter++;
}

