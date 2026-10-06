// uenux2/src/app/comum/md/cseguranca.cpp
// Reconstructed from vota_web_wasm.wasm (unit u24).
#include "comum/md/cseguranca.h"

#include "comum/md/cabrangencia.h"      // CUeComumMdError

namespace comum::md {

// wasm func 3823 (srclocs lines 24, 27, 30). Callers: CConversorSeguranca::DoDesconverte (11412),
// CGravadorBU (11629: BU envelope) and CControlaArmazenamentoDeImagens (2725: fingerprint envelope), the
// last two always with (0, 1, out.chave) from CCepescCipher::Encrypt.
CSeguranca::CSeguranca(uebyte idTipoArquivo, uebyte idCriptografia, const std::vector<uebyte>& chave)
    : m_idTipoArquivo(idTipoArquivo), m_idCriptografia(idCriptografia), m_chave(chave)
{
    if (idTipoArquivo >= 3)
        throw CUeComumMdError(EUeComumMdError{8925}, "Tipo do arquivo inválido.");       // line 24
    if (idCriptografia < 1 || idCriptografia > 3)          // compiled as ((id - 4) & 0xFF) <= 252
        throw CUeComumMdError(EUeComumMdError{8926}, "ID da criptografia inválido.");    // line 27
    if (chave.empty())
        throw CUeComumMdError(EUeComumMdError{8927}, "Chave vazia.");                    // line 30
}

} // namespace comum::md
