// FRAGMENTS of three attested comum/md files whose code exists only inlined into
// comum::asn::CConversorCabecalhoPacote::DoDesconverte (wasm func 11437). Reconstructed from vota_web_wasm.wasm
// (unit u35):
//   uenux2/src/app/comum/md/ccabecalhopacote.cpp  :33 :36 :39  CCabecalhoPacote::CCabecalhoPacote(...)
//   uenux2/src/app/comum/md/cidpacote.cpp         :21          CIDPacoteValidar(EUrnaFase)
//   uenux2/src/app/comum/md/cideleitoral.cpp      :21 :29 :37  CIDEleitoral::CriaProcessoEleitoral / CriaPleito / CriaEleicao
// Error family: CUeComumMdError = CBaseError<EUeComumMdError, {8900, 8950}> (constructor thunk comum_f591).
#include <string>

#include "comum/md/cabrangencia.h"      // CUeComumMdError
#include "comum/md/ccabecalhopacote.h"
#include "comum/md/cideleitoral.h"
#include "comum/md/cidpacote.h"

namespace comum::md {

// ---- cideleitoral.cpp -----------------------------------------------------------------------------------------
// md::CIDEleitoral (8 bytes): +0 uedword id, +4 tipo (1 processo eleitoral, 2 pleito, 3 eleição);
// the plain constructor is shared_f1081(this, id, tipo).
CIDEleitoral CIDEleitoral::CriaProcessoEleitoral(uedword id)
{
    if (id >= 100000)
        throw CUeComumMdError(EUeComumMdError{8911}, "ID inválido.");                       // line 21
    return CIDEleitoral(id, ETipoIDEleitoral(1));
}

CIDEleitoral CIDEleitoral::CriaPleito(uedword id)
{
    if (id >= 100000)
        throw CUeComumMdError(EUeComumMdError{8912}, "ID inválido.");                       // line 29
    return CIDEleitoral(id, ETipoIDEleitoral(2));
}

CIDEleitoral CIDEleitoral::CriaEleicao(uedword id)
{
    if (id >= 100000)
        throw CUeComumMdError(EUeComumMdError{8913}, "ID inválido.");                       // line 37
    return CIDEleitoral(id, ETipoIDEleitoral(3));
}

// ---- cidpacote.cpp --------------------------------------------------------------------------------------------
// Called first by every CIDPacote constructor (4 overloads: with / without UF, município, zona).
// md::CIDPacote (40 bytes): +0 CIDEleitoral, +8 EUrnaFase, +12 optional<std::string> UF (flag +24),
// +28 optional<TMunicipioID> (flag +32), +36 optional<TZonaID> (flag +38).
void CIDPacoteValidar(EUrnaFase fase)                  // name as printed by the srcloc record
{
    // The enum's two sentinels around '1'..'3' (values '0' and '4') are rejected; nothing else is (the compiled test is
    // (fase & ~4) == '0').
    if (fase == EUrnaFase('0') || fase == EUrnaFase('4'))
        throw CUeComumMdError(EUeComumMdError{8921}, "Fase inválida.");                      // line 21
}

// ---- ccabecalhopacote.cpp -------------------------------------------------------------------------------------
// md::CCabecalhoPacote (72 bytes): +0 ETipoPacote, +4 CIDPacote, +44 nome, +56 versão (12 digits), +68 ESistemaJE.
CCabecalhoPacote::CCabecalhoPacote(const ETipoPacote tipo, const CIDPacote& id, const std::string& nome,
                                   const std::string& versao, const ESistemaJE origem)
    : m_tipo(tipo), m_id(id), m_nome(nome), m_versao(versao), m_origem(origem)
{
    if (m_nome.empty())
        throw CUeComumMdError(EUeComumMdError{8908}, "Nome do pacote vazio.");              // line 33
    if (m_versao.size() != 12)
        throw CUeComumMdError(EUeComumMdError{8909}, "Versão deve ter tamanho 12.");        // line 36
    // Range 1..12 (the ESistemaJE values). Compiled as one unsigned test, (unsigned)(origem - 13) <= 0xFFFFFFF3
    // (i32.le_u), which is true exactly when origem is outside [1, 12]. Unreachable from the converter:
    // DesconverteIdSistema only returns 1..12.
    if (static_cast<int>(m_origem) < 1 || static_cast<int>(m_origem) > 12)
        throw CUeComumMdError(EUeComumMdError{8910}, "Valor de origem inválido.");          // line 39
}

} // namespace comum::md
