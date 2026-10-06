// uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by std::source_location records :81
// (GetDetalheCandidato), :89 (GetDetalheConsulta), :112 (GetOrdinalEscolha), :212..:240 (Valida).
// The inlined accessors GetDetalheCandidato / GetDetalheConsulta left their srcloc in every name
// getter, which is why the tools called funcs 1547, 2796, 2797, 3715, 3716 "GetDetalheConsulta" /
// "GetDetalheCandidato". Two more accessors (PossuiFoto 1546, GetQtdSuplentes 1157) are in
// ccargo.u07.cpp.
//
// Errors: CDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
// SErrorLimits{7800, 8600}> (typeinfo @1528076, constructor thunk func 170).
#include "comum/dados/md/processoeleitoral/ccargo.h"

#include <format>

#include "comum/dados/dadosdefs.h"   // CDadosError

namespace comum::md {

// wasm func 1388 (srcloc :81)
const CDetalheCandidato& CCargo::GetDetalheCandidato() const
{
    if (!m_detalheCandidato.has_value())
        throw CDadosError(EUeComumDadosError{8134}, "Não é cargo de candidato.");              // :81
    return *m_detalheCandidato;
}

// wasm func 1923 (srcloc :89) - through func 6029, a body merged by wasm-opt with
// CLocal::GetContingencia: `optional engaged ? &value : throw CDadosError(code, msg, srcloc)`.
const CDetalheConsulta& CCargo::GetDetalheConsulta() const
{
    if (!m_detalheConsulta.has_value())
        throw CDadosError(EUeComumDadosError{8135}, "Não é cargo de consulta.");              // :89
    return *m_detalheConsulta;
}

// wasm func 1547 - observed executing (screen title, audio texts, DS_NomeCargoNeutroComEscolha)
std::string CCargo::GetNome() const
{
    if (m_detalheCandidato.has_value())
        return m_detalheCandidato->GetNomes().GetNomeNeutro();                            // +24
    return GetDetalheConsulta().GetNome();                                                // +88 (:89)
}

// wasm func 3716
std::string CCargo::GetNomeMasculino() const
{
    if (m_detalheCandidato.has_value())
        return m_detalheCandidato->GetNomes().GetNomeMasculino();                         // +36
    return GetDetalheConsulta().GetNome();
}

// wasm func 2797
std::string CCargo::GetNomeAbreviado() const
{
    if (m_detalheCandidato.has_value())
        return m_detalheCandidato->GetNomes().GetNomeAbreviado();                         // +60
    return GetDetalheConsulta().GetNome();
}

// wasm func 2796 - the candidate's gender picks the form of the office name
std::string CCargo::GetNome(CSexo::ESexo sexo) const
{
    switch (sexo) {
    case CSexo::ESexo::MASCULINO:                                                         // 1
        return GetNomeMasculino();
    case CSexo::ESexo::FEMININO:                                                          // 2 (inlined)
        if (m_detalheCandidato.has_value())
            return m_detalheCandidato->GetNomes().GetNomeFeminino();                      // +48
        return GetDetalheConsulta().GetNome();
    default:
        return GetNome();
    }
}

// wasm func 3715 - name of the n-th running mate's office (vice, 1º/2º suplente), by gender
std::string CCargo::GetNomeSuplente(uebyte suplente, CSexo::ESexo sexo) const
{
    const CNomesCargo nomes = GetDetalheCandidato().GetSuplente(suplente).GetNomes();    // :81, func 1545 (copy)
    switch (sexo) {
    case CSexo::ESexo::MASCULINO: return nomes.GetNomeMasculino();
    case CSexo::ESexo::FEMININO:  return nomes.GetNomeFeminino();
    default:                      return nomes.GetNomeNeutro();
    }
}

// wasm func 2258 (srcloc :112) - observed executing. "1ª vaga", "2ª vaga" for multi-seat offices,
// empty for the others.
std::string CCargo::GetOrdinalEscolha(TQtdEscolha escolha) const
{
    if (m_qtdEscolhas == 1)
        return {};
    if (escolha > m_qtdEscolhas)
        throw CDadosError(EUeComumDadosError{8137},
                          std::format("Cargo {} com escolha invalida {}", GetNomeMasculino(), escolha));   // :112
    return std::format("{}ª vaga", escolha);
}

// wasm func 5657 (srcloc :212, :215, :218, :221, :226, :234, :240) - called by the CCargo
// constructor (inlined into CConversorCargo::DoDesconverte, func 11373).
void CCargo::Valida() const
{
    if (m_abrangencia >= 3)
        throw CDadosError(EUeComumDadosError{8138}, "Abrangência inválida.");                        // :212
    if (m_numeroDigitos < 1 || m_numeroDigitos > 5)
        throw CDadosError(EUeComumDadosError{8139}, "Número de dígitos inválido.");                  // :215
    if (m_qtdEscolhas == 0)
        throw CDadosError(EUeComumDadosError{8140}, "Quantidade de escolhas inválida.");             // :218
    if (m_paginaImpressaoVoto < 1 || m_paginaImpressaoVoto > 5)
        throw CDadosError(EUeComumDadosError{8141}, "Página de impressão do voto inválida.");        // :221
    switch (m_tipo) {
    case ETipo::CONSULTA:
        if (!m_detalheConsulta.has_value())
            throw CDadosError(EUeComumDadosError{8142},
                              "Tipo de cargo de consulta sem informações de consulta.");             // :226
        break;
    case ETipo::MAJORITARIO:
    case ETipo::PROPORCIONAL:
        if (!m_detalheCandidato.has_value())
            throw CDadosError(EUeComumDadosError{8143},
                              "Tipo de cargo de candidato sem informações de candidato.");           // :234
        break;
    default:
        throw CDadosError(EUeComumDadosError{8144}, "Tipo de cargo inválido.");                      // :240
    }
}
// Note: the compiled test is `tipo >= 2 ? (tipo == 2 ? consulta : invalid) : candidato`, so a
// negative tipo would be accepted as a candidate cargo (cannot come from the ASN.1 enumeration).

} // namespace comum::md
