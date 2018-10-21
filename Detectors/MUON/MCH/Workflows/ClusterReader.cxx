// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "ClusterReader.h"
#include "Cluster_generated.h"

namespace o2
{
namespace mch
{
ClusterReader::ClusterReader(const std::string& filename) : mFileName(filename),
                                                            mInputFile(mFileName)
{
}

void ClusterReader::init(o2::framework::InitContext& context)
{
}
void ClusterReader::run(o2::framework::ProcessingContext& context)
{
  if (mInputFile.eof()) {
    return;
  }
  int bufSize{0};
  mInputFile.read(reinterpret_cast<char*>(&bufSize), sizeof(int));
  if (mInputFile.eof()) {
    return;
  }

  auto buf = std::make_unique<char[]>(bufSize);
  if (mInputFile.eof()) {
    return;
  }

  auto clusterDE = o2::mch::GetClusterDE(buf.get());

  int detElemId = clusterDE->detElemId();

  int nblocks = clusterDE->clusterTimeBlocks()->Length();

  std::cout << "DE" << detElemId << " " << nblocks << " blocks\n";

}
}
}

