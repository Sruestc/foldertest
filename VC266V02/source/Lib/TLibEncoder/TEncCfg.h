/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2013, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TEncCfg.h
    \brief    encoder configuration class (header)
*/

#ifndef __TENCCFG__
#define __TENCCFG__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TLibCommon/CommonDef.h"
#include "TLibCommon/TComSlice.h"
#include <assert.h>

struct GOPEntry
{
  Int m_POC;
  Int m_QPOffset;
  Double m_QPFactor;
  Int m_tcOffsetDiv2;
  Int m_betaOffsetDiv2;
  Int m_temporalId;
  Bool m_refPic;
  Int m_numRefPicsActive;
  Char m_sliceType;
  Int m_numRefPics;
  Int m_referencePics[MAX_NUM_REF_PICS];
  Int m_usedByCurrPic[MAX_NUM_REF_PICS];
  Int m_interRPSPrediction;
  Int m_deltaRPS;
  Int m_numRefIdc;
  Int m_refIdc[MAX_NUM_REF_PICS+1];
  GOPEntry()
  : m_POC(-1)
  , m_QPOffset(0)
  , m_QPFactor(0)
  , m_tcOffsetDiv2(0)
  , m_betaOffsetDiv2(0)
  , m_temporalId(0)
  , m_refPic(false)
  , m_numRefPicsActive(0)
  , m_sliceType('P')
  , m_numRefPics(0)
  , m_interRPSPrediction(false)
  , m_deltaRPS(0)
  , m_numRefIdc(0)
  {
    ::memset( m_referencePics, 0, sizeof(m_referencePics) );
    ::memset( m_usedByCurrPic, 0, sizeof(m_usedByCurrPic) );
    ::memset( m_refIdc,        0, sizeof(m_refIdc) );
  }
};

std::istringstream &operator>>(std::istringstream &in, GOPEntry &entry);     //input
//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder configuration class
class TEncCfg
{
protected:
  //==== File I/O ========
  Int       m_iFrameRate;
  Int       m_FrameSkip;
  Int       m_iSourceWidth;
  Int       m_iSourceHeight;
  Window    m_conformanceWindow;
  Int       m_framesToBeEncoded;

  /* profile & level */
  Profile::Name m_profile;
  Level::Tier   m_levelTier;
  Level::Name   m_level;

  //====== Coding Structure ========
  UInt      m_uiIntraPeriod;
  UInt      m_uiDecodingRefreshType;            ///< the type of decoding refresh employed for the random access.
  Int       m_iGOPSize;
  GOPEntry  m_GOPList[MAX_GOP];
  Int       m_extraRPSs;
  Int       m_maxDecPicBuffering[MAX_TLAYER];
  Int       m_numReorderPics[MAX_TLAYER];
  
  Int       m_iQP;                              //  if (AdaptiveQP == OFF)
  
  Int       m_aiPad[2];
  

  Int       m_iMaxRefPicNum;                     ///< this is used to mimic the sliding mechanism used by the decoder
                                                 // TODO: We need to have a common sliding mechanism used by both the encoder and decoder

  Int       m_maxTempLayer;                      ///< Max temporal layer
  Bool m_useAMP;
  //======= Transform =============
  UInt      m_uiQuadtreeTULog2MaxSize;
  UInt      m_uiQuadtreeTULog2MinSize;
  UInt      m_uiQuadtreeTUMaxDepthInter;
  UInt      m_uiQuadtreeTUMaxDepthIntra;
  
  //====== Loop/Deblock Filter ========
  Bool      m_bLoopFilterDisable;
  Bool      m_loopFilterOffsetInPPS;
  Int       m_loopFilterBetaOffsetDiv2;
  Int       m_loopFilterTcOffsetDiv2;
  Bool      m_DeblockingFilterControlPresent;
  Bool      m_DeblockingFilterMetric;
  Bool      m_bUseSAO;
  Int       m_maxNumOffsetsPerPic;
  Bool      m_saoLcuBoundary;

