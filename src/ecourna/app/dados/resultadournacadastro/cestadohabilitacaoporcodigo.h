// ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// "Habilitação por código": the voter's fingerprint was not recognised and the mesário enabled the
// voter with a release code; the mesário may have identified himself by fingerprint.
// = ModuloResultadoUrnaCadastro::EstadoHabilitacaoPorCodigo
#pragma once

#include <optional>

#include "ecourna/app/dados/cregistroidentificacaoeleitor.h"

namespace ecourna::app::dados {

class CEstadoHabilitacaoPorCodigo {                                     // 28 bytes
public:
    enum ESituacaoReconhecimentoMesario : int {     // ASN value - 1
        NaoReconhecido = 0,                          // naoReconhecido (1)
        ReconhecidoDigitalCadastroUrna = 1,          // reconhecidoDigitalCadastroUrna (2)      (name inferred)
        ReconhecidoDigitalRegistroMesarios = 2,      // reconhecidoDigitalRegistroMesarios (3)  (name inferred)
    };

    explicit CEstadoHabilitacaoPorCodigo(ESituacaoReconhecimentoMesario situacao);          // func 5090
    CEstadoHabilitacaoPorCodigo(ESituacaoReconhecimentoMesario situacao,
                                const CRegistroIdentificacaoEleitor& identificacaoMesario); // func 5088

    ESituacaoReconhecimentoMesario GetSituacao() const { return m_situacao; }
    bool PossuiIdentificacaoMesario() const { return m_identificacaoMesario.has_value(); }
    const CRegistroIdentificacaoEleitor& GetIdentificacaoMesario() const;                    // func 9017

private:
    ESituacaoReconhecimentoMesario m_situacao;                            // +0
    std::optional<CRegistroIdentificacaoEleitor> m_identificacaoMesario;  // +4 (flag +24)
};

} // namespace ecourna::app::dados
