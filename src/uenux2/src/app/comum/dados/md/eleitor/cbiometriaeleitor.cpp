// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.cpp
//
// CBiometriaEleitor = decrypted biometric package of one voter (ModuloEleitores::BiometriaEleitor,
// shipped encrypted as BiometriaEleitorCifrada inside the roll): optional photo + up to ten
// fingerprint templates, plus the decryption outcome.
// Layout (from GetDedo, CriaComum and the inlined GetFoto in func 3616):
//   +0  std::optional<CFoto> foto: CFoto = {int formato (+0); std::vector<uebyte> imagem (+4)},
//       engaged flag +16 (3616 copies the bytes from +4..+8 and tests the byte at +16)
//   +20 std::map<CDedo::TipoDedo, CDedo> dedos (+32 bool: dedos present)
//   +36 int estadoDecifracao (0 = ok; 9 = erro decifrando buffer; other > 0 = erro de criptografia)
// CDedo (20 bytes) = { TipoDedo tipo; int ?; std::vector<uebyte> template; }
#include "cbiometriaeleitor.h"

#include <format>

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 2799 (srclocs lines 25, 31). Called by comum::asn::CConversorBiometriaEleitor::DoDesconverte.
void CBiometriaEleitor::CriaComum(std::vector<CDedo> dedos)
{
    if (dedos.size() > 10)
        throw CUeComumDadosError(8056, std::format("Mais de dez dedos na biometria do eleitor: {}", dedos.size()));
    // index loop: the wasm re-reads dedos.size() on every iteration
    for (std::size_t i = 0; i < dedos.size(); ++i) {
        // The pair (tipo, copy of the CDedo) is built BEFORE the tree lookup and moved into the new
        // node (+16 key, +20 CDedo); on a duplicate the copied template vector is freed and 8057 thrown.
        // That is insert(pair&&): map::emplace(k, v) would search first and copy only when inserting.
        if (!m_dedos.insert({dedos[i].GetTipo(), dedos[i]}).second)
            throw CUeComumDadosError(8057, "Dedo já existente ou duplicado");
    }
}

// wasm func 3720 (srcloc line 93)
CDedo CBiometriaEleitor::GetDedo(CDedo::TipoDedo tipo) const
{
    const auto it = m_dedos.find(tipo);
    if (it == m_dedos.end())
        throw CUeComumDadosError(8058, "Dedo não encontrado.");
    return it->second;
}

// (no function of its own: inlined into the photo presentation code, func 3616)  (srcloc line 115)
const ecourna::app::dados::CFoto& CBiometriaEleitor::GetFoto() const
{
    if (!m_foto.has_value())
        throw CUeComumDadosError(8059, "Não há informação de foto.");
    return *m_foto;
}

}  // namespace comum::md
