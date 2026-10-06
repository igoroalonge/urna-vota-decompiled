// uenux2/src/app/comum/comparecimentomesario/ccontroladorreconhecimetomesario.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u22). See the .cpp for the layout.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "comum/comparecimentomesario/md/ccomparecimentomesario.h"

namespace comum {

using uebyte = std::uint8_t;

class CControladorReconhecimentoMesario
{
public:
    static CControladorReconhecimentoMesario& GetInst();                          // func 1149 (other unit)

    bool ComparaDigitais(const std::vector<uebyte>& digital1,
                         const std::vector<uebyte>& digital2);                    // func 5372 (:65, :76)
    void SalvarBiometriaMesarioRegistrado(const std::vector<uebyte>& digital,
                                          const std::string& titulo);            // lambda $_0 @1595836
    void SalvarBiometriaMesarioNaoRegistrado(const std::vector<uebyte>& digital); // lambda $_0 @1595908

    // plain data read/written by CPedeDigitalMesario and IGestorDadoMesario (names inferred)
    md::CDedo::TipoDedo m_dedo = md::CDedo::TipoDedo{0};                                         // +0
    int m_limiarScore = 20;                                                                       // +4
    md::CComparecimentoMesario::EstadoReconhecimentoBiometrico m_estado =
        md::CComparecimentoMesario::EstadoReconhecimentoBiometrico::NAO_COLETADA;                // +8
    std::optional<std::uint32_t> m_idArquivo;                                                     // +12
    std::size_t m_qtdArquivos = 0;                                                                // +20
};

} // namespace comum
