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

#include "rgx_modules.h"
#include "sc_fifo_top.h"

#include <queue>
#include <bitset>

namespace pim_tcm
{
const uint32_t NUM_DMA_TCM_WR_IF = 2;
const uint32_t NUM_DMA_NOC_IF = 2;
const uint32_t RVV_NUM = 4;

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
    TCMAddress addr;
    uint8_t burst_len;
    uint8_t sem_num;
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
    sc_fifo_out<Request>  m_dma_sem_out;
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
    SC_HAS_PROCESS(RVV_FE);

private:
    // interface between NIU_FE and NIU
    rgx_fifo_interface<IF_GEN::if_niu_tcm_wr>     m_niu_tcm_wr_if;
    rgx_fifo_interface<IF_GEN::if_niu_tcm_wrdata> m_niu_tcm_wrdata_if;
    rgx_fifo_interface<IF_GEN::if_niu_tcm_rd>     m_niu_tcm_rd_if;
    // rgx_port<IF_GEN::if_niu_tcm_wresp>           m_niu_tcm_wresp_if;
    // rgx_port<IF_GEN::if_niu_tcm_rtn>           m_niu_tcm_rtn_if;
    // rgx_port<IF_GEN::if_tcm_niu_remt_sem>           m_tcm_niu_remt_sem_if;


    // communication fifo between CP_FE and Copy Unit
    sc_fifo_out<Request>  m_wr_out;
    sc_fifo_out<Request>  m_rvc_wr_out;

    void ProcessNIUWr(void);

public:
    RVV_FE(sc_module_name module_name)
    : sc_module(module_name)
    , m_niu_tcm_wr_if("niu_tcm_wr_if")
    , m_niu_tcm_wrdata_if("niu_tcm_wrdata_if")
    , m_niu_tcm_rd_if("niu_tcm_rd_if")
    {
        SC_THREAD(ProcessNIUWr);
        sensitive << m_niu_tcm_wr_if.data_written_event()
                  << m_niu_tcm_wrdata_if.data_written_event()
                  << m_niu_tcm_rd_if.data_written_event()；
    }
};

}