// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcargo.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// One cargo (office) or consulta (referendum question) of an eleição, from <...>-ce.dat:
//   CargoPergunta ::= SEQUENCE { codigo CodigoCargoConsulta, tipo TipoCargoConsulta, numeroDigitos INTEGER (1..5),
//     qtdeEscolhas INTEGER (1..50), podeRepetir BOOLEAN, ordemAquisicao/ordemImpressao/ordemApuracao INTEGER (1..99),
//     paginaImpressaoVoto INTEGER (1..5), detalhe CHOICE { cargo [0] DetalheCargo, pergunta [1] DetalhePergunta } }
//   TipoCargoConsulta ::= ENUMERATED { majoritario(1), proporcional(2), consulta(3) }
// This is the data that decides, on the voting screen, how many digits a number has and how many
// choices the voter makes for the cargo.
#include "comum/dados/asn/processoeleitoral/cconversorcargo.h"

#include <format>

#include "comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.h"
#include "comum/dados/asn/processoeleitoral/cconversordetalhecandidato.h"
#include "comum/dados/asn/processoeleitoral/cconversordetalheconsulta.h"

namespace comum::asn {

// Inlined into wasm func 11373 (srcloc line 83)
md::CCargo::ETipo CConversorCargo::DesconverteTipoCargo(const ModuloTiposEleitorais::TipoCargoConsulta::NamedNumber tipo)
{
    const int valor = static_cast<int>(tipo) - 1;
    if (valor < 0 || valor > 2) {
        throw CDadosError(7949, std::format("Tipo inválido: {}", tipo));   // line 83 (enum formatter, slot 2737)
    }
    return static_cast<md::CCargo::ETipo>(valor);   // majoritario->0, proporcional->1, consulta->2
}

// wasm func 11373 (vtable slot 3; srcloc lines 69, 83)
md::CCargo CConversorCargo::DoDesconverte(const TEntidade& cargo) const
{
    const uebyte codigo = CConversorCodigoCargoConsulta().Desconverte(cargo.get_codigo());   // func 5828
    const md::CCargo::ETipo tipo = DesconverteTipoCargo(cargo.get_tipo());

    const auto numeroDigitos = static_cast<uebyte>(cargo.get_numeroDigitos());
    const auto qtdeEscolhas = static_cast<uebyte>(cargo.get_qtdeEscolhas());
    const bool podeRepetir = cargo.get_podeRepetir();
    const auto ordemAquisicao = static_cast<uebyte>(cargo.get_ordemAquisicao());
    const auto ordemImpressao = static_cast<uebyte>(cargo.get_ordemImpressao());
    const auto ordemApuracao = static_cast<uebyte>(cargo.get_ordemApuracao());
    const auto paginaImpressaoVoto = static_cast<uebyte>(cargo.get_paginaImpressaoVoto());

    const auto& detalhe = cargo.get_detalhe();
    switch (detalhe.currentSelection()) {
    case ModuloEleicao::DetalheCargoPergunta::cargo_: {       // choice id 0
        const auto detalheCandidato = CConversorDetalheCandidato().Desconverte(detalhe.get_cargo());
        // md::CCargo constructor (inlined): stores the fields and calls md::CCargo::Valida() (func 5657).
        return md::CCargo(codigo, tipo, m_abrangencia, numeroDigitos, qtdeEscolhas, podeRepetir, ordemAquisicao,
                          ordemImpressao, ordemApuracao, paginaImpressaoVoto, detalheCandidato);
    }
    case ModuloEleicao::DetalheCargoPergunta::pergunta_: {    // choice id 1
        const auto detalheConsulta = CConversorDetalheConsulta().Desconverte(detalhe.get_pergunta());
        return md::CCargo(codigo, tipo, m_abrangencia, numeroDigitos, qtdeEscolhas, podeRepetir, ordemAquisicao,
                          ordemImpressao, ordemApuracao, paginaImpressaoVoto, detalheConsulta);
    }
    }
    throw CDadosError(7948, "Opção de Detalhe Inválida.");   // line 69
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Library code emitted in this TU: wasm func 3018 — std::vector<md::CResposta>::__init_with_size (copy of the
// consulta's answer list, 28-byte elements {int numero; std::string resposta; std::string textoFonetico}),
// used by the md::CDetalheConsulta copy inside the CCargo constructor (also called by comum_f374, vota_f6627,
// CConversorDetalheConsulta::DoDesconverte).
