import os
import pandas as pd
import numpy as np
import gspread
from oauth2client.service_account import ServiceAccountCredentials
import pandas as pd

def read_sheet(credential_file,workbook,sheet_name):
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
