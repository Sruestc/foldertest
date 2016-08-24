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

/** \file     TAppEncCfg.h
    \brief    Handle encoder configuration parameters (header)
*/

#ifndef __TAPPENCCFG__
#define __TAPPENCCFG__

#include "TLibCommon/CommonDef.h"

#include "TLibEncoder/TEncCfg.h"
#include <sstream>
#include <vector>
//! \ingroup TAppEncoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder configuration class
class TAppEncCfg
{
protected:
  // file I/O
  Char*     m_pchInputFile;                                   ///< source file name
  Char*     m_pchBitstreamFile;                               ///< output bitstream file
  Char*     m_pchReconFile;                                   ///< output reconstruction file
  // source specification
  Int       m_iFrameRate;                                     ///< source frame-rates (Hz)
  UInt      m_FrameSkip;                                      ///< number of skipped frames from the beginning
  Int       m_iSourceWidth;                                   ///< source width in pixel
  Int       m_iSourceHeight;                                  ///< source height in pixel
  Int       m_iSourceHeightOrg;                               ///< original source height in pixel (when interlaced = frame height)
  Int       m_conformanceWindowMode;
  Int       m_confWinLeft;
  Int       m_confWinRight;
  Int       m_confWinTop;
  Int       m_confWinBottom;

  Int       m_framesToBeEncoded;                              ///< number of encoded frames
  Int       m_aiPad[2];                                       ///< number of padded pixels for width and height
  
  // profile/level
  Profile::Name m_profile;
  Level::Tier   m_levelTier;
  Level::Name   m_level;
  
  // coding structure
  Int       m_iIntraPeriod;                                   ///< period of I-slice (random access period)
  Int       m_iDecodingRefreshType;                           ///< random access type
  Int       m_iGOPSize;                                       ///< GOP size of hierarchical structure
  Int       m_extraRPSs;                                      ///< extra RPSs added to handle CRA
  GOPEntry  m_GOPList[MAX_GOP];                               ///< the coding structure entries from the config file
  Int       m_numReorderPics[MAX_TLAYER];                     ///< total number of reorder pictures
  Int       m_maxDecPicBuffering[MAX_TLAYER];                 ///< total number of pictures in the decoded picture buffer
  Bool      m_useTransformSkip;                               ///< flag for enabling intra transform skipping
  Bool      m_useTransformSkipFast;                           ///< flag for enabling fast intra transform skipping
  Bool      m_enableAMP;
  // coding quality
  Double    m_fQP;                                            ///< QP value of key-picture (floating point)
  Int       m_iQP;                                            ///< QP value of key-picture (integer)
  Char*     m_pchdQPFile;                                     ///< QP offset for each slice (initialized from external file)
  Int*      m_aidQP;                                          ///< array of slice QP values
  Int       m_iMaxDeltaQP;                                    ///< max. |delta QP|
  Int       m_iMaxCuDQPDepth;                                 ///< Max. depth for a minimum CuDQPSize (0:default)

  Int       m_cbQpOffset;                                     ///< Chroma Cb QP Offset (0:default) 
  Int       m_crQpOffset;                                     ///< Chroma Cr QP Offset (0:default)

  Int       m_maxTempLayer;                                  ///< Max temporal layer

  // coding unit (CU) definition
  UInt      m_uiMaxCUWidth;                                   ///< max. CU width in pixel
  UInt      m_uiMaxCUHeight;                                  ///< max. CU height in pixel
  UInt      m_uiMaxCUDepth;                                   ///< max. CU depth
  
  // transfom unit (TU) definition
  UInt      m_uiQuadtreeTULog2MaxSize;
  UInt      m_uiQuadtreeTULog2MinSize;
  
  UInt      m_uiQuadtreeTUMaxDepthInter;
  UInt      m_uiQuadtreeTUMaxDepthIntra;
  
  // coding tools (bit-depth)
  Int       m_inputBitDepthY;                               ///< bit-depth of input file (luma component)
  Int       m_inputBitDepthC;                               ///< bit-depth of input file (chroma component)
  Int       m_outputBitDepthY;                              ///< bit-depth of output file (luma component)
  Int       m_outputBitDepthC;                              ///< bit-depth of output file (chroma component)
  Int       m_internalBitDepthY;                            ///< bit-depth codec operates at in luma (input/output files will be converted)
  Int       m_internalBitDepthC;                            ///< bit-depth codec operates at in chroma (input/output files will be converted)

  // coding tools (PCM bit-depth)
  Bool      m_bPCMInputBitDepthFlag;                          ///< 0: PCM bit-depth is internal bit-depth. 1: PCM bit-depth is input bit-depth.

  // coding tool (lossless)
  Bool      m_bUseSAO; 
  Int       m_maxNumOffsetsPerPic;                            ///< SAO maximun number of offset per picture
  Bool      m_saoLcuBoundary;                                 ///< SAO parameter estimation using non-deblocked pixels for LCU bottom and right boundary areas

  // coding tools (loop filter)

#if ALF_TEST
  Bool      m_bUseALF;
#ifdef MQT_ALF_NPASS
  Int       m_iALFEncodePassReduction;                        ///< ALF encoding pass, 0 = original 16-pass, 1 = 1-pass, 2 = 2-pass
#endif
#endif

