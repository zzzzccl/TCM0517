#include "pim_tcm.h"

namespace pim_tcm
{

inline uint32_t GenByteMask(uint32_t mask4)
{
    uint32_t m = 0;

    if (mask4 & 0x1) m |= 0x000000FF;
    if (mask4 & 0x2) m |= 0x0000FF00;
    if (mask4 & 0x4) m |= 0x00FF0000;
    if (mask4 & 0x8) m |= 0xFF000000;

    return m;
}

void ExpandMask4To32(const uint32_t mask4_arr[4], uint32_t expanded_mask[32])
{
    for (uint32_t i = 0; i < 32; i++) {
        uint32_t mask_idx = i / 8;
        uint32_t sub_idx  = i % 8;

        uint32_t mask4 =
            (mask4_arr[mask_idx] >> (sub_idx * 4)) & 0xF;

        expanded_mask[i] = GenByteMask(mask4);
    }
}

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
        Sideband sb = {};
        Payload pl = {};
        Request req_out = {};
        sb.master_type = MASTER_DMA;
        sb.debug_tag = req_in.debug_tag;
        if (req_in.op == 2){
            if (sem_buf.valid1 == true){
                sem_buf.valid1 = false;
                sb.op_type = OP_SEM_POST;
                sb.addr = TCMAddress(req_in.write_sem_union.sem.addr);
                sb.sem_num = req_in.write_sem_union.sem.sem_num;
                sb.data_len = 64;
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
            sb.burst_len = 0;
            sb.instr_last = req_in.write_sem_union.write.instr_last;
            sb.resp_type = req_in.op;

            for(uint32_t u = 0; u < 32; u++)
            {
                pl.data[u] = req_in.write_sem_union.write.data[u];
            }
            ExpandMask4To32(req_in.write_sem_union.write.mask, pl.mask);

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
        Sideband sb = {};
        Payload pl = {};
        Request req_out = {};
        sb.master_type = MASTER_DMA;
        sb.debug_tag = req_in.debug_tag;
        if (req_in.op == 2){
            if (sem_buf.valid0 == true){
                sem_buf.valid0 = false;
                sb.op_type = OP_SEM_POST;
                sb.addr = TCMAddress(req_in.write_sem_union.sem.addr);
                sb.sem_num = req_in.write_sem_union.sem.sem_num;
                sb.data_len = 64;
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
            sb.burst_len = 0;
            sb.instr_last = req_in.write_sem_union.write.instr_last;
            sb.resp_type = req_in.op;

            for(uint32_t u = 0; u < 32; u++)
            {
                pl.data[u] = req_in.write_sem_union.write.data[u];
            }
            ExpandMask4To32(req_in.write_sem_union.write.mask, pl.mask);

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
        Sideband sb = {};
        Payload pl = {};
        Request req_out = {};
        sb.master_type = MASTER_DMA;
        sb.op_type = OP_READ;
        sb.debug_tag = req_in.debug_tag;
        sb.addr = TCMAddress(req_in.addr);
        sb.req_id = req_in.req_id;
        sb.burst_len = req_in.burst_len;
        for(uint32_t u = 0; u < 32; u++)
        {
            pl.mask[u] = 0xFFFFFFFF;
        }

        req_out.payload = pl;
        req_out.sideband = sb;
        uint8_t lane_idx = sb.addr.lane_index;
        for (uint32_t u = 0; u <= sb.burst_len; u++){
            req_out.mask[(lane_idx + u) % 8] = 1;
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
        Sideband sb = {};
        Request req_out = {};
        sb.master_type = MASTER_DMA;
        sb.op_type = OP_READ;
        sb.addr = TCMAddress(req_in.addr);
        sb.cl_id = req_in.cl_id;
        sb.burst_len = 3;
        sb.data_len = 256;
        for (uint32_t u = 0; u <= sb.burst_len; u++){
            sb.blk_id = u;
            sb.addr.bank_index = u;
            sb.addr.update_raw_addr();
            Payload pl = {};
            for (uint32_t i = 0; i < 8; i++){
                pl.mask[u * 8 + i] = 1;
            }
            req_out.sideband = sb;
            req_out.payload = pl;
            m_dma_rd_desc_out.write(req_out);
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
        Sideband sb = {};
        Payload pl = {};
        Request req_out = {};
        sb.master_type = MASTER_TC;
        sb.debug_tag = req_in.debug_tag;
        sb.op = req_in.op;
        sb.rvs_core_id = req_in.rvs_core_id;
        sb.instr_id = req_in.instr_id;
        if (req_in.op == 2){
            sb.op_type = OP_SEM_POST;
            sb.addr = TCMAddress(req_in.write_sem_union.sem.addr);
            sb.sem_num = req_in.write_sem_union.sem.sem_num;
            sb.instr_last = req_in.write_sem_union.sem.instr_last;
            sb.data_len = 64;
            req_out.sideband = sb;

            m_tc_sem_out.write(req_out);
            m_tc_wr_out.write(req_out);
        }
        else if (req_in.op == 1){
            sb.op_type = OP_WRITE;
            sb.addr = TCMAddress(req_in.write_sem_union.write.addr);
            sb.burst_len = 0;
            sb.instr_last = 1;

            for(uint32_t u = 0; u < 32; u++)
            {
                pl.data[u] = req_in.write_sem_union.write.data[u];
            }

            for(uint32_t u = 0; u < 32; u++)
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
        Sideband sb = {};
        Payload pl = {};
        Request req_out = {};
        sb.debug_tag = req_in.debug_tag;
        sb.master_type = MASTER_TC;
        sb.op_type = OP_READ;
        sb.addr = TCMAddress(req_in.addr);
        sb.rvs_core_id = req_in.rvs_core_id;
        sb.instr_id = req_in.instr_id;
        sb.buf_inx = req_in.buf_inx;
        sb.burst_len = req_in.burst_len;

        for(uint32_t u = 0; u < 32; u++)
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
        Sideband sb = {};
        Payload pl = {};
        Request req_out = {};
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
            sb.burst_len = req_in.SrcInfoUnion.matrix_a.burst_len;
        }
        else if (sb.matrix_src == 1){
            sb.addr = TCMAddress(req_in.SrcInfoUnion.matrix_c.addr);
            sb.grp_mask = req_in.SrcInfoUnion.matrix_c.grp_mask;
            sb.burst_len = req_in.SrcInfoUnion.matrix_c.burst_len;
        }
        else if (sb.matrix_src == 2){
            sb.addr = TCMAddress(req_in.SrcInfoUnion.matrix_sa.addr);
            sb.grp_mask = req_in.SrcInfoUnion.matrix_sa.grp_mask;
            sb.burst_len = req_in.SrcInfoUnion.matrix_sa.burst_len;
        }
        else if (sb.matrix_src == 3){
            sb.addr = TCMAddress(req_in.SrcInfoUnion.matrix_sb.addr);
            sb.grp_mask = req_in.SrcInfoUnion.matrix_sb.grp_mask;
            sb.burst_len = req_in.SrcInfoUnion.matrix_sb.burst_len;
        }
        uint8_t grp_mask_tmp = sb.grp_mask;
        for(uint32_t u = 0; u < 4; u++)
        {
            if (((grp_mask_tmp >> (3 - u)) & 0x1) == 1){
                for (uint32_t i = 0; i < 8; i++){
                    pl.mask[u * 8 + i] = 0xFFFFFFFF;
                }
            }
        }

        uint32_t lane_idx = sb.addr.lane_index;
        for (uint32_t u = 0; u <= sb.burst_len; u++){
            req_out.mask[(lane_idx + u) % 8] = 1;
        }
        req_out.sideband = sb;
        req_out.payload = pl;

        m_tc_rd1_out.write(req_out);
    }
}

void RVV_FE::ProcessVLSUWr(uint32_t src, rgx_fifo_interface<IF_GEN::if_vlsu_tcm_wr>& wr_in_if, sc_fifo_out<Request>& wr_out_if, sc_fifo_out<Request>& sem_out_if)
{
    IF_GEN::if_vlsu_tcm_wr req_in = {};

    while(true)
    {
        while(!wr_in_if.num_available())
        { 
            wait();
        }

        wr_in_if.nb_read(req_in);
        MasterType master_type = MASTER_DMA;
        switch (src){
            case 0:
                master_type = MASTER_RVV0;
                break;
            case 1:
                master_type = MASTER_RVV1;
                break;
            case 2:
                master_type = MASTER_RVV2;
                break;
            case 3:
                master_type = MASTER_RVV3;
                break;
            default:
                break;
        }
        if (req_in.op == 2){
            Sideband sb = {};
            Request req_out = {};
            sb.op = req_in.op;
            sb.master_type = master_type;
            sb.debug_tag = req_in.debug_tag;
            if (req_in.wr_union.vmem_fence.sem_post_en == 1){
                sb.op_type = OP_SEM_POST;
                sb.addr = TCMAddress(req_in.wr_union.vmem_fence.addr);
                sb.sem_num = 0;
                sb.data_len = 64;
                req_out.sideband = sb;
                sem_out_if.write(req_out);
            }
            sb.op_type = OP_VMEM_FENCE;
            req_out.sideband = sb;
            wr_out_if.write(req_out);
        }
        else if (req_in.op == 1){
            if (m_wr_burst_buf[src].valid == false){
                m_wr_burst_buf[src].valid = true;
                m_wr_burst_buf[src].expect_len = req_in.wr_union.write.burst_len + 1;
                m_wr_burst_buf[src].recv_len = 0;

                m_wr_burst_buf[src].req.sideband.op = req_in.op;
                m_wr_burst_buf[src].req.sideband.master_type = master_type;
                m_wr_burst_buf[src].req.sideband.op_type = OP_WRITE;
                m_wr_burst_buf[src].req.sideband.debug_tag = req_in.debug_tag;
                m_wr_burst_buf[src].req.sideband.addr = req_in.wr_union.write.addr;
                m_wr_burst_buf[src].req.sideband.burst_len = 0;
            }
            TCMAddress tmp_addr = req_in.wr_union.write.addr;
            uint32_t bank_idx = tmp_addr.bank_index;
            uint32_t bank_start_idx = bank_idx / 2;
            for (uint32_t u = 0; u < 2; u++){
                m_rd_burst_buf[src].mask[bank_start_idx * 2 + u] += req_in.wr_union.write.strb[u];
                for (uint32_t i = 0; i < 8; i++){
                    m_wr_burst_buf[src].req.payload.data[(bank_start_idx * 2 + u) * 8 + i] += req_in.wr_union.write.data[u * 8 + i];
                }
            }

            m_wr_burst_buf[src].recv_len++;

            if (req_in.wr_union.write.last == 1){
                Request req_out = {};
                m_wr_burst_buf[src].req.sideband.last = 1;
                ExpandMask4To32(m_wr_burst_buf[src].mask, m_wr_burst_buf[src].req.payload.mask);
                req_out = m_wr_burst_buf[src].req;

                m_wr_burst_buf[src].valid = false;
                m_wr_burst_buf[src].expect_len = 0;
                m_wr_burst_buf[src].recv_len = 0;
                m_wr_burst_buf[src].req = {};
                memset(m_wr_burst_buf[src].mask, 0, sizeof(m_wr_burst_buf[src].mask));

                req_out.sideband.addr.bank_index = 0;
                req_out.sideband.addr.byte_offset = 0;
                req_out.sideband.addr.update_raw_addr();

                uint32_t lane_idx = req_out.sideband.addr.lane_index;
                req_out.mask[lane_idx] = 1;

                wr_out_if.write(req_out);
            }
        }
    }
}

void RVV_FE::ProcessVLSU0Wr(void){
    ProcessVLSUWr(0, m_vlsu0_tcm_wr_if, m_vlsu0_wr_out, m_vlsu0_sem_out);
}

void RVV_FE::ProcessVLSU1Wr(void){
    ProcessVLSUWr(1, m_vlsu1_tcm_wr_if, m_vlsu1_wr_out, m_vlsu1_sem_out);
}

void RVV_FE::ProcessVLSU2Wr(void){
    ProcessVLSUWr(2, m_vlsu2_tcm_wr_if, m_vlsu2_wr_out, m_vlsu2_sem_out);
}

void RVV_FE::ProcessVLSU3Wr(void){
    ProcessVLSUWr(3, m_vlsu3_tcm_wr_if, m_vlsu3_wr_out, m_vlsu3_sem_out);
}

void RVV_FE::ProcessVLSURd(uint32_t src, rgx_fifo_interface<IF_GEN::if_vlsu_tcm_rd>& rd_in_if, sc_fifo_out<Request>& rd_out_if)
{
    IF_GEN::if_vlsu_tcm_rd req_in = {};

    while(true)
    {
        while(!rd_in_if.num_available())
        { 
            wait();
        }

        rd_in_if.nb_read(req_in);
        Sideband sb = {};
        Payload pl = {};
        Request req_out = {};
        switch (src){
            case 0:
                sb.master_type = MASTER_RVV0;
                break;
            case 1:
                sb.master_type = MASTER_RVV1;
                break;
            case 2:
                sb.master_type = MASTER_RVV2;
                break;
            case 3:
                sb.master_type = MASTER_RVV3;
                break;
            default:
                break;
        }
        sb.op_type = OP_READ;
        sb.debug_tag = req_in.debug_tag;
        sb.burst_len = req_in.burst_len;
        sb.addr = TCMAddress(req_in.addr);
        if (req_in.burst_len == 1){
            for (uint32_t u = 0; u < 32; u++){
                pl.mask[u] = 0xFFFFFFFF;
            }
        }
        else{
            uint32_t bank_idx = sb.addr.bank_index;
            for (uint32_t u = 0; u < 16; u++){
                pl.mask[bank_idx * 8 + u] = 0xFFFFFFFF;
            }
        }

        req_out.sideband = sb;
        req_out.payload = pl;
        uint32_t lane_idx = sb.addr.lane_index;
        req_out.mask[lane_idx] = 1;

        rd_out_if.write(req_out);
    }
}

void RVV_FE::ProcessVLSU0Rd(void)
{
    ProcessVLSURd(0, m_vlsu0_tcm_rd_if, m_vlsu0_rd_out);
}

void RVV_FE::ProcessVLSU1Rd(void)
{
    ProcessVLSURd(1, m_vlsu1_tcm_rd_if, m_vlsu1_rd_out);
}

void RVV_FE::ProcessVLSU2Rd(void)
{
    ProcessVLSURd(2, m_vlsu2_tcm_rd_if, m_vlsu2_rd_out);
}

void RVV_FE::ProcessVLSU3Rd(void)
{
    ProcessVLSURd(3, m_vlsu3_tcm_rd_if, m_vlsu3_rd_out);
}

void NIU_FE::ProcessNIUWr(void)
{
    while (true)
    {
        while (!m_niu_tcm_wr_if.num_available())
            wait();

        IF_GEN::if_niu_tcm_wr req_hdr;
        m_niu_tcm_wr_if.nb_read(req_hdr);

        Sideband sb = {};
        sb.master_type = MASTER_NIU;
        sb.id = req_hdr.id;
        sb.user = req_hdr.user;
        sb.addr = TCMAddress(req_hdr.addr);

        bool isSemPost = req_hdr.user & 0x1;
        if (isSemPost)
        {
            Request req_out = {};
            sb.op_type = OP_SEM_POST;
            sb.sem_num = (req_hdr.user >> 1) & 0x3;
            sb.data_len = 64;
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

                    Request req_out = {};
                    Payload pl = {};

                    for (uint32_t u = 0; u < 32; u++){
                        pl.data[u] = req_data.data[u];
                    }
                    ExpandMask4To32(req_data.strb, pl.mask);
                    sb.instr_last = req_data.last;
                    TCMAddress beat_addr(sb.addr.raw + recv_len * 128);
                    
                    if ((req_hdr.user >> 3) == 1){
                        sb.data_len = 1024;
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

                Request req_out = {};
                Payload pl = {};
                sb.atomic_type = req_hdr.atop;
                if (req_hdr.len == 2){
                    sb.data_len = 32;
                }
                else if(req_hdr.len == 3){
                    sb.data_len = 64;
                }
                uint32_t bank_idx = sb.addr.bank_index;
                uint32_t byte_offset = sb.addr.byte_offset;
                for (uint32_t u = 0; u < 32; u++){
                    pl.data[u] = req_data.data[u];
                }
                ExpandMask4To32(req_data.strb, pl.mask);

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
        Sideband sb = {};
        Payload pl = {};
        sb.master_type = MASTER_NIU;
        sb.op_type = OP_READ;
        sb.id = req_in.id;
        sb.user = req_in.user;
        sb.data_len = 1024;
        sb.burst_len = req_in.len;
        for(uint32_t u = 0; u < 32; u++)
        {
            pl.mask[u] = 0xFFFFFFFF;
        }

        for (uint32_t beat_num = 0; beat_num<= req_in.len; beat_num++){
            Request req_out = {};
            sb.addr = TCMAddress(req_in.addr + beat_num * 128);
            uint32_t lane_idx = sb.addr.lane_index;
            req_out.mask[lane_idx] = 1;
            if (beat_num == req_in.len){
                sb.last = 1;
            }
            else{
                sb.last = 0;
            }
            req_out.sideband = sb;
            req_out.payload = pl;

            m_rd_fifo.write(req_out);
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

        Sideband sb = {};
        Request req_out = {};
        sb.master_type = MASTER_RVC;
        sb.addr = TCMAddress(req_hdr.addr);
        sb.id = req_hdr.id;
        sb.burst_len = req_hdr.len;
        if (req_hdr.atop == 0){
            sb.op_type = OP_WRITE;
            sb.data_len = 256;
        }
        else{
            sb.op_type = OP_ATOMIC;
            sb.atomic_type = req_hdr.atop;
            if (req_hdr.size == 2){
                sb.data_len = 32;
            }
            else{
                sb.data_len = 64;
            }
        }
        
        while (!m_rvs_tcm_w_if.num_available())
            wait();

        IF_GEN::if_rvs_tcm_w req_data;
        m_rvs_tcm_w_if.nb_read(req_data);

        Payload pl = {};
        uint32_t bank_idx = sb.addr.bank_index;
        for (uint32_t u = 0; u < 8; u++){
            pl.data[bank_idx * 8 + u] = req_data.data[u];
        }
        uint32_t mask[4] = {};
        mask[bank_idx] = req_data.strb;
        ExpandMask4To32(mask, pl.mask);
        
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

        Sideband sb = {};
        Payload pl = {};
        Request req_out = {};
        sb.master_type = MASTER_RVC;
        sb.op_type = OP_READ;
        sb.id = req_in.id;
        sb.last = 1;
        sb.data_len = 256;
        sb.addr = TCMAddress(req_in.addr);
        uint32_t bank_idx = sb.addr.bank_index;
        for (uint32_t u = 0; u < 8; u++){
            pl.mask[bank_idx * 8 + u] = 0xFFFFFFFF;
        }

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
        sb.debug_tag = req_in.debug_tag;
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

        Sideband sb = {};
        Request req_out = {};
        sb.master_type = MASTER_RVC;
        sb.op_type = OP_BARRIER_INIT;
        sb.rvs_active_mask = req_in.rvs_active_mask;
        req_out.sideband = sb;

        m_rvc_barrier_cfi_out.write(req_out);
    }
}

void RVC_FE::ProcessRVCCfi(void)
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
            Sideband sb = {};
            Request req_out = {};
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

        Sideband sb = {};
        Request req_out = {};
        Payload pl = {};
        sb.master_type = MASTER_CP;
        sb.addr = TCMAddress(req_hdr.address);
        sb.burst_len = req_hdr.burst_length;
        sb.tag_sb = req_hdr.tag_sb;
        sb.data_len = 256;
        if (req_hdr.operation == 0){
            sb.op_type = OP_READ;
            uint32_t bank_idx = sb.addr.bank_index;
            for (uint32_t u = 0; u < 8; u++){
                pl.mask[bank_idx * 8 + u] = 0xFFFFFFFF;
            }
            req_out.sideband = sb;
            req_out.payload = pl;
            m_cp_wr_out.write(req_out);
        }
        else{
            sb.op_type = OP_WRITE;
            while (!m_bif_write_if.num_available())
                wait();

            IF_GEN::if_bif_write req_data;
            m_bif_write_if.nb_read(req_data);

            uint32_t bank_idx = sb.addr.bank_index;
            for (uint32_t u = 0; u < 8; u++){
                pl.data[bank_idx * 8 + u] = req_data.data[u];
            }
            uint32_t mask[4] = {};
            mask[bank_idx] = req_data.mask;
            ExpandMask4To32(mask, pl.mask);

            req_out.payload = pl;
            req_out.sideband = sb;
            m_cp_wr_out.write(req_out);
        }
    }
}

void DMA_BE::ProcessDMAWr0(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_dma_wr0_in.num_available())
        {
            wait();
        }

        m_dma_wr0_in.nb_read(req_in);
        IF_GEN::if_tcm_dma_bresp req_out;
        req_out.instr_id = req_in.sideband.instr_id;
        req_out.resp_type = req_in.sideband.resp_type;
        req_out.debug_tag = req_in.sideband.debug_tag;

        m_tcm_dma_bresp0_if.write(req_out);
    }
}

void DMA_BE::ProcessDMAWr1(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_dma_wr1_in.num_available())
        {
            wait();
        }

        m_dma_wr1_in.nb_read(req_in);
        IF_GEN::if_tcm_dma_bresp req_out;
        req_out.instr_id = req_in.sideband.instr_id;
        req_out.resp_type = req_in.sideband.resp_type;
        req_out.debug_tag = req_in.sideband.debug_tag;

        m_tcm_dma_bresp1_if.write(req_out);
    }
}

void DMA_BE::ProcessDMARdData(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_dma_rd_data_in.num_available())
        {
            wait();
        }

        m_dma_rd_data_in.nb_read(req_in);
        IF_GEN::if_tcm_dma_rd_data_rtn req_out;
        for (uint32_t u = 0; u < 32; u++){
            req_out.data[u] = req_in.payload.data[u];
        }
        req_out.blk_id = req_in.sideband.blk_id;
        req_out.req_id = req_in.sideband.req_id;
        req_out.debug_tag = req_in.sideband.debug_tag;

        m_tcm_dma_rd_data_rtn_if.write(req_out);
    }
}

