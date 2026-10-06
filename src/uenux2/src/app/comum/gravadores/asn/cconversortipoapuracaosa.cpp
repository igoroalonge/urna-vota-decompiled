// uenux2/src/app/comum/gravadores/asn/cconversortipoapuracaosa.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// comum::asn::CConversorTipoApuracaoSA : IConversorASN<ModuloTiposResultadosEcoUrna::TipoApuracaoSA,
// md::CTipoApuracaoSA> (vtable @1596864). Only used for BUs produced by the Sistema de Apuração (SA), i.e. when the
// votes of a section are counted outside the urna (contingency). VOTA never sets it (CGravadorBU +268 is empty).
//   md::CTipoApuracaoSA { int tipo (+0: 1 manual, 2 eletrônica, 3 mista com BU, 4 mista com MR); uebyte motivo (+4) }
#include "comum/gravadores/asn/cconversortipoapuracaosa.h"

#include <format>

#include "comum/gravadores/iresultado.h"

namespace comum::asn {

namespace {
// Motives: manual 1..3 or 99; eletrônica 1..3 or 99; mista com BU 1..5 or 99; mista com MR 1..6 or 99.
template <typename E>
E ConverteMotivo(uebyte motivo, int maximo, bool aceita99, int codigoErro)
{
    if ((motivo >= 1 && motivo <= maximo) || (aceita99 && motivo == 99))
        return E(motivo);
    throw CUeComumGravadoresError(codigoErro, std::format("Motivo inválido: {}", motivo));
}
}  // namespace

ModuloTiposResultadosEcoUrna::MotivoApuracaoManual CConversorTipoApuracaoSA::ConverteMotivoApuracaoManual(const uebyte m) const
{ return ConverteMotivo<ModuloTiposResultadosEcoUrna::MotivoApuracaoManual>(m, 3, true, 8623); }            // :98
ModuloTiposResultadosEcoUrna::MotivoApuracaoEletronica CConversorTipoApuracaoSA::ConverteMotivoApuracaoEletronica(const uebyte m) const
{ return ConverteMotivo<ModuloTiposResultadosEcoUrna::MotivoApuracaoEletronica>(m, 3, true, 8624); }        // :117
ModuloTiposResultadosEcoUrna::MotivoApuracaoMistaComBU CConversorTipoApuracaoSA::ConverteMotivoApuracaoBU(const uebyte m) const
{ return ConverteMotivo<ModuloTiposResultadosEcoUrna::MotivoApuracaoMistaComBU>(m, 5, true, 8625); }        // :140
ModuloTiposResultadosEcoUrna::MotivoApuracaoMistaComMR CConversorTipoApuracaoSA::ConverteMotivoApuracaoMR(const uebyte m) const
{ return ConverteMotivo<ModuloTiposResultadosEcoUrna::MotivoApuracaoMistaComMR>(m, 6, true, 8626); }        // :165

// wasm func 10283 (vtable slot 2; srcloc :54). The CHOICE alternative carries its own "tipoApuracao" INTEGER.
CConversorTipoApuracaoSA::TEntidade CConversorTipoApuracaoSA::DoConverte(const TDado& dado) const
{
    TEntidade entidade;
    switch (dado.GetTipo()) {
    case 1: entidade.set_apuracaoTotalmenteManual({1, ConverteMotivoApuracaoManual(dado.GetMotivo())}); break;
    case 2: entidade.set_apuracaoEletronica({2, ConverteMotivoApuracaoEletronica(dado.GetMotivo())}); break;
    case 3: entidade.set_apuracaoMistaBUAE({3, ConverteMotivoApuracaoBU(dado.GetMotivo())}); break;
    case 4: entidade.set_apuracaoMistaMR({4, ConverteMotivoApuracaoMR(dado.GetMotivo())}); break;
    default:
        throw CUeComumGravadoresError(8621, std::format("Tipo de apuração inválido: {}", dado.GetTipo()));   // :54
    }
    return entidade;
}

// wasm func 10282 (vtable slot 3; srcloc :78, plus the inlined CTipoApuracaoSA::ValidaCriacao :40 and
// ValidaMotivoApuracao :72 of md/ctipoapuracaosa.cpp)
md::CTipoApuracaoSA CConversorTipoApuracaoSA::DoDesconverte(const TEntidade& entidade) const
{
    static constexpr int tipos[4] = {4, 3, 1, 2};      // CHOICE index -> tipo (@544832): MR, BU, manual, eletrônica
    const int escolha = entidade.choiceIndex();
    if (escolha >= 4)
        throw CUeComumGravadoresError(8622, "Tipo de apuração inválido");                // :78
    return md::CTipoApuracaoSA(tipos[escolha], entidade.motivo());
}

}  // namespace comum::asn

namespace comum::md {
// Inlined into 10282 (md/ctipoapuracaosa.cpp:40 and :72; that file is not in this unit's list)
void CTipoApuracaoSA::ValidaCriacao() const
{
    if (m_tipo <= 0)                                                                    // compiled as (t - 5) <= -5
        throw CUeComumGravadoresError(8687, "Tipo de apuração inválido");                // :40
    ValidaMotivoApuracao();
}
void CTipoApuracaoSA::ValidaMotivoApuracao() const
{
    bool ok = false;
    switch (m_tipo) {
    case 1: ok = (m_motivo >= 1 && m_motivo <= 3) || m_motivo == 99; break;           // manual
    case 2: ok = (m_motivo >= 1 && m_motivo <= 3) || m_motivo == 99; break;           // eletrônica
    case 3: ok = (m_motivo >= 1 && m_motivo <= 5) || m_motivo == 99; break;           // mista com BU
    case 4: ok = (m_motivo >= 1 && m_motivo <= 6) || m_motivo == 99; break;           // mista com MR
    }
    if (!ok)
        throw CUeComumGravadoresError(8688, "Motivo de apuração inválido");              // :72
}
}  // namespace comum::md
