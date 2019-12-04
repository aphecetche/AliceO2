#!/usr/bin/env python

import argparse
import os
import elecmap.excel
import elecmap.gencode
import pandas as pd

parser = argparse.ArgumentParser()

parser.add_argument('--infile','-i', dest="inputfiles", required=True,
                    action="append",
                    help="input excel filename(s)")

# , metavar="FILE",i
#                     type=lambda x: elecmap.excel.is_valid_file(parser,x))

parser.add_argument('--excel','-e', dest="excel",
                    help="output excel filename", metavar="FILE")

parser.add_argument('--cpp','-c++','-c++','--c++',
                    dest="code",default=False,action="store_true",
                    help="output c++ code")

parser.add_argument('--verbose','-v',
                    dest="verbose",default=False,action="store_true",
                    help="verbose")

args = parser.parse_args()

df = pd.DataFrame()

for ifile in args.inputfiles:
    df = df.append(elecmap.excel.is_valid_file(parser, ifile))

if args.verbose:
  print(df.to_string())

# save to an excel file if so desired
if args.excel:
    df.to_excel(args.excel)

ch6r = { 604,603,602,601,600,617,616,615,614 } # CH6-R starting from top

if args.code:
    elecmap.gencode.do(df,'ch6r',ch6r)
