#pragma once

#include "interfaces/dma/if_dma_tcm_rd_data_req.h"
#include "interfaces/dma/if_tcm_dma_rd_data_rtn.h"
#include "interfaces/dma/if_dma_tcm_rd_desc_req.h"
#include "interfaces/dma/if_tcm_dma_rd_desc_rtn.h"
#include "interfaces/dma/if_dma_tcm_wr_req.h"
#include "interfaces/dma/if_tcm_dma_bresp.h"
#include "interfaces/tc/if_tc_tcm_wr.h"
#include "interfaces/tc/if_tc_tcm_bresp.h"
#include "interfaces/tc/if_tc_tcm_b_rd.h"
#include "interfaces/tc/if_tc_tcm_b_rtn.h"
#include "interfaces/tc/if_tc_tcm_ac_sf_rd.h"
#include "interfaces/tc/if_tc_tcm_ac_sf_rtn.h"
#include "interfaces/rvv/if_vlsu_tcm_wr.h"
#include "interfaces/rvv/if_vlsu_tcm_bresp.h"
#include "interfaces/rvv/if_vlsu_tcm_rd.h"
#include "interfaces/rvv/if_vlsu_tcm_rtn.h"
#include "interfaces/niu/if_niu_tcm_wr.h"
#include "interfaces/niu/if_niu_tcm_wrdata.h"
#include "interfaces/niu/if_niu_tcm_wresp.h"
#include "interfaces/niu/if_niu_tcm_rd.h"
#include "interfaces/niu/if_niu_tcm_rtn.h"
#include "interfaces/niu/if_tcm_niu_remt_sem.h"
#include "interfaces/rvc/if_rvs_tcm_aw.h"
#include "interfaces/rvc/if_rvs_tcm_w.h"
#include "interfaces/rvc/if_rvs_tcm_b.h"
#include "interfaces/rvc/if_rvs_tcm_ar.h"
#include "interfaces/rvc/if_rvs_tcm_r.h"
#include "interfaces/rvc/if_acc_req.h"
#include "interfaces/rvc/if_acc_resp.h"
#include "interfaces/rvc/if_rvs_tcm_kick.h"
#include "interfaces/rvc/if_rvs_tcm_cfi_req.h"
#include "interfaces/rvc/if_rvs_tcm_cfi_ack.h"
#include "interfaces/cp/if_bif_cmd.h"
#include "interfaces/cp/if_bif_write.h"
#include "interfaces/cp/if_bif_rtn.h"

#include "rgx_modules.h"
#include "sc_fifo_top.h"

#include <queue>
#include <bitset>
#include <mutex>
#include <cstring>

namespace pim_tcm
{
const uint32_t NUM_DMA_TCM_WR_IF = 2;
const uint32_t NUM_DMA_NOC_IF = 2;
const uint32_t RVV_NUM = 4;
const uint32_t RVC_NUM = 8;

const uint32_t BANK_ROW_NUM = 1024;
const uint32_t BANK_LANE_NUM = 8;
const uint32_t BANK_BANK_NUM = 4;
const uint32_t BANK_BYTE_NUM = 256;

// ============================================================================
// 指令类型
// ============================================================================
enum OpType {
    OP_READ      = 0,
    OP_WRITE     = 1,
    OP_ATOMIC    = 2,
    OP_CFI       = 3,
    OP_SEM_POST  = 4,
    OP_SEM_INIT  = 5,
    OP_SEM_WAIT  = 6,
    OP_VMEM_FENCE = 7,
    OP_BARRIER_INIT = 8,
    OP_BARRIER_PROC = 9
};

// ============================================================================
// master类型
// ============================================================================
enum MasterType {
    MASTER_DMA  = 0,
    MASTER_TC   = 1,
    MASTER_RVV0 = 2,
    MASTER_RVV1 = 3,
    MASTER_RVV2 = 4,
    MASTER_RVV3 = 5,
    MASTER_NIU  = 6,
    MASTER_RVC  = 7,
    MASTER_CP   = 8,
    MASTER_AS   = 9,
    MASTER_BANK = 10
};

// ============================================================================
// atomic类型
// ============================================================================
enum AtomicType {
    ATOM_NONE   = 0,
    ATOM_SWAP   = 0x30,
    ATOM_ADD    = 0x20,
    ATOM_CLR    = 0x21,
    ATOM_EOR    = 0x22,
    ATOM_SET    = 0x23,
    ATOM_SMAX   = 0x24,
    ATOM_SMIN   = 0x25,
    ATOM_UMAX   = 0x26,
    ATOM_UMIN   = 0x27
};

// ============================================================================
// Acc instruction类型
// ============================================================================
enum AccInstrType {
    BARRIER  = 0x8AB,
    SEM_INIT = 0x1AB,
    SEM_POST = 0x0AB,
    SEM_WAIT = 0x2AB,
    ACC_CFI  = 0xEAB
};

// ============================================================================
// Barrier Group类型
// ============================================================================
enum BarrierGroupType {
    PIM        = 0x0,
    SINGLE_NPU = 0x1,
    GROUP0     = 0x2,
    GROUP1     = 0x3
};

// ============================================================================
// 地址解析结构
// ============================================================================
struct TCMAddress {
    uint64_t raw;
    uint32_t byte_offset : 5;   // [4:0]   32 bytes in bank, 256 bits
    uint32_t bank_index  : 2;  // [6:5]   bank in lane
    uint32_t lane_index  : 3;  // [9:7]   lane index
    uint32_t row_index   : 10; // [19:10] row index
    uint64_t others;
    
    TCMAddress(uint64_t addr = 0) : raw(addr) {
        byte_offset = (addr >> 0)  & 0x1F;
        bank_index  = (addr >> 5)  & 0x3;
        lane_index  = (addr >> 7)  & 0x7;
        row_index   = (addr >> 10) & 0x3FF;
        others      = (addr >> 20);
    }
    
    uint32_t get_cache_tag() const {
        return (raw >> 7) & 0x1FFF;
    }
    
    void update_raw_addr() {
        raw =  (uint64_t(byte_offset) << 0)
             + (uint64_t(bank_index)  << 5)
             + (uint64_t(lane_index)  << 7)
             + (uint64_t(row_index)   << 10)
             + (uint64_t(others)      << 20);
    }

    uint32_t get_addr_in_lane() const {
        return byte_offset + (bank_index << 5);
    }
};

// ============================================================================
// Sideband 结构
// ============================================================================
struct Sideband {
    OpType op_type;
    MasterType master_type;
    dbgtag_t debug_tag;
    //wr_rd
    TCMAddress addr;
    uint32_t data_len; // bits，cacheline use
    uint32_t burst_len;
    //barrier
    uint8_t rvs_active_mask;
    BarrierGroupType barrier_group;
    //sem
    uint8_t sem_num;
    uint8_t expect_value;
    uint8_t wait_core_id;
    //atomic
    AtomicType atomic_type;
    //cacheline
    uint32_t way_idx;
    //others
    uint8_t instr_last;
    uint8_t instr_id;
    uint8_t rvs_id;
    uint8_t resp_type;   // 0=RVS load resp, 1=CP load resp 对应op字段
    uint8_t blk_id;
    uint8_t req_id;
    uint8_t cl_id;

    uint8_t buf_inx;
    uint8_t matrix_src;
    uint8_t grp_mask;
    uint8_t op;

    uint8_t id;
    uint8_t user;

    uint8_t trans_id;

    uint8_t tag_sb; 
};

// ============================================================================
// Payload 结构
// ============================================================================
struct Payload {
    uint32_t  data[32]; // max 1024 bits = 128 bytes
    uint32_t  mask[32]; // byte mask
};

// ============================================================================
// 请求结构
// ============================================================================
struct Request {
    Sideband sideband;
    Payload  payload;
    uint8_t  mask[8];
};

inline uint32_t GenByteMask(uint32_t mask4);

void ExpandMask4To32(const uint32_t mask4_arr[4], uint32_t expanded_mask[32]);

class DMA_FE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(DMA_FE);

private:
    bool isSem[2] = {};
    // interface between DMA_FE and DMA
    rgx_fifo_interface<IF_GEN::if_dma_tcm_wr_req>  m_dma_tcm_wr0_req_if;
    rgx_fifo_interface<IF_GEN::if_dma_tcm_wr_req>  m_dma_tcm_wr1_req_if;
    rgx_fifo_interface<IF_GEN::if_dma_tcm_rd_data_req>  m_dma_tcm_rd_data_req_if;
    rgx_fifo_interface<IF_GEN::if_dma_tcm_rd_desc_req>  m_dma_tcm_rd_desc_req_if;

    // communication fifo among DMA_FE, Bank and AS_pipe
    sc_fifo_out<Request>  m_dma_wr0_out;
    sc_fifo_out<Request>  m_dma_wr1_out;
    sc_fifo_out<Request>  m_dma_rd_data_out;
    sc_fifo_out<Request>  m_dma_rd_desc_out;
    sc_fifo_out<Request>  m_dma_sem_out;

    void ProcessDMAWr(
        uint32_t src, 
        rgx_fifo_interface<IF_GEN::if_dma_tcm_wr_req>& wr_in_if, 
        sc_fifo_out<Request>& wr_out_if,
        sc_fifo_out<Request>& sem_out_if
    );
    void ProcessDMAWr0(void);
    void ProcessDMAWr1(void);
    void ProcessDMARdData(void);
    void ProcessDMARdDesc(void);

public:
    DMA_FE(sc_module_name module_name)
    : sc_module(module_name)
    , m_dma_tcm_wr0_req_if("dma_tcm_wr0_req_if")
    , m_dma_tcm_wr1_req_if("dma_tcm_wr1_req_if")
    , m_dma_tcm_rd_data_req_if("dma_tcm_rd_data_req_if")
    , m_dma_tcm_rd_desc_req_if("dma_tcm_rd_desc_req_if")
    , m_dma_wr0_out("dma_wr0_out")
    , m_dma_wr1_out("dma_wr1_out")
    , m_dma_rd_data_out("dma_rd_data_out")
    , m_dma_rd_desc_out("dma_rd_desc_out")
    , m_dma_sem_out("dma_sem_out")
    {
        SC_THREAD(ProcessDMAWr0);
        sensitive << m_dma_tcm_wr0_req_if.data_written_event();

        SC_THREAD(ProcessDMAWr1);
        sensitive << m_dma_tcm_wr1_req_if.data_written_event();

        SC_THREAD(ProcessDMARdData);
        sensitive << m_dma_tcm_rd_data_req_if.data_written_event();

        SC_THREAD(ProcessDMARdDesc);
        sensitive << m_dma_tcm_rd_desc_req_if.data_written_event();
    }
};

