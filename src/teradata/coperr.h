#ifndef COPERR_H
#define COPERR_H
#define COPERR_H_REV "20.00.00.09"
/*******************************************************************************
 *
 *
 *  WARNING: Because coperr.h is shared and is used as a header
 *           file for mainframe SAS/C compiles under VM/CMS, the logical
 *           record length (i.e., line length) must not exceed 80
 *           characters.
 *-----------------------------------------------------------------------------*
 *                   <-- ( 8 0    c h a r a c t e r s) -->
 *******************************************************************************
 *  TITLE:       COPERR.H .. Error code definitions for "C"          20.00.00.09
 *
 *  Purpose      To contain the error codes for the workstation
 *               code written in "C".
 *
 *  Copyright 1983-2025 by Teradata Corporation.  All rights reserved.
 *
 *  Description  This include file contains the error codes for
 *               the workstation version of BTEQ, CLIV2, MTDP,
 *               and MOSI. The text associated with the codes is
 *               is contained in a file which is referenced by
 *               the OsGetErr routine using the input code value
 *               to index into the file.
 *
 *  History
 *   F.1   86Dec10  DAA  DCR3075  Coded
 *   F.2   87Jan16  DAA  DR8678
 *   F.3   87Jan20  DAA  DR8678   Fix spacing of comments
 *   F.4   87Feb20  KKL  DR9315   Add EM_NOHOST; Reassign
 *                                MOSI errno numbers
 *   F.5   87Feb26  KKL  DR9379   Added ER_CONNID, ER_MAXSESS
 *   F.6.  87Mar17  DAA  DR9596   Fixed mtdpcount value
 *   F.7   87Apr29  DAA  DR9899   Changed DBRPUTZ
 *   F.8   87Jun05  DAA  DCR3430  Vax support
 *   F.9   87Jul01  DAA  DCR3430  Vax support
 *   F.10  87Jul21  DAA  DCR3780  Add options parcel
 *   F.11  87Aug11  DAA  DR10661  Wake process on break
 *   F.12  87Aug25  DAA  DR10809  Fix multisession Reconn
 *   F.13  87Nov15  SCT  DR10471  Narrowed to <= 65 cols.
 *   F.14  87Nov15  DAA  DR11311  Add errmsg version No.
 *   F.15  87Nov15  DAA  DR 10471 Fix it
 *   F.16  88Jan28  DRP  DR11926  (MR# fe87-18102a0)
 *   F.17  88Apr04  DAA  DR11970  Fix floc/fvar ncompatibility
 *   F.18  88Apr04  DAA  DR11970  Fix incorrect release
 *   F.19  88Apr22  KKL  DR12497  Add ER_CONNREFUSED
 *   F.20  88Apr22  KKL  DR12497  Reship DR12497
 *   F.21  88Jul01  WBJ  DR10377  Add BADRSUP
 *   G0_00 88Jul25  SN   Created  For Release G.0
 *   G0_01 88Oct07  WBJ  DR12820  Enhance error reporting
 *   G0_02 88Oct13  WBJ  DCR4029 & 4278 new option errors
 *   G0_03 89Dec15  KXO  DCR5239  Add Error 327, 328, 329
 *         90Feb02  KXO  DR18467  Add Error 330
 *   G0_04 90Sep14  KXO           Revised for UNIXTOOL KIT
 *   G0_05 91Jan05  DJL  DCR5366  Tandem port
 *   G0_06 90Aug16  GLL  DR19967  Add MOSI 143,CLI 331
 *   H0_00 93May20  SNT  DCR6711  Created for Release H.0
 *   H0_01 93Aug19  Bg1  DCR5571  Add for Cli User Exit.
 *   H0_02 94Mar11  JAE  DR28060  Added REQOVFLOW (CLI 333).
 *   H0_03 94Mar25  JAE  DCR6172  Added 2PC error codes,
 *                                changed USREXT to
 *                                349, changed REQOVFLOW to
 *                                350.
 *   H0_04 94Apr04  JAE  DCR6172  Added EM_NOSESSNO for 2PC.
 *   H0_05 94Mar25  km5  DCR6842&6852 Added SQLUSREXT (CLI 351).
 *   HN_00 94Aug19  JAE  Added new error codes for NDM.
 *   HN_01 95FEB09  ALB  DR32294 Merged 5.0 and AFCAC code.
 *   H3_00 95Aug23  JAE  DR32294 Completed code merge by removing #ifdef NDM.
 *   H3_01 95Sep13  TH4  DR34047  Added the #ifndef XXXX #define XXXX.
 *   04.00.00.01 TH4 96Jul12 DR36785 Version number change.
 *   04.01.00.02 JAE 96Nov11 DR38038 Changed 312 error name to INVALIDFN.
 *   04.01.00.03 TH4 96Dec30 DR38016 Don't kill application when PCLI deadlock.
 *   04.01.00.04 JAE 97Jan13 DR38017 Added new PCLI error messages (359-368)
 *   04.01.00.05 TH4 97Feb26 DR38910 Added new PCLI error message 369.
 *   04.02.00.00 JTH July97  dr40000 Code merge
 *   04.03.00.00 TH4 98Feb05 DR40973 Porting Parallel CLI to WinCLI.
 *   04.04.02.00 TH4 99Feb02 DR37947 new error code for cancel logon.
 *   04.05.00.00 CSG 2000Mar03 DR48735 TDSP Support
 *   04.05.00.01 CSG       2000Mar22 DR50139 TDSP Report Error 374 (NOTDSP)
 *   04.06.00.00 ASG       2001Jan17 DR52052 Support SSO
 *   04.06.01.00 CSG       2001Sep05 DR57420 Added error 376 (NOBUFFER).
 *   04.06.01.01 cc151010  2001Aug30 DR52050 Support multi-threaded CLI.
 *   04.06.02.00 cc151010  2002Jan11 DR59375 Add NOACTIVEREQUEST(385).
 *   04.06.02.01 CSG       2002Jan28 DR57144 CLI should not allow options
 *                                           var_len_fetch=Y and parcel_mode=N
 *   04.06.02.02  CSG      2002Mar26 DR51742 support req_proc_opt=B in
 *                                           parcel mode
 *   04.06.07.00  ASG      2002Apr02 DR57583 LOB Support
 *   04.07.00.01  CSG      2002Jun17 DR61637 Support enlarged parcel usage.
 *   04.07.00.02  CSG      2002Aug22 DR52059 L10N support.
 *   04.07.01.00  CSG      2003Mar03 DR61527 Error codes for DR60695,
 *                                           DR67168, DR 61527 and DR 66712.
 *   04.07.01.01 mg180007  2003Mar12 DR67744 Remove HPUX compilation warnings.
 *   04.07.01.02 CSG       2003Mar24 DR67771 Replace UCS2 with UTF16.
 *   04.07.01.03 mg180007  2003Apr24 DR68084 renamed SSO errors to GSS
 *   04.08.00.00 mg180007  2003Jul22 DR68511 clean up defines, prototypes
 *                                   DR68139 self-sufficient headers
 *   04.08.00.01 mg180007  2004Jan07 DR68829 changed return type for CatOpen()
 *                                           changed EM_DLLLOADFAIL comment
 *   04.08.00.02 ASG       2003Dec05 DR68838 Add CLI501 (array ops)
 *   04.08.00.03 ASG       2004Jan28 DR68835 APH / 1 MB Responses
 *   04.08.00.04 ASG       2004Feb10 DR58352 ESS
 *   04.08.00.05 ASG       2004Feb19 DR85431 universal headers
 *   04.08.00.06 mg180007  2004May18 DR68842 security (507, 508)
 *   04.08.00.07 mg180007  2004May29 DR68842 security (509)
 *   04.08.00.08 ASG       2004Sep17 DR89408 Fix 64-bit prob w/ array ops feat
 *   04.08.00.09 mg180007  2004Nov11 DR88745 ICU loading (510)
 *   04.08.01.00 mg180007  2005Feb14 DR92232 merge and update copyright year
 *   04.08.01.01 mg180007  2005Apr21 DR93472 added header revision info
 *   04.08.01.02 ASG       2005Jun23 DR94622 dual-component feature support
 *   04.08.01.03 mg180007  2005Aug09 DR93472 fixed header revision info
 *   04.08.02.00 bh185000  2005Dec15 DR98024 Support for MaxDecimalPrecision
 *                                   DR98025 Support for AGKR_option
 *                                   DR98026 Support for StatementInfo
 *   04.08.02.01 ASG       2006Feb14 DR102061 reconfig for 80-byte lines
 *   04.09.00.00 bh185000  2006May30 DR102528 Support for DynamicResultSets
 *   04.09.00.01 mg180007  2006Jul11 DR101287 Support for Default Connect
 *   12.00.00.02 fs185009  2006Nov29 DR108852 version change to 12.0.x
 *   12.00.00.03 bh185000  2006Nov30 DR108909 Added an error code for workload
 *   13.00.00.00 mg180007  2008Jan09 DR117140 copyright update
 *   13.00.00.01 fs185009  2008Jan14 DR107850 added 3 more icu load error codes
 *   13.00.00.02 mg180007  2008Feb27 DR112838 TDUSR loading (530)
 *   13.01.00.00 bh185000  2008Nov17 DR116354 Support Extended Object Name
 *   13.10.00.00 kl185018  2009Jun23 DR133468 TTU13.1 Version Renumbering
 *                                            to TTU13.10
 *   14.00.00.00 bh185000  2010Oct06 DR121202 Support Statement Independence
 *   14.00.00.01 mg180007  2010Dec10 DR140617 Added BADMSGLEN
 *   14.00.00.02 bh185000  2011Feb24 DR145224 Support Array Data Type
 *   14.10.00.00 bh185000  2012Jan10 CLAC-28875 Support XML Data Type
 *   14.10.00.01 mg180007  2012Jan31 CLAC-663   Added TDWALLET errors 543,544
 *   14.10.00.02 bh185000  2012May16 CLAC-29283 Support Extended Multiload
 *   14.10.00.03 mg180007  2012Jun13 CLAC-29119 Support Automatic Redrive
 *   14.10.00.04 hs186016  2013Feb21 CLAC-30478 Support TASM FastFail
 *   15.00.00.00 hs186016  2013Nov14 CLAC-30554 Support SLOB
 *   15.10.00.00 mg180007  2014Apr28 CLAC-32368 version and copyright update
 *   16.00.00.00 bh185000  2015Jun24 CLAC-29115 Support 1MB Response Rows
 *   17.00.00.00 hs186016  2018Nov13 CLIWS-7163 version and copyright update
 *   17.00.00.01 vt186023  2019May22 CLIWS-7192 Support TLS v1.2
 *   17.10.00.00 hs186016  2020Jul27 CLIWS-7606 Update version
 *   17.10.00.01 hs186016  2020Aug10 CLIWS-7327 Verify server certificate
 *   17.10.00.02 hs186016  2020Sep03 CLIWS-7647 Handle gateway error 8059
 *   17.10.00.03 rp255060  2021Jan25 CLIWS-7635 Connection string messages
 *   17.10.00.04 rp255060  2021Apr09 CLIWS-7901 Return error on invalid sslmode
 *   17.10.00.05 hs186016  2021Jun03 CLIWS-7751 Support mainframe on z/OS
 *   17.20.00.00 hs186016  2022Jan01 CLIWS-8198 version and copyright update
 *   17.20.00.01 hs186016  2022Mar22 CLIWS-8295 Support RACF JWT for mainframe
 *   17.20.00.02 hs186016  2022Apr12 CLIWS-8295 Add more return codes from RACF JWT
 *   17.20.00.03 rp255060  2022Jul08 CLIWS-8410 Browser error code
 *   17.20.00.04 rp255060  2022Jul12 CLIWS-8233 OCSP support
 *   17.20.00.05 hs186016  2022Sep09 CLIWS-8512 Provide detailed info when TLS fails
 *   17.20.00.06 rp255060  2022Dec01 CLIWS-8614 CRL support
 *   17.20.00.07 hs186016  2022Dec22 CLIWS-8618 Update the IDT error messages for z/OS
 *   20.00.00.00 hs186016  2023May11 CLIWS-8696 version and copyright update
 *   20.00.00.01 rp255060  2023Aug01 CLIWS-8661 Proxy server support
 *   20.00.00.02 rp255060  2024Feb29 CLIWS-9071 Client credentials flow support
 *   20.00.00.03 rp255060  2024Mar01 CLIWS-9074 Improve error message when IdP certificate not trusted
 *   20.00.00.04 hs186016  2024Mar19 CLIWS-9070 Device Authorization Grant (device flow) support
 *   20.00.00.05 rp255060  2024Jun14 CLIWS-9221 Add error for Fips mode failure
 *   20.00.00.06 rp255060  2024Sep18 CLIWS-9380 Add bad logmech data error
 *   20.00.00.07 rp255060  2024Nov11 CLIWS-9492 New OIDC token cache
 *   20.00.00.08 hs186016  2025Jan09 CLIWS-9597 Fix with SSLMODE=verify-full, when OCSP related
 *                                              validation error occurs, CLI returns error without
 *                                              checking CRL (z/OS only)
 *   20.00.00.09 hs186016  2025Jan29 CLIWS-9637 Support TLS 1.3
 ******************************************************************************/


