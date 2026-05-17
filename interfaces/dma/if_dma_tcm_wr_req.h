#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct WriteField
{
    uint32_t addr : 21; // TCM address, 1024 bits aligned.
    uint32_t data[32];  // Data to be written into TCM, 1024 bits.
    uint32_t mask[4];   // Byte mask for 1024-bits data, 128 bits.
    uint8_t  instr_id : 4; // Instruction id received from RVS, maintained in DMA for outstanding response.
    uint8_t  instr_last : 1; // Indicate that this is the last transaction of a DMA instruction.
};

struct SemField
{
    uint32_t addr    : 21; // TCM address, 64 bits aligned.
    uint8_t  sem_num : 2;  // Number of semaphore to be updated.
};

union WriteSemUnion
{
    WriteField write;
    SemField   sem;
};

struct if_dma_tcm_wr_req
{
    dbgtag_t debug_tag;

    /**
     * 0: RVS write operation.
       1: CP write operation.
       2: Sem.post instruction.
    */
    uint8_t op         : 2; // 0: RVS write; 1: CP write; 2: Sem post
    uint8_t rvs_id     : 3; // Scalar Core id
    

    /**
     * union
     * {
     *      struct write
     *      {
     *          addr : 21
     *          data : 1024
     *          mask : 128
     *          instr_id : 4
     *          instr_last : 1
     *      }
     * 
     *      struct sem_post
     *      {
     *          addr    : 21
     *          sem_num : 2
     *      }
     * }
    */
    WriteSemUnion write_sem_union;

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_dma_tcm_wr_req";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (op) = 0x"         << std::hex << op;
        outfile << " (rvs_id) = 0x"     << std::hex << rvs_id;

        if(op == 0 || op == 1)
        {
            outfile << " (addr) = 0x" << std::hex << write_sem_union.write.addr;

            for(uint32_t u = 0; u < 32; u++)
            {
                outfile << " (data" << u << ") = 0x" << std::hex << write_sem_union.write.data[u];
            }

            for(uint32_t u = 0; u < 4; u++)
            {
                outfile << " (mask" << u << ") = 0x" << std::hex << write_sem_union.write.mask[u];
            }

            outfile << " (instr_id) = 0x"   << std::hex << write_sem_union.write.instr_id;
            outfile << " (instr_last) = 0x" << std::hex << write_sem_union.write.instr_last;

            outfile << std::endl;
        }
        else if(op == 2)
        {
            outfile << " (addr) = 0x"    << std::hex << write_sem_union.sem.addr;
            outfile << " (sem_num) = 0x" << std::hex << write_sem_union.sem.sem_num;
            outfile << std::endl;
        }

    }

    friend std::ostream &operator<<(std::ostream &os, const if_dma_tcm_wr_req &req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.op << " ";
        os << req.rvs_id << " ";

        if(req.op == 0 || req.op == 1)
        {
            os << req.write_sem_union.write.addr << " ";

            for(uint32_t u = 0; u < 32; u++)
            {
                os << req.write_sem_union.write.data[u] << " ";
            }

            for(uint32_t u = 0; u < 4; u++)
            {
                os << req.write_sem_union.write.mask[u] << " ";
            }

            os << req.write_sem_union.write.instr_id << " ";
            os << req.write_sem_union.write.instr_last << " ";
        }
        else if(req.op == 2)
        {
            os << req.write_sem_union.sem.addr << " ";
            os << req.write_sem_union.sem.sem_num << " ";
        }

        return os;
    }
};

};