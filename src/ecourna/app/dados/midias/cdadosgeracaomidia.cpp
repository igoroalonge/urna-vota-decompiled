// ecourna-lib/ecourna/app/dados/midias/cdadosgeracaomidia.cpp   (path inferred: next to caplicativo.cpp,
//   cautenticacao.cpp and cinformacaomidia.cpp, all srcloc-attested in ecourna/app/dados/midias/)
// Reconstructed from vota_web_wasm.wasm (unit u11).
//
// "Dados de geração da mídia": who/what generated a medium (flash card / pen drive) for the urna.
//   DadosGeracaoMidia ::= SEQUENCE { serialMidia GeneralString, usuario GeneralString,
//                                    identificadorGeradorMidia IdentificadorGeradorMidia, data DataHoraJE }
//   IdentificadorGeradorMidia ::= SEQUENCE { nome, serialCertificadoTPM, serialInstalacao GeneralString }
//
// CDadosGeracaoMidia (72 bytes) is declared by unit u14 in cinformacaomidia.h (declarations kept together):
//   +0  CSerialMidia m_serialMidia                          (12 bytes; ctor func 9259, u14)
//   +12 std::string  m_usuario
//   +24 CIdentificadorGeradorMidia m_identificadorGerador   (nome, serialCertificadoTPM, serialInstalacao;
//                                                            copy ctor func 9872 "libcxx_f9872")
//   +64 boost::posix_time::ptime m_data                     (64-bit)
// The constructor below must be added to that declaration.
#include "ecourna/app/dados/midias/cinformacaomidia.h"

namespace ecourna::app::dados {

// wasm func 9034 (table slot 6736) - name inferred. Only caller: CConversorDadosGeracaoMidia::DoDeconverte
// (func 9202). Plain member-wise copies, no validation.
CDadosGeracaoMidia::CDadosGeracaoMidia(const CSerialMidia& serialMidia, const std::string& usuario,
                                       const CIdentificadorGeradorMidia& identificadorGerador,
                                       const boost::posix_time::ptime& data)
    : m_serialMidia(serialMidia)
    , m_usuario(usuario)
    , m_identificadorGerador(identificadorGerador)
    , m_data(data)
{
}

}  // namespace ecourna::app::dados
