// ecourna-lib/ecourna/app/dados/asn/cconversorcabecalhoentidade.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/cconversorcabecalhoentidade.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   CabecalhoEntidade ::= SEQUENCE { dataGeracao DataHoraJE ("YYYYMMDDThhmmss"),
//                                    idEleitoral IDEleitoral CHOICE { idProcessoEleitoral [1],
//                                                                     idPleito [2], idEleicao [3] } }
//   <-> CCabecalhoEntidade { +0 ptime dataGeracao, +8 int id, +12 tipo of id (0 PE, 1 pleito, 2 eleição) }
// This is the header of every ecourna data file (-pu, -cfm, -fe, the RC attendance file...).
// DoDeconverte was observed executing during the recorded votes (start-up file loading).
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// wasm func 9219 (vtable slot 2; srcloc line 48)
ModuloTiposEleitorais::CabecalhoEntidade CConversorCabecalhoEntidade::DoConverte(const CCabecalhoEntidade& cabecalho) const
{
    ModuloTiposEleitorais::CabecalhoEntidade entidade;
    // func 9220: std::format("{:04}{:02}{:02}T{:02}{:02}{:02}", ...) of the ptime (DataHoraJE)
    entidade.set_dataGeracao(FormataDataHoraJE(cabecalho.GetDataGeracao()));
    switch (cabecalho.GetTipoId()) {
    case CCabecalhoEntidade::IdProcessoEleitoral:  entidade.ref_idEleitoral().select_idProcessoEleitoral() = cabecalho.GetId(); break;
    case CCabecalhoEntidade::IdPleito:             entidade.ref_idEleitoral().select_idPleito() = cabecalho.GetId(); break;
    case CCabecalhoEntidade::IdEleicao:            entidade.ref_idEleitoral().select_idEleicao() = cabecalho.GetId(); break;
    case CCabecalhoEntidade::IdInvalido:           // ? value 3, an explicit sentinel enumerator
        throw CAsnError(2235, "Tipo de id de cabeçalho inválido.");   // line 48
    }
    // Any other value (>= 4, or negative: the br_table index is unsigned) falls through and leaves
    // idEleitoral unselected; the result is then rejected by the validity check of
    // IConversorASN::Converte.
    return entidade;
}

// wasm func 9218 (vtable slot 3; srcloc line 85)
CCabecalhoEntidade CConversorCabecalhoEntidade::DoDeconverte(const TEntidade& cabecalho) const
{
    const auto& id = cabecalho.get_idEleitoral();
    const int tipo = id.currentSelection();
    if (static_cast<unsigned>(tipo) >= 3) {                          // also catches "unselected" (-1)
        throw CAsnError(2236, "Tipo de id de cabeçalho inválido.");  // line 85
    }
    const int valor = id.getSelection().asInt();
    // func 1877: DataHoraJE -> ptime (boost parse_iso_time)
    return CCabecalhoEntidade(ConverteDataHoraJE(cabecalho.get_dataGeracao()), valor,
                              static_cast<CCabecalhoEntidade::ETipoId>(tipo));   // func 5112
}

} // namespace ecourna::app::dados::asn
