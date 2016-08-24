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

/** \file     TDecEntropy.cpp
    \brief    entropy decoder class
*/

#include "TDecEntropy.h"
#if ALF_TEST_DECODER
#include <math.h>
#endif

//! \ingroup TLibDecoder
//! \{

Void TDecEntropy::setEntropyDecoder         ( TDecEntropyIf* p )
{
  m_pcEntropyDecoderIf = p;
}

#include "TLibCommon/TComSampleAdaptiveOffset.h"

#if ALF_TEST_DECODER
#include "TLibCommon/TComAdaptiveLoopFilter.h"
Void TDecEntropy::decodeAux(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  Int sqrFiltLengthTab[3] = { SQR_FILT_LENGTH_9SYM, SQR_FILT_LENGTH_7SYM, SQR_FILT_LENGTH_5SYM };
  Int FiltTab[3] = { 9, 7, 5 };

  pAlfParam->filters_per_group = 0;

  memset(pAlfParam->filterPattern, 0, sizeof(Int)*NO_VAR_BINS);
#if ENABLE_FORCECOEFF0
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  if (!uiSymbol) pAlfParam->filtNo = -1;
  else pAlfParam->filtNo = uiSymbol; //nonZeroCoeffs
#else
  pAlfParam->filtNo = 1; //nonZeroCoeffs
#endif

#if MQT_BA_RA
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  pAlfParam->alf_pcr_region_flag = uiSymbol;
#endif

  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
  Int TabIdx = uiSymbol;
  pAlfParam->realfiltNo = 2 - TabIdx;
  pAlfParam->tap = FiltTab[pAlfParam->realfiltNo];
#if TI_ALF_MAX_VSIZE_7
  pAlfParam->tapV = TComAdaptiveLoopFilter::ALFTapHToTapV(pAlfParam->tap);
#endif
  pAlfParam->num_coeff = sqrFiltLengthTab[pAlfParam->realfiltNo];

  if (pAlfParam->filtNo >= 0)
  {
    if (pAlfParam->realfiltNo >= 0)
    {
      // filters_per_fr
      m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
      pAlfParam->noFilters = uiSymbol + 1;
      pAlfParam->filters_per_group = pAlfParam->noFilters;

      if (pAlfParam->noFilters == 2)
      {
        m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
        pAlfParam->startSecondFilter = uiSymbol;
        pAlfParam->filterPattern[uiSymbol] = 1;
      }
      else if (pAlfParam->noFilters > 2)
      {
        pAlfParam->filters_per_group = 1;
        for (int i = 1; i < NO_VAR_BINS; i++)
        {
          m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
          pAlfParam->filterPattern[i] = uiSymbol;
          pAlfParam->filters_per_group += uiSymbol;
        }
      }
    }
  }
  else
  {
    memset(pAlfParam->filterPattern, 0, NO_VAR_BINS*sizeof(Int));
  }
  // Reconstruct varIndTab[]
  memset(pAlfParam->varIndTab, 0, NO_VAR_BINS * sizeof(int));
  if (pAlfParam->filtNo >= 0)
  {
    for (Int i = 1; i < NO_VAR_BINS; ++i)
    {
      if (pAlfParam->filterPattern[i])
        pAlfParam->varIndTab[i] = pAlfParam->varIndTab[i - 1] + 1;
      else
        pAlfParam->varIndTab[i] = pAlfParam->varIndTab[i - 1];
    }
  }
}


Void TDecEntropy::readFilterCodingParams(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  int ind, scanPos;
  int golombIndexBit;
  int kMin;
  int maxScanVal;
  int *pDepthInt;
  int fl;

  // Determine fl
  if (pAlfParam->num_coeff == SQR_FILT_LENGTH_9SYM)
    fl = 4;
  else if (pAlfParam->num_coeff == SQR_FILT_LENGTH_7SYM)
    fl = 3;
  else
    fl = 2;

  // Determine maxScanVal
  maxScanVal = 0;
  pDepthInt = pDepthIntTab[fl - 2];
  for (ind = 0; ind < pAlfParam->num_coeff; ind++)
    maxScanVal = max(maxScanVal, pDepthInt[ind]);

  // Golomb parameters
  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
  pAlfParam->minKStart = 1 + uiSymbol;

  kMin = pAlfParam->minKStart;
  for (scanPos = 0; scanPos < maxScanVal; scanPos++)
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    golombIndexBit = uiSymbol;
    if (golombIndexBit)
      pAlfParam->kMinTab[scanPos] = kMin + 1;
    else
      pAlfParam->kMinTab[scanPos] = kMin;
    kMin = pAlfParam->kMinTab[scanPos];
  }
}


