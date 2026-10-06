// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/clocal.h
//
// comum::md::CLocal = where this urna is (ModuloLocal::Local in the *-lo.dat file): country, UF,
// municipality and either a polling section or a contingency (reserve) urna identification.
// (Not to be confused with comum::CLocal in dados/clocal.cpp, the singleton wrapper, unit u04.)
#pragma once

#include <optional>
#include <string>

#include "municipiozona/cmunicipio.h"

namespace comum::md {

class CLocal {
public:
    const CSecaoEleitoral& GetSecao() const;                                   // func 943  (:59)
    const CIdentificacaoUrnaContingencia& GetContingencia() const;             // func 3724 (:67)
    void ValidaCriacao() const;                                                // func 5673 (:75..89)

private:
    int m_idPE;                                                    // +0  (processo eleitoral)
    std::string m_pais;                                            // +4
    std::string m_siglaUF;                                         // +16 (2 letters)
    std::string m_nomeUF;                                          // +28
    CMunicipio m_municipio;                                        // +40 (codigo at +40)
    // ... complemento do município (+60..+83) ?
    std::optional<CSecaoEleitoral> m_secao;                        // +84 (engaged +120; municipio at +92)
    std::optional<CIdentificacaoUrnaContingencia> m_contingencia;  // +124 (engaged +136; municipio at +128)
};

}  // namespace comum::md
