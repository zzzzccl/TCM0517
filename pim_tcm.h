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

namespace pim_tcm
{
const uint32_t NUM_DMA_TCM_WR_IF = 2;
const uint32_t NUM_DMA_NOC_IF = 2;
const uint32_t RVV_NUM = 4;

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
    MASTER_RVV  = 2,
    MASTER_NIU  = 3,
    MASTER_RVC  = 4,
    MASTER_CP   = 5
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
    uint32_t raw;
    uint32_t byte_offset : 5;   // [4:0]   32 bytes in bank, 256 bits
    uint32_t bank_index   : 2;  // [6:5]   bank in lane
    uint32_t lane_index   : 3;  // [9:7]   lane index
    uint32_t row_index    : 10; // [19:10] row index
    
    TCMAddress(uint32_t addr = 0) : raw(addr) {
        byte_offset = (addr >> 0)  & 0x1F;
        bank_index  = (addr >> 5)  & 0x3;
        lane_index  = (addr >> 7)  & 0x7;
        row_index   = (addr >> 10) & 0x3FF;
    }
    
    uint32_t get_cache_tag() const {
        return (raw >> 7) & 0x1FFF;
    }
    
    uint32_t update_raw_addr() const {
        raw = byte_offset + (bank_index << 5) + (lane_index << 7) + (row_index << 10);
        return raw;
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
    uint32_t data_len; // bits
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
    //others
    uint8_t instr_last;
    uint8_t instr_id;
    uint8_t rvs_id;
    uint8_t resp_type;   // 0=RVS load resp, 1=CP load resp 对应op字段
    uint8_t blk_id;
    uint8_t req_id;
    uint8_t cl_id;

    uint8_t rvs_core_id;
    uint8_t buf_inx;
    uint8_t matrix_src;
    uint8_t grp_mask;
    uint8_t op;

    uint8_t id;
    uint8_t user;
    uint8_t last;

    uint8_t trans_id;

    uint8_t tag_sb; 
};

// ============================================================================
// Payload 结构
// ============================================================================
struct Payload {
    uint32_t  data[32];  // max 1024 bits = 128 bytes
    uint32_t  mask[4];  // byte mask
};

// ============================================================================
// 请求结构
// ============================================================================
struct Request {
    Sideband sideband;
    Payload  payload;
    uint8_t  mask[8];
};

class DMA_FE : public sc_module
{
    SC_HAS_PROCESS(DMA_FE);

private:
    struct SemBuf {
        bool valid0 = false;
        bool valid1 = false;
    };
    SemBuf sem_buf;
    // interface between DMA_FE and DMA
    rgx_fifo_interface<IF_GEN::if_dma_tcm_wr_req>  m_dma_tcm_wr0_req_if;
    rgx_fifo_interface<IF_GEN::if_dma_tcm_wr_req>  m_dma_tcm_wr1_req_if;
    rgx_fifo_interface<IF_GEN::if_dma_tcm_rd_data_req>  m_dma_tcm_rd_data_req_if;
    rgx_fifo_interface<IF_GEN::if_dma_tcm_rd_desc_req>  m_dma_tcm_rd_desc_req_if;
    // rgx_port<IF_GEN::if_tcm_dma_bresp>           m_tcm_dma_bresp_if;
    // rgx_port<IF_GEN::if_tcm_dma_rd_data_rtn>           m_tcm_dma_rd_data_rtn_if;
    // rgx_port<IF_GEN::if_tcm_dma_rd_desc_rtn>           m_tcm_dma_rd_desc_rtn_if;

    // communication fifo between CP_FE and Copy Unit
    sc_fifo_out<Request>  m_dma_wr0_out;
    sc_fifo_out<Request>  m_dma_wr1_out;
    sc_fifo_out<Request>  m_dma_rd_data_out;
    sc_fifo_out<Request>  m_dma_rd_desc_out(4);
    sc_fifo_out<Request>  m_dma_sem_out;
    // sc_fifo_in<uint8_t>       m_cp_resp_fifo_in;

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
    SC_HAS_PROCESS(TC_FE);

private:
    // interface between TC_FE and TC
    rgx_fifo_interface<IF_GEN::if_tc_tcm_wr>  m_tc_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_tc_tcm_b_rd>  m_tc_tcm_b_rd_if;
    rgx_fifo_interface<IF_GEN::if_tc_tcm_ac_sf_rd>  m_tc_tcm_ac_sf_rd_if;
    // rgx_port<IF_GEN::if_tc_tcm_bresp>           m_tc_tcm_bresp_if;
    // rgx_port<IF_GEN::if_tc_tcm_b_rtn>           m_tc_tcm_b_rtn_if;
    // rgx_port<IF_GEN::if_tc_tcm_ac_sf_rtn>           m_tc_tcm_ac_sf_rtn_if;

