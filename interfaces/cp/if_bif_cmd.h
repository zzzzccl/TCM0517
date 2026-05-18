#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_bif_cmd
{
    dbgtag_t debug_tag;

    uint8_t  operation    : 2;  // 0: Load; 1: Store.
    uint64_t address      : 43; // Read/write data address.
    uint8_t  burst_length : 3;  // Burst length. CP control stream uses 1~4, otherwise 1.
    uint8_t  tag_sb       : 3;  // 0~3: cmdparser request; 4~7: compute executor request.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_bif_cmd";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (operation) = 0x"    << std::hex << operation;
        outfile << " (address) = 0x"      << std::hex << address;
        outfile << " (burst_length) = 0x" << std::hex << burst_length;
        outfile << " (tag_sb) = 0x"       << std::hex << tag_sb << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const if_bif_cmd& req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.operation << " ";
        os << req.address << " ";
        os << req.burst_length << " ";
        os << req.tag_sb << " ";

        return os;
    }
};

};
