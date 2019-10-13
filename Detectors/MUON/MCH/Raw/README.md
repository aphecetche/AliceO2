\page refMUONMCHRaw MCH Raw

# MCH Raw Data CoDec

There are currently two data formats to be dealt with : the "bare" one coming out of the CRU when no user logic (UL) is used, or the "UL" one where user logic is actually used.

On the reading/consumer/decoding end, the choice of the decoder to be used is as follows :

~~~.cpp
#include "MCHRaw/Decoder.h"

bool chargeSumMode=false; // whether the sampa is in cluster sum mode or not

auto decode = o2::mch::raw::createBareDecoder(rdhHandler,channelHandler,chargeSumMode);

// or (when UL will be implemented)

auto decode = o2::mch::raw::createUserLogicDecoder(rdhHandler,channelHandler);

// get some memory buffer from somewhere ...
buffer = ... // container of uint32_t words

// decode that buffer
decode(buffer);

~~~

Note how the cluster sum mode has to be properly selected, as there is unfortunately no way to infer it from the data itself...

The RDH handler (of type o2::mch::raw::RawDataHeaderHandler) and channel handler (of type o2::mch::raw::SampaChannelHandler) can be defined as lambdas, regular free functions, or member functions of some class, etc...

~~~.cpp
auto rdhHandler = [](const RAWDataHeader& rdh) {
  // return true if the payload corresponding to this rdh must be decoded
  // or false otherwise
  return true;
};

auto channelHandler = [](uint8_t cruId, uint8_t linkId, uint8_t chip,
    uint8_t channel, o2::mch::raw::SampaCluster sc) {
  // do something with the cluster information here
};
~~~

## Bare data format

This format is basically the Sampa data format, serialized by the GBT.

The basis is a stream of bits coming from one dual sampa chip connected to one Elink. 40 Elinks are then grouped within one GBT link. One GBT word is 80 bits, meaning 2 bits per dual sampa are transmitted at each LHC clock.

### ElinkEncoder

The o2::mch::raw::ElinkEncoder class builds a stream of bits for the data of one dual sampa.

### GBT

## UL data format

## TODO

Still to be written/investigated/discussed :

- how to go from detection element-wise dsid to flex dsid (0..15) (and reverse)
