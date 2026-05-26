#pragma once
#include "dbgtag.h"

namespace IF_GEN
{
struct if_dma_tcm_wr_req
{
    dbgtag_t debug_tag;

    /**
     * 0: RVS write operation.
       1: CP write operation.
       2: Sem.post instruction.
    */
    uint8_t op         : 2; // 0: RVS write; 1: CP write; 2: Sem post
       
    union wr_info_union
    {
        struct write
        {
            uint32_t addr : 21; // TCM address, 1024 bits aligned.
            uint32_t data[32];  // Data to be written into TCM, 1024 bits.
            uint32_t mask[4];   // Byte mask for 1024-bits data, 128 bits.
            uint8_t  instr_id : 4; // Instruction id received from RVS, maintained in DMA for outstanding response.
            uint8_t  instr_last : 1; // Indicate that this is the last transaction of a DMA instruction.
        };
        write write_data;
        struct sem_post
        {
            uint32_t addr    : 21; // TCM address, 64 bits aligned.
            uint8_t  sem_num : 2;  // Number of semaphore to be updated.
        };
        sem_post sem_post_data;
    };
    wr_info_union wr_info_union_data;

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_dma_tcm_wr_req";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (op) = 0x"         << std::hex << op;

        if(op == 0 || op == 1)
        {
            outfile << " (addr) = 0x" << std::hex << wr_info_union_data.write_data.addr;

            for(uint32_t u = 0; u < 32; u++)
            {
                outfile << " (data" << u << ") = 0x" << std::hex << wr_info_union_data.write_data.data[u];
            }

            for(uint32_t u = 0; u < 4; u++)
            {
                outfile << " (mask" << u << ") = 0x" << std::hex << wr_info_union_data.write_data.mask[u];
            }

            outfile << " (instr_id) = 0x"   << std::hex << wr_info_union_data.write_data.instr_id;
            outfile << " (instr_last) = 0x" << std::hex << wr_info_union_data.write_data.instr_last;

            outfile << std::endl;
        }
        else if(op == 2)
        {
            outfile << " (addr) = 0x"    << std::hex << wr_info_union_data.sem_post_data.addr;
            outfile << " (sem_num) = 0x" << std::hex << wr_info_union_data.sem_post_data.sem_num;
            outfile << std::endl;
        }

    }

    friend std::ostream &operator<<(std::ostream &os, const if_dma_tcm_wr_req &req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.op << " ";

        if(req.op == 0 || req.op == 1)
        {
            os << req.wr_info_union_data.write_data.addr << " ";

            for(uint32_t u = 0; u < 32; u++)
            {
                os << req.wr_info_union_data.write_data.data[u] << " ";
            }

            for(uint32_t u = 0; u < 4; u++)
            {
                os << req.wr_info_union_data.write_data.mask[u] << " ";
            }

            os << req.wr_info_union_data.write_data.instr_id << " ";
            os << req.wr_info_union_data.write_data.instr_last << " ";
        }
        else if(req.op == 2)
        {
            os << req.wr_info_union_data.sem_post_data.addr << " ";
            os << req.wr_info_union_data.sem_post_data.sem_num << " ";
        }

        return os;
    }
};

};