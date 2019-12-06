#!/bin/sh

srcdir=/Users/laurent/ownCloud/archive/2019/MRRTF/raw-data-encdec

for chamber in CH5R CH5L CH6R
do
   ./elecmap.py -i ${srcdir}/Mapping-${chamber}.xlsx -c ${chamber}
done

