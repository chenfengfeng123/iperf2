 /*---------------------------------------------------------------
 * Copyright (c) 1999,2000,2001,2002,2003
 * The Board of Trustees of the University of Illinois
 * All Rights Reserved.
 *---------------------------------------------------------------
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software (Iperf) and associated
 * documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 *
 * Redistributions of source code must retain the above
 * copyright notice, this list of conditions and
 * the following disclaimers.
 *
 *
 * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimers in the documentation and/or other materials
 * provided with the distribution.
 *
 *
 * Neither the names of the University of Illinois, NCSA,
 * nor the names of its contributors may be used to endorse
 * or promote products derived from this Software without
 * specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE CONTIBUTORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ________________________________________________________________
 * National Laboratory for Applied Network Research
 * National Center for Supercomputing Applications
 * University of Illinois at Urbana-Champaign
 * http://www.ncsa.uiuc.edu
 * ________________________________________________________________
 *
 * Reporter.h
 * by Kevin Gibbs <kgibbs@nlanr.net>
 *
 * Since version 2.0 this handles all reporting.
 * ________________________________________________________________ */

#ifndef REPORTER_H
#define REPORTER_H

#include "headers.h"
#include "Mutex.h"
#include "histogram.h"

struct thread_Settings;
struct server_hdr;

#include "Settings.hpp"

#define NUM_REPORT_STRUCTS 5000
#define NUM_MULTI_SLOTS    5
// If the minimum latency exceeds the boundaries below
// assume the clocks are not synched and suppress the
// latency output. Units are seconds
#define UNREALISTIC_LATENCYMINMIN -1
#define UNREALISTIC_LATENCYMINMAX 60

