// uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.h   (path inferred from the .cpp source_location)
//
// Reconstructed from vota_web_wasm.wasm (unit u02). vota::CInformacaoEleitor has no RTTI (it is not
// polymorphic); it is known from one std::source_location record,
//   cinformacaoeleitor.cpp:181  void vota::CInformacaoEleitor::CarregarDadosEstaticos()
// found inside wasm func 7787, and from its lazily created singleton (wasm func 509, 12 bytes at
// the global pointer 1833284, freed at exit by wasm func 13695).
//
// "Informação do eleitor": the voter-side ("eleitor" = voter terminal) start-up state of VOTA:
// which data sets are loaded and how the voter audio is configured.

#pragma once

#include <cstdint>

namespace vota {

class CInformacaoEleitor
{
public:
    /// wasm func 509 (not in unit u02). Lazily creates the 12-byte object:
    /// {m_dadosEstaticosCarregados=0, m_dadosDinamicosCarregados=0, m_modoAudio=2, 0, 0, 0}.
    static CInformacaoEleitor& GetInst();

    /// wasm func 7787 (name inferred; formerly shown by the tools as CHKDFSeed::GetSeed, because the first
    /// source_location it contains belongs to that inlined function). The web entry point
    /// votaInit / CVotaWebEngine::Init (wasm func 7840) calls it once, through invoke_v slot 39.
    static void Inicializar();                               // name inferred

    /// wasm func 6734 (owned by another unit; mis-named comum::CEleitores::CompleteLoad because
    /// CEleitores::CompleteLoad is inlined into it). Guarded by m_dadosDinamicosCarregados (+1).
    void CarregarDadosDinamicos();                           // name inferred

    /// wasm func 6737 (owned by another unit). Called by votaInit after Inicializar() and by
    /// vota::CGeraDadosDinamicos::StartState.
    void GerarDadosDinamicos();                              // name inferred

private:
    // The following members exist only inlined into wasm func 7787.
    void ApagarDadosDinamicos();                             // name inferred
    void InicializarPersistencia();                          // name inferred
    void CarregarDadosEstaticos();                           // srcloc cinformacaoeleitor.cpp:181

    bool         m_dadosEstaticosCarregados = false;         // +0   set at the end of CarregarDadosEstaticos
    bool         m_dadosDinamicosCarregados = false;         // +1   tested by wasm func 6734
    std::int32_t m_modoAudio = 2;                            // +4   (?) tested "!= 2" by the audio states
    bool         m_persistenciaInicializada = false;         // +8   set by InicializarPersistencia
    std::uint8_t m_flag9 = 0;                                // +9   (?) read by wasm func ~12500
    std::uint8_t m_flag10 = 0;                               // +10  (?)
};

} // namespace vota
