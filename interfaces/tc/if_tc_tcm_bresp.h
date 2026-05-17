#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_tc_tcm_bresp
{
    dbgtag_t debug_tag;

    uint8_t op          : 2; // 0: read; 1: write; 2: sem.post.
    uint8_t rvs_core_id : 3; // RVS core id.
    uint8_t instr_id    : 4; // Instruction id.
    uint8_t resp        : 2; // 0: OKAY; 1: EXOKAY; 2: SLVERR; 3: DECERR.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_tc_tcm_bresp";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (op) = 0x"          << std::hex << op;
        outfile << " (rvs_core_id) = 0x" << std::hex << rvs_core_id;
        outfile << " (instr_id) = 0x"    << std::hex << instr_id;
        outfile << " (resp) = 0x"        << std::hex << resp << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_tc_tcm_bresp &resp)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << resp.op << " ";
        os << resp.rvs_core_id << " ";
        os << resp.instr_id << " ";
        os << resp.resp << " ";

        return os;
    }
};

};
