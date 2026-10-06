// uenux2/src/app/comum/relatorios/cgeradorbubase.h
// Reconstructed from vota_web_wasm.wasm (unit u25). Only instantiation: <comum::CRdvVota, comum::CEleitores>.
//
// comum::CGeradorBUBase<RDV, DSAptos> : CGeradorRelBase<RDV, DSAptos> — the per-cargo blocks of the
// Boletim de Urna. Each block prints ready-made forms (built by CGeradorBU's constructor) whose data
// fields read the CURRENT cargo / party / candidacy at print time, and appends the numbers of the block
// to the código-verificador chain (CCalculaCV::IncluiString) BEFORE the form holding the CV is printed.
//
// RTTI: typeinfo @1575216 (si, base CGeradorRelBase), vtable @1575372:
//   [0] ~CGeradorBUBase             wasm 5612     [1] deleting dtor icf_tiny_vf1@325
//   [2] ImprimePreTexto()           pure          [3] ImprimeProporcionalPartido()  wasm 11255 (srcloc :187)
//   [4] ImprimeMajoritario()        wasm 11254 (srcloc :246)
//   [5] ImprimeConsulta()           wasm 11253 (srcloc :286)
// srclocs :66..:111 = the ten "<parte> nulo" checks of the constructor (inlined in wasm 12110),
//         :222 = ImprimeVotosCandidatos(const std::vector<TCandidatoID>&) (inlined in 11255).
// Layout: CGeradorRelBase (36 bytes) + ten shared_ptr<IForm<IPaper>> at +36 .. +108 (116 bytes).
//
// Rendered example (samples/bu-real/run-full/reports/bu.txt):
//     ==============TREINAMENTO=============      <- m_headerProporcional (separador de fase,
//                                                    blank line, cargo name centred in '-')
//     ---------------VEREADOR---------------
//     Partido: 91 - PEsp                          <- m_headerProporcionalPartido (data slot 2921)
//     Nome do candidato       Num cand Votos
//
//       Golfe                    91001  0001      <- m_detalheCandidato (data slot 1636, func 11972)
//         Votos de legenda              0000      <- m_trailerProporcionalPartido (data slot 2922)
//         Total do partido              0001
//
//         Código Verificador: 9.977.581.304
#pragma once

#include <algorithm>
#include <format>
#include <vector>

#include "comum/dados/ccandidaturas.h"
#include "comum/dados/ccargos.h"
#include "comum/dados/cpartidos.h"
#include "comum/relatorios/ccalculacv.h"
#include "comum/relatorios/cgeradorrelbase.h"
#include "comum/relatorios/crelutil.h"

namespace comum {

template <typename RDV, typename DSAptos>
class CGeradorBUBase : public CGeradorRelBase<RDV, DSAptos> {
public:
    // Inlined into wasm 12110. Codes 9055..9064, srclocs :66, :71, :76, ... :111 (in parameter order).
    CGeradorBUBase(SharedPaperForm header, SharedPaperForm trailer, SharedPaperForm qrcode,
                   SharedPaperForm headerProporcional, SharedPaperForm headerProporcionalPartido,
                   SharedPaperForm partidoApenasVotoLegenda, SharedPaperForm trailerProporcionalPartido,
                   SharedPaperForm cargoSemCandidato, SharedPaperForm trailerCargoSemCandidato,
                   SharedPaperForm trailerProporcional, SharedPaperForm headerMajoritario,
                   SharedPaperForm detalheCandidato, SharedPaperForm trailerMajoritario)
        : CGeradorRelBase<RDV, DSAptos>(std::move(header), std::move(trailer), std::move(qrcode))
        , m_headerProporcional(std::move(headerProporcional))
        , m_headerProporcionalPartido(std::move(headerProporcionalPartido))
        , m_partidoApenasVotoLegenda(std::move(partidoApenasVotoLegenda))
        , m_trailerProporcionalPartido(std::move(trailerProporcionalPartido))
        , m_cargoSemCandidato(std::move(cargoSemCandidato))
        , m_trailerCargoSemCandidato(std::move(trailerCargoSemCandidato))
        , m_trailerProporcional(std::move(trailerProporcional))
        , m_headerMajoritario(std::move(headerMajoritario))
        , m_detalheCandidato(std::move(detalheCandidato))
        , m_trailerMajoritario(std::move(trailerMajoritario))
    {
        auto verifica = [](const SharedPaperForm& parte, int codigo, const char* mensagem) {
            if (!parte)
                throw CRelatoriosError(EUeComumRelatoriosError{codigo}, mensagem);
        };
        verifica(m_headerProporcional, 9055, "headerProporcional nulo");                    // :66
        verifica(m_headerProporcionalPartido, 9056, "headerProporcionalPartido nulo");      // :71
        verifica(m_partidoApenasVotoLegenda, 9057, "partidoApenasVotoLegenda nulo");        // :76
        verifica(m_trailerProporcionalPartido, 9058, "trailerProporcionalPartido nulo");    // :81
        verifica(m_cargoSemCandidato, 9059, "cargoSemCandidato nulo");                      // :86
        verifica(m_trailerCargoSemCandidato, 9060, "trailerCargoSemCandidato nulo");        // :91
        verifica(m_trailerProporcional, 9061, "trailerProporcional nulo");                  // :96
        verifica(m_headerMajoritario, 9062, "headerMajoritario nulo");                      // :101
        verifica(m_detalheCandidato, 9063, "detalheCandidato nulo");                        // :106
        verifica(m_trailerMajoritario, 9064, "trailerMajoritario nulo");                    // :111
    }

