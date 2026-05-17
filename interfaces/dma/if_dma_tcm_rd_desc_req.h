#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_dma_tcm_rd_desc_req
{
    dbgtag_t debug_tag;

    uint32_t addr      : 21; // TCM address, 1024 bits aligned
    uint8_t  cl_id     : 3;  // Cacheline id the returned data will be written to.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_dma_tcm_rd_desc_req";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (addr) = 0x"  << std::hex << addr;
        outfile << " (cl_id) = 0x" << std::hex << cl_id << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_dma_tcm_rd_desc_req &req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.addr << " ";
        os << req.cl_id << " ";

        return os;
    }
};

};