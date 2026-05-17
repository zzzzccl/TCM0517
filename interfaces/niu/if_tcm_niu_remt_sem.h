#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_tcm_niu_remt_sem
{
    dbgtag_t debug_tag;

    uint64_t sem_addr : 48; // Remote semaphore address.
    uint8_t  sem_num  : 2;  // Number of semaphores for continuous operation.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_tcm_niu_remt_sem";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (sem_addr) = 0x" << std::hex << sem_addr;
        outfile << " (sem_num) = 0x"  << std::hex << sem_num << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_tcm_niu_remt_sem &req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.sem_addr << " ";
        os << req.sem_num << " ";

        return os;
    }
};

};
