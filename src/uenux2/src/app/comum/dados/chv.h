// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/chv.h
//
// CHV = "Horário de Verão" (daylight-saving time) of the urna's município. Singleton created at
// start-up (CreateInst, chv.cpp:32/47/54/68, inlined into 7787): in the 2nd round it reads the
// "complementos de municípios" file (<fase><pe>...-cm.dat, ModuloComplementosMunicipios); otherwise
// it takes the information from the CLocal/CInfoMunicipio data. Not polymorphic; 28 bytes.
// Only used by the printer self-test report (CRelatorioTesteImpressora, "teste de impressora").
#pragma once

#include <mutex>
#include <string>

#include "md/chorarioveraomunicipio.h"

namespace comum {

class CHV {
public:
    static CHV& GetInst();                                                // line 27 (wasm 2817)
    static void CreateInst(const md::estadoaplicacao::CEstadoGeral& eg,
                           const EFlashOrigem origem);                    // lines 32..68 (in 7787)

    const md::CHorarioVerao& GetHorarioVerao() const;                     // wasm 5745, name inferred
    void VerificaEntraEmHorarioVerao(const std::string& funcao) const;    // line 117 (inlined)

private:
    md::CHorarioVeraoMunicipio m_hv;   // +0: município (+0), optional<CHorarioVerao> (+4, flag +24)

    static std::mutex s_mutex;         // @1838848
    static CHV*       s_inst;          // @1838872
};

}  // namespace comum
