// uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.h   (path inferred: RTTI only; siblings in
// comum/relatorios). Reconstructed from vota_web_wasm.wasm (unit u35; methods also in the fragments of units u02
// (cgeradorrelversaopacotedados.u02.cpp) and u26 (comum/u26-foreign-fragments.cpp)).
//
// "Versões de Pacotes" report ("versões dos pacotes de dados", item of the "Mais informações" menu,
// vota::CImpressaoVersaoPacotes::StartState = func 11905): one line per data package loaded into the urna
// ("<nome do pacote>      <versão>"), read from dadoscarga.dat (DadosDisponiveisCarga, CConversorDadosDisponiveisCarga),
// plus a short hash of the package versions recorded in eg.bin.
//
// RTTI: comum::CGeradorRelVersaoPacoteDados (class without base), vtable @1576284:
//   [0] 3691 ~CGeradorRelVersaoPacoteDados  [1] 11224 deleting  [2] 5592 Imprime (u26)  [3] 11223 MontaCorpo (u02)
//   [4] 11222 MontaDados (this unit)        [5] 5596 MontaRodape (u26)
#pragma once

#include <string>
#include <vector>

#include "api/gui/cpaperformbuilder.h"
#include "comum/md/ccabecalhopacote.h"

namespace comum {

class CGeradorRelVersaoPacoteDados
{
public:
    explicit CGeradorRelVersaoPacoteDados(std::vector<md::CCabecalhoPacote> pacotes)   // inlined in 11905
        : m_pacotes(std::move(pacotes)) {}
    virtual ~CGeradorRelVersaoPacoteDados();                                            // func 3691 (+ 11224)

    virtual void Imprime(const std::string& titulo, const std::string& via);           // slot 2, func 5592 (u26)

protected:
    virtual void MontaCorpo();                                                          // slot 3, func 11223 (u02)
    virtual void MontaDados();                                                          // slot 4, func 11222
    virtual void MontaRodape();                                                         // slot 5, func 5596 (u26)

    api::CPaperFormBuilder m_relatorio;                                                 // +4
    std::vector<md::CCabecalhoPacote> m_pacotes;                                        // +16 (72-byte elements)
};

} // namespace comum
