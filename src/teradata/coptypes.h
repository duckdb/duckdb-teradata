#ifndef COPTYPES_H
#define COPTYPES_H
#define COPTYPES_H_REV "20.00.00.00"
/*******************************************************************************
 *
 *
 *  WARNING: Because coptypes.h is shared and is used as a header
 *           file for mainframe SAS/C compiles under VM/CMS, the logical
 *           record length (i.e., line length) must not exceed 80
 *           characters.
 *-----------------------------------------------------------------------------*
 *                   <-- ( 8 0    c h a r a c t e r s) -->
 *******************************************************************************
 *  TITLE:       COPTYPES.H .. Major type definitions for "C"        20.00.00.00
 *
 *  Purpose      To contain the major type definitions for
 *               the workstation code written in "C".
 *
 *  Copyright 1983-2023 by Teradata Corporation.  All rights reserved.
 *
 *  Description  This include file contains the major type
 *               definitions for the workstation version of
 *               BTEQ and CLIV2.
 *
 *  History
 *    D.1   86Jul28  KKL  DCR3074  Coded
 *    D.2   87Jan30  DRP  DR8682   add link down state MTDP
 *    F.3   87Feb26  KKL  DR9379   Added SysMaxSess
 *    F.4   87Apr29  DAA  DR9321   fix SysMaxRequestSize
 *    F.5   87Oct28  SCT  DR 10471 Narrowed to <=65 columns
 *    F.6   87Nov17  DAA  DR 10471 fix it
 *    F.7   88Jan28  WHY  DR 11926 (MR# fe87-18102a0)
 *    G0_01 88Sep23  MBH  DR 14058 Shipped as CAPH file
 *    G0_02 89Jun05  KKL  DR 16460 Added OS dependent stuff
 *    G0_03 89Jun05  KKL  DR 16460 Re-ship DR16460
 *    G0_04 89Jun07  KKL  DR 16494 redefine errno by !using
 *                                 stdlib.h
 *    G0_05 90Feb05  KXO  DR 18467 Add constant for DirectReq
 *    G0_06 90May07  KBC  DR 19675 Use proper errono definition
 *                                 for Microsoft C5.1 or later
 *    G0_07 90Sep14  KXO           Revised for UNIX TOOL KIT
 *    G0_08 90Sep25  KBC  DR 19675 Fix to still compile under C5.0
 *    G0_09 90Nov13  KBC  DR 20832 Remove errno definition for PC.
 *                                 PC programs must include stdlib.h
 *                                 to get proper definition of errno,
 *                                 since it is compiler level
 *                                 dependent.
 *    G0_10 91Jan03  JML           Fix for syntax error induced
 *                                 during code beautification (G0_07).
 *    G0_11 91Jan05  DJL  DCR 5366 Tandem interface.
 *    G0_12 91Feb15  GLL  DCR5689  Add definition for FILEPTR
 *    G0_13 92Jan13  BTA  DCR5957  Support Restructured Mosidos
 *    H0_00 93May20  SNT  DCR6711  Created for Release H.0
 *    H0_01 93Aug17  Bg1  DR28043  Convert the unsigned char type to char.
 *    HN_00 94Sep12  JAE  DCR6848  Added NDM support.
 *    HN_01 94Nov07  SNT  DCR6848  Removed NDM compile flag.
 *    HN_02 95FEB09  ALB  DR32294 Merged 5.0 and AFCAC code.
 *    HN_03 95Mar13  BG1  DR32448  Defined BOOL_CLI, character type
 *                                 boolean for Cli only.
 *    HN_04 95Mar30  ALB  DR28043  Changed Boolean typedebf back to unsigned
 *                                 char, added typedef for XWINDOWS support.
 *    HN_05 95Mar30  JAE  DR32448  Corrected fix from BG1 (added #ifndef)
 *    H3_00 95Sep13  TH4  DR34047  Added the #ifndef XXXX #define XXXX.
 *    H3_01 96Mar08  SYY        DR35631  Merge WinCLI 3.0 changes into main
 *    04.00.00.01 TH4 96Jul12   DR36785 Version number change.
 *    04.04.00.00 MDV 98Jul07   DR43005 Changed the value of SysMaxParcelSize
 *                                      to accommodate the 64K row size.
 *    04.05.00.00 CSG 2000Apr10 DR50943 Added STRING_CLI for compiling with
 *                                      C++ compilers.
 *    04.06.00.00 ASG 2001Jan17 DR52052 Support SSO
 *    04.06.01.00 CSG 2001May25 DR55267 Native 64bit support
 *    04.06.02.00 CSG 2002Mar27 DR58353 Sub-Second support
 *    04.07.00.00 CSG 2002May08 DR58369 RFC: Port 64 bit CLIv2 to HPUX.
 *    04.07.00.01 CSG 2002May18 DR58915 RFC: Port 64 bit CLIv2 to AIX.
 *    04.07.00.02 CSG 2002Jun04 DR59337 Port CLI to 64 Bit Windows XP.
 *    04.07.00.03 CSG 2002Jun04 DR57320 Support Increased response size.
 *    04.07.00.04 CSG 2002Jun17 DR61637 Support enlarged parcel usage.
 *    04.07.00.05 CSG 2002Aug27 DR60830 Port CLI to 64bit linux
 *    04.07.01.00 ASG 2003Feb10 DR66590 Add 64-bit integer data type.
 *    04.07.01.01 mg180007 2003Apr24 DR68084 renamed TimeStamp to time_stamp
 *    04.08.00.00 mg180007 2003Jul22 DR68511 clean up defines, prototypes
 *    04.08.00.01 ASG      2004Feb23 DR85431 universalize headers
 *    04.08.00.02 ASG      2004Jun22 DR87725 avoid name collision with BTEQ
 *    04.08.01.00 mg180007 2005Apr21 DR93472 added header revision info
 *    04.08.01.01 bh185000 2005Jun10 DR96357 added ifndef to resolve the issue
 *    04.08.02.00 ASG      2006Feb14 DR102061 reconfig for 80-bye lines
 *    04.08.02.01 ASG      2006Feb16 DR67535 accommodate TPT data types
 *    04.09.00.00 ASG      2006Sep06 DR103365 handle exapanded hash buckets
 *    04.09.00.01 bh185000 2006Oct03 DR105994 sepetate errpt_t and error_t
 *    12.00.00.02 fs185009 2006Nov29 DR108852 version change to 12.0.x
 *    13.00.00.00 mg180007 2008Jan09 DR117140 copyright update
 *    13.10.00.00 kl185018 2009Jun23 DR133468 TTU13.1 Version Renumbering
 *                                            to TTU13.10
 *    14.10.00.00 mg180007 2012Feb03 CLAC-663 added cli_buffer_desc
 *    14.10.00.01 mg180007 2012Apr16 CLAC-663 fixed cli_buffer_desc
 *    14.10.00.02 mg180007 2012Apr28 CLAC-29437 use newer HPUX-IA toolset
 *    15.10.00.00 mg180007 2014Apr28 CLAC-32368 version and copyright update
 *    17.00.00.00 hs186016 2018Nov13 CLIWS-7163 version and copyright update
 *    17.10.00.00 hs186016 2020Jul27 CLIWS-7606 version change to 17.10
 *    17.10.00.01 hs186016 2021Jun03 CLIWS-7751 Support mainframe on z/OS
 *    17.20.00.00 hs186016 2022Jan01 CLIWS-8198 version and copyright update
 *    20.00.00.00 hs186016 2023May11 CLIWS-8696 version and copyright update
 ******************************************************************************/

