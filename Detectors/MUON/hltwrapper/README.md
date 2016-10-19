# Testing the (HLT) preclustering device

## Preparatory steps (once) 

In order to avoid having the preclustering component compute its internal mapping all over again each time it is
started, the mapping must be created once for all, using the `dhlt-generate-binmapfile` executable :

```
dhlt-generate-binmapfile -cdbpath local://$HOME/alicesw/AliRoot/OCDB -run 0 -binmapfile binmapfile.dat
```

Get a digit file, e.g. :

```
curl -o merged.digits.MB.196099.root https://cernbox.cern.ch/index.php/s/u5qWkslrDvciLCH/download
```

You also _must_ set the `ALIHLT_HCDBDIR` to point to a valid OCDB, e.g. :

```
export ALIHLT_HCDBDIR=$HOME/alicesw/AliRoot/OCDB
```

## Execution

### Start digit reader

Launch, on one terminal, the Digit Reader

```bash
aliceHLTWrapperApp 'DigitReader' 1 -x --output type=push,size=10,method=bind,address=tcp://*:45000 --library libAliHLTMUON.so --component MUONDigitReader --parameter '-datafile merged.digits.MB.196099.root'
```

Hit 'r' to run it. Assuming this is the first device you launch, nothing should happen as no other device is asking for the data this one is producing.

### Start precluster finder

Get one (or several) cluster finder(s) running :

```bash
aliceHLTWrapperApp 'Cluster Finder' 1 -x --input type=pull,method=connect,size=10,address=tcp://localhost:45000 --library libAliHLTMUON.so --component MUONPreclusterFinder --parameter '-binmapfile binmapfile.dat'
```

Hit 'r' to run it. That should trigger the start of the Digit Reader in the other terminal.

### Misc

Alternatively you can also try to launch the Digit Inspector (in another terminal) :

```bash
mch-digit-inspector --source tcp://localhost:45000
```

Hit 'r' to run it. That should trigger the start of the Digit Reader and display a raw dump of the digits in the current terminal.
