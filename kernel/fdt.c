/*
 * Simple Flattened Device Tree (FDT) printer
 * 
 * Parses and prints a device tree blob in a human-readable text format,
 * similar to the output of `dtc -I dtb -O dts`.
 */

#include "fdt.h"
#include "sbi/sbi.h"
#include "klibc.h"

/* DTB pointer, set by entry.S */
void *dtb_ptr = 0;

void *fdt_get_dtb(void)
{
    return dtb_ptr;
}

/* Check if FDT is valid */
static int fdt_check_header(void *fdt)
{
    struct fdt_header *hdr = (struct fdt_header *)fdt;
    
    if (!fdt)
        return -1;
    
    if (fdt32_to_cpu(hdr->magic) != FDT_MAGIC)
        return -1;
    
    return 0;
}

/* Print indentation */
static void print_indent(int depth)
{
    for (int i = 0; i < depth; i++)
        sbi_puts("    ");
}

/* Print a string, escaping special characters */
static void print_escaped_string(const char *s, int len)
{
    for (int i = 0; i < len && s[i]; i++) {
        char c = s[i];
        if (c == '\n')
            sbi_puts("\\n");
        else if (c == '\t')
            sbi_puts("\\t");
        else if (c == '\r')
            sbi_puts("\\r");
        else if (c == '\\')
            sbi_puts("\\\\");
        else if (c == '"')
            sbi_puts("\\\"");
        else if (c >= 32 && c < 127)
            sbi_printf("%c", c);
        else
            sbi_printf("\\x%02x", (unsigned char)c);
    }
}

/* Check if data looks like a printable string */
static int is_printable_string(const char *data, int len)
{
    if (len == 0)
        return 0;
    
    /* Must end with null terminator */
    if (data[len - 1] != '\0')
        return 0;
    
    /* Check all characters are printable or common escapes */
    for (int i = 0; i < len - 1; i++) {
        char c = data[i];
        if (c == '\0') {
            /* Embedded null - could be string list, check next part */
            continue;
        }
        if (c < 32 && c != '\n' && c != '\t' && c != '\r')
            return 0;
        if (c >= 127)
            return 0;
    }
    
    return 1;
}

/* Print property value based on heuristics */
static void print_prop_value(const char *name, const u8 *data, int len)
{
    /* Empty property */
    if (len == 0) {
        return;
    }
    
    /* Check for string or string list */
    if (is_printable_string((const char *)data, len)) {
        const char *p = (const char *)data;
        int first = 1;
        
        while (p < (const char *)data + len) {
            if (!first)
                sbi_puts(", ");
            sbi_puts("\"");
            print_escaped_string(p, len - (p - (const char *)data));
            sbi_puts("\"");
            first = 0;
            p += strlen(p) + 1;
        }
        return;
    }
    
    /* 4-byte aligned data - print as cells */
    if ((len % 4) == 0) {
        sbi_puts("<");
        for (int i = 0; i < len; i += 4) {
            u32 cell = fdt32_to_cpu(*(u32 *)(data + i));
            if (i > 0)
                sbi_puts(" ");
            sbi_printf("0x%x", cell);
        }
        sbi_puts(">");
        return;
    }
    
    /* Arbitrary bytes */
    sbi_puts("[");
    for (int i = 0; i < len; i++) {
        if (i > 0)
            sbi_puts(" ");
        sbi_printf("%02x", data[i]);
    }
    sbi_puts("]");
}

/* Main FDT printing function */
void fdt_print(void *fdt)
{
    struct fdt_header *hdr;
    u32 *p;
    const char *strings;
    int depth = 0;
    
    if (fdt_check_header(fdt) != 0) {
        sbi_puts("fdt: invalid or missing device tree\n");
        return;
    }
    
    hdr = (struct fdt_header *)fdt;
    
    sbi_printf("/dts-v1/;\n");
    sbi_printf("// FDT version %d, size %d bytes\n\n",
               fdt32_to_cpu(hdr->version),
               fdt32_to_cpu(hdr->totalsize));
    
    /* Get pointers to structure and strings blocks */
    p = (u32 *)((u8 *)fdt + fdt32_to_cpu(hdr->off_dt_struct));
    strings = (const char *)((u8 *)fdt + fdt32_to_cpu(hdr->off_dt_strings));
    
    /* Walk the structure block */
    while (1) {
        u32 token = fdt32_to_cpu(*p++);
        
        switch (token) {
        case FDT_BEGIN_NODE: {
            const char *name = (const char *)p;
            int namelen = strlen(name);
            
            print_indent(depth);
            if (namelen == 0)
                sbi_puts("/");
            else
                sbi_printf("%s", name);
            sbi_puts(" {\n");
            
            depth++;
            
            /* Advance past name (aligned to 4 bytes) */
            p += (namelen + 4) / 4;
            break;
        }
        
        case FDT_END_NODE:
            depth--;
            print_indent(depth);
            sbi_puts("};\n");
            if (depth == 0)
                sbi_puts("\n");
            break;
        
        case FDT_PROP: {
            u32 len = fdt32_to_cpu(*p++);
            u32 nameoff = fdt32_to_cpu(*p++);
            const char *name = strings + nameoff;
            const u8 *data = (const u8 *)p;
            
            print_indent(depth);
            sbi_printf("%s", name);
            
            if (len > 0) {
                sbi_puts(" = ");
                print_prop_value(name, data, len);
            }
            sbi_puts(";\n");
            
            /* Advance past data (aligned to 4 bytes) */
            p += (len + 3) / 4;
            break;
        }
        
        case FDT_NOP:
            break;
        
        case FDT_END:
            return;
        
        default:
            sbi_printf("fdt: unknown token 0x%x\n", token);
            return;
        }
    }
}
