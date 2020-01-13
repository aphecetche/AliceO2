# MCH Raw Data Tools

## Raw Dump

The `o2-mch-rawdump` executable reads events from a raw data and computes the mean and sigma of the data of each channel.

## Raw Plot

The `o2-mch-rawplot` can be used to make SVG plots from mean and sigma values of channel data.
Typical usage is to pipe it from the output of `o2-mc-rawdump` like so :

```
o2-mch-rawdump -i $HOME/cernbox/o2muon/data-DE617.raw -j -n 10000 -o 432 | stage/bin/o2-mch-rawplot > svg.html
```
