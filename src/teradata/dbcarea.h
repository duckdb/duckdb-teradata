#ifndef DBCAREA_H
#define DBCAREA_H
#define DBCAREA_H_REV "20.00.00.01"
/*******************************************************************************
 *  TITLE:       DBCAREA.H .. CLIv2 type definitions for "C"         20.00.00.01
 *
 *  Purpose      To contain the type definitions needed for
 *               interfacing to CLIv2 from a "C" program.
 *
 *  Copyright 1987-2024 by Teradata Corporation.  All rights reserved.
 *
 *  Description  This include file contains the type
 *               definitions needed for an application program
 *               to make calls to CLIv2.
 *
 *  History
 *
 *    F.1    87Nov23  DRP  DR 11416 Coded
 *    F.2    88Jan25  KKE  DR11889  Fix 4  compatiblity
 *                                  w/ other DBCAREAs
 *    F.3    88Feb26  DRP  DR11889  add a ;
 *    G0_00  88Jul25  SN   Created  for Release G.0
 *    G0_01  88Oct06  WBJ  DCR4029 & other G.0 mods
 *    G0_02  88Oct26  WBJ  DCR4029  Fix dupe element
 *    G0_03  89Dec15  KXO  DCR5239  Add DBCAREA EXT(ParamSQL)
 *           90Jan15  KXO  DR18467  DirectReq (HUT interface)
 *    G0_04  90May07  KBC  DR19675  Add CLI prototypes
 *    G0_05  90Sep14  KXO           Revised for UNIX TOOL KIT
 *    G0_06  90Nov13  KBC  DR20832  Add Record_Error
 *    G0_07  90Dec18 HEIDI DR21134  Remove Record_Error (put in cliv2.c)
 *    G0_08  91Jan05  DJL  DCR5366  Tandem Interface
 *    G0_09  91May02 HEIDI DR21506  ifdef out prototyping for PP2(II)
 *    G0_10  91Dec01  BGN  DCR6174  First revision for OS/2
 *    G0_11  91Dec31  KBC  DCR5966  Support for Windows WinCLIV2.DLL
 *    H0_00  93May20  SNT  DCR6711  Created for Release H.0
 *    H0_01  94Apr04  JAE  DCR6172  Merged 2PC code for Genesis II
 *    H0_02  94May13  JAE  DR29062  Changed give_msg to conf_logoff
 *    H0_03  94Jun29  JAE  DR29062  Added give_msg back.
 *    H0_04  95Mar08  JAE  DCR6949  #define for delay options, AP Reset.
 *    H0_05  95Mar15  ALB  DR32387  added connect_wait to CLIV2options.
 *    H0_06  95Mar28  JAE  DCR6847,6848  NDM Support.
 *    H0_07  95Mar28  JAE  DR24722  This change included this DR as well
 *                                  as other mainframe changes. H0_07 is
 *                                  used here to label the code merge.
 *    H3_00  95Aug03  MDV           Merged NDM code and other fixed DRs.
 *                                  Added sess_2pc_mode by JAE.
 *                                  Added DBF2PC function code by JAE.
 *                                  Added init_with_2pc by JAE.
 *                                  Added DBFIWPF, DBFRECV, DBFALSC,
 *                                  DBFCMD function codes by JAE.
 *                         DR24722  Cleanup prototypes for IBM370 by AFK.
 *                         DCR6669  Added NP changes G0_10 - G0_12 by RPA.
 *                         DR29485  ActivityCount and QUIET by AFK.
 *                         DCR6847  NDM support by JAE.
 *    H3_01  95Aug14  TH4  DR33969  Added extern "C" for C++ linker.
 *    H3_02  95Aug23  JAE  DR32294  Removed #ifdef NDM .
 *    H3_03  96Mar08  SYY  DR35631  Merge WinCLI 3.0 changes into mainstream.
 *
 *    04.00.00.01 TH4 96Jul12 DR36785 Version number change.
 *    04.02.00.00 JTH July97  DR40000 Code merge
 *    04.02.00.01 TH4 97Sep05 DR36456 64KB row support.
 *    04.02.00.02 TH4 97Sep10 DR40417 ANSI Date/Time Support.
 *    04.04.00.00 JTH 98Feb26 DR41392 consolidate this header file
 *                For Windows use def PCDOS/PCDOSWIN/WIN32
 *                For IBM mainframe use def IBM370,I370
 *                Unix is the default
 *    04.04.00.01 DHF 98Jun12 DR41392 Updated to add WIN32 also.
 *
 * Revision    Date       DCR   DID      Comments
 * ----------- --------   ----- -------- ---------------------------------------
 * 04.04.00.02 08/11/1998 43449 TH4      Fix the pointer to context area
 * 04.05.00.00 01/18/2000 45724 CSG      CLI2 C++ safe
 * 04.05.00.01 03/03/2000 48735 CSG      TDSP Support
 * 04.05.00.02 08/04/2000 48823 ASG      OS/390 USS
 * 04.06.01.00 05/25/2001 55267 CSG      Native 64bit support
 * 04.06.01.01 09/03/2001 52050 ASG      Multi-threaded support.
 * 04.06.01.02 11/05/2001 58769 ASG      Support TWB operators with OS/390 C/C++
 * 04.06.02.00 03/27/2002 58353 CSG      CLI Sub-Second Timing Capability
 * 04.07.00.00 05/08/2002 58369 CSG      Native 64 bit support to CLIv2 on HPUX
 * 04.07.00.01 05/18/2002 58915 CSG      Native 64 bit support to CLIv2 on AIX.
 * 04.07.00.02 06/06/2002 59337 CSG      Port CLI to 64 Bit Windows XP.
 * 04.07.00.03 06/17/2002 61637 CSG      Support enlarged parcel usage.
 * 04.07.00.04 06/03/2002 57583 ASG      LOB support
 * 04.07.00.05 08/22/2002 52059 CSG      L10N support
 * 04.07.00.06 09/17/2002 64147 CSG      JDBC compilation fails on AIX.
 * 04.07.01.00 03/31/2003 60695 ASG      Cursor positioning.
 * 04.07.01.01 04/08/2003 67881 cc151010 Remove DBCHQE() prototype.
 * 04.07.01.02 04/03/2003 66712 ASG      Data encryption.
 * 04.07.01.03 04/24/2003 68084 mg180007 changed TimeStamp to time_stamp
 * 04.08.00.00 07/22/2003 68511 mg180007 clean up defines, prototypes
 *                        68139          self-sufficient headers
 *                        68140          fix EXPENTRY definition
 * 04.08.00.01 12/02/2003 68838 ASG      Array Support for DML Operations
 * 04.08.00.02 09/28/2003 68835 ASG      APH / 1 MB Responses
 * 04.08.00.03 09/29/2003 69228 CSG      Support for character object state
 * 04.08.00.04 02/10/2004 85431 ASG      Universalize headers
 * 04.08.00.05 02/10/2004 58352 ASG      ESS
 * 04.08.00.06 01/27/2004 84603 CSG      Add pack(1) for Win-32 platform
 * 04.08.00.07 05/14/2004 68842 mg180007 TTU 8.0 Security support
 * 04.08.00.08 06/01/2004 87586 ASG      Fix linkage specifier for TWB
 * 04.08.00.09 08/09/2004 89199 DRL      Re-add fix for DR83781
 * 04.08.00.10 09/17/2004 89408 ASG      Fix 64-bit problem with array ops
 * 04.08.00.11 12/06/2004 91963 ASG      Correct 32- vs. 64-bit discrepancy
 * 04.08.01.00 02/14/2005 92232 mg180007 merge and update copyright year
 * 04.08.01.01 03/01/2005 90995 mg180007 Support Unicode credentials
 * 04.08.01.02 04/21/2005 93472 mg180007 Added header revision info
 * 04.08.01.03 06/23/2005 94622 ASG      dual-component feature support
 * 04.08.01.04 08/11/2005 97745 fs185009 DBCHSAD supporting 64-bit pointers
 * 04.08.02.00 12/15/2005 98024 bh185000 Support for MaxDecimalPrecision
 *                        98025 bh185000 Support for AGKR_option
 *                        98026 bh185000 Support for StatementInfo
 * 04.08.02.01 02/14/2006 102061 ASG      reconfig for 80-byte lines
 * 04.09.00.00 05/30/2006 102528 bh185000 Support for DynamicResultSets
 * 04.09.00.01 07/11/2006 101287 mg180007 Support for Default Connection
 * 04.09.00.02 11/02/2006 101663 mg180007 Added send_delegated_credentials
 * 04.09.00.03 11/28/2006 108909 bh185000 Added workload_len and workload_ptr
 * 12.00.00.04 11/29/2006 108552 fs185009 version change from 4.09.x to 12.0.x
 * 12.00.00.05 02/06/2007 110710 bh185000 Use low-order byte to keep MF happy
 * 13.00.00.00 01/09/2008 117140 mg180007 copyright update
 * 13.01.00.00 11/17/2008 116354 bh185000 Support Extended Object Name Parcel
 * 13.01.00.01 12/09/2008 123516 bh185000 Support StmtInfo UDT Transforms Off
 * 13.01.00.02 12/18/2008 122304 bh185000 Support Trusted Sessions Security
 * 13.01.00.03 03/29/2009 126915 bh185000 Support TASM Utility Management
 * 13.10.00.00 06/23/2009 133468 kl185018 Version change from 13.01 to 13.10
 * 14.00.00.00 10/06/2010 121202 bh185000 Support Statement Independence
 * 14.00.00.01 12/15/2009 118581 mg180007 Added logon_timeout
 * 14.00.00.02 02/15/2011 145358 bh185000 Added automaticRedrive
 * 14.00.00.03 02/24/2011 145224 bh185000 Support Array Data Type
 * 14.10.00.00 01/10/2012 CLAC-28875 bh185000 Support XML Data Type
 * 14.10.00.01 03/22/2012 CLAC-28857 mg180007 Support ESS option 'D'
 * 14.10.00.02 05/16/2012 CLAC-29283 bh185000 Support Extended Multiload
 * 14.10.00.03 07/20/2012 CLAC-29878 hs186016 Fix comment in D8XIELEM structure
 * 14.10.00.04 10/24/2012 CLAC-29918 hs186016 Remove invalid request_mode 'D'
 * 14.10.00.05 02/21/2013 CLAC-30478 hs186016 Support TASM FastFail
 * 15.00.00.00 11/14/2013 CLAC-30554 hs186016 Support SLOB
 * 15.10.00.00 04/28/2014 CLAC-32368 mg180007 version and copyright update
 * 15.10.00.01 07/30/2014 CLAC-31559 bh185000 SLOB Server to Client support
 * 15.10.00.02 08/06/2014 CLAC-32067 hs186016 Support timer in DBCHCL
 * 15.10.00.03 09/12/2014 CLAC-32071 hs186016 Support User Selectable directory
 * 16.00.00.00 06/24/2015 CLAC-29115 bh185000 Additional fix for 1MB resp row
 * 16.00.00.01 07/14/2015 CLAC-33706 hs186016 Fix DBCHWL timer upon DBS crash
 * 17.00.00.00 11/13/2018 CLIWS-7163 hs186016 version and copyright update
 * 17.10.00.00 06/08/2020 CLIWS-7550 hs186016 Support TLS 1.2
 * 17.10.00.01 07/14/2020 CLIWS-7581 hs186016 Support HTTP/WebSocket
 * 17.10.00.02 08/13/2020 CLIWS-7604 hs186016 Support sslmode: allow and prefer
 * 17.10.00.03 01/26/2021 CLIWS-7635 rp255060 Connection string support
 * 17.10.00.04 06/03/2021 CLIWS-7751 hs186016 Support mainframe on z/OS
 * 17.20.00.00 01/01/2022 CLIWS-8198 hs186016 version and copyright update
 * 17.20.00.01 07/12/2022 CLIWS-8233 rp255060 OCSP support
 * 17.20.00.02 08/10/2022 CLIWS-8471 rp255060 Disable OCSP until ready for release
 * 17.20.00.03 12/01/2022 CLIWS-8614 rp255060 Support CRL on non-z/OS
 * 20.00.00.00 05/11/2023 CLIWS-8696 hs186016 version and copyright update
 * 20.00.00.01 06/27/2024 CLIWS-9273 rp255060 Add o_port to DBCAREA
 ******************************************************************************/

