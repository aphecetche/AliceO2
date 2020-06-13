// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file   MIDQC/GBTRawDataChecker.h
/// \brief  Class to check the raw data from a GBT link
/// \author Diego Stocco <Diego.Stocco at cern.ch>
/// \date   28 April 2020
#ifndef O2_MID_GBTRawDataChecker_H
#define O2_MID_GBTRawDataChecker_H

#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <gsl/gsl>
#include "DataFormatsMID/ROFRecord.h"
#include "MIDRaw/LocalBoardRO.h"

namespace o2
{
namespace mid
{
class GBTRawDataChecker
{
 public:
  void init(uint16_t feeId, uint8_t mask);
  bool process(gsl::span<const LocalBoardRO> localBoards, gsl::span<const ROFRecord> rofRecords, gsl::span<const ROFRecord> pageRecords);
  /// Gets the number of processed events
  unsigned int getNEventsProcessed() const { return mStatistics[0]; }
  /// Gets the number of faulty events
  unsigned int getNEventsFaulty() const { return mStatistics[1]; }
  /// Gets the number of busy raised
  unsigned int getNBusyRaised() const { return mStatistics[2]; }
  /// Gets the
  std::string getDebugMessage() const { return mDebugMsg; }
  void clear();

 private:
  struct Mask {
    std::array<uint16_t, 4> patternsBP{};  /// Bending plane mask
    std::array<uint16_t, 4> patternsNBP{}; /// Non-bending plane mask
  };

  struct GBT {
    std::vector<LocalBoardRO> regs{}; /// Regional boards
    std::vector<LocalBoardRO> locs{}; /// Local boards
    std::vector<long int> pages{};    /// Pages information
  };

  struct BoardInfo {
    LocalBoardRO board{};
    o2::InteractionRecord interactionRecord{};
    long int page{-1};
  };

  // void clearChecked(size_t nCheckedTriggers);
  void clearChecked(const o2::InteractionRecord& lastCheckTrigIR, const std::unordered_map<uint8_t, size_t>& lastIndexes);
  bool checkEvent(uint8_t triggerWord, const std::vector<LocalBoardRO>& regs, const std::vector<LocalBoardRO>& locs, std::string& debugMsg) const;
  bool checkEvents();
  bool checkConsistency(const LocalBoardRO& board, std::string& debugMsg) const;
  bool checkConsistency(const std::vector<LocalBoardRO>& boards, std::string& debugMsg) const;
  bool checkMasks(const std::vector<LocalBoardRO>& locs, std::string& debugMsg) const;
  bool checkLocalBoardSize(const LocalBoardRO& board, std::string& debugMsg) const;
  bool checkLocalBoardSize(const std::vector<LocalBoardRO>& boards, std::string& debugMsg) const;
  bool checkRegLocConsistency(const std::vector<LocalBoardRO>& regs, const std::vector<LocalBoardRO>& locs, std::string& debugMsg) const;
  uint8_t getElinkId(const LocalBoardRO& board) const;
  auto getLastCompleteTrigEvent() const;
  // uint64_t getTimeStamp(const o2::InteractionRecord& ir) const;
  std::string printBoards(const std::vector<LocalBoardRO>& boards) const;
  std::unordered_map<uint8_t, size_t> sortEvents(const o2::InteractionRecord& lastCompleteTrigEventIR);

  std::string mDebugMsg{};                        /// Debug message
  std::array<unsigned long int, 3> mStatistics{}; /// Processed events statistics
  std::unordered_map<uint8_t, Mask> mMasks;       /// Masks
  std::unordered_map<uint8_t, bool> mBusyFlag;    /// Busy flag
  uint8_t mCrateMask{0xFF};                       /// Crate mask
  uint16_t mFeeId{0};                             /// FeeId

  std::unordered_map<uint8_t, std::vector<BoardInfo>> mBoards{}; ///! Boards
  // std::unordered_map<uint8_t, std::vector<size_t>> mTriggeredEvents{}; ///! Index of triggered events
  std::map<o2::InteractionRecord, uint16_t> mTrigEvents{}; ///! Index of triggered events

  std::map<o2::InteractionRecord, std::vector<std::pair<uint8_t, size_t>>> mOrderedIndexes{}; ///! Ordered indexes
};
} // namespace mid
} // namespace o2

#endif /* O2_MID_GBTRawDataChecker_H */
