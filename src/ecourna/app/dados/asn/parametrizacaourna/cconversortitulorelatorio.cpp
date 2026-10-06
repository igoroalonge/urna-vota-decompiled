// ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   TituloRelatorio ::= SEQUENCE { alinhamento AlinhamentoTitulo {esquerdo(1), direito(2), centro(3)},
//                                  estilo EstiloTitulo {normal(1), expandido(2)}, texto GeneralString }
//   <-> CTituloRelatorio (enums 0-based)
// The error texts are file-level std::string constants (not literals) initialised by the global
// constructor (func 14478) at 1912256 and 1912268; the same group also holds 1912232 "Gênero do label
// inválido." (used by cconversorlabelparametrizado.cpp) and 1912244 "Forma de validar título inválido.",
// which nothing uses (dead constant, probably from a shared header).
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

namespace {
const std::string MSG_ESTILO_INVALIDO = "Estilo do título inválido.";              // @1912256 (name inferred)
const std::string MSG_ALINHAMENTO_INVALIDO = "Alinhamento do título inválido.";    // @1912268 (name inferred)
}

// wasm func 9147 (srcloc line 47)
ModuloParametrizacaoUrna::EstiloTitulo::NamedNumber
CConversorTituloRelatorio::ConverterEstiloTitulo(CTituloRelatorio::EEstiloTitulo estilo) const
{
    switch (estilo) {
    case CTituloRelatorio::Normal:    return ModuloParametrizacaoUrna::EstiloTitulo::normal;      // 1
    case CTituloRelatorio::Expandido: return ModuloParametrizacaoUrna::EstiloTitulo::expandido;   // 2
    }
    throw CAsnParametrizacaoUrnaError(2572, MSG_ESTILO_INVALIDO);   // line 47
}

// wasm func 9148 (srcloc line 80): thunk into the shared body func 6148 (count 3): 0-based -> 1-based.
ModuloParametrizacaoUrna::AlinhamentoTitulo::NamedNumber
CConversorTituloRelatorio::ConverterAlinhamentoTitulo(CTituloRelatorio::EAlinhamentoTitulo alinhamento) const
{
    if (static_cast<unsigned>(alinhamento) >= 3) {
        throw CAsnParametrizacaoUrnaError(2575, MSG_ALINHAMENTO_INVALIDO);   // line 80
    }
    return static_cast<ModuloParametrizacaoUrna::AlinhamentoTitulo::NamedNumber>(alinhamento + 1);
}

// inlined into func 9145 (srcloc lines 60, 64)
CTituloRelatorio::EEstiloTitulo CConversorTituloRelatorio::DesconverterEstiloTitulo(const TEntidade& titulo) const
{
    switch (titulo.get_estilo().asInt()) {
    case ModuloParametrizacaoUrna::EstiloTitulo::normal:    return CTituloRelatorio::Normal;
    case ModuloParametrizacaoUrna::EstiloTitulo::expandido: return CTituloRelatorio::Expandido;
    case -1:   // ? explicit "invalid" enumerator
        throw CAsnParametrizacaoUrnaError(2573, MSG_ESTILO_INVALIDO);   // line 60
    }
    throw CAsnParametrizacaoUrnaError(2574, MSG_ESTILO_INVALIDO);       // line 64
}

// inlined into func 9145 (srcloc lines 96, 100)
CTituloRelatorio::EAlinhamentoTitulo CConversorTituloRelatorio::DesconverterAlinhamentoTitulo(const TEntidade& titulo) const
{
    switch (titulo.get_alinhamento().asInt()) {
    case ModuloParametrizacaoUrna::AlinhamentoTitulo::esquerdo: return CTituloRelatorio::Esquerdo;
    case ModuloParametrizacaoUrna::AlinhamentoTitulo::direito:  return CTituloRelatorio::Direito;
    case ModuloParametrizacaoUrna::AlinhamentoTitulo::centro:   return CTituloRelatorio::Centro;
    case -1:   // ? explicit "invalid" enumerator
        throw CAsnParametrizacaoUrnaError(2576, MSG_ALINHAMENTO_INVALIDO);   // line 96
    }
    throw CAsnParametrizacaoUrnaError(2577, MSG_ALINHAMENTO_INVALIDO);       // line 100
}

// wasm func 9149 (vtable slot 2)
CConversorTituloRelatorio::TEntidade CConversorTituloRelatorio::DoConverte(const TDado& titulo) const
{
    ModuloParametrizacaoUrna::TituloRelatorio entidade;
    entidade.set_alinhamento(ConverterAlinhamentoTitulo(titulo.GetAlinhamento()));
    entidade.set_estilo(ConverterEstiloTitulo(titulo.GetEstilo()));
    entidade.set_texto(titulo.GetTexto());
    return entidade;
}

// wasm func 9145 (vtable slot 3; the tool named it after the inlined DesconverterEstiloTitulo)
CConversorTituloRelatorio::TDado CConversorTituloRelatorio::DoDeconverte(const TEntidade& titulo) const
{
    const auto alinhamento = DesconverterAlinhamentoTitulo(titulo);
    const auto estilo = DesconverterEstiloTitulo(titulo);
    return CTituloRelatorio(alinhamento, estilo, titulo.get_texto());
}

} // namespace ecourna::app::dados::asn
