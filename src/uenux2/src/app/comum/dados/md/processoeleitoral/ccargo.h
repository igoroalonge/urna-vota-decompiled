// uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.h  (path inferred from ccargo.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u22; see also ccargo.u07.cpp).
//
// comum::md::CCargo - one office ("cargo": Prefeito, Vereador, Senador...) or one referendum question
// ("consulta") of an eleição. Built by comum::asn::CConversorCargo from ModuloEleicao::CargoPergunta
// (u03), which calls Valida(). 140 bytes:
//   +0   TCargoID m_codigo (uebyte)
//   +4   ETipo m_tipo                         0 majoritário, 1 proporcional, 2 consulta
//   +8   int m_abrangencia                    < 3 (federal / estadual / municipal)
//   +12  uebyte m_numeroDigitos               1..5
//   +13  TQtdEscolha m_qtdEscolhas            >= 1 (2 for Senador when two seats are elected)
//   +18  uebyte m_paginaImpressaoVoto          1..5
//   +20  std::optional<CDetalheCandidato>      engaged flag +84
//          +20 bool temFoto; +24 nomeNeutro; +36 nomeMasculino; +48 nomeFeminino; +60 nomeAbreviado;
//          +72 std::vector<CSuplencia> (52-byte elements)
//   +88  std::optional<CDetalheConsulta>       engaged flag +136
//          +88 nome; +100 pergunta; ... respostas
#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "comum/dados/md/csexo.h"
#include "comum/dados/md/processoeleitoral/cdetalhecandidato.h"
#include "comum/dados/md/processoeleitoral/cdetalheconsulta.h"

namespace comum::md {

using TCargoID = std::uint8_t;
using TQtdEscolha = std::uint8_t;

class CCargo
{
public:
    enum class ETipo : int { MAJORITARIO = 0, PROPORCIONAL = 1, CONSULTA = 2 };   // names inferred (u07)

    TCargoID GetCodigo() const { return m_codigo; }
    TQtdEscolha GetQtdEscolhas() const { return m_qtdEscolhas; }

    const CDetalheCandidato& GetDetalheCandidato() const;                     // func 1388 (srcloc :81)
    const CDetalheConsulta& GetDetalheConsulta() const;                       // func 1923 (srcloc :89)

    // Names: candidate cargos use the NomesCargo of the DetalheCargo, consultas their own name.
    std::string GetNome() const;                                              // func 1547 (neutro)       name inferred
    std::string GetNomeMasculino() const;                                     // func 3716                name inferred
    std::string GetNomeAbreviado() const;                                     // func 2797                name inferred
    std::string GetNome(CSexo::ESexo sexo) const;                             // func 2796                name inferred
    std::string GetNomeSuplente(uebyte suplente, CSexo::ESexo sexo) const;    // func 3715                name inferred

    std::string GetOrdinalEscolha(TQtdEscolha escolha) const;                 // func 2258 (srcloc :112)
    void Valida() const;                                                      // func 5657 (srcloc :212..240)

private:
    TCargoID m_codigo;                                   // +0
    ETipo m_tipo;                                        // +4
    int m_abrangencia;                                   // +8
    std::uint8_t m_numeroDigitos;                        // +12
    TQtdEscolha m_qtdEscolhas;                           // +13
    std::uint8_t m_paginaImpressaoVoto;                  // +18
    std::optional<CDetalheCandidato> m_detalheCandidato; // +20 (flag +84)
    std::optional<CDetalheConsulta> m_detalheConsulta;   // +88 (flag +136)
};

} // namespace comum::md
