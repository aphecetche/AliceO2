from string import Template

def to_elec(df, out):

    s = '''// 
/// GENERATED CODE ! DO NOT EDIT !
///
#include <map>
#include <vector>

uint32_t code(uint16_t a, uint16_t b) {
  return a<<16|b;
}

std::map<int, int> toElec(const std::vector<std::vector<int>>& dedssolgroup)
{
    std::map<int, int> m;
    for (auto& v : dedssolgroup) {
        m.insert(std::make_pair(code(v[0], v[1]), code(v[2], v[3])));
    }
    return m;
}

std::vector<std::vector<int>> dedssolgroup_ch5 = {
'''

    t = Template("   {${deid},${dsid},${solarid},${groupid}},\n")

    for row in df.itertuples():
        s += t.substitute(deid=row.de_id,dsid=row.ds_id_0,solarid=row.solar_id,groupid=row.group_id)
        s += t.substitute(deid=row.de_id,dsid=row.ds_id_1,solarid=row.solar_id,groupid=row.group_id)
        if row.ds_id_2:
          s += t.substitute(deid=row.de_id,dsid=row.ds_id_2,solarid=row.solar_id,groupid=row.group_id)
        if row.ds_id_3:
          s += t.substitute(deid=row.de_id,dsid=row.ds_id_3,solarid=row.solar_id,groupid=row.group_id)
        if row.ds_id_4:
          s += t.substitute(deid=row.de_id,dsid=row.ds_id_4,solarid=row.solar_id,groupid=row.group_id)

    s.rstrip(",\n");
    s += '''
};

std::map<int,int> toElec() {
   static std::map<int,int> m(toElec(dedssolgroup_ch5));
   return m;
}
'''

    out.write(s)
