#ifndef PARCEL_H
#define PARCEL_H
#define PARCEL_H_REV "20.00.00.02"
/*******************************************************************************
 *  TITLE:       PARCEL.H .. Parcel type definitions for "C"         20.00.00.02
 *
 *  Purpose      To contain the parcel type information needed
 *               for MTDP, CLIv2, and application programs.
 *
 *  Copyright 1983-2023 Teradata. All rights reserved.
 *  TERADATA CONFIDENTIAL AND TRADE SECRET.
 *
 *  Description  This include file contains the parcel type
 *               definitions for the workstation version of
 *               BTEQ and CLIv1.
 *
 *  History
 *   D.1   86Jul28  DRP           Coded
 *   D.2   87Feb24  KKL  DR 9381  Corrected some comments
 *   D.3   87Mar17  KKL  DR 9381  changed PclLength to Length
 *                                or PLength
 *   F.1   87Jul22  DAA  DCR3780  added Options Parcel
 *   F.2   87Aug11  DAA  DCR3780  updated AssignRsp for Ver.
 *   F.3   87Jan08  DRP           Release F
 *   F.4   86Jan16  DRP  DR 8832  Fixed Success parcel &
 *                                PCLLENGTHs
 *   F.5   87Mar17  DRP  DR 9341  changed for release F Ver.
 *   F.6   87Apr29  DRP  DR 9923  fixed success parcel
 *   F.7   87Nov03  DAA  DR 11006 added HostID to AssignRsp
 *   F.8   87Nov15  SCT  DR 10471 Narrowed to <=65 characters
 *   F.9   87Nov15  DAA  DR 11006 add semicolon on
 *   F.10  87Nov17  SCT  DR 10471 Fixed my case errors
 *   F.11  87Nov17  SCT  DR 10471 Fixed my case
 *   F.12  88Jan10  DRP  DR 11680 Added  CliPrepInfoType and
 *                                CliPrepColInfoType.
 *   F.13  88Jan28  WHY  DR 11926 (MR# fe87-18102a0)
 *   F.14  88May12  YDS  DCR 4505 Add new SQLstmttypes.
 *   F.15  88Jul13  WBJ  DR 10533 Add PclNOFLAVOR
 *   G0_00 88Jul25  SN            Created for Release G.0
 *   G0_01 88Oct06  WBJ  DCR 4029 & other G.0 mods
 *   G0_02 88Oct17  WBJ  DCR 4029 configresp format refined
 *   G0_03 88Nov03  WBJ  DCR 4029 fix AMPArray def for  3b2
 *   G0_04 89Jun19  KKE  DCR 5094 Define set session
 *                                collation Pcl type.
 *   G0_05 89Dec22  SNT  DR 17876 Define Rel. Mload Pcltype
 *   G0_06 90Sep14  KXO           Revised for UNIXTOOL KIT
 *   G1_01 91Jan29  BUGS DCR4350  CBTEQ port.
 *   G1_02 91Jan29  BUGS DCR4350  Port to VM (GLL's 7/1/91 changes).
 *   G1_03 91Feb20  BUGS DR21617  Compile error (CR before #ifdef ATT)
 *   H0_00 93May20  SNT  DCR6711  Created for Release H.0
 *   H0_01 94Apr04  JAE  DCR6712  Merged 2PC code for Genesis II.
 *   HN_00 94Aug05  JAE  DCR6848  New Parcels for NDM Contract
 *   HN_01 94Nov30  JAE  DCR6848  Changed parcel constant names to
 *                                UPPER CASE for consistency.
 *                                This is also done for the 2PC constants
 *                                as well as the NDM constants.
 *   HN_02       ALB      95Feb09   DR32294 Merged 5.0 and AFCAC code.
 *   H3_00       TH4      95Sep13   DR34047 Added the #ifndef XXXX #define XXXX.
 *   04.00.00.01 TH4      96Jul12   DR36785 Version number change.
 *   04.02.00.00 TH4      97Sep10   DR40417 ANSI Date/Time Support.
 *   04.02.00.01 JTH      97Dec16   DR41042 Max request Length
 *   04.02.00.02 JTH      98jAN06   DR41136 fix regression caused by dr41042
 *   04.02.00.03 MDV      98Mar24   DR41685 Support database triggers.
 *   04.04.00.00 TH4      98Jun22   DR42286 Change the Int32 to UInt32 for AC.
 *   04.05.00.00 CSG      2000Mar03 DR48735 TDSP Support
 *   04.05.00.01 CSG      2000Mar22 DR50139 TDSP Report Error 374 (NOTDSP)
 *   04.06.00.00 ASG      2001Jan17 DR52052 Support SSO
 *   04.06.00.01 ASG      2001Feb26 DR54273 Fix re-logon problem
 *   04.06.01.00 ASG      2001Aug08 DR56467 #define non-standard parcel names
 *   04.06.02.00 CSG      2002Mar28 DR60508 Add PerformUpdate feature support
 *   04.07.00.00 CSG      2002Jun04 DR57320 Support Increased maximum
 *                                  response size
 *   04.07.00.01 CSG      2002Jun14 DR61637 Support enlarged parcel usage.
 *   04.07.00.02 ASG      2002Jun03 DR57583 LOB support
 *   04.07.00.03 CSG      2002Aug28 DR62240 New activity counts in DR59863.
 *   04.07.00.04 CSG      2002Sep16 DR59000 Port WinCli to XP-64 bit.
 *   04.07.01.00 ASG      2003Feb10 DR66590 Add PrepInfoX parcel.
 *   04.07.01.01 mg180007 2003Mar12 DR67744 Remove HPUX compilation warnings.
 *   04.07.01.02 ASG/DHV  2002Jul28 DR52869 Add GSS support
 *                                  mg180007 2003Mar27 merged changes by
 *                                  ASG/DHV into 4.7.x
 *   04.07.01.03 ASG      2003Mar31 DR60695 Cursor positioning.
 *   04.07.01.04 ASG      2003Apr04 DR68008 Add pad byte to ElicitFile.
 *   04.07.01.05 CSG      2003Apr04 DR67892 Add UDF stmt types.
 *   04.07.01.06 CSG      2003Apr16 DR68080 Resolve c++ compile errors
 *   04.07.01.07 ASG      2003Apr23 DR68105 Fix struct FieldType for DataInfoX.
 *   04.07.01.08 ASG      2003May13 DR68259 Fix alignment for SPARC, HPUX, AIX
 *   04.07.01.09 CSG      2003Jun24 DR68615 ppruntim compile error on
 *                                          Solaris Sparc/Define parcel.h
 *                                          own typedefs to resolve HPUX
 *                                          packing problem
 *   04.08.00.00 mg180007 2003Jul22 DR68511 clean up defines, prototypes
 *                                  DR68139 self-sufficient headers
 *   04.08.00.01 ASG      2003Dec30 DR68835 APH / 1 MB Responses
 *   04.08.00.02 CSG      2003Sep29 DR69228 Add support for character
 *                                          object state
 *   04.08.00.03 ASG      2004Feb10 DR58352 ESS
 *   04.08.00.04 CSG      2004Jan27 DR84603 Added pack(1) for Win-32
 *   04.08.00.05 ASG      2004Mar03 DR85431 universalize headers
 *   04.08.00.06 CSG      2004Mar16 DR52058 UTF16 support
 *   04.08.00.07 ASG      2004Apr12 DR86770 fix ErrorInfo parcel flavor
 *   04.08.00.08 mg180007 2004May14 DR68842 TTU 8.0 Security support
 *   04.08.00.09 ASG      2004Jul29 DR88784 Accommodate MF BTEQ
 *   04.08.00.10 ASG      2004Sep01 DR89769 Conform to DR 83892
 *   04.08.00.11 CSG      2005Jun24 DR94488 Port to HP-Itanium
 *   04.08.01.00 ASG      2005Mar10 DR92423 Add tx-isolation stmt type
 *   04.08.01.01 mg180007 2005Mar30 DR91422 HP_ALIGN pragma issue with aCC
 *   04.08.01.02 mg180007 2005Apr21 DR93472 added header revision info
 *   04.08.01.03 mg180007 2005Apr28 DR94976 Gateway UTF credentials support
 *   04.08.01.04 mg180007 2005Aug29 DR96966 Add PCLCLICONFIGTDGSS
 *   04.08.02.00 bh185000 2005Dec15 DR98024 Support for MaxDecimalPrecision
 *                                  DR98025 Support for AGKR_option
 *                                  DR98026 Support for StatementInfo
 *   04.08.02.01 bh185000 2006Jan12 DR101116 Data length need to be Uint16
 *   04.08.02.02 ASG      2006Feb14 DR102061 reconfig for 80-byte lines
 *   04.08.02.03 bh185000 2006Mar08 DR98026 Add StatementInfo defines
 *   04.08.02.04 ASG      2006Apr25 DR98026 Add StatementInfo MetaData detail
 *   04.09.00.00 bh185000 2006May31 DR102528 Add Dynamic Result Sets
 *   04.09.00.01 bh185000 2006Jun26 DR103360 Add Query Banding activity type
 *   04.09.00.02 bh185000 2006Jun26 DR105446 Complete activity type 155 to 172
 *   04.09.00.03 bh185000 2006Aug15 DR106619 Additional define for StatementInfo
 *   04.09.00.04 mg180007 2006Aug17 DR101074 add client version to cfg request
 *   04.09.00.05 mg180007 2006Jul11 DR101287 Add pclusernameresp_t
 *   12.00.00.06 fs185009 2006Nov29 DR108852 version change to 12.0.x
 *   12.00.00.07 ASG      2006Dec04 DR107934 Fix _O_BYTEALIGN check for MF SAS/C
 *   12.00.00.08 mg180007 2006Dec12 DR109200 DynamicResultSets positioning
 *   12.00.00.09 bh185000 2007Feb21 DR111230 Add SP Result Set activity code
 *   12.00.00.10 mg180007 2007Feb22 DR101074 fixed client version parcels
 *   13.00.00.00 bh185000 2007Sep26 DR113468 Support session node id
 *   13.00.00.01 bh185000 2007Nov12 DR115324 Support LOB Loader Phase 2
 *   13.00.00.02 bh185000 2007Nov13 DR115327 Support DDL Replication
 *   13.00.00.03 bh185000 2007Nov13 DR115330 Support User Defined SQL Operators
 *   13.00.00.04 mg180007 2008Jan09 DR117140 copyright update
 *   13.00.00.05 bh185000 2008Jan21 DR119336 Completed list of activity types
 *   13.01.00.00 bh185000 2008Nov17 DR116354 Support Extended Object Name Parcel
 *   13.01.00.01 bh185000 2008Dec03 DR121443 Support Mandatory Access Control
 *   13.01.00.02 bh185000 2008Dec09 DR123516 Support StmtInfo UDT Transforms Off
 *   13.01.00.03 kl185018 2009Mar12 DR125340 32-bit programming for HPUX-ia64
 *   13.01.00.04 bh185000 2009Mar29 DR126915 Support TASM Utility Management
 *   13.10.00.04 kl185018 2009Jun23 DR133468 Version change from 13.01 to 13.10
 *   14.00.00.00 bh185000 2010Oct06 DR121202 Support statement Independence
 *   14.00.00.01 bh185000 2010Dec11 DR145672 Support activity types in TTU14.0
 *   14.00.00.02 bh185000 2011Feb24 DR145224 Support Array Data Type
 *   14.00.01.00 bh185000 2011Jun28 DR147092 Support Client Attribute parcels
 *   14.10.00.00 hs186016 2011Dec15 CLAC-29160 Add missing defs from pclbody.h
 *   14.10.00.01 bh185000 2012Jan10 CLAC-28875 Support XML Data Type
 *   14.10.00.02 mg180007 2012Mar14 CLAC-29474 Add PclRevalidateRefStmt
 *   14.10.00.03 hs186016 2012Mar15 CLAC-29367 Remove unused CliPrepColInfoType
 *                                             and CliPrepColInfoTypeX
 *   14.10.00.04 mg180007 2012Mar28 CLAC-28857 Support ESS
 *   14.10.00.05 mg180007 2012Apr13 CLAC-29422 Activity types for Enhanced
 *                                             Query Capture Facility
 *   14.10.00.06 mg180007 2012Apr23 CLAC-29578 Activity type for Show in XML
 *   14.10.00.07 bh185000 2012May16 CLAC-29283 Support Extended Multiload
 *   14.10.00.08 mg180007 2012May16 CLAC-28857 Added ESS ResponseMode constants
 *   14.10.00.09 hs186016 2012May22 CLAC-29672 Restore CliPrepColInfoType and
 *                                             CliPrepColInfoTypeX into parcel.h
 *   14.10.00.10 en185000 2011Sep21 DR145358, CLAC-29119 AlwaysOn-Redrive
 *   14.10.00.11 mg180007 2012Jun08 CLAC-29222 Support Secuity Policy
 *   14.10.00.12 kl185018 2012Jun29 CLAC-29270 Support TASM FastFail Feature
 *   14.10.00.13 bh185000 2012Jul27 CLAC-29854 Activity types in TTU14.10
 *   14.10.00.14 mg180007 2012Nov05 CLAC-30059 ESS with XSP Default Connection
 *   14.10.00.15 hs186016 2013Jan06 CLAC-18937 NW CLIv2 support for Mac OS X
 *   14.10.00.16 hs186016 2013Feb21 CLAC-30478 Support TASM FastFail(redesigned)
 *   15.00.00.00 bh185000 2013May09 CLAC-30608 Support new activity types
 *   15.00.00.01 hs186016 2013Nov14 CLAC-30554 Support SLOB
 *   15.10.00.00 mg180007 2014Apr28 CLAC-32368 version and copyright update
 *   15.10.00.01 bh185000 2014Jul30 CLAC-32368 SLOB Server to Client support
 *   15.10.00.02 bh185000 2014Aug19 CLAC-32605 Update Activity Types
 *   15.10.00.03 mg180007 2014Sep01 CLAC-31283 Support negotiating mechanisms
 *   15.10.00.04 hs186016 2014Sep17 CLAC-30758 Add Parcel structure for SLOB
 *   16.00.00.00 bh185000 2015Mar12 CLAC-33337 Update Activity Types
 *   16.00.00.01 mg180007 2015Mar23 CLAC-29671 Support more ClientAttributes
 *   16.00.00.02 bh185000 2015Apr20 CLAC-33337 Additional update Activity Types
 *   16.00.00.03 bh185000 2015Jun24 CLAC-29115 Additional fix of 1MB resp row
 *   16.00.00.04 bh185000 2015Sep29 CLAC-34036 Add new activity type
 *   16.00.00.05 bh185000 2015Nov10 CLAC-34171 No Max size in struct definition
 *   16.20.00.00 hs186016 2017Dec06 CLIWS-6627 Add new activity type for
 *                                             Transform Group
 *   16.20.00.01 hs186016 2017Dec06 CLIWS-6720 Add new activity types for Alias
 *   16.20.00.02 hs186016 2018May04 CLIWS-6953 Add new activity type for
 *                                             Incremental Restore
 *   16.20.00.03 hs186016 2018Jul03 CLIWS-7014 Add new activity type for
 *                                             Set Session UDFSearchPath
 *   17.00.00.00 hs186016 2018Oct17 CLIWS-7052 Add new activity type for
 *                                             Execute Analytic Function
 *                                             PCLEXECFUNCSTMT (246)
 *   17.00.00.01 vt186023 2019May22 CLIWS-7192 Support TLS v1.2
 *   17.00.00.02 hs186016 2020Jan06 CLIWS-7256 Add new activity type for
 *                                             Multi-Output Table Support for
 *                                             Analytic functions
 *   17.00.00.03 hs186016 2020Mar10 CLIWS-7544 Add new activity type for
 *                                             Execute Analytic Function
 *                                             PCLEXECFUNCSTMTNOARTOUT (250)
 *   17.10.00.00 hs186016 2020Jul27 CLIWS-7606 version change to 17.10
 *   17.10.00.01 hs186016 2020Sep03 CLIWS-7647 Handle gateway error code 8059
 *   17.10.00.02 hs186016 2021Mar12 CLIWS-7750 Add new activity types for
 *                                             Compute Operational Group (COG)
 *   17.10.00.03 hs186016 2021Mar12 CLIWS-7870 Add new activity types for
 *                                             Spool Map
 *   17.10.00.04 hs186016 2021Mar22 CLIWS-7896 Use pack(1) and pack(pop) for AIX
 *   17.10.00.05 rp255060 2021Mar23 CLIWS-7780 Support ConfigPorts config-response
 *   17.10.00.06 hs186016 2021Apr15 CLIWS-7750 Update new activity types for
 *                                             Compute Operational Group (COG)
 *   17.10.00.07 hs186016 2021Jun03 CLIWS-7751 Support mainframe on z/OS
 *   17.10.00.08 hs186016 2021Jun07 CLIWS-7934 Add new activity types for Native
 *                                             Object Store (NOS) File System
 *   17.20.00.00 hs186016 2022Jan01 CLIWS-8198 version and copyright update
 *   17.20.00.01 rp255060 2022Feb04 CLIWS-7948 Federated authentication support
 *   17.20.00.02 rp255060 2022Oct03 CLIWS-8509 Store encryption mode in ClientAttributeEx
 *   17.20.00.03 js186142 2023Mar16  CLMF-9820 Add jobname and jobid client attributes
 *   17.20.00.04 rp255060 2023Mar27 CLIWS-8666 Support OIDC scope parameter in config response
 *   20.00.00.00 hs186016 2023May11 CLIWS-8696 version and copyright update
 *   20.00.00.01 hs186016 2023Sep15 CLIWS-8852 New Activity Types to support Open Table Format
 *                                             - Create/Alter/Drop DATALAKE
 *   20.00.00.02 hs186016 2023Dec13 CLIWS-8996 Add new Activity Types to support Open Table
 *                                             Format(OTF) - Replace DataLake
 ******************************************************************************/

/***************************************************************/
/*                          INTRODUCTION                       */
/***************************************************************/
/*   A parcel is a unit of information carried in a message &  */
/*   consists of:                                              */
/*                                                             */
/*     1. A "flavor" field (2 bytes) which specifies the kind  */
/*        of parcel.                                           */
/*     2. A length field (2 bytes) which specifies the total   */
/*        parcel length (n) in bytes.                          */
/*     3. The "body" (n-4 bytes) which contains data, the      */
/*        meaning (identification) of which depends on the     */
/*        parcel flavor.                                       */
/*                                                             */
/*   A message may be composed of one or more parcels; however */
/*   a parcel must be contained wholly within a message (i.e., */
/*   parcels cannot span messages).  So the maximum size of a  */
/*   message is 32KBytes - 1, and the fixed part of a message  */
/*   is 78 bytes for a normal request and 85 bytes for TDSP    */
/*   segmented request,  so the maximum size of a parcel is    */
/*                                                             */
/*                     32KBytes - 1 - 78                       */
/*                           OR                                */
/*                     32KBytes - 1 - 85                       */
/*                                                             */
/*                                                             */
/*    IT IS ADVISABLE THAT THE APPLICATIONS *DO NOT*           */
/*            EXCEED THE REQUEST MORE THAN                     */
/*            65000 BYTES FOR 64K SUPPORT                      */
/*                       AND                                   */
/*            32500 BYTES FOR 32K SUPPORT                      */
/*                                                             */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* Notes    Although we would like to hold a parcel flavor in  */
/*          an enumerated type, there would be a problem       */
/*          talking to non-C programs, or even code generated  */
/*          by C's which do different things with enumerated   */
/*          types.  For this reason, constants are defined     */
/*          below of the form PclXYZ where XYZ is an abbrevia- */
/*          tion for the parcel flavor.                        */
/***************************************************************/

#include "coptypes.h"

/***************************************************************/
/*                 TYPEDEFS, PclTYPE & PclHeadType             */
/***************************************************************/

/* DR 68259 ---> */
/* DR84603: Add pack(1) for Win-32 */
#if defined(WIN32) || defined(__APPLE__)       /* CLAC-18937 */
#pragma pack(push,1)
#else
#pragma pack(1)
#endif
/* <--- DR 68259 */

typedef    unsigned short       PclFlavor;
typedef    unsigned short       PclLength;
typedef    unsigned short       PclLength_t;   /* DR68080: Define data type
                                                   same as PclLength  */
/* CLAC-34171 --> */
#ifdef CLIV2_STRUCT_DEF_NO_DEPRECATE
typedef    char                 PclBodyType[SysMaxParcelSize];
#else
typedef    char                 PclBodyType[1];
#endif
/* <--CLAC-34171 */

                                           /*--> DR61637 */
