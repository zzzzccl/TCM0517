#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_tcm_dma_rd_data_rtn
{
    dbgtag_t debug_tag;

    uint32_t data[32];   // 1024 bits data returned from TCM
    uint8_t  blk_id : 3; // Transaction id in burst read request
    uint8_t  req_id : 6; // Read request ID

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_tcm_dma_rd_data_rtn";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        for(uint32_t u = 0; u < 32; u++)
        {
            outfile << " (data" << u << ") = 0x" << std::hex << data[u];
        }

        outfile << " (blk_id) = 0x" << std::hex << blk_id;
        outfile << " (req_id) = 0x" << std::hex << req_id;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_tcm_dma_rd_data_rtn &rtn)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        for(uint32_t u = 0; u < 32; u++)
        {
            os << rtn.data[u] << " ";
        }

        os << rtn.blk_id << " ";
        os << rtn.req_id << " ";

        return os;
    }
};

};