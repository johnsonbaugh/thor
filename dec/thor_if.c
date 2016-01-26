/*
Copyright (c) 2015, Cisco Systems
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "global.h"
#include "maindec.h"
#include "decode_frame.h"
#include "common_frame.h"
#include "getbits.h"
#include "../common/simd.h"
#include "../common/wt_matrix.h"
#include "thor_if.h"
#include "thor_trace.h"

static void SetOutputFrame(decoder_info_t* dec, int idx, THOR_OUTPUT_PICTURE* picInfo);
static int32_t SetDecodeCallback(THOR_DEC_HANDLE h, enum THOR_DEC_CALLBACKS eCallbackType, void* fnCallback, void* param);
static int reorder_frame_offset(int idx, int sub_gop);

static int32_t DecodeSequenceHeader(decoder_info_t* dec)
{
    stream_t* stream = dec->stream;
    int width;
    int height;
    int qmtx;
    int interp_ref;
    int r;

    THOR_TRACE_GROUP_START(stream, "sequence_header", THOR_TRACE_TYPE_SEQUENCE_HDR);

    /* Read sequence header */
    THOR_TRACE_SE(stream, "width");
    width = getbits(stream, 16);

    THOR_TRACE_SE(stream, "height");
    height = getbits(stream, 16);

    THOR_TRACE_SE(stream, "pb_split_enable");
    dec->pb_split = getbits(stream, 1);

    THOR_TRACE_SE(stream, "tb_split_enable");
    dec->tb_split_enable = getbits(stream, 1);

    THOR_TRACE_SE(stream, "max_num_ref");
    dec->max_num_ref = getbits(stream,2) + 1;
    THOR_TRACE_VAL(stream, dec->max_num_ref);

    THOR_TRACE_SE(stream, "interp_ref");
    interp_ref = getbits(stream, 1);

    THOR_TRACE_SE(stream, "max_delta_qp");
    dec->max_delta_qp = getbits(stream, 1);

    THOR_TRACE_SE(stream, "deblocking");
    dec->deblocking = getbits(stream, 1);

    THOR_TRACE_SE(stream, "clpf");
    dec->clpf = getbits(stream, 1);

    THOR_TRACE_SE(stream, "use_block_contexts");
    dec->use_block_contexts = getbits(stream, 1);

    THOR_TRACE_SE(stream, "bipred");
    dec->bipred = getbits(stream, 1);

    THOR_TRACE_SE(stream, "qmtx");
    qmtx = getbits(stream, 1);

    /* Reallocate memory if necessary */
    if (width != dec->width || height != dec->height || interp_ref != dec->interp_ref || qmtx != dec->qmtx)
    {
        dec->width = width;
        dec->height = height;

        for (r = 0; r < MAX_REORDER_BUFFER; r++)
        {
            if (dec->dpb.rec_buf[r].y != NULL)
                close_yuv_frame(&dec->dpb.rec_buf[r]);
            create_yuv_frame(&dec->dpb.rec_buf[r], width, height, 0, 0, 0, 0);
        }

        for (r = 0; r < MAX_REF_FRAMES; r++)
        {
            if (dec->dpb.ref_buf[r].y != NULL)
                close_yuv_frame(&dec->dpb.ref_buf[r]);

            create_yuv_frame(&dec->dpb.ref_buf[r], width, height, PADDING_Y, PADDING_Y, PADDING_Y / 2, PADDING_Y / 2);
            dec->ref[r] = &dec->dpb.ref_buf[r];
        }

#ifdef ENABLE_PREDICTION_OUTPUT
        create_yuv_frame(&dec->dpb.pred_frame, width, height, 0, 0, 0, 0);
#endif

        if (dec->deblock_data != NULL)
        {
            free(dec->deblock_data);
            dec->deblock_data = NULL;
        }

        dec->deblock_data = (deblock_data_t *)malloc((height/MIN_PB_SIZE) * (width/MIN_PB_SIZE) * sizeof(deblock_data_t));

        if (dec->stream->trace.clpf_bits != NULL)
            free(dec->stream->trace.clpf_bits);

        dec->stream->trace.clpf_bits = (uint8_t*)malloc((height / MAX_BLOCK_SIZE) * (width / MAX_BLOCK_SIZE) * sizeof(uint8_t));

        if (dec->interp_ref)
        {
            for (r = 0; r < MAX_SKIP_FRAMES; r++)
            {
                close_yuv_frame(dec->interp_frames[r]);

                free(dec->interp_frames[r]);
                dec->interp_frames[r] = NULL;
            }

            dec->interp_ref = 0;
        }

        if (interp_ref)
        {
            dec->interp_ref = interp_ref;

            for (r = 0; r < MAX_SKIP_FRAMES; r++)
            {
              dec->interp_frames[r] = malloc(sizeof(yuv_frame_t));
              create_yuv_frame(dec->interp_frames[r], width, height, PADDING_Y, PADDING_Y, PADDING_Y / 2, PADDING_Y / 2);
            }

        }

        if (dec->qmtx)
        {
            free_wmatrices(dec->iwmatrix);
            dec->qmtx = 0;
        }

        if (qmtx)
        {
          dec->qmtx = qmtx;
          alloc_wmatrices(dec->iwmatrix);
          make_wmatrices(NULL, dec->iwmatrix);
        }
    }

    THOR_TRACE_GROUP_END(stream);

    return 0;
}