typedef    unsigned int         PclAltLength;
/* CLAC-34171 --> */
#ifdef CLIV2_STRUCT_DEF_NO_DEPRECATE
typedef    char                 PclAltBodyType[SysExtMaxParcelSize];
#else
typedef    char                 PclAltBodyType[1];
#endif
/* <--CLAC-34171 */
                                          /*--> DR61637 */
                                          /*--> DR68615 */
typedef    char                 Pclchar;
typedef    unsigned char        PclByte;
typedef    short                PclInt16;
typedef    short *              PclInt16_Ptr;
typedef    unsigned short       PclWord;
typedef    unsigned short       PclUInt16;
typedef    unsigned short *     PclUInt16_Ptr;
typedef    int                  PclInt32;
typedef    int *                PclInt32_Ptr;
typedef    unsigned int         PclUInt32;
typedef    unsigned int *       PclUInt32_Ptr;
typedef    unsigned int         PclDouble_Word;
typedef    unsigned             Pclprotag_t;
typedef    char *               PclPtrType ;
typedef    char *               Pclstorid_t;

#ifdef WIN32
  typedef  __int64              PclInt64;
  typedef  unsigned __int64     PclUInt64;
#else
  typedef long long             PclInt64;
  typedef unsigned long long    PclUInt64;
#endif
                                       /*--> DR68615 */

/*==================*/
/* struct PclType   */
/*==================*/

struct  PclType
{
    PclFlavor           Flavor;
    PclLength           Length;
    PclBodyType         Body;
};

/*--> DR61637        */
/*===================*/
/* struct PclAltType */
/*===================*/
struct PclAltType
{
    PclFlavor           Flavor;
    PclLength           Length;
    PclAltLength        AltLength;
    PclAltBodyType      Body;
};

/*=========================*/
/* struct PclAltHeadType   */
/*=========================*/
struct PclAltHeadType
{
    PclFlavor           Flavor;
    PclLength           Length;
    PclAltLength        AltLength;
};
/*         DR61637<--*/

/*======================*/
/* struct PclHeadType   */
/*======================*/

struct  PclHeadType
{
    PclFlavor           Flavor;
    PclLength           Length;
};


/***************************************************************/
/*                        PARCEL FLAVORS                       */
/***************************************************************/

#define  PclNOFLAVOR     (PclFlavor)(0)
#define  PclREQ          (PclFlavor)(1)
#define  PclRUNSTARTUP   (PclFlavor)(2)
#define  PclDATA         (PclFlavor)(3)
#define  PclRESP         (PclFlavor)(4)
#define  PclKEEPRESP     (PclFlavor)(5)
#define  PclABORT        (PclFlavor)(6)
#define  PclCANCEL       (PclFlavor)(7)
#define  PclSUCCESS      (PclFlavor)(8)
#define  PclFAILURE      (PclFlavor)(9)
#define  PclRECORD       (PclFlavor)(10)
#define  PclENDSTATEMENT (PclFlavor)(11)
#define  PclENDREQUEST   (PclFlavor)(12)
#define  PclFMREQ        (PclFlavor)(13)
#define  PclFMRUNSTARTUP (PclFlavor)(14)
#define  PclVALUE        (PclFlavor)(15)
#define  PclNULLVALUE    (PclFlavor)(16)
#define  PclOK           (PclFlavor)(17)
#define  PclFIELD        (PclFlavor)(18)
#define  PclNULLFIELD    (PclFlavor)(19)
#define  PclTITLESTART   (PclFlavor)(20)
#define  PclTITLEEND     (PclFlavor)(21)
#define  PclFORMATSTART  (PclFlavor)(22)
#define  PclFORMATEND    (PclFlavor)(23)
#define  PclSIZESTART    (PclFlavor)(24)
#define  PclSIZEEND      (PclFlavor)(25)
#define  PclSIZE         (PclFlavor)(26)
#define  PclRECSTART     (PclFlavor)(27)
#define  PclRECEND       (PclFlavor)(28)
#define  PclPROMPT       (PclFlavor)(29)
#define  PclENDPROMPT    (PclFlavor)(30)
#define  PclREWIND       (PclFlavor)(31)
#define  PclNOP          (PclFlavor)(32)
#define  PclWITH         (PclFlavor)(33)
#define  PclPOSITION     (PclFlavor)(34)
#define  PclENDWITH      (PclFlavor)(35)
#define  PclLOGON        (PclFlavor)(36)
#define  PclLOGOFF       (PclFlavor)(37)
#define  PclRUN          (PclFlavor)(38)
#define  PclRUNRESP      (PclFlavor)(39)
#define  PclUCABORT      (PclFlavor)(40)
#define  PclHOSTSTART    (PclFlavor)(41)
#define  PclCONFIG       (PclFlavor)(42)
#define  PclCONFIGRESP   (PclFlavor)(43)
#define  PclSTATUS       (PclFlavor)(44)
#define  PclIFPSWITCH    (PclFlavor)(45)
#define  PclPOSSTART     (PclFlavor)(46)
#define  PclPOSEND       (PclFlavor)(47)
#define  PclBULKRESP     (PclFlavor)(48)
#define  PclERROR        (PclFlavor)(49)
#define  PclDATE         (PclFlavor)(50)
#define  PclROW          (PclFlavor)(51)
#define  PclHUTCREDBS    (PclFlavor)(52)
#define  PclHUTDBLK      (PclFlavor)(53)
#define  PclHUTDELTBL    (PclFlavor)(54)
#define  PclHUTINSROW    (PclFlavor)(55)
#define  PclHUTRBLK      (PclFlavor)(56)
#define  PclHUTSNDBLK    (PclFlavor)(57)
                                          /* 58 is  not in use */
#define  PclHUTRELDBCLK  (PclFlavor)(59)
#define  PclHUTNOP       (PclFlavor)(60)
#define  PclHUTBLD       (PclFlavor)(61)
#define  PclHUTBLDRSP    (PclFlavor)(62)  /* currently not used*/
#define  PclHUTGETDDT    (PclFlavor)(63)
#define  PclHUTGETDDTRSP (PclFlavor)(64)
#define  PclHUTIDx       (PclFlavor)(65)
#define  PclHUTIDxRsp    (PclFlavor)(66)
#define  PclFieldStatus  (PclFlavor)(67)
#define  PclINDICDATA    (PclFlavor)(68)
#define  PclINDICREQ     (PclFlavor)(69)
                                          /* 70 is not in use  */
#define  PclDATAINFO     (PclFlavor)(71)
#define  PclIVRUNSTARTUP (PclFlavor)(72)
#define  PclOPTIONS      (PclFlavor)(85)
#define  PclPREPINFO     (PclFlavor)(86)
#define  PclCONNECT      (PclFlavor)(88)
#define  PclLSN          (PclFlavor)(89)
#define  PclErrorCnt     (PclFlavor)(105)
#define  PclSESSINFO     (PclFlavor)(106)
#define  PclSESSINFORESP (PclFlavor)(107)
#define  PclSESSOPT      (PclFlavor)(114)
#define  PclVOTEREQUEST  (PclFlavor)(115)
#define  PclVOTETERM     (PclFlavor)(116)
#define  PclCMMT2PC      (PclFlavor)(117)
#define  PclABRT2PC      (PclFlavor)(118)
#define  PclFORGET       (PclFlavor)(119)

/*=============================*/
/*  NDM parcels only.  HN_00   */
/*=============================*/

#define  PclCURSORHOST    (PclFlavor)(120)    /* HN_00 - NDM Support */
#define  PclCURSORDBC     (PclFlavor)(121)    /* HN_00 - NDM Support */
#define  PclFLAGGER       (PclFlavor)(122)    /* HN_00 - NDM Support */

#define  PclPREPINFOX     (PclFlavor)(125)    /* DR 66590 */

/*=============================*/
/*  TDSP Parcels.  DR48735     */
/*=============================*/

#define  PclMULTITSR      (PclFlavor)(128)    /* DR48735 */
#define  PclSPOPTIONSTYPE (PclFlavor)(129)    /* DR48735 */

/*=============================*/
/*   SSO Parcels (DR52052)     */
/*=============================*/

/* Single Sign-On Related Parcels   DR52052, DR51567-rhh     */
#define PcLSSOAUTHREQ    (130)  /*  SSO Authorization Request       */
                                /* Only used in the gateway         */
#define PcLSSOAUTHRESP   (131)  /* SSO Authorization Response       */
                                /* Only used in the gateway         */
#define PcLSSOREQ        (132)  /* SSO Request                      */
#define PcLSSODOMAIN     (133)  /* SSO User Domain                  */
#define PcLSSORESP       (134)  /* SSO Response                     */
#define PcLSSOAUTHINFO   (135)  /* SSO Authentication Info          */
#define PcLUSERNAMEREQ   (136)  /* SSO User Name Request            */
#define PcLUSERNAMERESP  (137)  /* SSO Name Response                */
/* Single Sign-On Related Parcels   DR52052, DR51567-rhh     */
/* begin DR56467 */
#define PclSSOAUTHREQ    PcLSSOAUTHREQ
#define PclSSOAUTHRESP   PcLSSOAUTHRESP
#define PclSSOREQ        PcLSSOREQ
#define PclSSODOMAIN     PcLSSODOMAIN
#define PclSSORESP       PcLSSORESP
#define PclSSOAUTHINFO   PcLSSOAUTHINFO
#define PclUSERNAMEREQ   PcLUSERNAMEREQ
#define PclUSERNAMERESP  PcLUSERNAMERESP
/* end DR56467 */

/*=============================*/
/*     DR57583 - LOBs          */
/*                             */
/*=============================*/

#define PclQMD        (140)  /* MultipartData                    */
#define PclQEMD       (141)  /* EndMultipartData                 */
#define PclQMID       (142)  /* MultipartIndicData               */
#define PclQEMID      (143)  /* EndMultipartIndicData            */
#define PclSMR        (144)  /* MultipartRecord                  */
#define PclSEMR       (145)  /* EndMultipartRecord               */
#define PclXDIX       (146)  /* DataInfoX                        */
#define PclQMRS       (147)  /* MultipartRunStartup              */
#define PclQMR        (148)  /* MultipartRequest                 */
#define PclSEDM       (149)  /* ElicitDataMailbox                */
#define PclSED        (150)  /* ElicitData                       */
#define PclSEF        (151)  /* ElicitFile                       */
#define PclSEDR       (152)  /* ElicitDataReceived               */

/*=============================*/
/* DR57320 Extended Response   */
/*         Parcels             */
/*=============================*/

#define  PclBIGRESP      (PclFlavor)(153) /* DR57130               */
#define  PclBIGKEEPRESP  (PclFlavor)(154) /* DR57130               */

/*=============================*/
/* DR60695 Cursor Positioning  */
/*                             */
/*=============================*/

/* Parcel to indicate positioning may be requested */
#define  PclSETPOSITION    (PclFlavor)(157)
/* Parcel to define the first row to be returned */
#define  PclROWPOSITION     (PclFlavor)(158)
/* Parcel to define where the LOB data should be */
#define  PclOFFSETPOSITION  (PclFlavor)(159)

#define PclRESULTSUMMARY (163) /* DR58352: ResultSummary           */
#define PclERRORINFO     (164) /* DR86770: ErrorInfo               */

/*===============================================*/
/* DR61637 APH Parcels : Set MSB for APH flavors */
/*===============================================*/
#define SYSALTPCLINDICATOR             0x8000
#define PclAltREQ          (PclFlavor) (PclREQ        | SYSALTPCLINDICATOR)
#define PclAltRUNSTARTUP   (PclFlavor) (PclRUNSTARTUP | SYSALTPCLINDICATOR)
#define PclAltDATA         (PclFlavor) (PclDATA       | SYSALTPCLINDICATOR)
#define PclAltCANCEL       (PclFlavor) (PclCANCEL     | SYSALTPCLINDICATOR)
#define PclAltFMREQ        (PclFlavor) (PclFMREQ      | SYSALTPCLINDICATOR)
#define PclAltFMRUNSTARTUP (PclFlavor) (PclFMRUNSTARTUP | SYSALTPCLINDICATOR)
#define PclAltNOFLAVOR     (PclFlavor) (PclNOFLAVOR   | SYSALTPCLINDICATOR)
#define PclAltINDICDATA    (PclFlavor) (PclINDICDATA  | SYSALTPCLINDICATOR)
#define PclAltINDICREQ     (PclFlavor) (PclINDICREQ   | SYSALTPCLINDICATOR)
#define PclAltDATAINFO     (PclFlavor) (PclDATAINFO   | SYSALTPCLINDICATOR)
#define PclAltIVRUNSTARTUP (PclFlavor) (PclIVRUNSTARTUP | SYSALTPCLINDICATOR)
#define PclAltOPTIONS      (PclFlavor) (PclOPTIONS    | SYSALTPCLINDICATOR)
#define PclAltCURSORHOST   (PclFlavor) (PclCURSORHOST | SYSALTPCLINDICATOR)
#define PclAltMULTITSR     (PclFlavor) (PclMULTITSR   | SYSALTPCLINDICATOR)
#define PclAltSPOPTIONSTYPE (PclFlavor) (PclSPOPTIONSTYPE  | SYSALTPCLINDICATOR)
#define PclAltBIGRESP      (PclFlavor) (PclBIGRESP    | SYSALTPCLINDICATOR)
#define PclAltBIGKEEPRESP  (PclFlavor) (PclBIGKEEPRESP | SYSALTPCLINDICATOR)
#define PclAltRESP         (PclFlavor) (PclRESP    |  SYSALTPCLINDICATOR)
#define PclAltKEEPRESP     (PclFlavor) (PclKEEPRESP | SYSALTPCLINDICATOR)
#define PclAltQMD          (PclFlavor) (PclQMD      | SYSALTPCLINDICATOR)
#define PclAltQEMD         (PclFlavor) (PclQEMD     | SYSALTPCLINDICATOR)
#define PclAltQMID         (PclFlavor) (PclQMID     | SYSALTPCLINDICATOR)
#define PclAltQEMID        (PclFlavor) (PclQEMID    | SYSALTPCLINDICATOR)
#define PclAltXDIX         (PclFlavor) (PclXDIX     | SYSALTPCLINDICATOR)
#define PclAltQMRS         (PclFlavor) (PclQMRS     | SYSALTPCLINDICATOR)
#define PclAltQMR          (PclFlavor) (PclQMR      | SYSALTPCLINDICATOR)
#define PclAltREWIND       (PclFlavor) (PclREWIND   | SYSALTPCLINDICATOR)
/* <-- DR61637 */
/* DR 60695 -> */
#define  PclAltSETPOSITION (PclFlavor)(PclSETPOSITION | SYSALTPCLINDICATOR)
#define  PclAltROWPOSITION (PclFlavor)(PclROWPOSITION | SYSALTPCLINDICATOR)
#define  PclAltOFFSETPOSITION \
   (PclFlavor)(PclOFFSETPOSITION | SYSALTPCLINDICATOR)
/* <-- DR 60695 */
#define PclAltERRORINFO    (PclFlavor)(PclERRORINFO | SYSALTPCLINDICATOR)
/* DR 58352 --> */
#define PclAltRESULTSUMMARY (PclFlavor)(PclRESULTSUMMARY | SYSALTPCLINDICATOR)
/* <-- DR 58352 */

/* DR68842 -> */
#define PclGTWCONFIG        (165)    /* Gateway configuration parcel */
#define PclCLIENTCONFIG     (166)    /* Client configuration parcel  */
#define PclAUTHMECH         (167)    /* Gateway Auth Methods parcel  */
/* <- DR68842 */

/* DR98026 -> */
#define PclSTATEMENTINFO    (169)
#define PclSTATEMENTINFOEND (170)
#define PclAltSTATEMENTINFO \
       (PclFlavor)(PclSTATEMENTINFO | SYSALTPCLINDICATOR)
#define PclAltSTATEMENTINFOEND \
       (PclFlavor)(PclSTATEMENTINFOEND | SYSALTPCLINDICATOR)
/* <- DR98026 */

#define PclRESULTSET        (172)    /* DR102528, Dynamic Result Sets */
#define PclAltRESULTSET \
        (PclFlavor)(PclRESULTSET | SYSALTPCLINDICATOR)
#define PclRESULTSETCURPOS  (175)    /* DR109200, SPDR positioning    */
#define PclAltRESULTSETCURPOS \
        (PclFlavor)(PclRESULTSETCURPOS | SYSALTPCLINDICATOR)
#define PclELICITDATABYNAME (176)    /* DR115324, LOB Loader Pahse 2  */
#define PclAltELICITDATABYNAME \
        (PclFlavor)(PclELICITDATABYNAME | SYSALTPCLINDICATOR)
#define PclCLIENTATTRIBUTE  (189)    /* DR147092 Client Attribute     */
#define PclAltCLIENTATTRIBUTE \
        (PclFlavor)(PclCLIENTATTRIBUTE | SYSALTPCLINDICATOR)
#define PclSTMTERROR        (192)    /* DR121202 Statment Independence */
#define PclAltSTMTERROR \
        (PclFlavor)(PclSTMTERROR | SYSALTPCLINDICATOR)

/* DR145358, CLAC-29119 -> */
#define PclSESSIONSTATUS          (194)
#define PclSESSIONSTATUSRESP      (195)
#define PclCONTINUECONTEXT        (196)
#define PclAltCONTINUECONTEXT \
        (PclFlavor)(PclCONTINUECONTEXT | SYSALTPCLINDICATOR)
#define PclCONTROLDATASTART       (197)
#define PclAltCONTROLDATASTART \
        (PclFlavor)(PclCONTROLDATASTART | SYSALTPCLINDICATOR)
#define PclCONTROLDATAEND         (198)
#define PclAltCONTROLDATAEND \
        (PclFlavor)(PclCONTROLDATAEND | SYSALTPCLINDICATOR)
#define PclRECOVERABLEPROTOCOL    (200) /* GTW-DBS use only */
#define PclAltRECOVERABLEPROTOCOL \
        (PclFlavor)(PclRECOVERABLEPROTOCOL | SYSALTPCLINDICATOR)
#define PclREDRIVE                (201)
#define PclAltREDRIVE \
        (PclFlavor)(PclREDRIVE | SYSALTPCLINDICATOR)
#define PclACKREQUESTED           (202)
#define PclAltACKREQUESTED \
        (PclFlavor)(PclACKREQUESTED | SYSALTPCLINDICATOR)
#define PclREDRIVESUPPORTED       (204)
#define PclAltREDRIVESUPPORTED \
        (PclFlavor)(PclREDRIVESUPPORTED | SYSALTPCLINDICATOR)
#define PclSTATEMENTSTATUS        (205)     /* CLAC-28857: ESS               */
#define PclAltSTATEMENTSTATUS \
        (PclFlavor)(PclSTATEMENTSTATUS | SYSALTPCLINDICATOR)
#define PclRECOVERABLENPSUPPORTED (214)
#define PclAltRECOVERABLENPSUPPORTED \
        (PclFlavor)(PclRECOVERABLENPSUPPORTED | SYSALTPCLINDICATOR)
/* <- DR145358, CLAC-29119 */
/* CLAC-29222 -> */
#define PclSECURITYPOLICY         (206)
#define PclPROXYSECURITYPOLICY    (207)
#define PclSECURITYUSED           (208)
#define PclMESSAGEAUDITINFO       (209)
/* <- CLAC-29222 */

/* CLAC-31559 --> */
#define PclSLOBRESPONSE           (215)
#define PclAltSLOBRESPONSE \
        (PclFlavor)(PclSLOBRESPONSE | SYSALTPCLINDICATOR)
/* <-- CLAC-31559 */

/*  CLAC-30554 --> */
#define PclSLOBDATASTART          (220)
#define PclAltSLOBDATASTART \
        (PclFlavor)(PclSLOBDATASTART | SYSALTPCLINDICATOR)
#define PclSLOBDATA               (221)
#define PclAltSLOBDATA \
        (PclFlavor)(PclSLOBDATA | SYSALTPCLINDICATOR)
#define PclSLOBDATAEND            (222)
#define PclAltSLOBDATAEND \
        (PclFlavor)(PclSLOBDATAEND | SYSALTPCLINDICATOR)
/* <-- CLAC-30554  */
/***************************************************************/
/*                     Needed for low CLI                      */
/***************************************************************/

