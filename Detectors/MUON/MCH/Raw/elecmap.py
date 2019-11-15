#!/usr/bin/env python

import argparse
import os
import elecmap.excel
import elecmap.gencode

parser = argparse.ArgumentParser()

parser.add_argument('--infile','-i', dest="dataframe", required=True,
                    help="input excel filename", metavar="FILE",
                    type=lambda x: elecmap.excel.is_valid_file(parser,x))

parser.add_argument('--excel','-e', dest="excel",
                    help="output excel filename", metavar="FILE")

parser.add_argument('--cpp','-c++','-c++','--c++',
                    dest="code",type=argparse.FileType('w', encoding='UTF-8'),
                    help="output c++ code")

parser.add_argument('--verbose','-v',
                    dest="verbose",default=False,action="store_true",
                    help="verbose")

args = parser.parse_args()

df = args.dataframe
if args.verbose:
  print(df.to_string())

# save to an excel file if so desired
if args.excel:
    df.to_excel(args.excel)

if args.code:
    elecmap.gencode.to_elec(df,args.code)
