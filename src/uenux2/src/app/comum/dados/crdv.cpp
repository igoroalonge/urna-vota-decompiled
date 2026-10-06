// uenux2/src/app/comum/dados/crdv.cpp
// (original build path: /home/rubio/tse/uenux2/src/app/comum/dados/crdv.cpp)
//
// Reconstructed from vota_web_wasm.wasm by unit u02 (no other unit owns this file: none of its
// functions survived as a separate wasm function; everything below is inlined into wasm func 7787,
// through CRdvVota::CreateInst() -> CRdvVota::CRdvVota() -> CRdv::CRdv()).
//
// Evidence (std::source_location records):
//   crdv.cpp:60  TSharedSymmetricCipher comum::(anonymous namespace)::GetCifradorCryptoTable(const std::vector<uebyte>&)
//   crdv.cpp:78  (same function)
//   crdv.cpp:90  comum::CRdv::CRdv(md::RdvPosicionadorPtr, const std::vector<uebyte>&, uebyte)
//   crdv.cpp:93  (same constructor)
// dcmp lines 15714-16605 of the earlier chkdfseed.cpp.dcmp layout (see cinformacaoeleitor.cpp); func 7787 now
// lives in decompiled/app-vota/uenux2/src/app/vota/eleitor/cestadosvota.cpp.dcmp (search "srcloc crdv.cpp").
//
// Only the parts that execute in this build are reconstructed (the rest of CRdv, e.g. the methods
// that write rdv.dat, lives in other wasm functions owned by other units).

#include "comum/dados/crdv.h"

#include <array>
#include <format>

#include "api/pattern/cpolysingleton.h"
#include "api/hwil/iurna.h"
#include "ecourna/api/security/chkdfseed.hpp"
#include "ecourna/api/security/csha.hpp"
#include "ecourna/api/security/isymmetriccipherfactory.hpp"

namespace comum {

namespace {

using ecourna::api::security::CHKDFSeed;
using ecourna::api::security::CSha512;
using ecourna::api::security::ISymmetricCipherFactory;
using ecourna::api::security::TSharedSymmetricCipher;

/// Builds the symmetric cipher that protects the RDV (Registro Digital do Voto) file.
///
/// `cargos` is the sorted list of office codes (one byte per CCargo of the election configuration,
/// built by CRdvVota::CRdvVota and sorted with std::sort<uebyte*>, wasm funcs 4811..4820).
///
/// inlined into wasm func 7787 (dcmp 15714-16476)                        (srcloc lines 60, 78)
TSharedSymmetricCipher GetCifradorCryptoTable(const std::vector<uebyte>& cargos)
{
    // 128-byte secret table provided by the urna hardware layer.
    // IUrna vtable slot 6. In the simulator IUrna is api::teste::CUrnaMock, whose slot 6
    // (wasm func 8131) is `memset(tabela, 3, 128)`: the table is 128 bytes of 0x03.
    std::array<uebyte, 128> tabela;
    api::CPolySingleton<api::IUrna>::instance().GetCryptoTable(tabela);        // line 60, name inferred

    // salt = SHA-512 of the office list (64 bytes)
    std::vector<uebyte> hash;
    {
        CSha512 sha;                                                           // wasm func 2684
        sha.Update(cargos);                                                    // wasm func 3519
        hash = sha.Finish();                                                   // wasm func 3518
    }

    // IKM: 32 bytes picked from the secret table.
    // A 10-bit index is built from hash[32+i] (8 bits) and the two top bits of hash[i] shifted to
    // bits 8-9, but the table lookup masks it with 0x7F: bits 7-9 are discarded, so the XOR below
    // has no effect on the result (see "suspicious" in the u02 doc).
    std::vector<uint16_t> indices;
    for (std::size_t i = 32; i < 64; ++i)
        indices.push_back(hash.at(i));
    for (std::size_t i = 0; i < 32; ++i)
        indices.at(i) ^= static_cast<uint16_t>((hash.at(i) & 0xC0) << 2);

    std::vector<uebyte> chave;
    for (const auto indice : indices)
        chave.push_back(tabela[indice & 0x7F]);

    tabela.fill(0);                                   // memset(tabela, 0, 128): wipes the secret table

    const std::vector<uebyte> info{'R', 'D', 'V'};    // 3-byte vector {0x52, 0x44, 0x56}

    CHKDFSeed hkdf(hash, chave, info);                // HKDF-SHA512(salt=hash, IKM=chave, info="RDV")
    const std::vector<uebyte> semente = hkdf.GetSeed();   // 128 bytes

    // AES-256 key = semente[0..32), IV = semente[32..48); the other 80 bytes are not used.
    ecourna::api::security::CChave chaveRdv{                                   // (?) type name
        std::vector<uebyte>(semente.begin(), semente.begin() + 32),
        256,
        std::vector<uebyte>(semente.begin() + 32, semente.begin() + 48)};

    // ISymmetricCipherFactory vtable slot 3 = CSymmetricCipherFactory::vf3 (wasm func 9489):
    // builds a CBlockCipher<CAesCipher, CTrng> with mode 1 (CBC) from the key record.
    return api::CPolySingleton<ISymmetricCipherFactory>::instance().Create(chaveRdv);   // line 78
}

} // namespace

// Layout of CRdv (vtable comum::CRdv @1559900):
//   +0  vptr
//   +4  md::RdvPosicionadorPtr m_posicionador     (a single pointer: unique_ptr)
//   +8  TSharedSymmetricCipher m_cifrador         (shared_ptr: +8 object, +12 control block)
//   +16 uebyte m_qtdDigitosPartido
// CRdvVota (vtable @1560172, 100 bytes) continues at +20.

// inlined into wasm func 7787 (dcmp 15714-16605)                          (srcloc lines 90, 93)
CRdv::CRdv(md::RdvPosicionadorPtr posicionador,
           const std::vector<uebyte>& cargos,
           uebyte qtdDigitosPartido)
    : m_posicionador(std::move(posicionador))
    , m_cifrador(GetCifradorCryptoTable(cargos))
    , m_qtdDigitosPartido(qtdDigitosPartido)
{
    if (m_qtdDigitosPartido == 0)
        throw CRdvError(api::EUeRdvError{4650}, "Número de dígitos do partido nulo");             // line 90

    if (m_qtdDigitosPartido >= 6)
        throw CRdvError(api::EUeRdvError{4651},
                        std::format("Número de dígitos do partido ({}) supera o limite (5)",
                                    m_qtdDigitosPartido));                                       // line 93
}

} // namespace comum
