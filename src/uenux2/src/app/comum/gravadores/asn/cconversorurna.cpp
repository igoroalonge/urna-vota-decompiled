// uenux2/src/app/comum/gravadores/asn/cconversorurna.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// comum::asn::CConversorUrna : IConversorASN<ModuloTiposResultadosEcoUrna::Urna, md::CUrna> (vtable @1596460).
// [2] DoConverte = func 10287 (tools: ConverteTipoUrna, inlined), [3] DoDesconverte = func 10284 (tools:
// DesconverteTipoUrna, inlined).
#include "comum/gravadores/asn/cconversorurna.h"

#include <format>

#include "comum/gravadores/asn/cconversorcorrespresultado.h"
#include "comum/gravadores/asn/cconversortipoapuracaosa.h"
#include "comum/gravadores/iresultado.h"

namespace comum::asn {

std::vector<char> ConverteSerialFlash(const std::string& serial);           // util.cpp (func 3605)
std::string DesconverteSerialFlash(const std::vector<char>& bytes);         // util.cpp (func 3604)

// srcloc :77. EUrnaTipo '1'..'4' -> TipoUrna {secao 1, contingencia 3, contingenciaSecao 4,
// contingenciaEncerrandoSecao 6}  (table @544192)
ModuloTiposResultadosEcoUrna::TipoUrna::NamedNumber CConversorUrna::ConverteTipoUrna(EUrnaTipo tipo)
{
    static constexpr int tabela[4] = {1, 3, 4, 6};
    const int indice = static_cast<int>(tipo) - '1';
    if (indice < 0 || indice >= 4)
        throw CUeComumGravadoresError(8627, std::format("Tipo de urna inválido: {}", static_cast<int>(tipo)));   // :77
    return ModuloTiposResultadosEcoUrna::TipoUrna::NamedNumber(tabela[indice]);
}

// srcloc :95. TipoUrna -> EUrnaTipo (table @544208: 1->'1', 3->'2', 4->'3', 6->'4', others invalid)
EUrnaTipo CConversorUrna::DesconverteTipoUrna(ModuloTiposResultadosEcoUrna::TipoUrna::NamedNumber tipo)
{
    static constexpr int tabela[6] = {'1', 0, '2', '3', 0, '4'};
    const int indice = static_cast<int>(tipo) - 1;
    if (indice < 0 || indice >= 6 || tabela[indice] == 0)
        throw CUeComumGravadoresError(8628, std::format("Tipo de urna inválido: {}", static_cast<int>(tipo)));   // :95
    return EUrnaTipo(tabela[indice]);
}

// srcloc :117 / :139. ETipoArquivo '1'..'6' <-> TipoArquivo 1..6 (votacaoUE, votacaoRED, saMistaMRParcialCedula,
// saMistaBUImpressoCedula, saManual, saEletronica)
ModuloTiposResultadosEcoUrna::TipoArquivo::NamedNumber CConversorUrna::ConverteTipoArquivo(ETipoArquivo tipo)
{
    if (static_cast<int>(tipo) - '1' >= 6 || static_cast<int>(tipo) < '1')
        throw CUeComumGravadoresError(8629, std::format("Tipo de arquivo inválido: {}", static_cast<int>(tipo)));   // :117
    return ModuloTiposResultadosEcoUrna::TipoArquivo::NamedNumber(static_cast<int>(tipo) - '0');
}
ETipoArquivo CConversorUrna::DesconverteTipoArquivo(ModuloTiposResultadosEcoUrna::TipoArquivo::NamedNumber tipo)
{
    if (static_cast<int>(tipo) - 1 >= 6)
        throw CUeComumGravadoresError(8630, std::format("Tipo de arquivo inválido: {}", static_cast<int>(tipo)));   // :139
    return ETipoArquivo(static_cast<int>(tipo) + '0');
}

// wasm func 10287 (vtable slot 2)
CConversorUrna::TEntidade CConversorUrna::DoConverte(const TDado& urna) const
{
    TEntidade entidade;
    entidade.set_tipoUrna(ConverteTipoUrna(urna.GetTipoUrna()));
    entidade.set_versaoVotacao(urna.GetVersaoVotacao());
    entidade.set_correspondenciaResultado(CConversorCorrespResultado().Converte(urna.GetCorrespondencia()));   // iconversorasn.h:56
    entidade.set_tipoArquivo(ConverteTipoArquivo(urna.GetTipoArquivo()));
    entidade.set_numeroSerieFV(ConverteSerialFlash(urna.GetNumeroSerieFV()));        // 4 bytes (Constrained_OCTET_STRING<4,4>)
    if (urna.PossuiMotivoUtilizacaoSA())
        entidade.set_motivoUtilizacaoSA(CConversorTipoApuracaoSA().Converte(urna.GetMotivoUtilizacaoSA()));
    else
        entidade.omit_motivoUtilizacaoSA();
    return entidade;
}

// wasm func 10284 (vtable slot 3)
md::CUrna CConversorUrna::DoDesconverte(const TEntidade& entidade) const
{
    const EUrnaTipo tipoUrna = DesconverteTipoUrna(entidade.get_tipoUrna());
    const std::string versao = entidade.get_versaoVotacao();
    const auto correspondencia = CConversorCorrespResultado().Desconverte(entidade.get_correspondenciaResultado());
    const ETipoArquivo tipoArquivo = DesconverteTipoArquivo(entidade.get_tipoArquivo());
    const std::string serial = DesconverteSerialFlash(entidade.get_numeroSerieFV());
    if (entidade.hasOptionalField(0))
        return md::CUrna(tipoUrna, versao, correspondencia, tipoArquivo, serial,
                         CConversorTipoApuracaoSA().Desconverte(entidade.get_motivoUtilizacaoSA()));   // comum_f2855
    return md::CUrna(tipoUrna, versao, correspondencia, tipoArquivo, serial);                        // comum_f2854
}

}  // namespace comum::asn