  //====== Motion search ========
  Int       m_iFastSearch;                      //  0:Full search  1:Diamond  2:PMVFAST
  Int       m_iSearchRange;                     //  0:Full frame
  Int       m_bipredSearchRange;

  //====== Quality control ========
  Int       m_iMaxDeltaQP;                      //  Max. absolute delta QP (1:default)
  Int       m_iMaxCuDQPDepth;                   //  Max. depth for a minimum CuDQP (0:default)

  Int       m_chromaCbQpOffset;                 //  Chroma Cb QP Offset (0:default)
  Int       m_chromaCrQpOffset;                 //  Chroma Cr Qp Offset (0:default)

  //====== Tool list ========
  Bool      m_bUseHADME;
  Bool      m_useRDOQ;
  Bool      m_useRDOQTS;
  UInt      m_rdPenalty;
  Bool      m_bUseFastEnc;
  Bool      m_bUseEarlyCU;
  Bool      m_useFastDecisionForMerge;
  Bool      m_bUseCbfFastMode;
  Bool      m_useEarlySkipDetection;
  Bool      m_useTransformSkip;
  Bool      m_useTransformSkipFast;
  Int*      m_aidQP;
  
#if ALF_TEST
  Bool      m_bUseALF;
#if MQT_ALF_NPASS
  Int       m_iALFEncodePassReduction;
#endif
#endif

  Bool      m_bUseConstrainedIntraPred;
  Bool      m_usePCM;
  UInt      m_pcmLog2MaxSize;
  UInt      m_uiPCMLog2MinSize;

  Bool      m_bPCMInputBitDepthFlag;
  UInt      m_uiPCMBitDepthLuma;
  UInt      m_uiPCMBitDepthChroma;
  Bool      m_bPCMFilterDisableFlag;

  Int       m_iWaveFrontSynchro;
  Int       m_iWaveFrontSubstreams;

  Int       m_decodedPictureHashSEIEnabled;              ///< MD5(1)/disable(0) acting on decoded picture hash SEI message
  Int       m_recoveryPointSEIEnabled;

  //====== Weighted Prediction ========
  Bool      m_useWeightedPred;       //< Use of Weighting Prediction (P_SLICE)
  Bool      m_useWeightedBiPred;    //< Use of Bi-directional Weighting Prediction (B_SLICE)
  UInt      m_log2ParallelMergeLevelMinus2;       ///< Parallel merge estimation region
  UInt      m_maxNumMergeCand;                    ///< Maximum number of merge candidates
  Int       m_TMVPModeId;
  Int       m_signHideFlag;
  Bool      m_RCEnableRateControl;
  Int       m_RCTargetBitrate;
  Int       m_RCKeepHierarchicalBit;
  Bool      m_RCLCULevelRC;
  Bool      m_RCUseLCUSeparateModel;
  Int       m_RCInitialQP;
  Bool      m_RCForceIntraQP;
  Bool      m_TransquantBypassEnableFlag;                     ///< transquant_bypass_enable_flag setting in PPS.
  Bool      m_CUTransquantBypassFlagForce;                    ///< if transquant_bypass_enable_flag, then, if true, all CU transquant bypass flags will be set to true.
  TComVPS   m_cVPS;
  Bool      m_recalculateQPAccordingToLambda;                 ///< recalculate QP value according to the lambda value
  Bool      m_useStrongIntraSmoothing;                        ///< enable the use of strong intra smoothing (bi_linear interpolation) for 32x32 blocks when reference samples are flat.

#if _Cal_SSIM_
  Bool      m_printSSIM;                                      ///< pintout SSIM/MS-SSIM
#endif

public:
  TEncCfg()
  {}

  virtual ~TEncCfg()
  {}
  
  Void setProfile(Profile::Name profile) { m_profile = profile; }
  Void setLevel(Level::Tier tier, Level::Name level) { m_levelTier = tier; m_level = level; }

  Void      setFrameRate                    ( Int   i )      { m_iFrameRate = i; }
  Void      setFrameSkip                    ( UInt i ) { m_FrameSkip = i; }
  Void      setSourceWidth                  ( Int   i )      { m_iSourceWidth = i; }
  Void      setSourceHeight                 ( Int   i )      { m_iSourceHeight = i; }

