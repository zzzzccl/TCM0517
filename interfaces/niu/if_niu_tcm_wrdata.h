#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_niu_tcm_wrdata
{
    dbgtag_t debug_tag;

    uint32_t data[32];  // Write data, 1024 bits.
    uint32_t strb[4];   // Byte mask, 128 bits. 1 bit maps to 1 byte.
    uint8_t  last : 1;  // 1: last burst.
    uint8_t  user : 6;  // User sideband.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_niu_tcm_wrdata";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        for(uint32_t u = 0; u < 32; u++)
        {
            outfile << " (data" << u << ") = 0x" << std::hex << data[u];
        }

        for(uint32_t u = 0; u < 4; u++)
        {
            outfile << " (strb" << u << ") = 0x" << std::hex << strb[u];
        }

        outfile << " (last) = 0x" << std::hex << last;
        outfile << " (user) = 0x" << std::hex << user << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_niu_tcm_wrdata &req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        for(uint32_t u = 0; u < 32; u++)
        {
            os << req.data[u] << " ";
        }

        for(uint32_t u = 0; u < 4; u++)
        {
            os << req.strb[u] << " ";
        }

        os << req.last << " ";
        os << req.user << " ";

        return os;
    }
};

};
