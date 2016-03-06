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

#include "getbits.h"
#include "thor_if.h"
#include "thor_trace.h"
#include "maindec.h"

static uint8_t NextByte(stream_t* stream);
static void NextSegment(stream_t* stream);

void initbuffer(stream_t* stream, uint8_t* buf_ptr, uint32_t length)
{
    st_thor_buffer* buffer = &stream->trace.buffer;
    st_thor_buffer_segment* seg = &buffer->segment[0];

    buffer->active_segment = 0;

    if (buf_ptr != NULL)
    {
        buffer->end_of_stream = 1;
        buffer->read_segment = 0;
        seg->buf_ptr = buf_ptr;
        seg->byte_pos = 0;
        seg->bits_remaining = length << 3;
        seg->segment_id = 0;
        seg->segment_length = length;
    }
    else
    {
        buffer->read_segment = -1;
        seg->buf_ptr = NULL;
        buffer->end_of_stream = 0;

        NextSegment(stream);
    }

    buffer->bit_pos = 0;
    buffer->cur_word = 0;
}


int morebits(stream_t* stream)
{
    st_thor_buffer* buffer = &stream->trace.buffer;
    st_thor_buffer_segment* segment = &buffer->segment[buffer->active_segment];

    return (!buffer->end_of_stream || buffer->read_segment != buffer->active_segment || segment->bits_remaining > 0);
}

unsigned int getbits(stream_t *stream, int bits)
{
    uint32_t val = showbits(stream, bits);
    flushbits(stream, bits);

    THOR_TRACE_BITS(stream, bits, val);

    return val;
}

unsigned int getbits1(stream_t *stream)
{
    return getbits(stream, 1);
}

void bytealign(stream_t* stream)
{
    st_thor_buffer* buffer = &stream->trace.buffer;

    THOR_TRACE_SE(stream, "byte_alignment");

    getbits(stream, buffer->bit_pos & 7);
}

void flush_frame(stream_t* stream)
{
    st_thor_buffer* buffer = &stream->trace.buffer;

    /* Assumes byte alignment. */

    while (buffer->bits_flushed < buffer->frame_length)
    {
        showbits(stream, 8);
        flushbits(stream, 8);
    }
}

int flushbits(stream_t *stream, int bits)
{
    /* Bits to be flushed must have previously been 'Show'n */
    st_thor_buffer* buffer = &stream->trace.buffer;
    st_thor_buffer_segment* segment = &buffer->segment[buffer->active_segment];

    buffer->bit_pos -= bits;
    segment->bits_remaining -= bits;
    buffer->bits_flushed += bits;

    /* Handle segment boundaries */
    while (segment->bits_remaining < 0 && buffer->active_segment != buffer->read_segment)
    {
        bits = -segment->bits_remaining;
        buffer->active_segment = (buffer->active_segment + 1) & 3;
        segment = &buffer->segment[buffer->active_segment];
        segment->bits_remaining -= bits;
    }

    THOR_ASSERT(stream, segment->bits_remaining >= 0, "Read past end of data");
    
    THOR_TRACE_ENDPOS(stream);

    if (segment->bits_remaining == 0 && !buffer->end_of_stream)
    {
        if (buffer->active_segment != buffer->read_segment)
            buffer->active_segment = (buffer->active_segment + 1) & 3;
        else
            showbits(stream, 8);
    }

    return 0;
}

unsigned int showbits(stream_t *stream, int bits)
{
    st_thor_buffer* buffer = &stream->trace.buffer;

    while (buffer->bit_pos < (unsigned int)bits)
    {
        buffer->cur_word = (buffer->cur_word << 8) | NextByte(stream);
        buffer->bit_pos += 8;
    }

    return (uint32_t)((buffer->cur_word >> (buffer->bit_pos - bits)) & ((1ULL << bits) - 1));
}

static uint8_t NextByte(stream_t* stream)
{
    st_thor_buffer* buffer = &stream->trace.buffer;
    st_thor_buffer_segment* seg = &buffer->segment[buffer->read_segment];
    uint8_t byte = 0;

    if (seg->byte_pos >= seg->segment_length)
    {
        if (!buffer->end_of_stream)
        {
            NextSegment(stream);

            seg = &buffer->segment[buffer->read_segment];
            if (seg->buf_ptr == NULL || seg->byte_pos >= seg->segment_length)
                return 0;
        }
        else
        {
            return 0;
        }
    }

    byte = seg->buf_ptr[seg->byte_pos++];

    return byte;
}

static void NextSegment(stream_t* stream)
{
    st_thor_buffer* buffer = &stream->trace.buffer;
    st_thor_buffer_segment* seg;

    int next_segment = (buffer->read_segment + 1) & 3;

    seg = &buffer->segment[next_segment];

    if (stream->trace.callback[THOR_DEC_CB_BUFFER].fn != NULL)
        buffer->end_of_stream = ((ThorCallbackBufferFn)stream->trace.callback[THOR_DEC_CB_BUFFER].fn)(stream->trace.handle, &seg->buf_ptr, &seg->segment_length, &seg->segment_id, stream->trace.callback[THOR_DEC_CB_BUFFER].param);

    if (seg->buf_ptr == NULL || seg->segment_length == 0)
    {
        buffer->end_of_stream = 1;
        if (buffer->read_segment == -1)
            buffer->read_segment = 0;
    }
    else
    {
        seg->byte_pos = 0;
        seg->bits_remaining = seg->segment_length << 3;
    
        buffer->read_segment = next_segment;
    }

    if (buffer->segment[buffer->active_segment].bits_remaining <= 0 && buffer->read_segment != buffer->active_segment)
        buffer->active_segment = (buffer->active_segment + 1) & 3;
}