/***************************************************************/
/*  Instructions for setting up error codes and class specifier*/
/*  tables.                                                    */
/*                                                             */
/*  The error message generating routine has been written in a */
/*  very generic fashion to permit its future use in many other*/
/*  applications with any variety of error message text.  To   */
/*  provide enough information for meaningful processing of the*/
/*  error number codes into actual text messages, while still  */
/*  permitting the flexibility of modular development, error   */
/*  class specifier tables are used to indicate base error code*/
/*  values for specific errors. These tables include a base    */
/*  error number for the class, a count of the error codes as- */
/*  signed to the class, and an offset value to be subtracted  */
/*  from an error code to produce a true index into the error  */
/*  file pointer structure used by the OsGetErr routine.       */
/*                                                             */
/*  The only restriction imposed by this method is that the    */
/*  initialization of the error class specifier array be done  */
/*  from lowest base error number to highest, since OsGetErr   */
/*  scans the list of tables until the error code is greater   */
/*  than or equal to the base error code of a class table.     */
/*  Also, error codes within a class should be allocated in    */
/*  order, that is, without skipping any numbers.  If numbers  */
/*  are skipped, dummy error messages will have to be kept in  */
/*  the error message file, as place holders, since no search  */
/*  methods, or indirect referencing mechanism is used to      */
/*  fetch error text.  This method also requires that all      */
/*  error codes be referenced in the application only by the   */
/*  macro definition given in this file (this is good pro-     */
/*  gramming practice anyway).                                 */
/*                                                             */
/*  Two macro definitions in this file are used to provide     */
/*  flexibility  between applications and hosts.  ERRFNAME is  */
/*  used to define the name of the error message file for      */
/*  OsGetErr, thus providing a single point of control for     */
/*  various systems, and ERRTYPCNT specifies the number of     */
/*  error classes and, therefore, of error class tables.       */
/*                                                             */
/*                                                             */
/***************************************************************/

