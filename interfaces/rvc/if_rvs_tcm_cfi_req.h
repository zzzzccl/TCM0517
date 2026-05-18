#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_rvs_tcm_cfi_req
{
    dbgtag_t debug_tag;

    uint8_t valid : 1; // 1: cache flush.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_rvs_tcm_cfi_req";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (valid) = 0x" << std::hex << valid << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const if_rvs_tcm_cfi_req& req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.valid << " ";

        return os;
    }
};

};
