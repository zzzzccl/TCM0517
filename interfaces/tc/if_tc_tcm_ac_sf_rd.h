#pragma once
#include "dbgtag.h"

namespace IF_GEN
{
struct if_tc_tcm_ac_sf_rd
{
    dbgtag_t debug_tag;

    uint8_t rvs_core_id : 3; // RVS core id.
    uint8_t instr_id    : 4; // Instruction id.
    uint8_t buf_inx     : 5; // Credit buffer start index.
    uint8_t matrix_src  : 2; // 0: A; 1: C; 2: SF_A; 3: SF_B.

    union src_info_union
    {
        struct matrix_a
        {
            uint32_t addr      : 21; // TCM byte address.
            uint8_t  burst_len : 2;  // Burst len, always 0~3.
            uint8_t  grp_mask  : 4;  // 1 bit maps to 32-byte address, only 4'hF supported.
        };
        matrix_a matrix_a_data;
        struct matrix_sa
        {
            uint32_t addr      : 21; // TCM byte address.
            uint8_t  burst_len : 2;  // Burst len, always 0.
            uint8_t  grp_mask  : 4;  // 1 bit maps to 32-byte address, supports 4'h1/2/3/4/8/C/F.
        };
        matrix_sa matrix_sa_data;
        struct matrix_c
        {
            uint32_t addr      : 21; // TCM byte address.
            uint8_t  burst_len : 2;  // Burst len, always 0~3.
            uint8_t  grp_mask  : 4;  // 1 bit maps to 32-byte address, only 4'hF supported.
        };
        matrix_c matrix_c_data;
        struct matrix_sb
        {
            uint32_t addr      : 21; // TCM byte address.
            uint8_t  burst_len : 2;  // Burst len, 1 or 3.
            uint8_t  grp_mask  : 4;  // 1 bit maps to 32-byte address, only 4'hF supported.
        };
        matrix_sb matrix_sb_data;
    };
    src_info_union src_info_union_data;

    void printInterface(std::ofstream& outfile, uint32_t portId) const
    {
        outfile << "if_tc_tcm_ac_sf_rd";
        if (portId != 0xFFFFFFFF)
        {
            outfile << " (portId) = 0x" << std::hex << portId;
        }

        outfile << " (rvs_core_id) = 0x" << std::hex << rvs_core_id;
        outfile << " (instr_id) = 0x"    << std::hex << instr_id;
        outfile << " (buf_inx) = 0x"     << std::hex << buf_inx;
        outfile << " (matrix_src) = 0x"  << std::hex << matrix_src;

        if(matrix_src == 0)
        {
            printMatrixField(outfile, src_info_union_data.matrix_a_data);
        }
        else if(matrix_src == 1)
        {
            printMatrixField(outfile, src_info_union_data.matrix_c_data);
        }
        else if(matrix_src == 2)
        {
            printMatrixField(outfile, src_info_union_data.matrix_sa_data);
        }
        else if(matrix_src == 3)
        {
            printMatrixField(outfile, src_info_union_data.matrix_sb_data);
        }

        outfile << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const if_tc_tcm_ac_sf_rd &req)
    {
        os.flags(std::ios::hex | std::ios::right | std::ios::uppercase);

        os << req.rvs_core_id << " ";
        os << req.instr_id << " ";
        os << req.buf_inx << " ";
        os << req.matrix_src << " ";

        if(req.matrix_src == 0)
        {
            streamMatrixField(os, req.src_info_union_data.matrix_a_data);
        }
        else if(req.matrix_src == 1)
        {
            streamMatrixField(os, req.src_info_union_data.matrix_c_data);
        }
        else if(req.matrix_src == 2)
        {
            streamMatrixField(os, req.src_info_union_data.matrix_sa_data);
        }
        else if(req.matrix_src == 3)
        {
            streamMatrixField(os, req.src_info_union_data.matrix_sb_data);
        }

        return os;
    }

private:
    template <typename MatrixField>
    static void printMatrixField(std::ofstream& outfile, const MatrixField& field)
    {
        outfile << " (addr) = 0x"      << std::hex << field.addr;
        outfile << " (burst_len) = 0x" << std::hex << field.burst_len;
        outfile << " (grp_mask) = 0x"  << std::hex << field.grp_mask;
    }

    template <typename MatrixField>
    static void streamMatrixField(std::ostream& os, const MatrixField& field)
    {
        os << field.addr << " ";
        os << field.burst_len << " ";
        os << field.grp_mask << " ";
    }
};

};
