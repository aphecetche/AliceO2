# MCH Raw Data CoDec

There is currently two data formats to be dealt with : the "bare" one coming out of the CRU when no user logic (UL) is used, or the "UL" one where user logic is actually used.

## Bare data format

This format is basically the Sampa data format, serialized by the GBT.

The basis is a stream of bits coming from one dual sampa chip connected to one Elink. 40 Elinks are then grouped within one GBT link. One GBT word is 80 bits, meaning 2 bits per dual sampa are transmitted at each LHC clock.

### ElinkEncoder

The `ELinkEncoder` class builds a stream of bits for the data of one dual sampa.

### GBT

## UL data format

## TODO

Still to be written/investigated/discussed :

- how to go from detection element-wise dsid to flex dsid (0..15) (and reverse)
