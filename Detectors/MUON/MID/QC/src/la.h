#ifndef LA_H
#define LA_H

#include <gsl/span>
#include "MIDRaw/LocalBoardRO.h"
#include "DataFormatsMID/ROFRecord.h"

namespace la
{
using FeeId = uint8_t;
void debugHeader(FeeId feeId, gsl::span<const o2::mid::LocalBoardRO> localBoards,
                 gsl::span<const o2::mid::ROFRecord> rofRecords);
} // namespace la

#endif
