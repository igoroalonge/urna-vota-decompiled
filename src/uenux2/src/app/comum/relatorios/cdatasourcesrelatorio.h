// uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h
// Reconstructed from vota_web_wasm.wasm (unit u25). Only instantiation: <comum::CRdvVota, comum::CEleitores>.
//
// comum::CDataSourcesRelatorio<RDV, DSAptos> — the "data sources" of the vote-count reports (BU and
// zerésima): static std::string(*)() functions that a report form calls AT PRINT TIME through
// api::CDataText<std::string(*)()> (CPaperFormBuilder::AddData, wasm func 604). They read the CURRENT cargo of
// comum::CCargos, the current candidacy / party of the CDataMaps, and the in-memory RDV (RDV::GetInst()).
// While the BU is generated they also feed the "código verificador" chain (comum::CCalculaCV).
//
// srclocs (line = std::source_location inside the function):
//   :152 DetalheCandidatoZE()            wasm 11973, table slot 1635   Assert (qtd == 0u)
//   :188 TrailerProporcionalPartido()    wasm 11260, table slot 2922   CPolySingleton<CCalculaCV>::instance
//   :212 GetLinhasEleitoresAptos() lambda  wasm 11968 (other unit)     "Abrangência inválida: {}" (9089)
//   :241 TrailerProporcional()           wasm 11981, table slot 1625
//   :305 TrailerMajoritario()            wasm 11980, table slot 1626
//   :348 CodVerificador()                wasm 11974, table slot 1633
// Other members of the template, compiled in other units: 11975 (slot 1632, "cargo sem candidato"
// trailer: GetLinhasEleitoresAptos + "Comparecimento {:04}\n" ...), 11972 (slot 1636, BU candidate line
// "  {}  {}{:0{}}  {:04}"), 11261 (slot 2921, "{}: {} - {}\n" party header), 11259 (slot 2923).
//
// RDV interface used (comum::CRdv vtable slots, names from crdvvota.h): 3 Candidato(cargo, numero, digitos),
// 4 Legenda(cargo, partido), 5 Partido(cargo, partido), 6 Nominais, 7 Legendas, 8 Nulos, 9 Brancos,
// 10 Cargo (total apurado of the cargo).
//
// Rendered example (samples/bu-real/run-full/reports/bu.txt, 1 vote for Vereador 91001, 1 null Prefeito):
//         Votos de legenda              0000       <- TrailerProporcionalPartido
//         Total do partido              0001
//
//         Código Verificador: 9.977.581.304
//     --------------------------------------
//     Eleitores aptos                   0001       <- TrailerProporcional: GetLinhasEleitoresAptos()
//               Originais da seção      0001
//               Temporários na seção    0000
//     Total de votos Nominais           0001       <- CCargoDSLabelRelatorio::TotalVotoNominal() format
//     Total de votos de Legenda         0000
//     Brancos                           0000
//     Nulos                             0000
//     Total Apurado                     0001
//
//     Código Verificador: 5.947.877.038             <- CodVerificador (separate data field)
#pragma once

#include <format>
#include <functional>
#include <string>

#include "api/pattern/cpolysingleton.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/util/uassert.h"
#include "comum/dados/ccandidaturas.h"
#include "comum/dados/ccargods.h"             // CCargoDSLabelRelatorio::TotalVotoNominal (translated label)
#include "comum/dados/ccargos.h"
#include "comum/dados/cpartidos.h"
#include "comum/dados/md/ctradutorfrase.h"
#include "comum/relatorios/ccalculacv.h"
#include "comum/relatorios/crelutil.h"
#include "comum/relatorios/relatoriosdefs.h"

namespace comum {

template <typename RDV, typename DSAptos>
class CDataSourcesRelatorio {
public:
    static std::string DetalheCandidatoZE();          // wasm 11973
    static std::string TrailerProporcionalPartido();  // wasm 11260
    static std::string TrailerProporcional();         // wasm 11981
    static std::string TrailerMajoritario();          // wasm 11980
    static std::string CodVerificador();              // wasm 11974

