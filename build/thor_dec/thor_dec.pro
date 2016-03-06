TEMPLATE = lib
CONFIG += staticlib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG -= debug_and_release

QMAKE_MAC_SDK = macosx10.11

SOURCES += \
    ../../dec/decode_block.c \
    ../../dec/decode_frame.c \
    ../../dec/getvlc.c \
    ../../dec/maindec.c \
    ../../dec/read_bits.c \
    ../../common/common_block.c \
    ../../common/common_frame.c \
    ../../common/common_kernels.c \
    ../../common/inter_prediction.c \
    ../../common/intra_prediction.c \
    ../../common/simd.c \
    ../../common/snr.c \
    ../../common/transform.c \
    ../../dec/thor_trace.c \
    ../../dec/thor_buffer.c \
    ../../dec/thor_if.c \
    ../../common/wt_matrix.c \
    ../../common/temporal_interp.c

HEADERS += \
    ../../dec/decode_block.h \
    ../../dec/decode_frame.h \
    ../../dec/getbits.h \
    ../../dec/getvlc.h \
    ../../dec/maindec.h \
    ../../dec/read_bits.h \
    ../../common/common_block.h \
    ../../common/common_frame.h \
    ../../common/common_kernels.h \
    ../../common/global.h \
    ../../common/inter_prediction.h \
    ../../common/intra_prediction.h \
    ../../common/simd.h \
    ../../common/snr.h \
    ../../common/transform.h \
    ../../common/types.h \
    ../../dec/thor_if.h \
    ../../dec/thor_trace.h \
    ../../common/wt_matrix.h \
    ../../common/temporal_interp.h

INCLUDEPATH += ../../common

win32: DEFINES += "VS_2015=1"
