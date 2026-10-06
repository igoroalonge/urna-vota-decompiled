// ecourna-lib/ecourna/app/dados/midias/cinformacaomidia.h, caplicativo.h, cautenticacao.h,
// cdadosgeracaomidia.h  (paths inferred; declarations kept together)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// Contents of a medium's "infomidia.dat" (ModuloInformacaoMidia::InformacaoMidia): what kind of
// medium it is, for which election, who generated it and, for a result medium (MR), which
// applications it may start and with which password.
#pragma once

#include <optional>
#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "ecourna/app/dados/cserialmidia.h"
#include "ecourna/app/dados/tiposbasicos.h"

namespace ecourna::app::dados {

// ---- cautenticacao.h ----------------------------------------------------------------------
// = ModuloInformacaoMidia::Autenticacao (password protection of an application on the medium)
class CAutenticacao {                                                   // 56 bytes
public:
    const boost::posix_time::ptime& GetDataHoraInicial() const;        // func 9036
    const boost::posix_time::ptime& GetDataHoraFinal() const;          // func 9035
    int GetTamanhoSenha() const { return m_tamanhoSenha; }
    int GetNumeroTentativas() const { return m_numeroTentativas; }
    const std::vector<uebyte>& GetHashSenha() const { return m_hashSenha; }

private:
    std::optional<boost::posix_time::ptime> m_dataHoraInicial;  // +0  (flag +8)   [1] dataInicial
    std::optional<boost::posix_time::ptime> m_dataHoraFinal;    // +16 (flag +24)  [2] dataFinal
    int m_tamanhoSenha;                                         // +32 tamanhoSenha
    int m_numeroTentativas;                                     // +36 numeroTentativas
    std::vector<uebyte> m_hashSenha;                            // +40 hashSenha
};

// ---- caplicativo.h ------------------------------------------------------------------------
class CAplicativo {                                                     // 72 bytes
public:
    enum ETipoAplicativo : int {    // same numbers as ModuloInformacaoMidia::TipoAplicativo
        Vota = 1, SA = 2, RED = 3, VPP = 4, STE = 5, ADH = 6, ATUE = 7,   // names inferred
    };
    explicit CAplicativo(ETipoAplicativo tipo) : m_tipo(tipo) {}      // inlined into func 9196
    CAplicativo(ETipoAplicativo tipo, const CAutenticacao& autenticacao);   // func 9043 (name inferred)

    ETipoAplicativo GetTipo() const { return m_tipo; }
    bool PossuiAutenticacao() const { return m_autenticacao.has_value(); }
    const CAutenticacao& GetAutenticacao() const;                      // func 9042

private:
    ETipoAplicativo m_tipo;                         // +0
    std::optional<CAutenticacao> m_autenticacao;    // +8 (flag +64)
};
using TVectorAplicativo = std::vector<CAplicativo>;

// ---- cdadosgeracaomidia.h / cidentificadorgeradormidia.h ----------------------------------
struct CIdentificadorGeradorMidia {       // 36 bytes; copy ctor = func 9872 (rt:libcxx, unit u40)
    std::string m_nome;                   // +0  "simulador-votacao-ng" in the simulator
    std::string m_serialCertificadoTPM;   // +12 64 '0' characters in the simulator
    std::string m_serialInstalacao;       // +24
};

class CDadosGeracaoMidia {                // 72 bytes. Implicit copy = func 9032, implicit dtor = func 3490
public:
    const CSerialMidia& GetSerialMidia() const { return m_serialMidia; }
    const std::string& GetUsuario() const { return m_usuario; }
    const CIdentificadorGeradorMidia& GetIdentificadorGeradorMidia() const { return m_identificadorGerador; }
    const boost::posix_time::ptime& GetData() const { return m_data; }

private:
    CSerialMidia m_serialMidia;                       // +0
    std::string m_usuario;                            // +12
    CIdentificadorGeradorMidia m_identificadorGerador;// +24
    boost::posix_time::ptime m_data;                  // +64
};

// ---- cinformacaomidia.h -------------------------------------------------------------------
class CInformacaoMidia {                                                // 116 bytes
public:
    enum ETipoMidia : int {
        MR = 0,   // memória/mídia de resultado (result medium, the pen drive taken to the Junta)
        FC = 1,   // flash de carga
        FV = 2,   // flash de votação
    };

    // Any medium except MR (func 9031).
    CInformacaoMidia(ETipoMidia tipo, TFaseID fase, TProcessoEleitoralID idPE, const std::string& uf,
                     TTurno turno, const CDadosGeracaoMidia& dadosGeracao);
    // Result medium: the type is forced to MR (func 9033, name inferred, no srcloc).
    CInformacaoMidia(TFaseID fase, TProcessoEleitoralID idPE, const std::string& uf, TTurno turno,
                     const CDadosGeracaoMidia& dadosGeracao, const TVectorAplicativo& aplicativos);

    ETipoMidia GetTipoMidia() const { return m_tipoMidia; }
    TFaseID GetFase() const { return m_fase; }
    TProcessoEleitoralID GetIdPE() const { return m_idPE; }
    const std::string& GetUF() const { return m_uf; }
    TTurno GetTurno() const { return m_turno; }
    const CDadosGeracaoMidia& GetDadosGeracaoMidia() const { return m_dadosGeracao; }
    const TVectorAplicativo& GetAplicativos() const;                   // func 9030

private:
    ETipoMidia m_tipoMidia;                // +0
    TFaseID m_fase;                        // +4
    TProcessoEleitoralID m_idPE;           // +8
    std::string m_uf;                      // +12
    TTurno m_turno;                        // +24 (16-bit)
    CDadosGeracaoMidia m_dadosGeracao;     // +32
    TVectorAplicativo m_aplicativos;       // +104
};

} // namespace ecourna::app::dados
