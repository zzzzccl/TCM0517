#include "pim_tcm.h"

namespace pim_tcm
{

void DMA_FE::ProcessDMAWr0(void)
{
    IF_GEN::if_dma_tcm_wr_req req_in = {};

    while(true)
    {
        while(!m_dma_tcm_wr0_req_if.num_available())
        {
            wait();
        }

        m_dma_tcm_wr0_req_if.nb_read(req_in);
        Sideband sb;
        Payload pl;
        Request req_out;
        if (req_in.op == 2){
            sb.addr = TCMAddress(req_in.write_sem_union.sem.addr);
            sb.sem_num = req_in.write_sem_union.sem.sem_num;
        }
        else{
            sb.addr = TCMAddress(req_in.write_sem_union.write.addr);
            sb.instr_id = req_in.write_sem_union.write.instr_id;
            sb.instr_last = req_in.write_sem_union.write.instr_last;
            sb.resp_type = req_in.op;

            for(uint32_t u = 0; u < 32; u++)
            {
                pl.data[u] = req_in.write_sem_union.write.data[u];
            }

            for(uint32_t u = 0; u < 4; u++)
            {
                pl.mask[u] = req_in.write_sem_union.write.mask[u];
            }

            uint32_t lane_idx = sb.addr.lane_index;
            req_out.mask[lane_idx] = 1;
        }

        req_out.sideband = sb;
        req_out.payload = pl;



        m_dma_wr0_out.write(req_out);
    }
}

void DMA_FE::ProcessDMAWr1(void)
{
    IF_GEN::if_dma_tcm_wr_req req_in = {};

    while(true)
    {
        while(!m_dma_tcm_wr1_req_if.num_available())
        {
            wait();
        }

        m_dma_tcm_wr1_req_if.nb_read(req_in);
        Sideband sb;
        Payload pl;
        Request req_out;
        if (req_in.op == 2){
            sb.addr = TCMAddress(req_in.write_sem_union.sem.addr);
            sb.sem_num = req_in.write_sem_union.sem.sem_num;
        }
        else{
            sb.addr = TCMAddress(req_in.write_sem_union.write.addr);
            sb.instr_id = req_in.write_sem_union.instr_id;
            sb.instr_last = req_in.write_sem_union.write.instr_last;
            sb.resp_type = req_in.op;

            for(uint32_t u = 0; u < 32; u++)
            {
                pl.data[u] = req_in.write_sem_union.write.data[u];
            }

            for(uint32_t u = 0; u < 4; u++)
            {
                pl.mask[u] = req_in.write_sem_union.write.mask[u];
            }

            uint32_t lane_idx = sb.addr.lane_index;
            req_out.mask[lane_idx] = 1;
        }

        req_out.sideband = sb;
        req_out.payload = pl;



        m_dma_wr1_out.write(req_out);
    }
}

void DMA_FE::ProcessDMARdData(void)
{
    IF_GEN::if_dma_tcm_rd_data_req req_in = {};

    while(true)
    {
        while(!m_dma_tcm_rd_data_req_if.num_available())
        {
            wait();
        }

        m_dma_tcm_rd_data_req_if.nb_read(req_in);
        Sideband sb;
        Payload pl;
        Request req_out;
        sb.addr = TCMAddress(req_in.addr);
        sb.req_id = req_in.req_id;
        sb.burst_len = req_in.burst_len;

        req_out.sideband = sb;
        uint8_t lane_idx = sb.addr.lane_index;
        for (uint32_t u = 0; u <= sb.burst_len; u++){
            req_out.mask[lane_idx + u] = 1;
        }

        m_dma_wr0_out.write(req_out);
    }
}

void DMA_FE::ProcessDMARdDesc(void)
{
    IF_GEN::if_dma_tcm_rd_desc_req req_in = {};

    while(true)
    {
        while(!m_dma_tcm_rd_desc_req_if.num_available())
        {
            wait();
        }

        m_dma_tcm_rd_desc_req_if.nb_read(req_in);
        Sideband sb;
        Payload pl;
        Request req_out;
        sb.addr = TCMAddress(req_in.addr);
        sb.cl_id = req_in.cl_id;
        sb.burst_len = 3;
        for (uint32_t u = 0; u <= sb.burst_len; u++){
            sb.blk_id = u;
            req_out.sideband = sb;
            m_dma_wr0_out.write(req_out);
        }
    }
}

void TC_FE::ProcessTCWr(void)
{
    IF_GEN::if_tc_tcm_wr req_in = {};

    while(true)
    {
        while(!m_tc_tcm_wr_if.num_available())
        {
            wait();
        }

        m_tc_tcm_wr_if.nb_read(req_in);
        Sideband sb;
        Payload pl;
        Request req_out;
        if (req_in.op == 2){
            sb.addr = TCMAddress(req_in.write_sem_union.sem.addr);
            sb.sem_num = req_in.write_sem_union.sem.sem_num;
            sb.instr_last = req_in.write_sem_union.sem.instr_last;
        }
        else if (req_in.op == 1){
            sb.addr = TCMAddress(req_in.write_sem_union.write.addr);
            sb.op = req_in.op;
            sb.rvs_core_id = req_in.rvs_core_id;
            sb.instr_id = req_in.instr_id;

            for(uint32_t u = 0; u < 32; u++)
            {
                pl.data[u] = req_in.write_sem_union.write.data[u];
            }

            for(uint32_t u = 0; u < 4; u++)
            {
                pl.mask[u] = 0xFFFFFFFF;
            }

            uint32_t lane_idx = sb.addr.lane_index;
            req_out.mask[lane_idx] = 1;
        }

        req_out.sideband = sb;
        req_out.payload = pl;

        m_tc_wr_out.write(req_out);
    }
}

void TC_FE::ProcessTCRd0(void)
{
    IF_GEN::if_tc_tcm_b_rd req_in = {};

    while(true)
    {
        while(!m_tc_tcm_b_rd_if.num_available())
        {
            wait();
        }

        m_tc_tcm_b_rd_if.nb_read(req_in);
        Sideband sb;
        Payload pl;
        Request req_out;
        sb.addr = TCMAddress(req_in.addr);
        sb.rvs_core_id = req_in.rvs_core_id;
        sb.instr_id = req_in.instr_id;
        sb.buf_inx = req_in.buf_inx;
        sb.burst_len = req_in.burst_len;

        uint32_t lane_idx = sb.addr.lane_index;
        req_out.mask[lane_idx] = 1;

        req_out.sideband = sb;

        m_tc_rd0_out.write(req_out);
    }
}

void TC_FE::ProcessTCRd1(void)
{
    IF_GEN::if_tc_tcm_ac_sf_rd req_in = {};

    while(true)
    {
        while(!m_tc_tcm_ac_sf_rd_if.num_available())
        {
            wait();
        }

        m_tc_tcm_ac_sf_rd_if.nb_read(req_in);
        Sideband sb;
        Payload pl;
        Request req_out;
        sb.rvs_core_id = req_in.rvs_core_id;
        sb.instr_id = req_in.instr_id;
        sb.buf_inx = req_in.buf_inx;
        sb.matrix_src = req_in.matrix_src;
        if (sb.matrix_src == 0){
            sb.addr = TCMAddress(req_in.SrcInfoUnion.matrix_a.addr);
            sb.grp_mask = req_in.SrcInfoUnion.matrix_a.grp_mask;
        }
        else if (sb.matrix_src == 1){
            sb.addr = TCMAddress(req_in.SrcInfoUnion.matrix_c.addr);
            sb.grp_mask = req_in.SrcInfoUnion.matrix_c.grp_mask;
        }
        else if (sb.matrix_src == 2){
            sb.addr = TCMAddress(req_in.SrcInfoUnion.matrix_sa.addr);
            sb.grp_mask = req_in.SrcInfoUnion.matrix_sa.grp_mask;
        }
        else if (sb.matrix_src == 3){
            sb.addr = TCMAddress(req_in.SrcInfoUnion.matrix_sb.addr);
            sb.grp_mask = req_in.SrcInfoUnion.matrix_sb.grp_mask;
        }

        uint32_t lane_idx = sb.addr.lane_index;
        req_out.mask[lane_idx] = 1;

        req_out.sideband = sb;

        m_tc_rd1_out.write(req_out);
    }
}

void RVV_FE::ProcessVLSUWr(void)
{
    IF_GEN::if_vlsu_tcm_wr req_in = {};

    while(true)
    {
        while(!m_vlsu0_tcm_wr_if.num_available() && !m_vlsu1_tcm_wr_if.num_available() && !m_vlsu2_tcm_wr_if.num_available() && !m_vlsu3_tcm_wr_if.num_available())
        {
            wait();
        }
        uint8_t src = -1;

        if (m_vlsu0_tcm_wr_if.num_available()){
            m_vlsu0_tcm_wr_if.nb_read(req_in);
            src = 0;
        }
        else if (m_vlsu1_tcm_wr_if.num_available()){
            m_vlsu1_tcm_wr_if.nb_read(req_in);
            src = 1;
        }
        else if (m_vlsu2_tcm_wr_if.num_available()){
            m_vlsu2_tcm_wr_if.nb_read(req_in);
            src = 2;
        }
        else if (m_vlsu3_tcm_wr_if.num_available()){
            m_vlsu3_tcm_wr_if.nb_read(req_in);
            src = 3;
        }

        Sideband sb;
        Payload pl;
        Request req_out;
        sb.op = req_in.op;
        if (req_in.op == 2){
            req_out.sideband = sb;
            if (req_in.wr_union.vmem_fence.sem_post_en == 1){
                sb.addr = TCMAddress(req_in.wr_union.vmem_fence.addr);
                req_out.sideband = sb;
                switch (src) {
                    case 0:
                        m_vlsu0_sem_out.write(req_out);
                        break;
                    case 1:
                        m_vlsu1_sem_out.write(req_out);
                        break;
                    case 2:
                        m_vlsu2_sem_out.write(req_out);
                        break;
                    case 3:
                        m_vlsu3_sem_out.write(req_out);
                        break;
                    default:
                        break;
                }
            }
            switch (src) {
                case 0:
                    m_vlsu0_wr_out.write(req_out);
                    break;
                case 1:
                    m_vlsu0_wr_out.write(req_out);
                    break;
                case 2:
                    m_vlsu0_wr_out.write(req_out);
                    break;
                case 3:
                    m_vlsu0_wr_out.write(req_out);
                    break;
                default:
                    break;
            }
        }
        else if (req_in.op == 1){
            if (m_wr_burst_buf[src].beat_len == 0){
                uint32_t data_begin = 0;
                uint32_t data_end = 511;
                for (uint32_t ii = 1; ii < 512; ii++){
                    if (req_in.wr_union.write.strb[ii-1] == 0 && req_in.wr_union.write.strb[ii] == 1){
                        data_begin = ii;
                    }
                    else if (req_in.wr_union.write.strb[ii-1] == 1 && req_in.wr_union.write.strb[ii] == 0){
                        data_end = ii;
                    }
                }
                m_wr_burst_buf[src].beat_len = data_end - data_begin + 1;
                m_wr_burst_buf[src].expect_len = req_in.wr_union.write.burst_len + 1;
            }
            TCMAddress tmp_addr = req_in.wr_union.write.addr;
            uint32_t tmp_addr_in_lane = tmp_addr.get_addr_in_lane;
            for (uint32_t ii = tmp_addr_in_lane; ii < tmp_addr_in_lane + m_wr_burst_buf[src].beat_len; ii++){
                m_wr_burst_buf[src].req.payload.mask[ii] = 1;
                for (uint32_t jj = 0; jj < 8; jj++){
                    m_wr_burst_buf[src].req.payload.data[ii * 8 + jj] = req_in.wr_union.write.data[(ii - tmp_addr_in_lane) * 8 + jj];
                }
            }
            m_wr_burst_buf[src].recv_len++;

            if (req_in.write.last == 1){
                req_out.payload = m_wr_burst_buf[src].req.payload;
                m_wr_burst_buf[src].beat_len = 0;
                m_wr_burst_buf[src].expect_len = 0;
                m_wr_burst_buf[src].recv_len = 0;
                m_wr_burst_buf[src].req = {};

                tmp_addr.bank_index = 0;
                tmp_addr.byte_offset = 0;
                tmp_addr.update_raw_addr();
                sb.addr = tmp_addr;
                sb.instr_last = 1;
                req_out.sideband = sb;

                uint32_t lane_idx = sb.addr.lane_index;
                req_out.mask[lane_idx] = 1;

                switch (src) {
                    case 0:
                        m_vlsu0_wr_out.write(req_out);
                        break;
                    case 1:
                        m_vlsu0_wr_out.write(req_out);
                        break;
                    case 2:
                        m_vlsu0_wr_out.write(req_out);
                        break;
                    case 3:
                        m_vlsu0_wr_out.write(req_out);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void RVV_FE::ProcessVLSURd(void)
{
    IF_GEN::if_vlsu_tcm_rd req_in = {};

    while(true)
    {
        while(!m_vlsu0_tcm_rd_if.num_available() && !m_vlsu1_tcm_rd_if.num_available() && !m_vlsu2_tcm_rd_if.num_available() && !m_vlsu3_tcm_rd_if.num_available())
        {
            wait();
        }
        uint8_t src = -1;

        if (m_vlsu0_tcm_rd_if.num_available()){
            m_vlsu0_tcm_rd_if.nb_read(req_in);
            src = 0;
        }
        else if (m_vlsu1_tcm_rd_if.num_available()){
            m_vlsu1_tcm_rd_if.nb_read(req_in);
            src = 1;
        }
        else if (m_vlsu2_tcm_rd_if.num_available()){
            m_vlsu2_tcm_rd_if.nb_read(req_in);
            src = 2;
        }
        else if (m_vlsu3_tcm_rd_if.num_available()){
            m_vlsu3_tcm_rd_if.nb_read(req_in);
            src = 3;
        }

        Sideband sb;
        Payload pl;
        Request req_out;
        if (m_rd_burst_buf[src].beat_len == 0){
            m_rd_burst_buf[src].beat_len = 512;
            m_rd_burst_buf[src].expect_len = req_in.burst_len + 1;
        }
        TCMAddress tmp_addr = req_in.addr;
        for (uint32_t ii = m_rd_burst_buf[src].recv_len * m_rd_burst_buf[src].beat_len; ii < (m_rd_burst_buf[src].recv_len + 1) * m_rd_burst_buf[src].beat_len; ii++){
            m_rd_burst_buf[src].req.payload.mask[ii] = 1;
            for (uint32_t jj = 0; jj < 8; jj++){
                m_rd_burst_buf[src].req.payload.data[ii * 8 + jj] = req_in.wr_union.write.data[(ii - m_rd_burst_buf[src].recv_len * m_rd_burst_buf[src].beat_len) * 8 + jj];
            }
        }
        m_rd_burst_buf[src].recv_len++;

        if (m_rd_burst_buf[src].expect_len == m_rd_burst_buf[src].recv_len){
            req_out.payload = m_rd_burst_buf[src].req.payload;
            m_rd_burst_buf[src].beat_len = 0;
            m_rd_burst_buf[src].expect_len = 0;
            m_rd_burst_buf[src].recv_len = 0;
            m_rd_burst_buf[src].req = {};

            tmp_addr.bank_index = 0;
            tmp_addr.byte_offset = 0;
            tmp_addr.update_raw_addr();
            sb.addr = tmp_addr;
            req_out.sideband = sb;

            uint32_t lane_idx = sb.addr.lane_index;
            req_out.mask[lane_idx] = 1;

            switch (src) {
                case 0:
                    m_vlsu0_rd_out.write(req_out);
                    break;
                case 1:
                    m_vlsu1_rd_out.write(req_out);
                    break;
                case 2:
                    m_vlsu2_rd_out.write(req_out);
                    break;
                case 3:
                    m_vlsu3_rd_out.write(req_out);
                    break;
                default:
                    break;
            }
        }
    }
}

}