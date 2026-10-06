// uenux2/src/app/comum/dados/clocal.cpp  --  FRAGMENT written by unit u36 (file owned by u04; see also
// clocal.u02.cpp). Reconstructed from vota_web_wasm.wasm.
//
// Two CLocal accessors that the tools could not attach to a file (no std::source_location of their own:
// the only one they contain belongs to the inlined VerificaLido, line 345). Their bodies start with
// VerificaLido("<accessor name>"), the convention of every CLocal accessor, which gives their names.
//
// Layout reminder (clocal.h): CLocal = { std::unique_ptr<md::CLocal> m_local; }  (+0)
//   md::CLocal: +16 std::string sigla da UF; +84 std::optional<CSecaoEleitoral> (engaged flag +120)
//   md::CSecaoEleitoral: +24 std::vector<CIdentificacaoAgregada> (8-byte elements: +0 ?, +4 TSecaoID)
#include "comum/dados/clocal.h"

#include <string>
#include <vector>

#include "ecourna/api/util/cstringutils.h"   // CStringUtils::ToLower (wasm 1879)

namespace comum {

// wasm func 3752                                                                          // name inferred
// The sections aggregated to this one ("seções agregadas": sections with few voters that vote in this
// urna). Empty for a contingency urna. Callers: CGeradorBUQRCodeVota::PreencheCabecalho (11242, the "AGRE:"
// field of the BU QR code), CRelUtil::FormataSecoesAgregadas (5738) and api_f4160 (zerésima screen header).
std::vector<TSecaoID> CLocal::GetAgregadas() const
{
    VerificaLido("GetAgregadas");                               // wasm 782 (12-char string, heap-allocated)
    std::vector<TSecaoID> agregadas;
    if (m_local->TemSecao()) {                                  // optional<CSecaoEleitoral> engaged (+120)
        // md::CLocal::GetSecao() (wasm 943) is evaluated three times in the binary; the first two results
        // are unused (the calls survive only because GetSecao may throw).                          // ?
        for (const md::CIdentificacaoAgregada& agregada : m_local->GetSecao().GetAgregadas())
            agregadas.insert(agregadas.end(), 1, agregada.GetSecao());   // libc++ fill-insert, n = 1 (+4)
    }
    return agregadas;
}

// wasm func 3753                                                                          // name inferred
// The UF of the polling place in lower case ("ac", "sp"...), as used in every result file name
// ("t02410ac0000100010001-bu.dat"). GetUF() (wasm 1702, "GetUF") is inlined; the copy + ToLower is the
// merge-similar body wasm 1879. Callers: CGravadorUtil::DeterminaNomeArquivoSemLetra (3798) and
// vota::CGravaResultado::StartState (12098, twice). Unlike comum::(anon)::FormataUF (3773) it does not
// check that the UF has two letters.
std::string CLocal::GetUFMinuscula() const
{
    VerificaLido("GetUF");
    return ecourna::api::util::CStringUtils::ToLower(m_local->GetSiglaUF());   // md::CLocal +16
}

}  // namespace comum