/***************************************************************/
/*                DBCAREA  EXTENSION  structure                */
/*                            for                              */
/*                      Parameterized SQL                      */
/***************************************************************/

/***************************************************************/
/* DBCAREA Extension structure                                 */
/*     DBCAREAX is an extension to the DBCAREA and is of       */
/*     variable length. A field (extension_pointer) in the     */
/*     DBCAREA points to the DBCAREAX. DBCAREAX contains 2     */
/*     logical sections: header & repeatable elements.         */
/*                                                             */
/*     Applications with 64bit addressing should use D8CAIRX   */
/*     and related structures(begining with d8).               */
/*                                                             */
/***************************************************************/

/* DR85431 --> */
#include "coptypes.h" /* DR68139 */
/* <-- DR85431 */

typedef struct D8CAIRX {             /* DR55267: 64 bit support*/
    char             d8xiId[4];      /* Eyecatcher, predefined */
                                     /* string "IRX8"          */
/* DR58369: 64 bit support on HPUX */
/* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    Byte             d8xiNxt4[4];    /* Reserved: must be zero */
#else
    struct D8CAIRX*  d8xiNext;       /* Next-extension pointer */
#endif
    UInt32           d8xiSize;       /* Length of extension    */
    Byte             d8xiLvl;        /* Area's level           */
    Byte             d8xiFil1[3];    /* Reserved: must be zero */
/* DR58369: 64 bit support on HPUX */
/* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    struct D8CAIRX*  d8xiNext;       /* Next-extension pointer */
#else
    Byte             d8xiNxt8[8];    /* Reserved: must be zero */
#endif
} D8CAIRX;
                                                   /*->DR61637 */
/***************************************************************/
/*  DR61637: Extension element                                 */
/*  Two new structures are defined for supporting APH.         */
/*  Use of structure elements D8XILMNT and D8XILPTR defined by */
/*  64-bit implementation is  deprecated.                      */
/*  The two new element structures can be used to delivar both */
/*  APH and Non APH parcels to CLI.                            */
/*  These are compatible for both 32 and 64 bit modes          */
/***************************************************************/

