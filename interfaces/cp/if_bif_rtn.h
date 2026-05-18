#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_bif_rtn
{
    dbgtag_t debug_tag;

    uint32_t data[8]; // Read return data, 256 bits.
    uint8_t  tag_sb : 3; // 0~1: cmdparser request; 2~3: compute executor request.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_bif_rtn";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        for (uint32_t u = 0; u < 8; u++)
        {
            outfile << " (data" << u << ") = 0x" << std::hex << data[u];
        }
        outfile << " (tag_sb) = 0x" << std::hex << tag_sb << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const if_bif_rtn& resp)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        for (uint32_t u = 0; u < 8; u++)
        {
            os << resp.data[u] << " ";
        }
        os << resp.tag_sb << " ";

        return os;
    }
};

};
