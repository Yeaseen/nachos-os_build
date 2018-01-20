/* coff.h
 *   Data structures that describe the MIPS COFF format.
 */

#ifdef HOST_x86_64
typedef int DWORD;
#else
typedef long DWORD;
#endif

struct filehdr {
        unsigned short  f_magic;        /* magic number */
        unsigned short  f_nscns;        /* number of sections */
        DWORD           f_timdat;       /* time & date stamp */
        DWORD           f_symptr;       /* file pointer to symbolic header */
        DWORD           f_nsyms;        /* sizeof(symbolic hdr) */
        unsigned short  f_opthdr;       /* sizeof(optional hdr) */
        unsigned short  f_flags;        /* flags */
      };

#define  MIPSELMAGIC    0x0162

#define OMAGIC  0407
#define SOMAGIC 0x0701

typedef struct aouthdr {
        short   magic;          /* see above                            */
        short   vstamp;         /* version stamp                        */
        DWORD   tsize;          /* text size in bytes, padded to DW bdry*/
        DWORD   dsize;          /* initialized data "  "                */
        DWORD   bsize;          /* uninitialized data "   "             */
        DWORD   entry;          /* entry pt.                            */
        DWORD   text_start;     /* base of text used for this file      */
        DWORD   data_start;     /* base of data used for this file      */
        DWORD   bss_start;      /* base of bss used for this file       */
        DWORD   gprmask;        /* general purpose register mask        */
        DWORD   cprmask[4];     /* co-processor register masks          */
        DWORD   gp_value;       /* the gp value used for this object    */
      } AOUTHDR;
#define AOUTHSZ sizeof(AOUTHDR)
 

struct scnhdr {
        char            s_name[8];      /* section name */
        DWORD           s_paddr;        /* physical address, aliased s_nlib */
        DWORD           s_vaddr;        /* virtual address */
        DWORD           s_size;         /* section size */
        DWORD           s_scnptr;       /* file ptr to raw data for section */
        DWORD           s_relptr;       /* file ptr to relocation */
        DWORD           s_lnnoptr;      /* file ptr to gp histogram */
        unsigned short  s_nreloc;       /* number of relocation entries */
        unsigned short  s_nlnno;        /* number of gp histogram entries */
        DWORD           s_flags;        /* flags */
      };
 