#include "coptypes.h" /* DR68139 */

#define  ERRVERSION   "Version is 20.00"
#define  ERRTYPCNT     3              /* # of error classes    */
                                      /* (MOSI, MTDP, & CLIV2) */

struct  ErrClassSpec
{
    Int32      base;          /* base error number for  class  */
    Int32      offset;        /* offset value for computing    */
                              /* error file index              */
    Int32      max;           /* maximum allowed error index   */
                              /* for this class                */
};


/***************************************************************/
/*                                                             */
/*                       MOSI - error codes                    */
/*                                                             */
/***************************************************************/

/*==================*/
/*   Error Class    */
/*==================*/
#define  EC_MEMORY       101  /* memory management             */
#define  EC_TIME         102  /* time management               */
#define  EC_ENVIRONMENT  103  /* system environment management */
#define  EC_DATA         104  /* data management               */
#define  EC_HANDLER      109  /* interrupt management          */
#define  EC_NETWORK      150  /* network management            */
#define  EC_MTDP         201  /* mtdp; should be in mtdpif.h   */

/*==================*/
/*   Error Reason   */
/*==================*/

#define  MOSIBASE        100  /* base error number for MOSI    */
#define  ER_SUCCESS        0  /* successful function completion*/
#define  ER_OPSYS        100  /* operating system specific     */
#define  ER_SUPPORT      101  /* function not supported        */
#define  ER_MEMSIZE      111  /* invalid memory size           */
#define  ER_INSUFF       112  /* insufficient memory           */
#define  ER_MEMBLOCK     113  /* invalid storage block         */
#define  ER_VALUE        114  /* invalid time value            */
#define  ER_UNIT         115  /* invalid time unit             */
#define  ER_SOCKET       116  /* cannot open a socket          */
#define  ER_HOST         117  /* cannot find host              */
#define  ER_ACCEPT       118  /* cannot accept from remote host*/
#define  ER_SHUTDOWN     119  /* cannot shutdown connection    */
#define  ER_CONNECT      120  /* cannot connect w/ remote host */
#define  ER_NAMEFORMAT   121  /* invalid name format           */
#define  ER_SERVER       122  /* cannot find entry in services */
                              /* file                          */
#define  ER_BUFSIZE      123  /* invalid buffer size           */
#define  ER_SEND         124  /* cannot send data              */
#define  ER_RECEIVE      125  /* cannot receive data           */
#define  ER_ACTUAL       126  /* cannot send amount of data as */
                              /* req'd                         */
#define  ER_RESET        127  /* net hardware !reset or loaded */
#define  ER_RECBUFSIZE   128  /* Rsp Buffer not large enough   */
#define  ER_RET          129  /* cannot retrieve the data      */
#define  ER_LINKDOWN     130  /* the link between workstation  */
                              /* & dbc is down; dbc could be   */
                              /* down                          */
#define  ER_NODATA       131  /* data is not here yet          */
#define  ER_STRINGSIZE   132  /* invalid string size           */
#define  ER_CONNID       133  /* invalid connectid             */
#define  ER_MAXSESS      134  /* num of sessions opened exceed-*/
                              /* ing limit                     */
#define  ER_NOTDONE      135  /* write/send in progress        */
#define  ER_SEQUENCE     136  /* read/write calls out of order */
#define  ER_NOREQ        137  /* no such request               */
#define  ER_TIMEOUT      138  /* read wait timeout             */
#define  ER_BREAK        139  /* break received during wait    */
#define  ER_BADHOST      140  /* bad hosts file                */
#define  ER_CONNREFUSED  141  /* connection refused            */
#define  ER_NOHOST       142  /* cAn't open hostS file DR12820 */
#define  ER_UNKNOWNINT   143  /* DR19967 Unknown external interrupt */

