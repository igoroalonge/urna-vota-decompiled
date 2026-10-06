// uenux2/src/app/comum/dados/asn/candidatura/cvisitantefoto.h
// Reconstructed from vota_web_wasm.wasm (unit u03).
#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "asn1/iabstractvisitor.h"

namespace comum::asn {

// RTTI: comum::asn::CVisitanteFoto : ASN1::IAbstractVisitor
// vtable @1562444: [0] 5709 ~CVisitanteFoto [1] 11443 deleting dtor [2] 11442 visit
// Allocated with operator new(28) and shared with the partial converter CIndexadorFotos
// (IConversorParcialASN<EntidadeFotosCandidatos, md::CIndicesFotos, CVisitanteFoto>, inlined in func 7787,
// cfotos.cpp), which resets m_sequenciador to 0 before decoding.
//
// It indexes the candidate-photo file (<...>-fo.dat, EntidadeFotosCandidatos) without keeping the images in memory:
// for each FotoCandidato it records the [begin, end) byte range of the element and the candidate code.
class CVisitanteFoto : public ASN1::IAbstractVisitor
{
public:
    using TPosicao = std::pair<std::size_t, std::size_t>;         // name inferred
    using TIndice = std::pair<std::string, TPosicao>;             // 20 bytes {codigoCandidato, posição}   // name inferred

    ~CVisitanteFoto() override = default;   // wasm func 5709 (complete), 11443 (deleting)

    EVisitorResult visit(int nivel, const std::string& caminho, ASN1::AbstractData& dado,
                         std::size_t inicio, std::size_t fim) override;   // wasm func 11442

    const std::vector<TIndice>& GetIndices() const { return m_indices; }   // name inferred

private:
    int m_sequenciador = 0;        // +4  0 = expecting a photo element, 1 = expecting its candidate code
    TPosicao m_posicao;            // +8
    std::vector<TIndice> m_indices; // +16
};

} // namespace comum::asn
