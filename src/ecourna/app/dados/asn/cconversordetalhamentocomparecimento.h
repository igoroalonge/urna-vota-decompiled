// ecourna-lib/ecourna/app/dados/asn/cconversordetalhamentocomparecimento.h   (path inferred: the name is
//   already included by u23's cconversorentidadebu.cpp; ModuloBoletimUrna has no other ecourna converter, so a
//   boletimurna/ subdirectory - by analogy with midias/, federacoes/ - is also possible)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// The one piece of the BOLETIM DE URNA (BU) that the ecourna library converts: the optional
//   [1] DetalhamentoComparecimento ::= SEQUENCE { qtdEleitoresCompareceramSemBiometria INTEGER (0..9999),
//                                                 qtdEleitoresHabilitadosPorBiometria  INTEGER (0..9999),
//                                                 qtdEleitoresHabilitadosPorBiografia  INTEGER (0..9999) }
// of ModuloBoletimUrna::EntidadeBoletimUrna. comum::asn::CConversorEntidadeBU::DoConverte (func 10273) calls
// Converte on it when the BU has the detail (urna biométrica), otherwise omits the field.
#pragma once

#include <cstdint>

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ModuloBoletimUrna.h"

namespace ecourna::app::dados {

// cdetalhamentocomparecimento.h (path inferred; RTTI name ecourna::app::dados::CDetalhamentoComparecimento).
// 6 bytes: three CBaseType<unsigned short, 0, 9999, 15> ("QtdEleitor"). Constructor = func 5094 (not in u40),
// called only by DoDeconverte 9221 and by vota::CGravaResultado (func 12098), which builds it from three
// CEleitores counters (comum_f2822 / 2821 / 1935, each through a QtdEleitor CBaseType) and hands it to the BU
// writer (comum::CGravadorBU::MontaEntidadeBU keeps it only for a biometric urna, u23 3.4).
class CDetalhamentoComparecimento {
public:
    CDetalhamentoComparecimento(std::uint16_t semBiometria, std::uint16_t porBiometria, std::uint16_t porBiografia);
    std::uint16_t GetQtdCompareceramSemBiometria() const { return m_semBiometria; }   // +0   names inferred
    std::uint16_t GetQtdHabilitadosPorBiometria() const { return m_porBiometria; }    // +2
    std::uint16_t GetQtdHabilitadosPorBiografia() const { return m_porBiografia; }    // +4
private:
    std::uint16_t m_semBiometria;   // +0  voters who attended without biometric enabling
    std::uint16_t m_porBiometria;   // +2  enabled by fingerprint
    std::uint16_t m_porBiografia;   // +4  enabled by "biografia" (the mesário checked the voter's data)
};

namespace asn {

// RTTI: CConversorDetalhamentoComparecimento
//         : IConversorASN<ModuloBoletimUrna::DetalhamentoComparecimento, CDetalhamentoComparecimento>
// vtable @1123424: [0] 174  [1] 144  [2] 9222 DoConverte  [3] 9221 DoDeconverte
class CConversorDetalhamentoComparecimento
    : public api::asn::IConversorASN<ModuloBoletimUrna::DetalhamentoComparecimento, CDetalhamentoComparecimento>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9222
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9221 (curated; not in u40)
};

} // namespace asn
} // namespace ecourna::app::dados