/* CLIWS-7192 --> */ /* TLS error codes from MOSI */
#define  ER_TLS_CLIMETHOD     144 /* TLS connection method creation error */
#define  ER_TLS_APPCONTEXT    145 /* SSL_CTX object creation error        */
#define  ER_TLS_HANDLE        146 /* TLS structure creation error         */
#define  ER_TLS_SETCONNID     147 /* Connect TLS object with connid error */
#define  ER_TLS_LOADCERTSTORE 148 /* Set CA certificates location error   */
#define  ER_TLS_VERIFYPARAM   149 /* Get TLS verification parameter error */
#define  ER_TLS_CONNTIMEOUT   150 /* TLS connection timed out             */
#define  ER_TLS_CONNECT       151 /* TLS handshake error                  */
#define  ER_TLS_WRITE         152 /* TLS write error                      */
#define  ER_TLS_READ          153 /* TLS read error                       */
/* <-- CLIWS-7192 */
#define  ER_TLS_CERT_REVOKED  154 /* OCSP status error CLIWS-8233, CLIWS-8614 */
#define  ER_CLOSED_BY_REMOTE  155 /* Connection closed by remote CLIWS-8512 */
#define  ER_TLS_OCSP_STATUS   156 /* OCSP status error CLIWS-8614 */
#define  ER_TLS_CRL_STATUS    157 /* CRL status error CLIWS-8614          */
#define  ER_NOPROXYHOST       158 /* Could not resolve proxy hostname. */
#define  ER_PROXYAUTHERR      159 /* Proxy authentication error */
#define  ER_PROXYCONNREFUSED  160 /* Proxy server connection refused */
#define  ER_TLS_CERT_VAL_ERR  161 /* Certificate validation error, CLIWS-9597 */

#define  EM_OK             0  /* no error                      */

#define  MOSICNT          62  /* DR12820, CLIWS-7192, CLIWS-7327, CLIWS-8512 */
                              /* CLIWS-9597: 61 -> 62 */

#if !(defined(I370) || (defined(IBM370))) || defined(CLI_MF_GTW)    /* CLIWS-7751 */
/*=============================*/
/*    Error Codes from MTDP    */
/*=============================*/

#define  MTDPBASE        200  /* base error number for mtdp    */

#define  EM_VERSION      202  /* wrong software version        */
#define  EM_BUFSIZE      203  /* invalid or too small buf size */
#define  EM_MOSI         204  /* error coming from MOSI routine*/
#define  EM_FUNC         205  /* invalid mtdp function call    */
#define  EM_ASSIGN       206  /* err parcel rec'd during assign*/
#define  EM_NETCONN      207  /* err in doing a network connect*/
#define  EM_NOTIDLE      208  /* not ready to accept new req   */
#define  EM_REQID        209  /* request id not recognized     */
#define  EM_NOTACTIVE    210  /* no START/CONT outstanding     */
#define  EM_NODATA       211  /* no data received from dbc     */
#define  EM_DATAHERE     212  /* data is ready from dbc        */
#define  EM_ERRPARCEL    213  /* error parcel received         */
#define  EM_NOSCS        214  /* SCS not found in mtdp         */
#define  EM_CONNECT      215  /* error pcl rec'd during connect*/
#define  EM_BUFOVERFLOW  216  /* rsp buffer size !large enough */
#define  EM_TIMEOUT      217  /* Time out waiting for a RESP   */
#define  EM_BREAK        218  /* Break was hit                 */
#define  EM_DBC_CRASH_B  219  /* The DBC has CRASHED before    */
                              /* the request was sent          */
#define  EM_DBC_CRASH_A  220  /* The DBC has CRASHED after the */
                              /* request was sent              */
#define  EM_PUBENCRYPT   221  /* The Public encryption routine */
                              /* failed                        */
#define  EM_CONV         222  /* error in data conversion      */
#define  EM_ALLOCATE     223  /* error in allocating memory BLK*/
#define  EM_NOHOST       224  /* no DBC found in hosts file    */
#define  EM_SUPPORT      225  /* function not supported        */
#define  EM_CHARCODE     226  /* invalid char set code DCR4029 */
#define  EM_CHARNAME     227  /* invalid char set name DCR4029 */
#define  EM_NOSESSNO     228  /* session id not supplied,H0_04 */


/* DR52052, DR68084: GSS/SSO-related error conditions */
#define  EM_SENDAUTHREQFAIL    229  /* authreq parcel send failed  */
#define  EM_RECVAUTHRSPFAIL    230  /* authrsp parcel recv failed  */
#define  EM_RETMAUTHRSPFAIL    231  /* authrsp parcel retm failed  */
#define  EM_INVPCLAUTHRSP      232  /* invalid authrsp parcel      */
#define  EM_DLLLOADFAIL        233  /* LoadLibrary failed          */
#define  EM_GETPROCFAIL        234  /* GetProcAddr failed (UNUSED) */
#define  EM_GSSINITFAIL        235  /* call to gss_init failed     */
#define  EM_GSSCALLFAIL        236  /* call to gss_call failed     */
#define  EM_SENDGSSREQFAIL     237  /* ssoreq parcel send failed   */
#define  EM_RECVGSSRSPFAIL     238  /* ssorsp parcel recv failed   */
#define  EM_RETMGSSRSPFAIL     239  /* ssorsp parcel retm failed   */
#define  EM_INVPCLGSSRSP       240  /* invalid ssorsp parcel       */
#define  EM_ERRFAILPCLAUTHRSP  241  /* error or failure auth rsp   */
#define  EM_ERRFAILPCLGSSRSP   242  /* error or failure sso rsp    */
#define  EM_SSOUNAVAILABLE     243  /* SSO not available           */
#define  EM_SSOLOGONFAIL       244  /* SSO logon failed by gateway */
#define  EM_TIMEEXCEEDED       245  /* MTCLI: timeout from DBCHWL  */
#define  EM_LOCKFAILED         246  /* MTCLI: locking error        */
#define  EM_NOUTF16SUPPORT     247  /* NO UTF16 characterset support*/
                                    /* DR67771, Renamed UCS2 as UTF16*/
#define  EM_TLSCONN            248  /* TLS connection failed       */
#define  EM_WEBSOCKETCONN      249  /* WebSocket connection failed */
#define  EM_MUSTUSETLS         250  /* CLIWS-7647: Must use TLS because*/
                                    /*   gateway's TLS mode is require */
