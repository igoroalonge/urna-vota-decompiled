// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/cvalidadoridentidade.h
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "iregraidentidade.h"

namespace comum::md {

// Singleton (mutex @1839032, instance @1839028). The rule list is built at start-up (inlined in
// func 7787) from the identifier types enabled in CConfiguracaoEleicao:
//   1 -> CRegraTitulo, 2 -> CRegraCPF, 3 -> CRegraIdentidadeLivre, anything else -> CUeComumDadosError 7825.
class CValidadorIdentidade {
public:
    static CValidadorIdentidade& GetInst();   // cvalidadoridentidade.cpp:26
    explicit CValidadorIdentidade(std::vector<std::unique_ptr<IRegraIdentidade>> regras);

    void Valida(ETipoIdentificadorEleitor tipo, const std::string& identidade) const;   // :61

private:
    std::vector<std::unique_ptr<IRegraIdentidade>> m_regras;   // +0
};

}  // namespace comum::md
