#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_rvs_tcm_b
{
    dbgtag_t debug_tag;

    uint8_t id   : 7; // bit3:0 OS ID, bit6:4 RV core id.
    uint8_t resp : 2; // 0: OKAY; 1: EXOKAY; 2: SLVERR; 3: DECERR.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_rvs_tcm_b";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (id) = 0x" << std::hex << id;
        outfile << " (resp) = 0x" << std::hex << resp << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const if_rvs_tcm_b& resp)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << resp.id << " ";
        os << resp.resp << " ";

        return os;
    }
};

};