/***************************************************************/
/* D8XIELEM:  Header for the parcel element                    */
/* dbxieLen,  should be used to specify the total length of    */
/*            the element.                                     */
/*            Inline Method (i.e. d8xiepPt is null)            */
/*            d8xieLen = sizeof (D8XIELEM)  + sizeof(D8XIEP)   */
/*                            + sizeof (parcel data)           */
/*            Pointer Method (i.e. d8xiepPt is not null)       */
/*            d8xieLen = sizeof (D8XIELEM)  + sizeof(D8XIEP)   */
/***************************************************************/

typedef struct D8XIELEM {
    UInt16  d8xieLen;       /* Length of element               */
    UInt16  d8xieTyp;       /* Type of element                 */
                            /* 0 - Use of non APH              */
                            /* 1 - Use APH                     */
} D8XIELEM;

/***************************************************************/
/* D8XIEP:   Parcel element                                    */
/* d8xiepPt,                                                   */
/*           If it is zero or null, the parcel body data       */
/*           is contained within the element beginning after,  */
/*           and contiguous to, this element.                  */
/*           If not zero or null, the body data is addressed   */
/*           by it.                                            */
/*                                                             */
/*           If the length of the element would exceed 65535,  */
/*           then the pointer must be used to address the body */
/*           data.                                             */
/***************************************************************/

/* DR84603: Added pack(1) for Win-32 */

#if defined (_WIN64) || defined (WIN32)

#pragma pack(push,1)

#endif

typedef struct D8XIEP {
    UInt16    d8xiepF;       /* Parcel flavor                  */
    UInt16    d8xiepR1;      /* Reserved: must be zero         */
    UInt32    d8xiepLn;      /* Length of body                 */
                             /* if d8xiepPt NULL               */
                             /*  set d8xiepLn to 0             */
                             /* else                           */
                             /*  set d8xiepLn to body length   */
    Byte      d8xiepA[4];    /* Reserved: must be zero         */
#ifdef CLI_64BIT
    Byte      d8xiepP4[4];   /* Reserved: must be zero         */
#else
    char*   d8xiepPt ;       /* Pointer to body,if not inline  */
#endif
#ifdef CLI_64BIT
    char*    d8xiepPt;       /* Pointer to body,if not inline  */
#else
    Byte    d8xiepP8[8];     /* Reserved: must be zero         */
#endif
    Byte   d8xiepL8[8];      /* Reserved: must be zero         */
} D8XIEP;


/* DR84603: pop packing for Win-32 */
#if defined (_WIN64) || defined (WIN32)

#pragma pack(pop)

#endif

/* Value for d8xiId (EYE-CATCHER)                              */
static char D8XIIIRX[4] = {'I','R','X','8'};

/* Values for d8xiLvl                                          */
#define D8XIL64  ((Byte) 0)    /*64bit                         */
#define D8XILERS ((Byte) 1)    /*Element restructure           */

/* Value for d8xieTyp                                          */
#define D8XIETP ((UInt16) 1)   /* Parcel element               */

/* USE OF THE BELOW DEFINED EXTENSION ELEMENT STRUCTURES IS    */
/* DEPRECATED                                                  */
                                                   /* DR61637<-*/

/**********************************************************************/
/* Extension element                                                  */
/*   One or more elements follow the header. If d8xilTyp is less then */
/*   4096, the element describes a parcel while greater values might  */
/*   be used for other types of elements, none are currently defined. */
/*                                                                    */
/* Extension parcel-element pointer                                   */
/*    If high order bit is not set, the element is a parcel  whose    */
/*    header consists of dbxilTyp and dbxilLen and whose data         */
/*    immediately follows. If D8XILTP is set, the element describes   */
/*    the length of and pointer to the parcel data whose flavor is    */
/*    specified in d8xilTyp.                                          */
/**********************************************************************/

typedef struct D8XILMNT {            /* DR55267: 64 bit support */
    UInt16   d8xilTyp;               /* Type of element         */
    UInt16   d8xilLen;               /* Length of element       */
} D8XILMNT;

typedef struct D8XILPTR {             /* DR55267: 64 bit support */
    Byte     d8xilpF2[2];             /* Reserved: must be zero  */
 /* DR58369: 64 bit support on HPUX */
 /* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    Byte     d8xilpP4[4];             /* Reserved: must be zero  */
#else
    char     d8xilpPt[4];             /* Pointer to parcel-data  */
#endif
    UInt32   d8xilpF3[4];             /* Reserved: must be zero  */
    UInt32   d8xilpLn;                /* Length of parcel-data   */
    Byte     d8xilpA[4];              /* Reserved: must be zero  */
 /* DR58369: 64 bit support on HPUX */
 /* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    char     d8xilpPt[8];             /* Pointer to parcel-data  */
#else
    Byte     d8xilpP8[8];             /* Reserved: must be zero  */
#endif
} D8XILPTR;

 /* DR58369: 64 bit support on HPUX */
 /* DR58915: Native 64bit support for AIX */
#ifndef CLI_64BIT
typedef struct DBCAREAX
{
    char               x_eyecatcher[4]; /* predefined string  'DBCX'    */
    struct DBCAREAX*   x_next_ptr;      /* to build a list if necessary */
    short              x_total_length;  /* header & sum of all elements */
    short              x_reserved;      /* future use                   */
    char               x_elements;      /* start of elements            */
} DBCAREAX;

/*****************************************************************/
/*  The header information is followed by at least one variable  */
/*  DBCAREA EXTENSION ELEMENT.                                   */
/*                                                               */
/*  There are two different types of elements: PARCEL DATA       */
/*  ELEMENT & PARCEL POINTER ELEMENT. Parcel Data Element        */
/*  consists of Element Type Code, Length and Variable Length    */
/*  Data. The high order bit of Type Code is not set.            */
/*                                                               */
/*  Parcel Pointer Element consists of Element Type Code, Element*/
/*  Length, Parcel Data Length & Parcel Data Pointer. The high   */
/*  order bit of Type Code is set for Parcel Pointer Element.    */
/*  The value of low order 15bits of Type Code must be less      */
/*  than 4096.                                                   */
/*****************************************************************/

/*****************************************************************/
/*           DBCAREA EXTENSION DATA ELEMENT structure            */
/*****************************************************************/

typedef struct  x_element
{
    short      x_elm_type;           /* pcl body or pcl ptr type*/
    short      x_elm_length;         /* total length of element */
    char       x_elm_data;           /* first byte of pclbody   */
} x_element;


typedef struct  xp_element
{
    short      x_elm_type;           /* pcl body or pcl ptr type*/
    short      x_elm_length;         /* total length of element */
    short      x_pelm_len;           /* length of parcel data   */
    char       x_pelm_addr[4];       /* address to parcel data  */
                                     /*   char array to avoid 2 */
                                     /*   compiler padding bytes*/
} xp_element;

#endif /* CLI_64BIT */

