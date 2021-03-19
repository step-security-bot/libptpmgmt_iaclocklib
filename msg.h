/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* msg.h Create and parse PTP managment messages
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 * Created following "IEEE Std 1588-2008", PTP version 2
 * with some updates from "IEEE Std 1588-2019"
 */

#ifndef __MSG_H
#define __MSG_H

#include <memory>
#include <string>
#include <vector>
#include <stdint.h>
#include "cfg.h"

#ifndef INT48_MIN
#define INT48_MIN (-INT64_C(0x7fffffffffff) - 1)
#endif
#ifndef INT48_MAX
#define INT48_MAX (INT64_C(0x7fffffffffff))
#endif
#ifndef UINT48_MAX
#define UINT48_MAX (UINT64_C(0xffffffffffff))
#endif

class message;

enum MNG_PARSE_ERROR_e {
    MNG_PARSE_ERROR_OK,
    MNG_PARSE_ERROR_MSG,         /* Error message */
    MNG_PARSE_ERROR_INVALID_ID,  /* Invalid TLV mng id or action for TLV */
    MNG_PARSE_ERROR_INVALID_TLV, /* Wrong TLV header */
    MNG_PARSE_ERROR_SIZE_MISS,   /* size missmatch of field with length */
    MNG_PARSE_ERROR_TOO_SMALL,   /* buffer is too small */
    MNG_PARSE_ERROR_SIZE,        /* size is even */
    MNG_PARSE_ERROR_VAL,         /* Value is out of range or invalid */
    MNG_PARSE_ERROR_HEADER,      /* Wrong value in header */
    MNG_PARSE_ERROR_ACTION,      /* Wrong action value */
    MNG_PARSE_ERROR_UNSUPPORT,   /* Do not know how to parse the TLV data */
    MNG_PARSE_ERROR_MEM,         /* fail to allocate TLV data */
};
enum actionField_e : uint8_t {
    GET = 0,
    SET = 1,
    RESPONSE = 2,
    COMMAND = 3,
    ACKNOWLEDGE = 4,
};
#ifndef SWIG
enum mng_vals_e {
#define A(n, v, sc, a, sz, f) n,
#include "ids.h"
};
#endif
enum managementErrorId_e : uint16_t {
    RESPONSE_TOO_BIG = 0x0001,
    NO_SUCH_ID       = 0x0002,
    WRONG_LENGTH     = 0x0003,
    WRONG_VALUE      = 0x0004,
    NOT_SETABLE      = 0x0005,
    NOT_SUPPORTED    = 0x0006,
    GENERAL_ERROR    = 0xfffe,
};
enum clockType_e : uint16_t { // Bit mask!
    ordinaryClock       = 0x8000, /* ordinary clock */
    boundaryClock       = 0x4000, /* boundary clock */
    p2pTransparentClock = 0x2000, /* peer-to-peer transparent clock */
    e2eTransparentClock = 0x1000, /* end-to-end transparent clock */
    management          = 0x0800, /* management node (deprecated) */
};
enum networkProtocol_e : uint16_t {
    UDP_IPv4   = 1,
    UDP_IPv6   = 2,
    IEEE_802_3 = 3,
    DeviceNet  = 4,
    ControlNet = 5,
    PROFINET   = 6,
};
enum clockAccuracy_e : uint8_t {
    Accurate_within_1ps   = 0x17,
    Accurate_within_2_5ps = 0x18,
    Accurate_within_10ps  = 0x19,
    Accurate_within_25ps  = 0x1a,
    Accurate_within_100ps = 0x1b,
    Accurate_within_250ps = 0x1c,
    Accurate_within_1ns   = 0x1d,
    Accurate_within_2_5ns = 0x1e,
    Accurate_within_10ns  = 0x1f,
    Accurate_within_25ns  = 0x20,
    Accurate_within_100ns = 0x21,
    Accurate_within_250ns = 0x22,
    Accurate_within_1us   = 0x23,
    Accurate_within_2_5us = 0x24,
    Accurate_within_10us  = 0x25,
    Accurate_within_25us  = 0x26,
    Accurate_within_100us = 0x27,
    Accurate_within_250us = 0x28,
    Accurate_within_1ms   = 0x29,
    Accurate_within_2_5ms = 0x2a,
    Accurate_within_10ms  = 0x2b,
    Accurate_within_25ms  = 0x2c,
    Accurate_within_100ms = 0x2d,
    Accurate_within_250ms = 0x2e,
    Accurate_within_1s    = 0x2f,
    Accurate_within_10s   = 0x30,
    Accurate_more_10s     = 0x31,
    Accurate_Unknown      = 0xef,
};
enum faultRecord_e : uint8_t {
    Emergency     = 0x00, // system is unusable
    Alert         = 0x01, // immediate action needed
    Critical      = 0x02, // critical conditions
    Error         = 0x03, // error conditions
    Warning       = 0x04, // warning conditions
    Notice        = 0x05, // normal but significant condition
    Informational = 0x06, // informational messages
    Debug         = 0x07, // debug-level messages
};
enum timeSource_e : uint8_t {
    ATOMIC_CLOCK        = 0x10,
    GNSS                = 0x20,
    TERRESTRIAL_RADIO   = 0x30,
    SERIAL_TIME_CODE    = 0x39,
    PTP                 = 0x40,
    NTP                 = 0x50,
    HAND_SET            = 0x60,
    OTHER               = 0x90,
    INTERNAL_OSCILLATOR = 0xA0,
};
enum portState_e : uint8_t {
    INITIALIZING = 1,
    FAULTY       = 2,
    DISABLED     = 3,
    LISTENING    = 4,
    PRE_MASTER   = 5,
    MASTER       = 6,
    PASSIVE      = 7,
    UNCALIBRATED = 8,
    SLAVE        = 9,
};
/* linuxptp timestamp enum */
enum linuxptpTimeStamp_e : uint8_t {
    TS_SOFTWARE,
    TS_HARDWARE,
    TS_LEGACY_HW,
    TS_ONESTEP,
    TS_P2P1STEP,
};

