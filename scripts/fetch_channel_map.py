import pandas
import argparse 
import datetime
import os

tz = datetime.timezone.utc
ft = "%Y-%m-%dT%H:%M:%S%z"
current_time = datetime.datetime.now(tz=tz).strftime(ft)


parser = argparse.ArgumentParser(
                    prog='ChannelMapLoader',
                    description='Fetches the channel map from the google sheet and saves to json')
parser.add_argument('--sheet', default='1GrxYfy7o9omllbPBwJVsCvUxHIRMJQymIacTE2mFLqc')
parser.add_argument('--outfile', default='test.json')
parser.add_argument('--nan-json-value', help='What value to set all nans to in the json', default = int(-1e9))
parser.add_argument('-v', '--verbose',
                    action='store_true')  # on/off flag
args = parser.parse_args()

#Create a public URL
# https://docs.google.com/spreadsheets/d/1GrxYfy7o9omllbPBwJVsCvUxHIRMJQymIacTE2mFLqc/edit?usp=sharing
#get spreadsheets key from url
gsheetkey = args.sheet

#sheet name
sheet_name = 'Cable Layout'

url=f'https://docs.google.com/spreadsheet/ccc?key={gsheetkey}&output=xlsx'
df = pandas.read_excel(url,sheet_name=sheet_name)
# print(df)

rename_map = {
    "Crate":"crateNum",
    "AMC/WFD5":"amcSlotNum",
    "AMC Channel":"channelNum",
    "Detector System":"detectorSystem",
    "Subdetector":"subdetector",
}


dicti = {
    'time':f'{current_time}',
    'channelMap':[],
}
for i, row in df.iterrows():
    if pandas.isna(row['Subdetector']):
        continue
    detector = row['Subdetector'].strip()
    enabled = int(row['Enabled']) == 1.0
    if args.verbose:
        print('Found row:', row)
    if enabled:
        outrow = {}
        for key,val in row.to_dict().items():
            outrow[rename_map.get(key,key)] = val if not pandas.isna(val) else args.nan_json_value
        if args.verbose:
            print('Parsed to:', outrow)
            print('    -> Enabled!')
        dicti['channelMap'].append(outrow)


import json 
outfile = args.outfile
with open(outfile, 'w') as fout:
    json.dump(dicti, fout, indent=1)