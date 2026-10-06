// uenux2/src/app/comum/dados/asn/eleitor/cvisitanteeleitor.h
// Reconstructed from vota_web_wasm.wasm (unit u03).
#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <utility>

#include "asn1/iabstractvisitor.h"
#include "comum/dados/md/eleitor/celeitoridentidade.h"

namespace comum::asn {

// RTTI: comum::asn::CVisitanteEleitor : ASN1::IAbstractVisitor
// vtable @1567212: [0] 5698 ~CVisitanteEleitor [1] 11408 deleting dtor [2] 11407 visit
// Allocated with operator new(64) and held in a std::shared_ptr by the partial decoder of the voter file
// (IConversorParcialASN<EntidadeEleitores, CEntidadeEleitores, CVisitanteEleitor>::Desconverte, inlined in
// func 11523 = the std::function behind comum::CEleitores::GetEleitoresEstaticos).
//
// Streaming callback of ASN1::BERDecoderPartial (docs/libraries/asn1-runtime.md §6). It is registered for two
// paths and records, for every voter, WHERE in the file the (still encrypted) biometrics are, keyed by each of
// the voter's identities, so that the voter roll can be loaded without the fingerprint data.
class CVisitanteEleitor : public ASN1::IAbstractVisitor
{
public:
    using TPosicao = std::pair<std::size_t, std::size_t>;               // [begin, end) offsets in the file   // name inferred
    using TMapaPosicoes = std::map<md::CEleitorIdentidade, TPosicao>;   // name inferred

    ~CVisitanteEleitor() override = default;   // wasm func 5698 (complete), 11408 (deleting)

    EVisitorResult visit(int nivel, const std::string& caminho, ASN1::AbstractData& dado,
                         std::size_t inicio, std::size_t fim) override;   // wasm func 11407

    const TMapaPosicoes& GetPosicoes() const { return m_posicoes; }   // name inferred

private:
    // +4  int — written by the partial converter before decoding (1 for the voter file); not read here
    TPosicao m_posicaoBiometria;   // +8
    std::string m_inscricao;       // +16  last IdentificadorEleitor.numeroInscricao seen
    std::string m_cpf;             // +28  last numeroCPF seen
    std::string m_livre;           // +40  last identificacaoLivre seen
    TMapaPosicoes m_posicoes;      // +52
};

} // namespace comum::asn
