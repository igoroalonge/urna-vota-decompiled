// uenux2/src/app/comum/dados/ccargods.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// Text data sources about the current cargo (office) used by the voting screens and the printed reports.
// "Current" = position of the CCargos / CCandidaturas cursors (api::CDataMap), set by the screen being built.
// CCargos::Fim() below is the inlined "cursor at end" test (func 602: index >= vector size).   // name inferred
#include "comum/dados/ccargods.h"

#include <format>
#include <source_location>

#include "comum/dados/ccandidaturas.h"
#include "comum/dados/ccandidaturasds.h"
#include "comum/dados/ccargos.h"

namespace comum {

namespace {

constexpr std::size_t TAM_NOME_CARGO_ABREVIAR = 19;   // names this long are replaced by the abbreviation   // name inferred

// wasm func 5803 (srcloc line 25)
std::string GetNomeCargoComGenero(const std::string& contexto, const md::CSexo::ESexo sexo, const uebyte suplente)
{
    auto& cargos = CCargos::GetInst();
    if (cargos.Fim()) {
        throw CDadosError(7806, std::format("O cargo não foi posicionado corretamente para {}", contexto));   // line 25
    }
    const md::CCargo& cargo = cargos.GetCurrent();
    if (suplente == 0) {
        return cargo.GetNome(sexo);                        // func 2796: neutro/masculino/feminino (or consulta name)
    }
    return cargo.GetNomeSuplente(suplente, sexo);          // func 3715: DetalheCandidato.GetSuplente(n).nomes[sexo]
}

// Inlined into wasm func 2268 (srcloc line 41)
std::string GetNomeAbreviadoCargo(const std::string& contexto, const uebyte suplente)
{
    auto& cargos = CCargos::GetInst();
    if (cargos.Fim()) {
        throw CDadosError(7807, std::format("O cargo não foi posicionado corretamente para {}", contexto));   // line 41
    }
    const md::CCargo& cargo = cargos.GetCurrent();
    if (suplente == 0) {
        return cargo.GetNomeAbreviado();                   // func 2797: nomes.nomeAbreviado, or the consulta's name
    }
    return cargo.GetDetalheCandidato().GetSuplente(suplente).GetNomes().GetNomeAbreviado();   // CSuplencia +40
}

// Body shared by HeaderDetalhe/HeaderDetalheZE: wasm-opt merged them into func 6038, which receives the srcloc,
// the error code and pointers into the two 38-character literals.
std::string LabelPorTipoCargo(const char* labelCandidato, const char* labelConsulta, const int codigoErro,
                              const std::source_location local = std::source_location::current())   // name inferred
{
    auto& cargos = CCargos::GetInst();
    if (cargos.Fim()) {
        throw CDadosError(codigoErro, "O cargo não foi posicionado corretamente", local);
    }
    return cargos.GetCurrent().EhConsulta() ? labelConsulta : labelCandidato;   // CCargo +136 (optional<CDetalheConsulta>)
}

} // namespace

// Only visible inlined (with GetNomeCargoComGenero) into api::CDataText<CPadDS<CToUpperDS<CCargoDSNome>>>::GetText
// (func 11247), which keeps the pretty-function literal "std::string comum::CCargoDSNome::operator()() const".
std::string CCargoDSNome::operator()() const
{
    return GetNomeCargoComGenero(std::source_location::current().function_name(), m_sexo, 0);
}

// wasm func 2268 (srcloc lines 65, 72, 78)
std::string CCargoDSNomeSexoCandidato::operator()() const
{
    auto& candidaturas = CCandidaturas::GetInst();                     // func 521
    if (candidaturas.Fim()) {
        throw CDadosError(7808, "O candidato não foi posicionado corretamente");               // line 65
    }
    auto& cargos = CCargos::GetInst();
    if (cargos.Fim()) {
        throw CDadosError(7809, "O cargo não foi posicionado corretamente");                   // line 72
    }
    if (candidaturas.GetCurrent()->GetCodigoCargo() != cargos.GetCurrent().GetCodigo()) {
        throw CDadosError(7810, "Cargo e candidato não foram posicionados corretamente");      // line 78
    }

    // The sex is computed BEFORE the context string is built (wasm order). The context is the pretty function
    // name (64 chars, 72-byte heap block) passed as a temporary: it is freed right after the call and built a
    // second time for the abbreviation.
    const auto sexo = CCandidaturasDSSexo{m_suplente}();   // sex of the titular or of the suplente
    std::string nome = GetNomeCargoComGenero(std::source_location::current().function_name(), sexo, m_suplente);
    if (m_abreviaNomeLongo && nome.size() >= TAM_NOME_CARGO_ABREVIAR) {
        return GetNomeAbreviadoCargo(std::source_location::current().function_name(), m_suplente);
    }
    return nome;
}

// wasm func 5802 (srcloc line 98)
std::string CCargoDSLabelRelatorio::HeaderDetalhe()
{
    return LabelPorTipoCargo("Nome do candidato       Num cand Votos",
                             "Resposta                Num resp Votos", 7811);
}

// wasm func 11555 (srcloc line 113)
std::string CCargoDSLabelRelatorio::HeaderDetalheZE()
{
    return LabelPorTipoCargo("Nome do candidato             Num cand",
                             "Resposta                      Num resp", 7812);
}

// wasm func 5801 (srcloc line 128). The result is itself a std::format pattern ({:04} = vote count).
// The literals are Latin-1 in the binary ('á' = 0xE1): report text goes to the thermal printer as Latin-1.
std::string CCargoDSLabelRelatorio::TotalVotoNominal()
{
    auto& cargos = CCargos::GetInst();
    if (cargos.Fim()) {
        throw CDadosError(7813, "O cargo não foi posicionado corretamente");   // line 128
    }
    return cargos.GetCurrent().EhConsulta() ? "Total de votos Válidos            {:04}\n"
                                            : "Total de votos Nominais           {:04}\n";
}

} // namespace comum