class TC_FE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(TC_FE);

private:
    // interface between TC_FE and TC
    rgx_fifo_interface<IF_GEN::if_tc_tcm_wr>  m_tc_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_tc_tcm_b_rd>  m_tc_tcm_b_rd_if;
    rgx_fifo_interface<IF_GEN::if_tc_tcm_ac_sf_rd>  m_tc_tcm_ac_sf_rd_if;

    // communication fifo among TC_FE, Bank and AS_pipe
    sc_fifo_out<Request>  m_tc_wr_out;
    sc_fifo_out<Request>  m_tc_rd0_out;
    sc_fifo_out<Request>  m_tc_rd1_out;
    sc_fifo_out<Request>  m_tc_sem_out;

    void ProcessTCWr(void);
    void ProcessTCRd0(void);
    void ProcessTCRd1(void);

public:
    TC_FE(sc_module_name module_name)
    : sc_module(module_name)
    , m_tc_tcm_wr_if("tc_tcm_wr_if")
    , m_tc_tcm_b_rd_if("tc_tcm_b_rd_if")
    , m_tc_tcm_ac_sf_rd_if("tc_tcm_ac_sf_rd_if")
    , m_tc_wr_out("tc_wr_out")
    , m_tc_rd0_out("tc_rd0_out")
    , m_tc_rd1_out("tc_rd1_out")
    , m_tc_sem_out("tc_sem_out")
    {
        SC_THREAD(ProcessTCWr);
        sensitive << m_tc_tcm_wr_if.data_written_event();

        SC_THREAD(ProcessTCRd0);
        sensitive << m_tc_tcm_b_rd_if.data_written_event();

        SC_THREAD(ProcessTCRd1);
        sensitive << m_tc_tcm_ac_sf_rd_if.data_written_event();
    }
};

class RVV_FE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(RVV_FE);

private:
    struct BurstBuf {
        bool valid = false;
        uint32_t expect_len = 0;
        uint32_t recv_len = 0;
        Request  req = {};
        uint32_t mask[4] = {}; // byte mask
    };

    BurstBuf m_wr_burst_buf[RVV_NUM];

    // interface between RVV_FE and RVV
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_wr>  m_vlsu0_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_wr>  m_vlsu1_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_wr>  m_vlsu2_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_wr>  m_vlsu3_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_rd>  m_vlsu0_tcm_rd_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_rd>  m_vlsu1_tcm_rd_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_rd>  m_vlsu2_tcm_rd_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_rd>  m_vlsu3_tcm_rd_if;

    // communication fifo among RVV_FE, Bank and AS_pipe
    sc_fifo_out<Request>  m_vlsu0_wr_out;
    sc_fifo_out<Request>  m_vlsu1_wr_out;
    sc_fifo_out<Request>  m_vlsu2_wr_out;
    sc_fifo_out<Request>  m_vlsu3_wr_out;
    sc_fifo_out<Request>  m_vlsu0_rd_out;
    sc_fifo_out<Request>  m_vlsu1_rd_out;
    sc_fifo_out<Request>  m_vlsu2_rd_out;
    sc_fifo_out<Request>  m_vlsu3_rd_out;
    sc_fifo_out<Request>  m_vlsu0_sem_out;
    sc_fifo_out<Request>  m_vlsu1_sem_out;
    sc_fifo_out<Request>  m_vlsu2_sem_out;
    sc_fifo_out<Request>  m_vlsu3_sem_out;

    void ProcessVLSUWr(
        uint32_t src, 
        rgx_fifo_interface<IF_GEN::if_vlsu_tcm_wr>& wr_in_if, 
        sc_fifo_out<Request>& wr_out_if,
        sc_fifo_out<Request>& sem_out_if
    );
    void ProcessVLSU0Wr(void);
    void ProcessVLSU1Wr(void);
    void ProcessVLSU2Wr(void);
    void ProcessVLSU3Wr(void);

    void ProcessVLSURd(
        uint32_t src, 
        rgx_fifo_interface<IF_GEN::if_vlsu_tcm_rd>& rd_in_if, 
        sc_fifo_out<Request>& rd_out_if
    );
    void ProcessVLSU0Rd(void);
    void ProcessVLSU1Rd(void);
    void ProcessVLSU2Rd(void);
    void ProcessVLSU3Rd(void);

public:
    RVV_FE(sc_module_name module_name)
    : sc_module(module_name)
    , m_vlsu0_tcm_wr_if("vlsu0_tcm_wr_if")
    , m_vlsu1_tcm_wr_if("vlsu1_tcm_wr_if")
    , m_vlsu2_tcm_wr_if("vlsu2_tcm_wr_if")
    , m_vlsu3_tcm_wr_if("vlsu3_tcm_wr_if")
    , m_vlsu0_tcm_rd_if("vlsu0_tcm_rd_if")
    , m_vlsu1_tcm_rd_if("vlsu1_tcm_rd_if")
    , m_vlsu2_tcm_rd_if("vlsu2_tcm_rd_if")
    , m_vlsu3_tcm_rd_if("vlsu3_tcm_rd_if")
    , m_vlsu0_wr_out("vlsu0_wr_out")
    , m_vlsu1_wr_out("vlsu1_wr_out")
    , m_vlsu2_wr_out("vlsu2_wr_out")
    , m_vlsu3_wr_out("vlsu3_wr_out")
    , m_vlsu0_rd_out("vlsu0_rd_out")
    , m_vlsu1_rd_out("vlsu1_rd_out")
    , m_vlsu2_rd_out("vlsu2_rd_out")
    , m_vlsu3_rd_out("vlsu3_rd_out")
    , m_vlsu0_sem_out("vlsu0_sem_out")
    , m_vlsu1_sem_out("vlsu1_sem_out")
    , m_vlsu2_sem_out("vlsu2_sem_out")
    , m_vlsu3_sem_out("vlsu3_sem_out")
    {
        SC_THREAD(ProcessVLSU0Wr);
        sensitive << m_vlsu0_tcm_wr_if.data_written_event();

        SC_THREAD(ProcessVLSU1Wr);
        sensitive << m_vlsu1_tcm_wr_if.data_written_event();

        SC_THREAD(ProcessVLSU2Wr);
        sensitive << m_vlsu2_tcm_wr_if.data_written_event();

        SC_THREAD(ProcessVLSU3Wr);
        sensitive << m_vlsu3_tcm_wr_if.data_written_event();

        SC_THREAD(ProcessVLSU0Rd);
        sensitive << m_vlsu0_tcm_rd_if.data_written_event();

        SC_THREAD(ProcessVLSU1Rd);
        sensitive << m_vlsu1_tcm_rd_if.data_written_event();

        SC_THREAD(ProcessVLSU2Rd);
        sensitive << m_vlsu2_tcm_rd_if.data_written_event();

        SC_THREAD(ProcessVLSU3Rd);
        sensitive << m_vlsu3_tcm_rd_if.data_written_event();
    }
};

class NIU_FE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(NIU_FE);

private:
    // interface between NIU_FE and NIU
    rgx_fifo_interface<IF_GEN::if_niu_tcm_wr>     m_niu_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_niu_tcm_wrdata> m_niu_tcm_wrdata_if;
    rgx_fifo_interface<IF_GEN::if_niu_tcm_rd>     m_niu_tcm_rd_if;

    // communication fifo among NIU_FE, Bank and AS_pipe
    sc_fifo<Request> m_wr_fifo;
    sc_fifo<Request> m_rd_fifo;
    sc_fifo_out<Request>  m_niu_wr_rd_out;
    sc_fifo_out<Request>  m_niu_rvc_wr_rd_out;
    

    void ProcessNIUWr(void);
    void ProcessNIURd(void);
    void ProcessNIURoundRobin(void);

public:
    NIU_FE(sc_module_name module_name)
    : sc_module(module_name)
    , m_niu_tcm_wr_if("niu_tcm_wr_if")
    , m_niu_tcm_wrdata_if("niu_tcm_wrdata_if")
    , m_niu_tcm_rd_if("niu_tcm_rd_if")
    , m_wr_fifo("wr_fifo", 1)
    , m_rd_fifo("rd_fifo", 1)
    , m_niu_wr_rd_out("niu_wr_rd_out")
    , m_niu_rvc_wr_rd_out("niu_rvc_wr_rd_out")
    {
        SC_THREAD(ProcessNIUWr);
        sensitive << m_niu_tcm_wr_if.data_written_event()
                  << m_niu_tcm_wrdata_if.data_written_event();
        SC_THREAD(ProcessNIURd);
        sensitive << m_niu_tcm_rd_if.data_written_event();
        SC_THREAD(ProcessNIURoundRobin); 
        sensitive << m_wr_fifo.data_written_event()
                  << m_rd_fifo.data_written_event();
    }
};

class RVC_FE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(RVC_FE);

private:
    // interface between RVC_FE and RVC
    rgx_fifo_interface<IF_GEN::if_rvs_tcm_aw>      m_rvs_tcm_aw_if;
    rgx_fifo_interface<IF_GEN::if_rvs_tcm_w>       m_rvs_tcm_w_if;
    rgx_fifo_interface<IF_GEN::if_rvs_tcm_ar>      m_rvs_tcm_ar_if;
    rgx_fifo_interface<IF_GEN::if_rvs_tcm_kick>    m_rvs_tcm_kick_if;
    rgx_fifo_interface<IF_GEN::if_acc_req>         m_acc_req_if;
    rgx_fifo_interface<IF_GEN::if_rvs_tcm_cfi_req> m_rvs_tcm_cfi_req_if;

    // communication fifo between RVC_FE and AS_pipe
    sc_fifo<Request> m_wr_fifo;
    sc_fifo<Request> m_rd_fifo;
    sc_fifo_out<Request>  m_rvc_wr_rd_out;
    sc_fifo_out<Request>  m_rvc_acc_out;
    sc_fifo_out<Request>  m_rvc_barrier_cfi_out;
    
    void ProcessRVCWr(void);
    void ProcessRVCRd(void);
    void ProcessRVCAcc(void);
    void ProcessRVCKick(void);
    void ProcessRVCCfi(void);
    void ProcessRVCRoundRobin(void);

