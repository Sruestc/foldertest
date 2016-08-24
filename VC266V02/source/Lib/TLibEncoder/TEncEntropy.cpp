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

/** \file     TEncEntropy.cpp
    \brief    entropy encoder class
*/

#include "TEncEntropy.h"
#include "TLibCommon/TypeDef.h"
#include "TLibCommon/TComSampleAdaptiveOffset.h"
#if ALF_TEST
#include <math.h>
#include "../TLibCommon/TComAdaptiveLoopFilter.h"
#endif

//! \ingroup TLibEncoder
//! \{

#if ALF_TEST
Void TEncEntropy::codeFiltCountBit(ALFParam* pAlfParam, Int64* ruiRate)
{
  resetEntropy();
  resetBits();
  codeFilt(pAlfParam);
  *ruiRate = getNumberOfWrittenBits();
  resetEntropy();
  resetBits();
}

Void TEncEntropy::codeAuxCountBit(ALFParam* pAlfParam, Int64* ruiRate)
{
  resetEntropy();
  resetBits();
  codeAux(pAlfParam);
  *ruiRate = getNumberOfWrittenBits();
  resetEntropy();
  resetBits();
}

Int TEncEntropy::writeFilterCodingParams(int minKStart, int maxScanVal, int kMinTab[])
{
  int scanPos;
  int golombIndexBit;
  int kMin;

  // Golomb parameters
  m_pcEntropyCoderIf->codeAlfUvlc(minKStart - 1);

  kMin = minKStart;
  for (scanPos = 0; scanPos < maxScanVal; scanPos++)
  {
    golombIndexBit = (kMinTab[scanPos] != kMin) ? 1 : 0;

    assert(kMinTab[scanPos] <= kMin + 1);

    m_pcEntropyCoderIf->codeAlfFlag(golombIndexBit);
    kMin = kMinTab[scanPos];
  }

  return 0;
}

Int TEncEntropy::writeFilterCoeffs(int sqrFiltLength, int filters_per_group, int pDepthInt[],
  int **FilterCoeff, int kMinTab[])
{
  int ind, scanPos, i;

  for (ind = 0; ind < filters_per_group; ++ind)
  {
    for (i = 0; i < sqrFiltLength; i++)
    {
      scanPos = pDepthInt[i] - 1;
      golombEncode(FilterCoeff[ind][i], kMinTab[scanPos]);
    }
  }
  return 0;
}

Int TEncEntropy::golombEncode(int coeff, int k)
{
  int q, i, m;
  int symbol = abs(coeff);

  m = (int)pow(2.0, k);
  q = symbol / m;

  for (i = 0; i < q; i++)
    m_pcEntropyCoderIf->codeAlfFlag(1);
  m_pcEntropyCoderIf->codeAlfFlag(0);
  // write one zero

  for (i = 0; i < k; i++)
  {
    m_pcEntropyCoderIf->codeAlfFlag(symbol & 0x01);
    symbol >>= 1;
  }

  if (coeff != 0)
  {
    int sign = (coeff > 0) ? 1 : 0;
    m_pcEntropyCoderIf->codeAlfFlag(sign);
  }
  return 0;
}

Int TEncEntropy::lengthGolomb(int coeffVal, int k)
{
  int m = 2 << (k - 1);
  int q = coeffVal / m;
  if (coeffVal != 0)
    return(q + 2 + k);
  else
    return(q + 1 + k);
}

