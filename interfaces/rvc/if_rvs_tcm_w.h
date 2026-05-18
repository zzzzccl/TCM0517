#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_rvs_tcm_w
{
    dbgtag_t debug_tag;

    uint32_t data[8]; // Write data, 256 bits.
    uint32_t strb : 32;    // Byte mask, 32 bits. 1 bit maps to 1 byte.
    uint8_t  last : 1; // 1: last burst.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_rvs_tcm_w";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        for (uint32_t u = 0; u < 8; u++)
        {
            outfile << " (data" << u << ") = 0x" << std::hex << data[u];
        }

        outfile << " (strb) = 0x" << std::hex << strb;
        outfile << " (last) = 0x" << std::hex << last << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const if_rvs_tcm_w& req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        for (uint32_t u = 0; u < 8; u++)
        {
            os << req.data[u] << " ";
        }

        os << req.strb << " ";
        os << req.last << " ";

        return os;
    }
};

};