    // wasm func 5612 (vtable slot 0): releases the ten parts (+108 .. +36), then ~CGeradorRelBase (5609).
    ~CGeradorBUBase() override = default;

protected:
    void ImprimeProporcionalPartido() const override;   // wasm 11255
    void ImprimeMajoritario() const override;           // wasm 11254
    void ImprimeConsulta() const override;              // wasm 11253

    void ImprimeVotosCandidatos(const std::vector<TCandidatoID>& candidatos) const;   // :222, inlined in 11255

    SharedPaperForm m_headerProporcional;               // +36
    SharedPaperForm m_headerProporcionalPartido;        // +44
    SharedPaperForm m_partidoApenasVotoLegenda;         // +52  "Não há votos nominais"
    SharedPaperForm m_trailerProporcionalPartido;       // +60
    SharedPaperForm m_cargoSemCandidato;                // +68  "Não há candidatos concorrendo"
    SharedPaperForm m_trailerCargoSemCandidato;         // +76
    SharedPaperForm m_trailerProporcional;              // +84
    SharedPaperForm m_headerMajoritario;                // +92
    SharedPaperForm m_detalheCandidato;                 // +100
    SharedPaperForm m_trailerMajoritario;               // +108
};

// ------------------------------------------------------------------------------------------------------
// wasm func 11255 (srcloc :187), vtable slot 3. A proportional cargo (Vereador, Deputado...): one block per
// party with votes (ascending party number), each with the candidates that got votes.
template <typename RDV, typename DSAptos>
void CGeradorBUBase<RDV, DSAptos>::ImprimeProporcionalPartido() const
{
    m_headerProporcional->Imprime();
    const TCargoID cargo = CCargos::GetInst().GetCurrent().GetCodigo();
    auto& partidos = CPartidos::GetInst();                                          // ecourna_f819
    const RDV& rdv = RDV::GetInst();                                                // func 555
    const auto& candidaturas = CCandidaturas::GetInst();                            // wasm_entry_f521
    auto& calculadora = api::CPolySingleton<CCalculaCV>::instance();                // func 1282, :187

    if (!candidaturas.PossuiCandidatos(cargo)) {                                    // func 2271
        calculadora.IncluiString(std::format("{:02}{:04}", cargo, rdv.Cargo(cargo)));   // slot 10
        this->m_linhaVazia->Imprime();
        m_cargoSemCandidato->Imprime();
        m_trailerCargoSemCandidato->Imprime();
        return;
    }

    for (partidos.First(); !partidos.IsEnd(); partidos.Next()) {                   // 3697 = Next
        const TPartidoID partido = partidos.GetCurrent().GetNumero();              // func 1283
        if (rdv.Partido(cargo, partido) == 0)                                       // slot 5
            continue;
        calculadora.IncluiString(std::format("{:02}{:02}", cargo, partido));
        m_headerProporcionalPartido->Imprime();
        ImprimeVotosCandidatos(candidaturas.GetNumerosCandidatosAptos(cargo, partido));   // func 2272
        m_trailerProporcionalPartido->Imprime();                                    // computes its own CV
    }
    m_trailerProporcional->Imprime();
}

// srcloc :222 — inlined into 11255.
template <typename RDV, typename DSAptos>
void CGeradorBUBase<RDV, DSAptos>::ImprimeVotosCandidatos(const std::vector<TCandidatoID>& candidatos) const
{
    const md::CCargo& cargo = CCargos::GetInst().GetCurrent();
    const uebyte digitos = cargo.GetNumeroDigitos();                                // +12
    auto& candidaturas = CCandidaturas::GetInst();
    auto& calculadora = api::CPolySingleton<CCalculaCV>::instance();                // :222

    bool nenhumVoto = true;
    for (const TCandidatoID numero : candidatos) {
        const md::CCandidatura* candidatura = candidaturas.Localiza(cargo.GetCodigo(), numero);   // 1273
        // (candidatura is not null-checked: GetNumerosCandidatosAptos returned only existing numbers)
        const auto votos = RDV::GetInst().Candidato(cargo.GetCodigo(), candidatura->GetNumero(), digitos);
        if (votos == 0)
            continue;
        calculadora.IncluiString(std::format("{:05}{:04}", candidatura->GetNumero(), votos));
        m_detalheCandidato->Imprime();      // prints the CURRENT candidacy (Localiza positioned it)
        nenhumVoto = false;
    }
    if (nenhumVoto)
        m_partidoApenasVotoLegenda->Imprime();                                      // "Não há votos nominais"
}

// ------------------------------------------------------------------------------------------------------
// wasm func 11254 (srcloc :246), vtable slot 4. A majoritarian cargo (Prefeito, Governador, Senador...):
// every candidacy of the cargo that got votes, in CCandidaturas order (key cargo*1000000 + numero).
template <typename RDV, typename DSAptos>
void CGeradorBUBase<RDV, DSAptos>::ImprimeMajoritario() const
{
    m_headerMajoritario->Imprime();
    const md::CCargo& cargoAtual = CCargos::GetInst().GetCurrent();
    const uebyte digitos = cargoAtual.GetNumeroDigitos();
    const TCargoID cargo = cargoAtual.GetCodigo();
    auto& calculadora = api::CPolySingleton<CCalculaCV>::instance();                // :246
    calculadora.IncluiString(std::format("{:02}", cargo));

    auto& candidaturas = CCandidaturas::GetInst();
    const RDV& rdv = RDV::GetInst();
    if (!candidaturas.PossuiCandidatos(cargo)) {
        calculadora.IncluiString(std::format("{:02}{:04}", cargo, rdv.Cargo(cargo)));
        m_cargoSemCandidato->Imprime();
        m_trailerCargoSemCandidato->Imprime();
        return;
    }
    for (candidaturas.First(); !candidaturas.IsEnd(); candidaturas.Next()) {
        const md::CCandidatura& candidatura = candidaturas.GetCurrent();
        if (candidatura.GetCargo() != cargo)
            continue;
        const auto votos = rdv.Candidato(cargo, candidatura.GetNumero(), digitos);   // slot 3
        if (votos == 0)
            continue;
        calculadora.IncluiString(std::format("{:05}{:04}", candidatura.GetNumero(), votos));
        m_detalheCandidato->Imprime();
    }
    m_trailerMajoritario->Imprime();
}

// ------------------------------------------------------------------------------------------------------
// wasm func 11253 (srcloc :286), vtable slot 5. A referendum ("consulta"): the answers with votes, sorted
// by number, each printed with an ad-hoc one-line form (no data source).
template <typename RDV, typename DSAptos>
void CGeradorBUBase<RDV, DSAptos>::ImprimeConsulta() const
{
    m_headerMajoritario->Imprime();
    const md::CCargo& cargoAtual = CCargos::GetInst().GetCurrent();
    const uebyte digitos = cargoAtual.GetNumeroDigitos();
    const TCargoID cargo = cargoAtual.GetCodigo();
    auto& calculadora = api::CPolySingleton<CCalculaCV>::instance();                // :286
    calculadora.IncluiString(std::format("{:02}", cargo));

    std::vector<md::CRespostaConsulta> respostas = cargoAtual.GetDetalheConsulta().GetRespostas();   // 1923, copy
    std::ranges::sort(respostas, {}, &md::CRespostaConsulta::GetNumero);           // func 5610 (introsort)
    for (const auto& resposta : respostas) {
        const auto votos = RDV::GetInst().Candidato(cargo, resposta.GetNumero(), digitos);
        if (votos == 0)
            continue;
        calculadora.IncluiString(std::format("{:05}{:04}", resposta.GetNumero(), votos));
        const std::string numero = std::format("{:0{}}", resposta.GetNumero(), digitos);
        api::CPaperFormBuilder b;
        b.AddText("  " + CRelUtil::CompletaDireita(resposta.GetTexto(), 26)                 // func 1881
                      + api::CStringUtils::PadLeft(numero, ' ', 4) + "  " + std::format("{:04}", votos),
                  1, 0);                                                                     // func 753
        b.Build()->Imprime();
    }
    m_trailerMajoritario->Imprime();
}

// Template instances of std::sort for the answers (md::CRespostaConsulta, 28 bytes: {int numero;
// std::string resposta; std::string textoFonetico}), shared with CGeradorBUQRCode (wasm 5605):
//   wasm 5610 std::__introsort<...>   (recursive, 5,599 bytes)
//   wasm 248 iter_swap, 1920 __sort4, 2790 __insertion_sort_incomplete  (other units)

} // namespace comum