int32_t Thor_Decode(THOR_DEC_HANDLE h, uint8_t* data, int32_t len, int header)
{
    decoder_info_t* dec = (decoder_info_t*)h;
    stream_t* stream = dec->stream;
    st_thor_buffer* buffer = &stream->trace.buffer;
    st_thor_dpb* dpb = &dec->dpb;
    int output_buffer_idx;

    initbuffer(stream, data, len);

    while (morebits(stream))
    {
        if (setjmp(dec->stream->trace.error) == 0)
        {
            THOR_TRACE_GROUP_START(stream, "access_unit", THOR_TRACE_TYPE_ACCESSUNIT);

            THOR_TRACE_SE(stream, "frame_length");
            buffer->frame_length = getbits(stream, 32) << 3;
            buffer->bits_flushed = 0;

            if (header)
            {
                DecodeSequenceHeader(dec);
                header = 0;

                memset(dpb->rec_available, 0, sizeof(int) * MAX_REORDER_BUFFER);
                memset(dpb->rec_buffer_decode_frame_num, 0, sizeof(int) * MAX_REORDER_BUFFER);
                memset(dpb->rec_buffer_display_frame_num, 0, sizeof(int) * MAX_REORDER_BUFFER);
                dpb->rec_buffer_idx = 0;
                dpb->decode_frame_num = 0;
                dpb->last_frame_output = -1;
            }

            if (dec->width == 0 || dec->height == 0)
            {
                THOR_ERROR(stream, "No sequence header read");
                return 1;
            }

            dec->frame_info.decode_order_frame_num = dpb->decode_frame_num;

            dec->frame_info.decode_order_frame_num = dec->dpb.decode_frame_num;
            decode_frame(dec, dpb->rec_buf);
            dpb->rec_buffer_idx = dec->frame_info.display_frame_num % MAX_REORDER_BUFFER;
            dpb->rec_available[dpb->rec_buffer_idx] = 1;
            dpb->rec_buffer_decode_frame_num[dpb->rec_buffer_idx] = dec->frame_info.decode_order_frame_num;
            dpb->rec_buffer_display_frame_num[dpb->rec_buffer_idx] = dec->frame_info.display_frame_num;

            bytealign(stream);

            if (dec->decoding_mode == THOR_DEC_MODE_HEADER)
                flush_frame(stream);

            THOR_TRACE_GROUP_END(stream);

            output_buffer_idx = (dpb->last_frame_output + 1) % MAX_REORDER_BUFFER;

            if (dpb->rec_available[output_buffer_idx])
            {
                dpb->last_frame_output++;
                if (stream->trace.callback[THOR_DEC_CB_OUTPUT_FRAME].fn != NULL)
                {
                    THOR_OUTPUT_PICTURE picInfo;
                    SetOutputFrame(dec, output_buffer_idx, &picInfo);
                    ((ThorCallbackOutputFn)stream->trace.callback[THOR_DEC_CB_OUTPUT_FRAME].fn)(dec, &picInfo, stream->trace.callback[THOR_DEC_CB_OUTPUT_FRAME].param);
                }

                dpb->rec_available[output_buffer_idx] = 0;
            }

            dec->dpb.decode_frame_num++;
        }
        else
        {
            return 1;
        }
    }

    return 0;
}


THOR_DEC_HANDLE Thor_InitDecoder(void)
{
    decoder_info_t* dec = NULL;
    stream_t* stream = NULL;

    dec = (decoder_info_t*)calloc(1, sizeof(decoder_info_t));
    if (dec == NULL)
        return NULL;

    stream = (stream_t*)calloc(1, sizeof(stream_t));
    dec->stream = stream;

    dec->stream->trace.handle = dec;

    init_use_simd();
    Thor_ResetDecoder(dec);

    return (THOR_DEC_HANDLE)dec;
}