Int TDecEntropy::golombDecode(Int k)
{
  UInt uiSymbol;
  Int q = -1;
  Int nr = 0;
  Int m = (Int)pow(2.0, k);
  Int a;

  uiSymbol = 1;
  while (uiSymbol)
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    q++;
  }
  for (a = 0; a < k; ++a)          // read out the sequential log2(M) bits
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    if (uiSymbol)
      nr += 1 << a;
  }
  nr += q * m;                    // add the bits and the multiple of M
  if (nr != 0)
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    nr = (uiSymbol) ? nr : -nr;
  }
  return nr;
}


Void TDecEntropy::readFilterCoeffs(ALFParam* pAlfParam)
{
  int ind, scanPos, i;
  int *pDepthInt;
  int fl;

  if (pAlfParam->num_coeff == SQR_FILT_LENGTH_9SYM)
    fl = 4;
  else if (pAlfParam->num_coeff == SQR_FILT_LENGTH_7SYM)
    fl = 3;
  else
    fl = 2;

  pDepthInt = pDepthIntTab[fl - 2];

  for (ind = 0; ind < pAlfParam->filters_per_group_diff; ++ind)
  {
    for (i = 0; i < pAlfParam->num_coeff; i++)
    {
      scanPos = pDepthInt[i] - 1;
      pAlfParam->coeffmulti[ind][i] = golombDecode(pAlfParam->kMinTab[scanPos]);
    }
  }

}
Void TDecEntropy::decodeFilterCoeff(ALFParam* pAlfParam)
{
  readFilterCodingParams(pAlfParam);
  readFilterCoeffs(pAlfParam);
}



Void TDecEntropy::decodeFilt(ALFParam* pAlfParam)
{
  UInt uiSymbol;

  if (pAlfParam->filtNo >= 0)
  {
    pAlfParam->filters_per_group_diff = pAlfParam->filters_per_group;
    if (pAlfParam->filters_per_group > 1)
    {
#if ENABLE_FORCECOEFF0
      m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
      pAlfParam->forceCoeff0 = uiSymbol;

      if (pAlfParam->forceCoeff0)
      {
        pAlfParam->filters_per_group_diff = 0;
        for (int i = 0; i < pAlfParam->filters_per_group; i++)
        {
          m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
          pAlfParam->codedVarBins[i] = uiSymbol;
          pAlfParam->filters_per_group_diff += uiSymbol;
        }
      }
      else
#else
      pAlfParam->forceCoeff0 = 0;
#endif
      {
        for (int i = 0; i < NO_VAR_BINS; i++)
          pAlfParam->codedVarBins[i] = 1;

      }
      m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
      pAlfParam->predMethod = uiSymbol;
    }
    else
    {
      pAlfParam->forceCoeff0 = 0;
      pAlfParam->predMethod = 0;
    }

    decodeFilterCoeff(pAlfParam);
  }
}

