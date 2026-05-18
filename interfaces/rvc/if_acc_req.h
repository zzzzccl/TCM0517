#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_acc_req
{
    dbgtag_t debug_tag;

    uint32_t insn : 32;
    uint64_t rs1 : 64;
    uint64_t rs2 : 64;
    uint64_t rs3 : 64;
    uint8_t  trans_id : 4;
    uint8_t  rvs_id   : 3;

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_acc_req";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (insn) = 0x"     << std::hex << insn;
        outfile << " (rs1) = 0x"      << std::hex << rs1;
        outfile << " (rs2) = 0x"      << std::hex << rs2;
        outfile << " (rs3) = 0x"      << std::hex << rs3;
        outfile << " (trans_id) = 0x" << std::hex << trans_id;
        outfile << " (rvs_id) = 0x"   << std::hex << rvs_id << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const if_acc_req& req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.insn << " ";
        os << req.rs1 << " ";
        os << req.rs2 << " ";
        os << req.rs3 << " ";
        os << req.trans_id << " ";
        os << req.rvs_id << " ";

        return os;
    }
};

};
