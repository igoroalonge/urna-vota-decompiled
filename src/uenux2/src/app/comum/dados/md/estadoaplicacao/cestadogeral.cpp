// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeral.cpp
#include "cestadogeral.h"

#include "api/pkcs11/ipkcs11.h"

namespace comum::md::estadoaplicacao {

// wasm func 5635 (srcloc line 147)
// The urna's certificate (read once from the PKCS#11 token and cached in a function-level static
// vector @1839076). Used by comum_f5634 (BU QR-code generation: number of QR codes =
// ceil(2*len(certificate)/1082) when the certificate is included) and by CGeraBU::StartState.
// Web build: no class implementing api::pkcs11::IPkcs11 exists in the binary (no RTTI, no
// CPolySingletonList::push for it: the typeid name "N3api6pkcs117IPkcs11E" is referenced only by the
// lookup 3704), so the lookup throws EPatternError 1301
// "PolySingleton - solicitada uma instancia nao criada N3api6pkcs117IPkcs11E [<file>:147]" where
// <file> is the full path of cestadogeral.cpp (the std::source_location passed by this function).
std::vector<uebyte> CEstadoGeral::RecuperarCertificado()
{
    static std::vector<uebyte> certificado;   // @1839076
    if (certificado.empty()) {
        // CPolySingletonList::instance<IPkcs11>(registry, std::source_location::current()) (3704)
        auto& pkcs11 = api::CPolySingletonList::instance<api::pkcs11::IPkcs11>();
        pkcs11.AbreSessao();                               // ? vtable slot 23
        const auto lido = pkcs11.LeCertificado();          // ? vtable slot 10 (returns a byte vector)
        certificado.assign(lido.begin(), lido.end());      // comum_f1681 = vector::assign(first, last, n)
        pkcs11.FechaSessao();                              // ? vtable slot 24 (not reached if slot 10 throws)
    }
    return certificado;
}

}  // namespace comum::md::estadoaplicacao