void DMA_BE::ProcessDMARdDesc(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_dma_rd_desc_in.num_available())
        {
            wait();
        }

        m_dma_rd_desc_in.nb_read(req_in);
        IF_GEN::if_tcm_dma_rd_desc_rtn req_out;
        uint32_t bank_idx = req_in.sideband.addr.bank_index;
        for (uint32_t u = 0; u < 8; u++){
            req_out.data[u] = req_in.payload.data[bank_idx * 8 + u];
        }
        req_out.blk_id = req_in.sideband.blk_id;
        req_out.cl_id = req_in.sideband.cl_id;
        req_out.debug_tag = req_in.sideband.debug_tag;

        m_tcm_dma_rd_desc_rtn_if.write(req_out);
    }
}

void TC_BE::ProcessTCWr(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_tc_wr_in.num_available())
        {
            wait();
        }

        m_tc_wr_in.nb_read(req_in);
        IF_GEN::if_tc_tcm_bresp req_out;
        req_out.op = req_in.sideband.op;
        req_out.rvs_core_id = req_in.sideband.rvs_core_id;
        req_out.instr_id = req_in.sideband.instr_id;
        req_out.resp = 0;
        req_out.debug_tag = req_in.sideband.debug_tag;

        m_tc_tcm_bresp_if.write(req_out);
    }
}