  Window   &getConformanceWindow()                           { return m_conformanceWindow; }
  Void      setConformanceWindow (Int confLeft, Int confRight, Int confTop, Int confBottom ) { m_conformanceWindow.setWindow (confLeft, confRight, confTop, confBottom); }

  Void      setFramesToBeEncoded            ( Int   i )      { m_framesToBeEncoded = i; }
  
  //====== Coding Structure ========
  Void      setIntraPeriod                  ( Int   i )      { m_uiIntraPeriod = (UInt)i; }
  Void      setDecodingRefreshType          ( Int   i )      { m_uiDecodingRefreshType = (UInt)i; }
  Void      setGOPSize                      ( Int   i )      { m_iGOPSize = i; }
  Void      setGopList                      ( GOPEntry*  GOPList ) {  for ( Int i = 0; i < MAX_GOP; i++ ) m_GOPList[i] = GOPList[i]; }
  Void      setExtraRPSs                    ( Int   i )      { m_extraRPSs = i; }
  GOPEntry  getGOPEntry                     ( Int   i )      { return m_GOPList[i]; }
  Void      setMaxDecPicBuffering           ( UInt u, UInt tlayer ) { m_maxDecPicBuffering[tlayer] = u;    }
  Void      setNumReorderPics               ( Int  i, UInt tlayer ) { m_numReorderPics[tlayer] = i;    }
  
  Void      setQP                           ( Int   i )      { m_iQP = i; }
  
  Void      setPad                          ( Int*  iPad                   )      { for ( Int i = 0; i < 2; i++ ) m_aiPad[i] = iPad[i]; }
  
  Int       getMaxRefPicNum                 ()                              { return m_iMaxRefPicNum;           }
  Void      setMaxRefPicNum                 ( Int iMaxRefPicNum )           { m_iMaxRefPicNum = iMaxRefPicNum;  }

  Int       getMaxTempLayer                 ()                              { return m_maxTempLayer;              } 
  Void      setMaxTempLayer                 ( Int maxTempLayer )            { m_maxTempLayer = maxTempLayer;      }
  //======== Transform =============
  Void      setQuadtreeTULog2MaxSize        ( UInt  u )      { m_uiQuadtreeTULog2MaxSize = u; }
  Void      setQuadtreeTULog2MinSize        ( UInt  u )      { m_uiQuadtreeTULog2MinSize = u; }
  Void      setQuadtreeTUMaxDepthInter      ( UInt  u )      { m_uiQuadtreeTUMaxDepthInter = u; }
  Void      setQuadtreeTUMaxDepthIntra      ( UInt  u )      { m_uiQuadtreeTUMaxDepthIntra = u; }
  
  Void setUseAMP( Bool b ) { m_useAMP = b; }
  
  //====== Loop/Deblock Filter ========
  Void      setLoopFilterDisable            ( Bool  b )      { m_bLoopFilterDisable       = b; }
  Void      setLoopFilterOffsetInPPS        ( Bool  b )      { m_loopFilterOffsetInPPS      = b; }
  Void      setLoopFilterBetaOffset         ( Int   i )      { m_loopFilterBetaOffsetDiv2  = i; }
  Void      setLoopFilterTcOffset           ( Int   i )      { m_loopFilterTcOffsetDiv2    = i; }
  Void      setDeblockingFilterControlPresent ( Bool b ) { m_DeblockingFilterControlPresent = b; }
  Void      setDeblockingFilterMetric       ( Bool  b )      { m_DeblockingFilterMetric = b; }

  //====== Motion search ========
  Void      setFastSearch                   ( Int   i )      { m_iFastSearch = i; }
  Void      setSearchRange                  ( Int   i )      { m_iSearchRange = i; }
  Void      setBipredSearchRange            ( Int   i )      { m_bipredSearchRange = i; }

  //====== Quality control ========
  Void      setMaxDeltaQP                   ( Int   i )      { m_iMaxDeltaQP = i; }
  Void      setMaxCuDQPDepth                ( Int   i )      { m_iMaxCuDQPDepth = i; }

