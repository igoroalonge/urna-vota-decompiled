// uenux2/src/app/comum/relatorios/csubstituidortitulo.h  (path inferred from csubstituidortitulo.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u25).
//
// comum::CSubstituidorTitulo — tag substitution for the configurable report titles/footers of the
// -pu.dat (EntidadeParametrizacaoUrna: cabecalho, rodapeZEVOTA, rodapeBUVOTA, ... = SEQUENCE OF
// TituloRelatorio{alinhamento, estilo, texto}). The only tag used by VOTA is "<uf>" (replaced by the
// section's UF). Singleton, no vtable, 12 bytes (just the map).
//   instance: std::unique_ptr<CSubstituidorTitulo> @1839140 (atexit destructor = wasm 11175),
//   mutex residue @1839116 (the lock vanished in the single-threaded build).
#pragma once

#include <map>
#include <string>
#include <vector>

#include "api/gui/cpaperformbuilder.h"
#include "ecourna/app/dados/parametrizacaourna/cparametrosurna.h"   // ecourna::app::dados::CTituloRelatorio (20 bytes, u14)

namespace comum {

class CSubstituidorTitulo {
public:
    static CSubstituidorTitulo& GetInst();                                          // srcloc :21 (inlined in 3689)
    static void CreateInst(const std::map<std::string, std::string>& substituicoes); // srcloc :26 (inlined in 3689)
    static bool Existe();                                                           // inlined (name inferred)

    // wasm func 3689 (tools: "CSubstituidorTitulo::GetInst", the first srcloc it inlines). Prints a list
    // of configurable titles, creating the singleton with {"<uf>" -> uf} on first use.  name inferred
    static void IncluiTitulos(api::CPaperFormBuilder& b,
                              const std::vector<ecourna::app::dados::CTituloRelatorio>& titulos,
                              const std::string& uf);

    std::string Substitui(std::string texto) const;                                 // inlined (name inferred)

    ~CSubstituidorTitulo();                                                          // wasm 5576

private:
    explicit CSubstituidorTitulo(const std::map<std::string, std::string>& substituicoes)
        : m_substituicoes(substituicoes) {}

    std::map<std::string, std::string> m_substituicoes;                             // +0 (tag -> valor)
};

} // namespace comum