  Bool      m_bLoopFilterDisable;                             ///< flag for using deblocking filter
  Bool      m_loopFilterOffsetInPPS;                         ///< offset for deblocking filter in 0 = slice header, 1 = PPS
  Int       m_loopFilterBetaOffsetDiv2;                     ///< beta offset for deblocking filter
  Int       m_loopFilterTcOffsetDiv2;                       ///< tc offset for deblocking filter
  Bool      m_DeblockingFilterControlPresent;                 ///< deblocking filter control present flag in PPS
  Bool      m_DeblockingFilterMetric;                         ///< blockiness metric in encoder

  // coding tools (PCM)
  Bool      m_usePCM;                                         ///< flag for using IPCM
  UInt      m_pcmLog2MaxSize;                                 ///< log2 of maximum PCM block size
  UInt      m_uiPCMLog2MinSize;                               ///< log2 of minimum PCM block size
  Bool      m_bPCMFilterDisableFlag;                          ///< PCM filter disable flag

  // coding tools (encoder-only parameters)
  Bool      m_bUseHADME;                                      ///< flag for using HAD in sub-pel ME
  Bool      m_useRDOQ;                                       ///< flag for using RD optimized quantization
  Bool      m_useRDOQTS;                                     ///< flag for using RD optimized quantization for transform skip
  Int      m_rdPenalty;                                      ///< RD-penalty for 32x32 TU for intra in non-intra slices (0: no RD-penalty, 1: RD-penalty, 2: maximum RD-penalty)
  Int       m_iFastSearch;                                    ///< ME mode, 0 = full, 1 = diamond, 2 = PMVFAST
  Int       m_iSearchRange;                                   ///< ME search range
  Int       m_bipredSearchRange;                              ///< ME search range for bipred refinement
  Bool      m_bUseFastEnc;                                    ///< flag for using fast encoder setting
  Bool      m_bUseEarlyCU;                                    ///< flag for using Early CU setting
  Bool      m_useFastDecisionForMerge;                        ///< flag for using Fast Decision Merge RD-Cost 
  Bool      m_bUseCbfFastMode;                              ///< flag for using Cbf Fast PU Mode Decision
  Bool      m_useEarlySkipDetection;                         ///< flag for using Early SKIP Detection

  Int       m_iWaveFrontSynchro; //< 0: no WPP. >= 1: WPP is enabled, the "Top right" from which inheritance occurs is this LCU offset in the line above the current.
  Int       m_iWaveFrontSubstreams; //< If iWaveFrontSynchro, this is the number of substreams per frame (dependent tiles) or per tile (independent tiles).

  Bool      m_bUseConstrainedIntraPred;                       ///< flag for using constrained intra prediction
  
  Int       m_decodedPictureHashSEIEnabled;                    ///< MD5(1)/disable(0) acting on decoded picture hash SEI message
  Int       m_recoveryPointSEIEnabled;

  // weighted prediction
  Bool      m_useWeightedPred;                    ///< Use of weighted prediction in P slices
  Bool      m_useWeightedBiPred;                  ///< Use of bi-directional weighted prediction in B slices
  
  UInt      m_log2ParallelMergeLevel;                         ///< Parallel merge estimation region
  UInt      m_maxNumMergeCand;                                ///< Max number of merge candidates

  Int       m_TMVPModeId;
  Int       m_signHideFlag;
  Bool      m_RCEnableRateControl;                ///< enable rate control or not
  Int       m_RCTargetBitrate;                    ///< target bitrate when rate control is enabled
  Int       m_RCKeepHierarchicalBit;              ///< 0: equal bit allocation; 1: fixed ratio bit allocation; 2: adaptive ratio bit allocation
  Bool      m_RCLCULevelRC;                       ///< true: LCU level rate control; false: picture level rate control
  Bool      m_RCUseLCUSeparateModel;              ///< use separate R-lambda model at LCU level
  Int       m_RCInitialQP;                        ///< inital QP for rate control
  Bool      m_RCForceIntraQP;                     ///< force all intra picture to use initial QP or not

  Bool      m_TransquantBypassEnableFlag;                     ///< transquant_bypass_enable_flag setting in PPS.
  Bool      m_CUTransquantBypassFlagForce;                    ///< if transquant_bypass_enable_flag, then, if true, all CU transquant bypass flags will be set to true.

  Bool      m_recalculateQPAccordingToLambda;                 ///< recalculate QP value according to the lambda value
  Bool      m_useStrongIntraSmoothing;                        ///< enable strong intra smoothing for 32x32 blocks where the reference samples are flat

#if _Cal_SSIM_
  Bool      m_printSSIM;                                      ///< printout SSIM/MS-SSIM
#endif

  // internal member functions
  Void  xSetGlobal      ();                                   ///< set global variables
  Void  xCheckParameter ();                                   ///< check validity of configuration values
  Void  xPrintParameter ();                                   ///< print configuration values
  Void  xPrintUsage     ();                                   ///< print usage

public:
  TAppEncCfg();
  virtual ~TAppEncCfg();
  
public:
  Void  create    ();                                         ///< create option handling class
  Void  destroy   ();                                         ///< destroy option handling class
  Bool  parseCfg  ( Int argc, Char* argv[] );                 ///< parse configuration file to fill member variables
  
};// END CLASS DEFINITION TAppEncCfg

//! \}

#endif // __TAPPENCCFG__

