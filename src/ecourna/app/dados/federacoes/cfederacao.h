// ecourna-lib/ecourna/app/dados/federacoes/cfederacao.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// A "federação partidária" (party federation) from the -fe.dat file:
//   ModuloFederacoes::Federacao { identificador INTEGER(100..999), sigla, nome, partidos SEQUENCE OF INTEGER(0..99) }
#pragma once

#include <string>

#include "ecourna/app/dados/tiposbasicos.h"

namespace ecourna::app::dados {

class CFederacao {                                                   // 40 bytes
public:
    CFederacao(TFederacaoID id, const std::string& sigla, const std::string& nome,
               const TVectorNumeroPartido& partidos);                 // func 9047
    CFederacao(const CFederacao&) = default;                          // func 9044 (implicit copy, used by containers)

    TFederacaoID GetID() const { return m_id; }
    const std::string& GetSigla() const { return m_sigla; }
    const std::string& GetNome() const { return m_nome; }
    const TVectorNumeroPartido& GetPartidos() const { return m_partidos; }

private:
    TFederacaoID m_id;                   // +0  (16-bit)
    std::string m_sigla;                 // +4
    std::string m_nome;                  // +16
    TVectorNumeroPartido m_partidos;     // +28 (16-bit party numbers)
};

} // namespace ecourna::app::dados