  Void      setChromaCbQpOffset             ( Int   i )      { m_chromaCbQpOffset = i; }
  Void      setChromaCrQpOffset             ( Int   i )      { m_chromaCrQpOffset = i; }

  //====== Sequence ========
  Int       getFrameRate                    ()      { return  m_iFrameRate; }
  UInt      getFrameSkip                    ()      { return  m_FrameSkip; }
  Int       getSourceWidth                  ()      { return  m_iSourceWidth; }
  Int       getSourceHeight                 ()      { return  m_iSourceHeight; }
  Int       getFramesToBeEncoded            ()      { return  m_framesToBeEncoded; }

  //==== Coding Structure ========
  UInt      getIntraPeriod                  ()      { return  m_uiIntraPeriod; }
  UInt      getDecodingRefreshType          ()      { return  m_uiDecodingRefreshType; }
  Int       getGOPSize                      ()      { return  m_iGOPSize; }
  Int       getMaxDecPicBuffering           (UInt tlayer) { return m_maxDecPicBuffering[tlayer]; }
  Int       getNumReorderPics               (UInt tlayer) { return m_numReorderPics[tlayer]; }
  Int       getQP                           ()      { return  m_iQP; }
  
  Int       getPad                          ( Int i )      { assert (i < 2 );                      return  m_aiPad[i]; }
  
  //======== Transform =============
  UInt      getQuadtreeTULog2MaxSize        ()      const { return m_uiQuadtreeTULog2MaxSize; }
  UInt      getQuadtreeTULog2MinSize        ()      const { return m_uiQuadtreeTULog2MinSize; }
  UInt      getQuadtreeTUMaxDepthInter      ()      const { return m_uiQuadtreeTUMaxDepthInter; }
  UInt      getQuadtreeTUMaxDepthIntra      ()      const { return m_uiQuadtreeTUMaxDepthIntra; }
  
  //==== Loop/Deblock Filter ========
  Bool      getLoopFilterDisable            ()      { return  m_bLoopFilterDisable;       }
  Bool      getLoopFilterOffsetInPPS        ()      { return m_loopFilterOffsetInPPS; }
  Int       getLoopFilterBetaOffset         ()      { return m_loopFilterBetaOffsetDiv2; }
  Int       getLoopFilterTcOffset           ()      { return m_loopFilterTcOffsetDiv2; }
  Bool      getDeblockingFilterControlPresent()  { return  m_DeblockingFilterControlPresent; }
  Bool      getDeblockingFilterMetric       ()      { return m_DeblockingFilterMetric; }

  //==== Motion search ========
  Int       getFastSearch                   ()      { return  m_iFastSearch; }
  Int       getSearchRange                  ()      { return  m_iSearchRange; }

  //==== Quality control ========
  Int       getMaxDeltaQP                   ()      { return  m_iMaxDeltaQP; }
  Int       getMaxCuDQPDepth                ()      { return  m_iMaxCuDQPDepth; }
  
