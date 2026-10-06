// uenux2/src/app/comum/relatorios/cpartecandidatos.cpp
// Reconstructed from vota_web_wasm.wasm (unit u25).
// srclocs: :75  virtual void CParteCandidatosMajoritarios::Imprime() const   (wasm 11214)
//          :174 virtual void CParteCandidatosConsultas::Imprime() const      (wasm 11212)
// Functions of this unit: 11215, 11214, 11212 and the std::sort instantiation of 11212 (4816, 4813,
// 4814, 4815, 4809, 4810, 1506, 2578). Not executed in the recorded votes (the zerésima is printed
// before voting starts, a phase the web build skips).
//
// Rendered example (samples/bu-real/run-full/reports/ze.txt):
//     ==============TREINAMENTO=============       <- m_headerCargo
//
//     ---------------VEREADOR---------------
//     Partido: 91 - PEsp                           <- CParteCandidatosProporcionais (TituloPartido)
//     Nome do candidato             Num cand
//
//       Golfe                          91001       <- DetalheCandidatoZE
//       Beisebol                       91002

#include "comum/relatorios/cpartecandidatos.h"

#include <algorithm>
#include <format>
#include <vector>

#include "comum/dados/ccandidaturas.h"
#include "comum/dados/ccargos.h"
#include "comum/dados/crespostas.h"
#include "comum/relatorios/relatoriosdefs.h"

namespace comum {

CParteCandidatos::CParteCandidatos(SharedReportPart headerCargo, SharedReportPart semCandidatos,
                                   SharedReportPart majoritarios, SharedReportPart proporcionais,
                                   SharedReportPart consultas)
    : m_headerCargo(std::move(headerCargo))
    , m_semCandidatos(std::move(semCandidatos))
    , m_majoritarios(std::move(majoritarios))
    , m_proporcionais(std::move(proporcionais))
    , m_consultas(std::move(consultas))
{
}

// wasm func 11215 (vtable slot 2; tools: CParteCandidatos::vf2).
void CParteCandidatos::Imprime() const
{
    const md::CCargo& cargo = CCargos::GetInst().GetCurrent();
    m_headerCargo->Imprime();

    const IReportPart* parte = nullptr;
    if (cargo.TemDetalheCandidato() && !CCandidaturas::GetInst().PossuiCandidatos(cargo.GetCodigo()))   // 2271
        parte = m_semCandidatos.get();
    else if (cargo.TemDetalheCandidato() && cargo.GetTipo() == md::CCargo::ETipo::MAJORITARIO)
        parte = m_majoritarios.get();
    else if (cargo.TemDetalheCandidato() && cargo.GetTipo() == md::CCargo::ETipo::PROPORCIONAL)
        parte = m_proporcionais.get();
    else if (cargo.TemDetalheConsulta())                                                  // CCargo +136
        parte = m_consultas.get();
    else
        return;
    parte->Imprime();
}

CParteCandidatosMajoritarios::CParteCandidatosMajoritarios(SharedPaperForm header, SharedPaperForm detalhe,
                                                           SharedPaperForm trailer)
    : m_header(std::move(header)), m_detalhe(std::move(detalhe)), m_trailer(std::move(trailer))
{
}

// wasm func 11214 (srcloc :75). Every apt candidate of the cargo, in CCandidaturas order; Localiza
// positions the candidacy cursor that DetalheCandidatoZE reads.
void CParteCandidatosMajoritarios::Imprime() const
{
    const TCargoID cargo = CCargos::GetInst().GetCurrentCargoID();                        // func 1938
    auto& candidaturas = CCandidaturas::GetInst();                                        // wasm_entry_f521
    const std::vector<TCandidatoID> numeros = candidaturas.GetNumerosCandidatosAptos(cargo);   // ecourna_f2840
    m_header->Imprime();
    for (const TCandidatoID numero : numeros) {
        if (candidaturas.Localiza(cargo, numero) == nullptr)                              // shared_f1273
            throw CRelatoriosError(EUeComumRelatoriosError{9081},
                                   std::format("Candidato não encontrado: {}/{}", cargo, numero));   // :75
        m_detalhe->Imprime();
    }
    m_trailer->Imprime();
}

CParteCandidatosConsultas::CParteCandidatosConsultas(SharedPaperForm header, SharedPaperForm detalhe,
                                                     SharedPaperForm trailer)
    : m_header(std::move(header)), m_detalhe(std::move(detalhe)), m_trailer(std::move(trailer))
{
}

// wasm func 11212 (srcloc :174). The answers of the referendum, by number. CRespostas is a CDataMap keyed
// by cargo * 1000000 + número (the same scheme as CCandidaturas).
void CParteCandidatosConsultas::Imprime() const
{
    const TCargoID cargo = CCargos::GetInst().GetCurrentCargoID();
    auto& respostas = CRespostas::GetInst();                                              // func 2812

    std::vector<TRespostaID> numeros;
    const unsigned inicio = cargo * 1000000u;
    const unsigned fim = static_cast<uebyte>(cargo + 1) * 1000000u;
    for (const auto& [chave, resposta] : respostas.GetMapa())
        if (chave >= inicio && chave < fim)
            numeros.push_back(resposta.GetNumero());                                      // value +0 (func 543)
    std::sort(numeros.begin(), numeros.end());                                            // func 4816

    m_header->Imprime();
    for (const TRespostaID numero : numeros) {
        if (respostas.Localiza(cargo, numero) == nullptr)                                 // shared_f1273
            throw CRelatoriosError(EUeComumRelatoriosError{9083},
                                   std::format("Resposta não encontrada: {}/{}", cargo, numero));   // :174
        m_detalhe->Imprime();
    }
    m_trailer->Imprime();
}

// std::sort(vector<TRespostaID>) — libc++ instantiation for an arithmetic type with std::less, which uses
// the branch-free sorting networks and the bitset partition:
//   wasm 4816  std::__introsort<_ClassicAlgPolicy, __less<>&, unsigned*, /*branchless*/true>  (recursive)
//   wasm 4813  std::__insertion_sort_incomplete           wasm 4809  std::__sift_down (heap fallback)
//   wasm 1506  std::__sort3_maybe_branchless              wasm 4815  std::__sort4_maybe_branchless
//   wasm 4814  std::__sort5_maybe_branchless              wasm 2578  std::__partially_sorted_swap
//   wasm 4810  std::__swap_bitmap_pos (bitset partition)

} // namespace comum
