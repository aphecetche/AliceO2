# Testing

## Interactive mode

Interactivity is obtained, for the `aliceHLTWrapper` executable, by using the `-x` option

Get a digit file, e.g. :

```
curl -o merged.digits.MB.196099.root https://cernbox.cern.ch/index.php/s/u5qWkslrDvciLCH/download
```

You _must_ set the `ALIHLT_HCDBDIR` to point to a valid OCDB, e.g. :

```
export ALIHLT_HCDBDIR=/Users/laurent/o2/o2work/AliRoot/OCDB
```

Launch, on one terminal, the Digit Reader

```bash
aliceHLTwrapper 'DigitReader' 1 -x --output type=push,size=10,method=bind,address=tcp://*:45000 --library libAliHLTMUON.so --component MUONDigitReader --parameter '-datafile merged.digits.MB.196099.root'
```

Hit 'r' to run it. Assuming this is the first device you launch, nothing should happen as no other device is asking for the data this one is producing.

Get one (or several) cluster finder(s) running :

```bash
aliceHLTwrapper 'Cluster Finder' 1 -x --input type=pull,method=connect,size=10,address=tcp://localhost:45000 --library libAliHLTMUON.so --component MUONPreclusterFinder --parameter '-cdbpath local://$ALICE_ROOT/OCDB -run 169099'
```

Hit 'r' to run it. That should trigger the start of the Digit Reader in the other terminal.

 and display a raw dump of the digits in the current terminal.

Alternatively you can also try to launch the Digit Inspector (in another terminal) :

```bash
mch-digit-inspector --source tcp://localhost:45000
```

Hit 'r' to run it. That should trigger the start of the Digit Reader and display a raw dump of the digits in the current terminal.