#define  EM_CLOSED_BY_REMOTE   251  /* CLIWS-8512: Connection closed by remote*/
#define  EM_NOTFOUND           252  /* CLIWS-7948: not found */
#define  EM_NOPROXYHOST        253  /* CLIWS-8661: Could not resolve proxy hostname. */
#define  EM_PROXYAUTHERR       254  /* CLIWS-8661: Proxy authentication error */
#define  EM_PROXYCONNREFUSED   255  /* CLIWS-8661: Proxy server connection refused */
#define  EM_TLS_CERT_VAL_ERR   256  /* CLIWS-9597: Certificate validation error on z/OS */

#define  MTDPCNT          56        /* CLIWS-9597: 55 -> 56 */

/***************************************************************/
/*                                                             */
/*                      CLIV2 - error codes                    */
/*                                                             */
/***************************************************************/

#define  CLI2BASE        300  /* base error number for cliv2   */
#define  NOMEM           300  /* out of memory error           */
#define  SESSOVER        301  /* exceeded max no of sessions   */
#define  BADBUFRQ        302  /* invalid buffer size           */
#define  BADLOGON        303  /* invalid logon string          */
#define  NOSESSION       304  /* specified session doesn't     */
                              /* exist                         */
#define  NOREQUEST       305  /* specified req does not  exist */
#define  BADPARCEL       306  /* invalid parcel received       */
#define  REQEXHAUST      307  /* request data exhausted        */
#define  BUFOVFLOW       308  /* parcel data too large for req */
                              /* buffer                        */
#define  REWINDING       309  /* rewinding request             */
#define  NOREW           310  /* cannot rewind the request     */
#define  NOACTIVE        311  /* no active sessions            */
#define  INVALIDFN       312  /* wrong function code           */
#define  DBRPUTZ         312  /* wrong function code           */
#define  BADID           313  /* bad indentifier field in logon*/
                              /* string                        */
#define  NOPASSWORD      314  /* no password in logon string   */
#define  SESSCRASHED     315  /* session has crashed           */
#define  NOTINIT         316  /* cliv2 has not been initialized*/
#define  BADENVIRON      317  /* invalid environment variable  */
#define  NOSPB           318  /* no SPB file found             */
#define  BADSPB          319  /* SPB file has bad format       */
#define  BADRECON        320  /* Bad reconnect options combin- */
                              /* ation                         */
#define  BADLEN          321  /* Bad dbcarea length            */
#define  BADOPTS         322  /* Bad fvar/floc options         */
#define  BADRSUP         323  /* using data !permitted in RSUP */
                              /* DR10377                       */
#define  BADIDAT         324  /* bad indicdata option DCR4278  */
#define  BADREQMODE      325  /* bad req mode  option DCR4278  */
#define  BADDLIST        326  /* Descriptor list length not    */
                              /* multiple of 8                 */

                              /***** added Dec89 DCR5239   *****/
                              /*****  Parameterized SQL    *****/
#define  BADDBCX         327  /* invalid DBCAREA ext eyecatcher*/
                              /* must contain 'DBCX'           */
#define  BADELMTYPE      328  /* invalid DBCAREA ext element   */
                              /* type : must be from 1 to 4096 */
#define  BADELMLEN       329  /* for ptr type element,         */
                              /* fixed len, 10                 */
                              /*** END OF Parameterized SQL ****/

                              /***** added Feb90 DR18467   *****/
                              /***HUT Interface - Direct Req ***/
#define  BAD_MSG_CLASS   330  /* dbcarea.msgclass must be      */
                              /* SysMsgHUTClass : constant 25  */
#define  UNKNOWNINT      331  /* DR19967 unknown external interrupt */
#define  UNVAL2PC        332  /* Unrecognized sess_2pc_mode value   */
#define  NOSESSNO        333  /* DBC session id not supplied   */
#define  DBXMISS         334  /* No DBCAREA extension supplied */
#define  BAD2PXID        335  /* dbx2_id in DBCA2PX is not 'DBX2'      */
#define  BAD2XLEN        336  /* dbx2_len contains incorrect length    */
#define  MAINONLY        337  /* Mainframe only fields contain data    */
#define  UN2FUNC         338  /* dbx2_func contains unrecognized value */
#define  N2PCSESS        339  /* not a 2PC session                     */
#define  PEXTRA          340  /* extraneous parameters are supplied    */
#define  PMISSING        341  /* required parameters are missing       */
#define  PLENMAX         342  /* parameter length is longer than max.  */
#define  UNDBRIPF        343  /* init_with_2pc specifies unrecognized  */
                              /* value.                                */
#define  CRNOCOM         344  /* current run unit not completed.       */
#define  ABT2REQ         345  /* abort required for the current run-   */
                              /* unit on the session                   */
#define  SIOTRUNC        346  /* area too small to describe all        */
                              /* sessions -- truncated                 */
#define  NONETH          347  /* function not supported on network-    */
                              /* attached hosts.                       */
#define  SSINDOUBT       348  /* Session is in doubt.                  */
#define  USREXT          349  /* DCR5571 Cli User Exit function*/
#define  REQOVFLOW       350  /* DR 28060 -  Request size      */
                              /* exceeds maximum.              */
#define  SQLUSREXT       351  /* DCR6842&6852 User SQL exit    */
                              /* H3_00 */
                              /* HN_00 - NDM Error Codes       */
#define  BADTXSEM        352  /* Unrecognized Trans. Semantics option */
#define  BADLANCONF      353  /* Unrecognized Language Conf. option.  */
#define  NOQINFO         354  /* The requested query information is   */
                              /* unavailable.                         */
#define  QEPITEM         355  /* Requested item in DBCHQEP is invalid.*/
#define  QEPALEN         356  /* Return area length is too small.     */
#define  QEPAREA         357  /* No return area supplied in DBCHQEP.  */
/* DR40973 */
/* DR68511 --> */
#define  PDEADLOCK       358  /* DR38016, Parallel CLI deadlock. */

/* DR 38017 - Error messages (359 - 360) are for "Parallel CLI" (PCLI)   */

#define  PCLIENV         359  /* Invalid PCLI environment variable(s).   */
#define  PSEMINIT        360  /* Could not initialize PCLI semaphores.   */
#define  PSHMINIT        361  /* Could not initialize PCLI shared memory */
#define  PSEMCREATE      362  /* Could not setup specific PCLI semaphore.*/
#define  PALLOCSHM       363  /* Could not allocate shared memory for    */
                              /* specific PCLI structure.                */
