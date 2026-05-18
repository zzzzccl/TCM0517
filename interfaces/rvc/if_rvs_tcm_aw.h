#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_rvs_tcm_aw
{
    dbgtag_t debug_tag;

    uint8_t  burst : 2;  // Burst type. Fixed 1: INCR.
    uint32_t addr  : 32; // TCM byte address.
    uint8_t  size  : 3;  // Bytes per beat = 2^size. 2/3/5 for atomic32/atomic64/write.
    uint8_t  len   : 1;  // Burst len. Fixed 0.
    uint8_t  id    : 7;  // bit3:0 OS ID, bit6:4 RV core id.
    uint8_t  qos   : 4;  // Request priority. Fixed 0.
    uint8_t  atop  : 6;  // Atomic type.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_rvs_tcm_aw";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (burst) = 0x" << std::hex << burst;
        outfile << " (addr) = 0x"  << std::hex << addr;
        outfile << " (size) = 0x"  << std::hex << size;
        outfile << " (len) = 0x"   << std::hex << len;
        outfile << " (id) = 0x"    << std::hex << id;
        outfile << " (qos) = 0x"   << std::hex << qos;
        outfile << " (atop) = 0x"  << std::hex << atop << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const if_rvs_tcm_aw& req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.burst << " ";
        os << req.addr << " ";
        os << req.size << " ";
        os << req.len << " ";
        os << req.id << " ";
        os << req.qos << " ";
        os << req.atop << " ";

        return os;
    }
};

};
