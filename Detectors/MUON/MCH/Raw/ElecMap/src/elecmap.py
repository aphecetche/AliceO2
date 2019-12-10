#!/usr/bin/env python

import argparse
import os
import elecmap.excel
import elecmap.gencode
import elecmap.gs
import pandas as pd
import numpy as np

def _simplify_dataframe(df):

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
        df = df.append(elecmap.excel.is_valid_file(parser, ifile,args.sheet))

if args.gs_name:
    df = df.append(elecmap.gs.read_sheet(args.credentials,args.gs_name,args.sheet))

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

