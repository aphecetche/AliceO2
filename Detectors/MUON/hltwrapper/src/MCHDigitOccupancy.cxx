#include "MCHDigitOccupancy.h"
#include "FairMQMessage.h"
#include <cassert>

namespace AliceO2 {
namespace MUON {

MCHDigitOccupancy::MCHDigitOccupancy(const std::vector<uint32_t>& manuAbsId, const std::vector<uint8_t>& npad) : mManuAbsId(manuAbsId), mNumberOfPadsInManu(npad), mManuAbsIdToIndex(), mManuOccupancy(), mManuHit(), mNumberOfReceivedEvents(0)
{
    assert(mManuAbsId.size()<=16828);

    int ix(0);

    for ( auto m : mManuAbsId )
    {
        mManuAbsIdToIndex[m] = ix;
        ++ix;
    }

    mManuOccupancy.resize(mManuAbsId.size(),0);
    mManuHit.resize(mManuAbsId.size(),0);

    OnData("data-in",&MCHDigitOccupancy::HandleData);
}

bool MCHDigitOccupancy::HandleData(FairMQMessagePtr& input, int /*index*/)
{
    constexpr int FIRST_DIGIT_SIZE_POS=100;

    if ( input->GetSize() < FIRST_DIGIT_SIZE_POS ) return false;

    ++mNumberOfReceivedEvents;

    void* data = input->GetData();

    //FairMQMessagePtr outputMsg = new FairMQMessage();

    uint8_t* i8data = reinterpret_cast<uint8_t*>(data);

    uint32_t* i32data = reinterpret_cast<uint32_t*>(i8data+FIRST_DIGIT_SIZE_POS);
    int size = i32data[0];
    int offset(0);

    for ( int i = 0; i < size; ++i )
    {
        ++offset;
        uint32_t id = i32data[offset];
        uint16_t detElemId = id & 0x00000FFF;
        uint16_t manuId    = ( id & 0x00FFF000) >> 12;
        ++offset;
        // uint32_t ixadc = i32data[offset];
        // uint16_t adc = ( ixadc & 0xFFFF0000 ) >> 16;
        // uint16_t ix = ixadc & 0x0000FFFF;
        uint32_t manuIndex = mManuAbsIdToIndex[id & 0xFFFFFF];
        mManuHit[manuIndex]++;
    }

    if ( mNumberOfReceivedEvents % mNumberOfRequiredEventsForComputation == 0 )
    {
        LOG(WARN) << "Switching to new occupancy";
        ComputeOccupancy();
        DumpOccupancy();
        mManuHit.resize(mManuHit.size(),0);
    }

    return true;
}

void MCHDigitOccupancy::ComputeOccupancy()
{
    int ix(0);
    for (auto m: mManuHit)
    {
        float occ = m/(1.0*mNumberOfRequiredEventsForComputation*mNumberOfPadsInManu[ix]);
        mManuOccupancy[ix]=occ;
        ++ix;
    }
}

void MCHDigitOccupancy::DumpOccupancy(float threshold)
{
    int ix(0);
    for (auto m : mManuOccupancy)
    {
        if  ( m > threshold )
        {
            std::cout << std::setw(6) << ix 
                << std::setw(10) << mManuHit[ix]
                << " " << (int)mNumberOfPadsInManu[ix] <<  " " 
                << std::setw(10) << mNumberOfRequiredEventsForComputation
                << std::setw(10) << mNumberOfReceivedEvents << " " 
                << std::setprecision(2) << 100*m << " %" << std::endl;
        }
        ++ix;
    }
}

void MCHDigitOccupancy::ResetTask()
{
    ComputeOccupancy();
    DumpOccupancy();

    mManuOccupancy.resize(mManuOccupancy.size(),0);
    mManuHit.resize(mManuHit.size(),0);
    mNumberOfReceivedEvents = 0;
}
}
}
