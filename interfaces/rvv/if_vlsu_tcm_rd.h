#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_vlsu_tcm_rd
{
    dbgtag_t debug_tag;

    uint8_t  burst_len : 1; // Burst len, 0 or 1.
    uint32_t addr      : 21; // TCM byte address, must not cross 128-byte boundary.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_vlsu_tcm_rd";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (burst_len) = 0x" << std::hex << burst_len;
        outfile << " (addr) = 0x"      << std::hex << addr << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_vlsu_tcm_rd &req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.burst_len << " ";
        os << req.addr << " ";

        return os;
    }
};

};