void TC_BE::ProcessTCBRd0(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_tc_b_rd0_in.num_available())
        {
            wait();
        }

        m_tc_b_rd0_in.nb_read(req_in);
        IF_GEN::if_tc_tcm_b_rtn req_out;
        req_out.rvs_core_id = req_in.sideband.rvs_core_id;
        req_out.instr_id = req_in.sideband.instr_id;
        req_out.buf_inx = req_in.sideband.buf_inx;
        for (uint32_t u = 0; u < 32; u++){
            req_out.data[u] = req_in.payload.data[u];
        }
        req_out.debug_tag = req_in.sideband.debug_tag;

        m_tc_tcm_b_rtn0_if.write(req_out);
    }
}

void TC_BE::ProcessTCBRd1(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_tc_b_rd1_in.num_available())
        {
            wait();
        }

        m_tc_b_rd1_in.nb_read(req_in);
        IF_GEN::if_tc_tcm_b_rtn req_out;
        req_out.rvs_core_id = req_in.sideband.rvs_core_id;
        req_out.instr_id = req_in.sideband.instr_id;
        req_out.buf_inx = req_in.sideband.buf_inx;
        for (uint32_t u = 0; u < 32; u++){
            req_out.data[u] = req_in.payload.data[u];
        }
        req_out.debug_tag = req_in.sideband.debug_tag;

        m_tc_tcm_b_rtn1_if.write(req_out);
    }
}