#define  PALLOCLOCK      364  /* Could not setup lock semaphore for      */
                              /* specific PCLI structure.                */
#define  PATTNINIT       365  /* Could not initialize CLIV2ATTN flag.    */
#define  PCOMMPIPE       366  /* Could not setup communication pipe      */
                              /* between application server and recovery */
                              /* server.                                 */
#define  PRECOVFORK      367  /* Could not fork recovery server.         */
#define  PSERVINIT       368  /* MTDP Servers did not initialize.        */
#define  PMTDPCLEAN      369  /* DR38910, Error on creating MTDPCLEAN.   */
#define  CANCELLOGON     370  /* DR37947, new code for cancel logon.     */
#define  BADSEGNUM       371  /* DR48735, TDSP, Segmented request	 */
			      /* exceed the  limit set by Server	 */
#define  BADSEGOPTS      372  /* DR48735, TDSP, Invalid options for TDSP */
			      /* Segmented Request.			 */
#define  BADSEGMENT      373  /* DR48735, TDSP, Invalid Sequence	 */
			      /* of Segments for Segmented Transaction   */
#define  NOTDSP          374  /* DR50139, TDSP, Added error message  	 */
			      /* No TDSP support in the server		 */
#define  BADEXTENSION    375  /* SSO: DR52052 - bad dbcarea extension    */
#define  NOBUFFER        376  /* DR57420: No buffer provided for request */
                              /* mode 'B'                                */
#define  BADUEFUNC       377  /* DR52050 Invalid function number         */
#define  BADUECNTA       378  /* DR52050 Invalid context area pointer    */
#define  BADUEKIND       379  /* DR52050 Invalid event kind              */
#define  BADUEDATA       380  /* DR52050 Invalid data                    */
#define  BADAPPLQSEM     381  /* DR52050 Couldn't Find/Add sess semaphore*/
#define  BUSYAPPLQSEM    382  /* DR52050 Sess semaphore is busy          */
#define  NOAPPLQSEM      383  /* DR52050 Sess semaphore not found        */
#define  APPLQSEM_NOTACTIVE  384 /* DR52050 Sess semaphore not active    */
#define  NOACTIVEREQUEST 385  /* DR59375 No active request found         */
#define  BADMODE         386  /* DR57144 var_len_fetch err in buffer mode */
#define  BADMOVEMODE     387  /* DR57144 loc_mode err in buffer mode      */
#define  BADPROCOPT      388  /* DR51742 invalid request processing optn  */
#define  NOMULTIPART     389 /* DR57583 Multipart mode not supp by server */
#define  BADRETURNOBJ    390 /* DR57583 Unrecognized Return-object option */
#define  BADCONTDATA     391 /* DR57583 Unrecognized Continued-data option */
#define  INCONTDATA      392 /* DR57583 Inconsistent Continued-data option */
#define  NOALTSUPPORT    393 /* DR61637 No APH support at the server       */
#define  QEPTDP          394 /* DR61527 NULL TDP pointer                   */
#define  NOCURPOSSUP     395 /* DR60695 Keep-response=P is not supported  */
#define  BADKEEPRESP     396 /* DR60695 Positioning requires Keep-response=P */
#define  BADCURPOS       397 /* DR60695 Specified positioning not supported */
#define  NORESPFORREPOS  398 /* DR60695 Curr resp ineligible for positioning */
#define  STMTINVALID     399 /* DR60695 Statement ineligible for positioning */

/**************************************************************/
/* Error Codes 400 - 499 are reserved for WINCLI              */
/* CLIv2 error codes continue at 500                          */
/**************************************************************/

