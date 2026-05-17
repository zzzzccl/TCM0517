#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_niu_tcm_wr
{
    dbgtag_t debug_tag;

    uint8_t  id    : 8;  // Request id.
    uint32_t addr  : 32; // TCM byte address.
    uint8_t  len   : 8;  // Burst len, 0~63. Transfer count is len + 1.
    uint8_t  size  : 3;  // Bytes per beat = 2^size. Fixed 2/3/7.
    uint8_t  burst : 2;  // 0: FIXED; 1: INCR; 2: WARP. Fixed 1.
    uint8_t  qos   : 4;  // Request priority. Higher value means higher priority.
    uint8_t  atop  : 6;  // Atomic type. Only AtomicLoad and AtomicSwap are supported.
    uint8_t  user  : 6;  // bit0: sem.post; bit2:1 sem_num; bit5:3 RVC source.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_niu_tcm_wr";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (id) = 0x"    << std::hex << id;
        outfile << " (addr) = 0x"  << std::hex << addr;
        outfile << " (len) = 0x"   << std::hex << len;
        outfile << " (size) = 0x"  << std::hex << size;
        outfile << " (burst) = 0x" << std::hex << burst;
        outfile << " (qos) = 0x"   << std::hex << qos;
        outfile << " (atop) = 0x"  << std::hex << atop;
        outfile << " (user) = 0x"  << std::hex << user << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_niu_tcm_wr &req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.id << " ";
        os << req.addr << " ";
        os << req.len << " ";
        os << req.size << " ";
        os << req.burst << " ";
        os << req.qos << " ";
        os << req.atop << " ";
        os << req.user << " ";

        return os;
    }
};

};