#ifndef TPT_BUILD                       /* DR67535 */
/* DR68511 */
#ifndef BOOLT_CLI                       /* HN_05, added #ifndef  */
typedef char              bool_t;       /* boolean data types    */
typedef bool_t            flag_t;       /* DR32448 HN_03 bg1     */
#endif
#endif                                  /* DR67535 */

/* DR55267, 58369, 58915: defined generalized macro for 64 bit platforms */
/* DR59337: added implicit macro _WIN64 */
/* DR60830: Added implicit macro __ia64__ for 64bit linux on IA64 */
/* CLAC-29437: added _ILP32 for hpux-ia64.32 */
#if (defined(_LP64) || defined(__LP64__) || defined(__64BIT__) || \
    defined(_WIN64) || defined (__ia64__)) && !defined(_ILP32)
#define CLI_64BIT
#endif
#ifndef TPT_BUILD       /* DR67535 */
/*================*/
/*  General       */
/*================*/

/*
** Single Byte Data Types
*/
typedef unsigned char      Byte;         /* A 1 byte Unsigned Int     */

/*
** Two Byte Data Types
*/
typedef short              Int16;        /* A 2 byte integer          */
typedef short *            Int16_Ptr;    /* A Pointer to a 2 byte int */
typedef unsigned short     Word;         /* A 2 byte Unsigned Int     */
typedef unsigned short     UInt16;       /* A unsigned 2 byte integer */
typedef unsigned short *   UInt16_Ptr;   /* ptr unsgn 2 byte integer  */

