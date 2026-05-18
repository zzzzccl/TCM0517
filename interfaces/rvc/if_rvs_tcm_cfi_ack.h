#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_rvs_tcm_cfi_ack
{
    dbgtag_t debug_tag;

    uint8_t valid : 1;

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_rvs_tcm_cfi_ack";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (valid) = 0x" << std::hex << valid << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const if_rvs_tcm_cfi_ack& ack)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << ack.valid << " ";

        return os;
    }
};

};
