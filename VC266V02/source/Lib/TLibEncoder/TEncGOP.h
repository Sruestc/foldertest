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

/** \file     TEncGOP.h
    \brief    GOP encoder class (header)
*/

#ifndef __TENCGOP__
#define __TENCGOP__

#include <list>

#include <stdlib.h>

#include "TLibCommon/TComList.h"
#include "TLibCommon/TComPic.h"
#include "TLibCommon/TComBitCounter.h"
#include "TLibCommon/TComLoopFilter.h"
#include "TLibCommon/AccessUnit.h"
#include "TEncSampleAdaptiveOffset.h"
#include "TEncSlice.h"
#include "TEncEntropy.h"
#include "TEncCavlc.h"
#include "TEncSbac.h"
#include "SEIwrite.h"

#include "TEncAnalyze.h"
#include "TEncRateCtrl.h"
#include <vector>

#if ALF_TEST
#include "TEncAdaptiveLoopFilter.h"
#endif

//! \ingroup TLibEncoder
//! \{

class TEncTop;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// GOP encoder class
class TEncGOP
{
private:
  //  Data
  Bool                    m_bLongtermTestPictureHasBeenCoded;
  Bool                    m_bLongtermTestPictureHasBeenCoded2;
  UInt                    m_numLongTermRefPicSPS;
  UInt                    m_ltRefPicPocLsbSps[MAX_NUM_LONG_TERM_REF_PICS];
  Bool                    m_ltRefPicUsedByCurrPicFlag[MAX_NUM_LONG_TERM_REF_PICS];
  Int                     m_iLastIDR;
  Int                     m_iGopSize;
  Int                     m_iNumPicCoded;
  Bool                    m_bFirst;
#if ALLOW_RECOVERY_POINT_AS_RAP
  Int                     m_iLastRecoveryPicPOC;
#endif
  
  //  Access channel
  TEncTop*                m_pcEncTop;
  TEncCfg*                m_pcCfg;
  TEncSlice*              m_pcSliceEncoder;
  TComList<TComPic*>*     m_pcListPic;
  
  TEncEntropy*            m_pcEntropyCoder;
  TEncCavlc*              m_pcCavlcCoder;
  TEncSbac*               m_pcSbacCoder;
  TEncBinCABAC*           m_pcBinCABAC;
  TComLoopFilter*         m_pcLoopFilter;

  SEIWriter               m_seiWriter;

#if ALF_TEST
  // Adaptive Loop filter
  TEncAdaptiveLoopFilter* m_pcAdaptiveLoopFilter;
#endif

  //--Adaptive Loop filter
  TEncSampleAdaptiveOffset*  m_pcSAO;
  TComBitCounter*         m_pcBitCounter;
  TEncRateCtrl*           m_pcRateCtrl;
  // indicate sequence first
  Bool                    m_bSeqFirst;
  
  // clean decoding refresh
  Bool                    m_bRefreshPending;
  Int                     m_pocCRA;
  std::vector<Int>        m_storedStartCUAddrForEncodingSlice;
  NalUnitType             m_associatedIRAPType;
  Int                     m_associatedIRAPPOC;

  std::vector<Int> m_vRVM_RP;
  UInt                    m_lastBPSEI;
  UInt                    m_totalCoded;
  UInt                    m_cpbRemovalDelay;
  UInt                    m_tl0Idx;
  UInt                    m_rapIdx;

#if _Cal_SSIM_
  /* Input options */
  int             m_iWidth;
  int             m_iHeight;
  int             m_iFrames;
  int             m_iFormat;
  char            *m_sYUVnames[2];
  int             m_iWinWidth;
  int             m_iWinHeight;
  int             m_iStride;
  int             m_iMargin;

  /* original Y and distorted Y */
  Pel             *m_piPic1;
  Pel             *m_piPic2;

  /* statistics for SSIM */
  int             m_iBytesY;
  int             m_iBytesC;
  int             m_iBytesFrame;
  double          m_dCurSSIM;
  double          m_dSumSSIM;
  double          m_dSumMsSSIM;
  Pel             *m_piPicDown1; // downsample temp
  Pel             *m_piPicDown2; // downsample temp
#if DOWN_SAMPLE_LP
  Pel             *m_piPicLastScale1;
  Pel             *m_piPicLastScale2;
#endif
#endif

public:
  TEncGOP();
  virtual ~TEncGOP();
  
  Void  create      ();
  Void  destroy     ();
  
  Void  init        ( TEncTop* pcTEncTop );
  Void  compressGOP ( Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRec, std::list<AccessUnit>& accessUnitsInGOP );
  Void  xAttachSliceDataToNalUnit (OutputNALUnit& rNalu, TComOutputBitstream*& rpcBitstreamRedirect);

  
  Int   getGOPSize()          { return  m_iGopSize;  }
  
  TComList<TComPic*>*   getListPic()      { return m_pcListPic; }
  
  Void  printOutSummary      ( UInt uiNumAllPicCoded );
  Void  preLoopFilterPicAll  ( TComPic* pcPic, UInt64& ruiDist, UInt64& ruiBits );
  
  TEncSlice*  getSliceEncoder()   { return m_pcSliceEncoder; }
  NalUnitType getNalUnitType( Int pocCurr, Int lastIdr );
  Void arrangeLongtermPicturesInRPS(TComSlice *, TComList<TComPic*>& );

#if _Cal_SSIM_
  void    calSSIM         ( int iScale, double radStruct[] );
  double  scale           ( int iScale );
#endif

protected:
  TEncRateCtrl* getRateCtrl()       { return m_pcRateCtrl;  }

protected:
  
  Void  xInitGOP          ( Int iPOC, Int iNumPicRcvd, TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRecOut );
  Void  xGetBuffer        ( TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRecOut, Int iNumPicRcvd, Int iTimeOffset, TComPic*& rpcPic, TComPicYuv*& rpcPicYuvRecOut, Int pocCurr );
  
  Void  xCalculateAddPSNR ( TComPic* pcPic, TComPicYuv* pcPicD, const AccessUnit&, Double dEncTime );
  
  UInt64 xFindDistortionFrame (TComPicYuv* pcPic0, TComPicYuv* pcPic1);

  Double xCalculateRVM();

  Void xCreateLeadingSEIMessages (/*SEIMessages seiMessages,*/ AccessUnit &accessUnit, TComSPS *sps);
  Int xGetFirstSeiLocation (AccessUnit &accessUnit);
  Void dblMetric( TComPic* pcPic, UInt uiNumSlices );
};// END CLASS DEFINITION TEncGOP

// ====================================================================================================================
// Enumeration
// ====================================================================================================================
enum PROCESSING_STATE
{
  EXECUTE_INLOOPFILTER,
  ENCODE_SLICE
};

enum SCALING_LIST_PARAMETER
{
  SCALING_LIST_OFF,
  SCALING_LIST_DEFAULT,
  SCALING_LIST_FILE_READ
};

//! \}

#endif // __TENCGOP__

