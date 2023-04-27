/**
 Got this information from the mcuboot documentation
*/

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define IMAGE_MAGIC                 0x96f3b83d

#define IMAGE_HEADER_SIZE           32

struct image_version {
    uint8_t iv_major;
    uint8_t iv_minor;
    uint16_t iv_revision;
    uint32_t iv_build_num;
};

/** Image header.  All fields are in little endian byte order. */
struct image_header {
    uint32_t ih_magic;
    uint32_t ih_load_addr;
    uint16_t ih_hdr_size;           /* Size of image header (bytes). */
    uint16_t ih_protect_tlv_size;   /* Size of protected TLV area (bytes). */
    uint32_t ih_img_size;           /* Does not include header. */
    uint32_t ih_flags;              /* IMAGE_F_[...]. */
    struct image_version ih_ver;
    uint32_t _pad1;
};


#ifdef __cplusplus
}
#endif
