// uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35: slot 4). Other members: cgeradorrelversaopacotedados.u02.cpp
// (MontaCorpo, func 11223) and comum/u26-foreign-fragments.cpp (Imprime 5592, MontaRodape 5596). The destructor
// func 3691 (vector<CCabecalhoPacote> at +16 via comum_f3863, then the builder's vector of shared_ptr) belongs to
// another unit.
#include "comum/relatorios/cgeradorrelversaopacotedados.h"

#include <format>
#include <string>

#include "api/util/cstringutils.h"                 // api::CStringUtils::PadLeft (api_f753)
#include "comum/appinfo/cappinfo.h"
#include "ecourna/api/security/base64.hpp"

namespace comum {

namespace {
// wasm func 3703 (named "ecourna::api::security::EncodeBase64" by the tools): the first 8 characters of the Base64
// of CEstadoGeral::GetHashVersoesPacotes() (+168). See src/ecourna/api/security/base64.cpp. name inferred
std::string ResumoHashVersoesPacotes(const md::estadoaplicacao::CEstadoGeral& estado);
} // namespace

// wasm func 11222 - vtable slot 4. Name inferred (u26 calls slot 4 "MontaDados"). Not observed executing.
//   (blank line)
//   "t02400ac000010001-el.jez  202609101622"      one centred line per package: nome, spaces, versão (38 columns)
//   ...
//   "Dados:                          Ab3dE9x/"   the 8-character version hash, right-aligned
//   (blank line)
// NOTE: the padding is std::string(38 - (nome + versão), ' '): a package whose name has more than 26 characters
// (versão is always 12) makes the size negative -> std::length_error ("basic_string") and the report is aborted.
void CGeradorRelVersaoPacoteDados::MontaDados()
{
    m_relatorio.AddNewLine(1);                                                        // comum_f198
    const auto& estado = GetEstado<md::estadoaplicacao::CEstadoGeral>(CAppInfo::GetInst());   // func 291

    for (const md::CCabecalhoPacote& pacote : m_pacotes) {
        const std::string espacos(38 - (pacote.GetNome().size() + pacote.GetVersao().size()), ' ');
        m_relatorio.AddText(std::format("{}{}{}", pacote.GetNome(), espacos, pacote.GetVersao()), 1, 2);
    }

    const std::string hash = ResumoHashVersoesPacotes(estado);
    const std::string rotulo = "Dados:";
    m_relatorio.AddText(rotulo + api::CStringUtils::PadLeft(hash, ' ', 38 - rotulo.size()), 1, 0);
    m_relatorio.AddNewLine(1);
}

} // namespace comum
