//
//  thor_if.h
//

#ifndef thor_dec_thor_if_h
#define thor_dec_thor_if_h

#include <stdint.h>
#include "maindec.h"

#define VIDEO_FORMAT_YUV400 0
#define VIDEO_FORMAT_YUV420 1
#define VIDEO_FORMAT_YUV422 2
#define VIDEO_FORMAT_YUV444 3

typedef struct THOR_OUTPUT_PICTURE {
    int32_t  s32PicOrderCount;
    uint32_t u32DecodeOrder;
    uint64_t u64PTS;
    uint64_t u64DTS;
    uint16_t u16Width[3];
    uint16_t u16Height[3];
    uint8_t* ppxPic[3];
    uint16_t u16Stride[3];
    uint8_t  u8YUVFormat;
    uint8_t  u8PixelSize;
    uint8_t  u8BitDepthLuma;
    uint8_t  u8BitDepthChroma;
} THOR_OUTPUT_PICTURE;

enum THOR_DECODING_MODE {
    THOR_DEC_MODE_FULL = 0,
    THOR_DEC_MODE_PRED = 1,
    THOR_DEC_MODE_SYNTAX = 2,
    THOR_DEC_MODE_HEADER = 3
};


#define THOR_TRACE_TYPE_ALL            0x00FFFFFF
#define THOR_TRACE_TYPE_NONE           0x00000000
#define THOR_TRACE_TYPE_SEQUENCE_HDR   0x00000001
#define THOR_TRACE_TYPE_FRAME          0x00000002
#define THOR_TRACE_TYPE_CODING_TREE    0x00000004
#define THOR_TRACE_TYPE_BLOCK          0x00000008
#define THOR_TRACE_TYPE_RESIDUAL       0x00000010
#define THOR_TRACE_TYPE_CLPF           0x00000020
#define THOR_TRACE_TYPE_ACCESSUNIT     0x00000040
#define THOR_TRACE_TYPE_MESSAGE        0x80000000

#define THOR_TRACE_EVENT_GROUP_START   1
#define THOR_TRACE_EVENT_GROUP_END     2
#define THOR_TRACE_EVENT_SYNTAX        3
#define THOR_TRACE_EVENT_MESSAGE       4


typedef void* THOR_DEC_HANDLE;

typedef void    (*ThorCallbackFrameFn)(THOR_DEC_HANDLE dec, uint32_t length, void* param);
typedef void    (*ThorCallbackCTUFn)(THOR_DEC_HANDLE dec, uint16_t xpos, uint16_t ypos, uint8_t width, uint8_t height, void* param);
typedef void    (*ThorCallbackBlockFn)(THOR_DEC_HANDLE dec, uint16_t xpos, uint16_t ypos, uint8_t width, uint8_t height, block_info_dec_t* block, void* param);
typedef void    (*ThorCallbackOutputFn)(THOR_DEC_HANDLE dec, THOR_OUTPUT_PICTURE* picInfo, void* param);
typedef int32_t (*ThorCallbackErrorFn)(THOR_DEC_HANDLE dec, char* string, void* param);
typedef void    (*ThorCallbackTraceFn)(THOR_DEC_HANDLE dec, uint32_t eventType, void* param);
typedef int32_t (*ThorCallbackBufferFn)(THOR_DEC_HANDLE dec, uint8_t** buffer, uint32_t* length, uint64_t* buffer_id, void* param);


THOR_DEC_HANDLE Thor_InitDecoder(void);
int32_t Thor_ReleaseDecoder(THOR_DEC_HANDLE h);

int32_t Thor_Decode(THOR_DEC_HANDLE h, uint8_t* buffer, int32_t len, int header);
int32_t Thor_FlushDecodedPictures(THOR_DEC_HANDLE h);

int32_t Thor_ResetDecoder(THOR_DEC_HANDLE h);
int32_t Thor_ResetCallbacks(THOR_DEC_HANDLE h);
int32_t Thor_SetDecodingMode(THOR_DEC_HANDLE h, enum THOR_DECODING_MODE mode);

int32_t Thor_SetDecodeCallbackFrame(THOR_DEC_HANDLE h, ThorCallbackFrameFn fnCallback, void* param);
int32_t Thor_SetDecodeCallbackCTU(THOR_DEC_HANDLE h, ThorCallbackCTUFn fnCallback, void* param);
int32_t Thor_SetDecodeCallbackBlock(THOR_DEC_HANDLE h, ThorCallbackBlockFn fnCallback, void* param);
int32_t Thor_SetDecodeCallbackOutput(THOR_DEC_HANDLE h, ThorCallbackOutputFn fnCallback, void* param);
int32_t Thor_SetDecodeCallbackError(THOR_DEC_HANDLE h, ThorCallbackErrorFn fnCallback, void* param);
int32_t Thor_SetDecodeCallbackTrace(THOR_DEC_HANDLE h, ThorCallbackTraceFn fnCallback, uint32_t u32SyntaxTypes, uint32_t u32GroupTypes, void* param);
int32_t Thor_SetDecodeCallbackBuffer(THOR_DEC_HANDLE h, ThorCallbackBufferFn fnCallback, void* param);

int32_t Thor_GetPicInfo(THOR_DEC_HANDLE h, THOR_OUTPUT_PICTURE* picInfo);

#endif
