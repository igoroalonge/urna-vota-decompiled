// ecourna-lib/ecourna/api/security/imacalgorithm.hpp and csiphashmac.hpp   (paths inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// RTTI: IMacAlgorithm : pattern::NonCopyable   (typeinfo @1114792, vtable @1114744: [0] 174 trivial dtor,
//                                               [1] func 325, [2] [3] __cxa_pure_virtual)
//       CSiphashMac : IMacAlgorithm             (typeinfo @1113620, vtable @1113604: [0] 174 trivial dtor,
//                                               [1] 144 operator delete, [2] DoMac 9498, [3] DoVerify 9497)
// Template-method pattern: the public non-virtual Mac() validates the arguments, then calls DoMac.
// The only object in the binary is a CSiphashMac built on the stack by comum::CCalculaCV (func 3700),
// the calculator of the BU's "Código Verificador" (docs/bu/codigo-verificador.md).
#pragma once

#include <vector>

#include "ecourna/api/pattern/noncopyable.hpp"
#include "ecourna/types.hpp"

namespace ecourna::api::security {

class IMacAlgorithm : public pattern::NonCopyable {
public:
    virtual ~IMacAlgorithm() = default;

    // imacalgorithm.cpp lines 32/35 - no out-of-line copy: inlined into func 3700.
    void Mac(const std::vector<uebyte>& dados, std::vector<uebyte>& mac, const std::vector<uebyte>& chave) const;
    // A public Verify counterpart probably exists in the source but is not linked.                   // ?

protected:
    virtual void DoMac(const std::vector<uebyte>& dados, std::vector<uebyte>& mac,
                       const std::vector<uebyte>& chave) const = 0;
    virtual bool DoVerify(const std::vector<uebyte>& dados, const std::vector<uebyte>& mac,
                          const std::vector<uebyte>& chave) const = 0;
};

// csiphashmac.hpp (path inferred)
class CSiphashMac final : public IMacAlgorithm {   // "final": DoVerify calls DoMac directly (devirtualized)   // ?
protected:
    void DoMac(const std::vector<uebyte>& dados, std::vector<uebyte>& mac,
               const std::vector<uebyte>& chave) const override;          // wasm func 9498 (lines 52, 59, 66)
    bool DoVerify(const std::vector<uebyte>& dados, const std::vector<uebyte>& mac,
                  const std::vector<uebyte>& chave) const override;       // wasm func 9497 (lines 79, 88)
};

} // namespace ecourna::api::security
