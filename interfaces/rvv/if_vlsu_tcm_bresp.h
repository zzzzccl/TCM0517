#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_vlsu_tcm_bresp
{
    dbgtag_t debug_tag;

    uint8_t op   : 2; // 0: read reserved; 1: write; 2: vmem_fence.
    uint8_t resp : 2; // 0: OKAY; 1: EXOKAY; 2: SLVERR; 3: DECERR.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_vlsu_tcm_bresp";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (op) = 0x"   << std::hex << op;
        outfile << " (resp) = 0x" << std::hex << resp << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_vlsu_tcm_bresp &resp)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << resp.op << " ";
        os << resp.resp << " ";

        return os;
    }
};

};
