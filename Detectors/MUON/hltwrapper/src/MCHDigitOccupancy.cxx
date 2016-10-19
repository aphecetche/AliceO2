#include "MCHDigitOccupancy.h"
#include "FairMQMessage.h"

namespace AliceO2 {
namespace MUON {
void MCHDigitOccupancy::Run()
{
    static int c(0);

    while (CheckCurrentState(RUNNING) && c < 10)
    {
        ++c;
        std::unique_ptr<FairMQMessage> input(NewMessage());

        if ( Receive(input,"data-in") )
        {
            std::cout << "Msg size=" << input->GetSize()
                << std::endl;

            void* data = input->GetData();

            uint8_t* i8data = reinterpret_cast<uint8_t*>(data);

            uint32_t* i32data = reinterpret_cast<uint32_t*>(i8data+100);

            int size = i32data[0];
            int offset(0);

            std::cout << "size=" << size << std::endl;

            for ( int i = 0; i < size; ++i )
            {
                ++offset;
                uint32_t id = i32data[offset];
                uint16_t detElemId = id & 0x00000FFF;
                uint16_t manuId    = ( id & 0x00FFF000) >> 12;
                ++offset;
                uint32_t ixadc = i32data[offset];
                uint16_t adc = ( ixadc & 0xFFFF0000 ) >> 16;
                uint16_t ix = ixadc & 0x0000FFFF;
                std::cout << "DE " << detElemId << " MANU " << manuId << " (" << ix << " " << adc << ") "; 
            }

            std::cout << std::endl;
        }
    }

    WaitForEndOfState(RUNNING);
    ChangeState(END);
}
}
}