Int TEncEntropy::codeFilterCoeff(ALFParam* ALFp)
{
  int filters_per_group = ALFp->filters_per_group_diff;
  int sqrFiltLength = ALFp->num_coeff;
  int filtNo = ALFp->realfiltNo;
  int flTab[] = { 9 / 2, 7 / 2, 5 / 2 };
  int fl = flTab[filtNo];
  int i, k, kMin, kStart, minBits, ind, scanPos, maxScanVal, coeffVal, len = 0,
    *pDepthInt = NULL, kMinTab[MAX_SQR_FILT_LENGTH], bitsCoeffScan[MAX_SCAN_VAL][MAX_EXP_GOLOMB],
    minKStart, minBitsKStart, bitsKStart;

  pDepthInt = pDepthIntTab[fl - 2];

  maxScanVal = 0;
  for (i = 0; i < sqrFiltLength; i++)
    maxScanVal = max(maxScanVal, pDepthInt[i]);

  // vlc for all
  memset(bitsCoeffScan, 0, MAX_SCAN_VAL * MAX_EXP_GOLOMB * sizeof(int));
  for (ind = 0; ind < filters_per_group; ++ind)
  {
    for (i = 0; i < sqrFiltLength; i++)
    {
      scanPos = pDepthInt[i] - 1;
      coeffVal = abs(ALFp->coeffmulti[ind][i]);
      for (k = 1; k < 15; k++)
      {
        bitsCoeffScan[scanPos][k] += lengthGolomb(coeffVal, k);
      }
    }
  }

  minBitsKStart = 0;
  minKStart = -1;
  for (k = 1; k < 8; k++)
  {
    bitsKStart = 0;
    kStart = k;
    for (scanPos = 0; scanPos < maxScanVal; scanPos++)
    {
      kMin = kStart;
      minBits = bitsCoeffScan[scanPos][kMin];

      if (bitsCoeffScan[scanPos][kStart + 1] < minBits)
      {
        kMin = kStart + 1;
        minBits = bitsCoeffScan[scanPos][kMin];
      }
      kStart = kMin;
      bitsKStart += minBits;
    }
    if ((bitsKStart < minBitsKStart) || (k == 1))
    {
      minBitsKStart = bitsKStart;
      minKStart = k;
    }
  }

  kStart = minKStart;
  for (scanPos = 0; scanPos < maxScanVal; scanPos++)
  {
    kMin = kStart;
    minBits = bitsCoeffScan[scanPos][kMin];

    if (bitsCoeffScan[scanPos][kStart + 1] < minBits)
    {
      kMin = kStart + 1;
      minBits = bitsCoeffScan[scanPos][kMin];
    }

    kMinTab[scanPos] = kMin;
    kStart = kMin;
  }

  // Coding parameters
  ALFp->minKStart = minKStart;
  ALFp->maxScanVal = maxScanVal;
  for (scanPos = 0; scanPos < maxScanVal; scanPos++)
  {
    ALFp->kMinTab[scanPos] = kMinTab[scanPos];
  }
  len += writeFilterCodingParams(minKStart, maxScanVal, kMinTab);

  // Filter coefficients
  len += writeFilterCoeffs(sqrFiltLength, filters_per_group, pDepthInt, ALFp->coeffmulti, kMinTab);

  return len;
}

Void TEncEntropy::codeAux(ALFParam* pAlfParam)
{
#if 0 //ENABLE_FORCECOEFF0
  if (pAlfParam->filtNo >= 0) m_pcEntropyCoderIf->codeAlfFlag(1);
  else m_pcEntropyCoderIf->codeAlfFlag(0);
#endif
  Int FiltTab[3] = { 9, 7, 5 };
  Int Tab = FiltTab[pAlfParam->realfiltNo];
  //  m_pcEntropyCoderIf->codeAlfUvlc(pAlfParam->realfiltNo); 

#if 1 // MQT_BA_RA  
  m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->alf_pcr_region_flag);
#endif

  m_pcEntropyCoderIf->codeAlfUvlc((Tab - 5) / 2);

  if (pAlfParam->filtNo >= 0)
  {
    if (pAlfParam->realfiltNo >= 0)
    {
      // filters_per_fr
      m_pcEntropyCoderIf->codeAlfUvlc(pAlfParam->noFilters);

      if (pAlfParam->noFilters == 1)
      {
        m_pcEntropyCoderIf->codeAlfUvlc(pAlfParam->startSecondFilter);
      }
      else if (pAlfParam->noFilters == 2)
      {
        for (int i = 1; i < NO_VAR_BINS; i++) m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->filterPattern[i]);
      }
    }
  }
}

