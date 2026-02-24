/*
 * Address_Claimed.c
 *
 *  Created on: 14 juli 2021
 *      Author: Daniel Mårtensson
 */

#include "Network_Management_Layer.h"

/* Layers */
#include "../SAE_J1939-21_Transport_Layer/Transport_Layer.h"
#include "../../Hardware/Hardware.h"

static int compareJ1939Name(struct Name *a, struct Name *b);


/*
 * Send request address claimed to other ECU. Every time we asking addresses from other ECU, then we clear our storage of other ECU
 * PGN: 0x00EE00 (60928)
 */
ENUM_J1939_STATUS_CODES SAE_J1939_Send_Request_Address_Claimed(J1939 *j1939, uint8_t DA) {
	/* Delete all addresses by setting them to broadcast address and set the counters to 0 */
	memset(j1939->other_ECU_address, 0xFF, 0xFF);
	// j1939->number_of_cannot_claim_address = 0;
	j1939->number_of_other_ECU = 0;
	return SAE_J1939_Send_Request(j1939, DA, PGN_ADDRESS_CLAIMED);
}

/*
 * Response the request address claimed about this ECU to all ECU - Broadcast. This function must be called at the ECU start up according to J1939 standard
 * PGN: 0x00EE00 (60928)
 */
ENUM_J1939_STATUS_CODES SAE_J1939_Response_Request_Address_Claimed(J1939 *j1939) {
	uint32_t ID = (0x18EEFF << 8) | j1939->information_this_ECU.this_ECU_address;
	uint8_t data[8];
	data[0] = j1939->information_this_ECU.this_name.identity_number;
	data[1] = j1939->information_this_ECU.this_name.identity_number >> 8;
	data[2] = (j1939->information_this_ECU.this_name.identity_number >> 16) |  (j1939->information_this_ECU.this_name.manufacturer_code << 5);
	data[3] = j1939->information_this_ECU.this_name.manufacturer_code >> 3;
	data[4] = (j1939->information_this_ECU.this_name.function_instance << 3) | j1939->information_this_ECU.this_name.ECU_instance;
	data[5] = j1939->information_this_ECU.this_name.function;
	data[6] = j1939->information_this_ECU.this_name.vehicle_system << 1;
	data[7] = (j1939->information_this_ECU.this_name.arbitrary_address_capable << 7) | (j1939->information_this_ECU.this_name.industry_group << 4) | j1939->information_this_ECU.this_name.vehicle_system_instance;
	return CAN_Send_Message(ID, data);
}

/*
 * Store the address claimed information about other ECU
 * PGN: 0x00EE00 (60928)
 */
void SAE_J1939_Read_Response_Request_Address_Claimed(J1939 *j1939, uint8_t SA, uint8_t data[]) 
{
	struct Name incomingName={0};
	
	/* store the temporary information */
	incomingName.identity_number = ((data[2] & 0b00011111) << 16) | (data[1] << 8) | data[0];
	incomingName.manufacturer_code = (data[3] << 3) | (data[2] >> 5);
	incomingName.function_instance = data[4] >> 3;
	incomingName.ECU_instance = data[4] & 0b00000111;
	incomingName.function = data[5];
	incomingName.vehicle_system = data[6] >> 1;
	incomingName.arbitrary_address_capable = data[7] >> 7;
	incomingName.industry_group = (data[7] >> 4) & 0b0111;
	incomingName.vehicle_system_instance = data[7] & 0b00001111;
	incomingName.from_ecu_address = SA;
	
	/* Check if it's the same address */
	if(j1939->information_this_ECU.this_ECU_address == SA)
	{
		int cmp = compareJ1939Name(&j1939->information_this_ECU.this_name, &incomingName);
    	if(cmp < 0) 
		{
			// my id < incoming, we win and (re)send our claim
	        SAE_J1939_Response_Request_Address_Claimed(j1939);
			return;
		}
		else
		{
			// my id > incoming, we loose and need to shift
			j1939->current_ECU_address_in_use_we_lost = true;
			return;
		}
	}

	/* If not, then store the temporary information */
	j1939->from_other_ecu_name = incomingName;

	/* Remember the source address of the ECU */
	bool exist = false;
	uint8_t i;
	for (i = 0; i < j1939->number_of_other_ECU; i++){
		if (j1939->other_ECU_address[i] == SA){
			exist = true;
		}
	}
	if (!exist){
		j1939->other_ECU_address[j1939->number_of_other_ECU++] = SA;	/* For every new ECU address, count how many ECU */
		SAE_J1939_logger("New device added [%u]", SA);
	}

}


static int compareJ1939Name(struct Name *a, struct Name *b)
{
    if(a->identity_number != b->identity_number)
        return (a->identity_number < b->identity_number) ? -1 : 1;

    if(a->manufacturer_code != b->manufacturer_code)
        return (a->manufacturer_code < b->manufacturer_code) ? -1 : 1;

    if(a->function != b->function)
        return (a->function < b->function) ? -1 : 1;

    if(a->function_instance != b->function_instance)
        return (a->function_instance < b->function_instance) ? -1 : 1;

    if(a->ECU_instance != b->ECU_instance)
        return (a->ECU_instance < b->ECU_instance) ? -1 : 1;

    if(a->vehicle_system != b->vehicle_system)
        return (a->vehicle_system < b->vehicle_system) ? -1 : 1;

    if(a->arbitrary_address_capable != b->arbitrary_address_capable)
        return (a->arbitrary_address_capable < b->arbitrary_address_capable) ? -1 : 1;

    if(a->industry_group != b->industry_group)
        return (a->industry_group < b->industry_group) ? -1 : 1;

    if(a->vehicle_system_instance != b->vehicle_system_instance)
        return (a->vehicle_system_instance < b->vehicle_system_instance) ? -1 : 1;

    return 0; // Names are identical
}
