#pragma once
#include "dbgtag.h"

namespace IF_GEN
{
struct if_tc_tcm_wr
{
    dbgtag_t debug_tag;

    uint8_t op          : 2; // 0: read; 1: write; 2: sem.post.
    uint8_t rvs_core_id : 3; // RVS core id.
    uint8_t instr_id    : 4; // Instruction id.

    union wr_info_union
    {
        struct write
        {
            uint32_t addr : 21;      // TCM byte address, must not cross 128-byte boundary.
            uint32_t data[32];       // 1024 bits data.
            uint8_t  instr_last : 1; // 1: last burst of instruction.
        };
        write write_data;
        struct sem_post
        {
            uint32_t addr : 21;      // Semaphore address.
            uint8_t  num  : 2;       // Number of semaphores for continuous operation.
            uint8_t  instr_last : 1; // 1: last burst of instruction.
        };
        sem_post sem_post_data;
    };

    wr_info_union wr_info_union_data;

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_tc_tcm_wr";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (op) = 0x"          << std::hex << op;
        outfile << " (rvs_core_id) = 0x" << std::hex << rvs_core_id;
        outfile << " (instr_id) = 0x"    << std::hex << instr_id;

        if(op == 1)
        {
            outfile << " (addr) = 0x" << std::hex << wr_info_union_data.write_data.addr;
            
            for(uint32_t u = 0; u < 32; u++)
            {
                outfile << " (data" << u << ") = 0x" << std::hex << wr_info_union_data.write_data.data[u];
            }

            outfile << " (instr_last) = 0x" << std::hex << wr_info_union_data.write_data.instr_last;
        }
        else if(op == 2)
        {
            outfile << " (addr) = 0x" << std::hex << wr_info_union_data.sem_post_data.addr;
            outfile << " (num) = 0x"  << std::hex << wr_info_union_data.sem_post_data.num;
            outfile << " (instr_last) = 0x" << std::hex << wr_info_union_data.sem_post_data.instr_last;
        }

        outfile << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_tc_tcm_wr &req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.op << " ";
        os << req.rvs_core_id << " ";
        os << req.instr_id << " ";

        if(req.op == 1)
        {
            os << req.wr_info_union_data.write_data.addr << " ";

            for(uint32_t u = 0; u < 32; u++)
            {
                os << req.wr_info_union_data.write_data.data[u] << " ";
            }

            os << req.wr_info_union_data.write_data.instr_last << " ";
        }
        else if(req.op == 2)
        {
            os << req.wr_info_union_data.sem_post_data.addr << " ";
            os << req.wr_info_union_data.sem_post_data.num << " ";
            os << req.wr_info_union_data.sem_post_data.instr_last << " ";
        }

        return os;
    }
};

};