Void TEncEntropy::codeFilt(ALFParam* pAlfParam)
{
  if (pAlfParam->filters_per_group > 1)
  {
#if 0 // ENABLE_FORCECOEFF0
    m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->forceCoeff0);
    if (pAlfParam->forceCoeff0)
    {
      for (int i = 0; i < pAlfParam->filters_per_group; i++) m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->codedVarBins[i]);
    }
#endif 
    m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->predMethod);
  }
  codeFilterCoeff(pAlfParam);
}

Void TEncEntropy::encodeAlfCtrlFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD)
{
  if (bRD)
    uiAbsPartIdx = 0;

  m_pcEntropyCoderIf->codeAlfCtrlFlag(pcCU, uiAbsPartIdx);
}

Void TEncEntropy::encodeAlfParam(ALFParam* pAlfParam)
{
  m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->alf_flag);
  if (!pAlfParam->alf_flag)
    return;
  Int pos;
  codeAux(pAlfParam);
  codeFilt(pAlfParam);

  // filter parameters for chroma
  m_pcEntropyCoderIf->codeAlfUvlc(pAlfParam->chroma_idc);
  if (pAlfParam->chroma_idc)
  {
    m_pcEntropyCoderIf->codeAlfUvlc((pAlfParam->tap_chroma - 5) / 2);

    // filter coefficients for chroma
    for (pos = 0; pos < pAlfParam->num_coeff_chroma; pos++)
    {
      m_pcEntropyCoderIf->codeAlfSvlc(pAlfParam->coeff_chroma[pos]);
    }
  }

  // region control parameters for luma
  m_pcEntropyCoderIf->codeAlfFlag(pAlfParam->cu_control_flag);
  if (pAlfParam->cu_control_flag)
  {
    assert((pAlfParam->cu_control_flag && m_pcEntropyCoderIf->getAlfCtrl()) || (!pAlfParam->cu_control_flag && !m_pcEntropyCoderIf->getAlfCtrl()));
    m_pcEntropyCoderIf->codeAlfCtrlDepth();
  }
}
#endif

#if TSB_ALF_HEADER
Void TEncEntropy::encodeAlfCtrlParam(ALFParam* pAlfParam)
{
  m_pcEntropyCoderIf->codeAlfFlagNum(pAlfParam->num_alf_cu_flag, pAlfParam->num_cus_in_frame);

  for (UInt i = 0; i < pAlfParam->num_alf_cu_flag; i++)
  {
    m_pcEntropyCoderIf->codeAlfCtrlFlag(pAlfParam->alf_cu_flag[i]);
  }
}
#endif

Void TEncEntropy::setEntropyCoder ( TEncEntropyIf* e, TComSlice* pcSlice )
{
  m_pcEntropyCoderIf = e;
  m_pcEntropyCoderIf->setSlice ( pcSlice );
}

Void TEncEntropy::encodeSliceHeader ( TComSlice* pcSlice )
{
  m_pcEntropyCoderIf->codeSliceHeader( pcSlice );
  return;
}

Void  TEncEntropy::encodeTilesWPPEntryPoint( TComSlice* pSlice )
{
  m_pcEntropyCoderIf->codeTilesWPPEntryPoint( pSlice );
}

Void TEncEntropy::encodeTerminatingBit      ( UInt uiIsLast )
{
  m_pcEntropyCoderIf->codeTerminatingBit( uiIsLast );
  
  return;
}

Void TEncEntropy::encodeSliceFinish()
{
  m_pcEntropyCoderIf->codeSliceFinish();
}

Void TEncEntropy::encodePPS( TComPPS* pcPPS )
{
  m_pcEntropyCoderIf->codePPS( pcPPS );
  return;
}

Void TEncEntropy::encodeSPS( TComSPS* pcSPS )
{
  m_pcEntropyCoderIf->codeSPS( pcSPS );
  return;
}

Void TEncEntropy::encodeCUTransquantBypassFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  m_pcEntropyCoderIf->codeCUTransquantBypassFlag( pcCU, uiAbsPartIdx );
}