#ifdef __cplusplus
extern "C" {
#endif

/*
 *
 * Used for end/end latency measurements
 *
 */
typedef struct TransitStats {
    double maxTransit;
    double minTransit;
    double sumTransit;
    double lastTransit;
    double meanTransit;
    double m2Transit;
    double vdTransit;
    int cntTransit;
    double totmaxTransit;
    double totminTransit;
    double totsumTransit;
    int totcntTransit;
    double totmeanTransit;
    double totm2Transit;
    double totvdTransit;
} TransitStats;

#define TCPREADBINCOUNT 8
typedef struct ReadStats {
    int cntRead;
    int totcntRead;
    int bins[TCPREADBINCOUNT];
    int totbins[TCPREADBINCOUNT];
    int binsize;
} ReadStats;

typedef struct WriteStats {
    int WriteCnt;
    int WriteErr;
    int TCPretry;
    int totWriteCnt;
    int totWriteErr;
    int totTCPretry;
    int lastTCPretry;
    int cwnd;
    int rtt;
    double meanrtt;
    int up_to_date;
} WriteStats;

typedef struct IsochStats {
    int mFPS; //frames per second
    double mMean; //variable bit rate mean
    double mVariance; //vbr variance
    int mJitterBufSize; //Server jitter buffer size, units is frames
    intmax_t slipcnt;
    intmax_t framecnt;
    intmax_t framelostcnt;
    unsigned int mBurstInterval;
    unsigned int mBurstIPG; //IPG of packets within the burst
    int frameID;
} IsochStats;

/*
 * This struct contains all important information from the sending or
 * recieving thread.
 */
#define L2UNKNOWN  0x01
#define L2LENERR   0x02
#define L2CSUMERR  0x04

typedef enum WriteErrType {
    WriteNoErr  = 0,
    WriteErrAccount,
    WriteErrFatal,
    WriteErrNoAccount,
} WriteErrType;

typedef struct L2Stats {
    intmax_t cnt;
    intmax_t unknown;
    intmax_t udpcsumerr;
    intmax_t lengtherr;
    intmax_t tot_cnt;
    intmax_t tot_unknown;
    intmax_t tot_udpcsumerr;
    intmax_t tot_lengtherr;
} L2Stats;

typedef struct ReportStruct {
    intmax_t packetID;
    intmax_t packetLen;
    struct timeval packetTime;
    struct timeval sentTime;
    int errwrite;
    int emptyreport;
    int socket;
    int l2errors;
    int l2len;
    int expected_l2len;
    struct timeval isochStartTime;
    intmax_t prevframeID;
    intmax_t frameID;
    intmax_t burstsize;
    intmax_t burstperiod;
    intmax_t remaining;
} ReportStruct;

/*
 * The type field of ReporterData is a bitmask
 * with one or more of the following
 */
#define    TRANSFER_REPORT       0x00000001
#define    SERVER_RELAY_REPORT   0x00000002
#define    SETTINGS_REPORT       0x00000004
#define    CONNECTION_REPORT     0x00000008
#define    MULTIPLE_REPORT       0x00000010
#define    BIDIR_REPORT          0x00000020
#define    TRANSFER_REPORT_READY 0x00000040

typedef union {
    ReadStats read;
    WriteStats write;
} SendReadStats;

typedef struct Transfer_Info {
    void *reserved_delay;
    int transferID;
    int groupID;
    intmax_t cntError;
    intmax_t cntOutofOrder;
    intmax_t cntDatagrams;
    intmax_t IPGcnt;
    int socket;
    TransitStats transit;
    SendReadStats sock_callstats;
    // Hopefully int64_t's
    uintmax_t TotalLen;
    double jitter;
    double startTime;
    double endTime;
    double IPGsum;
    double tripTime;
    // chars
    char   mFormat;                 // -f
    char   mEnhanced;               // -e
    u_char mTTL;                    // -T
    char   mUDP;
    char   mTCP;
    int    free;  // A  misnomer - used by summing for a traffic thread counter
    histogram_t *latency_histogram;
    L2Stats l2counts;
    IsochStats isochstats;
    char   mIsochronous;                 // -e
    TransitStats frame;
    histogram_t *framelatency_histogram;
} Transfer_Info;

typedef struct Connection_Info {
    iperf_sockaddr peer;
    Socklen_t size_peer;
    iperf_sockaddr local;
    Socklen_t size_local;
    char *peerversion;
    int l2mode;
    double connecttime;
    double txholdbacktime;
    struct timeval epochStartTime;
    int winsize;
    int winsize_requested;
    int flags;
    int flags_extend;
    char mFormat;
} Connection_Info;

typedef struct ReporterData {
    char*  mHost;                   // -c
    char*  mLocalhost;              // -B
    char*  mIfrname;
    char*  mIfrnametx;
    char*  mSSMMulticastStr;
    // int's
    int type;
    intmax_t cntError;
    intmax_t lastError;
    intmax_t cntOutofOrder;
    intmax_t lastOutofOrder;
    intmax_t cntDatagrams;
    intmax_t lastDatagrams;
    intmax_t PacketID;
    uintmax_t TotalLen;
    uintmax_t lastTotal;

    int mBufLen;                    // -l
    int mMSS;                       // -M
    int mTCPWin;                    // -w
    intmax_t mUDPRate;            // -b or -u
    RateUnits mUDPRateUnits;        // -b is either bw or pps
    /*   flags is a BitMask of old bools
        bool   mBufLenSet;              // -l
        bool   mCompat;                 // -C
        bool   mDaemon;                 // -D
        bool   mDomain;                 // -V
        bool   mFileInput;              // -F or -I
        bool   mNodelay;                // -N
        bool   mPrintMSS;               // -m
        bool   mRemoveService;          // -R
        bool   mStdin;                  // -I
        bool   mStdout;                 // -o
        bool   mSuggestWin;             // -W
        bool   mUDP;
        bool   mMode_time;*/
    int flags;
    int flags_extend;
    // enums (which should be special int's)
    ThreadMode mThreadMode;         // -s or -c
    ReportMode mode;

    // doubles
    // shorts
    unsigned short mPort;           // -p
    // structs or miscellaneous
    Transfer_Info info;
    Connection_Info connection;
    struct timeval startTime;
    struct timeval packetTime;
    struct timeval nextTime;
    struct timeval intervalTime;
    struct timeval IPGstart;
    struct timeval clientStartTime;
    IsochStats isochstats;
    double TxSyncInterval;
    unsigned int FQPacingRate;
} ReporterData;


typedef struct ReporterMutex {
    Condition await_reporter;
    int reporter_running;
} ReporterMutex;

typedef struct MultiHeader {
    int groupID;
    int threads;
    Condition multibarrier_cond;
    int multibarrier_cnt;
    Mutex refcountlock;
    int refcount;
    int sockfd;
    ReporterData report;
    void (*output_sum_handler) (struct ReporterData *stats, int final);
} MultiHeader;

typedef struct PacketRing {
  // producer and consumer
  // must be an atomic type, e.g. int
  // otherwise reads/write can be torn
  int producer;
  int consumer;
  int maxcount;
  int consumerdone;
  int awaitcounter;
  ReportStruct metapacket;

  // Use a condition variables
  // o) await_consumer - producer waits for the consumer thread to
  //    make space or end (signaled by the consumer)
  // o) awake_consumer - signal the consumer thread to to run
  //    (signaled by the producer)
  Condition await_consumer;
  Condition *awake_consumer;
  ReportStruct *data;
} PacketRing;

typedef struct ReportHeader {
    ReporterData report;
    // function pointer for per packet processing
    void (*packet_handler) (struct ReportHeader *report, ReportStruct *packet);
    void (*output_handler) (struct ReporterData *stats, ReporterData *sumstats, ReporterData *bidirstats, int final);
    void (*output_sum_handler) (struct ReporterData *stats, int final);
    void (*output_bidir_handler) (struct ReporterData *stats, int final);
    MultiHeader *multireport;
    MultiHeader *bidirreport;
    struct ReportHeader *next;
    int delaycounter; // used to detect CPU bound systems
    PacketRing *packetring;
} ReportHeader;

typedef void* (* report_connection)( Connection_Info*, int );
typedef void (* report_settings)( ReporterData* );
typedef void (* report_statistics)( Transfer_Info* );
typedef void (* report_serverstatistics)( Connection_Info*, Transfer_Info* );

MultiHeader* InitSumReport( struct thread_Settings *agent, int inID);
MultiHeader* InitBiDirReport( struct thread_Settings *agent, int inID);
void InitReport( struct thread_Settings *agent );
void InitConnectionReport( struct thread_Settings *agent );
void UpdateConnectionReport(struct thread_Settings *mSettings, ReportHeader *reporthdr);
void BarrierClient(MultiHeader *agent, int timeflag);
void PostReport(ReportHeader *agent);
void ReportPacket( ReportHeader *agent, ReportStruct *packet );
void CloseReport( ReportHeader *agent,  ReportStruct *packet);
void EndReport( ReportHeader *agent );
void FreeReport(ReportHeader *agent);
Transfer_Info* GetReport( ReportHeader *agent );
void ReportServerUDP( struct thread_Settings *agent, struct server_hdr *server );
ReportHeader *ReportSettings( struct thread_Settings *agent );
void ReportConnections( struct thread_Settings *agent );
void reporter_peerversion (struct thread_Settings *inSettings, int upper, int lower);

extern ReporterMutex reporter_state;

extern void UpdateMultiHdrRefCounter(MultiHeader *reporthdr, int val, int sockfd);

extern report_connection connection_reports[];

extern report_settings settings_reports[];

extern report_statistics statistics_reports[];

extern report_serverstatistics serverstatistics_reports[];

extern report_statistics multiple_reports[];

#define SNBUFFERSIZE 120
extern char buffer[SNBUFFERSIZE]; // Buffer for printing

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // REPORTER_H
