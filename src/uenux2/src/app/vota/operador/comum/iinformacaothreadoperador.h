// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original (path inferred): uenux2/src/app/vota/operador/comum/iinformacaothreadoperador.h
// (the implementation file cinformacaothreadoperador.cpp is attested by std::source_location records;
//  IInformacaoThreadOperador::GetInst is cinformacaothreadoperador.cpp:554, wasm func 599, unit u19).
//
// "Informação da thread do operador": the blackboard shared by the states of the operator thread
// (CThreadOperador = the mesário's terminal, "micro-terminal" MT). While the mesário identifies a voter
// and releases ("habilita") him, each state leaves here what it learnt: the identity typed on the
// keypad, the identity found in the roll, how the voter was released (fingerprint / birth year / code),
// whether audio must be enabled, and the result of showing the voter's photo. CMostraEleitorVotando
// reads it back when the voter thread reports that the vote started (SalvaHabilitacaoEleitor).
//
// Registered through api::CPolySingletonList (default implementation vota::impl::CInformacaoThreadOperador,
// pushed by GetInst when no platform registered another one).
//
// RTTI: vota::impl::IInformacaoThreadOperador (typeinfo @1590320, class)
//       vota::impl::CInformacaoThreadOperador (typeinfo @1590328, si) - vtable @1590116, 31 slots.
// Slot names below are inferred from behaviour and callers except where a srcloc record exists
// (slots 23-26). The thin "api_f..." thunks listed per slot are one-line outlined calls
// `IInformacaoThreadOperador::GetInst().X()` kept by the optimizer (they live in the callers' units).
#pragma once

#include <string>

#include "api/util/cdatetime.h"                                  // api::CDateTime
#include "comum/dados/md/eleitor/celeitoridentidade.h"           // comum::md::CEleitorIdentidade
#include "comum/dados/celeitordetalhe.h"                         // comum::md::ETipoHabilitacao
#include "ecourna/app/dados/capresentacaofotoeleitor.h"          // ecourna::app::dados::CApresentacaoFotoEleitor

namespace vota::impl {

using ecourna::app::dados::ETipoIdentificadorEleitor;

class IInformacaoThreadOperador {
public:
    /// cinformacaothreadoperador.cpp:554 (wasm func 599): CPolySingletonList::instance<IInformacaoThreadOperador>(),
    /// pushing a new CInformacaoThreadOperador on first use.
    static IInformacaoThreadOperador& GetInst();

    virtual ~IInformacaoThreadOperador() = default;                              // slots 0 / 1

    // ---- state of the current habilitação -------------------------------------------------
    virtual void LimpaDadosHabilitacao() = 0;                                     // slot 2  (CCancelaHabilitacaoEleitor)
    virtual bool DeveHabilitarAudio() const = 0;                                  // slot 3  (thunk api_f2746)
    virtual bool GetAudioAtivado() const = 0;                                     // slot 4
    virtual bool EleitorNecessitaAudio() const = 0;                               // slot 5  (CHabilitaAudioEleitor)
    virtual bool GetAudioHabilitadoManualmente() const = 0;                       // slot 6  (thunk api_f1687)
    virtual void SetAudioHabilitadoManualmente(bool habilitado) = 0;              // slot 7  (thunk api_f5415)
    virtual std::string GetTextoAudio() const = 0;                                // slot 8  (thunk api_f10584)
    virtual bool HabilitadoPorCodigoMesario() const = 0;                          // slot 9
    virtual bool HabilitadoPorBiometria() const = 0;                              // slot 10
    virtual bool HabilitadoSemBiometria() const = 0;                              // slot 11
    virtual void SetHabilitacaoCodigoMesario() = 0;                               // slot 12 (CRegistraDigitalOperador)
    virtual void SetHabilitacaoBiometrica() = 0;                                  // slot 13 (CDigitalReconhecida)
    virtual void SetHabilitacaoSemBiometria() = 0;                                // slot 14 (thunk api_f3621)
    virtual comum::md::ETipoHabilitacao GetTipoHabilitacao() const = 0;           // slot 15 (SalvaHabilitacaoEleitor)

    // ---- operator panel / timers ------------------------------------------------------------
    virtual std::string GetTextoQtdVotaram() const = 0;                           // slot 16 (thunk libcxx_f10583)
    virtual void SorteiaProximaInspecao() = 0;                                    // slot 17 (thunk api_f3620)
    virtual api::CDateTime GetDataHoraProximaInspecao() const = 0;                // slot 18 (CPedeIdentidade::ProcessTick)
    virtual bool VotacaoBloqueadaPorHorario() const = 0;                          // slot 19 (thunk api_f2226)

    // ---- identity of the voter being released -----------------------------------------------
    virtual void LimpaIdentidades() = 0;                                          // slot 20 (CPedeIdentidade::StartState)
    virtual void SetIdentidadeDigitada(const std::string& identidade) = 0;        // slot 21 (thunk api_f5414)
    virtual void SetTipoIdentidadeDigitada(ETipoIdentificadorEleitor tipo) = 0;   // slot 22 (thunk api_f3619)
    virtual std::string GetIdentidadeDigitada() const = 0;                        // slot 23 (srcloc :474)
    virtual ETipoIdentificadorEleitor GetTipoIdentidadeDigitada() const = 0;      // slot 24 (srcloc :484)
    virtual comum::md::CEleitorIdentidade GetIdentidadeEleitor() const = 0;       // slot 25 (srcloc :494)
    virtual comum::md::CEleitorIdentidade GetIdentidadePrincipalEleitor() const = 0;  // slot 26 (srcloc :504)

    // ---- voter photo shown on the MT LCD (see operador/comum/capresentacaofotoeleitor.cpp, u05) ----
    virtual void SetEleitorSemFoto() = 0;                                         // slot 27 -> {0, 0}
    virtual void SetFotoApresentada() = 0;                                        // slot 28 -> {1, 0}
    virtual void SetFotoNaoApresentadaPorErro(int resultado) = 0;                 // slot 29 -> {2, resultado}
    virtual ecourna::app::dados::CApresentacaoFotoEleitor GetApresentacaoFoto() const = 0;   // slot 30
};

}  // namespace vota::impl
