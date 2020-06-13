// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file   MID/QC/src/GBTRawDataChecker.cxx
/// \brief  Class to check the raw data from a GBT link
/// \author Diego Stocco <Diego.Stocco at cern.ch>
/// \date   28 April 2020

#include "MIDQC/GBTRawDataChecker.h"

#include <sstream>
#include <fmt/format.h>
#include "MIDRaw/CrateParameters.h"

#include <iostream> // TODO: REMOVE

namespace o2
{
namespace mid
{

void GBTRawDataChecker::init(uint16_t feeId, uint8_t mask)
{
  /// Initializer
  mFeeId = feeId;
  mCrateMask = mask;
}

bool GBTRawDataChecker::checkLocalBoardSize(const LocalBoardRO& board, std::string& debugMsg) const
{
  /// Checks that the board has the expected non-null patterns

  // This test only make sense when we have a self-trigger,
  // since in this case we expect to have a variable number of non-zero pattern
  // as indicated by the corresponding word.
  for (int ich = 0; ich < 4; ++ich) {
    bool isExpectedNull = (((board.firedChambers >> ich) & 0x1) == 0);
    bool isNull = (board.patternsBP[ich] == 0 && board.patternsNBP[ich] == 0);
    if (isExpectedNull != isNull) {
      std::stringstream ss;
      ss << "wrong size for local board:\n";
      ss << board << "\n";
      debugMsg += ss.str();
      return false;
    }
  }
  return true;
}

bool GBTRawDataChecker::checkLocalBoardSize(const std::vector<LocalBoardRO>& boards, std::string& debugMsg) const
{
  /// Checks that the boards have the expected non-null patterns
  for (auto& board : boards) {
    if (!checkLocalBoardSize(board, debugMsg)) {
      return false;
    }
  }
  return true;
}

bool GBTRawDataChecker::checkConsistency(const LocalBoardRO& board, std::string& debugMsg) const
{
  /// Checks that the event information is consistent

  bool isSoxOrReset = board.triggerWord & (raw::sSOX | raw::sEOX | raw::sRESET);
  bool isCalib = raw::isCalibration(board.triggerWord);
  bool isPhys = board.triggerWord & raw::sPHY;

  if (isPhys) {
    if (isCalib) {
      debugMsg += "inconsistent trigger: calibration and physics trigger cannot be fired together\n";
      return false;
    }
    if (raw::isLoc(board.statusWord)) {
      if (board.firedChambers) {
        debugMsg += "inconsistent trigger: fired chambers should be 0\n";
        return false;
      }
    }
  }
  if (isSoxOrReset && (isCalib || isPhys)) {
    debugMsg += "inconsistent trigger: cannot be SOX and calibration\n";
    return false;
  }

  return true;
}

bool GBTRawDataChecker::checkConsistency(const std::vector<LocalBoardRO>& boards, std::string& debugMsg) const
{
  /// Checks that the event information is consistent
  for (auto& board : boards) {
    if (!checkConsistency(board, debugMsg)) {
      std::stringstream ss;
      ss << board << "\n";
      debugMsg += ss.str();
      return false;
    }
  }
  return true;
}

bool GBTRawDataChecker::checkMasks(const std::vector<LocalBoardRO>& locs, std::string& debugMsg) const
{
  /// Checks the masks
  for (auto loc : locs) {
    // The board patterns coincide with the masks ("overwritten" mode)
    if (loc.statusWord & raw::sOVERWRITTEN) {
      auto maskItem = mMasks.find(loc.boardId);
      for (int ich = 0; ich < 4; ++ich) {
        uint16_t maskBP = 0;
        uint16_t maskNBP = 0;
        if (maskItem != mMasks.end()) {
          maskBP = maskItem->second.patternsBP[ich];
          maskNBP = maskItem->second.patternsNBP[ich];
        }
        if (maskBP != loc.patternsBP[ich] || maskNBP != loc.patternsNBP[ich]) {
          std::stringstream ss;
          ss << "Pattern is not compatible with mask for:\n";
          ss << loc << "\n";
          debugMsg += ss.str();
          return false;
        }
      }
    }
  }
  return true;
}

bool GBTRawDataChecker::checkRegLocConsistency(const std::vector<LocalBoardRO>& regs, const std::vector<LocalBoardRO>& locs, std::string& debugMsg) const
{
  /// Checks consistency between local and regional info
  uint8_t regFired{0};
  for (auto& reg : regs) {
    uint8_t ireg = crateparams::getLocId(reg.boardId) % 2;
    auto busyItem = mBusyFlag.find(8 + ireg);
    if (reg.triggerWord == 0) {
      // Self-triggered event: check the decision
      regFired |= (reg.firedChambers << (4 * ireg));
    } else {
      // Triggered event: all active boards must answer
      regFired |= (mCrateMask << (4 * ireg));
    }
  }
  uint8_t locFired{0}, locBusy{0};
  for (auto& loc : locs) {
    auto linkId = getElinkId(loc);
    uint8_t mask = (1 << linkId);
    if (loc.triggerWord == 0) {
      // Self-triggered event: check the decision
      if (loc.firedChambers) {
        locFired |= mask;
      }
    } else {
      // Triggered event: all active boards must answer
      locFired |= mask;
    }
  }

  // The XOR returns 1 in case of a difference
  uint8_t problems = (regFired ^ locFired);

  if (problems) {
    // It can be that a busy signal was raised by one of the board in previous events
    // If the board is still busy it will not answer.
    uint8_t busy{0};
    for (uint8_t iboard = 0; iboard < crateparams::sNELinksPerGBT; ++iboard) {
      auto busyItem = mBusyFlag.find(iboard);
      if (busyItem != mBusyFlag.end() && busyItem->second) {
        busy |= (iboard < crateparams::sMaxNBoardsInLink) ? (1 << iboard) : (0xF << (4 * (iboard % 2)));
      }
    }

    if (problems & ~busy) {
      std::stringstream ss;
      ss << fmt::format("loc-reg inconsistency: fired locals ({:08b}) != expected from reg ({:08b});\n", locFired, regFired);
      ss << printBoards(regs);
      ss << printBoards(locs);
      debugMsg += ss.str();
      return false;
    }
  }
  return true;
}

std::string GBTRawDataChecker::printBoards(const std::vector<LocalBoardRO>& boards) const
{
  /// Prints the boards
  std::stringstream ss;
  for (auto& board : boards) {
    ss << board << "\n";
  }
  return ss.str();
}

bool GBTRawDataChecker::checkEvent(uint8_t triggerWord, const std::vector<LocalBoardRO>& regs, const std::vector<LocalBoardRO>& locs, std::string& debugMsg) const
{
  /// Checks the cards belonging to the same BC
  bool isOk = true;

  std::stringstream ss;

  if (!checkRegLocConsistency(regs, locs, debugMsg)) {
    return false;
  }

  if (!checkConsistency(regs, debugMsg) || !checkConsistency(locs, debugMsg)) {
    return false;
  }

  if (triggerWord == 0) {
    if (!checkLocalBoardSize(locs, debugMsg)) {
      return false;
    }

    if (!checkMasks(locs, debugMsg)) {
      return false;
    }
  }

  return true;
}

uint8_t GBTRawDataChecker::getElinkId(const LocalBoardRO& board) const
{
  /// Returns the e-link ID
  if (raw::isLoc(board.statusWord)) {
    return board.boardId % 8;
  }
  return 8 + board.boardId % 8;
}

// uint64_t GBTRawDataChecker::getTimeStamp(const o2::InteractionRecord& ir) const
// {
//   /// Returns the event timestamp.
//   /// Notice that we cannot use InteractionRecord.toLong(),
//   /// because this function assumes that the bc is smaller than the maximum number of bcs at the LHC.
//   /// However, this is not always the case in the tests of the electronics.
//   return ((static_cast<uint64_t>(ir.orbit) << 16ULL) | ir.bc);
// }

void GBTRawDataChecker::clearChecked(const o2::InteractionRecord& lastCheckTrigIR, const std::unordered_map<uint8_t, size_t>& lastIndexes)
{
  /// Clears the checked events

  // Create a new board map with the checked events stripped
  std::unordered_map<uint8_t, std::vector<BoardInfo>> boards{};
  for (auto& lastIdxItem : lastIndexes) {
    auto firstIdx = lastIdxItem.second + 1;
    auto& boardVec = mBoards[lastIdxItem.first];
    if (firstIdx < boardVec.size()) {
      auto& newVec = boards[lastIdxItem.first];
      newVec.insert(newVec.end(), boardVec.begin() + firstIdx, boardVec.end());
    }
    // std::cout << "Last for " << static_cast<int>(lastIdxItem.first) << " " << boardVec[lastIdxItem.second].interactionRecord << "  " << boardVec[lastIdxItem.second].board << std::endl; // TODO: REMOVE
  }
  mBoards.swap(boards);

  // Clears the map with the processed triggers
  auto low = mTrigEvents.begin();
  auto up = mTrigEvents.upper_bound(lastCheckTrigIR);
  mTrigEvents.erase(low, up);
}

// void GBTRawDataChecker::clearChecked(size_t nCheckedTriggers)
// {
//   /// Clears the checked events
//   std::unordered_map<uint8_t, std::vector<BoardInfo>> boards{};
//   std::unordered_map<uint8_t, std::vector<size_t>> triggeredEvents{};

//   // Create a new board map with the checked events stripped
//   for (auto& trigEvtItem : mTriggeredEvents) {
//     auto firstIdx = trigEvtItem.second[nCheckedTriggers - 1] + 1;
//     auto& boardVec = mBoards[trigEvtItem.first];
//     if (firstIdx < boardVec.size()) {
//       auto& newVec = boards[trigEvtItem.first];
//       newVec.insert(newVec.end(), boardVec.begin() + firstIdx, boardVec.end());
//     }
//     if (nCheckedTriggers < trigEvtItem.second.size()) {
//       // Modify the indexes accordingly
//       auto& newTrigVec = triggeredEvents[trigEvtItem.first];
//       for (auto trigIt = trigEvtItem.second.begin() + nCheckedTriggers, end = trigEvtItem.second.end(); trigIt != end; ++trigIt) {
//         newTrigVec.emplace_back(*trigIt - firstIdx);
//       }
//     }
//   }
//   mBoards.swap(boards);
//   mTriggeredEvents.swap(triggeredEvents);
// }

auto GBTRawDataChecker::getLastCompleteTrigEvent() const
{
  /// Checks if we have a triggered event with the information from all active boards
  /// The function returns true if it finds a complete event
  /// and it returns its interaction record as well.

  // The information for an event comes at different times for different boards,
  // depending on the length of the self-triggered event for that board.
  // So, before testing the consistency of the event,
  // we must wait to have received the information from all boards.
  // This can be checked in triggered events, since all boards should be present.

  // Check if we have a triggered event with the information from all active boards
  uint16_t fullMask = (3 << 8) | mCrateMask;
  auto trigEventIt = mTrigEvents.rbegin();
  auto end = mTrigEvents.rend();
  for (; trigEventIt != end; ++trigEventIt) {
    if ((trigEventIt->second & fullMask) == fullMask) {
      break;
    }
  }

  return trigEventIt;
}

std::unordered_map<uint8_t, size_t> GBTRawDataChecker::sortEvents(const o2::InteractionRecord& lastCompleteTrigEventIR)
{
  /// Sorts the event in time
  mOrderedIndexes.clear();
  std::unordered_map<uint8_t, size_t> lastIndexes;
  for (auto& boardItem : mBoards) {
    size_t lastIdx = 0;
    uint32_t nMissedOrbits = 0;
    auto prevIt = boardItem.second.begin();
    auto lastOrbitIt = prevIt;
    for (auto boardIt = boardItem.second.begin(), end = boardItem.second.end(); boardIt != end; ++boardIt) {
      if (boardItem.first < crateparams::sMaxNBoardsInLink) {
        // In the tests, when a local board is busy, it can happen that it does not send the answer to an orbit trigger.
        // When this happens, the orbit is not increased.
        // This affects all subsequent events, until the next orbit trigger.
        // If we observe a reset of the BC that is not associated to an orbit trigger,
        // we manually increase the orbit.
        // Notice that there are cases where we have:
        // - orbit trigger with local clock X
        // - busy flag event with local clock X
        // - event with reset local clock
        // So, if we observe a reset, we must check that the distance w.r.t. last  orbit trigger
        // is less than 2
        // FIXME: should this be handled at decoding level? Discussions with RO team ongoing
        if (boardIt->board.triggerWord & raw::sORB) {
          lastOrbitIt = boardIt;
          if (nMissedOrbits > 0) {
            --nMissedOrbits;
          }
        } else if (boardIt->interactionRecord.bc < prevIt->interactionRecord.bc && std::distance(lastOrbitIt, boardIt) > 2) {
          ++nMissedOrbits;
        }
        prevIt = boardIt;
        boardIt->interactionRecord.orbit += nMissedOrbits;
      }
      // auto ir = getTimeStamp(boardIt->interactionRecord);
      if (boardIt->interactionRecord > lastCompleteTrigEventIR) {
        break;
      }
      lastIdx = std::distance(boardItem.second.begin(), boardIt);
      mOrderedIndexes[boardIt->interactionRecord].emplace_back(boardItem.first, lastIdx);
    }
    lastIndexes[boardItem.first] = lastIdx;
  }
  return lastIndexes;
}

bool GBTRawDataChecker::checkEvents()
{
  /// Checks the events
  bool isOk = true;

  // if (mTriggeredEvents.empty()) {
  //   printf("Empty triggered events!\n"); // TODO: REMOVE
  //   return true;
  // }

  // // Check if we have a triggered event on all active boards
  // size_t nTriggered = mTriggeredEvents.begin()->second.size();
  // for (uint8_t iloc = 0; iloc < 10; ++iloc) {
  //   if (iloc >= 8 || mCrateMask & (1 << iloc)) {
  //     auto trigEvtItem = mTriggeredEvents.find(iloc);
  //     if (trigEvtItem == mTriggeredEvents.end()) {
  //       // Missing board: skip test
  //       printf("Missing info for %i!\n", iloc); // TODO: REMOVE
  //       return true;
  //     }
  //     if (nTriggered > trigEvtItem->second.size()) {
  //       nTriggered = trigEvtItem->second.size();
  //     }
  //   }
  // }

  // // Check if the last event has the same trigger type
  // // It can happen that we have two triggers at the same IR.
  // // In this case, a simple check on the size is not enough.
  // // We need to check that we have the same trigger word
  // size_t lastComplete = nTriggered - 1;
  // auto trigIt = mTriggeredEvents.begin();
  // auto& refBoard = mBoards[trigIt->first][trigIt->second[lastComplete]];
  // ++trigIt;
  // for (; trigIt != mTriggeredEvents.end(); ++trigIt) {
  //   if (mBoards[trigIt->first][trigIt->second[lastComplete]].board.triggerWord != refBoard.board.triggerWord) {
  //     std::cout << "MERDA: " << mBoards[trigIt->first][trigIt->second[lastComplete]].interactionRecord << "    " << refBoard.interactionRecord << std::endl; // TODO: REMOVE
  //     return true;
  //   }
  // }

  // // The regional information comes with a delay.
  // // So, self-triggered events coming from a previous event can be read after the triggered event.
  // // To take this into account, extend the search until we change orbit
  // // for (auto& trigEvtItem : mTriggeredEvents) {
  // for (uint8_t iloc = 8; iloc < 10; ++iloc) {
  //   auto trigEvtItem = mTriggeredEvents.find(iloc);
  //   auto& boardVec = mBoards[trigEvtItem->first];
  //   auto refBoardIt = boardVec.begin() + trigEvtItem->second[lastComplete];
  //   // if (refBoardIt->board.triggerWord & raw::sSOX) {
  //   //   // The IR for the SOX can be larger than other events in the orbit
  //   //   break;
  //   // }
  //   auto boardIt = refBoardIt;
  //   ++boardIt;
  //   // std::cout << "Ref: " << refBoardIt->interactionRecord << "  " << refBoardIt->board << std::endl; // TODO: REMOVE
  //   while (boardIt != boardVec.end() && boardIt->interactionRecord.orbit <= refBoardIt->interactionRecord.orbit) {
  //     std::cout << "Add: " << boardIt->interactionRecord << "  " << boardIt->board << std::endl; // TODO: REMOVE
  //     ++trigEvtItem->second[nTriggered - 1];
  //     ++boardIt;
  //   }
  // }

  // printf("Start check!\n"); // TODO: REMOVE

  // for (auto& trigEvtItem : mTriggeredEvents) {
  //   auto& boardVec = mBoards[trigEvtItem.first];
  //   for (auto boardIt = boardVec.begin(), end = boardVec.begin() + trigEvtItem.second[lastComplete] + 1; boardIt != end; ++boardIt) {
  //     orderedIndexes[boardIt->interactionRecord].emplace_back(trigEvtItem.first, std::distance(boardVec.begin(), boardIt));
  //   }
  // }

  std::unordered_map<uint8_t, GBT> gbtEvents;
  std::unordered_map<uint8_t, size_t> lastBusyIdx;
  // Loop on the event indexes
  for (auto& evtIdxItem : mOrderedIndexes) {
    // All of these boards have the same timestamp
    bool busyRaised = false;
    gbtEvents.clear();
    for (auto& evtPair : evtIdxItem.second) {
      auto& boardInfo = mBoards[evtPair.first][evtPair.second];
      // We now order all of the cards in the event according to the trigger word
      // We are obliged to account for the trigger word because, when a trigger occurs,
      // all of the cards are expected to answer.
      // This can happen on top of the self-triggered event, leading to two separate events for one BC
      // In this way, each element in the map contains all of the boards per GBT link per event
      uint8_t triggerId = boardInfo.board.triggerWord;
      auto elinkId = getElinkId(boardInfo.board);
      bool isBusy = ((boardInfo.board.statusWord & raw::sLOCALBUSY) != 0);

      // This is a protection for regional cards, where the BC is affected by a delay.
      // The self-triggered event arrived few BC later than the triggered one.
      auto lastBusyItem = lastBusyIdx.find(elinkId);
      if (lastBusyItem == lastBusyIdx.end() || evtPair.second > lastBusyItem->second) {
        mBusyFlag[elinkId] = isBusy;
      }

      if (isBusy) {
        lastBusyIdx[elinkId] = evtPair.second;
        busyRaised = true;
        if (triggerId == 0) {
          // This is a special event that just signals a busy.
          // Do not add the board to the events to be tested.
          // Even because this event can have the same IR and triggerWord (0) of a self-triggered event
          continue;
        }
      }
      auto& gbtEvent = gbtEvents[triggerId];
      if (raw::isLoc(boardInfo.board.statusWord)) {
        gbtEvent.locs.push_back(boardInfo.board);
      } else {
        gbtEvent.regs.push_back(boardInfo.board);
      }
      if (boardInfo.page >= 0) {
        if (std::find(gbtEvent.pages.begin(), gbtEvent.pages.end(), boardInfo.page) == gbtEvent.pages.end()) {
          gbtEvent.pages.push_back(boardInfo.page);
        }
      }
    }
    if (busyRaised) {
      ++mStatistics[2];
    }
    for (auto& gbtEvtItem : gbtEvents) {
      ++mStatistics[0];
      std::string debugStr;
      if (!checkEvent(gbtEvtItem.first, gbtEvtItem.second.regs, gbtEvtItem.second.locs, debugStr)) {
        std::stringstream ss;
        ss << std::hex << std::showbase << evtIdxItem.first;
        if (!gbtEvtItem.second.pages.empty()) {
          ss << "   [in";
          for (auto& page : gbtEvtItem.second.pages) {
            ss << std::dec << "  page: " << page << "  (line: " << 512 * page + 1 << ")  ";
          }
          ss << "]";
        }
        ss << "\n";
        isOk = false;
        ss << debugStr << "\n";
        mDebugMsg += ss.str();
        ++mStatistics[1];
      }
    }
  }

  // if (!isOk) {
  //   std::cout << "DEBUG: " << std::endl; // TODO: REMOVE
  //   for (auto& board : mBoards[9]) {
  //     std::cout << std::hex << std::showbase << board.interactionRecord << std::endl; // TODO: REMOVE
  //     std::cout << board.board << std::endl;                                          // TODO: REMOVE
  //   }
  // }

  // clearChecked(nTriggered);
  return isOk;
}

bool GBTRawDataChecker::process(gsl::span<const LocalBoardRO> localBoards, gsl::span<const ROFRecord> rofRecords, gsl::span<const ROFRecord> pageRecords)
{
  /// Checks the raw data
  mDebugMsg.clear();

  // Fill board information
  for (auto rofIt = rofRecords.begin(); rofIt != rofRecords.end(); ++rofIt) {
    for (auto locIt = localBoards.begin() + rofIt->firstEntry; locIt != localBoards.begin() + rofIt->firstEntry + rofIt->nEntries; ++locIt) {
      // Find what page this event corresponds to.
      // This is useful for debugging.
      long int page = -1;
      for (auto& rofPage : pageRecords) {
        if (rofIt->firstEntry >= rofPage.firstEntry && rofIt->firstEntry < rofPage.firstEntry + rofPage.nEntries) {
          page = rofPage.interactionRecord.orbit;
          break;
        }
      }

      // Store the information per local board.
      // The information should be already ordered in time
      auto id = getElinkId(*locIt);
      auto& elinkVec = mBoards[id];
      elinkVec.push_back({*locIt, rofIt->interactionRecord, page});

      // Keep track of the orbit triggers
      if (locIt->triggerWord & raw::sORB) {
        // auto ir = getTimeStamp(rofIt->interactionRecord);
        mTrigEvents[rofIt->interactionRecord] |= (1 << id);
        // mTriggeredEvents[id].emplace_back(elinkVec.size() - 1);
      }

      // Compute the masks
      if (locIt->triggerWord & raw::sSOX) {
        if (raw::isLoc(locIt->statusWord)) {
          auto maskItem = mMasks.find(locIt->boardId);
          // Check if we have already a mask for this
          if (maskItem == mMasks.end()) {
            // If not, read the map
            auto& mask = mMasks[locIt->boardId];
            for (int ich = 0; ich < 4; ++ich) {
              mask.patternsBP[ich] = locIt->patternsBP[ich];
              mask.patternsNBP[ich] = locIt->patternsNBP[ich];
            }
          }
        }
      }
    } // loop on local boards
  }   // loop on ROF records

  auto lastCompleteTrigEvent = getLastCompleteTrigEvent();
  if (lastCompleteTrigEvent == mTrigEvents.rend()) {
    // We might be still missing data from some boards.
    // Skip the test, waiting for more HBs
    return true;
  }

  auto lastCheckedIndexes = sortEvents(lastCompleteTrigEvent->first);
  bool isOk = checkEvents();
  clearChecked(lastCompleteTrigEvent->first, lastCheckedIndexes);
  return isOk;
}

void GBTRawDataChecker::clear()
{
  /// Resets the masks and flags
  mMasks.clear();
  mBusyFlag.clear();
  mStatistics.fill(0);
}

} // namespace mid
} // namespace o2