public:
    RVC_FE(sc_module_name module_name)
    : sc_module(module_name)
    , m_rvs_tcm_aw_if("rvs_tcm_aw_if")
    , m_rvs_tcm_w_if("rvs_tcm_w_if")
    , m_rvs_tcm_ar_if("rvs_tcm_ar_if")
    , m_rvs_tcm_kick_if("rvs_tcm_kick_if")
    , m_acc_req_if("acc_req_if")
    , m_rvs_tcm_cfi_req_if("rvs_tcm_cfi_req_if")
    , m_wr_fifo("wr_fifo", 1)
    , m_rd_fifo("rd_fifo", 1)
    , m_rvc_wr_rd_out("rvc_wr_rd_out")
    , m_rvc_acc_out("rvc_acc_out")
    , m_rvc_barrier_cfi_out("rvc_barrier_cfi_out")
    {
        SC_THREAD(ProcessRVCWr);
        sensitive << m_rvs_tcm_aw_if.data_written_event()
                  << m_rvs_tcm_w_if.data_written_event();
        SC_THREAD(ProcessRVCRd);
        sensitive << m_rvs_tcm_ar_if.data_written_event();
        SC_THREAD(ProcessRVCAcc);
        sensitive << m_acc_req_if.data_written_event();
        SC_THREAD(ProcessRVCKick);
        sensitive << m_rvs_tcm_kick_if.data_written_event();
        SC_THREAD(ProcessRVCCfi);
        sensitive << m_rvs_tcm_cfi_req_if.data_written_event();
        SC_THREAD(ProcessRVCRoundRobin); 
        sensitive << m_wr_fifo.data_written_event()
                  << m_rd_fifo.data_written_event();
    }
};

class CP_FE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(CP_FE);

private:
    // interface between CP_FE and CP
    rgx_fifo_interface<IF_GEN::if_bif_cmd>   m_bif_cmd_if;
    rgx_fifo_interface<IF_GEN::if_bif_write> m_bif_write_if;

    // communication fifo between CP_FE and AS_pipe
    sc_fifo_out<Request>  m_cp_wr_out;
    
    void ProcessCPWrRd(void);

public:
    CP_FE(sc_module_name module_name)
    : sc_module(module_name)
    , m_bif_cmd_if("bif_cmd_if")
    , m_bif_write_if("bif_write_if")
    , m_cp_wr_out("cp_wr_out")
    {
        SC_THREAD(ProcessCPWrRd);
        sensitive << m_bif_cmd_if.data_written_event()
                  << m_bif_write_if.data_written_event();
    }
};

class DMA_BE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(DMA_BE);

private:
    // communication fifo between DMA_BE and Bank
    sc_fifo_in<Request>  m_dma_wr0_in;
    sc_fifo_in<Request>  m_dma_wr1_in;
    sc_fifo_in<Request>  m_dma_rd_data_in;
    sc_fifo_in<Request>  m_dma_rd_desc_in;

    // interface between DMA_BE and DMA
    rgx_port<IF_GEN::if_tcm_dma_bresp>           m_tcm_dma_bresp0_if;
    rgx_port<IF_GEN::if_tcm_dma_bresp>           m_tcm_dma_bresp1_if;
    rgx_port<IF_GEN::if_tcm_dma_rd_data_rtn>           m_tcm_dma_rd_data_rtn_if;
    rgx_port<IF_GEN::if_tcm_dma_rd_desc_rtn>           m_tcm_dma_rd_desc_rtn_if;

    void ProcessDMAWr0(void);
    void ProcessDMAWr1(void);
    void ProcessDMARdData(void);
    void ProcessDMARdDesc(void);

public:
    DMA_BE(sc_module_name module_name)
    : sc_module(module_name)
    , m_dma_wr0_in("dma_wr0_in")
    , m_dma_wr1_in("dma_wr1_in")
    , m_dma_rd_data_in("dma_rd_data_in")
    , m_dma_rd_desc_in("dma_rd_desc_in")
    , m_tcm_dma_bresp0_if("tcm_dma_bresp0_if")
    , m_tcm_dma_bresp1_if("tcm_dma_bresp1_if")
    , m_tcm_dma_rd_data_rtn_if("tcm_dma_rd_data_rtn_if")
    , m_tcm_dma_rd_desc_rtn_if("tcm_dma_rd_desc_rtn_if")
    {
        SC_THREAD(ProcessDMAWr0);
        sensitive << m_dma_wr0_in.data_written_event();

        SC_THREAD(ProcessDMAWr1);
        sensitive << m_dma_wr1_in.data_written_event();

        SC_THREAD(ProcessDMARdData);
        sensitive << m_dma_rd_data_in.data_written_event();

        SC_THREAD(ProcessDMARdDesc);
        sensitive << m_dma_rd_desc_in.data_written_event();
    }
};

class TC_BE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(TC_BE);

private:
    // communication fifo between TC_BE and bank
    sc_fifo_in<Request>  m_tc_wr_in;
    sc_fifo_in<Request>  m_tc_b_rd0_in;
    sc_fifo_in<Request>  m_tc_b_rd1_in;
    sc_fifo_in<Request>  m_tc_mix_rd_in;

    // interface between TC_BE and TC
    rgx_port<IF_GEN::if_tc_tcm_bresp>           m_tc_tcm_bresp_if;
    rgx_port<IF_GEN::if_tc_tcm_b_rtn>           m_tc_tcm_b_rtn0_if;
    rgx_port<IF_GEN::if_tc_tcm_b_rtn>           m_tc_tcm_b_rtn1_if;
    rgx_port<IF_GEN::if_tc_tcm_ac_sf_rtn>           m_tc_tcm_ac_sf_rtn_if;

    void ProcessTCWr(void);
    void ProcessTCBRd0(void);
    void ProcessTCBRd1(void);
    void ProcessTCMixRd(void);

public:
    TC_BE(sc_module_name module_name)
    : sc_module(module_name)
    , m_tc_wr_in("tc_wr_in")
    , m_tc_b_rd0_in("tc_b_rd0_in")
    , m_tc_b_rd1_in("tc_b_rd1_in")
    , m_tc_mix_rd_in("m_tc_mix_rd_in")
    , m_tc_tcm_bresp_if("tc_tcm_bresp_if")
    , m_tc_tcm_b_rtn0_if("tc_tcm_b_rtn0_if")
    , m_tc_tcm_b_rtn1_if("tc_tcm_b_rtn1_if")
    , m_tc_tcm_ac_sf_rtn_if("tc_tcm_ac_sf_rtn_if")
    {
        SC_THREAD(ProcessTCWr);
        sensitive << m_tc_wr_in.data_written_event();

        SC_THREAD(ProcessTCBRd0);
        sensitive << m_tc_b_rd0_in.data_written_event();

        SC_THREAD(ProcessTCBRd1);
        sensitive << m_tc_b_rd1_in.data_written_event();

        SC_THREAD(ProcessTCMixRd);
        sensitive << m_tc_mix_rd_in.data_written_event();
    }
};

class RVV_BE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(RVV_BE);

private:
    // communication fifo between RVV_BE and Bank
    sc_fifo_in<Request>  m_vlsu_wr_in;
    sc_fifo_in<Request>  m_vlsu_rd_in;

    // interface between RVV_BE and RVV
    rgx_port<IF_GEN::if_vlsu_tcm_bresp>           m_vlsu0_tcm_bresp_if;
    rgx_port<IF_GEN::if_vlsu_tcm_bresp>           m_vlsu1_tcm_bresp_if;
    rgx_port<IF_GEN::if_vlsu_tcm_bresp>           m_vlsu2_tcm_bresp_if;
    rgx_port<IF_GEN::if_vlsu_tcm_bresp>           m_vlsu3_tcm_bresp_if;
    rgx_port<IF_GEN::if_vlsu_tcm_rtn>           m_vlsu0_tcm_rtn_if;
    rgx_port<IF_GEN::if_vlsu_tcm_rtn>           m_vlsu1_tcm_rtn_if;
    rgx_port<IF_GEN::if_vlsu_tcm_rtn>           m_vlsu2_tcm_rtn_if;
    rgx_port<IF_GEN::if_vlsu_tcm_rtn>           m_vlsu3_tcm_rtn_if;

    void ProcessVLSUWr(void);
    void ProcessVLSURd(void);

public:
    RVV_BE(sc_module_name module_name)
    : sc_module(module_name)
    , m_vlsu_wr_in("vlsu_wr_in")
    , m_vlsu_rd_in("vlsu_rd_in")
    , m_vlsu0_tcm_bresp_if("vlsu0_tcm_bresp_if")
    , m_vlsu1_tcm_bresp_if("vlsu1_tcm_bresp_if")
    , m_vlsu2_tcm_bresp_if("vlsu2_tcm_bresp_if")
    , m_vlsu3_tcm_bresp_if("vlsu3_tcm_bresp_if")
    , m_vlsu0_tcm_rtn_if("vlsu0_tcm_rtn_if")
    , m_vlsu1_tcm_rtn_if("vlsu1_tcm_rtn_if")
    , m_vlsu2_tcm_rtn_if("vlsu2_tcm_rtn_if")
    , m_vlsu3_tcm_rtn_if("vlsu3_tcm_rtn_if")
    {
        SC_THREAD(ProcessVLSUWr);
        sensitive << m_vlsu_wr_in.data_written_event();

        SC_THREAD(ProcessVLSURd);
        sensitive << m_vlsu_rd_in.data_written_event();
    }
};

class NIU_BE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(NIU_BE);

private:
    // communication fifo among NIU_BE, Bank and AS_pipe
    sc_fifo_in<Request>  m_bank_niu_wr_rd_in;
    sc_fifo_in<Request>  m_as_niu_wr_rd_in;

    sc_fifo<IF_GEN::if_niu_tcm_wresp> m_bank_niu_wresp_fifo;
    sc_fifo<IF_GEN::if_niu_tcm_wresp> m_as_niu_wresp_fifo;
    sc_fifo<IF_GEN::if_niu_tcm_rtn> m_bank_niu_rtn_fifo;
    sc_fifo<IF_GEN::if_niu_tcm_rtn> m_as_niu_rtn_fifo;
    // interface between NIU_BE and NIU
    rgx_port<IF_GEN::if_niu_tcm_wresp>    m_niu_tcm_wresp_if;
    rgx_port<IF_GEN::if_niu_tcm_rtn>      m_niu_tcm_rtn_if;
    rgx_port<IF_GEN::if_tcm_niu_remt_sem> m_tcm_niu_remt_sem_if;

    void ProcessNIUBankWrRd(void);
    void ProcessNIUASWrRd(void);
    void ProcessNIUWrespRoundRobin(void);
    void ProcessNIURtnRoundRobin(void);