Void TEncEntropy::encodeVPS( TComVPS* pcVPS )
{
  m_pcEntropyCoderIf->codeVPS( pcVPS );
  return;
}

Void TEncEntropy::encodeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if ( pcCU->getSlice()->isIntra() )
  {
    return;
  }
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  m_pcEntropyCoderIf->codeSkipFlag( pcCU, uiAbsPartIdx );
}

/** encode merge flag
 * \param pcCU
 * \param uiAbsPartIdx
 * \returns Void
 */
Void TEncEntropy::encodeMergeFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{ 
  // at least one merge candidate exists
  m_pcEntropyCoderIf->codeMergeFlag( pcCU, uiAbsPartIdx );
}

/** encode merge index
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiPUIdx
 * \param bRD
 * \returns Void
 */
Void TEncEntropy::encodeMergeIndex( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
    assert( pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N );
  }
  m_pcEntropyCoderIf->codeMergeIndex( pcCU, uiAbsPartIdx );
}

/** encode prediction mode
 * \param pcCU
 * \param uiAbsPartIdx
 * \param bRD
 * \returns Void
 */
Void TEncEntropy::encodePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  if ( pcCU->getSlice()->isIntra() )
  {
    return;
  }

  m_pcEntropyCoderIf->codePredMode( pcCU, uiAbsPartIdx );
}

// Split mode
Void TEncEntropy::encodeSplitFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  m_pcEntropyCoderIf->codeSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
}

/** encode partition size
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiDepth
 * \param bRD
 * \returns Void
 */
Void TEncEntropy::encodePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  m_pcEntropyCoderIf->codePartSize( pcCU, uiAbsPartIdx, uiDepth );
}

/** Encode I_PCM information. 
 * \param pcCU pointer to CU 
 * \param uiAbsPartIdx CU index
 * \param bRD flag indicating estimation or encoding
 * \returns Void
 */
Void TEncEntropy::encodeIPCMInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if(!pcCU->getSlice()->getSPS()->getUsePCM()
    || pcCU->getWidth(uiAbsPartIdx) > (1<<pcCU->getSlice()->getSPS()->getPCMLog2MaxSize())
    || pcCU->getWidth(uiAbsPartIdx) < (1<<pcCU->getSlice()->getSPS()->getPCMLog2MinSize()))
  {
    return;
  }
  
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  
  m_pcEntropyCoderIf->codeIPCMInfo ( pcCU, uiAbsPartIdx );
}