#define CLIENT SLAVE // linuxptp terminology
#define GPS GNSS     // "IEEE Std 1588-2008" use old keyword

/* IEEE 1558 PTP data types */
struct TimeInterval_t {
    uint64_t scaledNanoseconds;
};
struct Timestamp_t {
    uint64_t secondsField; // 48 bits
    uint32_t nanosecondsField;
};
struct ClockIdentity_t {
    uint8_t v[8];
};
struct PortIdentity_t {
    ClockIdentity_t clockIdentity;
    uint16_t portNumber;
};
struct PortAddress_t {
    networkProtocol_e networkProtocol;
    uint16_t addressLength;
    std::string addressField;
};
struct ClockQuality_t {
    uint8_t clockClass;
    clockAccuracy_e clockAccuracy;
    uint16_t offsetScaledLogVariance;
};
struct PTPText_t {
    uint8_t lengthField;
    std::string textField;
};
struct FaultRecord_t {
    uint16_t faultRecordLength;
    Timestamp_t faultTime;
    faultRecord_e severityCode; // 8 bits
    PTPText_t faultName;
    PTPText_t faultValue;
    PTPText_t faultDescription;
};
struct AcceptableMaster_t {
    PortIdentity_t acceptablePortIdentity;
    uint8_t alternatePriority1;
};
struct managementId_t {
    uint16_t value;
    uint8_t scope;   // scope_t
    uint8_t allowed; // action bits from allowAction_t
    // Size of dataField (fixed length)
    // -1 tlv not supported
    // -2 calculate
    ssize_t size;
};
struct msgParams {
    uint8_t transportSpecific;
    uint8_t domainNumber;
    uint8_t boundaryHops;
    bool isUnicast;
    bool useLinuxPTPTlvs;
    PortIdentity_t target;
    PortIdentity_t self_id;
    // Use linuxptp Implementation-specific
};
struct baseData {
};

/* Structure per each mng_vals_e id */
#include "proc.h"

class message
{
#define A(n, v, sc, a, sz, f) case##f(n)
#define caseUF(n) bool n##_f(n##_t &data);
  private:
    /* Per tlv ID callback for parse or build or both */
#include "ids.h"

    mng_vals_e      m_tlv_id;
    /* Send only */
    actionField_e   m_actionField;
    size_t          m_msgLen;
    /* Parsing parameters */
    uint16_t        m_sequence;
    bool            m_isUnicast;
    baseData        *m_dataSend;
    /* Used during parsing and build */
    uint8_t        *m_cur;
    ssize_t         m_left;
    size_t          m_size; // TLV data size on build
    bool            m_build; // true on build
    MNG_PARSE_ERROR_e m_err; // Last TLV err

    size_t          m_sendBufSize;
    std::vector<uint8_t> m_sendBuf;

    msgParams       m_prms;
    PortIdentity_t  m_peer;
    std::unique_ptr<baseData> m_dataGet;

    // Used for reserved values
    uint8_t reserved;

    /* For error messages */
    uint16_t m_errorId;
    PTPText_t m_errorDisplay;
    static const managementId_t mng_all_vals[];

    bool allowedAction(mng_vals_e id, actionField_e action);
    /* Parsing functions */
    void move(size_t val) {
        m_cur += val;
        m_left -= val;
        m_size += val;
    }
    bool findTlvId(uint16_t val); // val in network order
    bool checkReplyAction(uint8_t actionField);
    bool proc(uint8_t &val);
    bool proc(uint16_t &val);
    bool proc(uint32_t &val);
    bool proc48(uint64_t &val);
    bool proc(uint64_t &val);
    bool proc(int8_t &val);
    bool proc(int16_t &val);
    bool proc(int32_t &val);
    bool proc48(int64_t &val);
    bool proc(int64_t &val);
    bool proc(std::string &str, uint16_t len);
    bool proc(uint8_t *val, size_t len);
    bool proc(networkProtocol_e &val);
    bool proc(clockAccuracy_e &val);
    bool proc(faultRecord_e &val);
    bool proc(timeSource_e &val);
    bool proc(portState_e &val);
    bool proc(linuxptpTimeStamp_e &val);
    bool proc(TimeInterval_t &v);
    bool proc(Timestamp_t &d);
    bool proc(ClockIdentity_t &v);
    bool proc(PortIdentity_t &d);
    bool proc(PortAddress_t &d);
    bool proc(ClockQuality_t &d);
    bool proc(PTPText_t &d);
    bool proc(FaultRecord_t &d);
    bool proc(AcceptableMaster_t &d);
    /* linuxptp PORT_STATS_NP statistics use little endian */
    bool procLe(uint64_t &val);
    MNG_PARSE_ERROR_e call_tlv_data();
    /*
     * dataFieldSize() for sending SET/COMMAND
     * Get dataField of current m_tlv_id
     * For id with non fixed size
     * The size is determined by the m_dataSend content
     */
    ssize_t dataFieldSize();