int32_t Thor_ReleaseDecoder(THOR_DEC_HANDLE h)
{
    decoder_info_t* dec = (decoder_info_t*)h;
    int r;

    for (r=0;r<MAX_REORDER_BUFFER;r++)
    {
      close_yuv_frame(&dec->dpb.rec_buf[r]);
    }

    for (r=0;r<MAX_REF_FRAMES;r++)
    {
      close_yuv_frame(&dec->dpb.ref_buf[r]);
    }

#ifdef ENABLE_PREDICTION_OUTPUT
    close_yuv_frame(&dec->dpb.pred_frame);
#endif

    if (dec->stream != NULL)
        free(dec->stream);

    if (dec->deblock_data != NULL)
        free(dec->deblock_data);

    if (dec->stream->trace.clpf_bits != NULL)
        free(dec->stream->trace.clpf_bits);

    if (dec->qmtx)
    {
        free_wmatrices(dec->iwmatrix);
        dec->qmtx = 0;
    }

    free(dec);

    return 0;
}


int32_t Thor_FlushDecodedPictures(THOR_DEC_HANDLE h)
{
    decoder_info_t* dec = (decoder_info_t*)h;
    st_thor_trace_state* trace = &dec->stream->trace;
    st_thor_dpb* dpb = &dec->dpb;
    int i;

    // Output the tail
    for (i = 1; i <= MAX_REORDER_BUFFER; ++i)
    {
        dpb->rec_buffer_idx = (dpb->last_frame_output + i) % MAX_REORDER_BUFFER;
        if (dpb->rec_available[dpb->rec_buffer_idx])
        {
            if (trace->callback[THOR_DEC_CB_OUTPUT_FRAME].fn != NULL)
            {
                THOR_OUTPUT_PICTURE picInfo;
                SetOutputFrame(dec, dpb->rec_buffer_idx, &picInfo);
                ((ThorCallbackOutputFn)trace->callback[THOR_DEC_CB_OUTPUT_FRAME].fn)(dec, &picInfo, trace->callback[THOR_DEC_CB_OUTPUT_FRAME].param);
            }
        }

        else
            break;
    }

    return 0;
}

int32_t Thor_ResetDecoder(THOR_DEC_HANDLE h)
{
    decoder_info_t* dec = (decoder_info_t*)h;
    st_thor_dpb* dpb = &dec->dpb;

    dec->decoding_mode = THOR_DEC_MODE_FULL;

    memset(dpb->rec_available, 0, sizeof(int) * MAX_REORDER_BUFFER);
    dpb->rec_buffer_idx = 0;
    dpb->decode_frame_num = 0;
    //dpb->frame_count = 0;
    dpb->last_frame_output = -1;

    Thor_ResetCallbacks(h);

    return 0;
}


int32_t Thor_ResetCallbacks(THOR_DEC_HANDLE h)
{
    decoder_info_t* dec = (decoder_info_t*)h;
    st_thor_trace_state* trace = &dec->stream->trace;
    int32_t i;

    for (i = 0; i < THOR_DEC_NUM_CALLBACKS; i++)
    {
        trace->callback[i].fn = NULL;
        trace->callback[i].param = NULL;
    }

    trace->enable = 0;
    trace->depth = 0;

    return 0;
}

int32_t Thor_SetDecodingMode(THOR_DEC_HANDLE h, enum THOR_DECODING_MODE mode)
{
    decoder_info_t* dec = (decoder_info_t*)h;

    dec->decoding_mode = mode;

    return 0;
}

int32_t Thor_SetDecodeCallbackFrame(THOR_DEC_HANDLE h, ThorCallbackFrameFn fnCallback, void* param)
{
    return SetDecodeCallback(h, THOR_DEC_CB_FRAME_COMPLETE, fnCallback, param);
}

int32_t Thor_SetDecodeCallbackCTU(THOR_DEC_HANDLE h, ThorCallbackCTUFn fnCallback, void* param)
{
    return SetDecodeCallback(h, THOR_DEC_CB_CTU, fnCallback, param);
}

int32_t Thor_SetDecodeCallbackBlock(THOR_DEC_HANDLE h, ThorCallbackBlockFn fnCallback, void* param)
{
    return SetDecodeCallback(h, THOR_DEC_CB_BLOCK, fnCallback, param);
}

int32_t Thor_SetDecodeCallbackOutput(THOR_DEC_HANDLE h, ThorCallbackOutputFn fnCallback, void* param)
{
    return SetDecodeCallback(h, THOR_DEC_CB_OUTPUT_FRAME, fnCallback, param);
}

int32_t Thor_SetDecodeCallbackError(THOR_DEC_HANDLE h, ThorCallbackErrorFn fnCallback, void* param)
{
    return SetDecodeCallback(h, THOR_DEC_CB_ERROR, fnCallback, param);
}

