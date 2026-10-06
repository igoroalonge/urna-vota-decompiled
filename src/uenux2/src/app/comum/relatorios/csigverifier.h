// uenux2/src/app/comum/relatorios/csigverifier.h  (path inferred from csigverifier.cpp srcloc)
// Reconstructed from vota_web_wasm.wasm (unit u25).
//
// comum::CSigVerifier : api::ISigVerifier — before a report file is sent to the printer, verifies that the
// file written in the work directory still matches its signature package (<rel>.vsu) through the SAVD
// daemon. Built on the stack by the printing states (func 1540 = constructor): CImprimindoBU ("bu.dat",
// "bu.vsu"), CImprimindoZeresima / CReimprimindo*, CImprimirBJust ("buj.dat"), CImprimindoBim ("bim.dat"),
// CImprimindoBEHB ("behb.dat").
// RTTI typeinfo @1576908 (si, base api::ISigVerifier @1576920), vtable @1576880:
//   [0] ~CSigVerifier  wasm 11178   [1] deleting dtor  wasm 11176   [2] Verify() const  wasm 11179 (srcloc :32)
// Layout (44 bytes): +0 vptr, +4 ESavdAplic m_aplicacao (= 1), +8 std::string m_diretorio,
//                    +20 std::string m_arquivo, +32 std::string m_assinatura.
#pragma once

#include <string>

#include "api/io/isigverifier.h"
#include "comum/iinterfacesavd.h"

namespace comum {

class CSigVerifier : public api::ISigVerifier {
public:
    // wasm func 1540 (tools: vota_f1540): the three strings are moved in.
    CSigVerifier(std::string diretorio, std::string arquivo, std::string assinatura)
        : m_aplicacao(ESavdAplic{1})
        , m_diretorio(std::move(diretorio))
        , m_arquivo(std::move(arquivo))
        , m_assinatura(std::move(assinatura))
    {
    }
    ~CSigVerifier() override;                        // wasm 11178 / 11176

    void Verify() const override;                    // wasm 11179

private:
    ESavdAplic m_aplicacao;                          // +4
    std::string m_diretorio;                         // +8
    std::string m_arquivo;                           // +20
    std::string m_assinatura;                        // +32
};

} // namespace comum