Void TEncEntropy::xEncodeTransform( TComDataCU* pcCU,UInt offsetLuma, UInt offsetChroma, UInt uiAbsPartIdx, UInt uiDepth, UInt width, UInt height, UInt uiTrIdx, Bool& bCodeDQP )
{
  const UInt uiSubdiv = pcCU->getTransformIdx( uiAbsPartIdx ) + pcCU->getDepth( uiAbsPartIdx ) > uiDepth;
  const UInt uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()]+2 - uiDepth;
  UInt cbfY = pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA    , uiTrIdx );
  UInt cbfU = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrIdx );
  UInt cbfV = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrIdx );

  if(uiTrIdx==0)
  {
    m_bakAbsPartIdxCU = uiAbsPartIdx;
  }
  if( uiLog2TrafoSize == 2 )
  {
    UInt partNum = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
    if( ( uiAbsPartIdx % partNum ) == 0 )
    {
      m_uiBakAbsPartIdx   = uiAbsPartIdx;
      m_uiBakChromaOffset = offsetChroma;
    }
    else if( ( uiAbsPartIdx % partNum ) == (partNum - 1) )
    {
      cbfU = pcCU->getCbf( m_uiBakAbsPartIdx, TEXT_CHROMA_U, uiTrIdx );
      cbfV = pcCU->getCbf( m_uiBakAbsPartIdx, TEXT_CHROMA_V, uiTrIdx );
    }
  }
  
  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_NxN && uiDepth == pcCU->getDepth(uiAbsPartIdx) )
  {
    assert( uiSubdiv );
  }
  else if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTER && (pcCU->getPartitionSize(uiAbsPartIdx) != SIZE_2Nx2N) && uiDepth == pcCU->getDepth(uiAbsPartIdx) &&  (pcCU->getSlice()->getSPS()->getQuadtreeTUMaxDepthInter() == 1) )
  {
    if ( uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
    {
      assert( uiSubdiv );
    }
    else
    {
      assert(!uiSubdiv );
    }
  }
  else if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    assert( uiSubdiv );
  }
  else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    assert( !uiSubdiv );
  }
  else if( uiLog2TrafoSize == pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
  {
    assert( !uiSubdiv );
  }
  else
  {
    assert( uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) );
    m_pcEntropyCoderIf->codeTransformSubdivFlag( uiSubdiv, 5 - uiLog2TrafoSize );
  }

  const UInt uiTrDepthCurr = uiDepth - pcCU->getDepth( uiAbsPartIdx );
  const Bool bFirstCbfOfCU = uiTrDepthCurr == 0;
  if( bFirstCbfOfCU || uiLog2TrafoSize > 2 )
  {
    if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr - 1 ) )
    {
      m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr );
    }
    if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr - 1 ) )
    {
      m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr );
    }
  }
  else if( uiLog2TrafoSize == 2 )
  {
    assert( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr ) == pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr - 1 ) );
    assert( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr ) == pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr - 1 ) );
  }
  
  if( uiSubdiv )
  {
    UInt size;
    width  >>= 1;
    height >>= 1;
    size = width*height;
    uiTrIdx++;
    ++uiDepth;
    const UInt partNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
    
    xEncodeTransform( pcCU, offsetLuma, offsetChroma, uiAbsPartIdx, uiDepth, width, height, uiTrIdx, bCodeDQP );

    uiAbsPartIdx += partNum;  offsetLuma += size;  offsetChroma += (size>>2);
    xEncodeTransform( pcCU, offsetLuma, offsetChroma, uiAbsPartIdx, uiDepth, width, height, uiTrIdx, bCodeDQP );

    uiAbsPartIdx += partNum;  offsetLuma += size;  offsetChroma += (size>>2);
    xEncodeTransform( pcCU, offsetLuma, offsetChroma, uiAbsPartIdx, uiDepth, width, height, uiTrIdx, bCodeDQP );

    uiAbsPartIdx += partNum;  offsetLuma += size;  offsetChroma += (size>>2);
    xEncodeTransform( pcCU, offsetLuma, offsetChroma, uiAbsPartIdx, uiDepth, width, height, uiTrIdx, bCodeDQP );
  }
  else
  {
    {
      DTRACE_CABAC_VL( g_nSymbolCounter++ );
      DTRACE_CABAC_T( "\tTrIdx: abspart=" );
      DTRACE_CABAC_V( uiAbsPartIdx );
      DTRACE_CABAC_T( "\tdepth=" );
      DTRACE_CABAC_V( uiDepth );
      DTRACE_CABAC_T( "\ttrdepth=" );
      DTRACE_CABAC_V( pcCU->getTransformIdx( uiAbsPartIdx ) );
      DTRACE_CABAC_T( "\n" );
    }
    
    if( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA && uiDepth == pcCU->getDepth( uiAbsPartIdx ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, 0 ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, 0 ) )
    {
      assert( pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, 0 ) );
      //      printf( "saved one bin! " );
    }
    else
    {
      m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, pcCU->getTransformIdx( uiAbsPartIdx ) );
    }


    if ( cbfY || cbfU || cbfV )
    {
      // dQP: only for LCU once
      if ( pcCU->getSlice()->getPPS()->getUseDQP() )
      {
        if ( bCodeDQP )
        {
          encodeQP( pcCU, m_bakAbsPartIdxCU );
          bCodeDQP = false;
        }
      }
    }
    if( cbfY )
    {
      Int trWidth = width;
      Int trHeight = height;
      m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffY()+offsetLuma), uiAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_LUMA );
    }
    if( uiLog2TrafoSize > 2 )
    {
      Int trWidth = width >> 1;
      Int trHeight = height >> 1;
      if( cbfU )
      {
        m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffCb()+offsetChroma), uiAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_U );
      }
      if( cbfV )
      {
        m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffCr()+offsetChroma), uiAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_V );
      }
    }
    else
    {
      UInt partNum = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
      if( ( uiAbsPartIdx % partNum ) == (partNum - 1) )
      {
        Int trWidth = width;
        Int trHeight = height;
        if( cbfU )
        {
          m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffCb()+m_uiBakChromaOffset), m_uiBakAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_U );
        }
        if( cbfV )
        {
          m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffCr()+m_uiBakChromaOffset), m_uiBakAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_V );
        }
      }
    }
  }
}

