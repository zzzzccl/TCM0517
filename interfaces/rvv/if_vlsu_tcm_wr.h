#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct WriteField
{
    uint8_t  last      : 1; // 1: last burst of instruction.
    uint8_t  burst_len : 3; // Burst len, 0~7.
    uint32_t addr      : 21; // TCM byte address.
    uint32_t strb[2];        // Byte mask, 64 bits.
    uint32_t data[16];       // Write data, 512 bits, must not cross 128-byte boundary.
};

struct VmemFenceField
{
    uint8_t  last        : 1; // 1: last burst of instruction.
    uint8_t  sem_post_en : 1; // 1: vmem_fence + sem.post; 0: vmem_fence only.
    uint32_t addr        : 21; // Semaphore address.
};

union WrInfoUnion
{
    WriteField    write;
    VmemFenceField vmem_fence;
};

struct if_vlsu_tcm_wr
{
    dbgtag_t debug_tag;

    uint8_t op : 2; // 0: read reserved; 1: write; 2: vmem_fence.

    WrInfoUnion wr_union;

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_vlsu_tcm_wr";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (op) = 0x" << std::hex << op;

        if(op == 1)
        {
            outfile << " (last) = 0x"      << std::hex << wr_union.write.last;
            outfile << " (burst_len) = 0x" << std::hex << wr_union.write.burst_len;
            outfile << " (addr) = 0x"      << std::hex << wr_union.write.addr;

            for(uint32_t u = 0; u < 2; u++)
            {
                outfile << " (strb" << u << ") = 0x" << std::hex << wr_union.write.strb[u];
            }

            for(uint32_t u = 0; u < 16; u++)
            {
                outfile << " (data" << u << ") = 0x" << std::hex << wr_union.write.data[u];
            }
        }
        else if(op == 2)
        {
            outfile << " (last) = 0x"        << std::hex << wr_union.vmem_fence.last;
            outfile << " (sem_post_en) = 0x" << std::hex << wr_union.vmem_fence.sem_post_en;
            outfile << " (addr) = 0x"        << std::hex << wr_union.vmem_fence.addr;
        }

        outfile << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_vlsu_tcm_wr &req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.op << " ";

        if(req.op == 1)
        {
            os << req.wr_union.write.last << " ";
            os << req.wr_union.write.burst_len << " ";
            os << req.wr_union.write.addr << " ";

            for(uint32_t u = 0; u < 2; u++)
            {
                os << req.wr_union.write.strb[u] << " ";
            }

            for(uint32_t u = 0; u < 16; u++)
            {
                os << req.wr_union.write.data[u] << " ";
            }
        }
        else if(req.op == 2)
        {
            os << req.wr_union.vmem_fence.last << " ";
            os << req.wr_union.vmem_fence.sem_post_en << " ";
            os << req.wr_union.vmem_fence.addr << " ";
        }

        return os;
    }
};

};