/* CLAC-30554 --> */
/***************************************************************/
/*  Smart LOB (SLOB) Support                                   */
/***************************************************************/
typedef struct SLOB_data_struct
{
    UInt16     version;               /* version of the implementation      */
    UInt16     unused;                /* always 0 (zero)                    */
    UInt32     max_num_SLOBs_in_row;  /* Maximum # of SLOBs in a row        */
#ifndef CLI_64BIT
    char       SLOB_data_reserved[4];
#endif
    char       ***SLOB_data;          /* 2-D array of pointers to SLOB data */
#ifndef CLI_64BIT
    char       SLOB_ParameterOrderNo_reserved[4];
#endif
    UInt32     *SLOB_ParameterOrderNo;/* ParameterOrderNo of SLOBs          */
} SLOB_data_struct;
/* <-- CLAC-30554 */


/***************************************************************/
/*  Main DBCAREA structure                                     */
/***************************************************************/

typedef struct DBCAREA
{

/***************************************************************/
/*  Header Information                                         */
/***************************************************************/

    char       eyecatcher[8];
    Int32      total_len;

/***************************************************************/
/*  General inputs                                             */
/***************************************************************/
    Int32       func;             /* requested function        */
    Int32       i_sess_id;        /* connection id             */
    Int32       i_req_id;         /* request number            */
    Int32       req_buf_len;      /* desired length of req buf */
    Int32       resp_buf_len;     /* desired length of rsp buf */
    Int32       max_num_sess;     /* desired mas sessions      */
    Int32       token;            /* use request token         */

/***************************************************************/
/*  Values for Function Field                                  */
/***************************************************************/

#define         DBFCON   1        /* connect a DBC session     */
#define         DBFDSC   2        /* LOFOFF (disconnect) )     */
#define         DBFRSUP  3        /* perform runstartup        */
#define         DBFIRQ   4        /* initiate request          */
#define         DBFFET   5        /* fetch response data       */
#define         DBFREW   6        /* rewind & fetch            */
#define         DBFABT   7        /* abort last request        */
#define         DBFERQ   8        /* end last request          */
#define         DBFPRG   9        /* directed request          */
/* Added 2PC Function Codes - H0_01, JAE                       */
#define         DBF2PC  10        /* Added for 2PC, JAE        */
#define         DBFIWPF 11        /* Added for 2PC, JAE        */
#define         DBFRECV 12        /* Added for 2PC, JAE        */
#define         DBFALSC 13        /* Added for 2PC, JAE        */
#define         DBFCMD  14        /* Added for 2PC, JAE        */
#define         DBFCRQ  15        /* Added for LOB DR57583     */

#define         DBFMAX   DBFCRQ   /* DAA 87MAR12 DR9522        */
                                  /* Updated DBFMAX to DBFCMD  */
                                  /* H0_01, JAE                */

/***************************************************************/
/*  General outputs                                            */
/***************************************************************/
    Int32       return_cd;
    Int32       o_sess_id;
    Int32       o_req_id;
    char        o_dbcpath[8];
    Int32       o_dbc_sess_id;
    short       o_host_id;
    char        sess_status;
    char        fill_1;
    Int32       tdp_req_no;
    Int32       cur_req_buf_len;
    Int32       cur_resp_buf_len;

/***************************************************************/
/*  Function Argument and Information Structures               */
/*  The following functions use general input/output           */
/*  arguments:                                                 */
/*              DBFDSC                                         */
/*              DBFRSUP                                        */
/*              DBFABT                                         */
/*              DBFERQ                                         */
/***************************************************************/

/***************************************************************/
/*  The connect function sub structures.                       */
/***************************************************************/

    char      i_dbcpath[8];
    /* DR55267: Native 64bit support         */
    /* DR58369: Native 64bit support on HPUX */
    /* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    char      logon_ptr_4[4];    /* Reserved */
#else
    char      *logon_ptr;        /* For 32 bit mode */
#endif
    Int32     logon_len;
    /* DR55267: Native 64bit support         */
    /* DR58369: Native 64bit support on HPUX */
    /* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    char      run_ptr_4[4];  /* Reserved */
#else
    char      *run_ptr;      /* For 32 bit mode */
#endif
    Int32     run_len;

/***************************************************************/
/*  The IRQ function sub structures.                           */
/***************************************************************/

    /* DR55267: Native 64bit support         */
    /* DR58369: Native 64bit support on HPUX */
    /* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    char      req_ptr_4[4];      /* Reserved */
#else
    char      *req_ptr;
#endif                    /* For 32 bit mode */
    Int32     req_len;
    /* DR55267: Native 64bit support         */
    /* DR58369: Native 64bit support on HPUX */
    /* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    char      using_data_ptr_4[4];   /* Reserved */
#else
    char      *using_data_ptr;   /* For 32 bit mode */
#endif
    Int32     using_data_len;        /* DR68838 */
    short     msgclass;       /*  DirectReq DR18467            */
    short     msgkind;        /*        .                      */
    char      mailbox[6];     /*        .                      */
    short     reserved;
    Int32     open_reqs;

/***************************************************************/
/*  The Fetch function sub structures.                         */
/***************************************************************/

   /************************************************************/
   /*  Input or output arguments work as  mode operations:     */
   /*                                                          */
   /*     Move mode   - Application inputs address of and max  */
   /*                   length of return area. CLI outputs data*/
   /*                   and actual length of returned data.    */
   /*     Locate mode -                                        */
   /*     Parcel mode - CLI outputs address and length & flavor*/
   /*                   for this parcel                        */
   /*     Buffer mode - CLI outputs address and length & flavor*/
   /*                   for first parcel in response buffer    */
   /************************************************************/

    Int32     fet_max_data_len;
    /* DR55267: Native 64bit support         */
    /* DR58369: Native 64bit support on HPUX */
    /* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    char      fet_data_ptr_4[4];         /* Reserved */
#else
    char      *fet_data_ptr;             /* For 32 bit mode */
#endif
    Int32     fet_ret_data_len;          /* DR11889           */
    Int32     fet_parcel_flavor;
    unsigned char fet_error_ind;         /* H0_07             */
    char       fill_2[3];

/***************************************************************/
/*  Time Stamps from various phases of operation.:for VM & VMS */
/***************************************************************/
/* changed timestamp fields to be unsigned                     */
                                        /* -> DR58353          */

    char       hsisvc_time[8];          /* unused in NW-CLI    */
    UInt32     tdpwait_time;            /* when req. is sent   */
                                        /* to MTDP             */
    UInt32     tdpdbo_time;             /* unused in NW-CLI    */
    UInt32     tdpdbi_time;             /* unused in NW-CLI    */
    UInt32     tdpxmm_time;             /* unused in NW-CLI    */
    UInt32     srbsched_time;           /* First response      */
                                        /* recieved by CLI     */

                                        /* DR 58353     <-     */

/***************************************************************/
/* Pointer to International Character Set Code or Name DCR4029 */
/***************************************************************/
    /* DR55267: Native 64bit support         */
    /* DR58369: Native 64bit support on HPUX */
    /* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    char       inter_ptr_4[4];   /* Reserved */
#else
    char       *inter_ptr;     /* DCR4029 */ /* For 32 bit mode */
#endif

/***************************************************************/
/*  Time Stamps for UNIX, VAX/VMS, and MS/DOS                  */
/***************************************************************/

    time_stamp        mtdp_sent;         /* DR11889, DR68084   */
    time_stamp        mtdp_received;     /* DR11889, DR68084   */
    char              fill_3[16];        /* DCR4029            */
    /* DR55267: Native 64bit support         */
    /* DR58369: Native 64bit support on HPUX */
    /* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    char             extension_pointer_4[4];   /* Reserved */
#else
    void             *extension_pointer; /* DCR5239 Param. SQL */
#endif


/***************************************************************/
/*  Options Flags.  Character flags are used for portability.  */
/*  Legal values are 'Y' or 'N' unless specified otherwise.    */
/***************************************************************/

    char       change_opts;
    char       resp_mode;          /* Record 'R', Field 'F', or */
                                   /* Indicator 'I' mode for    */
                                   /* returned data, 'M' for    */
                                   /* multipart indicator mode  */
    char       use_presence_bits;
    char       keep_resp;

/* H0_04 delay options for AP Reset */
#define wait_across_delay wait_across_crash
#define tell_about_delay tell_about_crash

    char       wait_across_crash;
    char       tell_about_crash;
    char       connect_wait;        /* H0_05, DR32387-ALB    */
    char       loc_mode;
    char       var_len_req;
    char       var_len_fetch;
    char       save_resp_buf;
    char       two_resp_bufs;
    char       ret_time;
    char       parcel_mode;
    char       wait_for_resp;
    char       req_proc_opt;        /* prepare 'P', execute 'E',*/
                                    /* Parameterized SQL 'S'    */
    char       msg_security;        /* DR11889                  */

                                    /* Following three options  */
                                    /* added for DCR 4029       */
    char       charset_type;        /* char set code or name    */
                                    /* 'C'code, 'N'name provided*/
    char       connect_type;        /* Host Run or Connect Logon*/
                                    /* 'R' run logon,           */
                                    /* 'C' connect logon(used on*/
                                    /*                   host)  */
    char       request_mode;        /* options for req handling */
                                    /* 'P' parcelmode,          */
                                    /* 'B' buffermode,          */
                                    /* 'D' descriptor list      */
/* Added 2PC Options - H0_01, JAE                              */
    char       sess_2pc_mode;       /* 'Y' or 'N' for 2 PC Mode */
    char       init_with_2pc;       /* 'T','N','V' for DBFIWPF  */
                                    /* ('V' is mainframe only)  */
#define iwpf_function init_with_2pc
                                    /* H3_02                    */
                                    /* H0_06,  NDM Support      */
    char       tx_semantics;        /* Transaction Semantics    */
                                    /* 'D' server default       */
                                    /* 'T' Teradata DBS         */
                                    /* 'A' ANSI                 */
    char       lang_conformance;    /* Language Conformance     */
                                    /* 'N' None                 */
                                    /* '2' FIPS127-2            */
                                    /* '3' FIPS127-3            */

/***************************************************************/
/*  Optional Message.                                          */
/***************************************************************/

    char       fill_4[2];
    short      msg_len;
    char       msg_text[76];
    Int32      route_desc_codes;
    char       fill_5[16];         /* DR36456 */
    char       fill_6[2];          /* DR36456, DR40417 */
    char       date_form;          /* DR40417 */
    char       maximum_parcel;     /* DR36456 */
    char       dbcniLid[2];        /* DR52059, Language ID     */
                                   /* EN - English             */
                                   /* JA - Japanese            */

    char       fill_8[2];          /* DR47056, On Mainframes   */
    char       dbriSeg;            /*  TDSP Field, DR48735     */
                                   /* 'N' Default              */
                                   /* 'F' First segment        */
                                   /* 'I' Intermediate segment */
                                   /* 'L' Last segment         */
                                   /* 'C' Cancel Request       */

                                   /* DR 57583 (LOB)...        */
        char       return_object;  /* Return Objects as...     */
                                   /* 'D' Data                 */
                                   /* 'T' TX-related locators  */
                                   /* 'S' Spool-related locator*/
    #define dbriROb return_object

                                   /* DR 57583 (LOB)...        */
    char       continuation_code;  /* Continue data            */
                                   /* 'F' First continuation   */
                                   /* 'I' Intermediate cont.   */
                                   /* 'L' Last/only cont.      */
                                   /* 'C' Cancel continuation  */
    #define dbniCnt continuation_code

    char      data_encryption;     /* DR 66712...              */
                                   /* 'Y' Use encryption       */
                                   /* 'N' Use clear text       */
    #define dbriDEn data_encryption

    char      fill_8a;             /* reserved  - DR68835 */

    char statement_status;         /* DR 58352 Statement-Status    */
    #define       dbcniSS statement_status
                                   /* 'O' Original                 */
                                   /* 'E' Enhanced                 */
                                   /* 'D' Description, CLAC-28857  */

    char       continued_char_state;  /*continued-characters-state*/
                              /* 'L'Shift State across messages */
                              /* 'U'Unlock Shift state for each */
                              /*    message, if Multiindicator  */
                              /* parcel spans more than 1 message*/
    #define     dbcriCCS continued_char_state


    char       consider_APH_resps; /* DR68835, APH Responses */
                                /* 'Y' OK to receive APH responses */
                                /* 'N' Don't send APH responses */
    #define    dbriAPH consider_APH_resps

    char       return_statement_info; /* DR98026, StatementInfo */
    char       return_identity_data;  /* DR98025, AGKR_option   */
/*  char       fill_9[1];          DR98025: used all fill area  */
                                /* DR98026: changed from 2 to 1 */
                                /* DR68835: changed from 3 to 2 */
                                /* DR69228: changed from 6 to 3 */
                                /* DR68835: changed from 6 to 5 */
                                /* DR66712: changed from 7 to 6 */
                                   /* DR57583 */
                                   /* DR58353: changed fill9   */
                                   /* from 3 to 21             */

    UInt16     positioning_stmt_number; /* statement number         */

#define  dbfiPSNm positioning_stmt_number

/* DR83781 & DR89199 */
#ifdef WIN32
   unsigned __int64 positioning_value;   /* row position or BLOB/CLOB offset */
#define dbfiPVal positioning_value
#else
#ifdef   _NO_LONGLONG
   unsigned long         positioning_value_hw;
   unsigned long         positioning_value_lw;
#else
   unsigned long long positioning_value;   /* row position or LOB/CLOB offset */
#define dbfiPVal positioning_value
#endif
#endif

    UInt16     positioning_action; /* action:                       */
                                   /* 0 - position to next parcel   */
                                   /* 1 - position to specified row */
                                   /* 2 - position to BLOB offset   */
                                   /* 3 - position to CLOB ofset    */

#define dbfiPAct positioning_action

#define   DBFIPANP    0		   /*   -Next parcel               */
#define   DBFIPASR    1        /*   -Statement m row n         */
#define   DBFIPASB    2		   /*   -Stmt m byte-offset n      */
#define   DBFIPASC    3        /*   -Stmt m char-offset n      */
#define   DBFIPAMX    3        /*    (Maximum value)           */

/* <--- DR60695 */


    Int16      dbciTSP;            /* DR58353: Field for app.  */
                                   /* to mention time. precis. */
    char       dbclevel;           /* DR55267: binary 1,if enlarged */
                                   /* DR58353: changed offset   */
                                   /* of dbclevel from 352-372  */
/* CLAC-30554 --> */
#define   DBCLEXP1    1        /* First expansion  */
#define   DBCLEXP2    2        /* Second expansion */
#define   DBCLVLC     2        /* Current level    */
/* <-- CLAC-30554 */
    char       dbcoMsgC;           /* Unused                    */
    Int16      dbcoMsgR;           /* DR52059, Msg Return code  */
                                   /* the error message returns */
                                   /* 0 - no error occured      */
                                   /* 1 - if msg is truncated   */
    Int16      dbcoMsgL;           /* DR52059, MessageLength    */
                                   /* retuned in Area provided  */
    Int16      dbciMsgM;           /* DR52059, Msg Area Length  */
                                   /* provided by the app.      */
    /* DR55267: Native 64bit support         */
    /* DR58369: Native 64bit support on HPUX */
    /* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    char       dbciMsgP4[4];    /* Reserved */
#else
    char       *dbciMsgP;       /* DR52059, Pointer to msg. area */
                                /*          provided by app.     */
#endif

/**********************************************************************/
/* Native 64 bit support for Cliv2                                    */
/**********************************************************************/
    /* DR55267: Native 64bit support         */
    /* DR58369: Native 64bit support on HPUX */
    /* DR58915: Native 64bit support for AIX */

#ifdef CLI_64BIT
    char      req_buf_len_4[8];             /* Reserved */
    char      resp_buf_len_4[8];            /* Reserved */
    char      curr_req_buf_len_4[8];        /* Reserved */
    char      cur_resp_buf_len_4[8];        /* Reserved */
    char      *logon_ptr;                   /* Level 1  */
    char      *run_ptr;                     /* Level 1  */
    char      req_len_4[8];                 /* Reserved */
    char      *req_ptr;                     /* Level 1  */
    char      fill_11[4];                   /* Reserved */
    char      fill_12[4];                   /* Reserved */
    char      *using_data_ptr;              /* Level 1  */
    char      fill_12a[8];                  /* DR89408 */ /* DR91963 */
    char      fet_max_data_len_4[8];        /* Reserved */
    char      fill_13[4];                   /* Reserved */
    char      fill_14[4];                   /* Reserved */
    char      *fet_data_ptr;                /* Level 1  */
    char      fet_ret_data_len_4[8];        /* Reserved */
    char      *inter_ptr;                   /* Level 1  */
    void      *extension_pointer;           /* Level 1  */
    char      *dbciMsgP;            /* DR52059, Pointer to Msg area    */
                                    /*          provided by app.       */
#else
    char      req_buf_len_8[8];     /* Reserved */
    char      resp_buf_len_8[8];    /* Reserved */
    char      curr_req_buf_len_8[8];/* Reserved */
    char      cur_resp_buf_len_8[8];/* Reserved */
    char      logon_ptr_8[8];       /* Reserved */
    char      run_ptr_8[8];         /* Reserved */
    char      req_len_8[8];         /* Reserved */
    char      req_ptr_8[8];         /* Reserved */
    char      fill_11[4];           /* Reserved */
    char      fill_12[4];           /* Reserved */
    char      using_data_ptr_8[8];  /* Reserved */
    char      fill_12a[8];          /* DR89408  */
    char      fet_max_data_len_8[8];/* Reserved */
    char      fill_13[4];           /* Reserved */
    char      fill_14[4];           /* Reserved */
    char      fet_data_ptr_8[8];    /* Reserved */
    char      fet_ret_data_len_8[8];/* Reserved */
    char      inter_ptr_8[8];       /* Reserved */
    char      extension_pointer_8[8];/* Reserved */
    char      dbciMsgP8[8];          /* Reserved */
#endif
    char      fill_15[4];           /* DR58353: added new filler       */
    char      fill_16;              /* DR58353: added new filler       */
    char      dbcoTSS1;             /* DR58353: Track overflow of Time1*/
    char      dbcoTSS2;             /* DR58353: Unsed in NW-CLI        */
    char      dbcoTSS3;             /* DR58353: Unsed in NW-CLI        */
    char      dbcoTSS4;             /* DR58353: Unsed in NW-CLI        */
    char      dbcoTSS5;             /* DR58353: Track overflow of Time5*/
    char      exempt_sess_from_DBCHWAT; /* DR94622: dual-component     */
    char      create_default_connection;/* DR101287: default connection*/
    UInt32    using_data_count;     /* DR68838: Data array count       */
#define dbriUDC using_data_count    /* DR68838: MF compatibility       */
    char      logmech_name[8];      /* DR68842: Security support    -> */
#ifndef CLI_64BIT
    char      logmech_data_ptr_reserved[4]; /* Reserved */
#endif
    char     *logmech_data_ptr;
    UInt32    logmech_data_len;
#define MAX_LOGMECH_DATA_LEN 65535  /* <- DR68842: Security support    */
/* DR89408 --> */
    char      mechdata_Unicode_set; /* DR90995: Unicode credentials    */
    char      dynamic_result_sets_allowed; /* DR102528, Dynamic Result */
    unsigned char SP_return_result; /* DR102528, SP return result      */
    char      send_delegated_credentials; /* DR101663                  */
#ifndef CLI_64BIT
    char     using_data_ptr_array_reserved[4];
#endif
    char    *using_data_ptr_array;
#ifndef CLI_64BIT
    char     using_data_len_array_reserved[4];
#endif
    int     *using_data_len_array;
    UInt16   max_decimal_returned;  /* DR98024, MaxDecimalPrecision    */
/* DR108909 --> */
    char     transformsOff;         /* DR123516, offset 586            */
    char     periodAsStruct;        /* DR123516, offset 587            */
    unsigned int workload_len;
#ifndef CLI_64BIT
    char     workload_ptr_reserved[4];
#endif
    char    *workload_ptr;          /* DR110710, use low-order byte    */
/* <-- DR108909 */
    short    logon_timeout;         /* DR118581 */
    UInt16   wait_time;             /* CLAC-32067, CLAC-33706 */
    char     fill_17[8];            /* DR122304, CLAC-32067 offset 604 to 611*/
    char     trustedRequest;        /* DR122304, offset 612            */
    char     columnInfo;            /* DR116354, offset 613            */
    char     utilityWorkload;       /* DR126915, offset 614            */
    char     multiStatementErrors;  /* DR121202, offset 615            */
    char     fill_18[4];            /* DR145358, offset 616 to 619     */
    char     automaticRedrive;      /* DR145358, offset 620            */
    char     extendedLoadUsage;     /* CLAC-29283, offset 621          */
    char     fill_19[14];           /* CLAC-29283, offset from 15 to 14*/
    char     tasmFastFailReq;       /* CLAC-30478, offset 636          */
    char     xmlResponseFormat;     /* CLAC-28875, offset 637          */
    char     arrayTransformsOff;    /* DR145224, offset 638            */
    char     fill_20[1];            /* DR145224, offset 639            */
                                    /* DR145224, offset from 19 to 17  */
                                    /* DR145358, change from 24 to 19  */
                                    /* DR121202, change from 25 to 24  */
                                    /* DR126915, change from 26 to 25  */
                                    /* DR116354, change from 40 to 26  */
                                    /* DR98024, change from 56 to 54   */
                                    /* DR108909, change from 54 to 40  */
                                    /* DR68838: Reserved               */
/* <-- DR89408 */


/* ===============>>> TO USE MEMBERS BELOW THIS LINE <<<==================== */
/*                                                                           */
/* Struct members below this point must be wrapped in the following if       */
/* condition statement before use:                                           */
/*                                                                           */
/*   if (DBCLEVEL >= DBCLEXP2)                                               */
/*   {                                                                       */
/*       ...                                                                 */
/*   }                                                                       */
/*                                                                           */
/* This will allow older apps built with a dbcarea size of 640 bytes or less */
/* to function with a new CLI.                                               */
/*                                                                           */
/* ========================================================================= */

#ifndef CLI_64BIT
    char     using_SLOB_data_struct_reserved[4];  /* Reserved          */
#endif
    struct SLOB_data_struct
            *using_SLOB_data_struct;/* CLAC-30554, use low-order byte  */
    UInt64   MaxSingleLOBBytes;     /* CLAC-31559, offset 648          */
    UInt64   MaxTotalLOBBytesPerRow;/* CLAC-31559, offset 656          */
    char     fill_21[16];           /* offset 664 Reserved for mainframe*/
#ifndef CLI_64BIT
    char     connect_ptr_reserved[4];   /* Reserved                    */
#endif
    char    *connect_ptr;           /* CLIWS-7635, offset 680 (0x02A8) */
    UInt32   connect_len;           /* CLIWS-7635, offset 688 (0x02B0) */
    char     sslmode;               /* CLIWS-7550, offset 692 (0x02B4) */
    char     o_sslmode;             /* CLIWS-7604, offset 693 (0x02B5) */
    UInt16   o_port;                /* CLIWS-9273, offset 694 (0x02B6) */

/* CLIWS-7581 --> */
#define CLI_SSLMODE_ALLOW           'A'
#define CLI_SSLMODE_DISABLE         'D'
#define CLI_SSLMODE_PREFER          'P'
#define CLI_SSLMODE_REQUIRE         'R'
#define CLI_SSLMODE_VERIFY_CA       'C'
#define CLI_SSLMODE_VERIFY_FULL     'F'
/* <-- CLIWS-7581 */

    char     fill_22[326];          /* (reserved)  offset 696 (0x02B8) */
    char     largeRow;              /* CLAC-29115, offset 1022(0x03FE) */
    char     fill_23[1];            /* CLAC-29115, offset 1023(0x03FF) */
} DBCAREA;                          /* H0_07 */
/* When you add a new field, BE SURE TO UPDATE THE SIZE OF 'fill_22' properly */

