import os
import pandas as pd
import numpy as np

def _simplify_dataframe(df):

   # remove lines where only the "CRATE #" column is
   # different from NaN
   df = df[df.count(1)>1]

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
           'ds_id_0': int(row.ds1),
           'ds_id_1': int(row.ds2)
       })
       d['ds_id_2'] = int(row.ds3) if pd.notna(row.ds3) else 0
       d['ds_id_3'] = int(row.ds4) if pd.notna(row.ds4) else 0
       d['ds_id_4'] = int(row.ds5) if pd.notna(row.ds5) else 0
       row_list.append(d)

   # create the output DataFrame (sf) from the row_list dict
   sf = pd.DataFrame(row_list,dtype=np.int16)

   return sf

def get_dataframe(filename):
    f = pd.read_excel(filename,names=["cru","fiber","crate","solar",
                                      "solar_local_id","j","slat",
                                      "length","de",
                                      "ds1","ds2","ds3","ds4","ds5"],
                      usecols="A:N",
                      na_values=[" "],
                      na_filter=True)
    return _simplify_dataframe(f)

def is_valid_file(parser, arg):
    if not os.path.isfile(arg):
        parser.error("The file %s does not exist!" % arg)
    else:
        return get_dataframe(arg)