/*=============================================================*/
/*   Purpose  To define the string representations for all     */
/*            parcels and define flavors.  It is always fixed  */
/*            length and is padded with blanks to make up the  */
/*            length.                                          */
/*=============================================================*/

#define  PclSREQ              "REQ             "
#define  PclSRUNSTARTUP       "RUNSTARTUP      "
#define  PclSDATA             "DATA            "
#define  PclSRESP             "RESP            "
#define  PclSKEEPRESP         "KEEPRESP        "
#define  PclSABORT            "ABORT           "
#define  PclSCANCEL           "CANCEL          "
#define  PclSSUCCESS          "SUCCESS         "
#define  PclSFAILURE          "FAILURE         "
#define  PclSERROR            "ERROR           "
#define  PclSRECORD           "RECORD          "
#define  PclSENDSTATEMENT     "ENDSTATEMENT    "
#define  PclSENDREQUEST       "ENDREQUEST      "
#define  PclSFMREQ            "FMREQ           "
#define  PclSFMRUNSTARTUP     "FMRUNSTARTUP    "
#define  PclSVALUE            "VALUE           "
#define  PclSNULLVALUE        "NULLVALUE       "
#define  PclSOK               "OK              "
#define  PclSFIELD            "FIELD           "
#define  PclSNULLFIELD        "NULLFIELD       "
#define  PclSTITLESTART       "TITLESTART      "
#define  PclSTITLEEND         "TITLEEND        "
#define  PclSFORMATSTART      "FORMATSTART     "
#define  PclSFORMATEND        "FORMATEND       "
#define  PclSSIZESTART        "SIZESTART       "
#define  PclSSIZEEND          "SIZEEND         "
#define  PclSSIZE             "SIZE            "
#define  PclSRECSTART         "RECSTART        "
#define  PclSRECEND           "RECEND          "
#define  PclSPROMPT           "PROMPT          "
#define  PclSENDPROMPT        "ENDPROMPT       "
#define  PclSREWIND           "REWIND          "
#define  PclSNOP              "NOP             "
#define  PclSWITH             "WITH            "
#define  PclSPOSITION         "POSITION        "
#define  PclSENDWITH          "ENDWITH         "
#define  PclSLOGON            "LOGON           "
#define  PclSLOGOFF           "LOGOFF          "
#define  PclSRUN              "RUN             "
#define  PclSRUNRESP          "RUNRESP         "
#define  PclSUCABORT          "UCABORT         "
#define  PclSHOSTSTART        "HOSTSTART       "
#define  PclSCONFIG           "CONFIG          "
#define  PclSCONFIGRESP       "CONFIGRESP      "
#define  PclSSTATUS           "STATUS          "
#define  PclSIFPSWITCH        "ITFSWITCH       "
#define  PclSPOSSTART         "POSSTART        "
#define  PclSPOSEND           "POSEND          "
#define  PclSBULKRESP         "BULKRESP        "
#define  PclSDATE             "DATE            "
#define  PclSROW              "ROW             "
#define  PclSHUTCREDBS        "HUTCREDBS       "
#define  PclSHUTDBLK          "HUTDBLK         "
#define  PclSHUTDELTBL        "HUTDELTBL       "
#define  PclSHUTINSROW        "HUTINSROW       "
#define  PclSHUTRBLK          "HUTRBLK         "
#define  PclSHUTSNDBLK        "HUTSNDBLK       "
#define  PclSHUTRELDBCLK      "HUTRELDBCLK     "
#define  PclSHUTNOP           "HUTNOP          "
#define  PclSHUTBLD           "HUTBLD          "
#define  PclSHUTBLDRSP        "HUTBLDRSP       "  /* obsolete  */
#define  PclSHUTGETDDT        "HUTGETDDT       "
#define  PclSHUTGETDDTRSP     "HUTGETDDTRSP    "
#define  PclSHUTIDX           "HUTIDX          "
#define  PclSHUTIDXRSP        "HUTIDXRSP       "
#define  PclSFIELDSTATUS      "FIELDSTATUS     "
#define  PclSINDICDATA        "INDICDATA       "
#define  PclSINDICREQ         "INDICREQ        "
#define  PclSSESSOPT          "SESSOPT         "
#define  PclSDATAINFO         "DATAINFO        "
#define  PclSIVRUNSTARTUP     "IVRUNSTARTUP    "

/* Added the following string representations for 2PC - H0_01, JAE  */
/* HN_01 - Change to upper case. */

#define  PclSASSIGNRSP        "ASSIGNRSP       "
#define  PclSSESSINFO         "SESSINFO        "
#define  PclSSESSINFORESP     "SESSINFORESP    "
#define  PclSVOTEREQUEST      "VOTEREQUEST     "
#define  PclSVOTETERM         "VOTETERM        "
#define  PclSFORGET           "YES/DONE        "
#define  PclS2PCCMMT          "2PCCOMMIT       "
#define  PclS2PCABRT          "2PCABORT        "

/* Added the following string representations for NDM - HN_00, JAE */
/* HN_01 - Change to upper case. */

#define  PclSCURSORHOST       "CURSORHOST      "
#define  PclSCURSORDBC        "CURSORDBC       "
#define  PclSFLAGGER          "FLAGGER         "
                                       /*-->DR61637 */
#define  PclSAltREQ           "AltREQ          "
#define  PclSAltRUNSTARTUP    "AltRUNSTARTUP   "
#define  PclSAltDATA          "AltDATA         "
#define  PclSAltCANCEL        "AltCANCEL       "
#define  PclSAltFMREQ         "AltFMREQ        "
#define  PclSAltFMRUNSTARTUP  "AltFMRUNSTARTUP "
#define  PclSAltNOFLAVOR      "AltNOFLAVOR     "
#define  PclSAltINDICDATA     "AltINDICDATA    "
#define  PclSAltINDICREQ      "AltINDICREQ     "
#define  PclSAltDATAINFO      "AltDATAINFO     "
#define  PclSAltOPTIONS       "AltOPTIONS      "
#define  PclSAltMULTITSR      "AltMULTITSR     "
#define  PclSAltSPOPTIONS     "AltSPOPTIONSTYPE"
#define  PclSAltCURSORHOST    "AltCURSORHOST   "
#define  PclSAltBIGRESP        "AltBIGRESP      "
#define  PclSAltBIGKEEPRESP    "AltBIGKEEPRESP  "
#define  PclSAltRESP           "AltRESP         "
#define  PclSAltKEEPRESP       "AltKEEPRESP     "

                                     /* DR61637<--*/

/*=============================================================*/
/* TYPE     PclStmtType                                        */
/*                                                             */
/*   Purpose  To define the data type for kinds of activities  */
/*            returned in the Ok and Success parcels.          */
/*                                                             */
/*=============================================================*/

typedef     unsigned short       PclStmtType; /* DR68511 */


/*=============================================================*/
/* Purpose  To express the kind of activity within the Ok      */
/*            and Success parcel.                              */
/*                                                             */
/* Notes    In case you hadn't noticed, UPDATE is different    */
/*          from UPDATE...RETRIEVING, and a COMMENT statement  */
/*          which sets the comment string is different from a  */
/*          COMMENT statement which returns this information.  */
/*          The guiding principle is that the recipient of an  */
/*          Ok or Success parcel should be able to tell what   */
/*          parcel sequence to expect next.                    */
/*                                                             */
/*          PclStmtNull is used as a bit of a placeholder in   */
/*          data structures where a PclStmtType value may      */
/*          appear, but is not set in the particular case.     */
/*                                                             */
/*=============================================================*/

#define  PclStmtNull      (PclStmtType)( 0)
#define  PclRetStmt       (PclStmtType)( 1)
#define  PclInsStmt       (PclStmtType)( 2)
#define  PclUpdStmt       (PclStmtType)( 3) /* UPDATE          */
#define  PclUpdRetStmt    (PclStmtType)( 4) /* UPDATE ...      */
                                            /* RETRIEVING      */
#define  PclDelStmt       (PclStmtType)( 5)
#define  PclCTStmt        (PclStmtType)( 6)
#define  PclModTabStmt    (PclStmtType)( 7)
#define  PclCVStmt        (PclStmtType)( 8)
#define  PclCMStmt        (PclStmtType)( 9)
#define  PclDropTabStmt   (PclStmtType)(10)
#define  PclDropViewStmt  (PclStmtType)(11)
#define  PclDropMacStmt   (PclStmtType)(12)
#define  PclDropIndStmt   (PclStmtType)(13)
#define  PclRenTabStmt    (PclStmtType)(14)
#define  PclRenViewStmt   (PclStmtType)(15)
#define  PclRenMacStmt    (PclStmtType)(16)
#define  PclCreIndStmt    (PclStmtType)(17)
#define  PclCDStmt        (PclStmtType)(18)
#define  PclCreUserStmt   (PclStmtType)(19)
#define  PclGrantStmt     (PclStmtType)(20)
#define  PclRevokeStmt    (PclStmtType)(21)
#define  PclGiveStmt      (PclStmtType)(22)
#define  PclDropDBStmt    (PclStmtType)(23)
#define  PclModDBStmt     (PclStmtType)(24)
#define  PclDatabaseStmt  (PclStmtType)(25)
#define  PclBTStmt        (PclStmtType)(26)
#define  PclETStmt        (PclStmtType)(27)
#define  PclAbortStmt     (PclStmtType)(28)
#define  PclNullStmt      (PclStmtType)(29)
#define  PclExecStmt      (PclStmtType)(30)
#define  PclCmntSetStmt   (PclStmtType)(31) /* COMMENT set     */
                                            /*  statement      */
#define  PclCmntGetStmt   (PclStmtType)(32) /* COMMENT return- */
                                            /*  ing statement  */
#define  PclEchoStmt      (PclStmtType)(33)
#define  PclRepViewStmt   (PclStmtType)(34)
#define  PclRepMacStmt    (PclStmtType)(35)
#define  PclCheckPtStmt   (PclStmtType)(36)
#define  PclDelJrnlStmt   (PclStmtType)(37)
#define  PclRollBackStmt  (PclStmtType)(38)
#define  PclRelLockStmt   (PclStmtType)(39)
#define  PclHUTConfigStmt (PclStmtType)(40)
#define  PclVCheckPtStmt  (PclStmtType)(41)
#define  PclDumpJrnlStmt  (PclStmtType)(42)
#define  PclDumpDBStmt    (PclStmtType)(43)
#define  PclRestoreStmt   (PclStmtType)(44)
#define  PclRollForwStmt  (PclStmtType)(45)
#define  PclDelDBStmt     (PclStmtType)(46)
#define  PclClearDumpStmt (PclStmtType)(47)
#define  PclSaveDumpStmt  (PclStmtType)(48)
#define  PclShowStmt      (PclStmtType)(49)
#define  PclHelpStmt      (PclStmtType)(50)
#define  PclBeginLoadStmt (PclStmtType)(51)
#define  PclChkPtLoadStmt (PclStmtType)(52)
#define  PclEndLoadStmt   (PclStmtType)(53)
#define  PclLinStmt       (PclStmtType)(54)

#define  PclGrantLogonStmt  (PclStmtType)(55) /* DCR4505 BEGIN */
#define  PclRevokeLogonStmt (PclStmtType)(56)
#define  PclBegAccLogStmt   (PclStmtType)(57)
#define  PclEndAccLogStmt   (PclStmtType)(58)
#define  PclCollStatStmt    (PclStmtType)(59)
#define  PclDropStatStmt    (PclStmtType)(60) /* DCR 4505  END */

#define  PclSetCollationStmt (PclStmtType)(61)/* DCR 5094      */

/* Begin CLAC-29160, missing definitions from DBS pclbody.h */
#define  PclBegEditStmt	     (PclStmtType)(62)
#define  PclEditStmt         (PclStmtType)(63)
#define  PclExecEditStmt     (PclStmtType)(64)
#define  PclEndEditStmt      (PclStmtType)(65)
/* End CLAC-29160 */

#define  PclReleaseMloadStmt (PclStmtType)(66)/* DR 17876      */

/* Added the following for 2PC - H0_01, JAE                    */

#define  PclEditDelStmt     (PclStmtType)(67)
#define  PclEditInsStmt     (PclStmtType)(68)
#define  PclEditUpdStmt     (PclStmtType)(69)
#define  Pcl2PCVoteReq      (PclStmtType)(76)
#define  Pcl2PCVoteTerm     (PclStmtType)(77)
#define  Pcl2PCCmmt         (PclStmtType)(78)
#define  Pcl2PCAbrt         (PclStmtType)(79)
#define  Pcl2PCForget       (PclStmtType)(80)
#define  PclRevalidateRefStmt (PclStmtType)(89) /* CLAC-29474 */
#define  PclCommitStmt      (PclStmtType)(90)   /* HN_00 */

/* Begin CLAC-29160, missing definitions from DBS pclbody.h */
#define  PclMonVConfig       (PclStmtType)(91) /* Monitor Virtual Config    */
#define  PclMonPConfig       (PclStmtType)(92) /* Monitor Physical Config   */
#define  PclMonVSummary      (PclStmtType)(93) /* Monitor Virtual Summary   */
#define  PclMonPSummary      (PclStmtType)(94) /* Monitor Physical Summary  */
#define  PclMonVRes          (PclStmtType)(95) /* Monitor Virtual Resource  */
#define  PclMonPRes          (PclStmtType)(96) /* Monitor Physical Resource */
/* End CLAC-29160 */

/*Begin DR41685*/
#define  PclCTrigStmt        (PclStmtType)(97)
#define  PclDTrigStmt        (PclStmtType)(98)
#define  PclRenTrigStmt      (PclStmtType)(99)
#define  PclRepTrigStmt      (PclStmtType)(100)
#define  PclAltTrigStmt      (PclStmtType)(101)
/*End DR41685*/

/* CLAC-29160, missing definition from DBS pclbody.h */
#define  PclRdlStmt          (PclStmtType)(102) /* Replication statement */

/* Begin DR48735 */
#define PclDropProcStmt      (PclStmtType)(103)
#define PclCreateProcStmt    (PclStmtType)(104)
#define PclCallStmt          (PclStmtType)(105)
#define PclRenProcStmt       (PclStmtType)(106)
#define PclRepProcStmt       (PclStmtType)(107) /* DR50139       */
/* End DR48735 */

/* CLAC-29160, missing definition from DBS pclbody.h */
#define PclSetAcct           (PclStmtType)(108) /* Set Session Account */

                                               /*->DR60508       */
/* Introduced through DBS DR 51806   */
#define PclHulStmt (109)                       /* Locking logger */

/* Introduced through DBS 50391      */
#define PclMonSql  (PclStmtType)(110)         /* Monitor Session */
#define PclMonVer  (PclStmtType)(111)         /* Monitor SQL     */

/*Introduced through DBS DR 55105    */
#define PclBeginDBQLStmt  (PclStmtType)(112)  /* Begin DBQL       */
#define PclEndDBQLStmt    (PclStmtType)(113)  /* End DBQL         */

/* Introduced through DBS DR 52715   */
#define PclCreRoleStmt    (PclStmtType)(114)  /* Create Role SQL   */
#define PclDrpRoleStmt    (PclStmtType)(115)  /* Drop Role SQL     */
#define PclGrantRoleStmt  (PclStmtType)(116)  /* Grant Role SQL    */
#define PclRevokeRoleStmt (PclStmtType)(117)  /* Revoke Role SQL   */
#define PclCreProfileStmt (PclStmtType)(118)  /* Create Profile SQL*/
#define PclModProfileStmt (PclStmtType)(119)  /* Modify Profile SQL*/
#define PclDrpProfileStmt (PclStmtType)(120)  /* Drop Profile SQL  */
#define PclSetRoleStmt    (PclStmtType)(121)  /* Set Role SQL      */

                                    /*--->  DR67892 stmt types UDF */
#define PclCreUDFStmt     (PclStmtType)(122) /* Create  UDF */
#define PclRplcUDFStmt    (PclStmtType)(123) /* Replace UDF */
#define PclDropUDFStmt    (PclStmtType)(124) /* Drop    UDF */
#define PclAlterUDFStmt   (PclStmtType)(125) /* Alter   UDF */
#define PclRenUDFStmt     (PclStmtType)(126) /* Rename  UDF */
                                    /*  DR67892 stmt types UDF <---*/

#define PclMrgMixedStmt      (PclStmtType)(127)
#define PclMrgUpdStmt        (PclStmtType)(128)
#define PclMrgInsStmt        (PclStmtType)(129)
                                               /*DR60508       <---*/

                                               /* DR62240--->      */

#define PclAlterProcStmt      (PclStmtType)(130) /* ALTER PROCEDURE SQL */
#define PclTDQMEnablEStmt     (PclStmtType)(131) /* Enable          */
#define PclTDQMStatsStmt      (PclStmtType)(132) /* Statistics      */
#define PclTDQMPerfGroupsStmt (PclStmtType)(133) /* get Perf Groups */

                                               /* DR62240       <---*/

/* DR85431 --> */
/* DR64870 -> User Defined Types */
#define PclCreUDTStmt    (PclStmtType)(134) /* Create UDT        */
#define PclDropUDTStmt   (PclStmtType)(135) /* Drop UDT          */
#define PclAlterUDTStmt  (PclStmtType)(136) /* Alter UDT         */
#define PclRplcUDTStmt   (PclStmtType)(137) /* Replace UDT       */
#define PclCreUDMStmt    (PclStmtType)(138) /* Create UDM        */
#define PclAlterUDMStmt  (PclStmtType)(139) /* Alter UDM         */
#define PclRplcUDMStmt   (PclStmtType)(140) /* Replace UDM       */
#define PclCreCastStmt   (PclStmtType)(141) /* Create Cast       */
#define PclRplcCastStmt  (PclStmtType)(142) /* Replace Cast      */
#define PclDropCastStmt  (PclStmtType)(143) /* Drop Cast         */
#define PclCreOrdStmt    (PclStmtType)(144) /* Create Ordering   */
#define PclRplcOrdStmt   (PclStmtType)(145) /* Replace Ordering  */
#define PclDropOrdStmt   (PclStmtType)(146) /* Drop Ordering     */
#define PclCreXFormStmt  (PclStmtType)(147) /* Create Transform  */
#define PclRplcXFormStmt (PclStmtType)(148) /* Replace Transform */
#define PclDropXFormStmt (PclStmtType)(149) /* Drop Transform    */
/* <- DR64870 */

/* DR68323 -> V2R6 External Routine Security - AUTHORIZATION */
#define PclCreAuthStmt    (PclStmtType)(150)       /* Create */
#define PclDrpAuthStmt    (PclStmtType)(151)       /* Drop   */
/* <- DR68323 */

/* DR68596 -> V2R6 Replication Services - REPLICATION GROUP */
#define PclCreRepGrpStmt  (PclStmtType)(152)      /* Create */
#define PclAltRepGrpStmt  (PclStmtType)(153)      /* Alter  */
#define PclDrpRepGrpStmt  (PclStmtType)(154)      /* Drop   */
/* <- DR68596 */
/* <-- DR85431 */

/* DR105446 -> */
#define PclTwmDelRqstChgStmt   (PclStmtType)(155)
#define PclTwmSummaryStmt      (PclStmtType)(156)
#define PclTwmDynRuleStmt      (PclStmtType)(157)
#define PclTwmDynObjStmt       (PclStmtType)(158)
#define PclTwmWdAssignmentStmt (PclStmtType)(159)
#define PclTwmDynBuildStmt     (PclStmtType)(160)
#define PclTwmListWdStmt       (PclStmtType)(161)
/* <- DR105446 */

/* DR92423 -> */
#define PclSetSesIsoLvlStmt  (PclStmtType)(162)  /* tx-isolation       */
/* <- DR92423 */

/* DR105446 -> */
#define PclInitIdxAnalysis   (PclStmtType)(163)  /* InitIndex Analysis */
#define PclRplcAuthStmt      (PclStmtType)(164)  /* Replace Auth Stmt  */
/* <- DR105446 */

/* DR103360 -> */
#define  PclSetQBandStmt     (PclStmtType)(165)  /* Query Banding      */
/* <- DR103360 */

/* DR105446 -> */
#define  PclLogArcOnStmt     (PclStmtType)(166)  /* Online Archieve    */
#define  PclLogArcOffStmt    (PclStmtType)(167)  /* Online Archieve    */
#define  PclMonQueryBandStmt (PclStmtType)(168)  /* Monitor Query Band */
#define  PclCreCorrStmt      (PclStmtType)(169)  /* Column Correlation */
#define  PclRepCorrStmt      (PclStmtType)(170)  /* Column Correlation */
#define  PclDrpCorrStmt      (PclStmtType)(171)  /* Column Correlation */
#define  PclAltCorrStmt      (PclStmtType)(172)  /* Column Correlation */
/* <- DR105446 */

