#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_rvs_tcm_r
{
    dbgtag_t debug_tag;

    uint8_t  id : 7;    // bit3:0 OS ID, bit6:4 RV core id.
    uint32_t data[8];   // Read return data, 256 bits.
    uint8_t  resp : 2;  // 0: OKAY; 1: EXOKAY; 2: SLVERR; 3: DECERR.
    uint8_t  last : 1;  // 1: last burst.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_rvs_tcm_r";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (id) = 0x" << std::hex << id;
        for (uint32_t u = 0; u < 8; u++)
        {
            outfile << " (data" << u << ") = 0x" << std::hex << data[u];
        }
        outfile << " (resp) = 0x" << std::hex << resp;
        outfile << " (last) = 0x" << std::hex << last << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const if_rvs_tcm_r& resp)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << resp.id << " ";
        for (uint32_t u = 0; u < 8; u++)
        {
            os << resp.data[u] << " ";
        }
        os << resp.resp << " ";
        os << resp.last << " ";

        return os;
    }
};

};
