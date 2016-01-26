//
//  thor_trace.h
//  thor_dec
//
//  Created by Ben Walton on 2013-02-04.
//  Copyright (c) 2013 Ben Walton. All rights reserved.
//

#ifndef thor_dec_thor_debug_h
#define thor_dec_thor_debug_h

#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdint.h>

#define THOR_TRACE_ENABLE
#define THOR_ASSERT_ENABLE

#ifdef THOR_TRACE_ENABLE

#define MAX_TRACE_DEPTH 16

enum THOR_DEC_CALLBACKS {
    THOR_DEC_CB_CTU,
    THOR_DEC_CB_BLOCK,
    THOR_DEC_CB_FRAME_COMPLETE,
    THOR_DEC_CB_OUTPUT_FRAME,
    THOR_DEC_CB_ERROR,
    THOR_DEC_CB_TRACE,
    THOR_DEC_CB_BUFFER,
    THOR_DEC_NUM_CALLBACKS
};

typedef struct ST_THOR_CALLBACK_INFO {
    void* fn;
    void* param;
} st_thor_callback_info;

typedef struct ST_THOR_BUFFER_SEGMENT {
    uint8_t* buf_ptr;
    uint64_t segment_id;
    uint32_t segment_length;
    uint32_t byte_pos;
    int32_t  bits_remaining;
} st_thor_buffer_segment;

typedef struct ST_THOR_BUFFER {
    st_thor_buffer_segment segment[4];
    uint64_t cur_word;
    uint32_t frame_length;
    uint32_t bits_flushed;
    uint32_t bit_pos;
    int32_t  active_segment;
    int32_t  read_segment;
    int32_t  end_of_stream;
} st_thor_buffer;


typedef struct ST_THOR_TRACE_STATE {
    st_thor_buffer buffer;
    uint8_t  enable;
    uint8_t  valid;
    uint8_t  bitCount;
    uint8_t  depth;
    uint8_t  format;
    uint32_t syntaxTypes;
    uint32_t groupTypes;
    uint32_t curType;
    uint32_t prevTypes[MAX_TRACE_DEPTH];
    uint32_t bitPos[MAX_TRACE_DEPTH];
    uint32_t groupBits;
    int32_t  value;
    uint32_t bitString;
    uint64_t segment_id_start;
    uint64_t segment_id_end;
    uint32_t bit_pos_start;
    uint32_t bit_pos_end;
    const char*  str;
    const char*  msg;
    const char*  note;
    st_thor_callback_info callback[THOR_DEC_NUM_CALLBACKS];
    jmp_buf error;
    void* handle;       /* Decoder handle */
    uint8_t* clpf_bits; /* Additional buffer to store CLPF bits, which are not otherwise accessible */
} st_thor_trace_state;

typedef enum {
    TRACE_FORMAT_INT = 0,
    TRACE_FORMAT_CHAR = 1,
    TRACE_FORMAT_BIN = 2,
    TRACE_FORMAT_HEX = 3
} THOR_TRACE_FORMAT;

void ThorTrace_Msg(st_thor_trace_state* trace, const char* str, ...);
void ThorTrace_SyntaxElement(st_thor_trace_state* trace, const char* str, THOR_TRACE_FORMAT format);
void ThorTrace_Bits(st_thor_trace_state* trace, uint32_t bits, uint32_t val);
void ThorTrace_Value(st_thor_trace_state* trace, int32_t val);
void ThorTrace_Note(st_thor_trace_state* trace, const char* str);
void ThorTrace_StartGroup(st_thor_trace_state* trace, int type, const char* str);
void ThorTrace_EndGroup(st_thor_trace_state* trace);
void ThorTrace_EndPos(st_thor_trace_state* trace);

#define THOR_TRACE_MSG(stream, str, ...) { if (stream->trace.enable) { ThorTrace_Msg(&stream->trace, str, ##__VA_ARGS__); } }
#define THOR_TRACE_SE(stream, str) { if (((stream_t*)stream)->trace.enable) ThorTrace_SyntaxElement(&((stream_t*)stream)->trace, str, TRACE_FORMAT_INT); }
#define THOR_TRACE_SE_FORMAT(stream, str, format) { if (stream->trace.enable) ThorTrace_SyntaxElement(&stream->trace, str, format); }
#define THOR_TRACE_ENDPOS(stream) { if (stream->trace.enable) ThorTrace_EndPos(&stream->trace); }
#define THOR_TRACE_BITS(stream, num, val) { if (stream->trace.enable) { ThorTrace_Bits(&stream->trace, num, val); } }
#define THOR_TRACE_VAL(stream, val) { if (stream->trace.enable) { ThorTrace_Value(&stream->trace, val); } }
#define THOR_TRACE_NOTE(stream, str) { if (stream->trace.enable) { ThorTrace_Note(&stream->trace, str); } }
#define THOR_TRACE_GROUP_START(stream, str, type) { if (stream->trace.enable) ThorTrace_StartGroup(&stream->trace, type, str); }
#define THOR_TRACE_GROUP_END(stream) { if (stream->trace.enable) ThorTrace_EndGroup(&stream->trace); }

#else

#define THOR_TRACE_MSG(stream, x, ...) { }
#define THOR_TRACE_SE(stream, x) { }
#define THOR_TRACE_SE_FORMAT(stream, x, y) { }
#define THOR_TRACE_ENDPOS(stream) { }
#define THOR_TRACE_BITS(stream, x, y) { }
#define THOR_TRACE_VAL(stream, x) { }
#define THOR_TRACE_NOTE(stream, x) { }
#define THOR_TRACE_GROUP_START(stream, str, type) { }
#define THOR_TRACE_GROUP_END(stream) { }

#endif

#ifdef THOR_ASSERT_ENABLE

void ThorError(st_thor_trace_state* trace, char* str);

#define THOR_ASSERT(stream, cond, str) { if (!(cond)) { ThorError(&stream->trace, str); } }
#define THOR_ERROR(stream, str) { ThorError(&stream->trace, str); }

#else

#define THOR_ERROR(stream, x) { }
#define THOR_ASSERT(stream, x, y) { }

#endif

#endif
