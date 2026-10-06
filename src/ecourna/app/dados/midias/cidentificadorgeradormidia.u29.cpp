// FRAGMENT of ecourna-lib/ecourna/app/dados/midias/cidentificadorgeradormidia.cpp (path inferred: the struct is
// declared next to CDadosGeracaoMidia, see src/ecourna/app/dados/midias/cinformacaomidia.h; the class name is
// attested by the srcloc IConversorASN<ModuloTiposEcoUrna::IdentificadorGeradorMidia,
// ecourna::app::dados::CIdentificadorGeradorMidia>) reconstructed by unit u29 from vota_web_wasm.wasm.
//
// Who generated a medium: ASN.1 IdentificadorGeradorMidia {nome, serialCertificadoTPM, serialInstalacao}.
// 36 bytes, three std::string. In the simulator: "simulador-votacao-ng" and a TPM serial of 64 '0'.
#include <string>

namespace ecourna::app::dados {

// wasm func 5109 (tools: ecourna_f5109; address-taken, table slot 6674). Member-wise constructor from three
// const references (each string copied). Callers: CConversorIdentificadorGeradorMidia::Deconverte (vf3,
// func 9224) and the fixture helper func 9862.
CIdentificadorGeradorMidia::CIdentificadorGeradorMidia(const std::string& nome,
                                                       const std::string& serialCertificadoTPM,
                                                       const std::string& serialInstalacao)
    : m_nome(nome), m_serialCertificadoTPM(serialCertificadoTPM), m_serialInstalacao(serialInstalacao)
{
}

// wasm func 9862 (a copy made through this constructor) is a helper of the web fixture: see
// src/uenux2/mock/app/comum/cappinfobuilder.u29.cpp.

}  // namespace ecourna::app::dados
