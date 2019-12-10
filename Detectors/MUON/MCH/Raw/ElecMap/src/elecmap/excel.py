import os
import pandas as pd
import numpy as np

def get_dataframe(filename,sheet):
    f = pd.read_excel(filename,sheet_name=sheet,
                      names=["cru","fiber","crate","solar",
                                      "solar_local_id","j","slat",
                                      "length","de",
                                      "ds1","ds2","ds3","ds4","ds5"],
                      usecols="A:N",
                      na_values=[" "],
                      na_filter=True)
    return f

def is_valid_file(parser, arg, sheet):
    print(arg,sheet)
    if not os.path.isfile(arg):
        parser.error("The file %s does not exist!" % arg)
    else:
        return get_dataframe(arg,sheet)

