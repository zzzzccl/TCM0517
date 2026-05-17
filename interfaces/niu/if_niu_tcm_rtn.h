#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_niu_tcm_rtn
{
    dbgtag_t debug_tag;

    uint8_t  id : 8;     // Request id.
    uint32_t data[32];   // Read returned data, 1024 bits.
    uint8_t  resp : 2;   // 0: OKAY; 1: EXOKAY; 2: SLVERR; 3: DECERR.
    uint8_t  last : 1;   // 1: last burst.
    uint8_t  user : 6;   // User sideband.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_niu_tcm_rtn";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (id) = 0x" << std::hex << id;

        for(uint32_t u = 0; u < 32; u++)
        {
            outfile << " (data" << u << ") = 0x" << std::hex << data[u];
        }

        outfile << " (resp) = 0x" << std::hex << resp;
        outfile << " (last) = 0x" << std::hex << last;
        outfile << " (user) = 0x" << std::hex << user << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_niu_tcm_rtn &rtn)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << rtn.id << " ";

        for(uint32_t u = 0; u < 32; u++)
        {
            os << rtn.data[u] << " ";
        }

        os << rtn.resp << " ";
        os << rtn.last << " ";
        os << rtn.user << " ";

        return os;
    }
};

};
