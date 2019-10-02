#ifndef O2_MCH_RAW_HEADER_RAWDATAHEADER_H
#define O2_MCH_RAW_HEADER_RAWDATAHEADER_H

// FIXME : get this from an authoritative source instead !

#include <cstdint>

namespace o2
{
namespace mch
{
namespace raw
{
typedef struct _RAWDataHeaderV4 {
  // 32-bits words

  union {
    uint32_t word3 = 0x00004004;
    //                     | | version 4
    //                     | header size 16x32 bit = 64 bytes
    struct {
      uint32_t version : 8;      /// bit 0 to 7: header version
      uint32_t headerSize : 8;   /// bit 8 to 15: header size
      uint32_t blockLength : 16; /// bit 16 to 31: block length
    };
  };

  union {
    uint32_t word2 = 0x00ffffff;
    struct {
      uint32_t feeId : 16;      /// bit 0 to 15: FEE id
      uint32_t priorityBit : 8; /// bit 16 to 23: priority bit
      uint32_t zero2 : 8;       /// bit 24 to 31: reserved
    };
  };

  union {
    uint32_t word1 = 0x0;
    struct {
      uint32_t offsetNextPacket : 16; /// bit 0 to 15: offset of next block
      uint32_t
        memorySize : 16; /// bit 16 to 31: size of block (in bytes) in memory
    };
  };

  union {
    uint32_t word0 = 0xffffffff;
    struct {
      uint32_t linkId : 8;        /// bit 0 to 7: link id (GBT channel number)
      uint32_t packetCounter : 8; /// bit 8 to 15: packet counter (increased at
                                  /// every packet received in the link)
      uint32_t cruId : 12;        /// bit 16 to 27: CRU id
      uint32_t dpwId : 4;         /// bit 28 to 31: data path wrapper id, used to
                                  /// identify one of the 2 CRU End Points
    };
  };

  union {
    uint32_t word7 = 0xffffffff;
    struct {
      uint32_t triggerOrbit; /// bit 0 to 31: TRG orbit
    };
  };

  union {
    uint32_t word6 = 0xffffffff;
    struct {
      uint32_t heartbeatOrbit; /// bit 0 to 31: HB orbit
    };
  };

  union {
    uint32_t word5 = 0x0;
    struct {
      uint32_t zero5; /// bit 0 to 31: reserved
    };
  };

  union {
    uint32_t word4 = 0x0;
    struct {
      uint32_t zero4; /// bit 0 to 31: reserved
    };
  };

  union {
    uint32_t word11 = 0x0;
    struct {
      uint32_t triggerBC : 12;   /// bit 0 to 11: TRG BC ID
      uint32_t zero11_0 : 4;     /// bit 12 to 15: reserved
      uint32_t heartbeatBC : 12; /// bit 16 to 27: HB BC ID
      uint32_t zero11_1 : 4;     /// bit 28 to 31: reserved
    };
  };

  union {
    uint32_t word10 = 0x0;
    struct {
      uint32_t triggerType : 32; /// bit 0 to 31: trigger types
    };
  };

  union {
    uint32_t word9 = 0x0;
    struct {
      uint32_t zero9; /// bit 0 to 31: reserved
    };
  };

  union {
    uint32_t word8 = 0x0;
    struct {
      uint32_t zero8; /// bit 0 to 31: reserved
    };
  };

  union {
    uint32_t word15 = 0x0;
    struct {
      uint32_t detectorField : 16; /// bit 0 to 15: detector field
      uint32_t par : 16;           /// bit 16 to 31: PAR
    };
  };

  union {
    uint32_t word14 = 0x0;
    struct {
      uint32_t stopBit : 8;       /// bit 0 to 7: stop bit
      uint32_t pagesCounter : 16; /// bit 8 to 23: pages counter
      uint32_t zero14 : 8;        /// bit 24 to 31: reserved
    };
  };

  union {
    uint32_t word13 = 0x0;
    struct {
      uint32_t zero13; /// bit 0 to 31: reserved
    };
  };

  union {
    uint32_t word12 = 0x0;
    struct {
      uint32_t zero12; /// bit 0 to 31: reserved
    };
  };

} RAWDataHeaderV4;

using RAWDataHeader = RAWDataHeaderV4;

} // namespace raw
} // namespace mch
} // namespace o2

#endif // ALICEO2_HEADER_RAWDATAHEADER_H
