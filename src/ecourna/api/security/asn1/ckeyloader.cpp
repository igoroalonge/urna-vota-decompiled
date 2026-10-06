// ecourna-lib/ecourna/api/security/asn1/ckeyloader.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/asn1/ckeyloader.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u01.
//
// Only one function of this file survives in the binary: DecipherKeyIfNeeded<EntidadeChave> (func 9473,
// throw at line 78). The rest of the loading sequence was inlined into its caller, see the note at the end.
#include "ecourna/api/security/asn1/ckeyloader.hpp"

#include <vector>

#include "ModuloEnvelopeChave.h"                      // III ASN.1 generated classes (EntidadeChave)
#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/api/security/esecurityerror.hpp"   // ESecurityError; CSecurityError = exception::CBaseError<ESecurityError, SErrorLimits{1325, 1725}> (alias name inferred)

namespace ecourna::api::security {

// wasm func 9473 (srcloc line 78)
//
// III ASN.1 layout used: EntidadeChave is a SEQUENCE whose field vector is at +8;
//   fields[2] = cifrado (BOOLEAN, value byte at +8), fields[4] = chave (OCTET STRING, std::vector<char> at +8).
template <typename T>
std::vector<uebyte> CKeyLoader::DecipherKeyIfNeeded(const T& entidade)
{
    const bool cifrado = entidade.get_cifrado();
    const std::vector<uebyte> chave(entidade.get_chave().begin(), entidade.get_chave().end());

    if (!cifrado) {
        return chave;   // a const local: the wasm makes a second copy instead of moving it
    }
    if (!m_cifrador) {
        throw CSecurityError(ESecurityError::CifradorNecessario /* 1469 */,      // line 78, name inferred
                             "Um TSharedSymmetricCipher é necessário para decifrar chaves.");
    }
    std::vector<uebyte> decifrada;
    m_cifrador->Decrypt(chave, decifrada);   // ISymmetricCipher vtable slot 3
    return decifrada;
}

template std::vector<uebyte>
CKeyLoader::DecipherKeyIfNeeded<ModuloEnvelopeChave::EntidadeChave>(const ModuloEnvelopeChave::EntidadeChave&);

// ---------------------------------------------------------------------------------------------------------
// How the only caller uses it (inlined into vota::CGeraBU::StartState, func 12110, through
// comum::util::(anonymous)::LeChave(const std::filesystem::path&), util.cpp:37; NOT part of this file):
//
//   path    = "/dsk/fi/estatico/chave/" / "cv.ber.pri";                 // func 1948 + "cv.ber.pri"
//   segredo = CPolySingletonList::instance<api::IKernelHSM>().<vf3>();  // bytes from the HSM driver
//   CSymmetricCipherFactory fabrica;                                    // a local object (vtable @1113840)
//   auto cifrador = fabrica.<vf2>(std::string(segredo.begin(), segredo.end()));
//                   // = CBlockCipher<CAesCipher,CTrng>, AES-256-CBC, key = SHA-512(segredo)[0..32), no IV
//   CKeyLoader loader(cifrador);
//   ModuloEnvelopeChave::EntidadeChave entidade;
//   ecourna::api::io::DeserializeFromBuffer(entidade, conteudoDoArquivo);   // func 2280, BER decode
//   auto bytes = loader.DecipherKeyIfNeeded(entidade);
//   CKey chave(descritor.nomeUsuario, descritor.serial, tipo /*0 secreta, 1 publica, else -1*/, bytes, tagChaves);
//   std::fill(bytes.begin(), bytes.end(), 0);                           // the key copy is wiped
//   ... the first 16 bytes of the key are copied into comum::CCalculaCV (the "código verificador" printed
//   with the BU; ccalculacv.cpp:74 rejects keys shorter than 16 bytes).
//
// In the web build no class implements api::IKernelHSM (no RTTI record, no CPolySingletonList::push for it),
// and /dsk/fi/estatico/chave/ is not shipped, so this path cannot succeed in the simulator.
// ---------------------------------------------------------------------------------------------------------

} // namespace ecourna::api::security
