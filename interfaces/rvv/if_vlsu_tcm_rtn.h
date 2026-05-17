#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_vlsu_tcm_rtn
{
    dbgtag_t debug_tag;

    uint8_t  last : 1; // 1: last burst of instruction.
    uint32_t data[16]; // Read returned data, 512 bits, must not cross 128-byte boundary.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_vlsu_tcm_rtn";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (last) = 0x" << std::hex << last;

        for(uint32_t u = 0; u < 16; u++)
        {
            outfile << " (data" << u << ") = 0x" << std::hex << data[u];
        }

        outfile << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_vlsu_tcm_rtn &rtn)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << rtn.last << " ";

        for(uint32_t u = 0; u < 16; u++)
        {
            os << rtn.data[u] << " ";
        }

        return os;
    }
};

};
