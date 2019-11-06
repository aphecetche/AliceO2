#ifndef O2_MCH_RAW_RDH_MANIP_H
#define O2_MCH_RAW_RDH_MANIP_H

#include <cstdint>
#include <iostream>
#include <vector>
#include <gsl/span>
#include <string_view>
#include <functional>

namespace o2
{
namespace mch
{
namespace raw
{

template <typename RDH>
void assertRDH(const RDH& rdh);

template <typename RDH>
void appendRDH(std::vector<uint8_t>& buffer, const RDH& rdh);

template <typename RDH>
void appendRDH(std::vector<uint32_t>& buffer, const RDH& rdh);

template <typename RDH>
RDH createRDH(gsl::span<uint8_t> buffer);

template <typename RDH>
RDH createRDH(gsl::span<uint32_t> buffer);

template <typename RDH>
RDH createRDH(uint16_t cruId, uint8_t linkId, uint32_t orbit, uint16_t bunchCrossing, uint16_t payloadSize);

template <typename RDH>
bool isValid(const RDH& rdh);

template <typename RDH>
size_t rdhPayloadSize(const RDH& rdh);

template <typename RDH>
uint32_t rdhOrbit(const RDH& rdh);

template <typename RDH>
int countRDHs(gsl::span<uint8_t> buffer);

template <typename RDH>
int showRDHs(gsl::span<uint32_t> buffer);

template <typename RDH>
int showRDHs(gsl::span<uint8_t> buffer);

void dumpRDHBuffer(gsl::span<uint32_t> buffer, std::string_view indent);

template <typename RDH>
int forEachRDH(gsl::span<uint32_t> buffer, std::function<void(RDH&)> f);

template <typename RDH>
int countRDHs(gsl::span<uint32_t> buffer);

template <typename RDH>
int countRDHs(gsl::span<uint8_t> buffer);

} // namespace raw
} // namespace mch
} // namespace o2

#endif
