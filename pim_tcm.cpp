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
        sb.master_type = MASTER_DMA;
        if (req_in.op == 2){
            if (sem_buf.valid1 == true){
                sem_buf.valid1 = false;
                sb.op_type = OP_SEM_POST;
                sb.addr = TCMAddress(req_in.write_sem_union.sem.addr);
                sb.sem_num = req_in.write_sem_union.sem.sem_num;
                req_out.sideband = sb;
                m_dma_sem_out.write(req_out);
            }
            else{
                sem_buf.valid0 = true;
            }
        }
        else{
            sb.op_type = OP_WRITE;
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
            req_out.sideband = sb;
            req_out.payload = pl;

            m_dma_wr0_out.write(req_out);
        }
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
        sb.master_type = MASTER_DMA;
        if (req_in.op == 2){
            if (sem_buf.valid0 == true){
                sem_buf.valid0 = false;
                sb.op_type = OP_SEM_POST;
                sb.addr = TCMAddress(req_in.write_sem_union.sem.addr);
                sb.sem_num = req_in.write_sem_union.sem.sem_num;
                req_out.sideband = sb;
                m_dma_sem_out.write(req_out);
            }
            else{
                sem_buf.valid1 = true;
            }
        }
        else{
            sb.op_type = OP_WRITE;
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
            req_out.sideband = sb;
            req_out.payload = pl;

            m_dma_wr1_out.write(req_out);
        }
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
        sb.master_type = MASTER_DMA;
        sb.op_type = OP_READ;
        sb.addr = TCMAddress(req_in.addr);
        sb.req_id = req_in.req_id;
        sb.burst_len = req_in.burst_len;
        for(uint32_t u = 0; u < 4; u++)
        {
            pl.mask[u] = 0xFFFFFFFF;
        }

        req_out.payload = pl;
        req_out.sideband = sb;
        uint8_t lane_idx = sb.addr.lane_index;
        for (uint32_t u = 0; u <= sb.burst_len; u++){
            req_out.mask[lane_idx + u] = 1;
        }

        m_dma_rd_data_out.write(req_out);
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
        Request req_out;
        sb.master_type = MASTER_DMA;
        sb.op_type = OP_READ;
        sb.addr = TCMAddress(req_in.addr);
        sb.cl_id = req_in.cl_id;
        sb.burst_len = 3;
        sb.data_len = 32;
        for (uint32_t u = 0; u <= sb.burst_len; u++){
            sb.blk_id = u;
            sb.addr.bank_index = u;
            sb.addr.update_raw_addr();
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
        sb.master_type = MASTER_TC;
        sb.debug_tag = req_in.debug_tag;
        if (req_in.op == 2){
            sb.op_type = OP_SEM_POST;
            sb.addr = TCMAddress(req_in.write_sem_union.sem.addr);
            sb.sem_num = req_in.write_sem_union.sem.sem_num;
            sb.instr_last = req_in.write_sem_union.sem.instr_last;
            req_out.sideband = sb;

            m_tc_sem_out.write(req_out);
        }
        else if (req_in.op == 1){
            sb.op_type = OP_WRITE;
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
            req_out.sideband = sb;
            req_out.payload = pl;

            m_tc_wr_out.write(req_out);
        }
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
        sb.debug_tag = req_in.debug_tag;
        sb.master_type = MASTER_TC;
        sb.op_type = OP_READ;
        sb.addr = TCMAddress(req_in.addr);
        sb.rvs_core_id = req_in.rvs_core_id;
        sb.instr_id = req_in.instr_id;
        sb.buf_inx = req_in.buf_inx;
        sb.burst_len = req_in.burst_len;

        for(uint32_t u = 0; u < 4; u++)
        {
            pl.mask[u] = 0xFFFFFFFF;
        }

        for (uint32_t u = 0; u < 8; u++){
            req_out.mask[u] = 1;
        }
        req_out.sideband = sb;
        req_out.payload = pl;

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
        sb.debug_tag = req_in.debug_tag;
        sb.master_type = MASTER_TC;
        sb.op_type = OP_READ;
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
        uint8_t grp_mask_tmp = sb.grp_mask;
        for(uint32_t u = 0; u < 4; u++)
        {
            if ((grp_mask_tmp >> u) & 0x1 == 1){
                pl.mask[u] = 0xFFFFFFFF;
            }
        }

        uint32_t lane_idx = sb.addr.lane_index;
        req_out.mask[lane_idx] = 1;
        req_out.sideband = sb;
        req_out.payload = pl;

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
        sb.master_type = MASTER_RVV;
        sb.op = req_in.op;
        sb.debug_tag = req_in.debug_tag;
        if (req_in.op == 2){
            if (req_in.wr_union.vmem_fence.sem_post_en == 1){
                sb.op_type = OP_SEM_POST;
                sb.sem_num = 0;
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
            sb.op_type = OP_VMEM_FENCE;
            req_out.sideband = sb;
            switch (src) {
                case 0:
                    m_vlsu0_wr_out.write(req_out);
                    break;
                case 1:
                    m_vlsu1_wr_out.write(req_out);
                    break;
                case 2:
                    m_vlsu2_wr_out.write(req_out);
                    break;
                case 3:
                    m_vlsu3_wr_out.write(req_out);
                    break;
                default:
                    break;
            }
        }
        else if (req_in.op == 1){
            sb.op_type = OP_WRITE;
            if (m_wr_burst_buf[src].beat_len == 0){
                uint32_t beat_len_tmp = 0;
                for (uint32_t ii = 0; ii < 64; ii++){
                    beat_len_tmp += req_in.wr_union.write.strb[ii];
                }
                m_wr_burst_buf[src].beat_len = beat_len_tmp;
                m_wr_burst_buf[src].expect_len = req_in.wr_union.write.burst_len + 1;
            }
            TCMAddress tmp_addr = req_in.wr_union.write.addr;
            uint32_t tmp_addr_in_lane = tmp_addr.get_addr_in_lane();
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
                        m_vlsu1_wr_out.write(req_out);
                        break;
                    case 2:
                        m_vlsu2_wr_out.write(req_out);
                        break;
                    case 3:
                        m_vlsu3_wr_out.write(req_out);
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
        sb.debug_tag = req_in.debug_tag;
        sb.master_type = MASTER_RVV;
        sb.op_type = OP_READ;
        if (m_rd_burst_buf[src].beat_len == 0){
            m_rd_burst_buf[src].beat_len = 64;
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

void NIU_FE::ProcessNIUWr(void)
{
    while (true)
    {
        while (!m_niu_tcm_wr_if.num_available())
            wait();

        IF_GEN::if_niu_tcm_wr req_hdr;
        m_niu_tcm_wr_if.nb_read(req_hdr);

        Sideband sb;
        sb.master_type = MASTER_NIU;
        sb.id = req_hdr.id;
        sb.user = req_hdr.user;
        sb.addr = TCMAddress(req_hdr.addr);

        bool isSemPost = req_hdr.user & 0x1;
        if (isSemPost)
        {
            Request req_out;
            sb.op_type = OP_SEM_POST;
            sb.sem_num = (req_hdr.user >> 1) & 0x3;
            req_out.sideband = sb;

            m_wr_fifo.write(req_out);
        }
        else{
            if (req_hdr.atop == 0){
                uint32_t expect_len = req_hdr.len + 1;
                uint32_t recv_len = 0;
                sb.op_type = OP_WRITE;
                while (recv_len < expect_len)
                {
                    while (!m_niu_tcm_wrdata_if.num_available())
                        wait();

                    IF_GEN::if_niu_tcm_wrdata req_data;
                    m_niu_tcm_wrdata_if.nb_read(req_data);

                    Request req_out;
                    Payload pl;

                    for (uint32_t u = 0; u < 32; u++){
                        pl.data[u] = req_data.data[u];
                    }
                    for (uint32_t u = 0; u < 4; u++){
                        pl.mask[u] = req_data.strb[u];
                    }
                    sb.instr_last = req_data.last;
                    TCMAddress beat_addr(sb.addr.raw + buf.recv_len * 128);
                    
                    if ((req_hdr.user >> 3) == 1){
                        req_out.sideband.data_len = 128;
                    }
                    else{
                        uint32_t lane_idx = beat_addr.lane_index;
                        req_out.mask[lane_idx] = 1;
                    }
                    req_out.payload = pl;
                    req_out.sideband = sb;
                    req_out.sideband.addr = beat_addr;
                    m_wr_fifo.write(req_out);
                    recv_len++;
                }
            }
            else{
                sb.op_type = OP_ATOMIC;
                while (!m_niu_tcm_wrdata_if.num_available())
                    wait();

                IF_GEN::if_niu_tcm_wrdata req_data;
                m_niu_tcm_wrdata_if.nb_read(req_data);

                Request req_out;
                Payload pl;
                sb.atomic_type = req_hdr.atop;
                if (req_hdr.len == 2){
                    sb.data_len = 4;
                }
                else if(req_hdr.len == 3){
                    sb.data_len = 8;
                }
                for (uint32_t u = 0; u < buf.req.sideband.data_len * 8; u++){
                    pl.data[u] = req_data.data[u];
                }
                req_out.payload = pl;
                req_out.sideband = sb;
                m_wr_fifo.write(req_out);
            }
        }
    }
}

void NIU_FE::ProcessNIURd(void)
{
    IF_GEN::if_niu_tcm_rd req_in = {};

    while(true)
    {
        while(!m_niu_tcm_rd_if.num_available())
        {
            wait();
        }

        m_niu_tcm_rd_if.nb_read(req_in);
        Sideband sb;
        Payload pl;
        sb.master_type = MASTER_TC;
        sb.op_type = OP_READ;
        sb.id = req_in.id;
        sb.user = req_in.user;
        sb.data_len = 128;
        for(uint32_t u = 0; u < 4; u++)
        {
            pl.mask[u] = 0xFFFFFFFF;
        }
        uint32_t beat_num = 0;
        while (beat_num <= req_in.len){
            Request req_out;
            sb.addr = TCMAddress(req_in.addr + beat_num * 128);
            uint32_t lane_idx = sb.addr.lane_index;
            req_out.mask[lane_idx] = 1;
            if (beat_num == req_in.len){
                sb.last = 1;
            }
            req_out.sideband = sb;
            req_out.payload = pl;

            m_rd_fifo.write(req_out);
            beat_num++;
        } 
    }
}

void NIU_FE::ProcessNIURoundRobin(void)
{
    int rr = 0; // 0 = WR, 1 = RD

    while (true)
    {
        // 等待至少有一个请求
        while (m_wr_fifo.num_available() == 0 && m_rd_fifo.num_available() == 0)
            wait();

        Request req;
        if (rr == 0 && m_wr_fifo.num_available()) {
            m_wr_fifo.nb_read(req);
            rr = 1;  // 下一次尝试 RD
        }
        else if (rr == 1 && m_rd_fifo.num_available()) {
            m_rd_fifo.nb_read(req);
            rr = 0;  // 下一次尝试 WR
        }
        else if (m_wr_fifo.num_available()) {  // 兜底
            m_wr_fifo.nb_read(req);
        }
        else if (m_rd_fifo.num_available()) {
            m_rd_fifo.nb_read(req);
        }

        // 下发到最终输出
        if (req.sideband.op_type == OP_READ) {
            if ((req.sideband.user >> 3) == 1)
                m_niu_rvc_wr_rd_out.write(req);
            else
                m_niu_wr_rd_out.write(req);
        }
        else if (req.sideband.op_type == OP_WRITE) {
            if ((req.sideband.user >> 3) == 1)
                m_niu_rvc_wr_rd_out.write(req);
            else
                m_niu_wr_rd_out.write(req);
        }
        else{
            m_niu_rvc_wr_rd_out.write(req);
        }
    }
}

void RVC_FE::ProcessRVCWr(void)
{
    while (true)
    {
        while (!m_rvs_tcm_aw_if.num_available())
            wait();

        IF_GEN::if_rvs_tcm_aw req_hdr;
        m_rvs_tcm_aw_if.nb_read(req_hdr);

        Sideband sb;
        Request req_out;
        sb.master_type = MASTER_RVC;
        sb.addr = TCMAddress(req_hdr.addr);
        sb.id = req_hdr.id;
        if (req_hdr.atop == 1){
            sb.op_type = OP_WRITE;
            sb.data_len = 32;
        }
        else{
            sb.op_type = OP_ATOMIC;
            sb.atomic_type = req_hdr.atop;
            if (req_hdr.size == 2){
                sb.data_len = 4;
            }
            else{
                sb.data_len = 8;
            }
        }
        
        while (!m_rvs_tcm_w_if.num_available())
            wait();

        IF_GEN::if_rvs_tcm_w req_data;
        m_rvs_tcm_w_if.nb_read(req_data);

        Payload pl;
        for (uint32_t u = 0; u < 8; u++){
            pl.data[u] = req_data.data[u];
        }
        uint32_t bank_idx = sb.addr.bank_index;
        pl.mask[bank_idx] = req_data.strb;
        sb.instr_last = req_data.last;
        req_out.payload = pl;
        req_out.sideband = sb;
        m_wr_fifo.write(req_out);
    }
}

void RVC_FE::ProcessRVCRd(void)
{
    IF_GEN::if_rvs_tcm_ar req_in = {};

    while (true)
    {
        while (!m_rvs_tcm_ar_if.num_available())
        {
            wait();
        }

        m_rvs_tcm_ar_if.nb_read(req_in);

        Sideband sb;
        Payload pl;
        Request req_out;
        sb.master_type = MASTER_RVC;
        sb.op_type = OP_READ;
        sb.id = req_in.id;
        sb.last = 1;
        sb.data_len = 32;

        req_out.sideband = sb;
        req_out.payload = pl;

        m_rd_fifo.write(req_out);
    }
}

void RVC_FE::ProcessRVCRoundRobin(void)
{
    int rr = 0; // 0 = WR, 1 = RD

    while (true)
    {
        // 等待至少有一个请求
        while (m_wr_fifo.num_available() == 0 && m_rd_fifo.num_available() == 0)
            wait();

        Request req;
        if (rr == 0 && m_wr_fifo.num_available()) {
            m_wr_fifo.nb_read(req);
            rr = 1;  // 下一次尝试 RD
        }
        else if (rr == 1 && m_rd_fifo.num_available()) {
            m_rd_fifo.nb_read(req);
            rr = 0;  // 下一次尝试 WR
        }
        else if (m_wr_fifo.num_available()) {  // 兜底
            m_wr_fifo.nb_read(req);
        }
        else if (m_rd_fifo.num_available()) {
            m_rd_fifo.nb_read(req);
        }
        m_rvc_wr_rd_out.write(req);
    }
}

void RVC_FE::ProcessRVCAcc(void)
{
    IF_GEN::if_acc_req req_in = {};

    while (true)
    {
        while (!m_acc_req_if.num_available())
        {
            wait();
        }

        m_acc_req_if.nb_read(req_in);

        Sideband sb = {};
        Request req_out = {};
        sb.master_type = MASTER_RVC;
        sb.trans_id = req_in.trans_id;
        sb.rvs_id = req_in.rvs_id;
        AccInstrType op_type = req_in.insn & 0xFFF;
        if (op_type == BARRIER){
            sb.op_type = OP_BARRIER_PROC;
            sb.barrier_group = (req_in.insn >> 17) & 0x3;
            req_out.sideband = sb;
            m_rvc_barrier_cfi_out.write(req_out);
        }
        else if (op_type == SEM_INIT){
            sb.op_type = OP_SEM_INIT;
            sb.sem_num = (req_in.insn >> 27) & 0x3;
            sb.expect_value = (req_in.insn >> 17) & 0x3FF;
            sb.addr = TCMAddress(req_in.rs1 & 0x1FFFFF);
            req_out.sideband = sb;
            m_rvc_acc_out.write(req_out);
        }
        else if (op_type == SEM_POST){
            sb.op_type = OP_SEM_POST;
            sb.sem_num = (req_in.insn >> 19) & 0x3;
            sb.addr = TCMAddress(req_in.rs1 & 0xFFFFFFFFFFFFFFFF);
            req_out.sideband = sb;
            m_rvc_acc_out.write(req_out);
        }
        else if (op_type == SEM_WAIT){
            sb.op_type = OP_SEM_WAIT;
            sb.addr = TCMAddress(req_in.rs1 & 0x1FFFFF);
            req_out.sideband = sb;
            m_rvc_acc_out.write(req_out);
        }
        else if (op_type == ACC_CFI){
            sb.op_type = OP_CFI;
            req_out.sideband = sb;
            m_rvc_barrier_cfi_out.write(req_out);
        }
    }
}

void RVC_FE::ProcessRVCKick(void)
{
    IF_GEN::if_rvs_tcm_kick req_in = {};

    while (true)
    {
        while (!m_rvs_tcm_kick_if.num_available())
        {
            wait();
        }

        m_rvs_tcm_kick_if.nb_read(req_in);

        Sideband sb;
        Request req_out;
        sb.master_type = MASTER_RVC;
        sb.op_type = OP_BARRIER_INIT;
        sb.rvs_active_mask = req_in.rvs_active_mask;
        req_out.sideband = sb;

        m_rvc_barrier_cfi_out.write(req_out);
    }
}

void RVC_FE::ProcessRVCCfiReq(void)
{
    IF_GEN::if_rvs_tcm_cfi_req req_in = {};

    while (true)
    {
        while (!m_rvs_tcm_cfi_req_if.num_available())
        {
            wait();
        }

        m_rvs_tcm_cfi_req_if.nb_read(req_in);
        if (req_in.valid)
        {
            Sideband sb;
            Request req_out;
            sb.master_type = MASTER_RVC;
            sb.op_type = OP_CFI;
            req_out.sideband = sb;

            m_rvc_barrier_cfi_out.write(req_out);
        }
    }
}

void CP_FE::ProcessCPWrRd(void)
{
    while (true)
    {
        while (!m_bif_cmd_if.num_available())
            wait();

        IF_GEN::if_bif_cmd req_hdr;
        m_bif_cmd_if.nb_read(req_hdr);

        Sideband sb;
        Request req_out;
        sb.master_type = MASTER_CP;
        sb.addr = TCMAddress(req_hdr.address);
        sb.burst_len = req_hdr.burst_length;
        sb.tag_sb = req_hdr.tag_sb;
        sb.data_len = 32;
        if (req_hdr.operation == 1){
            sb.op_type = OP_READ;
            req_out.sideband = sb;
            m_cp_wr_out.write(req_out);
        }
        else{
            sb.op_type = OP_WRITE;
            while (!m_bif_write_if.num_available())
                wait();

            IF_GEN::if_bif_write req_data;
            m_bif_write_if.nb_read(req_data);

            Payload pl;
            for (uint32_t u = 0; u < 8; u++){
                pl.data[u] = req_data.data[u];
            }
            for (uint32_t u = 0; u < 4; u++){
                for (uint32_t i = 0; i < 8; i++){
                    uint32_t val1 = (req_data.mask >> (31 - (u * 8 + i)) & 0x1);
                    uint32_t val2 = val1 + val1 << 1 + val1 << 2 + val1 << 3;
                    pl.mask[u] = pl.mask[u] << 4 + val2;
                }
            }
            req_out.payload = pl;
            req_out.sideband = sb;
            m_cp_wr_out.write(req_out);
        }
    }
}

void BANK::UpdateBuf(void)
{
    if (buf.ifbuf[DMA_W0].valid == false && m_dma_wr0_in.num_available()){
        buf.ifbuf[DMA_W0].valid = true;
        m_dma_wr0_in.nb_read(buf.ifbuf[DMA_W0].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[DMA_W1].valid == false && m_dma_wr1_in.num_available()){
        buf.ifbuf[DMA_W1].valid = true;
        m_dma_wr1_in.nb_read(buf.ifbuf[DMA_W1].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[TC_B_R].valid == false && m_tc_rd0_in.num_available()){
        buf.ifbuf[TC_B_R].valid = true;
        m_tc_rd0_in.nb_read(buf.ifbuf[TC_B_R].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[TC_MIX_R].valid == false && m_tc_rd1_in.num_available()){
        buf.ifbuf[TC_MIX_R].valid = true;
        m_tc_rd1_in.nb_read(buf.ifbuf[TC_MIX_R].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[DMA_R].valid == false && m_dma_rd_data_in.num_available()){
        buf.ifbuf[DMA_R].valid = true;
        m_dma_rd_data_in.nb_read(buf.ifbuf[DMA_R].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[AS_WR].valid == false && m_as_wr_rd_in.num_available()){
        buf.ifbuf[AS_WR].valid = true;
        m_as_wr_rd_in.nb_read(buf.ifbuf[AS_WR].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[TC_W].valid == false && m_tc_wr_in.num_available()){
        buf.ifbuf[TC_W].valid = true;
        m_tc_wr_in.nb_read(buf.ifbuf[TC_W].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[RVV0_R].valid == false && m_vlsu0_rd_in.num_available()){
        buf.ifbuf[RVV0_R].valid = true;
        m_vlsu0_rd_in.nb_read(buf.ifbuf[RVV0_R].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[RVV1_R].valid == false && m_vlsu1_rd_in.num_available()){
        buf.ifbuf[RVV1_R].valid = true;
        m_vlsu1_rd_in.nb_read(buf.ifbuf[RVV1_R].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[RVV2_R].valid == false && m_vlsu2_rd_in.num_available()){
        buf.ifbuf[RVV2_R].valid = true;
        m_vlsu2_rd_in.nb_read(buf.ifbuf[RVV2_R].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[RVV3_R].valid == false && m_vlsu3_rd_in.num_available()){
        buf.ifbuf[RVV3_R].valid = true;
        m_vlsu3_rd_in.nb_read(buf.ifbuf[RVV3_R].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[RVV0_W].valid == false && m_vlsu0_wr_in.num_available()){
        buf.ifbuf[RVV0_W].valid = true;
        m_vlsu0_wr_in.nb_read(buf.ifbuf[RVV0_W].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[RVV1_W].valid == false && m_vlsu1_wr_in.num_available()){
        buf.ifbuf[RVV1_W].valid = true;
        m_vlsu1_wr_in.nb_read(buf.ifbuf[RVV1_W].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[RVV2_W].valid == false && m_vlsu2_wr_in.num_available()){
        buf.ifbuf[RVV2_W].valid = true;
        m_vlsu2_wr_in.nb_read(buf.ifbuf[RVV2_W].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[RVV3_W].valid == false && m_vlsu3_wr_in.num_available()){
        buf.ifbuf[RVV3_W].valid = true;
        m_vlsu3_wr_in.nb_read(buf.ifbuf[RVV3_W].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[NIU_WR].valid == false && m_niu_wr_rd_in.num_available()){
        buf.ifbuf[NIU_WR].valid = true;
        m_niu_wr_rd_in.nb_read(buf.ifbuf[NIU_WR].req);
        buf.ValidBufNum++;
    }
}

void BANK::ProcessBuf(void)
{
    if (buf.ifbuf[DMA_W0].valid){
        uint8_t lane_idx = LaneCheck(buf.ifbuf[DMA_W0].req);
        if (lane_idx != -1){
            Request req_out = BankWrRd(buf.ifbuf[DMA_W0].req);
            m_dma_wr0_out.write(req_out);
            
            buf.ifbuf[DMA_W0].valid = false;
            buf.ifbuf[DMA_W0].req = {};
            buf.ValidBufNum--;
        }
    }
    if (buf.ifbuf[DMA_W1].valid){
        uint8_t lane_idx = LaneCheck(buf.ifbuf[DMA_W1].req);
        if (lane_idx != -1){
            Request req_out = BankWrRd(buf.ifbuf[DMA_W1].req);
            m_dma_wr1_out.write(req_out);
            
            buf.ifbuf[DMA_W1].valid = false;
            buf.ifbuf[DMA_W1].req = {};
            buf.ValidBufNum--;
        }
    }
    if (buf.ifbuf[TC_B_R].valid){
        uint8_t lane_idx = LaneCheck(buf.ifbuf[TC_B_R].req);
        if (lane_idx != -1){
            Request req_in = buf.ifbuf[TC_B_R].req;
            req_in.sideband.addr.lane_index = lane_idx;
            req_in.sideband.addr.update_raw_addr();
            Request req_out = BankWrRd(req_in);
            m_tc_b_rd0_out.write(req_out);

            buf.ifbuf[TC_B_R].req.mask[lane_idx] = 0;
            bool isEmpty = true;
            for (uint32_t u = 0; u < 8; u++){
                if (buf.ifbuf[TC_B_R].req.mask[u] == 1){
                    isEmpty = false;
                }
            }

            if (!isEmpty){
                lane_idx = LaneCheck(buf.ifbuf[TC_B_R].req);
                if (lane_idx != -1){
                    req_in = buf.ifbuf[TC_B_R].req;
                    req_in.sideband.addr.lane_index = lane_idx;
                    req_in.sideband.addr.update_raw_addr();
                    req_out = BankWrRd(req_in);
                    m_tc_b_rd1_out.write(req_out);

                    buf.ifbuf[TC_B_R].req.mask[lane_idx] = 0;
                    isEmpty = true;
                    for (uint32_t u = 0; u < 8; u++){
                        if (buf.ifbuf[TC_B_R].req.mask[u] == 1){
                            isEmpty = false;
                        }
                    }
                }
            }
            if (isEmpty){
                buf.ifbuf[TC_B_R].valid = false;
                buf.ifbuf[TC_B_R].req = {};
                buf.ValidBufNum--;
            }
        }
    }
    if (buf.ifbuf[TC_MIX_R].valid){
        uint8_t lane_idx = LaneCheck(buf.ifbuf[TC_MIX_R].req);
        if (lane_idx != -1){
            Request req_in = buf.ifbuf[TC_MIX_R].req;
            req_in.sideband.addr.lane_index = lane_idx;
            req_in.sideband.addr.update_raw_addr();
            Request req_out = BankWrRd(req_in);
            m_tc_mix_rd_out.write(req_out);

            buf.ifbuf[TC_MIX_R].req.mask[lane_idx] = 0;
            bool isEmpty = true;
            for (uint32_t u = 0; u < 8; u++){
                if (buf.ifbuf[TC_MIX_R].req.mask[u] == 1){
                    isEmpty = false;
                }
            }
            if (isEmpty){
                buf.ifbuf[TC_MIX_R].valid = false;
                buf.ifbuf[TC_MIX_R].req = {};
                buf.ValidBufNum--;
            }
        }
    }
    if (buf.ifbuf[DMA_R].valid){
        uint8_t lane_idx = LaneCheck(buf.ifbuf[DMA_R].req);
        if (lane_idx != -1){
            Request req_in = buf.ifbuf[DMA_R].req;
            req_in.sideband.addr.lane_index = lane_idx;
            req_in.sideband.addr.update_raw_addr();
            Request req_out = BankWrRd(req_in);
            m_dma_rd_data_out.write(req_out);

            buf.ifbuf[DMA_R].req.mask[lane_idx] = 0;
            bool isEmpty = true;
            for (uint32_t u = 0; u < 8; u++){
                if (buf.ifbuf[DMA_R].req.mask[u] == 1){
                    isEmpty = false;
                }
            }
            if (isEmpty){
                buf.ifbuf[DMA_R].valid = false;
                buf.ifbuf[DMA_R].req = {};
                buf.ValidBufNum--;
            }
        }
    }
    if (buf.ifbuf[AS_WR].valid){
        uint8_t lane_idx = LaneCheck(buf.ifbuf[AS_WR].req);
        if (lane_idx != -1){
            Request req_out = BankWrRd(buf.ifbuf[AS_WR].req);
            if (req_out.sideband.op_type == OP_READ){
                m_as_rd_out.write(req_out);
            }
            buf.ifbuf[AS_WR].valid = false;
            buf.ifbuf[AS_WR].req = {};
            buf.ValidBufNum--;
        }
    }
    if (buf.ifbuf[TC_W].valid){
        uint8_t lane_idx = LaneCheck(buf.ifbuf[TC_W].req);
        if (lane_idx != -1){
            Request req_out = BankWrRd(buf.ifbuf[TC_W].req);
            m_tc_wr_out.write(req_out);
            
            buf.ifbuf[TC_W].valid = false;
            buf.ifbuf[TC_W].req = {};
            buf.ValidBufNum--;
        }
    }



    if (buf.ifbuf[RVV0_R].valid || buf.ifbuf[RVV1_R].valid || buf.ifbuf[RVV1_R].valid || buf.ifbuf[RVV1_R].valid){
        for (uint32_t u = 0; u < 4; u++){
            InterfaceType iftype_tmp = RVV0_R + rr_rvv_r;
            rr_rvv_r = (rr_rvv_r + 1) % 4;
            uint8_t lane_idx = LaneCheck(buf.ifbuf[iftype_tmp].req);
            if (lane_idx != -1){
                rvv_niu_arb_buf[0].valid = true;
                rvv_niu_arb_buf[0].iftype = iftype_tmp;
                break;
            }
        }
    }
    
    if (buf.ifbuf[RVV0_W].valid == false && m_vlsu0_wr_in.num_available()){
        buf.ifbuf[RVV0_W].valid = true;
        m_vlsu0_wr_in.nb_read(buf.ifbuf[RVV0_W].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[RVV1_W].valid == false && m_vlsu1_wr_in.num_available()){
        buf.ifbuf[RVV1_W].valid = true;
        m_vlsu1_wr_in.nb_read(buf.ifbuf[RVV1_W].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[RVV2_W].valid == false && m_vlsu2_wr_in.num_available()){
        buf.ifbuf[RVV2_W].valid = true;
        m_vlsu2_wr_in.nb_read(buf.ifbuf[RVV2_W].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[RVV3_W].valid == false && m_vlsu3_wr_in.num_available()){
        buf.ifbuf[RVV3_W].valid = true;
        m_vlsu3_wr_in.nb_read(buf.ifbuf[RVV3_W].req);
        buf.ValidBufNum++;
    }
    if (buf.ifbuf[NIU_WR].valid == false && m_niu_wr_rd_in.num_available()){
        buf.ifbuf[NIU_WR].valid = true;
        m_niu_wr_rd_in.nb_read(buf.ifbuf[NIU_WR].req);
        buf.ValidBufNum++;
    }
}



uint8_t BANK::LaneCheck(Request req)
{

}

Request Bank::BankWrRd(Request req)
{

}

void BANK::ProcessBank(void)
{

    while (true)
    {
        UpdateBuf();
        if (buf.ValidBufNum == 0)
        {
            wait();
        }
        else{
            ProcessBuf();
        }

        m_rvs_tcm_cfi_req_if.nb_read(req_in);
        if (req_in.valid)
        {
            Sideband sb;
            Request req_out;
            sb.master_type = MASTER_RVC;
            sb.op_type = OP_CFI;
            req_out.sideband = sb;

            m_rvc_barrier_cfi_out.write(req_out);
        }
    }
}


}
