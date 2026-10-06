// uenux2/src/app/comum/relatorios/ccalculacv.cpp  --  FRAGMENT written by unit u13
// (file known from the srclocs ccalculacv.cpp:58/65/74 (constructor, inlined in func 12110) and :109
// (IncluiString, func 942)). The whole class is reconstructed and explained in docs/bu/codigo-verificador.md;
// only wasm func 3700, which the unit builder filed under ecourna's imacalgorithm.cpp because it inlines
// IMacAlgorithm::Mac, is written here.
//
// comum::CCalculaCV (56 bytes, no vtable): +0 std::string m_identificacao, +12 char m_tipo ('F', or 'A' in demo
// mode), +16 std::string m_dados (running buffer), +28 std::vector<uebyte> m_chave (16 bytes, from
// /dsk/fi/estatico/chave/cv.ber.pri), +40 std::string m_ultimoCV, +52 unsigned m_contador.
#include "comum/relatorios/ccalculacv.h"

#include <format>
#include <string>
#include <vector>

#include "ecourna/api/security/imacalgorithm.hpp"   // CSiphashMac
#include "ecourna/api/util/cstringutils.hpp"

namespace comum {

namespace {
// Inlined into func 3700 with its OWN stack frame (sp-416 ... sp+416 inside the 400-byte frame of Calcula),
// i.e. a separate function in the source; it builds the CSiphashMac, the data vector and the tag, runs
// IMacAlgorithm::Mac (itself inlined, 32-byte frame) and returns the lower-case hex of the tag. Its name and
// whether it is a member are unknown.                                                                // ?name
std::string MacHexa(const std::string& dados, const std::vector<uebyte>& chave)
{
    ecourna::api::security::CSiphashMac mac;                           // stack object, vtable @1113604
    const std::vector<uebyte> bytes(dados.begin(), dados.end());
    std::vector<uebyte> tag;
    mac.Mac(bytes, tag, chave);     // inlined IMacAlgorithm::Mac: 1432 "Vetor de dados vazio." / 1433 "Chave vazia.",
                                    // then virtual DoMac = SipHash-4-6 (func 9498), 8-byte tag

    std::string hexa;
    for (std::size_t i = 0; i < tag.size(); ++i)                       // index loop: tag.size() re-read each turn
        hexa += std::format("{:02x}", tag[i]);                         // @8153, lower case, byte order
    return hexa;
}
} // namespace

// wasm func 3700 - name inferred ("Calcula"). Callers: CRelUtil::DSCodigoVerificador (func 11182),
// CDataSourcesRelatorio<CRdvVota, CEleitores>::TrailerProporcionalPartido (11260) and ::CodVerificador (11974),
// i.e. every "Código Verificador" printed on the BU. Returns the 10-digit code (the callers insert the dots).
std::string CCalculaCV::Calcula()
{
    m_dados += std::to_string(++m_contador);                           // api_f327 = to_string(unsigned)

    const std::string hexa = MacHexa(m_dados, m_chave);                // inlined helper (see above)
    const ueqword valor = ecourna::api::util::CStringUtils::HexToQWord(hexa);   // inlined, line 754 -> func 2202
                                                                        // (the tag read as a big-endian integer)
    Reinicia(std::to_string(valor));                                   // to_chars (func 2866) inlined; func 5615:
                                                                        // the chain restarts from the full value
    return std::format("{:010}", valor % 10000000000ULL);              // @8976
}

} // namespace comum