/***************************************************************/
/*  END of MAIN DBCAREA  structure                             */
/***************************************************************/

#ifdef IBMASM_NAMES                /* Begin of H0_07 merge     */
/***************************************************************/
/*  Name definitions to match DBCAREA macro                    */
/***************************************************************/
#pragma options copts(dollars)
#define DBCAID eyecatcher
#define DBCSIZE total_len
#define DBCAFUNC func
#define DBCICONN i_sess_id
#define DBCICCMD 0xFFFFFFFF
#define DBCIREQN i_req_id
#define DBCIRBRL req_buf_len
#define DBCIFBRL resp_buf_len
#define DBCISMAX max_num_sess
#define DBCIURQN token
#define         DBF$CON  DBFCON
#define         DBF$DSC  DBFDSC
#define         DBF$RSUP DBFRSUP
#define         DBF$IRQ  DBFIRQ
#define         DBF$FET  DBFFET
#define         DBF$REW  DBFREW
#define         DBF$ABT  DBFABT
#define         DBF$ERQ  DBFERQ
#define         DBF$PRG  DBFPRG
#define         DBF$IWPF DBFIWPF
#define         DBF$CMD  DBFCMD
#define         DBF$CRQ  DBFCRQ
#define         DBF$MAX  DBF$CRQ
#define DBCRCODE return_cd
#define DBCOCONN o_sess_id
#define DBCOREQN o_req_id
#define DBCOSID  o_dbc_path
#define DBCOSIT  DBCOSID
#define DBCOSISN o_dbc_sess_id
#define DBCOSILH o_host_id
#define DBCOFLGS sess_status
#define DBCOFLG1 DBCOFLGS
#define DBCOFLG2 fill_2
#define DBF1$POL 0x80
#define DBF1$TRX 0x40
#define DBF1$CLR 0x20
#define DBF1$ID  0x210
#define DBCOARQN tdp_req_no
#define DBCORBLA cur_req_buf_len
#define DBCOFBLA cur_resp_buf_len
#define DBCNITDP i_dbcpath
#define DBCNISSN i_dbcpath
#define DBCNIVMN i_dbcpath
#define DBCNILSP logon_ptr
#define DBCNILSL logon_len
#define DBCNIRSP run_ptr
#define DBCNIRSL run_len
#define DBRIRQP req_ptr
#define DBRIRQL req_len
#define DBRIUDP using_data_ptr
#define DBRIUDPL using_data_len
#define DBROREQC open_reqs
#define DBFIFDL fet_max_data_len
#define DBFXFDP fet_data_ptr
#define DBFOFDL fet_ret_data_len
#define DBFOPFLV fet_parcel_flavor
#define DBFOEFLG fet_error_ind
#define DBCTSTMP hsisvc_time
#define DBCTIME1 tdpwait_time
#define DBCTIME2 tdpdbo_time
#define DBCTIME3 tdpdbi_time
#define DBCTIME4 tdpxmm_time
#define DBCTIME5 srbsched_time
#define DBCCSNP inter_ptr
#define DBCCSCP inter_ptr
#define DBCAXP extension_pointer
#define DBOSETO change_opts
#define DBOSE$Y 'Y'
#define DBOSE$N 'N'
#define DBORMOD resp_mode
#define DBORM$R 'R'
#define DBORM$F 'F'
#define DBORM$I 'I'
#define DBOIDTA use_presence_bits
#define DBOID$Y 'Y'
#define DBOID$N 'N'
#define DBOKRSP keep_resp
#define DBOKR$Y 'Y'
#define DBOKR$N 'N'
#define DBOKR$P 'P'					/* DR60695 */
#define DBOCRSW wait_across_crash
#define DBOCW$Y 'Y'
#define DBOCW$N 'N'
#define DBOCRTL tell_about_crash
#define DBOCL$Y 'Y'
#define DBOCL$N 'N'
#define DBORSV2 connect_wait         /* for W/S only */
#define DBOFLOC loc_mode
#define DBOFLO$Y 'Y'
#define DBOFLO$N 'N'
#define DBORVAR var_len_req
#define DBORVA$Y 'Y'
#define DBORVA$N 'N'
#define DBOFVAR var_len_fetch
#define DBOFVA$Y 'Y'
#define DBOFVA$N 'N'
#define DBOFSVB save_resp_buf
#define DBOFSB$Y 'Y'
#define DBOFSB$N 'N'
#define DBO2FBU two_resp_bufs
#define DBO2FB$Y 'Y'
#define DBO2FB$N 'N'
#define DBORTS  ret_time
#define DBORT$Y  'Y'
#define DBORT$N  'N'
#define DBOBTPM parcel_mode
#define DBOBT$Y  'Y'
#define DBOBT$N  'N'
#define DBOBSYW  wait_for_resp
#define DBOBS$Y  'Y'
#define DBOBS$N  'N'
#define DBOFUNT req_proc_opt
#define DBOFU$E  'E'
#define DBOFU$P  'P'
#define DBOFU$S  'S'
#define DBORSV1  msg_security
#define DBOSCS charset_type
#define DBOSC$C 'C'
#define DBOSC$N 'N'
#define DBOCTYPE connect_type
#define DBOCT$R 'R'
#define DBOCT$C 'C'
#define DBOQMOD request_mode
#define DBOQM$P 'P'
#define DBOQM$B 'B'
#define two_phase_commit sess_2pc_mode
#define DBO2PC sess_2pc_mode
#define DBO2P$Y 'Y'
#define DBO2P$N 'N'
#define DBRIPF init_with_2pc
#define DBRIP$N 'N'
#define DBRIP$V 'V'
#define DBRIP$T 'T'
                                                    /* H3_02 - begin NDM */