int32_t Thor_SetDecodeCallbackTrace(THOR_DEC_HANDLE h, ThorCallbackTraceFn fnCallback, uint32_t u32SyntaxTypes, uint32_t u32GroupTypes, void* param)
{
    decoder_info_t* dec = (decoder_info_t*)h;
    int32_t ret;

#ifdef THOR_TRACE_ENABLE
    st_thor_trace_state* trace = &dec->stream->trace;

    ret = SetDecodeCallback(h, THOR_DEC_CB_TRACE, fnCallback, param);
    trace->enable = (fnCallback != NULL);
    trace->syntaxTypes = u32SyntaxTypes;
    trace->groupTypes = u32GroupTypes;

    trace->str = NULL;
    trace->bitString = 0;
    trace->value = 0;
    trace->bitCount = 0;
    trace->valid = 0;
    trace->depth = 0;

    return ret;
#else
    return 1;
#endif
}

int32_t Thor_SetDecodeCallbackBuffer(THOR_DEC_HANDLE h, ThorCallbackBufferFn fnCallback, void* param)
{
    return SetDecodeCallback(h, THOR_DEC_CB_BUFFER, fnCallback, param);
}

static int32_t SetDecodeCallback(THOR_DEC_HANDLE h, enum THOR_DEC_CALLBACKS eCallbackType, void* fnCallback, void* param)
{
    decoder_info_t* dec = (decoder_info_t*)h;
    st_thor_trace_state* trace = &dec->stream->trace;

    if (eCallbackType < THOR_DEC_NUM_CALLBACKS)
    {
        trace->callback[eCallbackType].fn = fnCallback;
        trace->callback[eCallbackType].param = param;
        return 0;
    }
    else
    {
        return 1;
    }
}

int32_t Thor_GetPicInfo(THOR_DEC_HANDLE h, THOR_OUTPUT_PICTURE* picInfo)
{
    decoder_info_t* dec = (decoder_info_t*)h;

    SetOutputFrame(dec, dec->dpb.rec_buffer_idx, picInfo);

    return 1;
}

static void SetOutputFrame(decoder_info_t* dec, int idx, THOR_OUTPUT_PICTURE* picInfo)
{
    st_thor_dpb* dpb = &dec->dpb;

#ifdef ENABLE_PREDICTION_OUTPUT
    yuv_frame_t* rec = dec->decoding_mode == THOR_DEC_MODE_PRED ? &dpb->pred_frame : &dpb->rec_buf[idx];
#else
    yuv_frame_t* rec = &dpb->rec_buf[idx];
#endif

    picInfo->s32PicOrderCount = dpb->rec_buffer_display_frame_num[idx];
    picInfo->u32DecodeOrder = dpb->rec_buffer_decode_frame_num[idx];
    picInfo->u64PTS = 0;
    picInfo->u64DTS = 0;
    picInfo->u16Width[0] = dec->width;
    picInfo->u16Width[1] = dec->width >> 1;
    picInfo->u16Width[2] = dec->width >> 1;
    picInfo->u16Height[0] = dec->height;
    picInfo->u16Height[1] = dec->height >> 1;
    picInfo->u16Height[2] = dec->height >> 1;
    picInfo->ppxPic[0] = (uint8_t*)rec->y;
    picInfo->ppxPic[1] = (uint8_t*)rec->u;
    picInfo->ppxPic[2] = (uint8_t*)rec->v;
    picInfo->u16Stride[0] = rec->stride_y;
    picInfo->u16Stride[1] = rec->stride_c;
    picInfo->u16Stride[2] = rec->stride_c;
    picInfo->u8YUVFormat = 1;
    picInfo->u8PixelSize = 1;
    picInfo->u8BitDepthLuma = 8;
    picInfo->u8BitDepthChroma = 8;
}


// Coding order to display order
static const int cd1[1] = {0};
static const int cd2[2] = {1,0};
static const int cd4[4] = {3,1,0,2};
static const int cd8[8] = {7,3,1,5,0,2,4,6};
static const int cd16[16] = {15,7,3,11,1,5,9,13,0,2,4,6,8,10,12,14};
static const int* dyadic_reorder_code_to_display[5] = {cd1,cd2,cd4,cd8,cd16};

// Display order to coding order
//static const int dc1[1+1] = {-1,0};
//static const int dc2[2+1] = {-2,1,0};
//static const int dc4[4+1] = {-4,2,1,3,0};
//static const int dc8[8+1] = {-8,4,2,5,1,6,3,7,0};
//static const int dc16[16+1] = {-16,8,4,9,2,10,5,11,1,12,6,13,3,14,7,15,0};
//static const int* dyadic_reorder_display_to_code[5] = {dc1,dc2,dc4,dc8,dc16}; //Not used in decoder?

static int reorder_frame_offset(int idx, int sub_gop)
{
#if DYADIC_CODING
  return dyadic_reorder_code_to_display[log2i(sub_gop)][idx]-sub_gop+1;
#else
  if (idx==0) return 0;
  else return idx-sub_gop;
#endif
}

