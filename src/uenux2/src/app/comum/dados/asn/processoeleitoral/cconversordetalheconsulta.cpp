// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversordetalheconsulta.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35). The md constructors inlined into DoDesconverte are attested by
// srclocs: cdetalheconsulta.cpp:29/:32 and crespostaconsulta.cpp:29/:32.
#include "comum/dados/asn/processoeleitoral/cconversordetalheconsulta.h"

#include <string>
#include <vector>

#include "ecourna/api/util/cstringutils.hpp"   // Split (func 1880), Trim (func 1374)

// md::CRespostaConsulta / md::CDetalheConsulta constructors (crespostaconsulta.cpp:29/:32, cdetalheconsulta.cpp:29/:32),
// inlined here: src/uenux2/src/app/comum/dados/md/processoeleitoral/{crespostaconsulta,cdetalheconsulta}.u35.cpp

namespace comum::asn {

namespace {
// The question text may contain several lines separated by '|' (the screen breaks the line there); the converter
// normalises it: split at '|', trim every part (bytes <= ' ' at both ends), join again with '|'.   name inferred
std::string NormalizaLinhas(const std::string& texto)
{
    using ecourna::api::util::CStringUtils;
    std::vector<std::string> partes = CStringUtils::Split(texto, '|');                 // func 1880 (never empty)
    for (std::string& parte : partes)
        parte = CStringUtils::Trim(parte);                                               // func 1374
    std::string resultado = partes.front();
    for (auto it = partes.begin() + 1; it != partes.end(); ++it)
        resultado = resultado + "|" + *it;
    return resultado;
}
} // namespace

// wasm func 11377 - vtable slot 3. Not observed executing (the recorded scenarios have no consulta cargo).
md::CDetalheConsulta CConversorDetalheConsulta::DoDesconverte(const ModuloEleicao::DetalhePergunta& pergunta) const
{
    const std::string nome = pergunta.get_nome();
    const std::string texto = NormalizaLinhas(pergunta.get_pergunta());

    md::TVectorRespostaConsulta respostas;
    for (const auto& resposta : pergunta.get_respostas()) {
        const std::string fonetico = resposta.hasOptionalField(0) ? std::string(resposta.get_textoFonetico())
                                                                  : std::string("");
        respostas.emplace_back(static_cast<TCandidatoID>(resposta.get_numero()), resposta.get_resposta(), fonetico);
    }

    const std::string fonetico = pergunta.hasOptionalField(0) ? std::string(pergunta.get_textoFonetico())
                                                              : std::string("");
    return md::CDetalheConsulta(nome, texto, respostas, fonetico);
}

} // namespace comum::asn
