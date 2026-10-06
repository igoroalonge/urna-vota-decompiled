// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/correspondencia/ccarga.h
#pragma once

#include <string>

namespace comum::md {

class CCarga {                               // sizeof 76 (copy ctor = func 1557, not in this unit)
public:
    void ValidaCriacao() const;              // func 5670

private:
    int m_numeroInternoUrna;                 // +0
    std::string m_serialMC;                  // +4  8 hex chars (numeroSerieFC, 4 bytes)
    std::string m_dataHoraCarga;             // +16 ?
    std::string m_codigoCarga;               // +28 24 decimal digits
    std::string m_nomeGerador;               // +40 ? IdentificadorGeradorMidia.nome
    std::string m_serialCertificadoTPM;      // +52 ?
    std::string m_serialInstalacao;          // +64 ?
};

}  // namespace comum::md
