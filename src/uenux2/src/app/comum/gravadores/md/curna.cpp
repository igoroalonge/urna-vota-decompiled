// uenux2/src/app/comum/gravadores/md/curna.cpp  (+ .h)
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// md::CUrna = ModuloTiposResultadosEcoUrna::Urna { tipoUrna, versaoVotacao, correspondenciaResultado,
//                                                  tipoArquivo, numeroSerieFV (4 bytes), motivoUtilizacaoSA OPTIONAL }
// Layout (from ValidaCriacao and the copy in CGravadorBU::GravaResultado, func 11629):
//   +0   EUrnaTipo m_tipoUrna            ('1' seção, '2' contingência, '3', '4'; see CConversorUrna)
//   +4   std::string m_versaoVotacao     ("10.23.0.1 - DESENVOLVIMENTO" in this build)
//   +16  CCorrespondenciaResultado m_correspondencia (88 bytes: município, zona, seção, CCarga @+24, tipo urna)
//   +104 ETipoArquivo m_tipoArquivo      ('1' votacaoUE ... '6' saEletronica)
//   +108 bool                            (copied by CEntidadeBU; ?)
//   +112 std::string m_numeroSerieFV     (8 hex chars: serial of the voting flash, "00000000" by default)
//   +124 std::optional<CTipoApuracaoSA> m_motivoUtilizacaoSA
// Constructors: comum_f2854 (without motivo) and comum_f2855 (with motivo), both call ValidaCriacao.
#include "comum/gravadores/md/curna.h"

#include "comum/gravadores/iresultado.h"

namespace comum::md {

// wasm func 5862 (srcloc curna.cpp:58, 61, 64, 69)
void CUrna::ValidaCriacao() const
{
    if (m_tipoUrna == EUrnaTipo('0'))
        throw CUeComumGravadoresError(8689, "Tipo de urna inválido");                              // :58
    if (m_tipoArquivo == ETipoArquivo('0'))
        throw CUeComumGravadoresError(8690, "Tipo de arquivo inválido");                           // :61
    if (m_numeroSerieFV.size() != 8)
        throw CUeComumGravadoresError(8691, "Serial da MV inválido [" + m_numeroSerieFV + "]");    // :64
    for (char c : m_numeroSerieFV)                                                                 // 8 x memchr
        if (std::string_view("0123456789ABCDEFabcdef").find(c) == std::string_view::npos)
            throw CUeComumGravadoresError(8692, "Serial da MV inválido [" + m_numeroSerieFV + "]"); // :69
}

}  // namespace comum::md
