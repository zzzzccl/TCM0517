#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_tcm_dma_bresp
{
    dbgtag_t debug_tag;

    uint8_t instr_id  : 4; // Instruction id received from RVS, maintained in DMA for outstanding response.
    uint8_t rvs_id    : 3; // Scalar Core id
    uint8_t resp_type : 1; // 0: RVS load resp;  1: CP load resp

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_tcm_dma_bresp";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (instr_id) = 0x" << std::hex << instr_id;
        outfile << " (rvs_id) = 0x"   << std::hex << rvs_id;
        outfile << " (resp_type) = 0x" << std::hex << resp_type << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_tcm_dma_bresp &resp)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << resp.instr_id << " ";
        os << resp.rvs_id << " ";
        os << resp.resp_type << " ";

        return os;
    }
};

};