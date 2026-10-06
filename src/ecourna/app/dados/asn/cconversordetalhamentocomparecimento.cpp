// ecourna-lib/ecourna/app/dados/asn/cconversordetalhamentocomparecimento.cpp   (path inferred, see the .h)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40 (DoConverte); DoDeconverte (9221) added for completeness.
#include "ecourna/app/dados/asn/cconversordetalhamentocomparecimento.h"

#include "ecourna/api/pattern/cbasetype.hpp"

namespace ecourna::app::dados::asn {

using TQtdEleitor = api::pattern::CBaseType<unsigned short, 0, 9999, 15>;   // "QtdEleitor" (func 1960)

// wasm func 9222 (vtable slot 2). BU encerramento path (not observed: the web page never closes the
// section). Each count goes through a Constrained_INTEGER<0, 9999> temporary (vtable @1598604) whose value
// is then stored into the field; no range check happens here (Converte validates the whole entity after).
ModuloBoletimUrna::DetalhamentoComparecimento
CConversorDetalhamentoComparecimento::DoConverte(const CDetalhamentoComparecimento& dado) const
{
    ModuloBoletimUrna::DetalhamentoComparecimento entidade;
    entidade.set_qtdEleitoresCompareceramSemBiometria(dado.GetQtdCompareceramSemBiometria());
    entidade.set_qtdEleitoresHabilitadosPorBiometria(dado.GetQtdHabilitadosPorBiometria());
    entidade.set_qtdEleitoresHabilitadosPorBiografia(dado.GetQtdHabilitadosPorBiografia());
    return entidade;
}

// wasm func 9221 (vtable slot 3, not in u40): each field through CBaseType<unsigned short, 0, 9999, 15>
// (CPatternError 1300 if > 9999), then the constructor func 5094.
CDetalhamentoComparecimento
CConversorDetalhamentoComparecimento::DoDeconverte(const ModuloBoletimUrna::DetalhamentoComparecimento& entidade) const
{
    const TQtdEleitor semBiometria(entidade.get_qtdEleitoresCompareceramSemBiometria());
    const TQtdEleitor porBiometria(entidade.get_qtdEleitoresHabilitadosPorBiometria());
    const TQtdEleitor porBiografia(entidade.get_qtdEleitoresHabilitadosPorBiografia());
    return CDetalhamentoComparecimento(semBiometria, porBiometria, porBiografia);
}

}  // namespace ecourna::app::dados::asn