  //==== Tool list ========
  Void      setUseHADME                     ( Bool  b )     { m_bUseHADME   = b; }
  Void      setUseRDOQ                      ( Bool  b )     { m_useRDOQ    = b; }
  Void      setUseRDOQTS                    ( Bool  b )     { m_useRDOQTS  = b; }
  Void      setRDpenalty                 ( UInt  b )     { m_rdPenalty  = b; }
  Void      setUseFastEnc                   ( Bool  b )     { m_bUseFastEnc = b; }
  Void      setUseEarlyCU                   ( Bool  b )     { m_bUseEarlyCU = b; }
  Void      setUseFastDecisionForMerge      ( Bool  b )     { m_useFastDecisionForMerge = b; }
  Void      setUseCbfFastMode            ( Bool  b )     { m_bUseCbfFastMode = b; }
  Void      setUseEarlySkipDetection        ( Bool  b )     { m_useEarlySkipDetection = b; }
  Void      setUseConstrainedIntraPred      ( Bool  b )     { m_bUseConstrainedIntraPred = b; }
  Void      setPCMInputBitDepthFlag         ( Bool  b )     { m_bPCMInputBitDepthFlag = b; }
  Void      setPCMFilterDisableFlag         ( Bool  b )     {  m_bPCMFilterDisableFlag = b; }
  Void      setUsePCM                       ( Bool  b )     {  m_usePCM = b;               }
  Void      setPCMLog2MaxSize               ( UInt u )      { m_pcmLog2MaxSize = u;      }
  Void      setPCMLog2MinSize               ( UInt u )     { m_uiPCMLog2MinSize = u;      }
  Void      setdQPs                         ( Int*  p )     { m_aidQP       = p; }
  Bool      getUseHADME                     ()      { return m_bUseHADME;   }
  Bool      getUseRDOQ                      ()      { return m_useRDOQ;    }
  Bool      getUseRDOQTS                    ()      { return m_useRDOQTS;  }
  Int      getRDpenalty                  ()      { return m_rdPenalty;  }
  Bool      getUseFastEnc                   ()      { return m_bUseFastEnc; }
  Bool      getUseEarlyCU                   ()      { return m_bUseEarlyCU; }
  Bool      getUseFastDecisionForMerge      ()      { return m_useFastDecisionForMerge; }
  Bool      getUseCbfFastMode           ()      { return m_bUseCbfFastMode; }
  Bool      getUseEarlySkipDetection        ()      { return m_useEarlySkipDetection; }
  Bool      getUseConstrainedIntraPred      ()      { return m_bUseConstrainedIntraPred; }
  Bool      getPCMInputBitDepthFlag         ()      { return m_bPCMInputBitDepthFlag;   }
  Bool      getPCMFilterDisableFlag         ()      { return m_bPCMFilterDisableFlag;   } 
  Bool      getUsePCM                       ()      { return m_usePCM;                 }
  UInt      getPCMLog2MaxSize               ()      { return m_pcmLog2MaxSize;  }
  UInt      getPCMLog2MinSize               ()      { return  m_uiPCMLog2MinSize;  }

  Bool getUseTransformSkip                             ()      { return m_useTransformSkip;        }
  Void setUseTransformSkip                             ( Bool b ) { m_useTransformSkip  = b;       }
  Bool getUseTransformSkipFast                         ()      { return m_useTransformSkipFast;    }
  Void setUseTransformSkipFast                         ( Bool b ) { m_useTransformSkipFast  = b;   }
  Int*      getdQPs                         ()      { return m_aidQP;       }

#if ALF_TEST
  Bool      getUseALF()      { return m_bUseALF; }
  Void      setUseALF(Bool  b)     { m_bUseALF = b; }
#endif