/* DR111230 --> */
#define  PclUserEventControlStmt (PclStmtType)(173) /* User Event Cntl  */
#define  PclEventStatusStmt  (PclStmtType)(174)  /* Event Status        */
#define  PclMonAWTResStmt    (PclStmtType)(175)  /* Monitor AWT Res     */
#define  PclSPDynResultSet   (PclStmtType)(176)  /* SP Result Set       */
/* <-- DR111230 */

/* DR115327 --> */
#define  PclCreRepRulSetStmt (PclStmtType)(177)  /* Create Rep. RulSet  */
#define  PclRplRepRulSetStmt (PclStmtType)(178)  /* Replace Rep. RulSet */
#define  PclDrpRepRulSetStmt (PclStmtType)(179)  /* Drop Rep. RulSet    */
/* <-- DR115327 */

/* DR115330 --> */
#define PclCreUDOStmt (PclStmtType)(180)         /* Create Operator     */
#define PclRepUDOStmt (PclStmtType)(181)         /* Replace Operator    */
#define PclRenUDOStmt (PclStmtType)(182)         /* Rename Operator     */
#define PclDrpUDOStmt (PclStmtType)(183)         /* Drop Operator       */
/* <-- DR115330 */

/* DR119336 --> */
#define  PclGrantConnThrStmt  (PclStmtType)(184) /* Grant Conn. Through */
#define  PclRevokeConnThrStmt (PclStmtType)(185) /* Revoke Conn. Through*/
#define  PclCreateGLOPSetStmt (PclStmtType)(186) /* Create GLOP Set     */
#define  PclDropGLOPSetStmt   (PclStmtType)(187) /* Drop GLOP Set       */
/* <-- DR119336 */

/* DR121443 --> */
#define  PclCreConstraint     (PclStmtType)(188) /* Create Sec. Const.  */
#define  PclAltConstraint     (PclStmtType)(189) /* Alter Sec. Const.   */
#define  PclDrpConstraint     (PclStmtType)(190) /* Drop Sec. Const.    */
/* <-- DR121443 */

/* DR145672, CLAC-30608 --> */
#define  PclCreateIndexTypeStmt  (PclStmtType)(191) /* Create Ind type  */
#define  PclDropIndexTypeStmt    (PclStmtType)(192) /* Drop Index Type  */
#define  PclReplaceIndexTypeStmt (PclStmtType)(193) /* Replace Ind Type */
#define  PclAlterIndexStmt       (PclStmtType)(194) /* Alter Index      */
#define  PclCheckWorkloadForStmt (PclStmtType)(195) /* Chk Workload For */
#define  PclNoSplExportStmt      (PclStmtType)(196) /* Fexp No Spooling */
#define  PclCheckWorkloadEndStmt (PclStmtType)(197) /* Chk Workload End */
#define  PclFlushDBQLStmt        (PclStmtType)(198) /* Flush DBQL Cache */
#define  PclTDWMExpStmt          (PclStmtType)(199) /* TDWM Exception   */
#define  PclTDWMTestStmt         (PclStmtType)(200) /* TDWM Test        */
#define  PclMonTDWMResrStmt      (PclStmtType)(201) /* Monitor TDWM Res */
#define  PclMonWDResrStmt        (PclStmtType)(202) /* Monitor WD PM    */
#define  PclRegisterObj          (PclStmtType)(203) /* Register XML     */
#define  PclSetSessCalendarStmt  (PclStmtType)(204) /* Exp Business day */
/* <-- DR145672, CLAC-30608 */
/* CLAC-29283 --> */
#define  PclMonReqStmt          (PclStmtType)(205)  /* Monitor Request  */
#define  PclMrgDelStmt          (PclStmtType)(206)  /* Merge-Into Del   */
/* <--CLAC-29283 */
/* CLAC-29422 --> */
#define  PclBeginDBQCStmt       (PclStmtType)(207)  /* Begin Query Capt */
#define  PclEndDBQCStmt         (PclStmtType)(208)  /* End Query Capt   */
/* <-- CLAC-29422 */
/* CLAC-29578 --> */
#define PclShowXMLStmt          (PclStmtType)(209)  /* Show obj in XML  */
/* <-- CLAC-29578 */
/* CLAC-29854 --> */
#define PclGtwHostGroupStmt     (PclStmtType)(210)  /* Gtw Hostgroup Pro*/
#define PclProxyConfigStmt      (PclStmtType)(211)  /* ProxyConfig      */
#define PclSecureAttributeStmt  (PclStmtType)(212)  /* SecureAttribute  */
#define PclSetSessVeryHotStmt   (PclStmtType)(213)  /* VH FSG Cache     */
/* <-- CLAC-29854 */
/* CLAC-30608 --> */
#define PclUnitySQLStmt         (PclStmtType)(214)  /* Unity SQL Stmt   */
#define PclSetSessSearchUIFDbPath (PclStmtType)(215) /* Aster Support   */
#define PclCreateZoneStmt       (PclStmtType)(216)  /* Create Zone      */
#define PclAlterZoneStmt        (PclStmtType)(217)  /* Alter Zone       */
#define PclDropZoneStmt         (PclStmtType)(218)  /* Drop Zone        */
#define PclCreateSrvStmt        (PclStmtType)(219)  /* Create Fore Srv  */
#define PclAlterSrvStmt         (PclStmtType)(220)  /* Alter Fore Srv   */
#define PclDropSrvStmt          (PclStmtType)(221)  /* Drop Foreign Srv */
#define PclBeginIsoLoadStmt     (PclStmtType)(222)  /* Begin Iso Load   */
#define PclChkPointLoadStmt     (PclStmtType)(223)  /* Checkpt Iso Load */
#define PclEndIsoLoadStmt       (PclStmtType)(224)  /* End Iso Load     */
#define PclSetSessLoadStmt      (PclStmtType)(225)  /* Set Sess Load    */
#define PclGrantZoneStmt        (PclStmtType)(226)  /* Grant Zone       */
#define PclRevokeZoneStmt       (PclStmtType)(227)  /* Revoke Zone      */
/* <-- CLAC-30608 */
/* CLAC-32605 --> */
#define PclForViewpointStmt     (PclStmtType)(228)  /* For Viewpoint    */
#define PclSetSessJSONIgnoreErrors (PclStmtType)(229)  /* Set Sess JSON */
#define PclInitiateChkStmt      (PclStmtType)(230)  /* CheckJI Act Type */
/* <-- CLAC-32605 */
/* CLAC-33337 --> */
#define PclCreateMapStmt        (PclStmtType)(231)  /* Create Map       */
#define PclSetSessUPTStmt       (PclStmtType)(232)  /* Set Session UPT  */
#define PclDropMapStmt          (PclStmtType)(233)  /* Drop Map         */
#define PclGrantMapStmt         (PclStmtType)(234)  /* Grant Map        */
#define PclRevokeMapStmt        (PclStmtType)(235)  /* Revoke Map       */
#define PclSetSessDotNotationOnError (PclStmtType)(236) /* Sess Dot No. */
#define PclCreSchemaStmt        (PclStmtType)(237)  /* Create Schema    */
#define PclDropSchemaStmt       (PclStmtType)(238)  /* Drop Schema      */
#define PclSetSessUdbStmt       (PclStmtType)(239)  /* Debug Function   */
/* <-- CLAC-33337 */
/* CLAC-34036 --> */
#define PclSetFSAttr            (PclStmtType)(240)  /* Foreign Server   */
/* <-- CLAC-34036 */
/* CLIWS-6627 --> */
#define PclSetTransformStmt     (PclStmtType)(241)  /* Set Transform Group */
/* <-- CLIWS-6627 */
/* CLIWS-6720 --> */
#define PclCreAliasStmt         (PclStmtType)(242)  /* Create Alias     */
#define PclRepAliasStmt         (PclStmtType)(243)  /* Replace Alias    */
#define PclDrpAliasStmt         (PclStmtType)(244)  /* Drop Alias       */
/* <-- CLIWS-6720 */
/* CLIWS-6953 --> */
#define PclIncRestoreAllWriteStmt (PclStmtType)(245) /* Incremental Restore */
/* <-- CLIWS-6953 */
/* CLIWS-7052 --> */
#define PclExecFuncStmt         (PclStmtType)(246)  /* Execute Analytic Func */
/* <-- CLIWS-7052 */
/* CLIWS-7014 --> */
#define PclSetSessUDFSearchPath (PclStmtType)(247)  /* Set Sess UDFSearchPath*/
/* <-- CLIWS-7014 */
/* CLIWS-7256 --> */
#define PclFncMapRetStmt        (PclStmtType)(248)  /* Select with table     */
                                                    /* creation message info */
#define PclFncMapInsSelStmt     (PclStmtType)(249)  /* Insert with table     */
                                                    /* creation message info */
/* <-- CLIWS-7256 */
/* CLIWS-7544 --> */
#define PclExecFuncStmtNoARTOut (PclStmtType)(250)  /* Execute Analytic Func */
                                                    /* No ART table Output   */
                                               /* ART: Analytic Result Table */
/* <-- CLIWS-7544 */

/* CLIWS-7934 --> */
#define PclCreateStorageStmt    (PclStmtType)(253)  /* Create Storage        */
#define PclAlterStorageStmt     (PclStmtType)(254)  /* Alter Storage         */
#define PclDropStorageStmt      (PclStmtType)(255)  /* Drop Storage          */
#define PclGrantStorageStmt     (PclStmtType)(256)  /* Grant Storage         */
#define PclRevokeStorageStmt    (PclStmtType)(257)  /* Revoke Storage        */
/* <-- CLIWS-7934 */

/* CLIWS-7750 --> */
#define PclCreateCOGMapStmt     (PclStmtType)(258)  /* Create COG Map        */
#define PclDropCOGMapStmt       (PclStmtType)(259)  /* Drop COG Map          */
#define PclCreCOGGroupStmt      (PclStmtType)(260)  /* Create COG Group      */
#define PclDropCOGGroupStmt     (PclStmtType)(261)  /* Drop COG Group        */
#define PclModCOGGroupStmt      (PclStmtType)(262)  /* Modify COG Group      */
#define PclCreCOGPrfStmt        (PclStmtType)(263)  /* Create COG Profile    */
#define PclDropCOGPrfStmt       (PclStmtType)(264)  /* Drop COG Profile      */
#define PclModCOGPrfStmt        (PclStmtType)(265)  /* Modify COG Profile    */
#define PclResumeCOGPrfStmt     (PclStmtType)(266)  /* Resume COG Profile    */
#define PclSuspendCOGPrfStmt    (PclStmtType)(267)  /* Suspend COG Profile   */
#define PclSetSessCOGGroupStmt  (PclStmtType)(268)  /* Set Session COG Group */
/* <-- CLIWS-7750 */

/* CLIWS-7870 --> */
#define PclCreateSpoolMapStmt   (PclStmtType)(269)  /* Create Spool Map      */
#define PclEnableSpoolMapStmt   (PclStmtType)(270)  /* Enable Spool Map      */
#define PclDisableSpoolMapStmt  (PclStmtType)(271)  /* Disable Spool Map     */
#define PclDropSpoolMapStmt     (PclStmtType)(272)  /* Drop Spool Map        */
/* <-- CLIWS-7870 */

/* CLIWS-7750 --> */
#define PclGrantOnCOGGrpStmt    (PclStmtType)(273)  /* Grant on COG Group    */
#define PclRevokeOnCOGGrpStmt   (PclStmtType)(274)  /* Revoke on COG Group   */
/* <-- CLIWS-7750 */

/* CLIWS-8852 --> */
#define PclCreateDataLakeStmt    (PclStmtType)(275)  /* Create DataLake      */
#define PclAlterDataLakeStmt     (PclStmtType)(276)  /* Alter DataLake       */
#define PclDropDataLakeStmt      (PclStmtType)(277)  /* Drop DataLake        */
/* <-- CLIWS-8852 */

/* CLIWS-8996 --> */
#define PclReplaceDataLakeStmt   (PclStmtType)(278)  /* Replace DataLake     */
/* <-- CLIWS-8996 */

/*----------------------------------------------------------------------*/
/* It is expected ( particularly by BTEQ ) that the names added for     */
/* activity types will mirror the spelling that the DBS uses for the    */
/* macros they define in their PclBody.h header file. DBS developers    */
/* use DR 59863 to reserve the numbers and names. The only difference   */
/* between what is used in DBS' PclBody.h and CLI's parcel.h entries    */
/* should be in case. DBS uses all uppercase and CLI uses mixed case.   */
/*                                                                      */
/* For example... If DBS is adding PCLCREUDOSTMT, it will be expected   */
/* that CLI in turn will add PclCreUDOStmt.                             */
/*----------------------------------------------------------------------*/

/*=============================*/
/*  LAN message parcels only.  */
/*=============================*/

#define  PclASSIGN        (PclFlavor)(100)
#define  PclASSIGNRSP     (PclFlavor)(101)

/*====================*/
/* STRUCT PCLSUCCESS  */
/*====================*/

struct  PclSUCCESSType
{
    PclFlavor          PclKind;          /* 8                 */
    PclLength          Length;           /* 14 to 269         */
    PclWord            StatementNo;
    Pclchar            ActivityCount[4]; /* four byte integer */
    PclWord            WarningCode;
    PclWord            FieldCount ;
    PclWord            ActivityType;
    PclWord            WarningLength;
    Pclchar            WarningMsg[255];  /* 0 to 255 bytes    */
};

struct  PclAltSUCCESSType
{
    PclFlavor          PclKind;          /* 0x8008            */
    PclLength          Length;           /* 0                 */
    PclAltLength       AltLength;        /* varies            */
    PclWord            StatementNo;
    Pclchar            ActivityCount[4]; /* four byte integer */
    PclWord            WarningCode;
    PclWord            FieldCount ;
    PclWord            ActivityType;
    PclWord            WarningLength;
    Pclchar            WarningMsg[255];  /* 0 to 255 bytes    */
};

/*====================*/
/*   STRUCT PCLOk     */
/*====================*/

struct PclOkType
{
    PclFlavor          PclKind;          /* 17                */
    PclLength          Length;           /* 14 to 269         */
    PclWord            StatementNo;
    PclWord            FieldCount ;
    PclUInt32          ActivityCount;    /* DR42286, unsigned */
    PclWord            ActivityType;
    PclWord            WarningCode;
    PclWord            WarningLength;
    Pclchar            WarningMsg[255];  /* 0 to 255 bytes    */
};

struct PclAltOkType
{
    PclFlavor          PclKind;          /* 0x8011            */
    PclLength          Length;           /* 0                 */
    PclAltLength       AltLength;        /* varies            */
    PclWord            StatementNo;
    PclWord            FieldCount ;
    PclUInt32          ActivityCount;    /* DR42286, unsigned */
    PclWord            ActivityType;
    PclWord            WarningCode;
    PclWord            WarningLength;
    Pclchar            WarningMsg[255];  /* 0 to 255 bytes    */
};

/* DR58352 --> */

struct  PclResultSummaryType
{
    PclFlavor          PclKind;          /* 163               */
    PclLength          Length;           /* varies            */
    PclWord       	   StatementNo;
    PclUInt64	 	   ActivityCount;  /* unsigned 64-bit integer */
    PclWord            FieldCount;
    PclWord            ActivityType;
    Pclchar            Mode; /* R=Record, F=Field, I=Indicator, M=Multipart */
    Pclchar            reserved[9];	/* for future use */
};

struct  PclAltResultSummaryType
{
    PclFlavor          PclKind;          /* 0x80a3            */
    PclLength          Length;           /* 0                 */
    PclAltLength       AltLength;        /* varies            */
    PclWord       	   StatementNo;
    PclUInt64	 	   ActivityCount;  /* unsigned 64-bit integer */
    PclWord            FieldCount;
    PclWord            ActivityType;
    Pclchar            Mode; /* R=Record, F=Field, I=Indicator, M=Multipart */
    Pclchar            reserved[9];	/* for future use */
};

struct ResultSummaryExtension
{
#define WARN_MESSAGE          1
   PclWord                    ExtInfoID;   /* unsigned 16-bit integer */
   PclWord                    ExtInfoLength; /* unsigned 16-bit integer */
   PclWord                    ExtInfoBody;
};

struct ResultSummaryExtensionWarning
{
   PclWord                  code;
   char                     warning_msg[1];
};

/* <-- DR58352 */

/*====================*/
/* STRUCT PCLFAILURE  */
/*====================*/

struct PclFAILUREType
{
    PclFlavor          PclKind;          /*  9                */
    PclLength          PLength;          /* 13 to 267         */
    PclWord            StatementNo;
    PclWord            Info;
    PclWord            Code;
    PclWord            Length;
    Pclchar            Msg[255];         /* 1 to 255 bytes    */
};

struct PclAltFAILUREType
{
    PclFlavor          PclKind;          /* 0x8009            */
    PclLength          PLength;          /* 0                 */
    PclAltLength       AltLength;        /* varies            */
    PclWord            StatementNo;
    PclWord            Info;
    PclWord            Code;
    PclWord            Length;
    Pclchar            Msg[255];         /* 1 to 255 bytes    */
};

/*====================*/
/*   STRUCT PCLERROR  */
/*====================*/

struct PclERRORType
{
    PclFlavor          PclKind;          /* 49                */
    PclLength          PLength;          /* 13 to 267         */
    PclWord            StatementNo;
    PclWord            Info;
    PclWord            Code;
    PclWord            Length;
    Pclchar            Msg[255];         /* 1 to 255 bytes    */
};

struct PclAltERRORType
{
    PclFlavor          PclKind;          /* 0x8031            */
    PclLength          PLength;          /* 0                 */
    PclAltLength       AltLength;        /* varies            */
    PclWord            StatementNo;
    PclWord            Info;
    PclWord            Code;
    PclWord            Length;
    Pclchar            Msg[255];         /* 1 to 255 bytes    */
};

/*====================*/
/*   STRUCT PCLRUN    */
/*====================*/

struct PclRUNType
{
    PclFlavor          PclKind;          /* 38 */
    PclLength          Length;           /* 20 */
    Pclchar            Runstring[16];
};

/*====================*/
/*   STRUCT PCLAltRUN */
/*====================*/
                     /*--> DR61637 */
struct PclAltRUNType
{
    PclFlavor       PclKind;          /* 32812 */
    PclLength       Length;           /* 0 */
    PclAltLength    AltLength;        /* 24 */
    Pclchar         Runstring[16];
};
                   /* DR61637<---*/

/*====================*/
/*   STRUCT PCLASG    */
/*====================*/

struct PclASSIGNType
{
    PclFlavor       PclKind;          /* 100 */
    PclLength       Length;           /* 97  */
    Pclchar         username[32];   /* DR52058: To be increased to
                                       92 ? */
};

/*====================*/
/* STRUCT PCLASGRESP  */
/*====================*/

struct PclASSIGNRSPType
{
    PclFlavor       PclKind;         /* 101                    */
    PclLength       Length;          /* 98                     */
    Pclchar         PublicKey[8];    /* Public key is up to an */
                                     /* 8 digit number which   */
                                     /* is the encryption part */
                                     /* of the public key.     */
    Pclchar         SesCopAddr[32];  /* SesCopAddr is the net- */
                                     /* work address to connect*/
                                     /* to for the session     */
    Pclchar         PublicKeyN[32];

                   /* PublicKeyN is the value you mod the      */
                   /* encryption value by.  Also it informs the*/
                   /* user how many bytes to encrypt at one    */
                   /* time.  N should be a value large  enough */
                   /* to encrypt 32 bits of data at a time.    */
                   /* If the value is small,  say under 255,   */
                   /* this is the initial code which  does not */
                   /* require long math.  This will be fixed   */
                   /* later.   So encrypt 32 bits at a time    */
                   /* into 64 bits values.  If it is the       */
                   /* initial code, then use a nibble at a time*/
                   /* and  encrypt into a byte (8 bits).       */

    Pclchar         RelArray[6];     /* RelArray contains the  */
                                     /* release number string  */
    Pclchar         VerArray[14];    /* VerArray contains the  */
                                     /* version number string  */
    PclInt16        HostID;          /* HostID contains logical*/
                                     /* hostid to be returned  */
                                     /* in dbcarea             */
                                     /* 87Nov03 DR11006 DAA    */
};