Void TDecEntropy::decodeAlfParam(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  Int iSymbol;
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  pAlfParam->alf_flag = uiSymbol;

  if (!pAlfParam->alf_flag)
  {
    m_pcEntropyDecoderIf->setAlfCtrl(false);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(0); //unncessary
    return;
  }

  Int pos;
  decodeAux(pAlfParam);
  decodeFilt(pAlfParam);
  // filter parameters for chroma
  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
  pAlfParam->chroma_idc = uiSymbol;

  if (pAlfParam->chroma_idc)
  {
    m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
    pAlfParam->tap_chroma = (uiSymbol << 1) + 5;
    pAlfParam->num_coeff_chroma = ((pAlfParam->tap_chroma*pAlfParam->tap_chroma + 1) >> 1) + 1;

    // filter coefficients for chroma
    for (pos = 0; pos < pAlfParam->num_coeff_chroma; pos++)
    {
      m_pcEntropyDecoderIf->parseAlfSvlc(iSymbol);
      pAlfParam->coeff_chroma[pos] = iSymbol;
    }
  }

  // region control parameters for luma
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  pAlfParam->cu_control_flag = uiSymbol;
  if (pAlfParam->cu_control_flag)
  {
    m_pcEntropyDecoderIf->setAlfCtrl(true);
    m_pcEntropyDecoderIf->parseAlfCtrlDepth(uiSymbol);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(uiSymbol);
#if TSB_ALF_HEADER
    pAlfParam->alf_max_depth = uiSymbol;
    decodeAlfCtrlParam(pAlfParam);
#endif
  }
  else
  {
    m_pcEntropyDecoderIf->setAlfCtrl(false);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(0); //unncessary
  }
}

#if TSB_ALF_HEADER
Void TDecEntropy::decodeAlfCtrlParam(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  m_pcEntropyDecoderIf->parseAlfFlagNum(uiSymbol, pAlfParam->num_cus_in_frame, pAlfParam->alf_max_depth);
  pAlfParam->num_alf_cu_flag = uiSymbol;

  for (UInt i = 0; i < pAlfParam->num_alf_cu_flag; i++)
  {
    m_pcEntropyDecoderIf->parseAlfCtrlFlag(pAlfParam->alf_cu_flag[i]);
  }
}
#endif

Void TDecEntropy::decodeAlfCtrlFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
  m_pcEntropyDecoderIf->parseAlfCtrlFlag(pcCU, uiAbsPartIdx, uiDepth);
}
#endif

Void TDecEntropy::decodeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseSkipFlag( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodeCUTransquantBypassFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseCUTransquantBypassFlag( pcCU, uiAbsPartIdx, uiDepth );
}

/** decode merge flag
 * \param pcSubCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \param uiPUIdx 
 * \returns Void
 */
Void TDecEntropy::decodeMergeFlag( TComDataCU* pcSubCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx )
{ 
  // at least one merge candidate exists
  m_pcEntropyDecoderIf->parseMergeFlag( pcSubCU, uiAbsPartIdx, uiDepth, uiPUIdx );
}

/** decode merge index
 * \param pcCU
 * \param uiPartIdx 
 * \param uiAbsPartIdx 
 * \param puhInterDirNeighbours pointer to list of inter direction from the casual neighbours
 * \param pcMvFieldNeighbours pointer to list of motion vector field from the casual neighbours
 * \param uiDepth
 * \returns Void
 */
Void TDecEntropy::decodeMergeIndex( TComDataCU* pcCU, UInt uiPartIdx, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiMergeIndex = 0;
  m_pcEntropyDecoderIf->parseMergeIndex( pcCU, uiMergeIndex );
  pcCU->setMergeIndexSubParts( uiMergeIndex, uiAbsPartIdx, uiPartIdx, uiDepth );
}

