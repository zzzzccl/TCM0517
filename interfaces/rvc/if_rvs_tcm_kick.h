#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_rvs_tcm_kick
{
    dbgtag_t debug_tag;

    uint8_t rvs_active_mask : 8;

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_rvs_tcm_kick";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (rvs_active_mask) = 0x" << std::hex << rvs_active_mask << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const if_rvs_tcm_kick& req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.rvs_active_mask << " ";

        return os;
    }
};

};
