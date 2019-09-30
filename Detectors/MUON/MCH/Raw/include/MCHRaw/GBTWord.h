// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef O2_MCH_RAW_ELINK_GBTWORD_H
#define O2_MCH_RAW_ELINK_GBTWORD_H

#include <boost/multiprecision/cpp_int.hpp>
namespace bmp = boost::multiprecision;
typedef bmp::number<bmp::cpp_int_backend<80, 80, bmp::unsigned_magnitude, bmp::checked, void>> GBTWord;
#endif