Void TDecEntropy::decodeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parsePredMode( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parsePartSize( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodePredInfo    ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU )
{
  if( pcCU->isIntra( uiAbsPartIdx ) )                                 // If it is Intra mode, encode intra prediction mode.
  {
    decodeIntraDirModeLuma  ( pcCU, uiAbsPartIdx, uiDepth );
    decodeIntraDirModeChroma( pcCU, uiAbsPartIdx, uiDepth );
  }
  else                                                                // if it is Inter mode, encode motion vector and reference index
  {
    decodePUWise( pcCU, uiAbsPartIdx, uiDepth, pcSubCU );
  }
}

/** Parse I_PCM information. 
 * \param pcCU  pointer to CUpointer to CU
 * \param uiAbsPartIdx CU index
 * \param uiDepth CU depth
 * \returns Void
 */
Void TDecEntropy::decodeIPCMInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if(!pcCU->getSlice()->getSPS()->getUsePCM()
    || pcCU->getWidth(uiAbsPartIdx) > (1<<pcCU->getSlice()->getSPS()->getPCMLog2MaxSize())
    || pcCU->getWidth(uiAbsPartIdx) < (1<<pcCU->getSlice()->getSPS()->getPCMLog2MinSize()) )
  {
    return;
  }
  
  m_pcEntropyDecoderIf->parseIPCMInfo( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseIntraDirLumaAng( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseIntraDirChroma( pcCU, uiAbsPartIdx, uiDepth );
}

/** decode motion information for every PU block.
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \param pcSubCU
 * \returns Void
 */
Void TDecEntropy::decodePUWise( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU )
{
  PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );
  UInt uiNumPU = ( ePartSize == SIZE_2Nx2N ? 1 : ( ePartSize == SIZE_NxN ? 4 : 2 ) );
  UInt uiPUOffset = ( g_auiPUOffset[UInt( ePartSize )] << ( ( pcCU->getSlice()->getSPS()->getMaxCUDepth() - uiDepth ) << 1 ) ) >> 4;

  TComMvField cMvFieldNeighbours[MRG_MAX_NUM_CANDS << 1]; // double length for mv of both lists
  UChar uhInterDirNeighbours[MRG_MAX_NUM_CANDS];

  for ( UInt ui = 0; ui < pcCU->getSlice()->getMaxNumMergeCand(); ui++ )
  {
    uhInterDirNeighbours[ui] = 0;
  }
  Int numValidMergeCand = 0;
  Bool isMerged = false;

  pcSubCU->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_0 );
  pcSubCU->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_1 );
  for ( UInt uiPartIdx = 0, uiSubPartIdx = uiAbsPartIdx; uiPartIdx < uiNumPU; uiPartIdx++, uiSubPartIdx += uiPUOffset )
  {
    decodeMergeFlag( pcCU, uiSubPartIdx, uiDepth, uiPartIdx );
    if ( pcCU->getMergeFlag( uiSubPartIdx ) )
    {
      decodeMergeIndex( pcCU, uiPartIdx, uiSubPartIdx, uiDepth );
      UInt uiMergeIndex = pcCU->getMergeIndex(uiSubPartIdx);
      if ( pcCU->getSlice()->getPPS()->getLog2ParallelMergeLevelMinus2() && ePartSize != SIZE_2Nx2N && pcSubCU->getWidth( 0 ) <= 8 ) 
      {
        pcSubCU->setPartSizeSubParts( SIZE_2Nx2N, 0, uiDepth );
        if ( !isMerged )
        {
          pcSubCU->getInterMergeCandidates( 0, 0, cMvFieldNeighbours, uhInterDirNeighbours, numValidMergeCand );
          isMerged = true;
        }
        pcSubCU->setPartSizeSubParts( ePartSize, 0, uiDepth );
      }
      else
      {
        uiMergeIndex = pcCU->getMergeIndex(uiSubPartIdx);
        pcSubCU->getInterMergeCandidates( uiSubPartIdx-uiAbsPartIdx, uiPartIdx, cMvFieldNeighbours, uhInterDirNeighbours, numValidMergeCand, uiMergeIndex );
      }
      pcCU->setInterDirSubParts( uhInterDirNeighbours[uiMergeIndex], uiSubPartIdx, uiPartIdx, uiDepth );

      TComMv cTmpMv( 0, 0 );
      for ( UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++ )
      {        
        if ( pcCU->getSlice()->getNumRefIdx( RefPicList( uiRefListIdx ) ) > 0 )
        {
          pcCU->setMVPIdxSubParts( 0, RefPicList( uiRefListIdx ), uiSubPartIdx, uiPartIdx, uiDepth);
          pcCU->setMVPNumSubParts( 0, RefPicList( uiRefListIdx ), uiSubPartIdx, uiPartIdx, uiDepth);
          pcCU->getCUMvField( RefPicList( uiRefListIdx ) )->setAllMvd( cTmpMv, ePartSize, uiSubPartIdx, uiDepth, uiPartIdx );
          pcCU->getCUMvField( RefPicList( uiRefListIdx ) )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex + uiRefListIdx ], ePartSize, uiSubPartIdx, uiDepth, uiPartIdx );
        }
      }
    }
    else
    {
      decodeInterDirPU( pcCU, uiSubPartIdx, uiDepth, uiPartIdx );
      for ( UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++ )
      {        
        if ( pcCU->getSlice()->getNumRefIdx( RefPicList( uiRefListIdx ) ) > 0 )
        {
          decodeRefFrmIdxPU( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx, RefPicList( uiRefListIdx ) );
          decodeMvdPU      ( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx, RefPicList( uiRefListIdx ) );
          decodeMVPIdxPU   ( pcSubCU, uiSubPartIdx-uiAbsPartIdx, uiDepth, uiPartIdx, RefPicList( uiRefListIdx ) );
        }
      }
    }
    if ( (pcCU->getInterDir(uiSubPartIdx) == 3) && pcSubCU->isBipredRestriction(uiPartIdx) )
    {
      pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMv( TComMv(0,0), ePartSize, uiSubPartIdx, uiDepth, uiPartIdx);
      pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( -1, ePartSize, uiSubPartIdx, uiDepth, uiPartIdx);
      pcCU->setInterDirSubParts( 1, uiSubPartIdx, uiPartIdx, uiDepth);
    }
  }
  return;
}

