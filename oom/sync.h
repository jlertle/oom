//=========================================================
//  OOMidi
//  OpenOctave Midi and Audio Editor
//  $Id: sync.h,v 1.1.1.1.2.2 2009/04/01 01:37:11 terminator356 Exp $
//
//  (C) Copyright 2003 Werner Schweer (ws@seh.de)
//=========================================================

#ifndef __SYNC_H__
#define __SYNC_H__

#include "mtc.h"
#include "value.h"
#include "globaldefs.h"

class Xml;
//class MidiDevice;

//class MidiSyncPort

class MidiSyncInfo {
private:
    int _port;

    int _idOut;
    int _idIn;

    bool _sendMC;
    bool _sendMRT;
    bool _sendMMC;
    bool _sendMTC;
    bool _recMC;
    bool _recMRT;
    bool _recMMC;
    bool _recMTC;

    int _recMTCtype;

    bool _recRewOnStart;
    //bool _sendContNotStart;

    double _lastClkTime;
    double _lastTickTime;
    double _lastMRTTime;
    double _lastMMCTime;
    double _lastMTCTime;
    double _lastActTime[MIDI_CHANNELS];
    bool _clockTrig;
    bool _tickTrig;
    bool _MRTTrig;
    bool _MMCTrig;
    bool _MTCTrig;
    bool _actTrig[MIDI_CHANNELS];
    bool _clockDetect;
    bool _tickDetect;
    bool _MRTDetect;
    bool _MMCDetect;
    bool _MTCDetect;
    bool _actDetect[MIDI_CHANNELS];
    int _actDetectBits;

public:
    MidiSyncInfo();
    MidiSyncInfo & operator=(const MidiSyncInfo &sp);
    MidiSyncInfo& copyParams(const MidiSyncInfo &sp);

    int port() const {
        return _port;
    }

    void setPort(const int p) {
        _port = p;
    }

    int idOut() const {
        return _idOut;
    }

    int idIn() const {
        return _idIn;
    }

    void setIdOut(const int v) {
        _idOut = v;
    }

    void setIdIn(const int v) {
        _idIn = v;
    }

    bool MCOut() const {
        return _sendMC;
    }

    bool MRTOut() const {
        return _sendMRT;
    }

    bool MMCOut() const {
        return _sendMMC;
    }

    bool MTCOut() const {
        return _sendMTC;
    }

    bool MCIn() const {
        return _recMC;
    }

    bool MRTIn() const {
        return _recMRT;
    }

    bool MMCIn() const {
        return _recMMC;
    }

    bool MTCIn() const {
        return _recMTC;
    }

    void setMCOut(const bool v) {
        _sendMC = v;
    }

    void setMRTOut(const bool v) {
        _sendMRT = v;
    }

    void setMMCOut(const bool v) {
        _sendMMC = v;
    }

    void setMTCOut(const bool v) {
        _sendMTC = v;
    }

    void setMCIn(const bool v);
    void setMRTIn(const bool v);
    void setMMCIn(const bool v);
    void setMTCIn(const bool v);

    void setTime();

    bool recRewOnStart() const {
        return _recRewOnStart;
    }

    void setRecRewOnStart(const bool v) {
        _recRewOnStart = v;
    }
    //bool sendContNotStart() const           { return _sendContNotStart; }
    //void setSendContNotStart(const bool v)  { _sendContNotStart = v; }

    bool MCSyncDetect() const {
        return _clockDetect;
    }
    void trigMCSyncDetect();

    bool tickDetect() const {
        return _tickDetect;
    }
    void trigTickDetect();

    bool MTCDetect() const {
        return _MTCDetect;
    }
    void trigMTCDetect();

    int recMTCtype() const {
        return _recMTCtype;
    }

    void setRecMTCtype(int t) {
        _recMTCtype = t;
    }

    bool MRTDetect() const {
        return _MRTDetect;
    }
    void trigMRTDetect();

    bool MMCDetect() const {
        return _MMCDetect;
    }
    void trigMMCDetect();

    int actDetectBits() const {
        return _actDetectBits;
    }
    bool actDetect(const int ch) const;
    void trigActDetect(const int ch);

    bool isDefault() const;
    void read(Xml& xml);
    //void write(int level, Xml& xml, MidiDevice* md);
    void write(int level, Xml& xml);
};

//extern MidiSync midiSyncPorts[MIDI_PORTS];

extern bool debugSync;

//extern int rxSyncPort;
//extern int txSyncPort;
//extern int rxDeviceId;
//extern int txDeviceId;

extern int mtcType;
extern MTC mtcOffset;
extern BValue extSyncFlag;
//extern bool genMTCSync;       // output MTC Sync
//extern bool genMCSync;        // output MidiClock Sync
//extern bool genMMC;           // output Midi Machine Control
//extern bool acceptMTC;
//extern bool acceptMC;
//extern bool acceptMMC;
extern int volatile curMidiSyncInPort;
extern BValue useJackTransport;
extern bool volatile jackTransportMaster;
extern unsigned int syncSendFirstClockDelay; // In milliseconds.


#endif