// Intra direction for Luma
Void TEncEntropy::encodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt absPartIdx, Bool isMultiplePU )
{
  m_pcEntropyCoderIf->codeIntraDirLumaAng( pcCU, absPartIdx , isMultiplePU);
}

// Intra direction for Chroma
Void TEncEntropy::encodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  
  m_pcEntropyCoderIf->codeIntraDirChroma( pcCU, uiAbsPartIdx );
}

Void TEncEntropy::encodePredInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  if( pcCU->isIntra( uiAbsPartIdx ) )                                 // If it is Intra mode, encode intra prediction mode.
  {
    encodeIntraDirModeLuma  ( pcCU, uiAbsPartIdx,true );
    encodeIntraDirModeChroma( pcCU, uiAbsPartIdx, bRD );
  }
  else                                                                // if it is Inter mode, encode motion vector and reference index
  {
    encodePUWise( pcCU, uiAbsPartIdx, bRD );
  }
}

/** encode motion information for every PU block
 * \param pcCU
 * \param uiAbsPartIdx
 * \param bRD
 * \returns Void
 */
Void TEncEntropy::encodePUWise( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if ( bRD )
  {
    uiAbsPartIdx = 0;
  }
  
  PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );
  UInt uiNumPU = ( ePartSize == SIZE_2Nx2N ? 1 : ( ePartSize == SIZE_NxN ? 4 : 2 ) );
  UInt uiDepth = pcCU->getDepth( uiAbsPartIdx );
  UInt uiPUOffset = ( g_auiPUOffset[UInt( ePartSize )] << ( ( pcCU->getSlice()->getSPS()->getMaxCUDepth() - uiDepth ) << 1 ) ) >> 4;

  for ( UInt uiPartIdx = 0, uiSubPartIdx = uiAbsPartIdx; uiPartIdx < uiNumPU; uiPartIdx++, uiSubPartIdx += uiPUOffset )
  {
    encodeMergeFlag( pcCU, uiSubPartIdx );
    if ( pcCU->getMergeFlag( uiSubPartIdx ) )
    {
      encodeMergeIndex( pcCU, uiSubPartIdx );
    }
    else
    {
      encodeInterDirPU( pcCU, uiSubPartIdx );
      for ( UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++ )
      {
        if ( pcCU->getSlice()->getNumRefIdx( RefPicList( uiRefListIdx ) ) > 0 )
        {
          encodeRefFrmIdxPU ( pcCU, uiSubPartIdx, RefPicList( uiRefListIdx ) );
          encodeMvdPU       ( pcCU, uiSubPartIdx, RefPicList( uiRefListIdx ) );
          encodeMVPIdxPU    ( pcCU, uiSubPartIdx, RefPicList( uiRefListIdx ) );
        }
      }
    }
  }

  return;
}

Void TEncEntropy::encodeInterDirPU( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if ( !pcCU->getSlice()->isInterB() )
  {
    return;
  }

  m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
  return;
}

/** encode reference frame index for a PU block
 * \param pcCU
 * \param uiAbsPartIdx
 * \param eRefList
 * \returns Void
 */
Void TEncEntropy::encodeRefFrmIdxPU( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  assert( !pcCU->isIntra( uiAbsPartIdx ) );
  {
    if ( ( pcCU->getSlice()->getNumRefIdx( eRefList ) == 1 ) )
    {
      return;
    }

    if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
    {
      m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
    }
  }

  return;
}