#define DBCNITSM tx_semantics
#define DBCNITD 'D'
#define DBCNITT 'T'
#define DBCNITA 'A'
#define DBCNILCS lang_conformance
#define DBCNILN 'N'
#define DBCNILF2 '2'
#define DBCNILF3 '3'
                                                    /* H3_02 - end NDM   */
#define DBCMSGA fill_8
#define DBCMSGL msg_len
#define DBCMSG msg_text
#define DBCMSGRD route_desc_codes

#define DBRIROB return_object
#define DBRIROBD 'D'
#define DBRIROBT 'T'
#define DBRIROBS 'S'

#define DBNICNT continuation_code
#define DBNICNTF 'F'
#define DBNICNTI 'I'
#define DBNICNTL 'L'
#define DBNICNTC 'C'

#endif

/* DR102528 --> */
#define DBC_RetnRsltRqstr        1    /* -Requester           */
#define DBC_RetnRsltAppl         2    /* -Application         */
#define DBC_RetnRsltCaller       3    /* -Caller              */
#define DBC_RetnRsltRqstrAppl    4    /* -Requester & Appl    */
#define DBC_RetnRsltRqstrCaller  5    /* -Requester & Caller  */
/* <-- DR102528 */

/*                                  DR 52050-> */
#ifndef cli_session_defined
#define cli_session_defined
typedef struct cli_session {
    Int32  session;
    Int32  reserved1;
    Int32  reserved2;
} SESSIONS, *SESSIONS_PTR;
#endif