/*
** Four Byte Data Types
*/
/* DR55267:  Int32 and UInt32 defined as int and unsigned int */
typedef int                Int32;        /* A 4 byte integer            */
typedef int *              Int32_Ptr;    /* A ptr to a 4 byte int       */
typedef unsigned int       UInt32;       /* A 4 byte unsigned integer   */
typedef unsigned int *     UInt32_Ptr;   /* A ptr to a 4 byte unsgn int */
typedef unsigned int       Double_Word;  /* A 4 byte Unsigned Int       */

/*
** Misc. Data Types
*/
typedef unsigned           protag_t;
typedef char *             PtrType;
/* DR87725 --> */
#if defined(__MVS__) && !defined(CLI_MF_GTW) /* CLIWS-7751 */
typedef void *             storid_t;
#else
typedef char *             storid_t;
#endif
/* <-- DR87725 */

/* DR 66590 --> */
typedef long long          Int64;
typedef unsigned long long UInt64;
/* <-- DR 66590 */

#ifndef STRING_CLI                     /* DR50943, For C++ compilers */
   typedef char *          string;     /* char array pointer         */
#endif

typedef char *             string_t;   /* zeroterminated string */
typedef unsigned           systag_t;
typedef Int32              time_stamp; /* The Time Stamp type *//* DR68084 */
               /* ->  DR58353  */
#endif         /* DR67535 */
typedef struct save_time
{
    Int32 Days;
    Int32 Hours;
    Int32 Minutes;
} SAVETIME;

/* Utilities can use this structure to fetch  min and max    */
/* timestamp precision suppoted  on a platform using DBCHQE  */
typedef struct
{
    Int16 minimum;
    Int16 maximum;
} TSP_QUERY;
/* DR 58353   <- */

typedef struct cli_buffer_desc_struct { /* CLAC-663 -> */
#ifdef CLI_64BIT
    UInt64      length;
#else
    UInt32      length;
#endif
    void       *value;
} cli_buffer_desc, *cli_buffer_t;

#define CLI_C_EMPTY_BUFFER {0, NULL}    /* <- CLAC-663 */

/*================*/
/*  Error type    */
/*================*/

/* DR55267: type changed to UInt32 */
typedef  struct  err_code
{
    UInt32      e_class;
    UInt32      e_reason;
    UInt32      e_syst;
} *errpt_t
#ifndef NO_CLIV2_ERROR_T              /* DR96357, DR105994 */
, error_t;
#else
;
#endif                                /* DR96357, DR105994 */

/***************************************************************/
/*            Some CONSTANTS needed by BTEQ and CLI.           */
/***************************************************************/

#define  Ok                  0
#define  NilPtr              0
#define  TRUE                1
#define  FALSE               0

#define  DATAHERE            1
#define  NODATA              0