/*===================*/
/*  STRUCT PCLRESP   */
/*===================*/

struct PclRespType
{
    PclFlavor       PclKind;          /* 4 */
    PclLength       Length;           /* 6 */
    PclInt16        MaxMsgSize;
};
                      /*--> DR57320 */
/*===================*/
/* STRUCT PCLBIGRESP */
/*===================*/
struct PclBigRespType

{
    PclFlavor       PclKind;          /* 153 or 154 */
    PclLength       Length;           /* 8          */
    PclUInt32       MaxMsgSize;
};
                      /* DR57320 <--*/

                       /*--> DR61637 */
/*======================*/
/* STRUCT PCLALTBIGRESP */
/*======================*/
struct PclAltBigRespType
{
    PclFlavor       PclKind;          /* 153 + 0x8000 */
    PclLength       Length;           /*  0           */
    PclAltLength    AltLength;        /* 12           */
    PclUInt32       MaxMsgSize;
};

/*======================*/
/* STRUCT PCLALTRESP */
/*======================*/
struct PclAltRespType
{
    PclFlavor       PclKind;          /* 153 + 0x8000 */
    PclLength       Length;           /*  0           */
    PclAltLength    AltLength;        /* 10           */
    PclUInt16       MaxMsgSize;
};
                      /* DR61637 <--*/

/*===================*/
/*   STRUCT PCLREQ   */
/*===================*/

struct PclReqType
{
    PclFlavor       PclKind;          /* 1 */
    PclLength       Length;           /* ?? */
    /* CLAC-34171 --> */
    #ifdef CLIV2_STRUCT_DEF_NO_DEPRECATE
    Pclchar         Body[SysMaxRequestSize];
    #else
    Pclchar         Body[1];
    #endif
    /* <--CLAC-34171 */
};

                                /*-->DR61637 */
/*===================*/
/*  STRUCT PCLAltREQ */
/*===================*/

struct PclAltReqType
{
    PclFlavor       PclKind;          /* 32769 */
    PclLength       Length;           /* 0     */
    PclAltLength    AltLength;        /* ??    */
    /* CLAC-34171 --> */
    #ifdef CLIV2_STRUCT_DEF_NO_DEPRECATE
    Pclchar         Body[SysAltMaxRequestSize];
    #else
    Pclchar         Body[1];
    #endif
    /* <--CLAC-34171 */
};
                              /* DR61637<--*/

/*******************************************************************
* Parcel Structures - DR48735, TDSP                                *
*                     PclSPOptionsType                             *
*                     PclMultiTsr                                  *
*  added for TDSP support                                          *
*******************************************************************/

struct PclSPOptionsType          /* DR48735, TDSP             */
{
    PclFlavor     Flavor;        /*  PCLSPOPTIONS		      */
    PclLength     Length;        /*  6	                      */
    Pclchar       SPPrintOption; /* 'P' - PRINT	              */
                                 /* 'N' - NOPRINT             */
    Pclchar       WithSPLText;   /* 'Y' - SPL Code needs encryption */
};

                                               /*-->DR61637   */
struct PclAltSPOptionsType
{
    PclFlavor     Flavor;        /*  PclAltSPOPTIONSTYPE      */
    PclLength     Length;        /*   0                       */
    PclAltLength  AltLength;     /*  10                       */
    Pclchar       SPPrintOption; /* 'P' - PRINT               */
                                 /* 'N' - NOPRINT             */
    Pclchar       WithSPLText;   /* 'Y' - SPL Code needs encryption   */
};
                                                /* DR61637<---*/


struct PclMultiTSR	       /* DR48735, TDSP			          */
{
    PclFlavor      Flavor;      /* New Parcel Flavor          */
    PclLength      Length;      /* Length of this structure	  */
    PclWord        SeqNum;      /* Sequence No. starting from 1 within */
                                /* the series of messages     */
    PclByte        IsLast;      /* 1 - Message is last		  */
                                /* 0 - Else			          */
};

                                               /*-->DR61637   */
struct PclAltMultiTSR
{
    PclFlavor      Flavor;      /* New Parcel Flavor          */
    PclLength      Length;      /* 0                  	      */
    PclAltLength   AltLength;   /* Length of this structure   */
    PclWord        SeqNum;      /* Sequence No. starting from 1 within */
                                /* the series of messages     */
    PclByte        IsLast;      /* 1 - Message is last        */
                                /* 0 - Else                   */
};
                                               /* DR61637<---*/

/*******************************************************************/
/*  HN_00 - The following defines the NDM Parcels - JAE 05AUG94    */
/*******************************************************************/

/*==================*/
/* STRUCT FLAGTYPE  */
/*==================*/

typedef struct PclFlagType
{
    Int16       Location;            /* Warning position                */
    Int16       Reason;              /* Warning number                  */
    Int16       MsgLength;           /* Length in bytes of text message */
    Pclchar     Msg[SysMaxAnsiMsg];  /* Message text                    */
} PclFlagType;

typedef struct PclFlaggerType
{
    PclFlavor                  Flavor;
    PclLength                  Length;
    PclFlagType                Flags[1];
} PclFlaggerType;

typedef struct CliFlaggerType
{
    PclFlagType                Flags[1];
} CliFlaggerType;

/*========================*/
/* STRUCT CURSORHOSTTYPE  */
/*========================*/

typedef struct PclCursorHostType
{
    PclFlavor                  Flavor;
    PclLength                  Length;
    PclInt16                   AMP;
    PclByte                    Row[8];
    PclByte                    Request[4];
} PclCursorHostType;

                               /*  DR61637 <--*/
typedef struct PclAltCursorHostType
{
    PclFlavor                  Flavor;
    PclLength                  Length;
    PclAltLength               AltLength;
    PclInt16                   AMP;
    PclByte                    Row[8];
    PclByte                    Request[4];
} PclAltCursorHostType;
                               /*--> DR61637  */

typedef struct CliCursorHostType
{
    PclInt16                   AMP;
    PclByte                    Row[8];
    PclByte                    Request[4];
} CliCursorHostType;

/*========================*/
/* STRUCT CURSORDBCTYPE  */
/*========================*/

typedef struct PclCursorDBCType
{
    PclFlavor                  Flavor;
    PclLength                  Length;
    PclInt16                   AMP;
    PclByte                    Row[8];
} PclCursorDBCType;

typedef struct CliCursorDBCType
{
    PclInt16                   AMP;
    PclByte                    Row[8];
} CliCursorDBCType;

/**********************/
/* End OF NDM Parcels */
/**********************/


/*******************************************************************/
/*  The following defines the 2PC Parcels - JAE 23Nov92            */
/*******************************************************************/

/*==================*/
/* STRUCT SESSOPT   */
/*==================*/

struct PclSessOptListType        /* HN_00 - NDM Contract Support   */
                                 /* Added ANSI_Tran and FIPS_Flag. */
{
    Pclchar            ANSI_Tran;
    Pclchar            TPCOption;
    Pclchar            FIPS_Flag;
    Pclchar            date_form;    /* DR40417 */
    Pclchar            ESS_flag;     /* DR58352 */
    Pclchar            utilityWorkload; /* DR126915 */
    Pclchar            Redrive;      /* DR145358, CLAC-29119 */
    Pclchar            extendedLoadUsage; /* CLAC-29283 */
    Pclchar            RFU_9;
    Pclchar            RFU_10;
};


struct PclSessOptType
{
    PclFlavor                   Flavor;
    PclLength                   Length;
    struct   PclSessOptListType List;
};

/**********************/
/* STRUCT VOTEREQ     */
/**********************/

typedef char PclIDType[SysMaxName];

struct PclForeignIDType
{
    PclInt16        Length;
    PclIDType       Name;
};

struct PclVoteReqType
{
    PclFlavor                  PclKind;
    PclLength                  Length;
    struct   PclForeignIDType  Coordinator;
    struct   PclForeignIDType  RunUnitID;
};

/**********************/
/* STRUCT VOTETERM    */
/**********************/

struct PclVoteTermType
{
    PclFlavor                  PclKind;
    PclLength                  Length;
};

/**********************/
/* STRUCT CMMT2PC     */
/**********************/

struct PclCmmt2PCType
{
    PclFlavor                  PclKind;
    PclLength                  Length;
};

/**********************/
/* STRUCT ABT2PC      */
/**********************/

struct PclAbrt2PCType
{
    PclFlavor                  PclKind;
    PclLength                  Length;
};

/*  END OF 2PC PARCELS */

/*=============================================================*/
/* 87Sep18 DAA DCR3780 we did not get this in the correct      */
/* version the first time, so let's try it again with feeling..*/
/* The following definitions are required to support the new   */
/*  options parcel and connect parcel, and the LSN functions   */
/*=============================================================*/


struct PclOptListType
{
    Pclchar            RequestMode;
    Pclchar            DBCFunction;
    Pclchar            SelectData;  /*DR57583*/
    Pclchar            CCS; /* DR69228 */
    Pclchar            APH_responses; /* DR68835 */
    Pclchar            ReturnStatementInfo; /* DR98026  */
    Pclchar            transformsOff;       /* DR123516 */
    Pclchar            MaxDecimalPrecision; /* DR98024  */
    Pclchar            AGKR_option;         /* DR98025  */
    Pclchar            DynamicResultSets;   /* DR102528 */
    PclByte            SPReturnResult;      /* DR102528 */
    Pclchar            periodAsStruct;      /* DR123516 */
    PclByte            columnInfo;          /* DR116354 */
    Pclchar            TrustedSessions;     /* DR116354 */
    Pclchar            multiStatementErrors;/* DR121202 */
    Pclchar            arrayTransformsOff;  /* DR145224 */
    Pclchar            xmlResponseFormat;   /* CLAC-28875 */
    Pclchar            tasmFastFailReq;     /* CLAC-29270, 30478 */
    Pclchar            Reserved1;           /* CLAC-29115 */
    Pclchar            Reserved2;           /* CLAC-29115 */
    Pclchar            largeRowOpt;         /* CLAC-29115 */
};


struct PclOptionsType
{
    PclFlavor                 Flavor;
    PclLength                 Length;
    struct     PclOptListType OptList;
};
                            /*--> DR61637 */
struct PclAltOptionsType
{
    PclFlavor                  Flavor;
    PclLength                  Length;
    PclAltLength               AltLength;
    struct     PclOptListType  OptList;
};
                            /* DR61637 <--*/


/***************************************************************/
/*                                                             */
/*  PrepInfo parcel definition                                 */
/*                                                             */
/***************************************************************/

struct CliPrepColInfoType
{
    PclInt16       DataType;
    PclUInt16      DataLen;   /* DR101116 */
    /* CLAC-29672 -->
    PclInt16       NameLen;           ****************************************
    Pclchar        Name[NameLen];     * Name, Format and Title have variable *
    PclInt16       FormatLen;         * lengths and are listed for reference *
    Pclchar        Format[FormatLen]; * only.                                *
    PclInt16       TitleLen;          ****************************************
    Pclchar        Title[TitleLen];
    <-- CLAC-29672 */
};

/* DR66590 ---> */
struct CliPrepColInfoTypeX
{
    PclInt16       DataType;
    PclInt64       DataLen;
    /* DR89769 --> */
/*  PclInt16       DataExtType;     */
    Pclchar        ColumnCharSetCode;
    Pclchar        ColumnExtInfo;
    /* <-- DR89769 */
    /* CLAC-29672 -->
    PclInt16       NameLen;           ****************************************
    Pclchar        Name[NameLen];     * Name, Format and Title have variable *
    PclInt16       FormatLen;         * lengths and are listed for reference *
    Pclchar        Format[FormatLen]; * only.                                *
    PclInt16       TitleLen;          ****************************************
    Pclchar        Title[TitleLen];
    <-- CLAC-29672 */
};
/* <--- DR66590 */


struct CliPrepInfoType
{
    Pclchar        CostEstimate[8]; /* CostEstimate looks like a */
                                    /* 8 byte float value type?? */
                                    /* How can this be since     */
                                    /* unix floats are 4 bytes?? */
                                    /* We will need to look into */
                                    /* this : This field is RFU  */
                                    /* so the type might change  */
    PclInt16       SummaryCount;
    PclInt16       ColumnCount ;

    /* The following field is commented out to show it has variable length.  */
    /* The size can be greater than or equal to 0 up to maximum parcel size. */

    /* Pclchar     ColInfo[N];                                               */
};

/* CLAC-29367 ->

      +------------------------+
      | Flavor                 |
      +------------------------+
      | Length                 |
      +------------------------+
      | CostEstimate           |
      +------------------------+
      | SummaryCount           |
      +--+---------------------+
         | ColumnCount         |         <------------+
         +--+------------------+                      |
            | DataType         |   <--+               | SummaryCount+1
            +------------------+      | ColumnCount   | repetitions
            | DataLen          |      | repetitions   |
            +------------------+      | (may be 0)    |
            | ColumnName       |      |               |
            +------------------+      |               |
            | ColumnFormat     |      |               |
            +------------------+      |               |
            | ColumnTitle      |   <--+  <------------+
            +------------------+

      This figure shows PrepInfo parcel structure.  More information about the
      PrepInfo and PrepInfoX can be found in "Chapter 9: Parcels" in "Teradata
      Call-Level Interface Reference for Network-Attached Systems".
      Note: The first two byte of ColumnName, ColumnFormat, and ColumnTitle
            are used for size in bytes of each data.  The following is an
            example in which the first two byte is 4.

                       0       2   3   4   5
                      +-------+---+---+---+---+
                      |   4   |'c'|'i'|'t'|'y'|
                      +-------+---+---+---+---+

<- CLAC-29367 */

struct PclCONNECTType
{
    PclFlavor       Flavor;
    PclLength       Length;
    Pclchar         PartitionName[16];
    PclInt32        LSN;
    PclInt16        Function;
    PclInt16        Pad2bytes;        /* to make 3b2 happy */
};


struct CliCONNECTType
{
    Pclchar            PartitionName[16];
    PclInt32           LSN;
    PclInt16           Function;
    PclInt16           Pad2bytes;        /* to make 3b2 happy */
};


#define  ASSOCLSN  2
#define  ALLOCLSN  1
#define  NOPLSN    0

struct PclLSNType
{
    PclFlavor       Flavor;
    PclLength       Length;
    PclInt32        LSN;
};


struct CliLSNType
{
    PclInt32           LSN;
};


struct  CliDInfoType   /* Internal part of the DataInfo parcel */
{
    PclWord            SQLType;
    PclWord            SQLLen;
};


struct  CliDataInfoType  /* To define a DataInfo parcel       */
{
    PclWord              FieldCount;
    struct CliDInfoType  InfoVar[300];
};


struct  CliEndStateType  /* To define the End Statement parcel*/
{
    PclWord            StatementNo;
};


struct  CliEndWithType   /* To define the EndWith parcel      */
{
    PclWord            WithId;
};


struct  CliFailureType   /* To define the Failure parcel       */
{
    PclWord            StatementNo;
    PclWord            Info;
    PclWord            Code;
    PclWord            Length;
    Pclchar            Msg[255];                /* 1 to 255 bytes  */
};


struct  CliErrorType   /* To define the Error parcel           */
{
    PclWord            StatementNo;
    PclInt16           Info;
    PclWord            Code;
    PclWord            Length;
    Pclchar            Msg[SysMaxErrMsg];
};


struct  CliFieldType   /* To define the Field parcel           */
{
    /* CLAC-34171 --> */
    #ifdef CLIV2_STRUCT_DEF_NO_DEPRECATE
    Pclchar            Data[SysMaxParcelSize];
    #else
    Pclchar            Data[1];
    #endif
    /* <--CLAC-34171 */
};


struct  CliFMReqType   /* DBC/SQL Field Mode Request parcel    */
{
    /* CLAC-34171 --> */
    #ifdef CLIV2_STRUCT_DEF_NO_DEPRECATE
    Pclchar            ReqText[SysMaxParcelSize];
    #else
    Pclchar            ReqText[1];
    #endif
    /* <--CLAC-34171 */
};


struct  CliHostStartType   /* To define the HostStartup Parcel */
{
    PclWord            StartType;
    PclWord            Scope;
};


struct  CliIndicDataType   /* To define Indicator Data parcel  */
{
    /* CLAC-34171 --> */
    #ifdef CLIV2_STRUCT_DEF_NO_DEPRECATE
    Pclchar            Data[SysMaxParcelSize];
    #else
    Pclchar            Data[1];
    #endif
    /* <--CLAC-34171 */
};


struct  CliIndicReqType    /* To define the DBC/SQL Indicator  */
{                          /* record Mode Request parcel       */

    /* CLAC-34171 --> */
    #ifdef CLIV2_STRUCT_DEF_NO_DEPRECATE
    Pclchar            ReqText[SysMaxRequestSize];
    #else
    Pclchar            ReqText[1];
    #endif
    /* <--CLAC-34171 */
};


struct  CliKeepRespType    /* To define the KeepRespond parcel */
{
    PclWord            MaxMsgSize;
};


struct  CliNullValueType   /* To define a Null Value parcel    */
{
    PclWord            FieldNum;
};


struct  CliOkType          /* To define the Ok parcel          */
{
    PclWord            StatementNo;
    PclWord            FieldCount;
    PclUInt32          ActivityCount; /* DR42286, unsigned         */
    PclWord            ActivityType;
    PclWord            WarningCode;
    PclWord            WarningLength;
    Pclchar            WarningMsg[SysMaxErrMsg];
};


struct  CliPositionType   /* To define the Position parcel     */
{
    PclWord            ColumnNo;
};


struct  CliPromptType     /* To define the Prompt parcel       */
{
    PclWord            NumFields;
};


struct  CliRecordType     /* To define the Record parcel       */
{
    /* CLAC-34171 --> */
    #ifdef CLIV2_STRUCT_DEF_NO_DEPRECATE
    Pclchar            Body[SysMaxParcelSize];
    #else
    Pclchar            Body[1];
    #endif
    /* <--CLAC-34171 */
};


struct  CliReqType        /* To define a DBC/SQL Record Mode   */
{                         /* Request parcel                    */

    /* CLAC-34171 --> */
    #ifdef CLIV2_STRUCT_DEF_NO_DEPRECATE
    Pclchar            ReqText[SysMaxRequestSize];
    #else
    Pclchar            ReqText[1];
    #endif
    /* <--CLAC-34171 */
};


struct  CliRespType       /* To define the Respond parcel      */
{

    PclWord            MaxMsgSize;
};


struct  CliSizeType       /* To define the Size parcel         */
{
    PclWord            MaxFldSize;
};

struct  CliSessOptListType
{
    Pclchar            AbortLevel;
    Pclchar            Mode2PC;
    Pclchar            RFU[8];
};

struct  CliSuccessType    /* To define the Success parcel       */
{
    PclWord            StatementNo;
/* DR88784 --> */
#if (defined(I370) && _O_BYTEALIGN)  /* DR107934 */
    PclUInt32          ActivityCount;
#else
    Pclchar            ActivityCount[4]; /* should be 4 byte Int   */
                                         /* however alignment prob */
#endif
/* <-- DR88784 */
    PclWord            WarningCode;
    PclWord            FieldCount ;
    PclWord            ActivityType;
    PclWord            WarningLength;
    Pclchar            WarningMsg[SysMaxErrMsg];
};

/* DR58352 --> */

struct  CliResultSummaryType    /* To define the Result Summary parcel       */
{
    PclWord       	   StatementNo;
/* DR88784 --> */
#if (defined(I370) && _O_BYTEALIGN)  /* DR107934 */
    PclUInt64          ActivityCount;
#else
    Pclchar            ActivityCount[8]; /* should be 8 byte Int   */
                                         /* however alignment prob */
#endif
/* <-- DR88784 */
    PclWord            FieldCount;
    PclWord            ActivityType;
    Pclchar            Mode; /* R=Record, F=Field, I=Indicator, M=Multipart */
    Pclchar            reserved[9];	/* for future use */
};

/* <-- DR58352 */

struct  CliValueType    /* To define the Value parcel           */
{
    PclWord            FieldNum;
    /* CLAC-34171 --> */
    #ifdef CLIV2_STRUCT_DEF_NO_DEPRECATE
    Pclchar            Data[SysMaxParcelSize-2];
    #else
    Pclchar            Data[1];
    #endif
    /* <--CLAC-34171 */
};


struct  CliWithType     /* To define the With parcel             */
{
    PclWord            WithId;
};