  public:
    message();
    message(msgParams prms);
    msgParams getParams() { return m_prms; }
    bool updateParams(msgParams prms);
    mng_vals_e getTlvId() { return m_tlv_id; } // Both send and reply
    void setAllPorts();
    bool isAllPorts();
    bool useConfig(configFile &cfg, const char *section = nullptr);
    bool useConfig(configFile &cfg, std::string &section);
    /* Convert to string */
    static const char *err2str_c(MNG_PARSE_ERROR_e err);
    static const char *mng2str_c(mng_vals_e id);
    static const char *errId2str_c(managementErrorId_e err);
    static const char *clkType2str_c(clockType_e val);
    static const char *netProt2str_c(networkProtocol_e val);
    static const char *clockAcc2str_c(clockAccuracy_e val);
    static const char *faultRec2str_c(faultRecord_e val);
    static const char *timeSrc2str_c(timeSource_e val);
    static const char *portState2str_c(portState_e val);
    static const char *pppText2str_c(PTPText_t &v) { return v.textField.c_str(); }
    static const char *ts2str_c(linuxptpTimeStamp_e val);
    static std::string c2str(const Timestamp_t &v);
    static std::string c2str(const ClockIdentity_t &v);
    static std::string c2str(const PortIdentity_t &val);
    static std::string c2str(const PortAddress_t &d);
    static std::string b2str(const uint8_t *id, size_t len);
    static std::string b2str(const std::string &id);
    static std::string ipv42str(const std::string &id);
    static std::string ipv62str(const std::string &id);
    static double getInterval(const TimeInterval_t &v) {
        return (double)v.scaledNanoseconds / 0x10000;
    }
    /* Flags functions */
    static uint8_t is_LI_61(uint8_t flags) { return flags & (1 << 0) ? 1 : 0; }
    static uint8_t is_LI_59(uint8_t flags) { return flags & (1 << 1) ? 1 : 0; }
    static uint8_t is_UTCV(uint8_t flags) { return flags & (1 << 2) ? 1 : 0; }
    static uint8_t is_PTP(uint8_t flags) { return flags & (1 << 3) ? 1 : 0; }
    static uint8_t is_TTRA(uint8_t flags) { return flags & (1 << 4) ? 1 : 0; }
    static uint8_t is_FTRA(uint8_t flags) { return flags & (1 << 5) ? 1 : 0; }
    /* for send */
    static bool isEmpty(mng_vals_e id);
    bool setAction(actionField_e actionField, mng_vals_e tlv_id);
    bool setAction(actionField_e actionField, mng_vals_e tlv_id,
        baseData &dataSend);
    MNG_PARSE_ERROR_e build(const void *buf, size_t bufSize, uint16_t sequence);
    /* send buffer will be allocated dynamiclly */
    MNG_PARSE_ERROR_e build(uint16_t sequence);
    actionField_e getAction() { return m_actionField; } // Send only
    size_t getMsgLen() { return m_msgLen; }
    /* Size based on m_tlv_id and m_dataSend, can get size before build()! */
    ssize_t getMsgPlanedLen(); // size for build only
    /* Get sennd buffer used with build */
    ssize_t getSendBufSize() { return m_sendBufSize; }
    /* Do not release this memory, message class will! */
    void *getSendBuf() { return m_sendBuf.data(); }
    /* Parsed message functions */
    MNG_PARSE_ERROR_e parse(const void *buf, ssize_t msgSize);
    bool isUnicast() { return m_isUnicast; }
    uint16_t getSequence() { return m_sequence; }
    const PortIdentity_t &getPeer() { return m_peer; }
    const baseData *getData() { return m_dataGet.get(); }
    /* Parse error */
    managementErrorId_e getErrId() { return (managementErrorId_e)m_errorId; }
    const std::string &getErrDisplay() { return m_errorDisplay.textField; }
    const char *getErrDisplay_c() { return pppText2str_c(m_errorDisplay); }
    /* Size functions */
    static size_t PortAddress_l(PortAddress_t &d);
    static size_t PTPText_l(PTPText_t &d);
    static size_t FaultRecord_l(FaultRecord_t &d);
    static const char *getVersion();
    static int getVersionMajor();
    static int getVersionMinor();
};

#endif /*__MSG_H*/
