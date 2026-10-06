// uenux2/src/api/io/cencryptedfile.cpp  -- FRAGMENT reconstructed by unit u12
// (the file belongs to the uenux2 api/io unit. Func 2767 landed in u12 because the analyzer named it
//  "CFile::Flush", after the srcloc of the inlined CFile::Flush, cfile.cpp:145)
//
// Reconstructed from vota_web_wasm.wasm.
//
// api::CEncryptedFile layout used here: +0 std::shared_ptr<ecourna::api::security::ISymmetricCipher> m_cifrador
// (+4 control block), +8 std::vector<uebyte> m_dados. Callers:
//   * funcs 5736 / 5737, called by func 6737 (CGeraDadosDinamicos) during votaInit: they create the RDV
//     (rdv.dat). These are the calls seen in the recorded sessions (analysis/runtime/*.edges.tsv).
//   * the per-vote RDV writer (impl::CSincronismoVotoEleitor::vf2, func 7174): the RDV is saved encrypted to
//     "<rdv>.tmp", then read back and checked (see docs/modules/u07-...md). The web build does not persist votes
//     to the RDV (docs/modules/u01-...md §3.1, analysis/runtime/README.md).
// For the RDV the cipher's key AND IV come from the HKDF seed (GetCifradorCryptoTable, u01 §2.6/§3.1): the IV is
// fixed and is not appended to the file.
#include <string>
#include <vector>

#include "api/io/cencryptedfile.h"
#include "api/io/euioerror.h"
#include "ecourna/api/io/cfile.hpp"

namespace api {

// wasm func 2767 (srcloc cencryptedfile.cpp:88; ecourna::api::io::CFile::Flush inlined, cfile.cpp:145)
void CEncryptedFile::Save(const std::string& arquivo) const
{
    if (arquivo.empty())
        throw CUeIoError(EUeIoError(5994), "Nome de arquivo vazio");                // line 88

    // PKCS#7-style padding to a multiple of 16 bytes: 1..16 bytes, each equal to the pad length.
    const uebyte pad = static_cast<uebyte>(16 - (m_dados.size() & 15));
    std::vector<uebyte> claro;
    claro.reserve(m_dados.size() + pad);
    claro.assign(m_dados.begin(), m_dados.end());                                  // shared_f1927
    for (uebyte i = 0; i < pad; ++i)
        claro.push_back(pad);

    std::vector<uebyte> cifrado;
    m_cifrador->Encrypt(claro, cifrado);        // vtable slot 2: CBlockCipher<CAesCipher,CTrng>::Encrypt (unit u01).
                                                // OpenSSL's default PKCS#7 padding adds another 16-byte block
                                                // (confirmed by decrypting rdv.dat with -nopad, u01 §3.1).

    ecourna::api::io::CFile file(arquivo, "wb");                                   // func 517
    file.RawWrite(cifrado.data(), static_cast<uedword>(cifrado.size()));           // func 1886
    file.Flush();                                                                  // inlined (fflush)
    file.Sync();                                                                   // func 5181 (fflush + fsync)
    file.Close();                                                                  // func 336 (Sync again: "wb")
}

} // namespace api
