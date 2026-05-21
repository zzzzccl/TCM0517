#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_bif_write
{
    dbgtag_t debug_tag;

    uint32_t mask : 32;    // Double-word mask. 1 bit maps to a 4-byte address.
    uint32_t data[8]; // Store data, 256 bits.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_bif_write";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (mask) = 0x" << std::hex << mask;
        for (uint32_t u = 0; u < 8; u++)
        {
            outfile << " (data" << u << ") = 0x" << std::hex << data[u];
        }
        outfile << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const if_bif_write& req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.mask << " ";
        for (uint32_t u = 0; u < 8; u++)
        {
            os << req.data[u] << " ";
        }

        return os;
    }
};

};