#define CLI_WIN_TIMEOUT  1  /* Timeout event */
struct cli_win_event {
    Int32 kind;  /* event type */
    void *data;  /* pointer to user supplied data */
};

#define INSERT_EVENT 1
#define DELETE_EVENT 2
/*                                  ->DR 52050 */

#ifndef NOPROT /* DR21506 */

#if (defined(__MVS__) && !defined(__cplusplus)) /* DR87586 */
#ifndef CLI_MF_GTW  /* CLIWS-7751 */
#pragma linkage(DBCHATTN,OS)
#pragma linkage(DBCHBRK,OS)
#pragma linkage(DBCHCL,OS)
#pragma linkage(DBCHCLN,OS)
#pragma linkage(DBCHERR,OS)
#pragma linkage(DBCHFER,OS)
#pragma linkage(DBCHFERL,OS)
#pragma linkage(DBCHINI,OS)
#pragma linkage(DBCHPEC,OS)
#pragma linkage(DBCHREL,OS)
#pragma linkage(DBCHSAD,OS)
#pragma linkage(DBCHUEC,OS)
#pragma linkage(DBCHUE,OS)
#pragma linkage(DBCHWAT,OS)
#pragma linkage(DBCHWL,OS)
#endif  /* CLI_MF_GTW */ /* CLIWS-7751 */
#endif