/** decode inter direction for a PU block
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \param uiPartIdx 
 * \returns Void
 */
Void TDecEntropy::decodeInterDirPU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx )
{
  UInt uiInterDir;

  if ( pcCU->getSlice()->isInterP() )
  {
    uiInterDir = 1;
  }
  else
  {
    m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx );
  }

  pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiPartIdx, uiDepth );
}

Void TDecEntropy::decodeRefFrmIdxPU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList )
{
  Int iRefFrmIdx = 0;
  Int iParseRefFrmIdx = pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList );

  if ( pcCU->getSlice()->getNumRefIdx( eRefList ) > 1 && iParseRefFrmIdx )
  {
    m_pcEntropyDecoderIf->parseRefFrmIdx( pcCU, iRefFrmIdx, eRefList );
  }
  else if ( !iParseRefFrmIdx )
  {
    iRefFrmIdx = NOT_VALID;
  }
  else
  {
    iRefFrmIdx = 0;
  }

  PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );
  pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, ePartSize, uiAbsPartIdx, uiDepth, uiPartIdx );
}

/** decode motion vector difference for a PU block
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \param uiPartIdx
 * \param eRefList 
 * \returns Void
 */
Void TDecEntropy::decodeMvdPU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList )
{
  if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
  {
    m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, uiPartIdx, uiDepth, eRefList );
  }
}

Void TDecEntropy::decodeMVPIdxPU( TComDataCU* pcSubCU, UInt uiPartAddr, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList )
{
  Int iMVPIdx = -1;

  TComMv cZeroMv( 0, 0 );
  TComMv cMv     = cZeroMv;
  Int    iRefIdx = -1;

  TComCUMvField* pcSubCUMvField = pcSubCU->getCUMvField( eRefList );
  AMVPInfo* pAMVPInfo = pcSubCUMvField->getAMVPInfo();

  iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
  cMv = cZeroMv;

  if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) )
  {
    m_pcEntropyDecoderIf->parseMVPIdx( iMVPIdx );
  }
  pcSubCU->fillMvpCand(uiPartIdx, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
  pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
  pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
  if ( iRefIdx >= 0 )
  {
    m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, cMv);
    cMv += pcSubCUMvField->getMvd( uiPartAddr );
  }

  PartSize ePartSize = pcSubCU->getPartitionSize( uiPartAddr );
  pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, 0, uiPartIdx);
}

