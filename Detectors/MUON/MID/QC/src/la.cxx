#include "la.h"
#include <fmt/format.h>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace la
{

std::string asString(const o2::mid::ROFRecord& rof)
{
  return fmt::format("ORBIT {:10d} BX {:4d} TYPE {:1d} FIRST {:4d} N {:2d}",
                     rof.interactionRecord.orbit, rof.interactionRecord.bc,
                     rof.eventType,
                     rof.firstEntry,
                     rof.nEntries);
}

std::string asString(const o2::mid::LocalBoardRO& board)
{
  return fmt::format("{} {:d}", (o2::mid::raw::isLoc(board.statusWord) ? "LOC" : "REG"), board.boardId);
}

void debugHeader(FeeId feeId,
                 gsl::span<const o2::mid::LocalBoardRO> localBoards,
                 gsl::span<const o2::mid::ROFRecord> rofRecords)
{
  if (localBoards.size() != rofRecords.size()) {
    throw std::logic_error("bad luck");
  }
  auto nofLocals = std::count_if(localBoards.begin(), localBoards.end(),
                                 [](const o2::mid::LocalBoardRO& board) {
                                   return o2::mid::raw::isLoc(board.statusWord);
                                 });
  auto nofRegionals = localBoards.size() - nofLocals;
  std::cout << fmt::format("feeId {:4d} has {:4d} boards ({:4d} loc and {:4d} reg) and rof records\n", feeId, localBoards.size(), nofLocals, nofRegionals);
  size_t i{0};
  for (const auto& r : rofRecords) {
    std::cout << fmt::format("    {} {}\n", asString(r), asString(localBoards[i]));
    ++i;
  }
}
} // namespace la