#ifndef EXPENTRY
#ifdef WIN32
#define EXPENTRY __stdcall /* DR68139, DR68140, DR86451 */
#else  /* WIN32 */
#define EXPENTRY
#endif /* WIN32 */
#endif /* ifndef EXPENTRY */

/* DR85431 --> */
#ifdef __cplusplus
#if defined(__MVS__) && !defined(CLI_MF_GTW)
extern "OS" {
#else
extern "C" {
#endif
#endif
Int32  EXPENTRY DBCHATTN(Int32 *);
Int32  EXPENTRY DBCHBRK (void);
Int32  EXPENTRY DBCHCL  (Int32 *, char *, struct DBCAREA *);
Int32  EXPENTRY DBCHCLN (Int32 *, char *);
Int32  EXPENTRY DBCHERO (Int32 *, char *, struct DBCAREA *);
Int32  EXPENTRY DBCHERR (Int32, struct DBCAREA *, errpt_t);
Int32  EXPENTRY DBCHFER (Int32, register errpt_t, char *);
Int32  EXPENTRY DBCHFERL(Int32, register errpt_t, char *, UInt16, UInt16*, Int16*);
Int32  EXPENTRY DBCHINI (Int32 *, char *, struct DBCAREA *);
Int32  EXPENTRY DBCHPEC (Int32 *, Int32 *);
Int32  EXPENTRY DBCHREL (Int32 *, char *, Int32, char *, char *);
void   EXPENTRY DBCHSAD (Int32 *, void **, void *);
Int32  EXPENTRY DBCHUEC (Int32 *, char *, Int32 *); /* <-DR59337 */
Int32  EXPENTRY DBCHUE  (Int32 *, char **, struct cli_win_event *, Int32);
Int32  EXPENTRY DBCHWAT (Int32 *, char *, Int32 *, Int32 *);
Int32  EXPENTRY DBCHWL  (Int32 *, char **, SESSIONS_PTR, Int32 *, Int32 *);

char * EXPENTRY OsGetCOPCLIVersion(void);

#ifdef __cplusplus
}
#endif

/* CLIWS-7751 --> */
#ifdef CLI_MF_GTW
#pragma export(DBCHATTN)
#pragma export(DBCHBRK)
#pragma export(DBCHCL)
#pragma export(DBCHCLN)
#pragma export(DBCHERO)
#pragma export(DBCHERR)
#pragma export(DBCHFER)
#pragma export(DBCHFERL)
#pragma export(DBCHINI)
#pragma export(DBCHPEC)
#pragma export(DBCHREL)
#pragma export(DBCHSAD)
#pragma export(DBCHUEC)
#pragma export(DBCHUE)
#pragma export(DBCHWAT)
#pragma export(DBCHWL)
#pragma export(OsGetCOPCLIVersion)
#endif  /* CLI_MF_GTW */
/* <-- CLIWS-7751 */

#endif  /* NOPROT */

/***************************************************************/
/*                       END of DBCAREA.H                      */
/***************************************************************/
#endif /* DBCAREA_H */