void TC_BE::ProcessTCMixRd(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_tc_mix_rd_in.num_available())
        {
            wait();
        }

        m_tc_mix_rd_in.nb_read(req_in);
        IF_GEN::if_tc_tcm_ac_sf_rtn req_out;
        req_out.rvs_core_id = req_in.sideband.rvs_core_id;
        req_out.instr_id = req_in.sideband.instr_id;
        req_out.buf_inx = req_in.sideband.buf_inx;
        req_out.matrix_src = req_in.sideband.matrix_src;
        req_out.grp_mask = req_in.sideband.grp_mask;
        for (uint32_t u = 0; u < 32; u++){
            req_out.data[u] = req_in.payload.data[u];
        }
        req_out.debug_tag = req_in.sideband.debug_tag;

        m_tc_tcm_ac_sf_rtn_if.write(req_out);
    }
}

void RVV_BE::ProcessVLSUWr(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_vlsu_wr_in.num_available())
        {
            wait();
        }

        m_vlsu_wr_in.nb_read(req_in);
        IF_GEN::if_vlsu_tcm_bresp req_out;
        req_out.op = req_in.sideband.op;
        req_out.resp = 0;
        req_out.debug_tag = req_in.sideband.debug_tag;

        switch (req_in.sideband.master_type){
            case MASTER_RVV0:
                m_vlsu0_tcm_bresp_if.write(req_out);
                break;
            case MASTER_RVV1:
                m_vlsu1_tcm_bresp_if.write(req_out);
                break;
            case MASTER_RVV2:
                m_vlsu2_tcm_bresp_if.write(req_out);
                break;
            case MASTER_RVV3:
                m_vlsu3_tcm_bresp_if.write(req_out);
                break;
            default:
                break;
        }
    }
}

