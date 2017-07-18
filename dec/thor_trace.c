//
//  thor_trace.c
//  thor_dec
//
//  Created by Ben Walton on 2013-02-09.
//  Copyright (c) 2013 Ben Walton. All rights reserved.
//

#include <stdarg.h>
#include <stdint.h>

#include "thor_if.h"
#include "maindec.h"
#include "thor_trace.h"

#ifdef THOR_TRACE_ENABLE

static void ThorTrace_CloseSE(st_thor_trace_state* trace);


void ThorTrace_Msg(st_thor_trace_state* trace, const char* str, ...)
{
    char msg[100];

    va_list args;
    va_start (args, str);

    if (trace->valid)
        ThorTrace_CloseSE(trace);
    
#ifdef _WIN32
    if (str != NULL)
        vsprintf_s(msg, 100, str, args);
#else
    if (str != NULL)
        vsnprintf(msg, 100, str, args);
#endif

    va_end(args);
    
    trace->msg = msg;
    
    if (((trace->syntaxTypes & THOR_TRACE_TYPE_MESSAGE) != 0) && trace->callback[THOR_DEC_CB_TRACE].fn != NULL)
        ((ThorCallbackTraceFn)trace->callback[THOR_DEC_CB_TRACE].fn)(trace->handle, THOR_TRACE_EVENT_MESSAGE, trace->callback[THOR_DEC_CB_TRACE].param);
}

void ThorTrace_SyntaxElement(st_thor_trace_state* trace, const char* str, THOR_TRACE_FORMAT format)
{
    st_thor_buffer_segment* seg = &trace->buffer.segment[trace->buffer.active_segment];

    if (trace->valid)
        ThorTrace_CloseSE(trace);
    
    trace->str = str;
    trace->format = format;
    trace->valid |= 1;

    trace->segment_id_start = seg->segment_id;
    trace->bit_pos_start = (seg->segment_length << 3) - seg->bits_remaining;
}

void ThorTrace_EndPos(st_thor_trace_state* trace)
{
    st_thor_buffer_segment* seg = &trace->buffer.segment[trace->buffer.active_segment];

    trace->segment_id_end = seg->segment_id;
    trace->bit_pos_end = (seg->segment_length << 3) - seg->bits_remaining;
}

void ThorTrace_Bits(st_thor_trace_state* trace, uint32_t bits, uint32_t val)
{
    if (bits == 32)
        trace->bitString = 0;
    else
        trace->bitString <<= bits;
    trace->bitCount += (uint8_t)bits;
    trace->bitString |= val;

    trace->valid |= 2;
}

void ThorTrace_Value(st_thor_trace_state* trace, int32_t val)
{
    trace->value = val;
    trace->valid |= 4;
}

void ThorTrace_Note(st_thor_trace_state* trace, const char* str)
{
    trace->note = str;
}

static void ThorTrace_CloseSE(st_thor_trace_state* trace)
{
    if (((trace->syntaxTypes & trace->curType) != 0) && trace->callback[THOR_DEC_CB_TRACE].fn != NULL)
        ((ThorCallbackTraceFn)trace->callback[THOR_DEC_CB_TRACE].fn)(trace->handle, THOR_TRACE_EVENT_SYNTAX, trace->callback[THOR_DEC_CB_TRACE].param);
    
    trace->str = NULL;
    trace->note = NULL;
    trace->value = 0;
    trace->bitCount = 0;
    trace->bitString = 0;
    trace->valid = 0;
}

void ThorTrace_StartGroup(st_thor_trace_state* trace, int type, const char* str)
{
    if (trace->valid)
        ThorTrace_CloseSE(trace);
    
    trace->str = str;
    if (trace->depth < MAX_TRACE_DEPTH)
    {
        trace->prevTypes[trace->depth] = trace->curType;
        trace->bitPos[trace->depth] = trace->buffer.frame_length;
    }
    
    trace->depth++;
    trace->curType = type;
    
    if (((trace->groupTypes & trace->curType) != 0) && trace->callback[THOR_DEC_CB_TRACE].fn != NULL)
        ((ThorCallbackTraceFn)trace->callback[THOR_DEC_CB_TRACE].fn)(trace->handle, THOR_TRACE_EVENT_GROUP_START, trace->callback[THOR_DEC_CB_TRACE].param);
}

void ThorTrace_EndGroup(st_thor_trace_state* trace)
{
    uint32_t bitPos;
    
    if (trace->valid)
        ThorTrace_CloseSE(trace);
    
    bitPos = trace->buffer.frame_length;
    trace->groupBits = bitPos - trace->bitPos[trace->depth - 1];
    
    if (((trace->groupTypes & trace->curType) != 0) && trace->callback[THOR_DEC_CB_TRACE].fn != NULL)
        ((ThorCallbackTraceFn)trace->callback[THOR_DEC_CB_TRACE].fn)(trace->handle, THOR_TRACE_EVENT_GROUP_END, trace->callback[THOR_DEC_CB_TRACE].param);
    
    trace->depth--;
    if (trace->depth < MAX_TRACE_DEPTH)
        trace->curType = trace->prevTypes[trace->depth];
}

#endif


#ifdef THOR_ASSERT_ENABLE

void ThorError(st_thor_trace_state* trace, char* str)
{
    char msg[100];
    int32_t ret = 0;

#ifdef _WIN32
	sprintf_s(msg, 100, "ERROR: %s", str);
#else
    snprintf(msg, 100, "ERROR: %s", str);
#endif

    if (trace->callback[THOR_DEC_CB_ERROR].fn != NULL)
    {
        ret = ((ThorCallbackErrorFn)trace->callback[THOR_DEC_CB_ERROR].fn)(trace->handle, msg, trace->callback[THOR_DEC_CB_ERROR].param);
    }
#if USE_CONSOLE
    else
    {
        printf("%s\n", msg);
    }
#endif

    if (ret == 0)
        longjmp(trace->error, 1);
}

#endif
