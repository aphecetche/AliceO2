\page refMUONMCHRaw MCH Raw

# MCH Raw Data CoDec

There are currently two data formats to be dealt with : the "bare" one coming out of the CRU when no user logic (UL) is used, or the "UL" one where user logic is actually used.

On the reading/consumer/decoding end, the choice of the decoder to be used is as follows :

```.cpp
#include "MCHRaw/Decoder.h"

bool chargeSumMode=false; // whether the sampa is in cluster sum mode or not

auto decode = o2::mch::raw::createBareDecoder(rdhHandler,channelHandler,chargeSumMode);

// or (when UL will be implemented)

auto decode = o2::mch::raw::createUserLogicDecoder(rdhHandler,channelHandler);

// get some memory buffer from somewhere ...
buffer = ... // container of uint32_t words

// decode that buffer
decode(buffer);

```

Note how the cluster sum mode has to be properly selected, as there is unfortunately no way to infer it from the data itself...

The RDH handler (of type o2::mch::raw::RawDataHeaderHandler) and channel handler (of type o2::mch::raw::SampaChannelHandler) can be defined as lambdas, regular free functions, or member functions of some class, etc...

```.cpp
auto rdhHandler = [](const RAWDataHeader& rdh) {
  // return true if the payload corresponding to this rdh must be decoded
  // or false otherwise
  return true;
};

auto channelHandler = [](uint8_t cruId, uint8_t linkId, uint8_t chip,
    uint8_t channel, o2::mch::raw::SampaCluster sc) {
  // do something with the cluster information here
};
```

## Bare data format

This format is the raw Sampa data format, serialized by the GBT.

### Encoding

Given a o2::mch::raw::SampaCluster for one given channel, the o2::mch::raw::CRUEncoder class dispatch it to one of its 24 o2::mch::raw::GBTEncoder, which in turns dispatch it to one of its 40 o2::mch::raw::ElinkEncoder. The o2::mch::raw::ElinkEncoder class builds a stream of bits corresponding to the SampaCluster.
Then to build 80-bits GBT words the bitstreams of 40 ElinkEncoders are aligned (i.e. brought to the same length: the one of the widest elink in the group) by a GBTEncoder. Finally the GBT words from 24 GBTs are bundled together by the CRUEncoder.

### Decoding

A data buffer (basically corresponding to one RDH) of 32-bits words is given to a o2::mch::raw::CRUBareDecoder which dispatch it to one of its 24 o2::mch::raw::GBTDecoder. The buffer is dispatched 4 words at a time to reach 128 bits GBT words (out of which 80 are usefull data bits). The GBTDecoder then dispatches the 2 x 40 bits to its 40 o2::mch::raw::ElinkDecoder. Each ElinkDecoder then processes its own bitstream, based on the expected Sampa protocol, i.e. search first for a 50-bits SYNC word, then a sequence of header+payload pairs, where header is 50-bits and the payload size (in multiple of 10 bits) is given in the header.

## UL data format

## TODO

Still to be written/investigated/discussed :

- how to go from detection element-wise dsid to flex dsid (0..15) (and reverse)

## Generation of electronic mapping

From a reference Excel file :

```
./elecmap.py -i Mapping-CH5L.xlsx -c elecmap.cxx -e elecmap.xlsx
```

The (optional) `elecmap.xlsx` output file contains the same information as `elecmap.cxx`