struct  ConnectBody
{
    Pclchar            PartName[16];
    PclInt32           LSN;
    PclInt16           Func;
    PclInt16           Pad2byte;    /* to make 3b2 happy */
};


/*===========================================================*/
/* 87Nov03 DAA DCR3273 Error Message Inserts support.  The   */
/*  following structures/definitions have been added for     */
/*  support of error message inserts                         */
/*===========================================================*/

/*===========================================================*/
/* The InsTriad structure will appear at the end of error    */
/* and failure parcels following the InsCount field, which   */
/* indicates how many triads follow the message text         */
/*===========================================================*/

struct InsTriad
{
    Pclchar            Kind;   /* Kind is the type of insert   */
                               /* valid types are defined below*/
    Pclchar            Offset; /* Offset is the offset from the*/
                               /* beginning of the message text*/
                               /* where the inserted text begin*/
    Pclchar            Size;   /* Size is the size of the      */
                               /* inserted text.               */
};


#define  PclErrIns_VSTR   0
#define  PclErrIns_FSTR   1
#define  PclErrIns_DBID   2
#define  PclErrIns_TVMID  3
#define  PclErrIns_FLDID  4

/*=============================================================*/
/*             Following added for DCR4029  -  WBJ             */
/*=============================================================*/

struct PclConfigType
{
    PclFlavor       Flavor;
    PclLength       Length;
};

/*=============================================================*/
/*  The following structures are nested within the variable    */
/*  appendages hung on the config response parcel. A word      */
/*  about their organization is in order.                      */
/*  The Config Response Parcel will look like:                 */
/*       [ Fixed Portion - thru ProcCount                   ]  */
/*       [ The number of IFP records specified by ProcCount ]  */
/*       [ A 16 bit value indicating the number of AMP recs ]  */
/*       [ The appropriate number of AMP records (AMPCount) ]  */
/*       [ 16 bits count of entries in char set             ]  */
/*       [ The set of character code to name map entries    ]  */
/*       [ InDoubt field indicating if InDoubt sessions exist] */
/*       [ 1 byte value indicating if extension area follows]  */
/*       [ Extension area (Features, Length, Tx_Semantic Def.] */
/*=============================================================*/

/*==================*/
/*  The IFP Record  */
/*==================*/

struct IFPrec
{
    PclInt16   IFPPhyProcNo;
    PclByte    IFPState;
    PclByte    Pad;
};

/*==================*/
/*  The AMP Record  */
/*==================*/

typedef   PclInt16   AMPArray;

/*==================================*/
/*  The Character Code to Name Map  */
/*==================================*/

#define FLTMAXTRANSLATENAME 30  /* length of char set names */

struct FltCharArrayNameCodeType
{
    PclByte    CharSetCode;
    PclByte    Pad;
    PclByte    CharSetName[FLTMAXTRANSLATENAME];
};

/*==================================*/
/*  In Doubt field of ConfigRsp     */
/*==================================*/

typedef   Pclchar    InDoubt;

/*==================================*/
/*  Extension Area of ConfigRsp     */
/*==================================*/

struct  PclCfgExtendType
{
    PclInt16           Feature;
    PclInt16           Length;
};

#define PCLCfgANSI 1          /* ANSI_Feature - 1 indicates NDM release */

struct  PclCfgNDMFeatureType
{
    PclInt16           ANSI_Feature;  /* Set to PCLCfgANSI for NDM release */
    PclInt16           ANSI_Length;
    Pclchar            ANSI_Tran;     /* 'A' ANSI, 'T' Teradata */
};

/*==============================================*/
/*  Finally  -  The config response parcel!!    */
/*==============================================*/

struct PclConfigRspType
{
    PclFlavor       Flavor;
    PclLength       Length;
    PclInt32        FirstSessNo;
    PclInt32        FirstReqNo;
    PclInt16        MaxSessCnt;
    PclInt16        ProcCount;  /* count of IFPs              */
    Pclchar         VarPart[1]; /* start of variable portion  */

     /*=======================================================*/
     /* Variable Part is organized as follows :               */
     /* [ The number of IFP records specified by ProcCount ]  */
     /* [ A 16 bit value indicating the number of AMP recs ]  */
     /* [ The specified number of AMP records (AMPCount)   ]  */
     /* [ 16 bits count of entries in char set             ]  */
     /* [ The set of character code to name map entries    ]  */
     /*                                                       */
     /* The types for the IFP records, AMP records and Char   */
     /* codes are defined above.                              */
     /*                                                       */
     /*=======================================================*/
     /* NDM Changes:                                          */
     /* For the NDM Query Environment Facility, this parcel   */
     /* is returned as a response to the LAN Request.  There  */
     /* is an extension to this parcel that contains other    */
     /* information following the character set entries.      */
     /* [ 1 byte "IN DOUBT" field.                         ]  */
     /* [ 1 byte indicating if more data follows this byte.]  */
     /* [ 2 byte Feature field (1 - NDM features exist). ]    */
     /* [ 2 byte length field.                             ]  */
     /* [ 1 byte Transaction Semantics default ('A' or 'T')]  */
     /*                                                       */
};

/*============================================================*/
/*                    End DCR4029 mods                        */
/*============================================================*/
/*#ifdef WIN32 */
/* SSO: start DR 52052 */
/*   Purpose  To define the Authorization Request Parcel */

typedef struct
{
    PclFlavor 	    PclKind;    /* PCLSSOAUTHREQ */
    PclLength_t 	PclLength;
} pclssoauthreq_t;

/*  Fields  PclKind is PCLSSOAUTHREQ.

            PclLength is the length of the parcel.

    MsgType Request.

    Notes   The PclSSOAuthReq parcel is used to request which
            authentication methods are available.
*/

/*   Purpose  To define the Authorization Response Parcel */

typedef struct
{
    PclFlavor  	    PclKind;    /* PCLSSOAUTHRESP */
    PclLength_t     PclLength;  /* DR68080: PclLength_t used */
    PclByte         NumberOfMethods;
    unsigned char   Methods[255];
} pclssoauthrsp_t;

/*  Fields  PclKind is PCLSSOAUTHRESP.

            PclLength is the length of the parcel.

            NumberOfMethods is the number of methods returned
            as an array or character Methods

            Methods is an array of supported methods for this platform.

    MsgType Request.

    Notes   The PclSSOAuthReq parcel is used to request which
            authentication methods are available.
*/

/*   Purpose  To define the SSOREQ Parcel */

typedef struct
{
    PclFlavor 	    PclKind;    /* PclSSOREQ */
    PclLength_t     PclLength;  /* DR68080: PclLength_t used */
    PclByte         Method;
    PclByte         Trip;
    PclUInt16       AuthDataLen;
    Pclchar         AuthData[1];  /* DR54273 */
} pclssoreq_t;

#define SIZEOF_PCLSSOREQ_T 8 /* DR52869: unix rounds up char[1] to a sizeof 2
                                causing a problem with parcel data size */
/*  Fields  PclKind is PclSSOREQ.

            PclLength is the length of the parcel.

            Method is the method to use for authentication which
            must be one of the values in PclSSOMethods_t and must
            be one of the values returned in the AuthMethods parcel.

            Trip is incremented each time a COPSSOREQ message is sent
            in either direction.

            AuthDataLen defines the length of the authentication
            data.

            AuthData represents the opaque authentication data
*/

/*   Purpose  To define the SSOREQ Parcel */

typedef struct
{
    PclFlavor 	    PclKind;    /* PclSSORESP */
    PclLength_t 	PclLength; /* DR68080: PclLength_t used */
    PclByte         Method;
    PclByte         Code;
    PclByte         Trip;
    PclByte         MustBeZero;
    PclUInt16       AuthDataLen;
    Pclchar         AuthData[1];	/* DR54273 */
} pclssorsp_t;

/*  Fields  PclKind is PclSSORESP.

            PclLength is the length of the parcel.

            Method is the method to use for authentication which
            must be one of the values in PclSSOMethods_t and must
            be one of the values returned in the AuthMethods parcel.

            Code is one of the two values SSOLOGONOK or
            SSOLOGONCONTINUED.  SSOLOGONFAILURE is used internally
            by the Gateway

            Trip is incremented each time a COPSSOREQ message is sent
            in either direction.

            AuthDataLen defines the length of the authentication
            data.

            AuthData represents the opaque authentication data
*/

/*   Purpose  To define the SSOREQ Parcel */

typedef struct
{
    PclFlavor 	    PclKind;    /* PCLUSERNAMEREQ */
    PclLength_t 	PclLength;  /* DR68080: PclLength_t used */
} pclusernamereq_t;

/* DR116354 --> */
typedef struct
{
    PclFlavor       PclKind;    /* PCLUSERNAMEREQ */
    PclLength_t     PclLength;  /* Parcel length  */
    PclByte         EONResponse;
} pclusernamereqEON_t;
/* <-- DR116354 */

/*  Fields  PclKind is PCLUSERNAMEREQ. */

/*  Purpose     To define an enumeration value for each authentication
                method that is supported by Teradata Software
*/

#ifdef CLI_MF_GTW          /* CLIWS-7751 */
#pragma enum(4)
#endif  /* CLI_MF_GTW */

typedef enum
{
    AuthNotSpecified = 0,      /* DR68842: Used for 6.0 tdgss */
    /* AuthWinNegotiate = 1, *//* DR68842: no longer offered */
    AuthWinNTLM      = 2,
    AuthWinKerberos  = 3,
    AuthTeradata     = 8       /* DR52869 */
} PclSSOMethods_t;

#ifdef CLI_MF_GTW          /* CLIWS-7751 */
#pragma enum(reset)
#endif  /* CLI_MF_GTW */


/*  The following defines are used for translating the
//  Package Name returned by the security system to the
//  enumerated methods defined above.
*/

#define AUTHNOTSPECIFIEDSTR ""
#define AUTHWINNEGOTIATESTR "Negotiate"  /* DR68842: backwards compatibility? */
#define AUTHWINNTLMSTR      "NTLM"
#define AUTHWINKERBEROSSTR  "Kerberos"
#define AUTHTERADATASTR     "TERADATA"   /* DR52869-dhv-01 */

/* SSO: end DR 52052 */
/*#endif*/


/*============================================================*/
/*            The following was added for DR68842             */
/*============================================================*/

/* Purpose  To define the type of gateway configuration response
            parcel extension data present in gateway configuration
            response parcel.

   Fields   pclgtwextnone means there is no configuration response
            information present in the pclgtwconfigrsp_t parcel.
            For pre-V2R6 gateway this is always the definition.

            pclgtwextselfdef means that the pclgtwconfigrsp_t parcel
            contains self-defining structures. The structures always
            appear after the pclgtwconfigextend_t in the parcel. The
            self-defining structures have the following general
            format:

               Feature  :  pclgtwconfigfeature_t;
               Length   :  pclgtwconfigfeaturelen_t;
               ** Misc data fields for feature
*/

/*  Purpose     To define the self-defining gateway feature number */
/*              As defined for DBS configuration parcel.           */
typedef unsigned short pclgtwconfigfeature_t, pclcliconfigfeature_t;

/* Purpose    To define the length of the self-defining feature of a */
/*            gateway configuration response parcel.                 */
typedef unsigned short pclgtwconfigfeaturelen_t, pclcliconfigfeaturelen_t;

#ifdef CLI_MF_GTW          /* CLIWS-7751 */
#pragma enum(4)
#endif  /* CLI_MF_GTW */

typedef enum
{
    pclgtwextnone = 0,
    pclgtwextselfdef = 1
} pclgtwconfigextend_t, pclcliconfigextend_t;

#ifdef CLI_MF_GTW          /* CLIWS-7751 */
#pragma enum(reset)
#endif  /* CLI_MF_GTW */

/* pclclientconfig_t and pclgtwconfig_t are the same structures. We use dif- */
/* ferent variable names for config extensions for sanity purposes and ease  */
/* of debugging. */
typedef struct
{
    PclFlavor             PclKind;        /* PclCLIENTCONFIG */
    PclLength_t           PclLength;
    pclcliconfigextend_t  ClientConfigExt;
} pclclientconfig_t;

typedef struct
{
    PclFlavor             PclKind;        /* PclGTWCONFIG */
    PclLength_t           PclLength;
    pclgtwconfigextend_t  GtwConfigExt;
} pclgtwconfig_t;

/* Gateway self-defining element (SDE) values */
#define PCLGTWCONFIGSSO                 1 /* SSO */
#define PCLGTWCONFIGTDGSS               2 /* GSS */
#define PCLGTWCONFIGUTF                 3 /* DR94976: Unicode */
#define PCLGTWCONFIGNODEID              4 /* DR113468: Session ID */
#define PCLGTWCONFIGRECOVERABLEPROTOCOL 5 /* DR145358, CLAC-29119 -> */
#define PCLGTWCONFIGCONTROLDATA         6
#define PCLGTWCONFIGREDRIVE             7 /* <- DR145358, CLAC-29119 */
#define PCLGTWCONFIGSECURITYPOLICY      10 /* CLAC-29222 */
#define PCLGTWCONFIGNEGOMECH            12 /* CLAC-31283 */
#define PCLGTWCONFIGIDP                 15 /* CLIWS-7948 */
#define PCLGTWCONFIGPORTS               16 /* CLIWS-7780 */
#define PCLGTWCONFIGIDPSCOPE            17 /* CLIWS-8666 */

/* Client self-defining element values */
#define PCLCLICONFIGVERSION 1 /* DR101074: client config version */
#define PCLCLICONFIGTDGSS   2 /* DR96966: GSS. featurelen for this feature */
                              /* should be 0 (due to original 4013 error   */
                              /* (and JDBC).                               */
#define PCLCLICONFIGRECOVERABLEPROTOCOL 3 /* DR145358, CLAC-29119 -> */
#define PCLCLICONFIGCONTROLDATA         4
#define PCLCLICONFIGREDRIVE             5 /* <- DR145358, CLAC-29119 */
#define PCLCLICONFIGSECURITYPOLICY      8 /* CLAC-29222 */
#define PCLCLICONFIGESS                 9 /* CLAC-28857: ESS */
#define PCLCLICONFIGNEGOMECH           11 /* CLAC-31283 */
#define PCLCLICONFIGIDP                14 /* CLIWS-7948 */
#define PCLCLICONFIGTLS                15 /* CLIWS-7647 */
#define PCLCLICONFIGIDPSCOPE           16 /* CLIWS-8666 */

#define CLICONFIGVERSIONLEN 12
#define CLICONFIGTDGSSLEN   4

typedef struct cli_config_ext_version_s /* DR101074 */
{
    pclcliconfigfeature_t    feature;   /* PCLCLICONFIGVERSION */
    pclcliconfigfeaturelen_t length;    /* CLICONFIGVERSIONLEN */
    unsigned char            value[CLICONFIGVERSIONLEN];
} cli_config_ext_version_t;

typedef struct cli_config_ext_tdgss_s   /* DR96966, DR101074 */
{
    pclcliconfigfeature_t    feature;   /* PCLCLICONFIGTDGSS */
    pclcliconfigfeaturelen_t length;    /* CLICONFIGTDGSSLEN */
    unsigned char            value[CLICONFIGTDGSSLEN];
} cli_config_ext_tdgss_t;

/* DR145358, CLAC-29119 -> */
typedef struct cli_config_ext_recoverableprotocol_s
{
    pclcliconfigfeature_t    feature;   /* PCLCLICONFIGRECOVERABLEPROTOCOL */
    pclcliconfigfeaturelen_t length;    /* 0 */
} cli_config_ext_recoverableprotocol_t;

typedef struct cli_config_ext_controldata_s
{
    pclcliconfigfeature_t    feature;   /* PCLCLICONFIGCONTROLDATA */
    pclcliconfigfeaturelen_t length;    /* 0 */
} cli_config_ext_controldata_t;

typedef struct cli_config_ext_redrive_s
{
    pclcliconfigfeature_t    feature;   /* PCLCLICONFIGREDRIVE */
    pclcliconfigfeaturelen_t length;    /* 0 */
} cli_config_ext_redrive_t;
/* <- DR145358, CLAC-29119 */

typedef struct cli_config_ext_ess_s       /* CLAC-28857 */
{
    pclcliconfigfeature_t    ESS_Feature; /* PCLCLICONFIGESS */
    pclcliconfigfeaturelen_t ESS_Length;  /* 1 */
    unsigned char            ESS_flag;    /* 0 */
} cli_config_ext_ess_t;

typedef struct cli_config_ext_securitypolicy_s /* CLAC-29222 */
{
    pclcliconfigfeature_t    feature; /* PCLCLICONFIGSECURITYPOLICY */
    pclcliconfigfeaturelen_t length;  /* 1 */
    unsigned char            level;   /* 1 */
} cli_config_ext_securitypolicy_t;

typedef struct cli_config_ext_negomech_s /* CLAC-31283 */
{
    pclcliconfigfeature_t    feature; /* PCLCLICONFIGNEGOMECH */
    pclcliconfigfeaturelen_t length;  /* 1 */
    unsigned char            level;   /* 1 */
} cli_config_ext_negomech_t;

typedef struct cli_config_ext_tls_s   /* CLIWS-7647 */
{
    pclcliconfigfeature_t    feature; /* PCLCLICONFIGTLS */
    pclcliconfigfeaturelen_t length;  /* 0 */
} cli_config_ext_tls_t;

typedef struct cli_config_ext_idp_s   /* CLIWS-7948 */
{
    pclcliconfigfeature_t    feature; /* PCLCLICONFIGIDP */
    pclcliconfigfeaturelen_t length;  /* 0 */
} cli_config_ext_idp_t;


/* Purpose    To define the legal values for the SSO_Setting field of */
/*            pclgtwconfig parcel.                                    */
#define       SSO_ON    0x01
#define       SSO_OFF   0x02
#define       SSO_ONLY  0x03

/* Fields     SSO_Feature must be set to PCLGTWCONFIGSSO.*/
/*            SSO_Length must be set to 5.               */
/*            SSO_Setting is defined above.              */
typedef struct
{
    pclgtwconfigfeature_t    SSO_Feature;
    pclgtwconfigfeaturelen_t SSO_Length;
    unsigned char            SSO_Setting;
} pclgtwconfigsso_t;

/* Fields     TDGSS_Feature (PCLGTWCONFIGTDGSS, PCLGTWCONFIGUTF)          */
/*            TDGSS_Length                                                */
typedef struct
{
    pclgtwconfigfeature_t    TDGSS_Feature;
    pclgtwconfigfeaturelen_t TDGSS_Length;
}  pclgtwconfigtdgss_t;

/* DR113468 --> */
/* Fields     UID_Feature must be set to PCLGTWCONFIGNODEID */
/*            UID_Length must be set to 6.                  */
/*            UID_Setting is defined above.                 */
typedef struct
{
    pclgtwconfigfeature_t    UID_Feature;
    pclgtwconfigfeaturelen_t UID_Length;
    unsigned short           UID_Setting;
} pclgtwconfiguid_t;
/* <-- DR113468 */

/* DR145358, CLAC-29119 -> */
typedef struct
{
    pclgtwconfigfeature_t    RECOVERABLEPROTOCOL_Feature;
                                          /* PCLGTWCONFIGRECOVERABLEPROTOCOL */
    pclgtwconfigfeaturelen_t RECOVERABLEPROTOCOL_Length;  /* 4 */
} pclgtwconfigrecoverableprotocol_t;

typedef struct
{
    pclgtwconfigfeature_t    CONTROLDATA_Feature; /* PCLGTWCONFIGCONTROLDATA */
    pclgtwconfigfeaturelen_t CONTROLDATA_Length;  /* 4 */
} pclgtwconfigcontroldata_t;

typedef struct
{
    pclgtwconfigfeature_t    REDRIVE_Feature; /* PCLGTWCONFIGREDRIVE */
    pclgtwconfigfeaturelen_t REDRIVE_Length;  /* 4 */
} pclgtwconfigredrive_t;
/* <- DR145358, CLAC-29119 */

/* CLAC-29222 -> */
typedef struct
{
    pclgtwconfigfeature_t     SECURITYPOLICY_Feature;
    /* PCLGTWCONFIGSECURITYPOLICY */
    pclgtwconfigfeaturelen_t  SECURITYPOLICY_Length;  /* 5 */
    unsigned char             SECURITYPOLICY_Level;   /* 1 */
} pclgtwconfigsecuritypolicy_t;

