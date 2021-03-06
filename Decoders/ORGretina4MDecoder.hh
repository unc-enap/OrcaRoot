// ORGretaDecoder.hh

#ifndef _ORGretina4MDecoder_hh_
#define _ORGretina4MDecoder_hh_

#include <map>
#include "ORVDigitizerDecoder.hh"

/*
  Decoder for MJD-specific firmware for the Gretina digitizer. This firmware
  delivers a hybrid waveform with different sampling frequencies before and
  after a window surrounding the trigger on the rising edge. In addition to
  providing the 
*/ 
   

class ORGretina4MDecoder : public ORVDigitizerDecoder
{
  public:
    enum EGretina4MConsts { kEventDataLen = 1024, kEventHeaderLen = 15, kWFLen = 2018 };
    enum ECardPars { 
      kIntTime = 0, // Integration time (rise time) of trap filter
      kDownSample, // Down sampling. Use kDownSampleFactor to get multiplier
      kNCardPars
    };
    enum EChanPars { 
      kChPreSum = kNCardPars, // channel presum (n samples)
      kChPreSumDiv, // channel presum divider
      kMRPreSum, // additional presum for multi-rate mode (i.e. for baseline, flattop)
      kMRPreSumDiv, // presum divider for multi-rate mode
      kEnabled,
      kPSEnabled, 
      kPreRECnt, // number of samples before rising edge, after baseline
      kPostRECnt, // number of samples after rising edge, before flat top
      kFTCnt, // number of pre-summed flat top samples
      kNPars, 
      kNChanPars = kNPars-kNCardPars
    };
    static const UInt_t kDownSampleFactor[];
    static const UInt_t kPSFactor[];
    static const UInt_t kPSDivider[];

    ORGretina4MDecoder();
    virtual ~ORGretina4MDecoder() {}

    virtual inline std::string GetDataObjectPath() { return "ORGretina4M:Gretina4M"; }  
    virtual inline std::string GetDictionaryObjectPath() { return "ORGretina4MModel"; }  

    // Complete the set of functions satisfying the ORVDigitizerDecoder
    // interface. Gives access to the raw (hybrid) waveform; the
    // GetSamplingFrequency() response is for the underlying ADC speed. Use the
    // channel-specific interface below for unambiguous access to the "energy"
    // and "rising edge" waveforms, correcting for presumming, etc.
    virtual bool SetDataRecord(UInt_t* dataRecord);
    virtual inline double GetSamplingFrequency() { return 0.1; } // in GHz. 
    virtual inline UShort_t GetBitResolution() { return 14; }
    virtual inline size_t GetNumberOfEvents() { return (LengthOf(fDataRecord)-2)/kEventDataLen; }
    virtual inline ULong64_t GetEventTime(size_t iEvent) { return GetTimeStamp(EP(iEvent)); }
    virtual inline UInt_t GetEventEnergy(size_t iEvent) { return GetEnergy(EP(iEvent)); }
    virtual inline UShort_t GetEventChannel(size_t iEvent) { return GetChannel(EP(iEvent)); }
    virtual inline size_t GetEventWaveformLength(size_t) { return kWFLen; }
    virtual inline UInt_t GetEventWaveformPoint(size_t iEvent, size_t iSample) { return WFPS(iEvent)[iSample]; }
    virtual inline Short_t GetSignedWaveformSample(size_t iEvent, size_t iSample) { return WFPS(iEvent)[iSample]; }

    // energy waveform: pre-sum and shift every N samples of rising-edge portion
    // as necessary to get one waveform of constant sampling frequency
    //virtual double GetEnergyWFSamplingFrequency(); // in GHz. 

    // rising edge waveform: access to everything that is at the highest
    // sampling frequency
    //virtual double GetRisingEdgeWFSamplingFrequency(); // in GHz. 

    // Other available digitizer information
    virtual inline UShort_t GetBoardSerialNumber() 
      { return (fDataRecord[1] & 0xffff); }
    virtual inline Bool_t IsFifoAlmostFull()
      { return fDataRecord[1] & 0x80000000; }
    virtual inline Bool_t IsFifoHalfFull()
      { return fDataRecord[1] & 0x40000000; }

    // Functions related to setting / accessing card parameters)
    virtual void SetDecoderDictionary(const ORDecoderDictionary* dict);
    virtual UInt_t GetCardParameter(ECardPars par, UInt_t crate, UInt_t card); 
    virtual UInt_t GetChannelParameter(EChanPars par, UInt_t crate, UInt_t card, UInt_t channel); 
    virtual inline UInt_t GetEnergyNormalization()
      { return GetCardParameter(kIntTime, CrateOf(), CardOf()); }
    virtual inline UInt_t GetDownSampleFactor()
      { return kDownSampleFactor[GetCardParameter(kDownSample, CrateOf(), CardOf())]; }
    virtual inline UInt_t GetMRPreSum(size_t iEvent)
      { return kPSFactor[GetChannelParameter(kMRPreSum, CrateOf(), CardOf(), GetEventChannel(iEvent))]; }
    virtual inline UInt_t GetMRPreSumDiv(size_t iEvent)
      { return kPSDivider[GetChannelParameter(kMRPreSumDiv, CrateOf(), CardOf(), GetEventChannel(iEvent))]; }
    virtual inline UInt_t GetPreRECnt(size_t iEvent)
      { return GetChannelParameter(kPreRECnt, CrateOf(), CardOf(), GetEventChannel(iEvent)); }
    virtual inline UInt_t GetPostRECnt(size_t iEvent)
      { return GetChannelParameter(kPostRECnt, CrateOf(), CardOf(), GetEventChannel(iEvent)); }
    virtual inline UInt_t GetFTCnt(size_t iEvent)
      { return GetChannelParameter(kFTCnt, CrateOf(), CardOf(), GetEventChannel(iEvent)); }

  protected:
    // fast pointer to event data
    virtual UInt_t* EP(size_t iEvent) { return fDataRecord + 2 + iEvent*kEventDataLen; } 

    // fast pointers to waveform data
    virtual UInt_t* WFP(size_t iEvent) { return EP(iEvent)+kEventHeaderLen; }
    virtual Short_t* WFPS(size_t iEvent) { return (Short_t*) WFP(iEvent); }

    // event header decoding
    virtual ULong64_t GetTimeStamp(UInt_t* header); // time of trigger, in clock ticks
    virtual UInt_t GetEnergy(UInt_t* header);
    virtual UShort_t GetChannel(UInt_t* header) { return header[1] & 0xf; }

    // internal interface for parameters
    virtual UInt_t GetParameter(UInt_t par, UInt_t ccc);

  protected:
    std::map< UInt_t, std::map<UInt_t, UInt_t> > fCardPars;
};

#endif

