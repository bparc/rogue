typedef struct
{
    uint8_t*bytes;
    int32_t size;
    int32_t at;
} byte_stream_t;

fn byte_stream_t MakeByteStream(uint8_t*bytes,int32_t size,int32_t start_from);
fn byte_stream_t SplitByteStream(byte_stream_t*stream,int32_t partition_size);
fn byte_stream_t ByteStreamFromFile(file_t file,int32_t start_from);

#define ByteStreamReadStruct(stream,type) (type*)ByteStreamReadBytes(stream,sizeof(type))
fn void *ByteStreamReadBytes(byte_stream_t *stream, int32_t size);
fn int32_t ByteStreamEOF(byte_stream_t*stream);

#pragma pack(push,1)

/*
field	size	type	pos	comment
lineHeight	2	uint	0	
base	2	uint	2	
scaleW	2	uint	4	
scaleH	2	uint	6	
pages	2	uint	8	
bitField	1	bits	10	bits 0-6: reserved, bit 7: packed
alphaChnl	1	uint	11	
redChnl	1	uint	12	
greenChnl	1	uint	13	
blueChnl	1	uint	14	
*/

#define BMFONT_INFO             1
#define BMFONT_COMMON           2
#define BMFONT_PAGES            3
#define BMFONT_CHARS            4
#define BMFONT_KERNING_PAIRS    5

typedef struct
{
    uint16_t line_height;
    uint16_t base;
    uint16_t scale_w;
    uint16_t scale_h;
    uint16_t pages;
    int8_t   bit_field;
    uint8_t  alpha_chnl;
    uint8_t  chnl_red;
    uint8_t  chnl_green;
    uint8_t  chnl_blue;
} bmfont_common_t;

typedef struct
{
    uint32_t id;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    int16_t  x_offset;
    int16_t  y_offset;
    int16_t  x_advance;
    uint8_t  page;
    uint8_t  chn1;
} bmfont_char_t;

typedef struct
{
    uint8_t signature[3];
    uint8_t version;
} bmfont_header_t;

typedef struct
{
    uint8_t type;
    int32_t size;
} bmfont_block_t;

#pragma pack(pop)

typedef struct
{
    char filename[32];
} bmfont_page_t;

typedef struct
{
    int32_t loaded;
    bmfont_char_t chars[256];
    bmfont_page_t pages[1];
    bmfont_common_t common;
    bitmap_t bitmap;
} bmfont_t;

fn int32_t LoadBMFont(bmfont_t*font,const char *path);