void RVV_BE::ProcessVLSURd(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_vlsu_rd_in.num_available())
        {
            wait();
        }

        m_vlsu_rd_in.nb_read(req_in);
        IF_GEN::if_vlsu_tcm_rtn req_out0;
        IF_GEN::if_vlsu_tcm_rtn req_out1;

        if (req_in.sideband.burst_len == 0){
            req_out0.debug_tag = req_in.sideband.debug_tag;
            req_out0.last = 1;
            uint32_t bank_idx = req_in.sideband.addr.bank_index;
            for (uint32_t u = 0; u < 16; u++){
                req_out0.data[u] = req_in.payload.data[bank_idx * 8 + u];
            }
        }
        else{
            req_out0.debug_tag = req_in.sideband.debug_tag;
            req_out0.last = 0;
            for (uint32_t u = 0; u < 16; u++){
                req_out0.data[u] = req_in.payload.data[16 + u];
            }

            req_out1.debug_tag = req_in.sideband.debug_tag;
            req_out1.last = 1;
            for (uint32_t u = 0; u < 16; u++){
                req_out1.data[u] = req_in.payload.data[u];
            }
        }

        switch (req_in.sideband.master_type){
            case MASTER_RVV0:
                m_vlsu0_tcm_rtn_if.write(req_out0);
                if (req_in.sideband.burst_len == 1){
                    m_vlsu0_tcm_rtn_if.write(req_out1);
                }
                break;
            case MASTER_RVV1:
                m_vlsu1_tcm_rtn_if.write(req_out0);
                if (req_in.sideband.burst_len == 1){
                    m_vlsu1_tcm_rtn_if.write(req_out1);
                }
                break;
            case MASTER_RVV2:
                m_vlsu2_tcm_rtn_if.write(req_out0);
                if (req_in.sideband.burst_len == 1){
                    m_vlsu2_tcm_rtn_if.write(req_out1);
                }
                break;
            case MASTER_RVV3:
                m_vlsu3_tcm_rtn_if.write(req_out0);
                if (req_in.sideband.burst_len == 1){
                    m_vlsu3_tcm_rtn_if.write(req_out1);
                }
                break;
            default:
                break;
        }
    }
}

void NIU_BE::ProcessNIUBankWrRd(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_bank_niu_wr_rd_in.num_available())
        {
            wait();
        }

        m_bank_niu_wr_rd_in.nb_read(req_in);
        if (req_in.sideband.op_type == OP_WRITE){
            IF_GEN::if_niu_tcm_wresp req_out;
            req_out.id = req_in.sideband.id;
            req_out.user = req_in.sideband.user;
            req_out.resp = 0;

            m_bank_niu_wresp_fifo.write(req_out);
        }
        else{
            IF_GEN::if_niu_tcm_rtn req_out;
            req_out.id = req_in.sideband.id;
            req_out.last = req_in.sideband.last;
            req_out.user = req_in.sideband.user;
            req_out.resp = 0;
            for (uint32_t u = 0; u < 32; u++){
                req_out.data[u] = req_in.payload.data[u];
            }

            m_bank_niu_rtn_fifo.write(req_out);
        }
    }
}

