// uenux2/src/app/comum/relatorios/ccalculacv.h  (path inferred from ccalculacv.cpp srclocs)
// Reconstructed from vota_web_wasm.wasm (unit u25; see also ccalculacv.u13.cpp and
// docs/bu/codigo-verificador.md, which re-ran the whole chain in Python).
//
// comum::CCalculaCV — computes the "Código Verificador" (CV) printed under each block of the Boletim de
// Urna: a chain of SipHash-4-6 MACs over the printed numbers, keyed with the 16-byte secret of
// /dsk/fi/estatico/chave/cv.ber.pri. No vtable, 56 bytes (operator new(56) in vota::CGeraBU::StartState).
// It lives in api::CPolySingletonList while the BU is generated: CGeraBU pushes it (cpolysingletonlist.h:129)
// and ~CGeradorBU erases it (wasm 3698); the report data sources reach it with
// api::CPolySingleton<CCalculaCV>::instance() (wasm 1282) and test it with exists<CCalculaCV>() (wasm 1954).
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "comum/tipos.h"   // uebyte

namespace comum {

class CCargos;

class CCalculaCV {
public:
    // Inlined into vota::CGeraBU::StartState (wasm 12110). srclocs ccalculacv.cpp:58 / :65 / :74.
    CCalculaCV(const std::string& identificacao, const std::string& primeiroCV, char tipo,
               std::size_t tamanhoChave);
    ~CCalculaCV();                                        // wasm 5614 (unit u08 file), 11263 via shared_ptr

    void IncluiString(const std::string& texto);          // wasm 942  (srcloc :109)
    std::string Calcula();                                // wasm 3700 (u13 fragment)               name inferred
    const std::string& GetUltimoCV() const { return m_ultimoCV; }   // inlined into 5967        name inferred

private:
    void Reinicia(const std::string& cv);                 // wasm 5615 (tools: ecourna_f5615)       name inferred

    std::string m_identificacao;      // +0   {municipio:05}{zona:04}{secao:04}{pleito...}{idUE}{pleito}{fase}
    char m_tipo;                      // +12  'F' (oficial) or 'A' (demonstration mode)  (cgeradorbu.cpp:161 DefineTipo)
    std::string m_dados;              // +16  running buffer MACed by Calcula()
    std::vector<uebyte> m_chave;      // +28  16-byte SipHash key
    std::string m_ultimoCV;           // +40  decimal text of the last full 64-bit value
    unsigned m_contador = 0;          // +52  number of CVs computed so far (never reset)
};

// wasm 5967 (tools: IncluiString@5967): the "final strings" of the chain. NOT a CCalculaCV member: its wasm
// parameters are (const CCargos&, CCalculaCV&) in that order (both callers, 11260 and 11974, pass
// CCargos::GetInst() first and the calculator second), whereas a member would receive `this` first.
// Name and home file inferred (the tools filed it in ccalculacv.cpp because IncluiString is inlined at its head).
void IncluiFinal(const CCargos& cargos, CCalculaCV& calculadora);

} // namespace comum