    // communication fifo between CP_FE and Copy Unit
    sc_fifo_out<Request>  m_tc_wr_out;
    sc_fifo_out<Request>  m_tc_rd0_out;
    sc_fifo_out<Request>  m_tc_rd1_out;
    sc_fifo_out<Request>  m_tc_sem_out;
    // sc_fifo_in<uint8_t>       m_cp_resp_fifo_in;

    void ProcessTCWr(void);
    void ProcessTCRd0(void);
    void ProcessTCRd1(void);

public:
    TC_FE(sc_module_name module_name)
    : sc_module(module_name)
    , m_tc_tcm_wr_if("tc_tcm_wr_if")
    , m_tc_tcm_b_rd_if("tc_tcm_b_rd_if")
    , m_tc_tcm_ac_sf_rd_if("tc_tcm_ac_sf_rd_if")
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
    SC_HAS_PROCESS(RVV_FE);

private:
    struct BurstBuf {
        uint32_t beat_len = 0;
        uint32_t expect_len = 0;
        uint32_t recv_len = 0;
        Request  req = {};
    };

    BurstBuf m_wr_burst_buf[RVV_NUM];
    BurstBuf m_rd_burst_buf[RVV_NUM];

    // interface between RVV_FE and RVV
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_wr>  m_vlsu0_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_wr>  m_vlsu1_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_wr>  m_vlsu2_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_wr>  m_vlsu3_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_rd>  m_vlsu0_tcm_rd_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_rd>  m_vlsu1_tcm_rd_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_rd>  m_vlsu2_tcm_rd_if;
    rgx_fifo_interface<IF_GEN::if_vlsu_tcm_rd>  m_vlsu3_tcm_rd_if;
    // rgx_port<IF_GEN::if_vlsu_tcm_bresp>           m_vlsu0_tcm_bresp_if;
    // rgx_port<IF_GEN::if_vlsu_tcm_bresp>           m_vlsu1_tcm_bresp_if;
    // rgx_port<IF_GEN::if_vlsu_tcm_bresp>           m_vlsu2_tcm_bresp_if;
    // rgx_port<IF_GEN::if_vlsu_tcm_bresp>           m_vlsu3_tcm_bresp_if;
    // rgx_port<IF_GEN::if_vlsu_tcm_rtn>           m_vlsu0_tcm_rtn_if;
    // rgx_port<IF_GEN::if_vlsu_tcm_rtn>           m_vlsu1_tcm_rtn_if;
    // rgx_port<IF_GEN::if_vlsu_tcm_rtn>           m_vlsu2_tcm_rtn_if;
    // rgx_port<IF_GEN::if_vlsu_tcm_rtn>           m_vlsu3_tcm_rtn_if;

    // communication fifo between CP_FE and Copy Unit
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

    void ProcessVLSUWr(void);
    void ProcessVLSURd(void);

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
    {
        SC_THREAD(ProcessVLSUWr);
        sensitive << m_vlsu0_tcm_wr_if.data_written_event()
                  << m_vlsu1_tcm_wr_if.data_written_event()
                  << m_vlsu2_tcm_wr_if.data_written_event()
                  << m_vlsu3_tcm_wr_if.data_written_event();

        SC_THREAD(ProcessVLSURd);
        sensitive << m_vlsu0_tcm_rd_if.data_written_event()
                  << m_vlsu1_tcm_rd_if.data_written_event()
                  << m_vlsu2_tcm_rd_if.data_written_event()
                  << m_vlsu3_tcm_rd_if.data_written_event();
    }
};

class NIU_FE : public sc_module
{
    SC_HAS_PROCESS(NIU_FE);

private:
    // interface between NIU_FE and NIU
    rgx_fifo_interface<IF_GEN::if_niu_tcm_wr>     m_niu_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_niu_tcm_wrdata> m_niu_tcm_wrdata_if;
    rgx_fifo_interface<IF_GEN::if_niu_tcm_rd>     m_niu_tcm_rd_if;
    // rgx_port<IF_GEN::if_niu_tcm_wresp>           m_niu_tcm_wresp_if;
    // rgx_port<IF_GEN::if_niu_tcm_rtn>           m_niu_tcm_rtn_if;
    // rgx_port<IF_GEN::if_tcm_niu_remt_sem>           m_tcm_niu_remt_sem_if;