void NIU_BE::ProcessNIUASWrRd(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_as_niu_wr_rd_in.num_available())
        {
            wait();
        }

        m_as_niu_wr_rd_in.nb_read(req_in);
        if (req_in.sideband.master_type == MASTER_RVC){
            IF_GEN::if_tcm_niu_remt_sem req_out;
            req_out.sem_addr = req_in.sideband.addr.raw & 0xFFFFFFFFFFFF;
            req_out.sem_num = req_in.sideband.sem_num;
            m_tcm_niu_remt_sem_if.write(req_out);
        }
        else{
            if (req_in.sideband.op_type == OP_ATOMIC){
                IF_GEN::if_niu_tcm_wresp req_wresp_out;
                IF_GEN::if_niu_tcm_rtn req_rtn_out;
                req_wresp_out.id = req_in.sideband.id;
                req_wresp_out.user = req_in.sideband.user;
                req_wresp_out.resp = 0;

                m_as_niu_wresp_fifo.write(req_wresp_out);

                req_rtn_out.id = req_in.sideband.id;
                req_rtn_out.last = 1;
                req_rtn_out.user = req_in.sideband.user;
                req_rtn_out.resp = 0;
                for (uint32_t u = 0; u < 32; u++){
                    req_rtn_out.data[u] = req_in.payload.data[u];
                }

                m_as_niu_rtn_fifo.write(req_rtn_out);
            }
            else if (req_in.sideband.op_type == OP_READ){
                IF_GEN::if_niu_tcm_rtn req_out;
                req_out.id = req_in.sideband.id;
                req_out.last = req_in.sideband.last;
                req_out.user = req_in.sideband.user;
                req_out.resp = 0;
                for (uint32_t u = 0; u < 32; u++){
                    req_out.data[u] = req_in.payload.data[u];
                }

                m_as_niu_rtn_fifo.write(req_out); 
            }
            else{ //WRITE SEM_POST
                IF_GEN::if_niu_tcm_wresp req_out;
                req_out.id = req_in.sideband.id;
                req_out.user = req_in.sideband.user;
                req_out.resp = 0;

                m_as_niu_wresp_fifo.write(req_out);
            }
        }
    }
}

void NIU_BE::ProcessNIUWrespRoundRobin(void)
{
    int rr = 0; // 0 = WR, 1 = RD

    while (true)
    {
        // 等待至少有一个请求
        while (m_bank_niu_wresp_fifo.num_available() == 0 && m_as_niu_wresp_fifo.num_available() == 0)
            wait();

        IF_GEN::if_niu_tcm_wresp req;
        if (rr == 0 && m_as_niu_wresp_fifo.num_available()) {
            m_as_niu_wresp_fifo.nb_read(req);
            rr = 1;
        }
        else if (rr == 1 && m_bank_niu_wresp_fifo.num_available()) {
            m_bank_niu_wresp_fifo.nb_read(req);
            rr = 0;
        }
        else if (m_as_niu_wresp_fifo.num_available()) {  // 兜底
            m_as_niu_wresp_fifo.nb_read(req);
        }
        else if (m_bank_niu_wresp_fifo.num_available()) {
            m_bank_niu_wresp_fifo.nb_read(req);
        }

        // 下发到最终输出
        m_niu_tcm_wresp_if.write(req);
    }
}

void NIU_BE::ProcessNIURtnRoundRobin(void)
{
    int rr = 0; // 0 = WR, 1 = RD

    while (true)
    {
        // 等待至少有一个请求
        while (m_bank_niu_rtn_fifo.num_available() == 0 && m_as_niu_rtn_fifo.num_available() == 0)
            wait();

        IF_GEN::if_niu_tcm_rtn req;
        if (rr == 0 && m_as_niu_rtn_fifo.num_available()) {
            m_as_niu_rtn_fifo.nb_read(req);
            rr = 1;
        }
        else if (rr == 1 && m_bank_niu_rtn_fifo.num_available()) {
            m_bank_niu_rtn_fifo.nb_read(req);
            rr = 0;
        }
        else if (m_as_niu_rtn_fifo.num_available()) {  // 兜底
            m_as_niu_rtn_fifo.nb_read(req);
        }
        else if (m_bank_niu_rtn_fifo.num_available()) {
            m_bank_niu_rtn_fifo.nb_read(req);
        }

        // 下发到最终输出
        m_niu_tcm_rtn_if.write(req);
    }
}

void RVC_BE::ProcessRVCWrRd(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_as_rvc_in.num_available())
        {
            wait();
        }

        m_as_rvc_in.nb_read(req_in);
        if (req_in.sideband.op_type == OP_WRITE){
            IF_GEN::if_rvs_tcm_b req_out;
            req_out.id = req_in.sideband.id;
            req_out.resp = 0;

            m_rvs_tcm_b_if.write(req_out);
        }
        else if (req_in.sideband.op_type == OP_READ){
            IF_GEN::if_rvs_tcm_r req_out;
            req_out.id = req_in.sideband.id;
            req_out.resp = 0;
            req_out.last = req_in.sideband.last;
            uint32_t bank_idx = req_in.sideband.addr.bank_index;
            for (uint32_t u = 0; u < 8; u++){
                req_out.data[u] = req_in.payload.data[bank_idx * 8 + u]; 
            }

            m_rvs_tcm_r_if.write(req_out);
        }
        else if (req_in.sideband.op_type == OP_ATOMIC){
            IF_GEN::if_rvs_tcm_b req_b_out;
            req_b_out.id = req_in.sideband.id;
            req_b_out.resp = 0;

            m_rvs_tcm_b_if.write(req_b_out);

            IF_GEN::if_rvs_tcm_r req_r_out;
            req_r_out.id = req_in.sideband.id;
            req_r_out.resp = 0;
            req_r_out.last = 1;
            uint32_t bank_idx = req_in.sideband.addr.bank_index;
            for (uint32_t u = 0; u < 8; u++){
                req_r_out.data[u] = req_in.payload.data[bank_idx * 8 + u]; 
            }

            m_rvs_tcm_r_if.write(req_r_out);
        }
        else if (req_in.sideband.op_type == OP_SEM_WAIT || req_in.sideband.op_type == OP_BARRIER_PROC){
            IF_GEN::if_acc_resp req_out;
            req_out.wakeup_mode = 1;
            req_out.trans_id = req_in.sideband.trans_id;
            req_out.rvs_id = req_in.sideband.rvs_id;

            m_acc_resp_if.write(req_out);
        }
    }
}

