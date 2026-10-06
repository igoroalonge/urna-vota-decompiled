// uenux2/src/app/comum/dados/md/eleitor/celeitor.cpp  --  FRAGMENT written by unit u13 (owner u05)
#include "celeitor.h"

#include <string>

#include "ecourna/api/util/cstringutils.hpp"

namespace comum::md {

// wasm func 5665 (component app:api in the database) - name inferred.
// Year of birth of the voter: the first four characters of m_dataNascimento (+68, ASN.1 DataJE
// "YYYYMMDD"). This confirms the "+68 ?" dataNascimento member guessed by u05.
// Callers: vota::CVerificaDadoEleitor::vf7 (func 10443) and vota::CPedeAnoNascimentoSemBiometria::vf7
// (func 10489), which compare it with the year the poll worker types (read with std::stoul) when a voter
// without biometrics is enabled.
uedword CEleitor::GetAnoNascimento() const
{
    return ecourna::api::util::CStringUtils::ToDWord(m_dataNascimento.substr(0, 4));
}

} // namespace comum::md