/** encode motion vector difference for a PU block
 * \param pcCU
 * \param uiAbsPartIdx
 * \param eRefList
 * \returns Void
 */
Void TEncEntropy::encodeMvdPU( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  assert( !pcCU->isIntra( uiAbsPartIdx ) );

  if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
  {
    m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
  }
  return;
}

Void TEncEntropy::encodeMVPIdxPU( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) )
  {
    m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
  }

  return;
}

Void TEncEntropy::encodeQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, eType, uiTrDepth );
}

Void TEncEntropy::encodeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx )
{
  m_pcEntropyCoderIf->codeTransformSubdivFlag( uiSymbol, uiCtx );
}

Void TEncEntropy::encodeQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  m_pcEntropyCoderIf->codeQtRootCbf( pcCU, uiAbsPartIdx );
}

Void TEncEntropy::encodeQtCbfZero( TComDataCU* pcCU, TextType eType, UInt uiTrDepth )
{
  m_pcEntropyCoderIf->codeQtCbfZero( pcCU, eType, uiTrDepth );
}
Void TEncEntropy::encodeQtRootCbfZero( TComDataCU* pcCU )
{
  m_pcEntropyCoderIf->codeQtRootCbfZero( pcCU );
}

// dQP
Void TEncEntropy::encodeQP( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  
  if ( pcCU->getSlice()->getPPS()->getUseDQP() )
  {
    m_pcEntropyCoderIf->codeDeltaQP( pcCU, uiAbsPartIdx );
  }
}


// texture
/** encode coefficients
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiDepth
 * \param uiWidth
 * \param uiHeight
 */
Void TEncEntropy::encodeCoeff( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, Bool& bCodeDQP )
{
  UInt uiMinCoeffSize = pcCU->getPic()->getMinCUWidth()*pcCU->getPic()->getMinCUHeight();
  UInt uiLumaOffset   = uiMinCoeffSize*uiAbsPartIdx;
  UInt uiChromaOffset = uiLumaOffset>>2;
    
  if( pcCU->isIntra(uiAbsPartIdx) )
  {
    DTRACE_CABAC_VL( g_nSymbolCounter++ )
    DTRACE_CABAC_T( "\tdecodeTransformIdx()\tCUDepth=" )
    DTRACE_CABAC_V( uiDepth )
    DTRACE_CABAC_T( "\n" )
  }
  else
  {
    if( !(pcCU->getMergeFlag( uiAbsPartIdx ) && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N ) )
    {
      m_pcEntropyCoderIf->codeQtRootCbf( pcCU, uiAbsPartIdx );
    }
    if ( !pcCU->getQtRootCbf( uiAbsPartIdx ) )
    {
      return;
    }
  }
  
  xEncodeTransform( pcCU, uiLumaOffset, uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth, uiHeight, 0, bCodeDQP);
}

Void TEncEntropy::encodeCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiTrWidth, UInt uiTrHeight, UInt uiDepth, TextType eType )
{
  // This is for Transform unit processing. This may be used at mode selection stage for Inter.
  m_pcEntropyCoderIf->codeCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiTrWidth, uiTrHeight, uiDepth, eType );
}

Void TEncEntropy::estimateBit (estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, TextType eTType)
{  
  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : TEXT_CHROMA;
  
  m_pcEntropyCoderIf->estBit ( pcEstBitsSbac, width, height, eTType );
}

Int TEncEntropy::countNonZeroCoeffs( TCoeff* pcCoef, UInt uiSize )
{
  Int count = 0;
  
  for ( Int i = 0; i < uiSize; i++ )
  {
    count += pcCoef[i] != 0;
  }
  
  return count;
}

/** encode quantization matrix
 * \param scalingList quantization matrix information
 */
Void TEncEntropy::encodeScalingList( TComScalingList* scalingList )
{
  m_pcEntropyCoderIf->codeScalingList( scalingList );
}

//! \}
