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
    out.write('''///
              /// GENERATED CODE ! DO NOT EDIT !
              ///
              ''')

def insert_row_in_map(out,row):
    def insert_in_map(dsid):
        out.write("m.insert(std::make_pair(encode({},{}),encode({},{})));\n"
                  .format(row.de_id,dsid,row.solar_id,row.group_id))
    insert_in_map(row.ds_id_0)
    insert_in_map(row.ds_id_1)
    if row.ds_id_2:
        insert_in_map(row.ds_id_2)
    if row.ds_id_3:
        insert_in_map(row.ds_id_3)
    if row.ds_id_4:
         insert_in_map(row.ds_id_4)

def cru2solar(df, out):

    s = '''
    namespace {
    std::map<uint16_t,std::set<uint16_t>> createCru2SolarMap() {
    std::map<uint16_t,std::set<uint16_t>> m;
'''

    c = Template("  m[${cruid}].insert(${solarid});\n");

    cru2solar_dict = {}

    for row in df.itertuples():
        if not row.cru_id in cru2solar_dict:
            cru2solar_dict[row.cru_id] = set()
            cru2solar_dict[row.cru_id].add(row.solar_id)

    for k, v in cru2solar_dict.items():
        for sid in v:
            s += c.substitute(cruid=k,solarid=sid)

    s += "return m;}}\n"
    out.write(s)

    c = ""
    for k in cru2solar_dict.keys():
        c += "{},".format(k);
    
    return c

def generate_one_chamber_file(df,chamber,deids):
    out = open_generated("Gen{}.cxx".format(chamber))

    t = Template('''

                 #include <cstdint>
                 #include <map>

                 inline uint32_t encode(uint16_t a, uint16_t b) {
                 return a<<16|b;
                }

                 void fill${ch}(std::map<uint16_t,uint16_t>& m) {
                 ''')

    out.write(t.substitute(ch=chamber))

    for row in df.itertuples():
        if row.de_id in deids:
            insert_row_in_map(out,row)
    out.write("}")
    close_generated(out)


def do(df):

    out = open_generated("GenElectronicMapper.cxx")

    out.write('''
              #include <map>
              #include <vector>
              #include <set>
              #include <utility>

              #include "MCHRawEncoder/ElectronicMapper.h"
              ''')

    deds2solargrp(df,out)

    crus = cru2solar(df,out)

    out.write('''
              namespace o2::mch::raw {

              struct ElectronicMapperGeneratedImpl : public ElectronicMapper {

              std::pair<uint16_t, uint16_t>
              solarIdAndGroupIdFromDeIdAndDsId(uint16_t deid, uint16_t dsid)
              const override
              {
              static std::map<uint16_t,uint16_t> m = createDeDsMap();
              auto it = m.find(encode(deid,dsid));
              return *it;
             }

              std::set<uint16_t> solarIds(uint8_t cruId) const override
              {
              static std::map<uint16_t,std::set<uint16_t>> m = createCru2SolarMap();
              return m[cruId];
             }

              std::set<uint16_t> cruIds() const override {
              ''')
    out.write(" return {{ {} }};".format(crus))

    out.write('}};}')

    close_generated(out)

def deds2solargrp(df,out):

    chambers = { "ch5" }
    s = ""

    for chamber in chambers:
        out.write('#include "Gen{}.cxx"\n'.format(chamber))
        generate_one_chamber_file(df,chamber,{501,502,505})
        s += ' fill{}(m);\n'.format(chamber)
    
    out.write('''
              namespace {
    std::map<uint16_t,uint16_t> createDeDsMap() {
              std::map<uint16_t,uint16_t> m;
              ''')
    out.write(s)
    out.write(' return m;} }')

