// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/** @file Digit.h
 * C++ simple Muon MCH digit.
 * @author  Michael Winn
 */

#ifndef ALICEO2_MCH_BASE_DIGIT_H_
#define ALICEO2_MCH_BASE_DIGIT_H_

#include "Rtypes.h"

namespace o2
{
namespace mch
{

// \class Digit
/// \brief MCH digit implementation
class Digit
{
 public:
  Digit() = default;

  Digit(int detid, int pad, unsigned long adc, int32_t time, uint16_t nSamples = 1);
  ~Digit() = default;

  bool operator==(const Digit&) const;

  // time in bunch crossing units, relative to the beginning of the TimeFrame
  void setTime(int32_t t) { mTFtime = t; }
  int32_t getTime() const { return mTFtime; }

  /// flag indicating wether the time stamp is valid or not
  bool isTimeValid() const;
  void setTimeValid(bool valid);

  /// set the index of the TimeFrame this digit belongs to. Possible values:
  /// idx = 0 -> the digit belongs to the TF before the currently processed one
  /// idx = 1 -> the digit belongs to the currently processed TF
  /// idx = 2 -> the digit belongs to the TF after the currently processed one
  ///
  /// LA: assuming we really need this index, should use constants (part of
  /// the interface), e.g. PREVIOUS_TIME_FRAME, CURRENT_TIME_FRAME, NEXT_TIME_FRAME
  /// ?
  uint8_t getTFindex() const;
  void setTFindex(uint8_t idx);

  uint16_t nofSamples() const { return mNofSamples; }
  void setNofSamples(uint16_t n) { mNofSamples = n; }

  int getDetID() const { return mDetID; }

  int getPadID() const { return mPadID; }
  void setPadID(int padID) { mPadID = padID; }

  unsigned long getADC() const { return mADC; }
  void setADC(unsigned long adc) { mADC = adc; }

 private:
  int32_t mTFtime;      /// time since the beginning of the time frame, in bunch crossing units
  uint8_t mTimeFlags;   /// flags indicating wether the time stamp is valid and to which TimeFrame the digit belongs to
  uint16_t mNofSamples; /// number of samples in the signal
  int mDetID;           /// DetectorIndex to which the digit corresponds to
  int mPadID;           /// PadIndex to which the digit corresponds to
  unsigned long mADC;   /// Amplitude of signal

  ClassDefNV(Digit, 2);
}; //class Digit

} //namespace mch
} //namespace o2
#endif // ALICEO2_MCH_DIGIT_H_
