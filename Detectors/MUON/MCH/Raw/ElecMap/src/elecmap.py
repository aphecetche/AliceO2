#!/usr/bin/env python

from oauth2client.service_account import ServiceAccountCredentials
from string import Template
from subprocess import call
import argparse
import gspread
import numpy as np
import os
import pandas as pd
import sys

def gencode_clang_format(filename):
    """ Run clang-format on file """
    lc = ["clang-format","-i",filename] 
    return call(lc)

def gencode_open_generated(filename):
    """ Open a new file and add a Copyright on it """
    out= open(filename,"w")
    gencode_generated_code(out)
    return out

def gencode_close_generated(out):
    """ Format and close """
    out.close()
    gencode_clang_format(out.name)

def gencode_generated_code(out):
    """ Add full O2 Copyright to out"""
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

def gencode_insert_row_in_map(out,row):
    def insert_in_map(dsid,index):
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

def gencode_do(df,chamber):
    """ Generate code for one chamber


    Information from the dataframe df is used to create c++ code that
    builds a couple of std::map
    """

    out = gencode_open_generated(chamber + ".cxx")


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
        gencode_insert_row_in_map(out,row)

    out.write("}")

    out.write("void fillSolar2Cru{}(std::map<uint16_t, uint16_t>& s2c){{".format(chamber))
    out.write("}")
    gencode_close_generated(out)

def gs_read_sheet(credential_file,workbook,sheet_name):
    """ Read a Google Spreadsheet 

    """

    scope = ['https://spreadsheets.google.com/feeds',
             'https://www.googleapis.com/auth/drive']

    credentials = ServiceAccountCredentials.from_json_keyfile_name(
             credential_file, scope) # Your json file here

    gc = gspread.authorize(credentials)

    wks = gc.open(workbook).worksheet(sheet_name)

    data = wks.get_all_values()

    cols = np.array([0,1,3,4,5,6,7,8,9,10,11,12,13,14])
    df = pd.DataFrame(np.asarray(data)[:,cols], columns=["cru","fiber","crate","solar",
                                      "solar_local_id","j","slat",
                                      "length","de",
                                      "ds1","ds2","ds3","ds4","ds5"])
    return df.iloc[3:]

def excel_get_dataframe(filename,sheet):
    """ Read a dataframe from an excel file """

    f = pd.read_excel(filename,sheet_name=sheet,
                      names=["cru","fiber","crate","solar",
                                      "solar_local_id","j","slat",
                                      "length","de",
                                      "ds1","ds2","ds3","ds4","ds5"],
                      usecols="A:N",
                      na_values=[" "],
                      na_filter=True)
    return f

def excel_is_valid_file(parser, arg, sheet):
    print(arg,sheet)
    if not os.path.isfile(arg):
        parser.error("The file %s does not exist!" % arg)
    else:
        return excel_get_dataframe(arg,sheet)


def _simplify_dataframe(df):
   """ Do some cleanup on the dataframe """

   # remove lines where only the "CRATE #" column is
   # different from NaN
   df = df[df.crate!=""]

   # row_list is a dictionary where we'll put only the information we need
   # from the input DataFrame (df)
   row_list = []

   for row in df.itertuples():
       #print(row)
       crate = int(str(row.crate).strip('C '))
       solar_pos = int(row.solar.split('-')[2].strip('S '))-1
       group_id = int(row.solar.split('-')[3].strip('J '))-1
       solar_id = crate*8 + solar_pos
       de_id = int(row.de)
       d = dict({
           'cru_id': row.cru,
           'solar_id': solar_id,
           'group_id': group_id,
           'de_id': de_id,
           'ds_id_0': int(row.ds1)
       })
       d['ds_id_1'] = int(row.ds2) if pd.notna(row.ds2) and len(row.ds2)>0 else 0
       d['ds_id_2'] = int(row.ds3) if pd.notna(row.ds3) and len(row.ds3)>0 else 0
       d['ds_id_3'] = int(row.ds4) if pd.notna(row.ds4) and len(row.ds4)>0 else 0
       d['ds_id_4'] = int(row.ds5) if pd.notna(row.ds5) and len(row.ds5)>0 else 0
       row_list.append(d)

   # create the output DataFrame (sf) from the row_list dict
   sf = pd.DataFrame(row_list,dtype=np.int16)

   return sf

parser = argparse.ArgumentParser()

parser.add_argument('--excel','-e', dest="excel_filename",
                    action="append",
                    help="input excel filename(s)")

parser.add_argument('--google_sheet','-gs', dest="gs_name",
                    help="input google sheet name")

parser.add_argument('-s','--sheet',
                    dest="sheet",
                    required=True,
                    help="name of the excel sheet to consider in the excel file")

parser.add_argument('-c','--chamber',
                    dest="chamber",
                    help="output c++ code for chamber")

parser.add_argument('--verbose','-v',
                    dest="verbose",default=False,action="store_true",
                    help="verbose")

parser.add_argument('--credentials',
                    dest="credentials",
                    help="json credential file for Google Sheet API access")

parser.add_argument("--fec_map","-f",
                    dest="fecmapfile",
                    help="fec.map output filename")

args = parser.parse_args()

df = pd.DataFrame()

if args.excel_filename:
    for ifile in args.excel_filename:
        df = df.append(excel_is_valid_file(parser, ifile,args.sheet))

if args.gs_name:
    df = df.append(gs_read_sheet(args.credentials,args.gs_name,args.sheet))

df = _simplify_dataframe(df)

if args.verbose:
  print(df.to_string())

if args.chamber:
    elecmap.gencode.do(df,args.chamber)

if args.fecmapfile:
    s = df.to_string(
              columns=["solar_id","group_id","de_id","ds_id_0","ds_id_1","ds_id_2","ds_id_3","ds_id_4"],
              header=False,
              index=False,
              formatters={
                  "solar_id": lambda x: "%-6s" % x,
                  "group_id": lambda x: "%2s" % x,
                  "de_id": lambda x: "%9s   " % x,
                  "ds_id_0": lambda x: " %-6s" % x,
                  "ds_id_1": lambda x: " %-6s" % x,
                  "ds_id_2": lambda x: " %-6s" % x,
                  "ds_id_3": lambda x: " %-6s" % x,
                  "ds_id_4": lambda x: " %-6s" % x,
              })
    out = open(args.fecmapfile,"w")
    out.write(s)

