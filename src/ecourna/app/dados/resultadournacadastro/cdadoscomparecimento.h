// ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.h,
// ccomparecimentomesario.h, cidentificacaojustificativa.h  (paths inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// = ModuloResultadoUrnaCadastro::DadosComparecimento: the attendance of the whole section, sent back
// to the voter registry (Cadastro Eleitoral) after the election: justifications, every voter's state,
// and the mesários registered at opening (abertura) and closing (encerramento).
#pragma once

#include <optional>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "ecourna/app/dados/cregistroidentificacaoeleitor.h"
#include "ecourna/app/dados/resultadournacadastro/cestadocomparecimento.h"

namespace ecourna::app::dados {

// = ComparecimentoMesario { identificacaoMesario, dataHoraColeta, estadoColetaDigital OPTIONAL }
class CComparecimentoMesario {                                          // 40 bytes
public:
    enum EEstadoColetaDigital : int {   // same numbers as the ASN enum; 0 = field absent
        NaoInformado = 0,                                       // (name inferred)
        NaoColetadaReconhecidaNoCadastro = 1,
        ColetadaMesarioNaoPossuiDigitalNoCadastro = 2,
        ColetadaDigitalNaoReconhecidaNoCadastro = 3,
        ColetadaMesarioNaoPertenceASecao = 4,
    };
    CComparecimentoMesario(const CRegistroIdentificacaoEleitor& identificacao,
                           const boost::posix_time::ptime& dataHoraColeta,
                           EEstadoColetaDigital estado);          // func 3486 (name inferred, no srcloc)

    const CRegistroIdentificacaoEleitor& GetIdentificacaoMesario() const { return m_identificacao; }
    const boost::posix_time::ptime& GetDataHoraColeta() const { return m_dataHoraColeta; }
    EEstadoColetaDigital GetEstadoColetaDigital() const { return m_estadoColetaDigital; }

private:
    CRegistroIdentificacaoEleitor m_identificacao;     // +0
    boost::posix_time::ptime m_dataHoraColeta;         // +24
    EEstadoColetaDigital m_estadoColetaDigital;        // +32
};
using TVectorComparecimentoMesario = std::vector<CComparecimentoMesario>;

// = IdentificacaoJustificativa { identificacaoEleitor, anoNascimentoEleitor INTEGER(0..9999) }  (unit u40/u11)
struct CIdentificacaoJustificativa {                   // 24 bytes
    CRegistroIdentificacaoEleitor m_identificacao;     // +0
    std::uint16_t m_anoNascimento;                     // +20
};

// = IdentificacaoSecaoEleitoral (município, zona, local, seção)  (unit u40/u11)
struct CIdentificacaoSecaoEleitoral { std::uint32_t m_valores[4]; };   // 16 bytes, ? field layout

class CDadosComparecimento {                                            // 72 bytes
public:
    // Implicit copy constructor = func 5846 (address-taken, slot 7026); implicit destructor = func 1162.
    const std::vector<CIdentificacaoJustificativa>& GetJustificativas() const { return m_justificativas; }
    const CIdentificacaoSecaoEleitoral& GetIdentificacaoSecao() const { return m_identificacaoSecao; }
    const std::vector<CEstadoComparecimento>& GetEleitores() const { return m_eleitores; }
    bool PossuiMesariosAbertura() const { return m_mesariosAbertura.has_value(); }
    bool PossuiMesariosEncerramento() const { return m_mesariosEncerramento.has_value(); }
    const TVectorComparecimentoMesario& GetMesariosAbertura() const;       // func 9024
    const TVectorComparecimentoMesario& GetMesariosEncerramento() const;   // func 9023

private:
    std::vector<CIdentificacaoJustificativa> m_justificativas;          // +0
    CIdentificacaoSecaoEleitoral m_identificacaoSecao;                  // +12
    std::vector<CEstadoComparecimento> m_eleitores;                     // +28 (element dtor func 1006)
    std::optional<TVectorComparecimentoMesario> m_mesariosAbertura;     // +40 (flag +52)
    std::optional<TVectorComparecimentoMesario> m_mesariosEncerramento; // +56 (flag +68)
};

} // namespace ecourna::app::dados