public:
    NIU_BE(sc_module_name module_name)
    : sc_module(module_name)
    , m_bank_niu_wr_rd_in("bank_niu_wr_rd_in")
    , m_as_niu_wr_rd_in("as_niu_wr_rd_in")
    , m_bank_niu_wresp_fifo("bank_niu_wresp_fifo", 1)
    , m_as_niu_wresp_fifo("as_niu_wresp_fifo", 1)
    , m_bank_niu_rtn_fifo("bank_niu_rtn_fifo", 1)
    , m_as_niu_rtn_fifo("as_niu_rtn_fifo", 1)
    , m_niu_tcm_wresp_if("niu_tcm_wresp_if")
    , m_niu_tcm_rtn_if("niu_tcm_rtn_if")
    , m_tcm_niu_remt_sem_if("tcm_niu_remt_sem_if")
    {
        SC_THREAD(ProcessNIUBankWrRd);
        sensitive << m_bank_niu_wr_rd_in.data_written_event();
        SC_THREAD(ProcessNIUASWrRd);
        sensitive << m_as_niu_wr_rd_in.data_written_event();
        SC_THREAD(ProcessNIUWrespRoundRobin);
        sensitive << m_bank_niu_wresp_fifo.data_written_event()
                  << m_as_niu_wresp_fifo.data_written_event();
        SC_THREAD(ProcessNIURtnRoundRobin);
        sensitive << m_bank_niu_rtn_fifo.data_written_event()
                  << m_as_niu_rtn_fifo.data_written_event();
    }
};

class RVC_BE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(RVC_BE);

private:
    // communication fifo between RVC_BE and AS_pipe
    sc_fifo_in<Request> m_as_rvc_in;
    sc_fifo_in<Request> m_as_cfi_in;

    // interface between RVC_BE and RVC
    rgx_port<IF_GEN::if_rvs_tcm_b>       m_rvs_tcm_b_if;
    rgx_port<IF_GEN::if_rvs_tcm_r>       m_rvs_tcm_r_if;
    rgx_port<IF_GEN::if_acc_resp>        m_acc_resp_if;
    rgx_port<IF_GEN::if_rvs_tcm_cfi_ack> m_rvs_tcm_cfi_ack_if;
    
    void ProcessRVCWrRd(void);
    void ProcessRVCCfi(void);

public:
    RVC_BE(sc_module_name module_name)
    : sc_module(module_name)
    , m_as_rvc_in("as_rvc_in")
    , m_as_cfi_in("as_cfi_in")
    , m_rvs_tcm_b_if("rvs_tcm_b_if")
    , m_rvs_tcm_r_if("rvs_tcm_r_if")
    , m_acc_resp_if("acc_resp_if")
    , m_rvs_tcm_cfi_ack_if("rvs_tcm_cfi_ack_if")
    {
        SC_THREAD(ProcessRVCWrRd);
        sensitive << m_as_rvc_in.data_written_event();
        SC_THREAD(ProcessRVCCfi);
        sensitive << m_as_cfi_in.data_written_event();
    }
};

class CP_BE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(CP_BE);

private:
    // communication fifo between CP_BE and AS_pipe
    sc_fifo_in<Request> m_as_cp_in;

    // interface between CP_BE and CP
    rgx_port<IF_GEN::if_bif_rtn>           m_bif_rtn_if;

    void ProcessCPRd(void);

public:
    CP_BE(sc_module_name module_name)
    : sc_module(module_name)
    , m_as_cp_in("as_cp_in")
    , m_bif_rtn_if("bif_rtn_if")
    {
        SC_THREAD(ProcessCPRd);
        sensitive << m_as_cp_in.data_written_event();
    }
};

class BANK : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(BANK);

private:
    struct InterfaceBuf {
        bool valid = false;
        Request  req = {};
    };
    struct BankBuf{
        uint32_t ValidBufNum = 0;
        InterfaceBuf ifbuf[16];
    };
    enum InterfaceType {
        DMA_W0   = 0x0,
        DMA_W1   = 0x1,
        TC_B_R   = 0x2,
        TC_MIX_R = 0x3,
        DMA_R    = 0x4,
        AS_WR    = 0x5,
        TC_W     = 0x6,
        RVV0_R   = 0x7,
        RVV1_R   = 0x8,
        RVV2_R   = 0x9,
        RVV3_R   = 0xA,
        RVV0_W   = 0xB,
        RVV1_W   = 0xC,
        RVV2_W   = 0xD,
        RVV3_W   = 0xE,
        NIU_WR   = 0xF  
    };
    struct RVVNIUARB{
        bool valid = false;
        InterfaceType iftype = DMA_W0;
        uint8_t lane_idx = 0;
    };
    
    BankBuf buf;
    RVVNIUARB rvv_niu_arb_buf[3];

    uint8_t lanemask[8] = {1,1,1,1,1,1,1,1};

    uint32_t storage[BANK_ROW_NUM][BANK_LANE_NUM][BANK_BANK_NUM][BANK_BYTE_NUM / 32];

    uint8_t rr_rvv_w = 0;
    uint8_t rr_rvv_r = 0;
    uint8_t rr_rvv_niu = 0;
    
    // interface between FE and BANK
    sc_fifo_in<Request> m_dma_wr0_in;
    sc_fifo_in<Request> m_dma_wr1_in;
    sc_fifo_in<Request> m_dma_rd_data_in;

    sc_fifo_in<Request> m_tc_wr_in;
    sc_fifo_in<Request> m_tc_rd0_in;
    sc_fifo_in<Request> m_tc_rd1_in;

    sc_fifo_in<Request> m_vlsu0_wr_in;
    sc_fifo_in<Request> m_vlsu1_wr_in;
    sc_fifo_in<Request> m_vlsu2_wr_in;
    sc_fifo_in<Request> m_vlsu3_wr_in;
    sc_fifo_in<Request> m_vlsu0_rd_in;
    sc_fifo_in<Request> m_vlsu1_rd_in;
    sc_fifo_in<Request> m_vlsu2_rd_in;
    sc_fifo_in<Request> m_vlsu3_rd_in;

    sc_fifo_in<Request> m_niu_wr_rd_in;

    sc_fifo_in<Request> m_as_wr_rd_in;

    // communication fifo between BANK and Copy BE
    sc_fifo_out<Request>  m_dma_wr0_out;
    sc_fifo_out<Request>  m_dma_wr1_out;
    sc_fifo_out<Request>  m_dma_rd_data_out;

    sc_fifo_out<Request>  m_tc_wr_out;
    sc_fifo_out<Request>  m_tc_b_rd0_out;
    sc_fifo_out<Request>  m_tc_b_rd1_out;
    sc_fifo_out<Request>  m_tc_mix_rd_out;

    sc_fifo_out<Request>  m_vlsu_wr_out;
    sc_fifo_out<Request>  m_vlsu_rd_out;

    sc_fifo_out<Request>  m_niu_wr_rd_out;

    sc_fifo_out<Request>  m_as_rd_out;
    
    void ProcessBank(void);
    void UpdateBankBuf(void);
    void ProcessBuf(void);
    int LaneCheck(Request req);
    Request BankWrRd(Request req);

public:
    BANK(sc_module_name module_name)
    : sc_module(module_name)
    , m_dma_wr0_in("dma_wr0_in")
    , m_dma_wr1_in("dma_wr1_in")
    , m_dma_rd_data_in("dma_rd_data_in")
    , m_tc_wr_in("tc_wr_in")
    , m_tc_rd0_in("tc_rd0_in")
    , m_tc_rd1_in("tc_rd1_in")
    , m_vlsu0_wr_in("vlsu0_wr_in")
    , m_vlsu1_wr_in("vlsu1_wr_in")
    , m_vlsu2_wr_in("vlsu2_wr_in")
    , m_vlsu3_wr_in("vlsu3_wr_in")
    , m_vlsu0_rd_in("vlsu0_rd_in")
    , m_vlsu1_rd_in("vlsu1_rd_in")
    , m_vlsu2_rd_in("vlsu2_rd_in")
    , m_vlsu3_rd_in("vlsu3_rd_in")
    , m_niu_wr_rd_in("niu_wr_rd_in")
    , m_as_wr_rd_in("as_wr_rd_in")
    , m_dma_wr0_out("dma_wr0_out")
    , m_dma_wr1_out("dma_wr1_out")
    , m_dma_rd_data_out("dma_rd_data_out")
    , m_tc_wr_out("tc_wr_out")
    , m_tc_b_rd0_out("tc_b_rd0_out")
    , m_tc_b_rd1_out("tc_b_rd1_out")
    , m_tc_mix_rd_out("tc_mix_rd_out")
    , m_vlsu_wr_out("vlsu_wr_out")
    , m_vlsu_rd_out("vlsu_rd_out")
    , m_niu_wr_rd_out("niu_wr_rd_out")
    , m_as_rd_out("as_rd_out")
    {
        SC_THREAD(ProcessBank);
        sensitive << m_dma_wr0_in.data_written_event()
                  << m_dma_wr1_in.data_written_event()
                  << m_dma_rd_data_in.data_written_event()
                  << m_tc_wr_in.data_written_event()
                  << m_tc_rd0_in.data_written_event()
                  << m_tc_rd1_in.data_written_event()
                  << m_vlsu0_wr_in.data_written_event()
                  << m_vlsu1_wr_in.data_written_event()
                  << m_vlsu2_wr_in.data_written_event()
                  << m_vlsu3_wr_in.data_written_event()
                  << m_vlsu0_rd_in.data_written_event()
                  << m_vlsu1_rd_in.data_written_event()
                  << m_vlsu2_rd_in.data_written_event()
                  << m_vlsu3_rd_in.data_written_event()
                  << m_niu_wr_rd_in.data_written_event();
    }
};

class AS_PIPE : public sc_module
{
    friend class PIM_TCM;
    SC_HAS_PROCESS(AS_PIPE);

private:
    // 信号量结构
    struct SemVal {
        uint64_t raw;
        uint32_t wait         : 1;  // [0:0]
        uint32_t expect_value : 16; // [16:1]
        uint32_t post_value   : 16; // [32:17]
        uint32_t wait_core_id : 16; // [48:33]
        uint64_t trans_id     : 15; // [63:49]
        
        SemVal(uint64_t val = 0) : raw(val) {
            wait         = (val >> 0)  & 0x1;
            expect_value = (val >> 1)  & 0xFFFF;
            post_value   = (val >> 17) & 0xFFFF;
            wait_core_id = (val >> 33) & 0xFFFF;
            trans_id     = (val >> 49);
        }

