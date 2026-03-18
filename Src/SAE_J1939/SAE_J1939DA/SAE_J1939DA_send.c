#include "SAE_J1939DA.h"

#include "../../Hardware/Hardware.h"

void SAE_J1939_Send_Periodic_Message(J1939 *j1939, uint8_t priority, uint32_t PGN, uint8_t *data, uint8_t dataSize)
{
    uint8_t l_SA = j1939->information_this_ECU.this_ECU_address;
    uint8_t l_PF = (PGN >> 8) & 0xFF;
    uint32_t ID;

    if (l_PF < 240)   // PDU1 (destination specific)
    {
        uint8_t DA = 0xFF; // broadcast or specific ECU
        ID = ((uint32_t)priority << 26) |
             ((uint32_t)l_PF << 16) |
             ((uint32_t)DA << 8) |
             l_SA;
    }
    else            // PDU2 (broadcast PGN)
    {
        ID = ((uint32_t)priority << 26) |
             ((uint32_t)PGN << 8) |
             l_SA;
    }

    CAN_Send_Message(ID, data);
}