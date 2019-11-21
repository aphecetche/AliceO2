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
    def insert_in_map(dsid,index):
        out.write("m.insert(std::make_pair(encodeDeDs({},{}),encodeSolarGroupIndex({},{},{})));\n"
                  .format(row.de_id,dsid,row.solar_id,row.group_id,index))
    insert_in_map(row.ds_id_0,0)
    insert_in_map(row.ds_id_1,1)
    if row.ds_id_2:
        insert_in_map(row.ds_id_2,2)
    if row.ds_id_3:
        insert_in_map(row.ds_id_3,3)
    if row.ds_id_4:
         insert_in_map(row.ds_id_4,4)

def cru2solar(df, out,deids):

    s = '''
    namespace {
    std::map<uint16_t,std::set<uint16_t>> createCru2SolarMap() {
    std::map<uint16_t,std::set<uint16_t>> m;
'''

    c = Template("  m[${cruid}].insert(${solarid});\n");

    cru2solar_dict = {}

    for row in df.itertuples():
        if row.de_id in deids:
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

def deid2cru(df,out,deids):

    s='''
    std::map<uint16_t,uint8_t> createDeId2CruIdMap() {
    std::map<uint16_t,uint8_t> m;
'''

    c = Template("  m[${deid}]=${cruid};\n");

    deid2cruid_dict = {}

    for row in df.itertuples():
        if row.de_id in deids:
            if not row.de_id in deid2cruid_dict:
                deid2cruid_dict[row.de_id] = row.cru_id

    for k, v in deid2cruid_dict.items():
        s += c.substitute(deid=k,cruid=v)

    s += "return m;}\n"
    out.write(s)

def generate_one_chamber_file(df,chamber,deids):
    out = open_generated("Gen{}.cxx".format(chamber))

    t = Template('''

                 #include <cstdint>
                 #include <map>

                 void fill${ch}(std::map<uint32_t,uint16_t>& m) {
                 ''')

    out.write(t.substitute(ch=chamber))

    for row in df.itertuples():
        if row.de_id in deids:
            insert_row_in_map(out,row)
    out.write("}")
    close_generated(out)


def do(df,deids):

    out = open_generated("GenElectronicMapper.cxx")

    out.write('''
              #include <map>
              #include <vector>
              #include <set>
              #include <utility>

              #include "MCHRawEncoder/ElectronicMapper.h"

                 uint32_t encodeDeDs(uint16_t a, uint16_t b) {
                 return a<<16|b;
                }
                 uint16_t decode_a(uint32_t x) {
                 return static_cast<uint16_t>((x & 0xFFFF0000) >> 16);
                }
                 uint16_t decode_b(uint32_t x) {
                 return static_cast<uint16_t>(x & 0xFFFF);
                }

                 uint16_t encodeSolarGroupIndex(uint16_t solarId, uint8_t
              groupId, uint8_t index) {
                   return (solarId&0x3FF) | ((groupId&0x7) << 10) |
              ((index&0x7) << 13);
                }

                uint16_t decodeSolarId(uint16_t code) {
              return code & 0x3FF;
             }

              uint8_t decodeGroupId(uint16_t code) {
              return (code & 0x1C00)>>10;
             }

              uint8_t decodeElinkIndex(uint16_t code) {
              return (code & 0xE000)>>13;
             }
              ''')

    deds2solargrp(df,out,deids)

    crus = cru2solar(df,out,deids)

    deid2cru(df,out,deids)
    
    out.write('''
              namespace o2::mch::raw {

              struct ElectronicMapperGeneratedImpl : public ElectronicMapper {

              DualSampaElectronicLocation
              dualSampaElectronicLocation(uint16_t deid, uint16_t dsid)
              const override
              {
              static std::map<uint32_t,uint16_t> m = createDeDsMap();
              auto it = m.find(encodeDeDs(deid,dsid));
              if (it==m.end()) {
              return DualSampaElectronicLocation::Invalid();
             }
              return
              DualSampaElectronicLocation{decodeSolarId(it->second),decodeGroupId(it->second),decodeElinkIndex(it->second)};
             }

              std::set<uint16_t> solarIds(uint8_t cruId) const override
              {
              static std::map<uint16_t,std::set<uint16_t>> m = createCru2SolarMap();
              auto it = m.find(cruId);
              if (it==m.end()) {
              return {};
             }
              return m[cruId];
             }

              uint8_t cruId(uint16_t deid) const override
              {
              static std::map<uint16_t,uint8_t> m = createDeId2CruIdMap();
              auto it = m.find(deid);
              if (it==m.end()) {
              return 0xFF;
             }
              return m[deid];
             }

              std::set<uint16_t> cruIds() const override {
              ''')
    out.write(" return {{ {} }};".format(crus))

    out.write('}};')


    out.write('''
template <>
std::unique_ptr<ElectronicMapper> createElectronicMapper<ElectronicMapperGenerated>()
{
  return std::make_unique<ElectronicMapperGeneratedImpl>();
}
              ''')

    out.write('}')

    close_generated(out)

def deds2solargrp(df,out,deids):

    chambers = { "ch5" }
    s = ""

    for chamber in chambers:
        out.write('#include "Gen{}.cxx"\n'.format(chamber))
        generate_one_chamber_file(df,chamber,deids)
        s += ' fill{}(m);\n'.format(chamber)
    
    out.write('''
              namespace {
    std::map<uint32_t,uint16_t> createDeDsMap() {
              std::map<uint32_t,uint16_t> m;
              ''')
    out.write(s)
    out.write(' return m;} }')

