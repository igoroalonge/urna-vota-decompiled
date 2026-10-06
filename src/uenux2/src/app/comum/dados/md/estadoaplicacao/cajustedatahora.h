// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/estadoaplicacao/cajustedatahora.h
#pragma once

#include <cstdint>

namespace comum::md::estadoaplicacao {

using ueint32 = std::uint32_t;
enum class ETipoAjusteDataHora : int { /* 0, 1 ? */ eAlterarDataSistema = 2 };

class CAjusteDataHora {
public:
    void AdicionaDeltaT(const ueint32 deltaT);   // func 3702
private:
    ETipoAjusteDataHora m_tipo;   // +0
    ueint32 m_valor;              // +4
};

}  // namespace comum::md::estadoaplicacao
