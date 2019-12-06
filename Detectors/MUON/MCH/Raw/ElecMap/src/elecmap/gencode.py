from string import Template
import sys
from subprocess import call

def clang_format(filename):
    lc = ["clang-format","-i",filename] 
    return call(lc)

def open_generated(filename):
    out= open(filename,"w")
    generated_code(out)
    return out

def close_generated(out):
    out.close()
    clang_format(out.name)

def generated_code(out):
    out.write('''// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

              ///
              /// GENERATED CODE ! DO NOT EDIT !
              ///
              ''')

def insert_row_in_map(out,row):
    def insert_in_map(dsid,index):
        # out.write("e2d.emplace(encode(DsElecId({},{},{})),encode(DsDetId({},{})));\n"
        #           .format(row.solar_id,row.group_id,index,row.de_id,dsid))
        out.write("add(e2d,{},{},{},{},{});\n"
                  .format(row.de_id,dsid,row.solar_id,row.group_id,index))
    insert_in_map(row.ds_id_0,0)
    insert_in_map(row.ds_id_1,1)
    if row.ds_id_2:
        insert_in_map(row.ds_id_2,2)
    if row.ds_id_3:
        insert_in_map(row.ds_id_3,3)
    if row.ds_id_4:
         insert_in_map(row.ds_id_4,4)

def do(df,chamber):

    out = open_generated(chamber + ".cxx")


    out.write('''
              #include <map>
              #include <cstdint>
              #include "MCHRawElecMap/DsElecId.h"
              #include "MCHRawElecMap/DsDetId.h"
              using namespace o2::mch::raw;

              namespace {
              void add(std::map<uint16_t,uint32_t>& e2d, int deId, int dsId,
              uint16_t solarId, uint8_t groupId, uint8_t index) {
              e2d.emplace(encode(DsElecId(solarId,groupId,index)),encode(DsDetId(deId,dsId)));
             }
             }
              ''')

    out.write("void fillElec2Det{}(std::map<uint16_t,uint32_t>& e2d){{".format(chamber))

    for row in df.itertuples():
        insert_row_in_map(out,row)

    out.write("}")

    out.write("void fillSolar2Cru{}(std::map<uint16_t, uint16_t>& s2c){{".format(chamber))
    out.write("}")
    close_generated(out)
