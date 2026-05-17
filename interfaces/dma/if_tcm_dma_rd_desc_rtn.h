#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_tcm_dma_rd_desc_rtn
{
    dbgtag_t debug_tag;

    uint32_t data[8];    // 256-bits tensor descriptor.
    uint8_t  blk_id : 2; // 256-bits data offset in 1024 bits cacheline.
    uint8_t  cl_id  : 3; // Cacheline id the returned data will be written to.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_tcm_dma_rd_desc_rtn";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        for(uint32_t u = 0; u < 8; u++)
        {
            outfile << " (data" << u << ") = 0x" << std::hex << data[u];
        }

        outfile << " (blk_id) = 0x" << std::hex << blk_id;
        outfile << " (cl_id) = 0x"  << std::hex << cl_id << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_tcm_dma_rd_desc_rtn &rtn)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        for(uint32_t u = 0; u < 8; u++)
        {
            os << rtn.data[u] << " ";
        }

        os << rtn.blk_id << " ";
        os << rtn.cl_id << " ";

        return os;
    }
};

};