    // Members compiled in other units (names from u04 / inferred here):
    static std::string DetalheCandidato();            // wasm 11972, slot 1636 (BU candidate line)
    static std::string TrailerComparecimento();       // wasm 11975, slot 1632 (cargo without candidates)
    static std::string HeaderProporcionalPartido();   // wasm 11261, slot 2921 ("{}: {} - {}\n" + columns)
    static std::string HeaderDetalheSeHouverVotos();  // wasm 11259, slot 2923

private:
    static std::string GetLinhasEleitoresAptos();     // wasm 3871 (tools: comum_f3871)
    static std::string FormataCV(const std::string& cv)            // inlined 3 times    name inferred
    {
        return cv.substr(0, 1) + "." + cv.substr(1, 3) + "." + cv.substr(4, 3) + "." + cv.substr(7, 3);
    }
};

// ------------------------------------------------------------------------------------------------------
// wasm func 3871 (tools: comum_f3871). "Eleitores aptos ..." block for the abrangência of the current
// cargo, followed by a newline. The std::function holds the lambda of :212 (wasm 11968, vtable @1543880):
//   [] { const auto aptos = DSAptos::GetInst().GetQtdAptos();              // CEleitores slot 0 (map copy)
//        const auto abrangencia = CCargos::GetInst().GetCurrent().GetAbrangencia();   // CCargo +8
//        if (!aptos.contains(abrangencia))
//            throw CRelatoriosError(9089, std::format("Abrangência inválida: {}", abrangencia));  // :212
//        return aptos.at(abrangencia); }
template <typename RDV, typename DSAptos>
std::string CDataSourcesRelatorio<RDV, DSAptos>::GetLinhasEleitoresAptos()
{
    const std::function<SQtdeAptos()> aptos = [] {
        const auto qtdAptos = DSAptos::GetInst().GetQtdAptos();
        const auto abrangencia = CCargos::GetInst().GetCurrent().GetAbrangencia();
        if (!qtdAptos.contains(abrangencia))
            throw CRelatoriosError(EUeComumRelatoriosError{9089},
                                   std::format("Abrangência inválida: {}", abrangencia));    // :212
        return qtdAptos.at(abrangencia);
    };
    return CRelUtil::FormataQtdAptos(aptos) + "\n";                              // func 1921 (copy: 1922)
}

// ------------------------------------------------------------------------------------------------------
// wasm func 11973 (srcloc :152), slot 1635. One line of the zerésima's candidate list:
//     "  Golfe                          91001"
// name left-aligned in 31 columns (cut at 36; names of 32..36 characters continue on a second line with
// an arrow of dashes), then the number right-aligned in 5 columns. The zerésima must show zero votes.
template <typename RDV, typename DSAptos>
std::string CDataSourcesRelatorio<RDV, DSAptos>::DetalheCandidatoZE()
{
    const md::CCargo& cargo = CCargos::GetInst().GetCurrent();
    const uebyte digitos = cargo.GetNumeroDigitos();                                 // +12
    const TCargoID codigo = cargo.GetCodigo();                                       // +0
    auto& candidaturas = CCandidaturas::GetInst();                                   // wasm_entry_f521
    const TCandidatoID numero = candidaturas.GetCurrent().GetNumero();               // +4
    std::string nome = candidaturas.GetCurrent().GetTitular().GetNomeUrna();         // +8 +12

    if (nome.size() > 36)
        nome.resize(36);
    if (nome.size() <= 30)
        nome.append(31 - nome.size(), ' ');                                          // func 3511 (pad right)
    if (nome.size() >= 32) {
        nome.append(36 - nome.size(), '-');
        nome += "\n  ";
        nome.append(30, '-');
        nome += ">";
    }
    UE_ASSERT(RDV::GetInst().Candidato(codigo, numero, digitos) == 0u);              // :152 "Assert (qtd == 0u)"
    return std::format("  {}{}{:0{}}", nome, std::string(5 - digitos, ' '), numero, digitos);
}

// ------------------------------------------------------------------------------------------------------
// wasm func 11260 (srcloc :188), slot 2922. Totals of the current party in a proportional cargo, plus the
// block's CV (this data source computes the CV itself, there is no separate CodVerificador field).
template <typename RDV, typename DSAptos>
std::string CDataSourcesRelatorio<RDV, DSAptos>::TrailerProporcionalPartido()
{
    CCargos& cargos = CCargos::GetInst();
    const TCargoID cargo = cargos.GetCurrent().GetCodigo();
    const TPartidoID partido = CPartidos::GetInst().GetCurrent().GetNumero();        // ecourna_f819, 1283
    const RDV& rdv = RDV::GetInst();
    const auto legenda = rdv.Legenda(cargo, partido);                                // slot 4
    const auto total = rdv.Partido(cargo, partido);                                  // slot 5

    std::string texto;
    texto += std::format("{:<33s} {:04}\n", std::string_view("    Votos de legenda"), legenda);
    const std::string rotulo = md::CTradutorFrase::TraduzLabel("    Total <P|de|do|da> <PLSB>").substr(0, 33);
    texto += std::format("{:<33s} {:04}", rotulo, total);

    if (api::CPolySingletonList::exists<CCalculaCV>()) {                             // func 1954
        auto& calculadora = api::CPolySingleton<CCalculaCV>::instance();             // func 1282, :188
        calculadora.IncluiString(std::format("{:04}{:04}", legenda, total));
        if (cargos.GetIndice() + 1 == cargos.GetQuantidade())                        // last cargo printed
            IncluiFinal(cargos, calculadora);                                        // func 5967
        texto += "\n\n    Código Verificador: " + FormataCV(calculadora.Calcula());  // func 3700
    }
    return texto;
}

// ------------------------------------------------------------------------------------------------------
// wasm func 11981 (srcloc :241), slot 1625. Totals of a proportional cargo.
template <typename RDV, typename DSAptos>
std::string CDataSourcesRelatorio<RDV, DSAptos>::TrailerProporcional()
{
    const TCargoID cargo = CCargos::GetInst().GetCurrent().GetCodigo();
    const RDV& rdv = RDV::GetInst();
    const auto nominais = rdv.Nominais(cargo);       // slot 6
    const auto legendas = rdv.Legendas(cargo);       // slot 7
    const auto brancos = rdv.Brancos(cargo);         // slot 9
    const auto nulos = rdv.Nulos(cargo);             // slot 8
    const auto total = rdv.Cargo(cargo);             // slot 10

    if (api::CPolySingletonList::exists<CCalculaCV>())                               // func 1954
        api::CPolySingleton<CCalculaCV>::instance().IncluiString(                    // :241
            std::format("{:04}{:04}{:04}{:04}{:04}", nominais, legendas, brancos, nulos, total));

    std::string texto;
    texto += GetLinhasEleitoresAptos();                                              // func 3871
    texto += std::vformat(CCargoDSLabelRelatorio::TotalVotoNominal(), std::make_format_args(nominais));
    texto += std::format("Total de votos de Legenda         {:04}\n", legendas);
    texto += std::format("Brancos                           {:04}\n", brancos);
    texto += std::format("Nulos                             {:04}\n", nulos);
    texto += std::format("Total Apurado                     {:04}", total);
    return texto;
}

// ------------------------------------------------------------------------------------------------------
// wasm func 11980 (srcloc :305), slot 1626. Totals of a majoritarian cargo or referendum.
template <typename RDV, typename DSAptos>
std::string CDataSourcesRelatorio<RDV, DSAptos>::TrailerMajoritario()
{
    const TCargoID cargo = CCargos::GetInst().GetCurrent().GetCodigo();
    const RDV& rdv = RDV::GetInst();
    const auto nominais = rdv.Nominais(cargo);       // slot 6
    const auto brancos = rdv.Brancos(cargo);         // slot 9
    const auto nulos = rdv.Nulos(cargo);             // slot 8
    const auto total = rdv.Cargo(cargo);             // slot 10

    if (api::CPolySingletonList::exists<CCalculaCV>())
        api::CPolySingleton<CCalculaCV>::instance().IncluiString(                    // :305
            std::format("{:04}{:04}{:04}{:04}", nominais, brancos, nulos, total));

    std::string texto;
    texto += GetLinhasEleitoresAptos();
    texto += std::vformat(CCargoDSLabelRelatorio::TotalVotoNominal(), std::make_format_args(nominais));
    texto += std::format("Brancos                           {:04}\n", brancos);
    texto += std::format("Nulos                             {:04}\n", nulos);
    texto += std::format("Total Apurado                     {:04}", total);
    return texto;
}

// ------------------------------------------------------------------------------------------------------
// wasm func 11974 (srcloc :348), slot 1633. The CV of the header / cargo-total blocks.
// NOTE: unlike the three trailers above it does NOT test exists<CCalculaCV>(): printing a form that holds
// this field without a published calculator throws "PolySingleton - solicitada uma instancia nao criada".
template <typename RDV, typename DSAptos>
std::string CDataSourcesRelatorio<RDV, DSAptos>::CodVerificador()
{
    const CCargos& cargos = CCargos::GetInst();
    auto& calculadora = api::CPolySingleton<CCalculaCV>::instance();                 // :348
    if (cargos.GetIndice() + 1 == cargos.GetQuantidade())
        IncluiFinal(cargos, calculadora);                                            // func 5967
    return "Código Verificador: " + FormataCV(calculadora.Calcula());
}

} // namespace comum