#define  NOENCRYPTION    500 /* DR66712 Encrypt not supported/enabled on gtw */
/* <-- DR68511 */
#define  NOARRAYOPS      501 /* DR68838 Array-operations not supported. */
#define  BADARRAYOPS     502 /* DR89804 Invalid parm combo for array-ops */
#define  BADESSFLAG      503 /* DR58352 Unrecognized  statement_status val */
#define  NOESSSUP        504 /* DR58352 ESS not supported by server. */
#define  BADAPHRESP      505 /* DR68835 Unrecognized consider_APH_resps val */
#define  NOAPHRESP       506 /* DR68835 Server does not supp APH responses */
#define  BADLOGMECH      507 /* DR68842 requested logon mech not available */
#define  USREXTMAXLEN    508 /* DR68842 User exit exceeded max allowable len */
#define  LOGDATAMAXLEN   509 /* DR68842 logmech_data_len exceeded max length */
#define  ICULOADERROR    510 /* DR88745 dynamic loading of ICU library */
#define  BADEXMPTWAT     511 /* DR94622 Inv exempt_sess_from_DBCHWAT spec */
#define  BADSTATEMENTOPT 512 /* DR98026 Invalid parameter for StatementInfo */
#define  NOSTATEMENTOPT  513 /* DR98026 StatementInfo not supported */
#define  BADMAXDECIMAL   514 /* DR98024 Invalid parm for MaxDecimalPrecision */
#define  NOMAXDECIMAL    515 /* DR98024 MaxDecimalPrecision not supported */
#define  BADIDENTITYDATA 516 /* DR98025 Invalid parameter for AGKR_option  */
#define  NOIDENTITYDATA  517 /* DR98025 AGKR_option not supported */
#define  BADDEFCNX       518 /* DR101287 invalid create_default_connection */
#define  FNCLOADERROR    519 /* DR101287 can't get global symbol object*/
#define  DEFCNXSYMERROR  520 /* DR101287 can't locate FNC_Default_Connect() */
#define  DEFCNXCALLERROR 521 /* DR101287 FNC_Default_Connect() call failed */
#define  FNCUNLOADERROR  522 /* DR101287 can't free global symbol object */
#define  XSPNODATAENC    523 /* DR101287 no data enc for default connection */
#define  BADDYNRESULTSET 524 /* DR102528 Invalid parameter for DYN_result */
#define  NODYNRESULTSET  525 /* DR102528 DYN_result not supported */
#define  BADWORKLOADPTR  526 /* DR108909 Invalid Workload pointer */
#define  ICULOADLIBERR   527 /* DR107850 Unable to load tdicu libraries*/
#define  ICULOADSYMBERR  528 /* DR107850 Unable to load tdicu symbols*/
#define  ICUUNLOADERR    529 /* DR107850 Unable to unload tdicu*/
#define  TDUSRLOADERR    530 /* DR112838 TDUSR env-var vs library mismatch */
#define  BADTRANSFORMSOFF   531 /* DR116354 Invalid param for StmtInfo UDT */
#define  NOTRANSFORMSOFF    532 /* DR116354 No StmtInfo UDT support        */
#define  BADPERIODASSTRUCT  533 /* DR116354 Invalid param Period as Struct */
#define  NOPERIODASSTRUCT   534 /* DR116354 No Period as Struct support    */
#define  BADTRUSTEDSESSIONS 535 /* DR116354 Invalid param Trusted Sessions */
#define  NOTRUSTEDSESSIONS  536 /* DR116354 No Trusted Sessions support    */
#define  BADEXTOBJECTNAME   537 /* DR116354 Invalid param for EONP support */
#define  BADMULTISTATEMENTERRORS 538 /* DR121202 Invalid Stat Independence */
#define  NOMULTISTATEMENTERRORS  539 /* DR121202 No Statement Independence */
#define  BADMSGLEN          540 /* DR140617 fetched more data than expected */
#define  BADARRAYTRANSFORMSOFF 541 /* DR145224 Invalid Array Data Type     */
#define  BADXMLRESPONSEFORMAT  542 /* CLAC-28875 Invalid XML Data Type     */
#define  TDWALLETERROR        543 /* CLAC-663 Teradata Wallet Error        */
#define  TDWALLETNOTINSTALLED 544 /* CLAC-663 Teradata Wallet not installed */
#define  BADEXTENDEDLOADUSAGE 545 /* CLAC-29283 Support Extended Multiload  */
#define  BADAUTOREDRIVE       546 /* CLAC-29119 Support Automatic Redrive */
#define  BADTASMFASTFAIL      547 /* CLAC-30478 Support TASM FastFail     */
#define  NOSLOBSUPPORT        548 /* CLAC-30554 Support SLOB              */
#define  BAD1MBRESPROWS       549 /* CLAC-29115 Support 1MB Response rows */
#define  BADCONSTRFORMAT      550 /* CLIWS-7635 Invalid connection string attr */
#define  BADCONSTRVALUE       551 /* CLIWS-7635 Invalid connection string value */
#define  UNKNOWNCONSTRATTRIBUTE 552 /* CLIWS-7635 Unknown conn string attribute */
#define  BADSSLMODE           553 /* CLIWS-7901 Invalid sslmode value */
/* CLIWS-8296 --> */
#define  RACFJWTMISSINGUSERID 554 /* Userid is required in logon string for RACFJWT */
#define  RACFJWTPASSWORD      555 /* Password should be omitted in logon string for RACFJWT */
#define  IDTUSERIDUNDEF       556 /* Userid is not defined */
#define  IDTPASSTICKET        557 /* PassTicket Replay Protection is not bypassed */
#define  IDTRACROUTEFAIL      558 /* Error occurred with the RACROUTE REQUEST=VERIFY macro */
#define  IDTSMALLWORKAREA     559 /* JWT workarea length is not large enough */
#define  IDTTOKENERROR        560 /* Token is not set up correctly */
#define  IDTNOTAPFAUTH        561 /* Data set name or Task is not APF-authorized */
#define  IDTLONGUSERID        562 /* Userid is too long. Valid length is 1-8 bytes */
#define  IDTUSERIDNOTAUTH     563 /* Userid is not RACF authorized to use the ICSF API Service */
#define  IDTUSERABEND         564 /* Abnormal USER Abend condition is detected for Userid */
#define  IDTSYSTEMABEND       565 /* Abnormal SYSTEM Abend condition is detected for Userid */
#define  IDTINVALIDTOKEN      566 /* Invalid configuration for PKCS#11 Token */
#define  IDTUNEXPECTEDERROR   567 /* Unexpected error occurred from IDT module */
#define  RACFJWTERROR         568 /* Error occurred while processing RACFJWT in CLI */
/* <-- CLIWS-8296 */
#define  IDPURLNOTFOUND       569 /* CLIWS-7948 IdP metadata URL not found */
#define  AUTHENDPOINTNOTFOUND 570 /* CLIWS-7948 Authorization endpoint not found */
#define  TOKENENDPOINTNOTFOUND 571 /* CLIWS-7948 Token endpoint not found */
#define  BADHTTPRESPONSE       572 /* CLIWS-7948 Bad HTTP response code */
#define  HTTPSERVERERROR       573 /* CLIWS-7948 Error in local HTTP server */
#define  HTTPTIMEOUT           574 /* CLIWS-7948 Timeout waiting for reqeust or response */
#define  AUTHCODENOTFOUND      575 /* CLIWS-7948 Authorization code not found */
#define  JWTNOTFOUND           576 /* CLIWS-7948 JWT not found */
#define  BADHTTPREQEUST        577 /* CLIWS-7948 Bad HTTP request received */
#define  BADSSLCRCVALUE        578 /* CLIWS-8233 Bad sslcrc value  CLIWS-8614 */
#define  BROWSERCMDERROR       579 /* CLIWS-8410 Error running browser command */
/* CLIWS-8618 --> */
#define  IDTPASSTICKETERR      580 /* PassTicket Authorization failure for Userid */
#define  IDTRACROUTEERR        581 /* RACROUTE Authorization failure */
#define  IDTACEENOTFOUND       582 /* Unable to determine submitter Userid for Jobname */
#define  IDTTOKENNOTICSFDEF    583 /* PKCS#11 Token is not ICSF defined */
#define  IDTUNDEFRESOURCE      584 /* Profile resource name is not defined */
#define  IDTUNAUTHACCESS       585 /* Userid is unauthorized to access */
#define  IDTINVALIDRSAKEYLEN   586 /* PKCS#11 Token RSA key length is invalid */
/* <-- CLIWS-8618 */
#define  MISSINGJWSPRIVKEY     587 /* CLIWS-9071 LOGMECH=BEARER requires jws_private_key value in connection string*/
#define  LOADPRIVKEYERROR      588 /* CLIWS-9071 Error loading private key */
#define  UNSUPPORTEDJWSALG     589 /* CLIWS-9071 Unsupported JWS algorithm */
#define  MAKEJWTERROR          590 /* CLIWS-9071 Error creating JWT for bearer flow */
#define  PEERCERTERROR         591 /* CLIWS-9074 IdP server cert failed validation */
#define  UNKNOWNHTTPERROR      592 /* CLIWS-9074 An unknown HTTP error occurred */
#define  DEVICEAUTHERROR       593 /* CLIWS-9070 Device Authorization Grant error */
#define  DEVICETIMEOUT         594 /* CLIWS-9070 Timed out waiting for confirm user-code for Device Auth Grant */
#define  DEVICEFILEOPENERR     595 /* CLIWS-9070 File open error for writing user-code for Device Auth Grant */
#define  ACCESSDENIED          596 /* CLIWS-9070 Authorization request was denied */
#define  EXPIREDTOKEN          597 /* CLIWS-9070 The device code has expired. Try to logon again */
#define  FIPSINITERROR         598 /* CLIWS-9221 Error initializing FIPS mode */
#define  BADLOGMECHDATA        599 /* CLIWS-9380 Error in logmech data value */
#define  BADLOGMECHNAME        600 /* CLIWS-9492 Error in logmech name value */
#define  CLAIMNOTFOUND         601 /* CLIWS-9492 Claim not found in JWT */
#define  JWTERROR              602 /* CLIWS-9492 Error processing JWT */
#define  BADSSLPROTOCOL        603 /* CLIWS-9637 Bad sslprotocol value */