/* CLIWS-7780: Gateway port configuration */
typedef struct
{
    pclgtwconfigfeature_t     PORTS_Feature;     /* PCLGTWCONFIGPORTS */
    pclgtwconfigfeaturelen_t  PORTS_Length;      /* 6 */
    unsigned char             PORTS_Legacy;      /* 0 - disabled or 1 - enabled */
    unsigned char             PORTS_TLS;         /* 0 - disabled or 1 - enabled */
} pclgtwconfigports_t;

/* CLIWS-7948: Gateway IdP configuration */
typedef struct
{
    pclgtwconfigfeature_t     IDP_Feature;            /* PCLGTWCONFIGIDP */
    pclgtwconfigfeaturelen_t  IDP_Length;             /* 4 + 2 + 2 + IDP_ClientID_Length + IDP_MetadataURL_Length */
    unsigned short            IDP_ClientID_Length;
    char*                     IDP_ClientID;           /* ClientID */
} pclgtwconfigidp_t;

/* CLIWS-7948: Gateway IdP metadata URL */
/* Note: pclgtwconfigidpurl_t immediately follows pclgtwconfigidp_t in the parcel */
typedef struct
{
    unsigned short IDP_MetadataURL_Length;
    char*          IDP_MetadataURL;        /* MetadataURL */
} pclgtwconfigidpurl_t;

/* CLIWS-8666: Gateway OIDC scope parameter */
typedef struct
{
    pclgtwconfigfeature_t     IDP_Feature;            /* PCLGTWCONFIGIDPSCOPE */
    pclgtwconfigfeaturelen_t  IDP_Length;             /* 4 + 2 + IDP_Scope_Length */
    unsigned short            IDP_Scope_Length;
    char*                     IDP_Scope;
} pclgtwconfigidpscope_t;

/* Security Policy parcel */
#ifdef CLI_MF_GTW          /* CLIWS-7751 */
#pragma enum(4)
#endif  /* CLI_MF_GTW */

typedef enum
{
    pclsecpolicyextnone    = 0,
    pclsecpolicyextselfdef = 1
} pclsecpolicyextend_t;

#ifdef CLI_MF_GTW          /* CLIWS-7751 */
#pragma enum(reset)
#endif  /* CLI_MF_GTW */

typedef struct
{
    PclFlavor            PclKind; /* PCLSECURITYPOLICY, PCLSECURITYUSED */
    PclLength            Length;
    PclByte              securityRequired;
    PclByte              confidentialityRequired;
    PclByte              pad1[2];
    PclUInt32            securityLevel;
    PclByte              pad2[4];
    pclsecpolicyextend_t SecPolicyExt;
} pclsecuritypolicy_t;
/* <- CLAC-29222 */

/* PclKind       - Must be PCLAUTHMECH     .                                */
/* MechOidLength - Length of the authentication method Oid.                 */
/* MechOid       - Oid of the authentication method. In the structure below */
/*             the MechOid is a string defined as MechOidLength bytes long. */
typedef struct
{
    PclFlavor            PclKind;               /* PCLAUTHMECH */
    PclLength_t          PclLength;
    pclgtwconfigextend_t MechConfigExt;
    unsigned int         MechOidLength;
    unsigned char        MechOid[1];
    /* Followed by config extensions if MechConfigExt == pclgtwextselfdef */
} pclauthmech_t;

/*============================================================*/
/*                    End of DR68842 mods                     */
/*============================================================*/

/* CLAC-31283 --> */
/* Fields     NEGOMECH_Feature must be set to PCLGTWCONFIGNEGOMECH */
/*            NEGOMECH_Length must be set to 5.                    */
/*            NEGOMECH_Level is defined above.                     */
typedef struct
{
    pclgtwconfigfeature_t    NEGOMECH_Feature; /* PCLGTWCONFIGNEGOMECH */
    pclgtwconfigfeaturelen_t NEGOMECH_Length;  /* 5 */
    unsigned char            NEGOMECH_Level;   /* 1 */
} pclgtwconfignegomech_t;
/* <-- CLAC-31283 */

/*  DR 98026 -->  */
/*=============================*/
/* StatementInfo Parcel Types  */
/*=============================*/
typedef struct
{
    PclFlavor                 Flavor;
    PclLength                 Length;
    PclByte                   MetaData;  /* see below */
}
pclstatementinfo_t, PclStatementInfo_t;

typedef struct
{
    PclFlavor                 Flavor;
    PclLength                 Length;
    PclAltLength              AltLength;
    PclByte                   MetaData;  /* see below */
}
pclextstatementinfo_t, PclExtStatementInfo_t;
/* <-- DR 98026  */


/* DR57583 LOBs */

/*  DataInfoX request/response parcel */

struct FieldType
{
    PclUInt16 data_type;       /* DR68105 */
    union
    {
        /* non-numeric */
        Pclchar length_nonnumeric[8];

        /* numeric */
        struct _numeric
        {
            PclUInt32 num_digits;
            PclUInt32 num_fractional;
        } s;
    } u;
};

struct  PclXDIXType
{
    PclFlavor            PclKind;
    PclLength            Length;
    PclUInt32            FieldCount;
    struct FieldType     FieldEntries[1];  /* variable number of entries */
};

/* DR61637 APH version of Datainfox parcel */

struct PclAltXDIXTType
{
    PclFlavor            PclKind;
    PclLength            Length;
    PclAltLength         AltLength;
    PclUInt32            FieldCount;
    struct FieldType     FieldEntries[1];  /* variable number of entries */
};

struct PclSEDType
{
    PclFlavor            PclKind;
    PclLength            Length;
    PclUInt32            token;
};

/* DR85431 --> */

struct CliSEFType
{
    Pclchar              SourceLanguage;
    Pclchar              FileType;
    Pclchar              FileSupplied;
    Pclchar              pad;
    PclUInt16            FileLength;
    Pclchar              FileStart[1];   /* variable size */
};

struct PclSEFType
{
    PclFlavor            PclKind;
    PclLength            Length;
    struct CliSEFType    EFType;
};
/* <-- DR85431 */

/*=============================*/
/* DR60695 Cursor Positioning  */
/*       Parcel Types          */
/*=============================*/


struct PclSetPositionType

{
    PclFlavor       Flavor;          /* 157*/
    PclLength       Length;           /* 4  */
};

struct PclRowPositionType
{
    PclFlavor         Flavor;          /* 158 */
    PclLength         Length;           /*  16 */
    PclUInt16         statmnt_no;
    PclUInt16         dummy;
    PclUInt64         rownum;           /*unsigned 64 bit integer*/

};

struct PclOffsetPositionType
{
    PclFlavor          Flavor;          /* 159 */
    PclLength          Length;           /*  16 */
    PclUInt16          statmnt_no;
    PclUInt16          dummy;
    PclUInt64          offsetaddress;    /*unsigned 64 bit integer*/
};

struct PclAltSetPositionType

{
    PclFlavor       Flavor;       /* 0x8000 + 157	  */
    PclLength       Length;       /* 0    	      */
    PclAltLength    AltLength;    /* 8              */
};

struct PclAltRowPositionType
{
    PclFlavor         Flavor;       /* 0x8000 + 158	  */
    PclLength         Length;       /* 0    	      */
    PclAltLength      AltLength;    /* 20             */
    PclUInt16         statmnt_no;
    PclUInt16         dummy;
    PclUInt64         rownum;           /*unsigned 64 bit integer*/
};

struct PclAltOffsetPositionType
{
    PclFlavor          Flavor;       /* 0x8000 + 159	  */
    PclLength          Length;       /* 0    	      */
    PclAltLength       AltLength;    /* 20             */
    PclUInt16          statmnt_no;
    PclUInt16          dummy;
    PclUInt64          offsetaddress;    /*unsigned 64 bit integer*/
};

/*  <-- DR 68259  */

/* DR 68835 --> */

struct PclErrorInfoType
{
    PclFlavor          Flavor;       /* 164	  */
    PclLength          Length;       /* variable */
/* The following section may repeat as many times as will fit
   based on the parcel length (Length)                        */
    PclUInt16          info_type;
    PclUInt16          info_length;
    Pclchar            error_info[1];  /* variable-length field */
};

struct PclAltErrorInfoType
{
    PclFlavor          Flavor;       /* 164	  */
    PclLength          Length;       /* 0 */
    PclAltLength       AltLength;    /* variable */
/* The following section may repeat as many times as will fit
   based on the parcel length (AltLength)                     */
    PclUInt16          info_type;
    PclUInt16          info_length;
    Pclchar            error_info[1];  /* variable-length field */
};

typedef struct
{
    PclFlavor 	       PclKind;    /* PclUSERNAMERESP */
    PclLength_t        PclLength;
    unsigned char      ActualUserName[1]; /* variable length field */
} pclusernameresp_t; /* DR101287 */

#define PCLBUFFERSIZE 1 /* actual buffer size in case of >64KB message overflow.
        The info, i.e., the actual data is a 4byte unsigned integer value     */

/* <-- DR 68835 */

/* DR98026, Statment Info Feature -> */
/*--------------------------------------------------------------------*
*                                                                     *
*        STATEMENT-INFORMATION RESPONSE,                              *
*                            END-INFORMATION (PARAMETER,              *
*                                             QUERY,                  *
*                                             SUMMARY,                *
*                                             IDENTITY-COLUMN,        *
*                                             STORED-PROCEDURE-OUTPUT,*
*                                             STORED-PROCEDURE-ANSWER,*
*                                             ESTIMATED-PROCESSING)   *
*                                                                     *
*--------------------------------------------------------------------*/

struct TRDSATE
  {
  PclByte pbtieih[6];     /* INFORMATION HEADER                  */
#define PBTIEL 6          /* LENGTH OF INFORMATION               */
  };

#define PBTIEIH                         pbtieih


/*--------------------------------------------------------------------*
*                                                                     *
*        STATEMENT-INFORMATION RESPONSE,                              *
*                            FULL-LAYOUT (PARAMETER,                  *
*                                         QUERY,                      *
*                                         SUMMARY,                    *
*                                         IDENTITY-COLUMN,            *
*                                         STORED-PROCEDURE-OUTPUT,    *
*                                         STORED-PROCEDURE-ANSWER)    *
*                                                                     *
*                            IF PBROFN=P/S/B                          *
*                                                                     *
*--------------------------------------------------------------------*/

struct TRDSATF
  {
  struct TRDSATE info_h;     /* INFORMATION HEADER                   */
  PclUInt16 pbtifdbl;        /* DATABASE-NAME LENGTH                 */
  Pclchar pbtifdb[1];        /* DATABASE-NAME (var-length)           */
  PclUInt16 pbtiftbl;        /* TABLE/PROCEDURE-NAME LENGTH          */
  Pclchar pbtiftb[1];        /* TABLE/PROCEDURE-NAME (var-length)    */
  PclUInt16 pbtifcnl;        /* COLUMN/UDF/UDM/SP-PARM-NAME LENGTH   */
  Pclchar pbtifcn[1];        /* COLUMN/UDF/UDM/SP-PARM-NAME (var-length) */
  PclUInt16 pbtifcp;         /* COLUMN POSITION IN TABLE             */
  PclUInt16 pbtifanl;        /* AS-NAME LENGTH                       */
  Pclchar pbtifan[1];        /* AS-NAME (var-length)                 */
  PclUInt16 pbtiftl;         /* TITLE LENGTH                         */
  Pclchar pbtift[1];         /* TITLE (var-length)                   */
  PclUInt16 pbtiffl;         /* FORMAT LENGTH                        */
  Pclchar pbtiff[1];         /* FORMAT (var-length)                  */
  PclUInt16 pbtifdvl;        /* DEFAULT-VALUE LENGTH                 */
  Pclchar pbtifdv[1];        /* DEFAULT-VALUE (var-length)           */
  Pclchar pbtific;           /* IDENTITY-COLUMN                      */
#define PBTIFICY 'Y'         /* -YES                                 */
#define PBTIFICN 'N'         /* -NO                                  */
#define PBTIFICU 'U'         /* -UNKNOWN                             */
  Pclchar pbtifdw;           /* DEFINITEABLY WRITABLE                */
#define PBTIFDWY 'Y'         /* -YES                                 */
#define PBTIFDWN 'N'         /* -NO                                  */
  Pclchar pbtifnl;           /* NULLABLE                             */
#define PBTIFNLY 'Y'         /* -YES                                 */
#define PBTIFNLN 'N'         /* -NO                                  */
#define PBTIFNLU 'U'         /* -UNKNOWN                             */
  Pclchar pbtifmn;           /* MAY BE NULL                          */
#define PBTIFMNY 'Y'         /* -YES                                 */
#define PBTIFMNN 'N'         /* -NO                                  */
#define PBTIFMNU 'U'         /* -UNKNOWN                             */
  Pclchar pbtifsr;           /* SEARCHABLE                           */
#define PBTIFSRY 'Y'         /* -YES                                 */
#define PBTIFSRN 'N'         /* -NO                                  */
#define PBTIFSRU 'U'         /* -UNKNOWN                             */
  Pclchar pbtifwr;           /* WRITABLE                             */
#define PBTIFWRY 'Y'         /* -YES                                 */
#define PBTIFWRN 'N'         /* -NO                                  */
  PclUInt16 pbtifdt;         /* DATA TYPE                            */
#define PBTIFDTN 0           /* -NOT AVAILABLE                       */
  PclUInt16 pbtifut;         /* UDT INDICATOR                        */
#define PBTIFUTU 0           /* -UNKNOWN                             */
#define PBTIFUTS 1           /* -STRUCTURED UDT                      */
#define PBTIFUTD 2           /* -DISTINCT UDT                        */
#define PBTIFUTI 3           /* -INTERNAL UDT                        */
  PclUInt16 pbtiftyl;        /* TYPE NAME LENGTH                     */
  Pclchar pbtifty[1];        /* TYPE NAME (var-length)               */
  PclUInt16 pbtifmil;        /* DATATYPEMISCINFO LENGTH              */
  Pclchar pbtifmi[1];        /* DATATYPEMISCINFO (var-length)        */
  PclUInt64 pbtifmdl;        /* MAXIMUM DATA LENGTH IN BYTES         */
  PclUInt16 pbtifnd;         /* TOTAL NUMBER OF DIGITS               */
  PclUInt16 pbtifnid;        /* NUMBER OF INTERVAL DIGITS            */
  PclUInt16 pbtifnfd;        /* NUMBER OF FRACTIONAL DIGITS          */
  Pclchar pbtifct;           /* CHARACTER SET TYPE                   */
#define PBTIFCTN 0           /* -NOT CHARACTER DATA                  */
#define PBTIFCTL 1           /* -LATIN                               */
#define PBTIFCTU 2           /* -UNICODE                             */
#define PBTIFCTS 3           /* -KANJISJIS                           */
#define PBTIFCTG 4           /* -GRAPHIC                             */
#define PBTIFCTK 5           /* -KANJI1                              */
  PclUInt64 pbtifmnc;        /* MAXIMUM NUMBER OF CHARACTERS         */
  Pclchar pbtifcs;           /* CASE SENSITIVE                       */
#define PBTIFCSY 'Y'         /* -YES                                 */
#define PBTIFCSN 'N'         /* -NO                                  */
#define PBTIFCSU 'U'         /* -UNKNOWN                             */
  Pclchar pbtifsn;           /* SIGNED                               */
#define PBTIFSNY 'Y'         /* -YES                                 */
#define PBTIFSNN 'N'         /* -NO                                  */
#define PBTIFSNU 'U'         /* -UNKNOWN                             */
  Pclchar pbtifk;            /* KEY                                  */
#define PBTIFKY 'Y'          /* -YES                                 */
#define PBTIFKN 'N'          /* -NO                                  */
#define PBTIFKU 'U'          /* -UNKNOWN                             */
  Pclchar pbtifu;            /* UNIQUE                               */
#define PBTIFUY 'Y'          /* -YES                                 */
#define PBTIFUN 'N'          /* -NO                                  */
#define PBTIFUU 'U'          /* -UNKNOWN                             */
  Pclchar pbtife;            /* EXPRESSION                           */
#define PBTIFEY 'Y'          /* -YES                                 */
#define PBTIFEN 'N'          /* -NO                                  */
#define PBTIFEU 'U'          /* -UNKNOWN                             */
  Pclchar pbtifso;           /* SORTABLE                             */
#define PBTIFSOY 'Y'         /* -YES                                 */
#define PBTIFSON 'N'         /* -NO                                  */
#define PBTIFSOU 'U'         /* -UNKNOWN                             */
/* DR106619 --> */
  Pclchar pbtifpt;           /* PARAMETER TYPE                       */
#define PBTIFPTI 'I'         /* -INPUT                               */
#define PBTIFPTB 'B'         /* -INOUT (BOTH)                        */
#define PBTIFPTO 'O'         /* -OUTPUT                              */
#define PBTIFPTU 'U'         /* -UNKNOWN                             */
/* <-- DR106619 */
  };

#define PBTIFAN                         pbtifan[1]
#define PBTIFANL                        pbtifanl
#define PBTIFCN                         pbtifcn[1]
#define PBTIFCNL                        pbtifcnl
#define PBTIFCP                         pbtifcp
#define PBTIFCS                         pbtifcs
#define PBTIFCT                         pbtifct
#define PBTIFDB                         pbtifdb[1]
#define PBTIFDBL                        pbtifdbl
#define PBTIFDT                         pbtifdt
#define PBTIFDV                         pbtifdv[1]
#define PBTIFDVL                        pbtifdvl
#define PBTIFDW                         pbtifdw
#define PBTIFE                          pbtife
#define PBTIFF                          pbtiff[1]
#define PBTIFFL                         pbtiffl
#define PBTIFIC                         pbtific
#define PBTIFIH                         pbtifih
#define PBTIFK                          pbtifk
#define PBTIFMDL                        pbtifmdl
#define PBTIFMI                         pbtifmi[1]
#define PBTIFMIL                        pbtifmil
#define PBTIFMN                         pbtifmn
#define PBTIFMNC                        pbtifmnc
#define PBTIFND                         pbtifnd
#define PBTIFNFD                        pbtifnfd
#define PBTIFNID                        pbtifnid
#define PBTIFNL                         pbtifnl
#define PBTIFPT                         pbtifpt  /*DR106619*/
#define PBTIFSN                         pbtifsn
#define PBTIFSO                         pbtifso
#define PBTIFSR                         pbtifsr
#define PBTIFT                          pbtift[1]
#define PBTIFTB                         pbtiftb[1]
#define PBTIFTBL                        pbtiftbl
#define PBTIFTL                         pbtiftl
#define PBTIFTY                         pbtifty[1]
#define PBTIFTYL                        pbtiftyl
#define PBTIFU                          pbtifu
#define PBTIFUT                         pbtifut
#define PBTIFWR                         pbtifwr


/*--------------------------------------------------------------------*
*                                                                     *
*        STATEMENT-INFORMATION RESPONSE,                              *
*                            LIMITED-LAYOUT (QUERY,                   *
*                                            SUMMARY,                 *
*                                            IDENTITY-COLUMN,         *
*                                            STORED-PROCEDURE-OUTPUT, *
*                                            STORED-PROCEDURE-ANSWER) *
*                                                                     *
*                            IF PBROFN=E                              *
*                                                                     *
*--------------------------------------------------------------------*/

struct TRDSATL
  {
  struct TRDSATE info_h;         /* INFORMATION HEADER               */
  PclUInt16 pbtildt;             /* DATA TYPE                        */
#define PBTILDTN 0               /* -NOT AVAILABLE                   */
  PclUInt64 pbtilmdl;            /* MAXIMUM DATA LENGTH IN BYTES     */
  PclUInt16 pbtilnd;             /* TOTAL NUMBER OF DIGITS           */
  PclUInt16 pbtilnid;            /* NUMBER OF INTERVAL DIGITS        */
  PclUInt16 pbtilnfd;            /* NUMBER OF FRACTIONAL DIGITS      */

#define PBTILL 22                /* LENGTH OF INFORMATION            */
  };

#define PBTILDT                         pbtildt
#define PBTILIH                         pbtilih
#define PBTILMDL                        pbtilmdl
#define PBTILND                         pbtilnd
#define PBTILNFD                        pbtilnfd
#define PBTILNID                        pbtilnid



/*--------------------------------------------------------------------*
*                                                                     *
*        STATEMENT-INFORMATION RESPONSE,                              *
*                            STATISTIC-LAYOUT (ESTIMATED-PROCESSING)  *
*                                                                     *
*--------------------------------------------------------------------*/

