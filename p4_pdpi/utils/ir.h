// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef P4_PDPI_UTILS_IR_H
#define P4_PDPI_UTILS_IR_H

#include "gutil/status.h"
#include "p4_pdpi/ir.pb.h"

#if !defined(ABSL_IS_LITTLE_ENDIAN)
#error \
    "Some of the utility functions are not supported in Big Endian architecture"
#endif

namespace pdpi {

const uint32_t kNumBitsInByte = 8;
const uint32_t kNumBitsInMac = 48;
const uint32_t kNumBytesInMac = kNumBitsInMac / kNumBitsInByte;
const uint32_t kNumBitsInIpv4 = 32;
const uint32_t kNumBytesInIpv4 = kNumBitsInIpv4 / kNumBitsInByte;
const uint32_t kNumBitsInIpv6 = 128;
const uint32_t kNumBytesInIpv6 = kNumBitsInIpv6 / kNumBitsInByte;

// Returns the format for value, given the annotations on it, it's bitwidth
// and named type (if any).
gutil::StatusOr<Format> GetFormat(const std::vector<std::string> &annotations,
                                  const int bitwidth, bool is_sdn_string);

// Checks if the IrValue in the IR table entry is in the same format as
// specified in the P4Info.
absl::Status ValidateIrValueFormat(const IrValue &ir_value,
                                   const Format &format);

// Converts the IR value to a PI byte string and returns it.
gutil::StatusOr<std::string> IrValueToByteString(const IrValue &ir_value);

// Converts the PI value to an IR value and returns it.
gutil::StatusOr<IrValue> FormatByteString(const Format &format,
                                          const int bitwidth,
                                          const std::string &pi_value);

// Returns an IrValue based on a string value and a format. The value is
// expected to already be formatted correctly, and is just copied to the correct
// oneof field.
gutil::StatusOr<IrValue> FormattedStringToIrValue(const std::string &value,
                                                  Format format);

// Returns a string of length ceil(expected_bitwidth/8).
gutil::StatusOr<std::string> Normalize(const std::string &pi_byte_string,
                                       int expected_bitwidth);

// Convert the given byte string into a uint value.
gutil::StatusOr<uint64_t> PiByteStringToUint(const std::string &pi_bytes,
                                             int bitwidth);

// Convert the given uint to byte string.
gutil::StatusOr<std::string> UintToPiByteString(uint64_t value, int bitwidth);

// Convert the given byte string into a : separated MAC representation.
// Input string should be 6 bytes long.
gutil::StatusOr<std::string> PiByteStringToMac(
    const std::string &normalized_bytes);

// Convert the given : separated MAC representation into a byte string.
gutil::StatusOr<std::string> MacToPiByteString(const std::string &mac);

// Convert the given byte string into a . separated IPv4 representation.
// Input should be 4 bytes long.
gutil::StatusOr<std::string> PiByteStringToIpv4(
    const std::string &normalized_bytes);

// Convert the given . separated IPv4 representation into a byte string.
gutil::StatusOr<std::string> Ipv4ToPiByteString(const std::string &ipv4);

// Convert the given byte string into a : separated IPv6 representation.
// Input should be 16 bytes long.
gutil::StatusOr<std::string> PiByteStringToIpv6(
    const std::string &normalized_bytes);

// Convert the given : separated IPv6 representation into a byte string.
gutil::StatusOr<std::string> Ipv6ToPiByteString(const std::string &ipv6);

// Returns the number of bits used by the PI byte string interpreted as an
// unsigned integer.
uint32_t GetBitwidthOfPiByteString(const std::string &input_string);

}  // namespace pdpi
#endif  // P4_PDPI_UTILS_IR_H