/* 04.02.00.00 end */
/***************************************************************/
/* constant CLI2CNT need to be updated whenever errmsg is added*/
/***************************************************************/
                     /* H3_00 - CLI2CNT changed from 52 to 58. */
                     /* HN_00 - NDM Support                    */
/* DR40973 */
#define  CLI2CNT          191 /* CLI2CNT including NDM codes   */
                              /* 71 + 3, 3 added for TDSP, DR48735 */
                              /* 74 + 1,   added for TDSP, DR50139 */
                              /* 75 + 1,   added for SSO,  DR52052 */
                              /* 76 + 1,   added for NOBUFFER,DR57420*/
                              /* 77 + 8,   added for MTCLI,DR52050*/
                              /* 85 + 1,   added for DR59375      */
                              /* 86 + 2,   added for DR57144      */
                              /* 88 + 1,   added for DR51742      */
                              /* 89 + 4,   added for DR57583      */
                              /* 93 + 1,   added for DR61637      */
                              /* 94 + 6,   added for DR61527      */
                              /*                 for DR67168      */
                              /*                 for DR60695      */
                              /*                 for DR68835      */
                              /*                 for DR58352      */
                              /* DR38016,  added PDEADLOCK     */
                              /* DR 38017, added 10 messages.  */
                              /* DR 38910, added PMTDPCLEAN.   */
                              /* DR 37947, added CANCELLOGON.  */
                              /* DR 48735, added BADSEGNUM     */
                              /* DR 48735, added BADSEGOPTS    */
                              /* DR 48735, added BADSEGMENT    */
                              /* DR 50139, added NOTDSP        */
                              /* DR 94622, added BADEXMPTWAT   */
                              /* DR 98026, added BADSTATEMENTOPT, 106 to 107 */
                              /* DR 98026, added NOSTATEMENTOPT,  107 to 108 */
                              /* DR 98024, added BADMAXDECIMAL,   108 to 109 */
                              /* DR 98024, added NOMAXDECIMAL,    109 to 110 */
                              /* DR 98025, added BADIDENTITYDATA, 110 to 111 */
                              /* DR 98025, added NOIDENTITYDATA,  111 to 112 */
                              /* DR 102528, added BADDYNRESULT, 112 to 113 */
                              /* DR 102528, added NODYNRESULT, 113 to 114 */
                              /* DR 108909, added BADWORKLOADPTR, 114 to 115 */
                              /* DR 107850, added ICULOADLIBERR, 115 to 116 */
                              /* DR 107850, added ICULOADSYMBERR, 116 to 117 */
                              /* DR 107850, added ICUUNLOADERR, 117 to 118 */
                              /* DR 116354, added EONP and other, 118 to 125 */
                              /* DR 121202, added Stat. Indep.,   125 to 127 */
                              /* DR 140617, added BADMSGLEN,      127 to 128 */
                              /* DR 145224, added Array Data Type 128 to 129 */
                              /* CLAC-28875, added XML Data Type  129 to 130 */
                              /* CLAC-663,   added WDWALLET error 130 to 132 */
                              /* CLAC-29283, added Extended ML    132 to 133 */
                              /* CLAC-29119, added Auto Redrive   133 to 134 */
                              /* CLAC-30478, added TASM FastFail  134 to 135 */
                              /* CLAC-30554, added SLOB support   135 to 136 */
                              /* CLAC-29115, added 1MB Resp Rows  136 to 137 */
                              /* CLIWS-7635, connect string errs  138 to 140 */
                              /* CLIWS-7901, invalid sslmode err  140 to 141 */
                              /* CLIWS-8295, support RACF JWT     141 to 156 */
                              /* CLIWS-7948, federated SSO        156 to 165 */
                              /* CLIWS-8233, OCSP support         165 to 166 */
                              /* CLIWS-8410, browser cmd err      166 to 167 */
                              /* CLIWS-8618, Update IDT error     167 to 174 */
                              /* CLIWS-9071, Client Credential    174 to 180 */
                              /* CLIWS-9070, Device Flow          180 to 185 */
                              /* CLIWS-9221, FIPS mode            185 to 186 */
                              /* CLIWS-9380, Logmech data         186 to 187 */
                              /* CLIWS-9492, Logmech name         187 to 188 */
                              /* CLIWS-9492, Claim not found      188 to 189 */
                              /* CLIWS-9492, JWT error            189 to 190 */
                              /* CLIWS-9637, sslprotocol error    190 to 191 */

/***************************************************************/
/* following defines  no of record offset cells for errmsg.txt */
/***************************************************************/

#define  ERROR_REX       (CLI2CNT+MTDPCNT+MOSICNT)  /* DR12820 */

/************************************************************/
/*          Following are error routines in GENERR.C        */
/************************************************************/

 /* DR52059, These routines are called in  cliv2.c          */
 Int32 CatOpen(); /* DR68829: changed return type from void */
 UInt32 CatGetMsg(char *mes, UInt16 msglen, Int16 set, Int16 MesgNo);
 void CatClose();

#endif

/***************************************************************/
/*                                                             */
/*                      END OF COPERR.H                        */
/*                                                             */
/***************************************************************/
#endif  /* COPERR_H */
