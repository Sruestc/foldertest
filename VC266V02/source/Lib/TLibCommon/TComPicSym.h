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

/** \file     TComPicSym.h
    \brief    picture symbol class (header)
*/

#ifndef __TCOMPICSYM__
#define __TCOMPICSYM__


// Include files
#include "CommonDef.h"
#include "TComSlice.h"
#include "TComDataCU.h"
class TComSampleAdaptiveOffset;
class TComPPS;

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// picture symbol class
class TComPicSym
{
private:
  UInt          m_uiWidthInCU;
  UInt          m_uiHeightInCU;
  
  UInt          m_uiMaxCUWidth;
  UInt          m_uiMaxCUHeight;
  UInt          m_uiMinCUWidth;
  UInt          m_uiMinCUHeight;
  
  UChar         m_uhTotalDepth;       ///< max. depth
  UInt          m_uiNumPartitions;
  UInt          m_uiNumPartInWidth;
  UInt          m_uiNumPartInHeight;
  UInt          m_uiNumCUsInFrame;
  
  TComSlice**   m_apcTComSlice;
  UInt          m_uiNumAllocatedSlice;
  TComDataCU**  m_apcTComDataCU;        ///< array of CU data
  
  Int           m_iNumColumnsMinus1; 
  Int           m_iNumRowsMinus1;
  UInt*         m_puiCUOrderMap;       //the map of LCU raster scan address relative to LCU encoding order 
  UInt*         m_puiInverseCUOrderMap;

  SAOBlkParam *m_saoBlkParams;

public:
  Void        create  ( Int iPicWidth, Int iPicHeight, UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxDepth );
  Void        destroy ();

  TComPicSym  ();
  TComSlice*  getSlice(UInt i)          { return  m_apcTComSlice[i];            }
  UInt        getFrameWidthInCU()       { return m_uiWidthInCU;                 }
  UInt        getFrameHeightInCU()      { return m_uiHeightInCU;                }
  UInt        getMinCUWidth()           { return m_uiMinCUWidth;                }
  UInt        getMinCUHeight()          { return m_uiMinCUHeight;               }
  UInt        getNumberOfCUsInFrame()   { return m_uiNumCUsInFrame;  }
  TComDataCU*&  getCU( UInt uiCUAddr )  { return m_apcTComDataCU[uiCUAddr];     }
  
  Void        setSlice(TComSlice* p, UInt i) { m_apcTComSlice[i] = p;           }
  UInt        getNumAllocatedSlice()    { return m_uiNumAllocatedSlice;         }
  Void        allocateNewSlice();
  Void        clearSliceBuffer();
  UInt        getNumPartition()         { return m_uiNumPartitions;             }
  UInt        getNumPartInWidth()       { return m_uiNumPartInWidth;            }
  UInt        getNumPartInHeight()      { return m_uiNumPartInHeight;           }
  Void         setNumColumnsMinus1( Int i )                          { m_iNumColumnsMinus1 = i; }
  Int          getNumColumnsMinus1()                                 { return m_iNumColumnsMinus1; }  
  Void         setNumRowsMinus1( Int i )                             { m_iNumRowsMinus1 = i; }
  Int          getNumRowsMinus1()                                    { return m_iNumRowsMinus1; }
  Int          getNumTiles()                                         { return (m_iNumRowsMinus1+1)*(m_iNumColumnsMinus1+1); }

  Void         setCUOrderMap( Int encCUOrder, Int cuAddr )           { *(m_puiCUOrderMap + encCUOrder) = cuAddr; }
  UInt         getCUOrderMap( Int encCUOrder )                       { return *(m_puiCUOrderMap + (encCUOrder>=m_uiNumCUsInFrame ? m_uiNumCUsInFrame : encCUOrder)); }

  Void         setInverseCUOrderMap( Int cuAddr, Int encCUOrder )    { *(m_puiInverseCUOrderMap + cuAddr) = encCUOrder; }
  UInt         getInverseCUOrderMap( Int cuAddr )                    { return *(m_puiInverseCUOrderMap + (cuAddr>=m_uiNumCUsInFrame ? m_uiNumCUsInFrame : cuAddr)); }
  UInt         getPicSCUEncOrder( UInt SCUAddr );
  UInt         getPicSCUAddr( UInt SCUEncOrder );

  SAOBlkParam* getSAOBlkParam() { return m_saoBlkParams;}
  Void deriveLoopFilterBoundaryAvailibility(Int ctu, Bool& isLeftAvail,Bool& isRightAvail,Bool& isAboveAvail,Bool& isBelowAvail,Bool& isAboveLeftAvail,Bool& isAboveRightAvail,Bool& isBelowLeftAvail,Bool& isBelowRightAvail);

};// END CLASS DEFINITION TComPicSym

//! \}

#endif // __TCOMPICSYM__

