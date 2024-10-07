fn byte_stream_t MakeByteStream(uint8_t*bytes,int32_t size,int32_t start_from)
{
    byte_stream_t result={0};
    result.bytes=bytes;
    result.at=start_from;
    result.size=size;
    return result;
}

fn byte_stream_t SplitByteStream(byte_stream_t*stream,int32_t partition_size)
{
    byte_stream_t result = MakeByteStream(ByteStreamReadBytes(stream,partition_size), partition_size, 0);
    return result;
}

fn byte_stream_t ByteStreamFromFile(file_t file,int32_t start_from)
{
    byte_stream_t result=MakeByteStream(file.bytes,file.size,start_from);
    return result;
}

fn void *ByteStreamReadBytes(byte_stream_t *stream, int32_t size)
{
    void *result = 0;
    if(stream->at+size<=stream->size)
    {
        result = (void*)(stream->bytes+stream->at);
        stream->at+=size;
    }
    return result;
}

fn int32_t ByteStreamEOF(byte_stream_t*stream)
{
    int32_t result=stream->at>=stream->size;
    return result;
}

fn void BMF_ReadBlockDAta(bmfont_t*font,bmfont_block_t*block,byte_stream_t stream)
{
    switch(block->type)
    {
    case BMFONT_INFO:
    {

    } break;
    case BMFONT_COMMON:
    {
        bmfont_common_t common = *ByteStreamReadStruct(&stream,bmfont_common_t);
        font->common = common;
    } break;
    case BMFONT_PAGES:
    {
        char *filename = (char *)stream.bytes;
        for(int32_t char_index=0;
            char_index<ArraySize(font->pages[0].filename);
            char_index++)
        {
            if(*filename)
            {
                font->pages[0].filename[char_index]=*filename++;
                continue;
            }
            break;
        }
    } break;
    case BMFONT_CHARS:
    {
        while(!ByteStreamEOF(&stream))
        {
            bmfont_char_t*ch=ByteStreamReadStruct(&stream,bmfont_char_t);
            if(ch)
            {
                if(ch->id<ArraySize(font->chars))
                {
                    font->chars[ch->id]=*ch;
                }
                continue;
            }
            break;
        }
    } break;
    case BMFONT_KERNING_PAIRS:
    {

    } break;
    default:
    {
        Assert(0);
    } break;
    };
}

fn int32_t LoadBMFont(bmfont_t*font,const char *path)
{
    memset(font,0,sizeof(*font));
    file_t file=OpenSystemFile(path);
    if(file.bytes)
    {
        byte_stream_t stream=ByteStreamFromFile(file,0);
        bmfont_header_t*header=ByteStreamReadStruct(&stream,bmfont_header_t);
        if(header)
        {
            while(!ByteStreamEOF(&stream))
            {
                bmfont_block_t*block=ByteStreamReadStruct(&stream,bmfont_block_t);
                if(block)
                {
                    byte_stream_t block_stream = SplitByteStream(&stream,block->size);
                    if(block_stream.bytes)
                    {
                        BMF_ReadBlockDAta(font,block,block_stream);
                        continue;
                    }
                }
                break;
            }
        }
        CloseSystemFile(&file);
    }
    // Load bitmaps.
    {
        bmfont_page_t*page=font->pages;
        char full_path[64]="";
        const char *directory="assets/";
        sprintf(full_path,"%s%s",directory,page->filename);
        font->bitmap=LoadBitmapFromFile(full_path);
    }
    return TRUE;
}