    // communication fifo between CP_FE and Copy Unit
    sc_fifo<Request> m_wr_fifo(64);
    sc_fifo<Request> m_rd_fifo(64);
    sc_fifo_out<Request>  m_niu_wr_rd_out(64);
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
    {
        SC_THREAD(ProcessNIUWr);
        sensitive << m_niu_tcm_wr_if.data_written_event()
                  << m_niu_tcm_wrdata_if.data_written_event();
        SC_THREAD(ProcessNIURd);
        sensitive << m_niu_tcm_rd_if.data_written_event();
        SC_THREAD(ProcessNIURoundRobin); 
    }
};

class RVC_FE : public sc_module
{
    SC_HAS_PROCESS(RVC_FE);

private:
    // interface between RVC_FE and RVC
    rgx_fifo_interface<IF_GEN::if_rvs_tcm_aw>      m_rvs_tcm_aw_if;
    rgx_fifo_interface<IF_GEN::if_rvs_tcm_w>       m_rvs_tcm_w_if;
    rgx_fifo_interface<IF_GEN::if_rvs_tcm_ar>      m_rvs_tcm_ar_if;
    rgx_fifo_interface<IF_GEN::if_rvs_tcm_kick>    m_rvs_tcm_kick_if;
    rgx_fifo_interface<IF_GEN::if_acc_req>         m_acc_req_if;
    rgx_fifo_interface<IF_GEN::if_rvs_tcm_cfi_req> m_rvs_tcm_cfi_req_if;
    // rgx_port<IF_GEN::if_rvs_tcm_b>           m_rvs_tcm_b_if;
    // rgx_port<IF_GEN::if_rvs_tcm_r>           m_rvs_tcm_r_if;
    // rgx_port<IF_GEN::if_acc_resp>           m_acc_resp_if;
    // rgx_port<IF_GEN::if_rvs_tcm_cfi_ack>           m_rvs_tcm_cfi_ack_if;


    // communication fifo between CP_FE and Copy Unit
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
    }
};

class CP_FE : public sc_module
{
    SC_HAS_PROCESS(CP_FE);

private:
    // interface between CP_FE and CP
    rgx_fifo_interface<IF_GEN::if_bif_cmd>   m_bif_cmd_if;
    rgx_fifo_interface<IF_GEN::if_bif_write> m_bif_write_if;
    // rgx_port<IF_GEN::if_bif_rtn>           m_bif_rtn_if;

    // communication fifo between CP_FE and Copy Unit
    sc_fifo_out<Request>  m_cp_wr_out;
    
    void ProcessCPWrRd(void);

public:
    CP_FE(sc_module_name module_name)
    : sc_module(module_name)
    , m_bif_cmd_if("bif_cmd_if")
    , m_bif_write_if("bif_write_if")
    {
        SC_THREAD(ProcessRVCWrRd);
        sensitive << m_bif_cmd_if.data_written_event()
                  << m_bif_write_if.data_written_event();
    }
};

class BANK : public sc_module
{
    SC_HAS_PROCESS(BANK);

private:
    struct InterfaceBuf {
        bool valid = false;
        Request  req = {};
    };
    struct BankBuf{
        uint32_t ValidBufNum = 0;
        InterfaceBuf ifbuf[16];
    }
    enum InterfaceType {
        DMA_W0   = 0x0,
        DMA_W1   = 0x1,
        TC_B_R   = 0x2,
        TC_MIX_R = 0x3,
        DMA_R    = 0x4
        AS_WR    = 0x5
        TC_W     = 0x6
        RVV0_R   = 0x7
        RVV1_R   = 0x8
        RVV2_R   = 0x9
        RVV3_R   = 0xA
        RVV0_W   = 0xB
        RVV1_W   = 0xC
        RVV2_W   = 0xD
        RVV3_W   = 0xE
        NIU_WR   = 0xF  
    };
    struct RVVNIUARB{
        bool valid = false;
        InterfaceType iftype = 0;
    }
    
    BankBuf buf;
    RVVNIUARB rvv_niu_arb_buf[3];

    uint8_t lanemask[8] = {1,1,1,1,1,1,1,1};

    uint8_t storage[BANK_ROW_NUM][BANK_LANE_NUM][BANK_BANK_NUM][BANK_BYTE_NUM];

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
    void UpdateBank(void);
    void ProcessBuf(void);
    uint8_t LaneCheck(Request req);
    Request BankWrRd(Request req);

public:
    TCM_BANK(sc_module_name module_name)
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

}