        void update_raw_val() {
            raw = (uint64_t(wait)         << 0)
                + (uint64_t(expect_value) << 1)
                + (uint64_t(post_value)   << 17)
                + (uint64_t(wait_core_id) << 33)
                + (uint64_t(trans_id)     << 49);
        }
    };

    struct BarrierSideband {
        uint8_t init_val = 0;
        uint8_t rvc_cnt = 0;
        uint8_t trans_id[8] = {};

        void reset(){
            rvc_cnt = init_val;
            memset(trans_id, 0, sizeof(trans_id));
        }
    };

    struct CacheLine{
        bool tag_vld = 0;
        uint32_t tag_tag = 0;
        uint32_t tag_age_cnt = 0;
        bool cl_rdy = 0;
        bool cl_dirty = 0;
        uint32_t cl_miss_ref_cnt = 0;
        uint32_t data[BANK_BANK_NUM * BANK_BYTE_NUM / 32] = {};
        uint32_t cl_mask [BANK_BANK_NUM * BANK_BYTE_NUM / 32] = {};

        void reset(){
            tag_vld = 0;
            tag_tag = 0;
            tag_age_cnt = 0;
            cl_rdy = 0;
            cl_dirty = 0;
            cl_miss_ref_cnt = 0;
            memset(data, 0, sizeof(data));
            memset(cl_mask, 0, sizeof(cl_mask));
        }
    };

    enum CacheReplaceType{
        EMPTY  = 0x0,
        UNLOCK = 0x1
    };

    enum CacheMasterType{
        HIT_Q  = 0x0,
        MISS_Q = 0x1,
        BANK = 0x2
    };

    CacheLine cache[8];
    
    std::queue<Request> hit_q;
    std::queue<Request> miss_q;

    uint8_t plsu_n[3][4] = {};

    BarrierSideband barrier_sb_buf[3];

    sc_event m_hit_q_pop_event;
    sc_event m_miss_q_pop_event;
    sc_event m_cache_unlock_event;
    sc_event m_hit_q_push_event;
    sc_event m_miss_q_push_event;
 
    // interface among AS_PIPE, bank and MASKTER_FE
    sc_fifo_in<Request> m_dma_rd_desc_in;
    sc_fifo_in<Request> m_dma_sem_in;

    sc_fifo_in<Request> m_tc_sem_in;

    sc_fifo_in<Request> m_vlsu0_sem_in;
    sc_fifo_in<Request> m_vlsu1_sem_in;
    sc_fifo_in<Request> m_vlsu2_sem_in;
    sc_fifo_in<Request> m_vlsu3_sem_in;
    
    sc_fifo_in<Request> m_niu_rvc_wr_rd_in;

    sc_fifo_in<Request>  m_rvc_wr_rd_in;
    sc_fifo_in<Request>  m_rvc_acc_in;
    sc_fifo_in<Request>  m_rvc_barrier_cfi_in;

    sc_fifo_in<Request> m_cp_wr_in;

    sc_fifo_in<Request> m_bank_wr_in;

    // AS internal interface
    sc_fifo<Request> m_niu_sem;
    sc_fifo<Request> m_arb2_out;
    sc_fifo<Request> m_arb1_out;
    sc_fifo<Request> m_arb3_out;
    sc_fifo<Request> m_cfi_out;

    // communication fifo among as_pipe, bank and MASTER_BE
    sc_fifo_out<Request>  m_as_wr_rd_out;

    sc_fifo_out<Request>  m_dma_rd_desc_out;

    sc_fifo_out<Request>  m_as_niu_wr_rd_out;

    sc_fifo_out<Request>  m_as_rvc_out;
    sc_fifo_out<Request> m_as_cfi_out;

    sc_fifo_out<Request>  m_as_cp_out;
    
    void ProcessArb2(void);
    void ProcessArb1(void);
    void ProcessArb3(void);
    void HitTest(void);
    int PLSU(CacheReplaceType repl_type);
    void ProcessCacheArb(void);
    void ProcessCache(Request req, CacheMasterType cache_master_type);
    void ProcessSem(Request req);
    void ProcessAtomic(Request req);
    void CacheWrRd(Request req);
    void ProcessBarrier(void);
    void ProcessCfi(void);

public:
    AS_PIPE(sc_module_name module_name)
    : sc_module(module_name)
    , m_dma_rd_desc_in("dma_rd_desc_in")
    , m_dma_sem_in("dma_sem_in")
    , m_tc_sem_in("tc_sem_in")
    , m_vlsu0_sem_in("vlsu0_sem_in")
    , m_vlsu1_sem_in("vlsu1_sem_in")
    , m_vlsu2_sem_in("vlsu2_sem_in")
    , m_vlsu3_sem_in("vlsu3_sem_in")
    , m_niu_rvc_wr_rd_in("niu_rvc_wr_rd_in")
    , m_rvc_wr_rd_in("rvc_wr_rd_in")
    , m_rvc_acc_in("rvc_acc_in")
    , m_rvc_barrier_cfi_in("rvc_barrier_cfi_in")
    , m_cp_wr_in("cp_wr_in")
    , m_bank_wr_in("bank_wr_in")
    , m_niu_sem("niu_sem", 1)
    , m_arb2_out("arb2_out", 1)
    , m_arb1_out("arb1_out", 1)
    , m_arb3_out("arb3_out", 1)
    , m_cfi_out("cfi_out", 1)
    , m_as_wr_rd_out("as_wr_rd_out")
    , m_dma_rd_desc_out("dma_rd_desc_out")
    , m_as_niu_wr_rd_out("as_niu_wr_rd_out")
    , m_as_rvc_out("as_rvc_out")
    , m_as_cfi_out("as_cfi_out")
    , m_as_cp_out("as_cp_out")
    {
        SC_THREAD(ProcessArb2);
        sensitive << m_dma_rd_desc_in.data_written_event()
                  << m_niu_rvc_wr_rd_in.data_written_event()
                  << m_rvc_wr_rd_in.data_written_event()
                  << m_rvc_acc_in.data_written_event()
                  << m_cp_wr_in.data_written_event();
        SC_THREAD(ProcessArb1);
        sensitive << m_niu_sem.data_written_event()
                  << m_vlsu0_sem_in.data_written_event()
                  << m_vlsu1_sem_in.data_written_event()
                  << m_vlsu2_sem_in.data_written_event()
                  << m_vlsu3_sem_in.data_written_event()
                  << m_tc_sem_in.data_written_event()
                  << m_dma_sem_in.data_written_event();
        SC_THREAD(ProcessArb3);
        sensitive << m_arb2_out.data_written_event()
                  << m_arb1_out.data_written_event();
        SC_THREAD(HitTest);
        sensitive << m_arb3_out.data_written_event()
                  << m_cfi_out.data_written_event();
        SC_THREAD(ProcessCacheArb);
        SC_THREAD(ProcessBarrier);
        sensitive << m_rvc_barrier_cfi_in.data_written_event();
    }
};


// ============================================================================
//                             Top Module: PIM_TCM
// ============================================================================
class PIM_TCM : public sc_module
{
    SC_HAS_PROCESS(PIM_TCM);

public:
    // ========================= external DMA interfaces =========================
    rgx_export<IF_GEN::if_dma_tcm_wr_req>       m_ext_dma_tcm_wr0_req_if;
    rgx_export<IF_GEN::if_dma_tcm_wr_req>       m_ext_dma_tcm_wr1_req_if;
    rgx_export<IF_GEN::if_dma_tcm_rd_data_req>  m_ext_dma_tcm_rd_data_req_if;
    rgx_export<IF_GEN::if_dma_tcm_rd_desc_req>  m_ext_dma_tcm_rd_desc_req_if;

    rgx_export<IF_GEN::if_tcm_dma_bresp>        m_ext_tcm_dma_bresp0_if;
    rgx_export<IF_GEN::if_tcm_dma_bresp>        m_ext_tcm_dma_bresp1_if;
    rgx_export<IF_GEN::if_tcm_dma_rd_data_rtn>  m_ext_tcm_dma_rd_data_rtn_if;
    rgx_export<IF_GEN::if_tcm_dma_rd_desc_rtn>  m_ext_tcm_dma_rd_desc_rtn_if;

    // ========================== external TC interfaces =========================
    rgx_export<IF_GEN::if_tc_tcm_wr>            m_ext_tc_tcm_wr_if;
    rgx_export<IF_GEN::if_tc_tcm_b_rd>          m_ext_tc_tcm_b_rd_if;
    rgx_export<IF_GEN::if_tc_tcm_ac_sf_rd>      m_ext_tc_tcm_ac_sf_rd_if;

    rgx_export<IF_GEN::if_tc_tcm_bresp>         m_ext_tc_tcm_bresp_if;
    rgx_export<IF_GEN::if_tc_tcm_b_rtn>         m_ext_tc_tcm_b_rtn0_if;
    rgx_export<IF_GEN::if_tc_tcm_b_rtn>         m_ext_tc_tcm_b_rtn1_if;
    rgx_export<IF_GEN::if_tc_tcm_ac_sf_rtn>     m_ext_tc_tcm_ac_sf_rtn_if;

    // ========================== external RVV interfaces ========================
    rgx_export<IF_GEN::if_vlsu_tcm_wr>          m_ext_vlsu0_tcm_wr_if;
    rgx_export<IF_GEN::if_vlsu_tcm_wr>          m_ext_vlsu1_tcm_wr_if;
    rgx_export<IF_GEN::if_vlsu_tcm_wr>          m_ext_vlsu2_tcm_wr_if;
    rgx_export<IF_GEN::if_vlsu_tcm_wr>          m_ext_vlsu3_tcm_wr_if;

    rgx_export<IF_GEN::if_vlsu_tcm_rd>          m_ext_vlsu0_tcm_rd_if;
    rgx_export<IF_GEN::if_vlsu_tcm_rd>          m_ext_vlsu1_tcm_rd_if;
    rgx_export<IF_GEN::if_vlsu_tcm_rd>          m_ext_vlsu2_tcm_rd_if;
    rgx_export<IF_GEN::if_vlsu_tcm_rd>          m_ext_vlsu3_tcm_rd_if;

    rgx_export<IF_GEN::if_vlsu_tcm_bresp>       m_ext_vlsu0_tcm_bresp_if;
    rgx_export<IF_GEN::if_vlsu_tcm_bresp>       m_ext_vlsu1_tcm_bresp_if;
    rgx_export<IF_GEN::if_vlsu_tcm_bresp>       m_ext_vlsu2_tcm_bresp_if;
    rgx_export<IF_GEN::if_vlsu_tcm_bresp>       m_ext_vlsu3_tcm_bresp_if;