#define  IDLE                0
#define  ACTIVE              1

#define  OWNMOSI             1
#define  OWNMTDP             2
#define  OWNCLI              3
#define  SCS_VC_DOWN         2
#define  VERSION             2

#define  SysMaxDelayTime     50    /* max time to wait before  */
                                   /* polling MTDP (in 10ths of*/
                                   /* a second). On PC, this   */
                                   /* should be a value < 10   */
#define  SysMaxErrMsg        256
#define  SysMaxWarnMsg       128   /* DR85431 */
/* DR85431 --> */
#if defined(__MVS__) && !defined(CLI_MF_GTW)     /* CLIWS-7751 */
#define  SysMaxName          32
#define  SysMaxParcelSize    64260               /* DR44785, 39481 */
#else
#define  SysMaxName          30
#define  SysMaxParcelSize    65473 /* DR43005 */
#endif
/* <-- DR85431 */
#define  SysMaxRequestSize   8192

/*========================================*/
/* FOR APH and Extended Resposnse support */
/*========================================*/
#define  SysExtMaxParcelSize 1048514  /* DR57320 */
#define  SysAltMaxRequestSize 1048500 /* DR61637 */

#ifdef WIN32
#define  SysMaxSess          200  /* Max session No supported */
#else
#define  SysMaxSess          64   /* Max session No supported */
#endif
#define  SysMaxAnsiMsg       128   /* Max length of FIPS Flagger msg   */
#define  ParMaxFIPSFlags     10    /* Max FIPS flagged items per query */

/*=============================================================*/
/* DR18467 FEB90 KXO, Direct Req.                              */
/* following values should be same as in DBCTYPES.H of HUT(ARC)*/
/*=============================================================*/

#define  MSG_HUT_CLASS       25    /* SysMsgHUTClass   of HUT  */
#define  MSG_NO_ADDR          0    /* MsgNoAddr        of HUT  */
#define  MSG_TRANSIENT_ADDR   1    /* MsgTransientAddr of HUT  */
#define  MSG_PROCBOX_ADDR     2    /* MsgProcBoxAddr   of HUT  */
#define  MSG_HASHBOX_ADDR     3    /* MsgHashBoxAddr   of HUT  */
#define  MSG_GROUPBOX_ADDR    4    /* MsgGroupBoxAddr  of HUT  */
#define  MSG_EXPHASHBOX_ADDR 12    /* DR103365 */
#define  MAILBOX_SIZE         6

/*=============================================================*/
/*  END of DR18467 FEB90 KXO, Direct Req.                      */
/*=============================================================*/


/***************************************************************/
/* The followings are considered platform/protocol dependent.  */
/* Until we figure out where/how to group these into a more    */
/* portable fashion, here will be their temp home.             */
/***************************************************************/
/* Suggestions:                                                */
/*   1. bool_t should be used instead of 'Boolean' which may   */
/*      has a different definition from different compiler.    */
/*   2. Applications should have "errno" declared in coptypes.h*/
/*      due to different "errno" declaration from Microsoft    */
/*      Compiler.                                              */
/*                                                             */
/***************************************************************/

/* DR68511 --> */
#ifndef TPT_BUILD                  /* DR67535 */
#ifndef XWINDOWS                   /*  ALB - HN-04  DR 28043     */
typedef unsigned char Boolean;     /*  Boolean data types        */
#endif
#endif                             /* DR67535 */
/* <-- DR68511 */


/* CLIWS-7751 --> */
#if defined (CLI_MF_GTW)
    #define CLI_INLINE
#else
/* CLAC-32983 --> */
    #if defined(WIN32) || defined(AIX)
        #define CLI_INLINE __inline
    #else
        #define CLI_INLINE inline
    #endif
/* <-- CLAC-32983 */
#endif  /* CLI_MF_GTW */
/* <-- CLIWS-7751 */

/***************************************************************/
/*                       End of COPTYPES.H                     */
/***************************************************************/
#endif  /* COPTYPES_H */
