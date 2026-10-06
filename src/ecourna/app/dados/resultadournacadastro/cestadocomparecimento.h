// ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.h and
// capresentacaofotoeleitor.h  (paths inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// Attendance state of one voter of the section: did (s)he vote, how was (s)he enabled.
// = ModuloResultadoUrnaCadastro::EstadoComparecimento
#pragma once

#include <optional>

#include "ecourna/app/dados/cregistroidentificacaoeleitor.h"
#include "ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.h"

namespace ecourna::app::dados {

// = ApresentacaoFotoEleitor: was the voter's photo shown to the mesário.   (constructor = shared_f1081, ICF)
class CApresentacaoFotoEleitor {                        // 8 bytes
public:
    enum EEstado : int {       // ASN EstadoApresentacaoFotoEleitor - 1
        EleitorSemFoto = 0, Apresentada = 1, NaoApresentadaPorErro = 2,               // names inferred
    };
    enum EResultado : int {    // same numbers as ResultadoApresentacaoFotoEleitor (0 semErro ... 8)
        SemErro = 0, ErroFormato, ErroFormacaoImagem, ErroResolucao, ErroCompressao, ErroProfundidadeCores,
        ErroApresentacao, ErroDecifrandoBuffer, ErroDesconhecidoDeCriptografia,        // names inferred
    };
    CApresentacaoFotoEleitor(EEstado estado, EResultado resultado) : m_estado(estado), m_resultado(resultado) {}
    EEstado GetEstado() const { return m_estado; }
    EResultado GetResultado() const { return m_resultado; }
private:
    EEstado m_estado;          // +0
    EResultado m_resultado;    // +4
};

class CEstadoComparecimento {                                            // 96 bytes
public:
    enum ESituacaoComparecimento : int {   // mapped through tables @1135428 / @1135444, NOT value-1
        Faltou = 0,             // faltou (1)             (names inferred)
        SemCargoParaVotar = 1,  // semCargoParaVotar (4)
        NaoVotou = 2,           // naoVotou (2)
        Votou = 3,              // votou (3)
    };
    enum ESituacaoHabilitacaoAudio : int {  // ASN SituacaoHabilitacaoAudio - 1
        Automatica = 0, PeloMesario = 1, NaoHabilitado = 2,                             // names inferred
    };

    // Constructors in unit u40 (funcs 2658, 2193, 2192).
    CEstadoComparecimento(const CRegistroIdentificacaoEleitor& id, ESituacaoComparecimento situacao);
    CEstadoComparecimento(const CRegistroIdentificacaoEleitor& id, ESituacaoComparecimento situacao,
                          const CApresentacaoFotoEleitor& foto, ESituacaoHabilitacaoAudio audio);
    CEstadoComparecimento(const CRegistroIdentificacaoEleitor& id, ESituacaoComparecimento situacao,
                          const CApresentacaoFotoEleitor& foto, ESituacaoHabilitacaoAudio audio,
                          const CHabilitacaoBiometrica& biometria);

    const CRegistroIdentificacaoEleitor& GetIdentificacaoEleitor() const { return m_identificacao; }
    ESituacaoComparecimento GetSituacao() const { return m_situacao; }
    bool PossuiApresentacaoFoto() const { return m_apresentacaoFoto.has_value(); }
    bool PossuiSituacaoHabilitacaoAudio() const { return m_habilitacaoAudio.has_value(); }
    bool PossuiHabilitacaoBiometrica() const { return m_habilitacaoBiometrica.has_value(); }
    const CApresentacaoFotoEleitor& GetApresentacaoFoto() const;            // func 9021
    ESituacaoHabilitacaoAudio GetSituacaoHabilitacaoAudio() const;          // func 9020
    const CHabilitacaoBiometrica& GetHabilitacaoBiometrica() const;         // func 9019

private:
    CRegistroIdentificacaoEleitor m_identificacao;                      // +0
    ESituacaoComparecimento m_situacao;                                 // +20
    std::optional<CApresentacaoFotoEleitor> m_apresentacaoFoto;         // +24 (flag +32)
    std::optional<ESituacaoHabilitacaoAudio> m_habilitacaoAudio;        // +36 (flag +40)
    std::optional<CHabilitacaoBiometrica> m_habilitacaoBiometrica;      // +44 (flag +92)
};

} // namespace ecourna::app::dados
