#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_tc_tcm_b_rd
{
    dbgtag_t debug_tag;

    uint8_t  rvs_core_id : 3; // RVS core id.
    uint8_t  instr_id    : 4; // Instruction id.
    uint8_t  buf_inx     : 6; // Credit buffer start index.
    uint32_t addr        : 21; // TCM byte address, must not cross 128-byte boundary.
    uint8_t  burst_len   : 3; // Burst len, always 7.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_tc_tcm_b_rd";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (rvs_core_id) = 0x" << std::hex << rvs_core_id;
        outfile << " (instr_id) = 0x"    << std::hex << instr_id;
        outfile << " (buf_inx) = 0x"     << std::hex << buf_inx;
        outfile << " (addr) = 0x"        << std::hex << addr;
        outfile << " (burst_len) = 0x"   << std::hex << burst_len << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_tc_tcm_b_rd &req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.rvs_core_id << " ";
        os << req.instr_id << " ";
        os << req.buf_inx << " ";
        os << req.addr << " ";
        os << req.burst_len << " ";

        return os;
    }
};

};
