#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_niu_tcm_wresp
{
    dbgtag_t debug_tag;

    uint8_t id   : 8; // Request id.
    uint8_t resp : 2; // 0: OKAY; 1: EXOKAY; 2: SLVERR; 3: DECERR.
    uint8_t user : 6; // User sideband.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_niu_tcm_wresp";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (id) = 0x"   << std::hex << id;
        outfile << " (resp) = 0x" << std::hex << resp;
        outfile << " (user) = 0x" << std::hex << user << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_niu_tcm_wresp &resp)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << resp.id << " ";
        os << resp.resp << " ";
        os << resp.user << " ";

        return os;
    }
};

};