Void TDecEntropy::xDecodeTransform( TComDataCU* pcCU, UInt offsetLuma, UInt offsetChroma, UInt uiAbsPartIdx, UInt uiDepth, UInt width, UInt height, UInt uiTrIdx, Bool& bCodeDQP, Int quadtreeTULog2MinSizeInCU)
{
  UInt uiSubdiv;
  const UInt uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()]+2 - uiDepth;

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
  }
  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_NxN && uiDepth == pcCU->getDepth(uiAbsPartIdx) )
  {
    uiSubdiv = 1;
  }
  else if( (pcCU->getSlice()->getSPS()->getQuadtreeTUMaxDepthInter() == 1) && (pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTER) && ( pcCU->getPartitionSize(uiAbsPartIdx) != SIZE_2Nx2N ) && (uiDepth == pcCU->getDepth(uiAbsPartIdx)) )
  {
    uiSubdiv = (uiLog2TrafoSize > quadtreeTULog2MinSizeInCU);
  }
  else if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    uiSubdiv = 1;
  }
  else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    uiSubdiv = 0;
  }
  else if( uiLog2TrafoSize == quadtreeTULog2MinSizeInCU )
  {
    uiSubdiv = 0;
  }
  else
  {
    assert( uiLog2TrafoSize > quadtreeTULog2MinSizeInCU );
    m_pcEntropyDecoderIf->parseTransformSubdivFlag( uiSubdiv, 5 - uiLog2TrafoSize );
  }
  
  const UInt uiTrDepth = uiDepth - pcCU->getDepth( uiAbsPartIdx );
  {
    const Bool bFirstCbfOfCU = uiTrDepth == 0;
    if( bFirstCbfOfCU )
    {
      pcCU->setCbfSubParts( 0, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );
      pcCU->setCbfSubParts( 0, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );
    }
    if( bFirstCbfOfCU || uiLog2TrafoSize > 2 )
    {
      if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth - 1 ) )
      {
        m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth, uiDepth );
      }
      if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth - 1 ) )
      {
        m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth, uiDepth );
      }
    }
    else
    {
      pcCU->setCbfSubParts( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth - 1 ) << uiTrDepth, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );
      pcCU->setCbfSubParts( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth - 1 ) << uiTrDepth, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );
    }
  }
  
  if( uiSubdiv )
  {
    UInt size;
    width  >>= 1;
    height >>= 1;
    size = width*height;
    uiTrIdx++;
    ++uiDepth;
    const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
    const UInt uiStartAbsPartIdx = uiAbsPartIdx;
    UInt uiYCbf = 0;
    UInt uiUCbf = 0;
    UInt uiVCbf = 0;
    
    for( Int i = 0; i < 4; i++ )
    {
      xDecodeTransform( pcCU, offsetLuma, offsetChroma, uiAbsPartIdx, uiDepth, width, height, uiTrIdx, bCodeDQP, quadtreeTULog2MinSizeInCU );
      uiYCbf |= pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiTrDepth+1 );
      uiUCbf |= pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth+1 );
      uiVCbf |= pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth+1 );
      uiAbsPartIdx += uiQPartNum;
      offsetLuma += size;  offsetChroma += (size>>2);
    }
    
    for( UInt ui = 0; ui < 4 * uiQPartNum; ++ui )
    {
      pcCU->getCbf( TEXT_LUMA     )[uiStartAbsPartIdx + ui] |= uiYCbf << uiTrDepth;
      pcCU->getCbf( TEXT_CHROMA_U )[uiStartAbsPartIdx + ui] |= uiUCbf << uiTrDepth;
      pcCU->getCbf( TEXT_CHROMA_V )[uiStartAbsPartIdx + ui] |= uiVCbf << uiTrDepth;
    }
  }
  else
  {
    assert( uiDepth >= pcCU->getDepth( uiAbsPartIdx ) );
    pcCU->setTrIdxSubParts( uiTrDepth, uiAbsPartIdx, uiDepth );
    
    {
      DTRACE_CABAC_VL( g_nSymbolCounter++ );
      DTRACE_CABAC_T( "\tTrIdx: abspart=" );
      DTRACE_CABAC_V( uiAbsPartIdx );
      DTRACE_CABAC_T( "\tdepth=" );
      DTRACE_CABAC_V( uiDepth );
      DTRACE_CABAC_T( "\ttrdepth=" );
      DTRACE_CABAC_V( uiTrDepth );
      DTRACE_CABAC_T( "\n" );
    }
    
    pcCU->setCbfSubParts ( 0, TEXT_LUMA, uiAbsPartIdx, uiDepth );
    if( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA && uiDepth == pcCU->getDepth( uiAbsPartIdx ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, 0 ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, 0 ) )
    {
      pcCU->setCbfSubParts( 1 << uiTrDepth, TEXT_LUMA, uiAbsPartIdx, uiDepth );
    }
    else
    {
      m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, uiTrDepth, uiDepth );
    }


    // transform_unit begin
    UInt cbfY = pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA    , uiTrIdx );
    UInt cbfU = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrIdx );
    UInt cbfV = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrIdx );
    if( uiLog2TrafoSize == 2 )
    {
      UInt partNum = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
      if( ( uiAbsPartIdx % partNum ) == (partNum - 1) )
      {
        cbfU = pcCU->getCbf( m_uiBakAbsPartIdx, TEXT_CHROMA_U, uiTrIdx );
        cbfV = pcCU->getCbf( m_uiBakAbsPartIdx, TEXT_CHROMA_V, uiTrIdx );
      }
    }
    if ( cbfY || cbfU || cbfV )
    {
      // dQP: only for LCU
      if ( pcCU->getSlice()->getPPS()->getUseDQP() )
      {
        if ( bCodeDQP )
        {
          decodeQP( pcCU, m_bakAbsPartIdxCU);
          bCodeDQP = false;
        }
      }
    }
    if( cbfY )
    {
      Int trWidth = width;
      Int trHeight = height;
      m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, (pcCU->getCoeffY()+offsetLuma), uiAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_LUMA );
    }
    if( uiLog2TrafoSize > 2 )
    {
      Int trWidth = width >> 1;
      Int trHeight = height >> 1;
      if( cbfU )
      {
        m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, (pcCU->getCoeffCb()+offsetChroma), uiAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_U );
      }
      if( cbfV )
      {
        m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, (pcCU->getCoeffCr()+offsetChroma), uiAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_V );
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
          m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, (pcCU->getCoeffCb()+m_uiBakChromaOffset), m_uiBakAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_U );
        }
        if( cbfV )
        {
          m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, (pcCU->getCoeffCr()+m_uiBakChromaOffset), m_uiBakAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_V );
        }
      }
    }
    // transform_unit end
  }
}

