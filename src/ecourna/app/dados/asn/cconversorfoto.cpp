// ecourna-lib/ecourna/app/dados/asn/cconversorfoto.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/cconversorfoto.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   Foto ::= SEQUENCE { formato FormatoImagem {formatoJPEG(1), formatoBMP(2)}, imagem OCTET STRING }
//   <-> CFoto { +0 EFormatoImagem (same numbers), +4 std::vector<uebyte> imagem }  (class in unit u40)
// Used for candidate photos (comum::asn::CConversorFotoCandidato) and voter photos
// (CConversorBiometriaEleitor). DoDeconverte was observed executing during the recorded votes
// (the candidate photo shown on the confirmation screen).
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// inlined into func 9227 (srcloc line 30)
ModuloTiposEleitorais::FormatoImagem::NamedNumber CConversorFoto::ConverteFormatoImagem(CFoto::EFormatoImagem formato) const
{
    if (formato != CFoto::FormatoJPEG && formato != CFoto::FormatoBMP) {   // (formato - 1) >= 2 unsigned
        throw CAsnError(2254, "Código de formato da foto inválido.");    // line 30
    }
    return static_cast<ModuloTiposEleitorais::FormatoImagem::NamedNumber>(formato);
}

// inlined into func 9226 (srcloc line 44)
CFoto::EFormatoImagem CConversorFoto::DeconverteFormatoImagem(ModuloTiposEleitorais::FormatoImagem::NamedNumber formato) const
{
    if (formato != ModuloTiposEleitorais::FormatoImagem::formatoJPEG &&
        formato != ModuloTiposEleitorais::FormatoImagem::formatoBMP) {
        throw CAsnError(2255, "Código de formato da foto inválido.");    // line 44
    }
    return static_cast<CFoto::EFormatoImagem>(formato);
}

// wasm func 9227 (vtable slot 2; the tool named it after the inlined ConverteFormatoImagem)
CConversorFoto::TEntidade CConversorFoto::DoConverte(const TDado& foto) const
{
    ModuloTiposEleitorais::Foto entidade;
    entidade.set_formato(ConverteFormatoImagem(foto.GetFormato()));
    entidade.ref_imagem().assign(foto.GetImagem().begin(), foto.GetImagem().end());   // shared_f1927
    return entidade;
}

// wasm func 9226 (vtable slot 3; the tool named it after the inlined DeconverteFormatoImagem)
CConversorFoto::TDado CConversorFoto::DoDeconverte(const TEntidade& foto) const
{
    const auto formato = DeconverteFormatoImagem(foto.get_formato().asEnum());
    const std::vector<uebyte> imagem(foto.get_imagem().begin(), foto.get_imagem().end());
    return CFoto(formato, imagem);                                                    // shared_f5111
}

} // namespace ecourna::app::dados::asn