struct TRDSATS
  {
  struct TRDSATE info_h;         /* INFORMATION HEADER               */
  PclUInt64 pbtisee;             /* EXECUTION ESTIMATE (MILLISECS)   */

#define PBTISL 14                /* LENGTH OF INFORMATION            */
  };

#define PBTISEE                         pbtisee
#define PBTISIH                         pbtisih


/*====================================================================*
*                                                                     *
*        STATEMENT-INFORMATION RESPONSE (WHEN RORSI SPECIFIED)        *
*                                                                     *
*====================================================================*/
#define TRDSPFTI 169             /* FLAVOR                           */

struct TRDSPBTI
  {
/*                                                                    */
/*                           INFORMATION HEADER                       */
/*                                                                    */
  PclUInt16 pbtilout;             /* INFORMATION LAYOUT               */
#define PBTILFL 1                 /* -FULL LAYOUT                     */
#define PBTILLL 2                 /* -LIMITED LAYOUT                  */
#define PBTILSL 3                 /* -STATISTIC LAYOUT                */
#define PBTILEI 4                 /* -END INFORMATION                 */
  PclUInt16 pbtiid;               /* INFORMATION ID                   */
#define PBTIIP 1                  /* -PARAMETER                       */
#define PBTIIQ 2                  /* -QUERY                           */
#define PBTIIS 3                  /* -SUMMARY                         */
#define PBTIIIC 4                 /* -IDENTITY-COLUMN                 */
#define PBTIISPO 5                /* -STORED-PROCEDURE-OUTPUT         */
#define PBTIISPR 6                /* -STORED-PROCEDURE-RESULTSET      */
#define PBTIIEP 7                 /* -ESTIMATED-PROCESSING            */
  PclUInt16 pbtilen;              /* LENGTH OF INFORMATION DATA       */

#define PBTIBL 6                  /* LENGTH OF INFO-HEADER            */
  };

#define PBTIID                          pbtiid
#define PBTILEN                         pbtilen
#define PBTILOUT                        pbtilout

/* <- DR98026, Statement Info Feature */

/* DR102528, Dynamic Result Sets --> */
struct PclResultSetBdy
{
    PclUInt64 ResultSet_row;

    PclWord ResultSet_DB_len;
    Pclchar ResultSet_DB_name[1]; /* variable length of this object */
                                  /* specified in preceding field   */
    PclWord ResultSet_SP_len;
    Pclchar ResultSet_SP_name[1]; /* variable length of this object */
                                  /* specified in preceding field   */
};
/* <-- DR102528, Dynamic Result Sets */

/* DR109200, Stored Procedure Dynamic Result Sets Cursor Positioning -> */
struct PclResultSetPositionType
{
    PclFlavor         Flavor;       /* 173            */
    PclLength         Length;       /* 14             */
    PclUInt16         statmnt_no;
    PclUInt64         rownum;       /* unsigned 64-bit integer */
};

struct PclAltResultSetPositionType
{
    PclFlavor         Flavor;       /* 0x8000 + 173   */
    PclLength         Length;       /* 0              */
    PclAltLength      AltLength;    /* 18             */
    PclUInt16         statmnt_no;
    PclUInt64         rownum;       /* unsigned 64-bit integer */
};
/* <- DR109200, SPDR Cursor Positioning */

/* DR115324, LOB Loader Phase 2 --> */
struct PclElicitDataByNameType
{
    PclFlavor         Flavor;         /* 176                             */
    PclLength         Length;
    PclUInt16         filespeclen;    /* length of the filespec in bytes */
    Pclchar           filespec[1024]; /* name of file in client charset  */
};

struct PclAltElicitDataByNameType
{
    PclFlavor         Flavor;         /* 0x8000 + 176                    */
    PclLength         Length;         /* 0                               */
    PclAltLength      AltLength;
    PclUInt16         filespeclen;    /* length of the filespec in bytes */
    Pclchar           filespec[1024]; /* name of file in client charset  */
};
/* <-- DR115324, LOB Loader Phase 2 */

/* DR145358, CLAC-29119 -> */
#define PCLSESINFOEXTENDOPENRESPONSE 7 /* Open Responses Extension */
#define CONTCXTBUFFER_SIZE           256

typedef struct PclSessInfoReqEntryType
 {
    PclUInt32           RequestNo;
    PclByte             RequestState;
 } PclSessInfoReqEntryType;

/*      RequestState has the following values:
            Active{1)  request is active on the database
            Redrive(2)  request is being redriven by the database
            Complete(3)  request is complete and the response is available */

typedef struct PclSessinfoOpenRespType
 {
    PclUInt16           OpenRespNum;
    PclUInt16           OpenRespLen;
    PclByte             RedriveOrNewRequestOK;
    PclUInt16           EntryLength;
    PclSessInfoReqEntryType     Request[1];
 } PclSessinfoOpenRespType;

/*        RedriveOrNewRequestOK is binary 1 if the client can submit a new
          request or redrive an outstanding request without informing the
          application of the database or network failure.

          EntryLength is the number of bytes for the following Request array.
          If EntryLength is zero, then there are no entries in the array.

          Request is an array of request state information. */

typedef struct PclSessionStatusRespType
{
    PclFlavor         Flavor;	     /* 195                              */
    PclLength	      Length;	     /*                                  */
} PclSessionStatusRespType;

/* ControlDataLength in a control data parcel contains the length of all */
/*     parcels in the entire control data section, including the         */
/*     ControlDataStart and ControlDataEnd parcels                       */
typedef struct PclControlDataType
{
    PclFlavor         Flavor;         /* 197, 198                        */
    PclLength         Length;         /* 8                               */
    PclUInt32         ControlDataLength;
} PclControlDataType;

typedef struct PclAltControlDataType
{
    PclFlavor         Flavor;         /* 0x8000 + 197, 198               */
    PclLength         Length;         /* 0                               */
    PclAltLength      AltLength;      /* 12                              */
    PclUInt32         ControlDataLength;
} PclAltControlDataType;

typedef struct PclRecoverableProtocolType
{
    PclFlavor         Flavor;         /* 200                             */
    PclLength         Length;         /* 4                               */
} PclRecoverableProtocolType;

typedef struct PclAltRecoverableProtocolType
{
    PclFlavor         Flavor;         /* 0x8000 + 200                    */
    PclLength         Length;         /* 0                               */
    PclAltLength      AltLength;      /* 8                               */
} PclAltRecoverableProtocolType;

typedef struct PclRedriveType
{
    PclFlavor         Flavor;         /* 201                             */
    PclLength         Length;         /* 5                               */
    Pclchar           RedriveStatus;
} PclRedriveType;

typedef struct PclAltRedriveType
{
    PclFlavor         Flavor;         /* 0x8000 + 201                    */
    PclLength         Length;         /* 0                               */
    PclAltLength      AltLength;      /* 9                               */
    Pclchar           RedriveStatus;
} PclAltRedriveType;

typedef struct PclRedriveSupportedType
{
    PclFlavor         Flavor;         /* 204                             */
    PclLength         Length;         /* 5                               */
    Pclchar           RedriveSupportedStatus;
} PclRedriveSupportedType;

typedef struct PclAltRedriveSupportedType
{
    PclFlavor         Flavor;         /* 0x8000 + 204                    */
    PclLength         Length;         /* 0                               */
    PclAltLength      AltLength;      /* 9                               */
    Pclchar           RedriveSupportedStatus;
} PclAltRedriveSupportedType;

typedef struct PclRecoverableNPSupportedType
{
    PclFlavor         Flavor;         /* 214                             */
    PclLength         Length;         /* 5                               */
    Pclchar           RecoverableNPSupportedStatus;
} PclRecoverableNPSupportedType;

typedef struct PclAltRecoverableNPSupportedType
{
    PclFlavor         Flavor;         /* 0x8000 + 214                    */
    PclLength         Length;         /* 0                               */
    PclAltLength      AltLength;      /* 9                               */
    Pclchar           RecoverableNPSupportedStatus;
} PclAltRecoverableNPSupportedType;

typedef struct PclAcknowledgementRequestedType
{
    PclFlavor         Flavor;         /* 202                             */
    PclLength         Length;         /* 4                               */
} PclAcknowledgementRequestedType;

typedef struct PclAltAcknowledgementRequestedType
{
    PclFlavor         Flavor;         /* 0x8000 + 202                    */
    PclLength         Length;         /* 0                               */
    PclAltLength      AltLength;      /* 8                               */
} PclAltAcknowledgementRequestedType;

#define MAXCONTROLDATALEN sizeof(PclAltControlDataType) + \
    CONTCXTBUFFER_SIZE + \
    sizeof(PclAltRecoverableProtocolType) + \
    sizeof(PclAltRedriveType) + \
    sizeof(PclAltAcknowledgementRequestedType) + \
    sizeof(PclAltControlDataType)
/* <- DR145358, CLAC-29119 */

/* CLAC-28857 ESS -> */
struct PclStatementStatusType
{
    PclFlavor         Flavor;         /* PclSTATEMENTSTATUS              */
    PclLength         Length;
    Pclchar           StatementStatus;
    Pclchar           ResponseMode;
    Pclchar           Reserved_2[2];
    PclUInt32         StatementNo;
    PclUInt16         Code;
    PclUInt16         ActivityType;
    PclUInt64         ActivityCount;
    PclUInt32         FieldCount;
    Pclchar           Reserved_4[4];
};

struct PclAltStatementStatusType
{
    PclFlavor         Flavor;         /* PclAltSTATEMENTSTATUS           */
    PclLength         Length;         /* 0                               */
    PclAltLength      AltLength;
    Pclchar           StatementStatus;
    Pclchar           ResponseMode;
    Pclchar           Reserved_2[2];
    PclUInt32         StatementNo;
    PclUInt16         Code;
    PclUInt16         ActivityType;
    PclUInt64         ActivityCount;
    PclUInt32         FieldCount;
    Pclchar           Reserved_4[4];
};

struct CliStatementStatusType         /* same as above, less parcel header */
{
    Pclchar           StatementStatus;
    Pclchar           ResponseMode;
    Pclchar           Reserved_2[2];
    PclUInt32         StatementNo;
    PclUInt16         Code;
    PclUInt16         ActivityType;
    PclUInt64         ActivityCount;
    PclUInt32         FieldCount;
    Pclchar           Reserved_4[4];
};

/* Possible values for the CliStatementStatusType.StatementStatus field */
/*   and the existing parcels they corresponds to                       */
#define CLI_ESS_SUCCESS    0  /* PclSUCCESS and PclOK */
#define CLI_ESS_ERROR      1  /* PclERROR             */
#define CLI_ESS_FAILURE    2  /* PclFAILURE           */
#define CLI_ESS_STMTERROR  3  /* PclSTMTERROR         */

/* Possible values for the CliStatementStatusType.ResponseMode field */
#define CLI_ESS_RESP_FIELD_MODE           1
#define CLI_ESS_RESP_RECORD_MODE          2
#define CLI_ESS_RESP_INDIC_MODE           3
#define CLI_ESS_RESP_MULTIPART_INDIC_MODE 4

typedef struct CliEssExt_s
{
    PclUInt16         ExtInfoID;     /* ESS Extension Info ID             */
    PclUInt32         ExtInfoLength; /* length of the extension body only */
    /* followed by a possibly empty variable length extension body */
} CliEssExt_t, *CliEssExt_p;

#define CLI_ESS_EXT_WARNINGMSG 1
typedef struct CliEssExtWarningMsg_s
{
    PclUInt16         WarningCode;
    PclUInt16         ComponentID;
    PclUInt32         WarningLength; /* length of WarningMsg */
    /* if WarningLength > 0, followed by a variable length WarningMsg */
} CliEssExtWarningMsg_t, *CliEssExtWarningMsg_p;

#define CLI_ESS_EXT_ERRORFAILURE 2 /* CLAC-30059 */
typedef struct CliEssExtErrorFailure_s
{
    PclUInt16         ErrorFailureCode;
    PclUInt16         ComponentID;
    PclUInt32         ErrorFailureLength; /* length of ErrorFailureMsg */
    /* if ErrorFailureLength > 0, followed by a var len ErrorFailureMsg */
} CliEssExtErrorFailure_t, *CliEssExtErrorFailure_p;

#define CLI_ESS_EXT_CASCADEDERROR 3 /* CLAC-30059 */
typedef struct CliEssExtCascadedError_s
{
    PclUInt16         CascadingErrorCode;
    PclUInt16         ComponentID;
    PclUInt32         CascadingErrorLength; /* length of CascadingErrorMsg */
    /* if CascadingErrorLength > 0, followed by a var len CascadingErrorMsg */
} CliEssExtCascadedError_t, *CliEssExtCascadedError_p;

#define CLI_ESS_EXT_SYNTAXERRORLOCATION 4 /* CLAC-30059 */
typedef struct CliEssExtSyntaxErrorLocation_s
{
    PclUInt32         SyntaxErrorCharOffset;
} CliEssExtSyntaxErrorLocation_t, *CliEssExtSyntaxErrorLocation_p;

#define CLI_ESS_EXT_MINRESPBUFFERSIZE 5 /* CLAC-30059 */
typedef struct CliEssExtMinRespBufferSize_s
{
    PclUInt32         MinRespBufferSize;
} CliEssExtMinRespBufferSize_t, *CliEssExtMinRespBufferSize_p;

#define CLI_ESS_EXT_MERGEACTIVITYCOUNTS 10
typedef struct CliEssExtMergeActivityCounts_s
{
    PclUInt64         InsertCount;
    PclUInt64         UpdateCount;
    PclUInt64         DeleteCount; /* CLAC-29283 */
} CliEssExtMergeActivityCounts_t, *CliEssExtMergeActivityCounts_p;

#define CLI_ESS_EXT_MULTILOADACTIVITYCOUNTS 13
/* this ESS self-defining extension contains variable length fields
   and is only here for reference:
struct CliEssExtMultiLoadActivityCounts
{
    PclUInt64         InsertCount;
    PclUInt64         UpdateCount;
    PclUInt64         DeleteCount;
    PclUInt32         TableDBNameLength;
    Pclchar           TableDBName[TableDBNameLength];
    PclUInt32         TableNameLength;
    Pclchar           TableName[TableNameLength];
}; */

#define CLI_ESS_EXT_MULTILOADRESTARTINFO 28
typedef struct CliEssExtMultiLoadRestartInfo_s
{
    PclUInt16         RestartCode;
    PclUInt16         AMPDownCode;
} CliEssExtMultiLoadRestartInfo_t, *CliEssExtMultiLoadRestartInfo_p;
/* <- CLAC-28857 ESS */

/* CLAC-30555 MLOADX -> */
#define CLI_ESS_EXT_ARRAYINSERTACTIVITYCOUNTS 31
typedef struct CliEssExtArrayInsertActivityCounts_s
{
    PclUInt64         InsertCount;
    PclUInt64         ErrorCount;
} CliEssExtArrayInsertActivityCounts_t, *CliEssExtArrayInsertActivityCounts_p;
/* <- CLAC-30555 MLOADX */

/* CLAC-29115 1MB row -> */
#define CLI_ESS_EXT_MAXLARGEROWSRECSIZEINFO 32
typedef struct CliEssExtMaxLargeRowsRecSizeInfo_s
{
    PclUInt32         MaxLargeRowSize;
    PclUInt64         LargeRowCount;
} CliEssExtMaxLargeRowsRecSizeInfo_t, *CliEssExtMaxLargeRowsRecSizeInfo_p;
/* <- CLAC-29115 1MB row */

/* CLAC-30059 -> */
struct PclSTMTERRORType
{
    PclFlavor          PclKind;          /* PclSTMTERROR      */
    PclLength          PLength;          /* 12 to 267         */
    PclWord            StatementNo;
    PclWord            Info;
    PclWord            Code;
    PclWord            Length;
    Pclchar            Msg[255];         /* 1 to 255 bytes    */
};

struct PclAltSTMTERRORType
{
    PclFlavor          PclKind;          /* PclAltSTMTERROR   */
    PclLength          PLength;          /* 0                 */
    PclAltLength       AltLength;        /* 16 to 271         */
    PclWord            StatementNo;
    PclWord            Info;
    PclWord            Code;
    PclWord            Length;
    Pclchar            Msg[255];         /* 1 to 255 bytes    */
};
/* <- CLAC-30059 */

/* CLAC-31559 --> */
/*===========================*/
/* Small LOB (SLOB) Parcels  */
/*===========================*/
struct PclSLOBResponse
{
    PclFlavor                 Flavor;    /* 215 */
    PclLength                 Length;
    PclUInt64                 MaxSingleLOBBytes;
    PclUInt64                 MaxTotalLOBBytesPerRow;
};
struct PclAltSLOBResponse
{
    PclFlavor                 Flavor;    /* 0x8000 + 215 */
    PclLength                 Length;
    PclAltLength              AltLength;
    PclUInt64                 MaxSingleLOBBytes;
    PclUInt64                 MaxTotalLOBBytesPerRow;
};
/* <-- CLAC-31559 */

/*  CLAC-30554 -->           */
/*===========================*/
/* Small LOB (SLOB) Parcels  */
/*===========================*/
struct PclSLOBDataStart
{
    PclFlavor                 Flavor;    /* 220 */
    PclLength                 Length;
/*  CLAC-31559 -->  */
    PclUInt32                 MetadataItemNo;
/*  Optional field, not supported by CLI      */
/*  PclUInt32                 ArrayElementNo; */
/*  <-- CLAC-31559  */
};

struct PclAltSLOBDataStart
{
    PclFlavor                 Flavor;   /* 0x8000 + 220 */
    PclLength                 Length;
    PclAltLength              AltLength;
/*  CLAC-31559 -->  */
    PclUInt32                 MetadataItemNo;
/*  Optional field, not supported by CLI      */
/*  PclUInt32                 ArrayElementNo; */
/*  <-- CLAC-31559  */
};
/* <-- CLAC-30554  */

/* CLAC-30758 --> */
struct CliSlobDataStartType
{
  PclUInt32     MetadataItemNo;
};

struct CliSlobDataType
{
  PclUInt64     SlobLength;
  Pclchar       SlobData[1];   /* variable length */
};
/* <-- CLAC-30758 */

/* CLAC-29671 --> */
/*===========================*/
/* Client Attributes         */
/*===========================*/
#define PCLCLIATT_HOSTNAME                     7
#define PCLCLIATT_PROCTHRID                    8
#define PCLCLIATT_SYSUSERID                    9
#define PCLCLIATT_PROGNAME                    10
#define PCLCLIATT_OSNAME                      11
#define PCLCLIATT_JOBNAME                     13    /* CLMF-9820 */
#define PCLCLIATT_JOBID                       21    /* CLMF-9820 */
#define PCLCLIATT_CLIV2RELID                  24
#define PCLCLIATT_JOBDATA                     27
#define PCLCLIATT_CLIENTATTRIBUTESEX          30    /* CLIWS-8509 */
#define PCLCLIATT_CLIENT_IPADDR_BY_CLIENT     31
#define PCLCLIATT_CLIENT_PORT_BY_CLIENT       32
#define PCLCLIATT_SERVER_IPADDR_BY_CLIENT     33
#define PCLCLIATT_SERVER_PORT_BY_CLIENT       34
#define PCLCLIATT_COP_SUFFIXED_HOST_NAME      45
#define PCLCLIATT_CLIENT_CONFIDENTIALITY_TYPE 58    /* CLIWS-7192 */
#define PCLCLIATT_END                      32767

/* maximum length for PCLCLIATT_JOBDATA */
#define PCLCLIATT_JOBDATA_MAXLEN          128
/* <-- CLAC-29671*/

/* CLIWS-8509 maximum length for PCLCLIATT_CLIENTATTRIBUTESEX */
/* See https://docs.teradata.com/r/Teradata-VantageTM-Data-Dictionary/June-2022/Views-Reference/SessionInfoV-X */
#define PCLCLIATT_CLIENTATTRIBUTESEX_MAXLEN 512

/* DR 68259 ---> */
/* DR84603: Add pack(1) for Win32*/
#if defined(WIN32) || defined(__APPLE__) || defined(AIX) /* CLAC-18937, CLIWS-7896 */
#pragma pack(pop)
#elif defined(__MVS__)     /* DR106757 */
#pragma pack(reset)        /* DR106757 */
#else
#pragma pack()
#endif
/* <--- DR 68259 */

/**************************************************************/
/*                                                            */
/*                    END OF PARCEL.H                         */
/*                                                            */
/**************************************************************/
#endif