    rgx_export<IF_GEN::if_vlsu_tcm_rtn>         m_ext_vlsu0_tcm_rtn_if;
    rgx_export<IF_GEN::if_vlsu_tcm_rtn>         m_ext_vlsu1_tcm_rtn_if;
    rgx_export<IF_GEN::if_vlsu_tcm_rtn>         m_ext_vlsu2_tcm_rtn_if;
    rgx_export<IF_GEN::if_vlsu_tcm_rtn>         m_ext_vlsu3_tcm_rtn_if;

    // ========================== external NIU interfaces ========================
    rgx_export<IF_GEN::if_niu_tcm_wr>           m_ext_niu_tcm_wr_if;
    rgx_export<IF_GEN::if_niu_tcm_wrdata>       m_ext_niu_tcm_wrdata_if;
    rgx_export<IF_GEN::if_niu_tcm_rd>           m_ext_niu_tcm_rd_if;

    rgx_export<IF_GEN::if_niu_tcm_wresp>        m_ext_niu_tcm_wresp_if;
    rgx_export<IF_GEN::if_niu_tcm_rtn>          m_ext_niu_tcm_rtn_if;
    rgx_export<IF_GEN::if_tcm_niu_remt_sem>     m_ext_tcm_niu_remt_sem_if;

    // ========================== external RVC interfaces ========================
    rgx_export<IF_GEN::if_rvs_tcm_aw>           m_ext_rvs_tcm_aw_if;
    rgx_export<IF_GEN::if_rvs_tcm_w>            m_ext_rvs_tcm_w_if;
    rgx_export<IF_GEN::if_rvs_tcm_ar>           m_ext_rvs_tcm_ar_if;
    rgx_export<IF_GEN::if_rvs_tcm_kick>         m_ext_rvs_tcm_kick_if;
    rgx_export<IF_GEN::if_acc_req>              m_ext_acc_req_if;
    rgx_export<IF_GEN::if_rvs_tcm_cfi_req>      m_ext_rvs_tcm_cfi_req_if;

    rgx_export<IF_GEN::if_rvs_tcm_b>            m_ext_rvs_tcm_b_if;
    rgx_export<IF_GEN::if_rvs_tcm_r>            m_ext_rvs_tcm_r_if;
    rgx_export<IF_GEN::if_acc_resp>             m_ext_acc_resp_if;
    rgx_export<IF_GEN::if_rvs_tcm_cfi_ack>      m_ext_rvs_tcm_cfi_ack_if;

    // =========================== external CP interfaces ========================
    rgx_export<IF_GEN::if_bif_cmd>              m_ext_bif_cmd_if;
    rgx_export<IF_GEN::if_bif_write>            m_ext_bif_write_if;
    rgx_export<IF_GEN::if_bif_rtn>              m_ext_bif_rtn_if;

private:
    // ============================== internal fifo ==============================
    sc_fifo<Request> m_dma_wr0_fe2bank_fifo;
    sc_fifo<Request> m_dma_wr1_fe2bank_fifo;
    sc_fifo<Request> m_dma_rd_data_fe2bank_fifo;
    sc_fifo<Request> m_dma_rd_desc_fe2as_fifo;
    sc_fifo<Request> m_dma_sem_fe2as_fifo;

    sc_fifo<Request> m_tc_wr_fe2bank_fifo;
    sc_fifo<Request> m_tc_rd0_fe2bank_fifo;
    sc_fifo<Request> m_tc_rd1_fe2bank_fifo;
    sc_fifo<Request> m_tc_sem_fe2as_fifo;

    sc_fifo<Request> m_vlsu0_wr_fe2bank_fifo;
    sc_fifo<Request> m_vlsu1_wr_fe2bank_fifo;
    sc_fifo<Request> m_vlsu2_wr_fe2bank_fifo;
    sc_fifo<Request> m_vlsu3_wr_fe2bank_fifo;
    sc_fifo<Request> m_vlsu0_rd_fe2bank_fifo;
    sc_fifo<Request> m_vlsu1_rd_fe2bank_fifo;
    sc_fifo<Request> m_vlsu2_rd_fe2bank_fifo;
    sc_fifo<Request> m_vlsu3_rd_fe2bank_fifo;
    sc_fifo<Request> m_vlsu0_sem_fe2as_fifo;
    sc_fifo<Request> m_vlsu1_sem_fe2as_fifo;
    sc_fifo<Request> m_vlsu2_sem_fe2as_fifo;
    sc_fifo<Request> m_vlsu3_sem_fe2as_fifo;

    sc_fifo<Request> m_niu_wr_rd_fe2bank_fifo;
    sc_fifo<Request> m_niu_rvc_wr_rd_fe2as_fifo;

    sc_fifo<Request> m_rvc_wr_rd_fe2as_fifo;
    sc_fifo<Request> m_rvc_acc_fe2as_fifo;
    sc_fifo<Request> m_rvc_barrier_cfi_fe2as_fifo;

    sc_fifo<Request> m_cp_fe2as_fifo;

    sc_fifo<Request> m_dma_wr0_bank2be_fifo;
    sc_fifo<Request> m_dma_wr1_bank2be_fifo;
    sc_fifo<Request> m_dma_rd_data_bank2be_fifo;
    sc_fifo<Request> m_tc_wr_bank2be_fifo;
    sc_fifo<Request> m_tc_b_rd0_bank2be_fifo;
    sc_fifo<Request> m_tc_b_rd1_bank2be_fifo;
    sc_fifo<Request> m_tc_mix_rd_bank2be_fifo;
    sc_fifo<Request> m_vlsu_wr_bank2be_fifo;
    sc_fifo<Request> m_vlsu_rd_bank2be_fifo;
    sc_fifo<Request> m_niu_wr_rd_bank2be_fifo;
    sc_fifo<Request> m_as_rd_bank2as_fifo;

    sc_fifo<Request> m_as_wr_rd_as2bank_fifo;
    sc_fifo<Request> m_dma_rd_desc_as2be_fifo;
    sc_fifo<Request> m_niu_wr_rd_as2be_fifo;
    sc_fifo<Request> m_rvc_as2be_fifo;
    sc_fifo<Request> m_cfi_as2be_fifo;
    sc_fifo<Request> m_cp_as2be_fifo;

    // ============================= internal modules ============================
    DMA_FE  m_dma_fe;
    TC_FE   m_tc_fe;
    RVV_FE  m_rvv_fe;
    NIU_FE  m_niu_fe;
    RVC_FE  m_rvc_fe;
    CP_FE   m_cp_fe;

    BANK    m_bank;
    AS_PIPE m_as_pipe;

