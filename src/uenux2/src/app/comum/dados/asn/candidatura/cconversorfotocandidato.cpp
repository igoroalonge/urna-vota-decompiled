// uenux2/src/app/comum/dados/asn/candidatura/cconversorfotocandidato.cpp   (path inferred: sibling of
//   cvisitantefoto.cpp, which indexes the same EntidadeFotosCandidatos file)
// Reconstructed from vota_web_wasm.wasm (unit u11: attached here because it calls the ecourna
// IConversorASN<Foto, CFoto>::Deconverte instantiation).
//
// Candidate photos (<..>-fo.dat): EntidadeFotosCandidatos { cabecalho, abrangencia, fotos SEQUENCE OF FotoCandidato }
//   FotoCandidato ::= SEQUENCE { codigoCandidato GeneralString (SIZE(0..18)), foto Foto }
//   Foto          ::= SEQUENCE { formato FormatoImagem, imagem OCTET STRING }
//
// RTTI: comum::asn::CConversorFotoCandidato
//         : comum::asn::IConversorASN<ModuloFotosCandidatos::FotoCandidato, comum::md::CFotoCandidato>
// vtable @1562588: [0] 174  [1] 144
//                  [2] 11439 base-class DoConverte ("Método DoConverte() não implementado para {}", iconversorasn.h:89)
//                  [3] 11440 DoDesconverte
//
// comum::md::CFotoCandidato: +0 std::string codigo, +12 ecourna CFoto foto { +12 formato, +16 vector<uebyte> imagem }
//
// Build note: this function has no invoke_* at all (no exception landing pads). If the Foto validation
// (5708 -> 6032) throws, the local copy of `codigo` is not destroyed (leak; the CFoto does not exist yet).
// If one of the later allocations throws (result string copy, imagem vector), `codigo` and the CFoto leak.
#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/cfotocandidato.h"
#include "ecourna/app/dados/asn/cconversorfoto.hpp"   // ecourna CConversorFoto (9226/9227, unit u14)
#include "ModuloFotosCandidatos.h"

namespace comum::asn {

class CConversorFotoCandidato : public IConversorASN<ModuloFotosCandidatos::FotoCandidato, md::CFotoCandidato>
{
protected:
    // DoConverte not overridden: the urna never writes photo files (slot 2 = func 11439, base class)
    TDado DoDesconverte(const TEntidade& entidade) const override;   // wasm func 11440
};

// wasm func 11440 (vtable slot 3; was "comum::asn::CConversorFotoCandidato::vf3"). Observed at run time in
// both recorded votes (a candidate photo is decoded from the -fo.dat file by offset, see CVisitanteFoto).
md::CFotoCandidato CConversorFotoCandidato::DoDesconverte(const TEntidade& entidade) const
{
    const std::string codigo = entidade.get_codigoCandidato();                     // field 0, copied

    const ecourna::app::dados::asn::CConversorFoto conversorFoto;                  // vptr @1122824
    // func 5708 -> 6032 (no-landing-pad copy): validates the Foto, then CConversorFoto::DoDeconverte (9226)
    const ecourna::app::dados::CFoto foto = conversorFoto.Deconverte(entidade.get_foto());

    return md::CFotoCandidato{codigo, foto};   // ? aggregate: string copy + CFoto copy (formato, imagem vector)
}

}  // namespace comum::asn