void RVC_BE::ProcessRVCCfi(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_as_cfi_in.num_available())
        {
            wait();
        }

        m_as_cfi_in.nb_read(req_in);
        IF_GEN::if_rvs_tcm_cfi_ack req_out;
        req_out.valid = 1;

        m_rvs_tcm_cfi_ack_if.write(req_out);
    }
}

void CP_BE::ProcessCPRd(void)
{
    Request req_in = {};

    while(true)
    {
        while(!m_as_cp_in.num_available())
        {
            wait();
        }

        m_as_cp_in.nb_read(req_in);
        IF_GEN::if_bif_rtn req_out;
        req_out.tag_sb = req_in.sideband.tag_sb;
        uint32_t bank_idx = req_in.sideband.addr.bank_index;
        for (uint32_t u = 0; u < 8; u++){
            req_out.data[u] = req_in.payload.data[bank_idx * 8 + u];
        }

        m_bif_rtn_if.write(req_out);
    }
}

void BANK::UpdateBank(void)
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
        m_tc_wr_in.nb_read(buf.ifbuf[TC_W].req);
        if (buf.ifbuf[TC_W].req.sideband.op_type == OP_SEM_POST){
            m_tc_wr_out.write(buf.ifbuf[TC_W].req);
            buf.ifbuf[TC_W].req = {};
        }
        else{
            buf.ifbuf[TC_W].valid = true;
            buf.ValidBufNum++;
        }
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
        m_vlsu0_wr_in.nb_read(buf.ifbuf[RVV0_W].req);
        if (buf.ifbuf[RVV0_W].req.sideband.op_type == OP_VMEM_FENCE){
            m_vlsu_wr_out.write(buf.ifbuf[RVV0_W].req);
            buf.ifbuf[RVV0_W].req = {};
        }
        else{
            buf.ifbuf[RVV0_W].valid = true;
            buf.ValidBufNum++;
        }
    }
    if (buf.ifbuf[RVV1_W].valid == false && m_vlsu1_wr_in.num_available()){
        m_vlsu1_wr_in.nb_read(buf.ifbuf[RVV1_W].req);
        if (buf.ifbuf[RVV1_W].req.sideband.op_type == OP_VMEM_FENCE){
            m_vlsu_wr_out.write(buf.ifbuf[RVV1_W].req);
            buf.ifbuf[RVV1_W].req = {};
        }
        else{
            buf.ifbuf[RVV1_W].valid = true;
            buf.ValidBufNum++;
        }
    }
    if (buf.ifbuf[RVV2_W].valid == false && m_vlsu2_wr_in.num_available()){
        m_vlsu2_wr_in.nb_read(buf.ifbuf[RVV2_W].req);
        if (buf.ifbuf[RVV2_W].req.sideband.op_type == OP_VMEM_FENCE){
            m_vlsu_wr_out.write(buf.ifbuf[RVV2_W].req);
            buf.ifbuf[RVV2_W].req = {};
        }
        else{
            buf.ifbuf[RVV2_W].valid = true;
            buf.ValidBufNum++;
        }
    }
    if (buf.ifbuf[RVV3_W].valid == false && m_vlsu3_wr_in.num_available()){
        m_vlsu3_wr_in.nb_read(buf.ifbuf[RVV3_W].req);
        if (buf.ifbuf[RVV3_W].req.sideband.op_type == OP_VMEM_FENCE){
            m_vlsu_wr_out.write(buf.ifbuf[RVV3_W].req);
            buf.ifbuf[RVV3_W].req = {};
        }
        else{
            buf.ifbuf[RVV3_W].valid = true;
            buf.ValidBufNum++;
        }
    }
    if (buf.ifbuf[NIU_WR].valid == false && m_niu_wr_rd_in.num_available()){
        m_niu_wr_rd_in.nb_read(buf.ifbuf[NIU_WR].req);
        if (buf.ifbuf[NIU_WR].req.sideband.op_type == OP_SEM_POST){
            m_niu_wr_rd_out.write(buf.ifbuf[NIU_WR].req);
            buf.ifbuf[NIU_WR].req = {};
        }
        else{
            buf.ifbuf[NIU_WR].valid = true;
            buf.ValidBufNum++;
        }
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
            uint32_t buf_inx = req_in.sideband.buf_inx;
            for (uint32_t u = 0; u < lane_idx; u++){
                buf_inx += req_in.mask[u];
            }
            req_in.sideband.buf_inx = buf_inx;
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
                    buf_inx = req_in.sideband.buf_inx;
                    for (uint32_t u = 0; u <= lane_idx; u++){
                        buf_inx += req_in.mask[u];
                    }
                    req_in.sideband.buf_inx = buf_inx;
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
            uint32_t blk_id = 0;
            for (uint32_t u = 0; u < lane_idx; u++){
                blk_id += req_in.mask[u];
            }
            req_in.sideband.blk_id = blk_id;
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

    if (buf.ifbuf[RVV0_R].valid || buf.ifbuf[RVV1_R].valid || buf.ifbuf[RVV2_R].valid || buf.ifbuf[RVV3_R].valid){
        for (uint32_t u = 0; u < 4; u++){
            InterfaceType iftype_tmp = RVV0_R;
            switch (rr_rvv_r){
                case 0:
                    iftype_tmp = RVV0_R;
                    break;
                case 1:
                    iftype_tmp = RVV1_R;
                    break;
                case 2:
                    iftype_tmp = RVV2_R;
                    break;
                case 3:
                    iftype_tmp = RVV3_R;
                    break;
                default:
                    break;
            }
            rr_rvv_r = (rr_rvv_r + 1) % 4;
            if (buf.ifbuf[iftype_tmp].valid){
                uint8_t lane_idx = LaneCheck(buf.ifbuf[iftype_tmp].req);
                if (lane_idx != -1){
                    rvv_niu_arb_buf[0].valid = true;
                    rvv_niu_arb_buf[0].iftype = iftype_tmp;
                    rvv_niu_arb_buf[0].lane_idx = lane_idx;
                    break;
                }
            }
        }
    } 
    if (buf.ifbuf[RVV0_W].valid || buf.ifbuf[RVV1_W].valid || buf.ifbuf[RVV2_W].valid || buf.ifbuf[RVV3_W].valid){
        for (uint32_t u = 0; u < 4; u++){
            InterfaceType iftype_tmp = RVV0_W;
            switch (rr_rvv_w){
                case 0:
                    iftype_tmp = RVV0_W;
                    break;
                case 1:
                    iftype_tmp = RVV0_W;
                    break;
                case 2:
                    iftype_tmp = RVV0_W;
                    break;
                case 3:
                    iftype_tmp = RVV0_W;
                    break;
                default:
                    break;
            }
            rr_rvv_w = (rr_rvv_w + 1) % 4;
            if (buf.ifbuf[iftype_tmp].valid){
                uint8_t lane_idx = LaneCheck(buf.ifbuf[iftype_tmp].req);
                if (lane_idx != -1){
                    rvv_niu_arb_buf[1].valid = true;
                    rvv_niu_arb_buf[1].iftype = iftype_tmp;
                    rvv_niu_arb_buf[1].lane_idx = lane_idx;
                    break;
                }
            }
        }
    }
    if (buf.ifbuf[NIU_WR].valid == true){
        uint8_t lane_idx = LaneCheck(buf.ifbuf[NIU_WR].req);
        if (lane_idx != -1){
            rvv_niu_arb_buf[2].valid = true;
            rvv_niu_arb_buf[2].iftype = NIU_WR;
            rvv_niu_arb_buf[2].lane_idx = lane_idx;
        }
    }
    if (rvv_niu_arb_buf[0].valid || rvv_niu_arb_buf[1].valid || rvv_niu_arb_buf[2].valid){
        for (uint32_t u = 0; u < 3; u++){
            if (rvv_niu_arb_buf[u].valid){
                bool IsArb = false;
                for (uint32_t ii = u + 1; ii < 3; ii++){
                    if (rvv_niu_arb_buf[ii].valid && rvv_niu_arb_buf[u].lane_idx == rvv_niu_arb_buf[ii].lane_idx){
                        IsArb = true;
                        while (rr_rvv_niu != u && rr_rvv_niu != ii){
                            rr_rvv_niu = (rr_rvv_niu + 1) % 3;
                        }
                        if (rr_rvv_niu == u){
                            rvv_niu_arb_buf[ii].valid = false;
                        }
                        else{
                            rvv_niu_arb_buf[u].valid = false;
                        }
                    }
                }
                if (IsArb){
                    rr_rvv_niu = (rr_rvv_niu + 1) % 3;
                }
            }
        }
        for (uint32_t u = 0; u < 3; u++){
            if (rvv_niu_arb_buf[u].valid){
                Request req_out = BankWrRd(buf.ifbuf[rvv_niu_arb_buf[u].iftype].req);
                if (rvv_niu_arb_buf[u].iftype <= RVV3_R){
                    m_vlsu_rd_out.write(req_out);
                }
                else if (rvv_niu_arb_buf[u].iftype <= RVV3_W){
                    m_vlsu_wr_out.write(req_out);
                }
                else{
                    m_niu_wr_rd_out.write(req_out);
                }
                
                buf.ifbuf[rvv_niu_arb_buf[u].iftype].valid = false;
                buf.ifbuf[rvv_niu_arb_buf[u].iftype].req = {};
                buf.ValidBufNum--;
                rvv_niu_arb_buf[u].valid = false;
            }
        }
    }
    for (uint32_t u = 0; u < 8; u++){
        lanemask[u] = 1;
    }
}

