#pragma once
#include "dbgtag.h"

namespace IF_GEN
{

struct if_acc_resp
{
    dbgtag_t debug_tag;

    uint8_t wakeup_mode : 1; // 0: no wakeup; 1: wakeup.
    uint8_t trans_id    : 4; // Instruction id.
    uint8_t rvs_id      : 3; // RVS ID.

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_acc_resp";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (wakeup_mode) = 0x" << std::hex << wakeup_mode;
        outfile << " (trans_id) = 0x"    << std::hex << trans_id;
        outfile << " (rvs_id) = 0x"      << std::hex << rvs_id << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const if_acc_resp& resp)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << resp.wakeup_mode << " ";
        os << resp.trans_id << " ";
        os << resp.rvs_id << " ";

        return os;
    }
};

};
