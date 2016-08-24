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

#include "TLibCommon/TComBitCounter.h"
#include "TLibCommon/TComBitStream.h"
#include "TLibCommon/SEI.h"
#include "TLibCommon/TComSlice.h"
#include "SEIwrite.h"

//! \ingroup TLibEncoder
//! \{

#if ENC_DEC_TRACE
Void  xTraceSEIHeader()
{
  fprintf( g_hTrace, "=========== SEI message ===========\n");
}

Void  xTraceSEIMessageType(SEI::PayloadType payloadType)
{
  switch (payloadType)
  {
  case SEI::DECODED_PICTURE_HASH:
    fprintf( g_hTrace, "=========== Decoded picture hash SEI message ===========\n");
    break;
  case SEI::USER_DATA_UNREGISTERED:
    fprintf( g_hTrace, "=========== User Data Unregistered SEI message ===========\n");
    break;
  case SEI::ACTIVE_PARAMETER_SETS:
    fprintf( g_hTrace, "=========== Active Parameter sets SEI message ===========\n");
    break;
  case SEI::RECOVERY_POINT:
    fprintf( g_hTrace, "=========== Recovery point SEI message ===========\n");
    break;
  case SEI::TEMPORAL_LEVEL0_INDEX:
    fprintf( g_hTrace, "=========== Temporal Level Zero Index SEI message ===========\n");
    break;
  case SEI::DECODING_UNIT_INFO:
    fprintf( g_hTrace, "=========== Decoding Unit Information SEI message ===========\n");
    break;
  default:
    fprintf( g_hTrace, "=========== Unknown SEI message ===========\n");
    break;
  }
}
#endif

void SEIWriter::xWriteSEIpayloadData(TComBitIf& bs, const SEI& sei, TComSPS *sps)
{
  switch (sei.payloadType())
  {
  case SEI::USER_DATA_UNREGISTERED:
    xWriteSEIuserDataUnregistered(*static_cast<const SEIuserDataUnregistered*>(&sei));
    break;
  case SEI::DECODED_PICTURE_HASH:
    xWriteSEIDecodedPictureHash(*static_cast<const SEIDecodedPictureHash*>(&sei));
    break;
  case SEI::RECOVERY_POINT:
    xWriteSEIRecoveryPoint(*static_cast<const SEIRecoveryPoint*>(&sei));
    break;
  default:
    assert(!"Unhandled SEI message");
  }
}

/**
 * marshal a single SEI message sei, storing the marshalled representation
 * in bitstream bs.
 */
Void SEIWriter::writeSEImessage(TComBitIf& bs, const SEI& sei, TComSPS *sps)
{
  /* calculate how large the payload data is */
  /* TODO: this would be far nicer if it used vectored buffers */
  TComBitCounter bs_count;
  bs_count.resetBits();
  setBitstream(&bs_count);


#if ENC_DEC_TRACE
  Bool traceEnable = g_HLSTraceEnable;
  g_HLSTraceEnable = false;
#endif
  xWriteSEIpayloadData(bs_count, sei, sps);
#if ENC_DEC_TRACE
  g_HLSTraceEnable = traceEnable;
#endif

  UInt payload_data_num_bits = bs_count.getNumberOfWrittenBits();
  assert(0 == payload_data_num_bits % 8);

  setBitstream(&bs);

#if ENC_DEC_TRACE
  if (g_HLSTraceEnable)
  xTraceSEIHeader();
#endif

  UInt payloadType = sei.payloadType();
  for (; payloadType >= 0xff; payloadType -= 0xff)
  {
    WRITE_CODE(0xff, 8, "payload_type");
  }
  WRITE_CODE(payloadType, 8, "payload_type");

  UInt payloadSize = payload_data_num_bits/8;
  for (; payloadSize >= 0xff; payloadSize -= 0xff)
  {
    WRITE_CODE(0xff, 8, "payload_size");
  }
  WRITE_CODE(payloadSize, 8, "payload_size");

  /* payloadData */
#if ENC_DEC_TRACE
  if (g_HLSTraceEnable)
  xTraceSEIMessageType(sei.payloadType());
#endif

  xWriteSEIpayloadData(bs, sei, sps);
}

/**
 * marshal a user_data_unregistered SEI message sei, storing the marshalled
 * representation in bitstream bs.
 */
Void SEIWriter::xWriteSEIuserDataUnregistered(const SEIuserDataUnregistered &sei)
{
  for (UInt i = 0; i < 16; i++)
  {
    WRITE_CODE(sei.uuid_iso_iec_11578[i], 8 , "sei.uuid_iso_iec_11578[i]");
  }

  for (UInt i = 0; i < sei.userDataLength; i++)
  {
    WRITE_CODE(sei.userData[i], 8 , "user_data");
  }
}

/**
 * marshal a decoded picture hash SEI message, storing the marshalled
 * representation in bitstream bs.
 */
Void SEIWriter::xWriteSEIDecodedPictureHash(const SEIDecodedPictureHash& sei)
{
  WRITE_CODE(sei.method, 8, "hash_type");

  for(Int yuvIdx = 0; yuvIdx < 3; yuvIdx++)
  {
    if(sei.method == SEIDecodedPictureHash::MD5)
    {
      for (UInt i = 0; i < 16; i++)
      {
        WRITE_CODE(sei.digest[yuvIdx][i], 8, "picture_md5");
      }
    }
  }
}

Void SEIWriter::xWriteSEIRecoveryPoint(const SEIRecoveryPoint& sei)
{
  WRITE_SVLC( sei.m_recoveryPocCnt,    "recovery_poc_cnt"    );
  WRITE_FLAG( sei.m_exactMatchingFlag, "exact_matching_flag" );
  WRITE_FLAG( sei.m_brokenLinkFlag,    "broken_link_flag"    );
  xWriteByteAlign();
}
Void SEIWriter::xWriteByteAlign()
{
  if( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0)
  {
    WRITE_FLAG( 1, "bit_equal_to_one" );
    while( m_pcBitIf->getNumberOfWrittenBits() % 8 != 0 )
    {
      WRITE_FLAG( 0, "bit_equal_to_zero" );
    }
  }
};

//! \}
