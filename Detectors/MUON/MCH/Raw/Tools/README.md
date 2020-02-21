<!-- doxy
\page refDetectorsMUONMCHRawTools Tools
/doxy -->

# MCH RawData Tools

## Raw Dump

The `o2-mch-rawdump` executable reads events from a raw data file and computes
the mean and sigma of the data of each channel.

```bash
> o2-mch-rawdump -h
Generic options:
  -h [ --help ]           produce help message
  -i [ --input-file ] arg input file name
  -n [ --nrdhs ] arg      number of RDHs to go through
  -s [ --showRDHs ]       show RDHs
  -u [ --userLogic ]      user logic format
  -c [ --chargeSum ]      charge sum format
  -j [ --json ]           output means and rms in json format
  -d [ --de ] arg         detection element id of the data to be decoded
  --cru arg               force cruId

> o2-mch-rawdump -i $HOME/cernbox/o2muon/data-DE617.raw -j -n 10000 -cru 0
```

Note that options `-j` and `-s` are incompatible (as the rdh output is not in
json).

## Raw Plot

The `o2-mch-rawplot` can be used to make SVG plots from mean and sigma values of channel data.
Typical usage is to pipe it from the output of `o2-mc-rawdump` like so :

    o2-mch-rawdump -i $HOME/cernbox/o2muon/data-DE617.raw -j -n 10000 -o 432 | stage/bin/o2-mch-rawplot > svg.html