Void TDecEntropy::decodeQP          ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if ( pcCU->getSlice()->getPPS()->getUseDQP() )
  {
    m_pcEntropyDecoderIf->parseDeltaQP( pcCU, uiAbsPartIdx, pcCU->getDepth( uiAbsPartIdx ) );
  }
}


/** decode coefficients
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \param uiWidth
 * \param uiHeight 
 * \returns Void
 */
Void TDecEntropy::decodeCoeff( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, Bool& bCodeDQP )
{
  UInt uiMinCoeffSize = pcCU->getPic()->getMinCUWidth()*pcCU->getPic()->getMinCUHeight();
  UInt uiLumaOffset   = uiMinCoeffSize*uiAbsPartIdx;
  UInt uiChromaOffset = uiLumaOffset>>2;
  
  if( pcCU->isIntra(uiAbsPartIdx) )
  {
  }
  else
  {
    UInt uiQtRootCbf = 1;
    if( !( pcCU->getPartitionSize( uiAbsPartIdx) == SIZE_2Nx2N && pcCU->getMergeFlag( uiAbsPartIdx ) ) )
    {
      m_pcEntropyDecoderIf->parseQtRootCbf( uiAbsPartIdx, uiQtRootCbf );
    }
    if ( !uiQtRootCbf )
    {
      pcCU->setCbfSubParts( 0, 0, 0, uiAbsPartIdx, uiDepth );
      pcCU->setTrIdxSubParts( 0 , uiAbsPartIdx, uiDepth );
      return;
    }
    
  }
  
  Int getQuadtreeTULog2MinSizeInCU = pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx);
  
  xDecodeTransform( pcCU, uiLumaOffset, uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth, uiHeight, 0, bCodeDQP, getQuadtreeTULog2MinSizeInCU );
}

//! \}
