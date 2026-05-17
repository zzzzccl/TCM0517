#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_dma_tcm_rd_data_req
{
    dbgtag_t debug_tag;

    uint32_t addr      : 21; // TCM address, 1024 bits aligned
    uint8_t  burst_len : 3;  // Number of 1024-bits data to be returned
    uint8_t  req_id    : 6;  // Read request ID

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_dma_tcm_rd_data_req";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (addr) = 0x"      << std::hex << addr;
        outfile << " (burst_len) = 0x" << std::hex << burst_len;
        outfile << " (req_id) = 0x"    << std::hex << req_id;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_dma_tcm_rd_data_req &req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.addr << " ";
        os << req.burst_len << " ";
        os << req.req_id << " ";

        return os;
    }
};

};