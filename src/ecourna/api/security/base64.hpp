// ecourna-lib/ecourna/api/security/base64.hpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u01.
// Only the encoder is linked into the web build (no decoder string, table or code is present).
#pragma once

#include <string>
#include <vector>

namespace ecourna::api::security {

// Standard Base64 (RFC 4648 alphabet, '=' padding, no line breaks).
// Throws CSecurityError 1325 on empty input, 1327 if the conversion fails.
std::string EncodeBase64(const std::vector<uebyte>& dados);   // srcloc base64.cpp:247/267, body inlined into func 3703

} // namespace ecourna::api::security