    DMA_BE  m_dma_be;
    TC_BE   m_tc_be;
    RVV_BE  m_rvv_be;
    NIU_BE  m_niu_be;
    RVC_BE  m_rvc_be;
    CP_BE   m_cp_be;

public:
    PIM_TCM(sc_module_name module_name)
    : sc_module(module_name)
    , m_ext_dma_tcm_wr0_req_if("ext_dma_tcm_wr0_req_if")
    , m_ext_dma_tcm_wr1_req_if("ext_dma_tcm_wr1_req_if")
    , m_ext_dma_tcm_rd_data_req_if("ext_dma_tcm_rd_data_req_if")
    , m_ext_dma_tcm_rd_desc_req_if("ext_dma_tcm_rd_desc_req_if")
    , m_ext_tcm_dma_bresp0_if("ext_tcm_dma_bresp0_if")
    , m_ext_tcm_dma_bresp1_if("ext_tcm_dma_bresp1_if")
    , m_ext_tcm_dma_rd_data_rtn_if("ext_tcm_dma_rd_data_rtn_if")
    , m_ext_tcm_dma_rd_desc_rtn_if("ext_tcm_dma_rd_desc_rtn_if")
    , m_ext_tc_tcm_wr_if("ext_tc_tcm_wr_if")
    , m_ext_tc_tcm_b_rd_if("ext_tc_tcm_b_rd_if")
    , m_ext_tc_tcm_ac_sf_rd_if("ext_tc_tcm_ac_sf_rd_if")
    , m_ext_tc_tcm_bresp_if("ext_tc_tcm_bresp_if")
    , m_ext_tc_tcm_b_rtn0_if("ext_tc_tcm_b_rtn0_if")
    , m_ext_tc_tcm_b_rtn1_if("ext_tc_tcm_b_rtn1_if")
    , m_ext_tc_tcm_ac_sf_rtn_if("ext_tc_tcm_ac_sf_rtn_if")
    , m_ext_vlsu0_tcm_wr_if("ext_vlsu0_tcm_wr_if")
    , m_ext_vlsu1_tcm_wr_if("ext_vlsu1_tcm_wr_if")
    , m_ext_vlsu2_tcm_wr_if("ext_vlsu2_tcm_wr_if")
    , m_ext_vlsu3_tcm_wr_if("ext_vlsu3_tcm_wr_if")
    , m_ext_vlsu0_tcm_rd_if("ext_vlsu0_tcm_rd_if")
    , m_ext_vlsu1_tcm_rd_if("ext_vlsu1_tcm_rd_if")
    , m_ext_vlsu2_tcm_rd_if("ext_vlsu2_tcm_rd_if")
    , m_ext_vlsu3_tcm_rd_if("ext_vlsu3_tcm_rd_if")
    , m_ext_vlsu0_tcm_bresp_if("ext_vlsu0_tcm_bresp_if")
    , m_ext_vlsu1_tcm_bresp_if("ext_vlsu1_tcm_bresp_if")
    , m_ext_vlsu2_tcm_bresp_if("ext_vlsu2_tcm_bresp_if")
    , m_ext_vlsu3_tcm_bresp_if("ext_vlsu3_tcm_bresp_if")
    , m_ext_vlsu0_tcm_rtn_if("ext_vlsu0_tcm_rtn_if")
    , m_ext_vlsu1_tcm_rtn_if("ext_vlsu1_tcm_rtn_if")
    , m_ext_vlsu2_tcm_rtn_if("ext_vlsu2_tcm_rtn_if")
    , m_ext_vlsu3_tcm_rtn_if("ext_vlsu3_tcm_rtn_if")
    , m_ext_niu_tcm_wr_if("ext_niu_tcm_wr_if")
    , m_ext_niu_tcm_wrdata_if("ext_niu_tcm_wrdata_if")
    , m_ext_niu_tcm_rd_if("ext_niu_tcm_rd_if")
    , m_ext_niu_tcm_wresp_if("ext_niu_tcm_wresp_if")
    , m_ext_niu_tcm_rtn_if("ext_niu_tcm_rtn_if")
    , m_ext_tcm_niu_remt_sem_if("ext_tcm_niu_remt_sem_if")
    , m_ext_rvs_tcm_aw_if("ext_rvs_tcm_aw_if")
    , m_ext_rvs_tcm_w_if("ext_rvs_tcm_w_if")
    , m_ext_rvs_tcm_ar_if("ext_rvs_tcm_ar_if")
    , m_ext_rvs_tcm_kick_if("ext_rvs_tcm_kick_if")
    , m_ext_acc_req_if("ext_acc_req_if")
    , m_ext_rvs_tcm_cfi_req_if("ext_rvs_tcm_cfi_req_if")
    , m_ext_rvs_tcm_b_if("ext_rvs_tcm_b_if")
    , m_ext_rvs_tcm_r_if("ext_rvs_tcm_r_if")
    , m_ext_acc_resp_if("ext_acc_resp_if")
    , m_ext_rvs_tcm_cfi_ack_if("ext_rvs_tcm_cfi_ack_if")
    , m_ext_bif_cmd_if("ext_bif_cmd_if")
    , m_ext_bif_write_if("ext_bif_write_if")
    , m_ext_bif_rtn_if("ext_bif_rtn_if")
    , m_dma_wr0_fe2bank_fifo("dma_wr0_fe2bank_fifo", 1)
    , m_dma_wr1_fe2bank_fifo("dma_wr1_fe2bank_fifo", 1)
    , m_dma_rd_data_fe2bank_fifo("dma_rd_data_fe2bank_fifo", 1)
    , m_dma_rd_desc_fe2as_fifo("dma_rd_desc_fe2as_fifo", 1)
    , m_dma_sem_fe2as_fifo("dma_sem_fe2as_fifo", 1)
    , m_tc_wr_fe2bank_fifo("tc_wr_fe2bank_fifo", 1)
    , m_tc_rd0_fe2bank_fifo("tc_rd0_fe2bank_fifo", 1)
    , m_tc_rd1_fe2bank_fifo("tc_rd1_fe2bank_fifo", 1)
    , m_tc_sem_fe2as_fifo("tc_sem_fe2as_fifo", 1)
    , m_vlsu0_wr_fe2bank_fifo("vlsu0_wr_fe2bank_fifo", 1)
    , m_vlsu1_wr_fe2bank_fifo("vlsu1_wr_fe2bank_fifo", 1)
    , m_vlsu2_wr_fe2bank_fifo("vlsu2_wr_fe2bank_fifo", 1)
    , m_vlsu3_wr_fe2bank_fifo("vlsu3_wr_fe2bank_fifo", 1)
    , m_vlsu0_rd_fe2bank_fifo("vlsu0_rd_fe2bank_fifo", 1)
    , m_vlsu1_rd_fe2bank_fifo("vlsu1_rd_fe2bank_fifo", 1)
    , m_vlsu2_rd_fe2bank_fifo("vlsu2_rd_fe2bank_fifo", 1)
    , m_vlsu3_rd_fe2bank_fifo("vlsu3_rd_fe2bank_fifo", 1)
    , m_vlsu0_sem_fe2as_fifo("vlsu0_sem_fe2as_fifo", 1)
    , m_vlsu1_sem_fe2as_fifo("vlsu1_sem_fe2as_fifo", 1)
    , m_vlsu2_sem_fe2as_fifo("vlsu2_sem_fe2as_fifo", 1)
    , m_vlsu3_sem_fe2as_fifo("vlsu3_sem_fe2as_fifo", 1)
    , m_niu_wr_rd_fe2bank_fifo("niu_wr_rd_fe2bank_fifo", 1)
    , m_niu_rvc_wr_rd_fe2as_fifo("niu_rvc_wr_rd_fe2as_fifo", 1)
    , m_rvc_wr_rd_fe2as_fifo("rvc_wr_rd_fe2as_fifo", 1)
    , m_rvc_acc_fe2as_fifo("rvc_acc_fe2as_fifo", 1)
    , m_rvc_barrier_cfi_fe2as_fifo("rvc_barrier_cfi_fe2as_fifo", 1)
    , m_cp_fe2as_fifo("cp_fe2as_fifo", 1)
    , m_dma_wr0_bank2be_fifo("dma_wr0_bank2be_fifo", 1)
    , m_dma_wr1_bank2be_fifo("dma_wr1_bank2be_fifo", 1)
    , m_dma_rd_data_bank2be_fifo("dma_rd_data_bank2be_fifo", 1)
    , m_tc_wr_bank2be_fifo("tc_wr_bank2be_fifo", 1)
    , m_tc_b_rd0_bank2be_fifo("tc_b_rd0_bank2be_fifo", 1)
    , m_tc_b_rd1_bank2be_fifo("tc_b_rd1_bank2be_fifo", 1)
    , m_tc_mix_rd_bank2be_fifo("tc_mix_rd_bank2be_fifo", 1)
    , m_vlsu_wr_bank2be_fifo("vlsu_wr_bank2be_fifo", 1)
    , m_vlsu_rd_bank2be_fifo("vlsu_rd_bank2be_fifo", 1)
    , m_niu_wr_rd_bank2be_fifo("niu_wr_rd_bank2be_fifo", 1)
    , m_as_rd_bank2as_fifo("as_rd_bank2as_fifo", 1)
    , m_as_wr_rd_as2bank_fifo("as_wr_rd_as2bank_fifo", 1)
    , m_dma_rd_desc_as2be_fifo("dma_rd_desc_as2be_fifo", 1)
    , m_niu_wr_rd_as2be_fifo("niu_wr_rd_as2be_fifo", 1)
    , m_rvc_as2be_fifo("rvc_as2be_fifo", 1)
    , m_cfi_as2be_fifo("cfi_as2be_fifo", 1)
    , m_cp_as2be_fifo("cp_as2be_fifo", 1)
    , m_dma_fe("dma_fe")
    , m_tc_fe("tc_fe")
    , m_rvv_fe("rvv_fe")
    , m_niu_fe("niu_fe")
    , m_rvc_fe("rvc_fe")
    , m_cp_fe("cp_fe")
    , m_bank("bank")
    , m_as_pipe("as_pipe")
    , m_dma_be("dma_be")
    , m_tc_be("tc_be")
    , m_rvv_be("rvv_be")
    , m_niu_be("niu_be")
    , m_rvc_be("rvc_be")
    , m_cp_be("cp_be")
    {
        // external -> FE
        m_ext_dma_tcm_wr0_req_if.bind(m_dma_fe.m_dma_tcm_wr0_req_if);
        m_ext_dma_tcm_wr1_req_if.bind(m_dma_fe.m_dma_tcm_wr1_req_if);
        m_ext_dma_tcm_rd_data_req_if.bind(m_dma_fe.m_dma_tcm_rd_data_req_if);
        m_ext_dma_tcm_rd_desc_req_if.bind(m_dma_fe.m_dma_tcm_rd_desc_req_if);

        m_ext_tc_tcm_wr_if.bind(m_tc_fe.m_tc_tcm_wr_if);
        m_ext_tc_tcm_b_rd_if.bind(m_tc_fe.m_tc_tcm_b_rd_if);
        m_ext_tc_tcm_ac_sf_rd_if.bind(m_tc_fe.m_tc_tcm_ac_sf_rd_if);

        m_ext_vlsu0_tcm_wr_if.bind(m_rvv_fe.m_vlsu0_tcm_wr_if);
        m_ext_vlsu1_tcm_wr_if.bind(m_rvv_fe.m_vlsu1_tcm_wr_if);
        m_ext_vlsu2_tcm_wr_if.bind(m_rvv_fe.m_vlsu2_tcm_wr_if);
        m_ext_vlsu3_tcm_wr_if.bind(m_rvv_fe.m_vlsu3_tcm_wr_if);
        m_ext_vlsu0_tcm_rd_if.bind(m_rvv_fe.m_vlsu0_tcm_rd_if);
        m_ext_vlsu1_tcm_rd_if.bind(m_rvv_fe.m_vlsu1_tcm_rd_if);
        m_ext_vlsu2_tcm_rd_if.bind(m_rvv_fe.m_vlsu2_tcm_rd_if);
        m_ext_vlsu3_tcm_rd_if.bind(m_rvv_fe.m_vlsu3_tcm_rd_if);

        m_ext_niu_tcm_wr_if.bind(m_niu_fe.m_niu_tcm_wr_if);
        m_ext_niu_tcm_wrdata_if.bind(m_niu_fe.m_niu_tcm_wrdata_if);
        m_ext_niu_tcm_rd_if.bind(m_niu_fe.m_niu_tcm_rd_if);

        m_ext_rvs_tcm_aw_if.bind(m_rvc_fe.m_rvs_tcm_aw_if);
        m_ext_rvs_tcm_w_if.bind(m_rvc_fe.m_rvs_tcm_w_if);
        m_ext_rvs_tcm_ar_if.bind(m_rvc_fe.m_rvs_tcm_ar_if);
        m_ext_rvs_tcm_kick_if.bind(m_rvc_fe.m_rvs_tcm_kick_if);
        m_ext_acc_req_if.bind(m_rvc_fe.m_acc_req_if);
        m_ext_rvs_tcm_cfi_req_if.bind(m_rvc_fe.m_rvs_tcm_cfi_req_if);

        m_ext_bif_cmd_if.bind(m_cp_fe.m_bif_cmd_if);
        m_ext_bif_write_if.bind(m_cp_fe.m_bif_write_if);

        // BE -> external
        m_dma_be.m_tcm_dma_bresp0_if(m_ext_tcm_dma_bresp0_if);
        m_dma_be.m_tcm_dma_bresp1_if(m_ext_tcm_dma_bresp1_if);
        m_dma_be.m_tcm_dma_rd_data_rtn_if(m_ext_tcm_dma_rd_data_rtn_if);
        m_dma_be.m_tcm_dma_rd_desc_rtn_if(m_ext_tcm_dma_rd_desc_rtn_if);

        m_tc_be.m_tc_tcm_bresp_if(m_ext_tc_tcm_bresp_if);
        m_tc_be.m_tc_tcm_b_rtn0_if(m_ext_tc_tcm_b_rtn0_if);
        m_tc_be.m_tc_tcm_b_rtn1_if(m_ext_tc_tcm_b_rtn1_if);
        m_tc_be.m_tc_tcm_ac_sf_rtn_if(m_ext_tc_tcm_ac_sf_rtn_if);

        m_rvv_be.m_vlsu0_tcm_bresp_if(m_ext_vlsu0_tcm_bresp_if);
        m_rvv_be.m_vlsu1_tcm_bresp_if(m_ext_vlsu1_tcm_bresp_if);
        m_rvv_be.m_vlsu2_tcm_bresp_if(m_ext_vlsu2_tcm_bresp_if);
        m_rvv_be.m_vlsu3_tcm_bresp_if(m_ext_vlsu3_tcm_bresp_if);
        m_rvv_be.m_vlsu0_tcm_rtn_if(m_ext_vlsu0_tcm_rtn_if);
        m_rvv_be.m_vlsu1_tcm_rtn_if(m_ext_vlsu1_tcm_rtn_if);
        m_rvv_be.m_vlsu2_tcm_rtn_if(m_ext_vlsu2_tcm_rtn_if);
        m_rvv_be.m_vlsu3_tcm_rtn_if(m_ext_vlsu3_tcm_rtn_if);

        m_niu_be.m_niu_tcm_wresp_if(m_ext_niu_tcm_wresp_if);
        m_niu_be.m_niu_tcm_rtn_if(m_ext_niu_tcm_rtn_if);
        m_niu_be.m_tcm_niu_remt_sem_if(m_ext_tcm_niu_remt_sem_if);

        m_rvc_be.m_rvs_tcm_b_if(m_ext_rvs_tcm_b_if);
        m_rvc_be.m_rvs_tcm_r_if(m_ext_rvs_tcm_r_if);
        m_rvc_be.m_acc_resp_if(m_ext_acc_resp_if);
        m_rvc_be.m_rvs_tcm_cfi_ack_if(m_ext_rvs_tcm_cfi_ack_if);

        m_cp_be.m_bif_rtn_if(m_ext_bif_rtn_if);

        // FE -> BANK
        m_dma_fe.m_dma_wr0_out(m_dma_wr0_fe2bank_fifo);
        m_dma_fe.m_dma_wr1_out(m_dma_wr1_fe2bank_fifo);
        m_dma_fe.m_dma_rd_data_out(m_dma_rd_data_fe2bank_fifo);

        m_tc_fe.m_tc_wr_out(m_tc_wr_fe2bank_fifo);
        m_tc_fe.m_tc_rd0_out(m_tc_rd0_fe2bank_fifo);
        m_tc_fe.m_tc_rd1_out(m_tc_rd1_fe2bank_fifo);

        m_rvv_fe.m_vlsu0_wr_out(m_vlsu0_wr_fe2bank_fifo);
        m_rvv_fe.m_vlsu1_wr_out(m_vlsu1_wr_fe2bank_fifo);
        m_rvv_fe.m_vlsu2_wr_out(m_vlsu2_wr_fe2bank_fifo);
        m_rvv_fe.m_vlsu3_wr_out(m_vlsu3_wr_fe2bank_fifo);
        m_rvv_fe.m_vlsu0_rd_out(m_vlsu0_rd_fe2bank_fifo);
        m_rvv_fe.m_vlsu1_rd_out(m_vlsu1_rd_fe2bank_fifo);
        m_rvv_fe.m_vlsu2_rd_out(m_vlsu2_rd_fe2bank_fifo);
        m_rvv_fe.m_vlsu3_rd_out(m_vlsu3_rd_fe2bank_fifo);

        m_niu_fe.m_niu_wr_rd_out(m_niu_wr_rd_fe2bank_fifo);

        m_bank.m_dma_wr0_in(m_dma_wr0_fe2bank_fifo);
        m_bank.m_dma_wr1_in(m_dma_wr1_fe2bank_fifo);
        m_bank.m_dma_rd_data_in(m_dma_rd_data_fe2bank_fifo);
        m_bank.m_tc_wr_in(m_tc_wr_fe2bank_fifo);
        m_bank.m_tc_rd0_in(m_tc_rd0_fe2bank_fifo);
        m_bank.m_tc_rd1_in(m_tc_rd1_fe2bank_fifo);
        m_bank.m_vlsu0_wr_in(m_vlsu0_wr_fe2bank_fifo);
        m_bank.m_vlsu1_wr_in(m_vlsu1_wr_fe2bank_fifo);
        m_bank.m_vlsu2_wr_in(m_vlsu2_wr_fe2bank_fifo);
        m_bank.m_vlsu3_wr_in(m_vlsu3_wr_fe2bank_fifo);
        m_bank.m_vlsu0_rd_in(m_vlsu0_rd_fe2bank_fifo);
        m_bank.m_vlsu1_rd_in(m_vlsu1_rd_fe2bank_fifo);
        m_bank.m_vlsu2_rd_in(m_vlsu2_rd_fe2bank_fifo);
        m_bank.m_vlsu3_rd_in(m_vlsu3_rd_fe2bank_fifo);
        m_bank.m_niu_wr_rd_in(m_niu_wr_rd_fe2bank_fifo);

        // FE -> AS_PIPE
        m_dma_fe.m_dma_rd_desc_out(m_dma_rd_desc_fe2as_fifo);
        m_dma_fe.m_dma_sem_out(m_dma_sem_fe2as_fifo);
        m_tc_fe.m_tc_sem_out(m_tc_sem_fe2as_fifo);
        m_rvv_fe.m_vlsu0_sem_out(m_vlsu0_sem_fe2as_fifo);
        m_rvv_fe.m_vlsu1_sem_out(m_vlsu1_sem_fe2as_fifo);
        m_rvv_fe.m_vlsu2_sem_out(m_vlsu2_sem_fe2as_fifo);
        m_rvv_fe.m_vlsu3_sem_out(m_vlsu3_sem_fe2as_fifo);
        m_niu_fe.m_niu_rvc_wr_rd_out(m_niu_rvc_wr_rd_fe2as_fifo);
        m_rvc_fe.m_rvc_wr_rd_out(m_rvc_wr_rd_fe2as_fifo);
        m_rvc_fe.m_rvc_acc_out(m_rvc_acc_fe2as_fifo);
        m_rvc_fe.m_rvc_barrier_cfi_out(m_rvc_barrier_cfi_fe2as_fifo);
        m_cp_fe.m_cp_wr_out(m_cp_fe2as_fifo);

        m_as_pipe.m_dma_rd_desc_in(m_dma_rd_desc_fe2as_fifo);
        m_as_pipe.m_dma_sem_in(m_dma_sem_fe2as_fifo);
        m_as_pipe.m_tc_sem_in(m_tc_sem_fe2as_fifo);
        m_as_pipe.m_vlsu0_sem_in(m_vlsu0_sem_fe2as_fifo);
        m_as_pipe.m_vlsu1_sem_in(m_vlsu1_sem_fe2as_fifo);
        m_as_pipe.m_vlsu2_sem_in(m_vlsu2_sem_fe2as_fifo);
        m_as_pipe.m_vlsu3_sem_in(m_vlsu3_sem_fe2as_fifo);
        m_as_pipe.m_niu_rvc_wr_rd_in(m_niu_rvc_wr_rd_fe2as_fifo);
        m_as_pipe.m_rvc_wr_rd_in(m_rvc_wr_rd_fe2as_fifo);
        m_as_pipe.m_rvc_acc_in(m_rvc_acc_fe2as_fifo);
        m_as_pipe.m_rvc_barrier_cfi_in(m_rvc_barrier_cfi_fe2as_fifo);
        m_as_pipe.m_cp_wr_in(m_cp_fe2as_fifo);

        // BANK -> BE
        m_bank.m_dma_wr0_out(m_dma_wr0_bank2be_fifo);
        m_bank.m_dma_wr1_out(m_dma_wr1_bank2be_fifo);
        m_bank.m_dma_rd_data_out(m_dma_rd_data_bank2be_fifo);
        m_bank.m_tc_wr_out(m_tc_wr_bank2be_fifo);
        m_bank.m_tc_b_rd0_out(m_tc_b_rd0_bank2be_fifo);
        m_bank.m_tc_b_rd1_out(m_tc_b_rd1_bank2be_fifo);
        m_bank.m_tc_mix_rd_out(m_tc_mix_rd_bank2be_fifo);
        m_bank.m_vlsu_wr_out(m_vlsu_wr_bank2be_fifo);
        m_bank.m_vlsu_rd_out(m_vlsu_rd_bank2be_fifo);
        m_bank.m_niu_wr_rd_out(m_niu_wr_rd_bank2be_fifo);

        m_dma_be.m_dma_wr0_in(m_dma_wr0_bank2be_fifo);
        m_dma_be.m_dma_wr1_in(m_dma_wr1_bank2be_fifo);
        m_dma_be.m_dma_rd_data_in(m_dma_rd_data_bank2be_fifo);

        m_tc_be.m_tc_wr_in(m_tc_wr_bank2be_fifo);
        m_tc_be.m_tc_b_rd0_in(m_tc_b_rd0_bank2be_fifo);
        m_tc_be.m_tc_b_rd1_in(m_tc_b_rd1_bank2be_fifo);
        m_tc_be.m_tc_mix_rd_in(m_tc_mix_rd_bank2be_fifo);

        m_rvv_be.m_vlsu_wr_in(m_vlsu_wr_bank2be_fifo);
        m_rvv_be.m_vlsu_rd_in(m_vlsu_rd_bank2be_fifo);

        m_niu_be.m_bank_niu_wr_rd_in(m_niu_wr_rd_bank2be_fifo);

        // AS_PIPE <-> BANK
        m_as_pipe.m_as_wr_rd_out(m_as_wr_rd_as2bank_fifo);
        m_bank.m_as_wr_rd_in(m_as_wr_rd_as2bank_fifo);

        m_bank.m_as_rd_out(m_as_rd_bank2as_fifo);
        m_as_pipe.m_bank_wr_in(m_as_rd_bank2as_fifo);

        // AS_PIPE -> BE
        m_as_pipe.m_dma_rd_desc_out(m_dma_rd_desc_as2be_fifo);
        m_as_pipe.m_as_niu_wr_rd_out(m_niu_wr_rd_as2be_fifo);
        m_as_pipe.m_as_rvc_out(m_rvc_as2be_fifo);
        m_as_pipe.m_as_cfi_out(m_cfi_as2be_fifo);
        m_as_pipe.m_as_cp_out(m_cp_as2be_fifo);

        m_dma_be.m_dma_rd_desc_in(m_dma_rd_desc_as2be_fifo);
        m_niu_be.m_as_niu_wr_rd_in(m_niu_wr_rd_as2be_fifo);
        m_rvc_be.m_as_rvc_in(m_rvc_as2be_fifo);
        m_rvc_be.m_as_cfi_in(m_cfi_as2be_fifo);
        m_cp_be.m_as_cp_in(m_cp_as2be_fifo);
    }
};

}