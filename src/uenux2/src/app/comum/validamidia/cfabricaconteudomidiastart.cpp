// uenux2/src/app/comum/validamidia/cfabricaconteudomidiastart.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). Attested by the std::source_location record :62
// (static std::vector<std::string> comum::CFabricaConteudoMidiaStart::ConteudoIncializacao(
//  const EAplicativosDeUrna, const EUrnaTurno)). No out-of-line copy: the function is inlined into
// comum::impl::CValidaMidia::ValidaMidiaResultado (wasm func 11172).
//
// "Mídia start" = the result stick as prepared ("inicializada") by the electoral court before the election:
// this factory lists what such a stick must contain for each urna application.
//
// Header (cfabricaconteudomidiastart.h, path inferred) would declare:
//   namespace comum { class CFabricaConteudoMidiaStart { public:
//       static std::vector<std::string> ConteudoIncializacao(const EAplicativosDeUrna aplicativo,
//                                                            const EUrnaTurno turno); }; }
#include "comum/validamidia/cfabricaconteudomidiastart.h"

#include <format>
#include <string>
#include <vector>

#include "ecourna/api/exception/cbaseerror.hpp"

namespace comum {

/// CBaseError<comum::EUeComumValidaMidiaError, SErrorLimits{9200, 9250}> (typeinfo @1576976, vtable
/// @1576996). Built with the generic CError constructor (ecourna_f1143) + vptr store.
enum class EUeComumValidaMidiaError : int {};
using CUeComumValidaMidiaError =
    ecourna::api::exception::CBaseError<EUeComumValidaMidiaError, ecourna::api::exception::SErrorLimits{9200, 9250}>;

std::vector<std::string> CFabricaConteudoMidiaStart::ConteudoIncializacao(const EAplicativosDeUrna aplicativo,
                                                                          const EUrnaTurno turno)
{
    const int app = static_cast<int>(aplicativo);
    const bool turnoValido = turno == EUrnaTurno('1') || turno == EUrnaTurno('2');
    // bit mask 0x147800 = applications 11, 12, 13, 14, 18, 20: their stick does not depend on the turno
    const bool independeDoTurno = app <= 20 && ((1u << app) & 0x147800u);
    if (!turnoValido && !independeDoTurno)
        return {};

    std::vector<std::string> conteudo{"infomidia.dat", "infomidia.vsc"};
    switch (app) {
    case 7: case 8: case 9: case 10:            // (10 = VOTA in the web build)
        return {};
    case 11: case 12: case 13: case 14: case 18:
        return conteudo;
    case 15:                                    // treinamento
        conteudo.push_back(R"(#regex#t[0-9]{5}[a-z]{2}-pkgsa\.jez)");
        conteudo.push_back(R"(#regex#t[0-9]{5}[a-z]{2}-pkgsa\.vsc)");
        return conteudo;
    case 16:                                    // oficial
        conteudo.push_back(R"(#regex#o[0-9]{5}[a-z]{2}-pkgsa\.jez)");
        conteudo.push_back(R"(#regex#o[0-9]{5}[a-z]{2}-pkgsa\.vsc)");
        return conteudo;
    case 17:                                    // simulado
        conteudo.push_back(R"(#regex#s[0-9]{5}[a-z]{2}-pkgsa\.jez)");
        conteudo.push_back(R"(#regex#s[0-9]{5}[a-z]{2}-pkgsa\.vsc)");
        return conteudo;
    case 20:
        return {"##.pub", "##.id", "##ue", "###ue##.vpe"};       // '#' = any digit
    default:                                    // 19, and 1..6 when the turno is valid
        throw CUeComumValidaMidiaError(EUeComumValidaMidiaError(9200),
            std::format("Não há mídia de resultado de inialização para a aplicação '{}'.", aplicativo));   // :62 (sic)
    }
}

} // namespace comum
