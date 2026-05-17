#pragma once
#include <cstdint>
#include "../dma/dbgtag.h"

namespace IF_GEN
{

struct if_tc_tcm_ac_sf_rtn
{
    dbgtag_t debug_tag;

    uint8_t  rvs_core_id : 3; // RVS core id.
    uint8_t  instr_id    : 4; // Instruction id.
    uint8_t  buf_inx     : 5; // 1kb-level buffer index.
    uint8_t  matrix_src  : 2; // 0: A; 1: C; 2: SF_A; 3: SF_B.
    uint8_t  grp_mask    : 4; // 1 bit maps to 32-byte address.
    uint32_t data[32];        // Read returned data, 1024 bits.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_tc_tcm_ac_sf_rtn";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (rvs_core_id) = 0x" << std::hex << rvs_core_id;
        outfile << " (instr_id) = 0x"    << std::hex << instr_id;
        outfile << " (buf_inx) = 0x"     << std::hex << buf_inx;
        outfile << " (matrix_src) = 0x"  << std::hex << matrix_src;
        outfile << " (grp_mask) = 0x"    << std::hex << grp_mask;

        for(uint32_t u = 0; u < 32; u++)
        {
            outfile << " (data" << u << ") = 0x" << std::hex << data[u];
        }

        outfile << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_tc_tcm_ac_sf_rtn &rtn)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << rtn.rvs_core_id << " ";
        os << rtn.instr_id << " ";
        os << rtn.buf_inx << " ";
        os << rtn.matrix_src << " ";
        os << rtn.grp_mask << " ";

        for(uint32_t u = 0; u < 32; u++)
        {
            os << rtn.data[u] << " ";
        }

        return os;
    }
};

};