uint8_t BANK::LaneCheck(Request req)
{
    for (uint32_t u = 0; u < 8; u++){
        if (req.mask[u] == 1 && lanemask[u] == 1){
            lanemask[u] = 0;
            return u;
        }
    }
    return -1;
}

Request BANK::BankWrRd(Request req)
{

    if (req.sideband.op_type == OP_WRITE){
        for (uint32_t bank_idx = 0; bank_idx < 4; bank_idx++){
            for (uint32_t u = 0; u < 8; u++){
                storage[req.sideband.addr.row_index][req.sideband.addr.lane_index][bank_idx][u] = (storage[req.sideband.addr.row_index][req.sideband.addr.lane_index][bank_idx][u] & ~req.payload.mask[bank_idx * 8 + u]) | (req.payload.data[bank_idx * 8 + u] & req.payload.mask[bank_idx * 8 + u]);
            }
        }
    }
    else{
        for (uint32_t bank_idx = 0; bank_idx < 4; bank_idx++){
            for (uint32_t u = 0; u < 8; u++){
                req.payload.data[bank_idx * 8 + u] = storage[req.sideband.addr.row_index][req.sideband.addr.lane_index][bank_idx][u] & req.payload.mask[bank_idx * 8 + u];
            }
        }
    }
    return req;
}

void BANK::ProcessBank(void)
{

    while (true)
    {
        UpdateBank();
        if (buf.ValidBufNum == 0)
        {
            wait();
        }
        else{
            ProcessBuf();
        }
    }
}


}