  Void      setUseSAO                  (Bool bVal)     {m_bUseSAO = bVal;}
  Bool      getUseSAO                  ()              {return m_bUseSAO;}
  Void  setMaxNumOffsetsPerPic                   (Int iVal)            { m_maxNumOffsetsPerPic = iVal; }
  Int   getMaxNumOffsetsPerPic                   ()                    { return m_maxNumOffsetsPerPic; }
  Void  setSaoLcuBoundary              (Bool val)      { m_saoLcuBoundary = val; }
  Bool  getSaoLcuBoundary              ()              { return m_saoLcuBoundary; }
  Void  setWaveFrontSynchro(Int iWaveFrontSynchro)       { m_iWaveFrontSynchro = iWaveFrontSynchro; }
  Int   getWaveFrontsynchro()                            { return m_iWaveFrontSynchro; }
  Void  setWaveFrontSubstreams(Int iWaveFrontSubstreams) { m_iWaveFrontSubstreams = iWaveFrontSubstreams; }
  Int   getWaveFrontSubstreams()                         { return m_iWaveFrontSubstreams; }
  Void  setDecodedPictureHashSEIEnabled(Int b)           { m_decodedPictureHashSEIEnabled = b; }
  Int   getDecodedPictureHashSEIEnabled()                { return m_decodedPictureHashSEIEnabled; }
  Void  setRecoveryPointSEIEnabled(Int b)                { m_recoveryPointSEIEnabled = b; }
  Int   getRecoveryPointSEIEnabled()                     { return m_recoveryPointSEIEnabled; }
  Void      setUseWP               ( Bool b )    { m_useWeightedPred   = b;    }
  Void      setWPBiPred            ( Bool b )    { m_useWeightedBiPred = b;    }
  Bool      getUseWP               ()            { return m_useWeightedPred;   }
  Bool      getWPBiPred            ()            { return m_useWeightedBiPred; }
  Void      setLog2ParallelMergeLevelMinus2   ( UInt u )    { m_log2ParallelMergeLevelMinus2       = u;    }
  UInt      getLog2ParallelMergeLevelMinus2   ()            { return m_log2ParallelMergeLevelMinus2;       }
  Void      setMaxNumMergeCand                ( UInt u )    { m_maxNumMergeCand = u;      }
  UInt      getMaxNumMergeCand                ()            { return m_maxNumMergeCand;   }
  Void      setTMVPModeId ( Int  u ) { m_TMVPModeId = u;    }
  Int       getTMVPModeId ()         { return m_TMVPModeId; }
  Void      setSignHideFlag( Int signHideFlag ) { m_signHideFlag = signHideFlag; }
  Int       getSignHideFlag()                    { return m_signHideFlag; }
  Bool      getUseRateCtrl         ()              { return m_RCEnableRateControl;   }
  Void      setUseRateCtrl         ( Bool b )      { m_RCEnableRateControl = b;      }
  Int       getTargetBitrate       ()              { return m_RCTargetBitrate;       }
  Void      setTargetBitrate       ( Int bitrate ) { m_RCTargetBitrate  = bitrate;   }
  Int       getKeepHierBit         ()              { return m_RCKeepHierarchicalBit; }
  Void      setKeepHierBit         ( Int i )       { m_RCKeepHierarchicalBit = i;    }
  Bool      getLCULevelRC          ()              { return m_RCLCULevelRC; }
  Void      setLCULevelRC          ( Bool b )      { m_RCLCULevelRC = b; }
  Bool      getUseLCUSeparateModel ()              { return m_RCUseLCUSeparateModel; }
  Void      setUseLCUSeparateModel ( Bool b )      { m_RCUseLCUSeparateModel = b;    }
  Int       getInitialQP           ()              { return m_RCInitialQP;           }
  Void      setInitialQP           ( Int QP )      { m_RCInitialQP = QP;             }
  Bool      getForceIntraQP        ()              { return m_RCForceIntraQP;        }
  Void      setForceIntraQP        ( Bool b )      { m_RCForceIntraQP = b;           }
  Bool      getTransquantBypassEnableFlag()           { return m_TransquantBypassEnableFlag; }
  Void      setTransquantBypassEnableFlag(Bool flag)  { m_TransquantBypassEnableFlag = flag; }
  Bool      getCUTransquantBypassFlagForceValue()          { return m_CUTransquantBypassFlagForce; }
  Void      setCUTransquantBypassFlagForceValue(Bool flag) { m_CUTransquantBypassFlagForce = flag; }
  Void setVPS(TComVPS *p) { m_cVPS = *p; }
  TComVPS *getVPS() { return &m_cVPS; }
  Void      setUseRecalculateQPAccordingToLambda ( Bool b ) { m_recalculateQPAccordingToLambda = b;    }
  Bool      getUseRecalculateQPAccordingToLambda ()         { return m_recalculateQPAccordingToLambda; }

  Void      setUseStrongIntraSmoothing ( Bool b ) { m_useStrongIntraSmoothing = b;    }
  Bool      getUseStrongIntraSmoothing ()         { return m_useStrongIntraSmoothing; }

#if _Cal_SSIM_
  Bool      getPrintSSIM    ()             { return m_printSSIM;    }
  Void      setPrintSSIM    (Bool b)       { m_printSSIM = b;    }
#endif

};

//! \}

#endif // !defined(AFX_TENCCFG_H__6B99B797_F4DA_4E46_8E78_7656339A6C41__INCLUDED_)