// ----------------------------------------------------------------------------------------------------------
// Functions the tool attributes to this unit that are instantiations/lambdas built on the data sources above.
// Their real source locations are elsewhere (see the unit doc); they are reconstructed here for completeness.
//
// wasm func 12632 — api::CDataText<CCargoDSNomeSexoCandidato>::GetText() (IText vtable slot 2)   // name inferred
//     std::string GetText() const override { return m_ds(); }            // m_ds at +8
//
// wasm func 11247 — api::CDataText<CPadDS<CToUpperDS<CCargoDSNome>>>::GetText()                   // name inferred
//     (CDataText +8: CPadDS {size_t largura; CToUpperDS {... ESexo +16; void (*toUpper)(std::string&) +20}; char
//      preenchimento +48}; layout partly unknown)
//     std::string texto = GetNomeCargoComGenero("std::string comum::CCargoDSNome::operator()() const", sexo, 0);
//     toUpper(texto);                                  // function pointer call
//     if (texto.size() >= largura) return texto;
//     const auto falta = largura - texto.size();
//     return std::string(falta / 2, preenchimento) + texto + std::string(falta - falta / 2, preenchimento);
//
// Text sources handed as plain function pointers (table slots 1090-1095) by the confirmation-screen builders
// vota::(anonymous)::adicionaBaseTelaCompletaCandidatoCom1 (func 6671) / Com2 (func 6660) in
// vota/eleitor/comum/ctelasvota.cpp: captureless lambdas (path inferred: ctelasvota.cpp). The "cargo: nome"
// sources (slots 1090, 1092, 1093) go to the text-line builder vota_f3068 (CTextFieldDoubleLine); the
// "cargo only" sources (slots 1091, 1094, 1095) go to adicionaFotoEmoldurada (func 4181) as the caption under the
// suplente's photo. wasm-opt merged the pairs that differ only by a constant into shared bodies 6075 and 6076,
// leaving 4 thunks:
//   wasm func 6664  (slots 1091, 1095): []{ return CCargoDSNomeSexoCandidato{1, true}(); }          // "Vice-..."
//   wasm func 13298 (slot 1094):        []{ return CCargoDSNomeSexoCandidato{2, true}(); }
//   wasm func 6669  (slots 1090, 1092): []{ return CCargoDSNomeSexoCandidato{1, true}() + ": " + CCandidaturasDSNome{1}(); }
//   wasm func 13301 (slot 1093):        []{ return CCargoDSNomeSexoCandidato{2, true}() + ": " + CCandidaturasDSNome{2}(); }
//   wasm func 6075 = merged body of 6664/13298 (argument = the packed 2-byte CCargoDSNomeSexoCandidato: 257 / 258)
//   wasm func 6076 = merged body of 6669/13301 (arguments 257,1 / 258,2; CCandidaturasDSNome = func 5807)
