// uenux2/src/app/vota/operador/comum/iinformacaothreadoperador.h  -- FRAGMENT written by unit u33.
// Three more out-of-line one-liners "IInformacaoThreadOperador::GetInst().slotN(...)" (the same pattern as
// funcs 1687, 2226, 2745, 3619, 3621, 3623, 5414-5416 listed by units u10/u19). The interface and its
// default implementation are reconstructed by unit u10 (iinformacaothreadoperador.h,
// cinformacaothreadoperador.cpp); GetInst = func 599 (unit u19).
// None of them runs in the web build (operator thread).
#include <string>

#include "comum/md/celeitoridentidade.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"

namespace vota {

// wasm func 1535 - slot 25 GetIdentidadeEleitor() (srcloc cinformacaothreadoperador.cpp:494, err 9403).
// Returned by value: the wasm32 ABI passes the sret pointer BEFORE `this`, which is why the decompiled call
// looks like vf25(result, instance).                                               // name inferred
// Callers: comum::md::CBiometriaEleitor::GetFoto (3616), CMostraEleitorVotando::SalvaHabilitacaoEleitor (10425),
// IIniciaJustificativa slot 2 (10587), CPedeAnoNascimento::ProcessInput (10590), CProcuraEleitor slot 2 (10631).
inline comum::md::CEleitorIdentidade IdentidadeEleitor()
{
    return impl::IInformacaoThreadOperador::GetInst().GetIdentidadeEleitor();
}

// wasm func 2746 - slot 3 DeveHabilitarAudio(): manual audio on, or the voter's registration asks for it.
// Callers: the release-voter body 3883, CNomeEleitor (5400), CDigitalReconhecida (10481),
// CControlaReconhecimento (10511).                                                  // name inferred
inline bool DeveHabilitarAudio()
{
    return impl::IInformacaoThreadOperador::GetInst().DeveHabilitarAudio();
}

// wasm func 3620 - slot 17 SorteiaProximaInspecao(): draws the time of the next random inspection
// ("inspeção") of the voting booth, now + 60..90 min (unit u10).                    // name inferred
// Callers: the CPedeIdentidade constructor (inlined in its GetInst, func 652), CConfirmaInspecionada
// ProcessInput (10527), CPedeIdentidade slot 8 (10676).
inline void SorteiaProximaInspecao()
{
    impl::IInformacaoThreadOperador::GetInst().SorteiaProximaInspecao();
}

} // namespace vota
