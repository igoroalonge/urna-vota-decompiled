// uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.cpp   (path inferred, see the header)
//
// Reconstructed from vota_web_wasm.wasm (unit u33).
#include "comum/gravadores/cgravadorversoesarquivos.h"

#include "api/io/asn/cfileasn.h"                           // api::CFileASN (units u12/u18)
#include "comum/gravadores/asn/cconversorversoesarquivos.h" // comum::asn::CConversorVersoesArquivos (unit u21)
#include "ModuloVersaoArquivos.h"

namespace comum {

// wasm func 11582 - vtable slot 7 (srclocs iconversorasn.h:56, cfileasn.h:161, cfileasn.h:171)
//
// Called by IGravador::Grava / GravaMV (slots 4/5), which open "<trab>/<fase><pleito:05><uf><município:05>
// <zona:04><seção:04>-mr.ver" with mode "wb" and pass the api::CFile. One source line; everything below it
// is inlined (it is the same sequence as CFileASN::WriteDataToFile<TConversor>, func 6006, whose thunks the
// other writers call - this writer got its own inlined copy):
void CGravadorVersoesArquivos::GravaResultado(api::CFile& arquivo) const
{
    api::CFileASN::WriteDataToFile<asn::CConversorVersoesArquivos>(arquivo, m_versoes);

    // Expanded, in the order of the binary:
    //
    // (1) IConversorASN<EntidadeVersaoArquivos, md::CVersoesArquivos>::Converte (iconversorasn.h:56):
    //       asn::CConversorVersoesArquivos conversor;                 // vptr @1599296, on the stack
    //       ModuloVersaoArquivos::EntidadeVersaoArquivos entidade = conversor.DoConverte(m_versoes);  // slot 2 (10264)
    //       if (!entidade.isValid() || !entidade.isStrictlyValid()) {
    //           std::ostringstream os;
    //           ASN1::trace_invalid(os, "N20ModuloVersaoArquivos22EntidadeVersaoArquivosE", entidade);
    //           throw CUeComumAsnError(EUeComumAsnError{7653},
    //                                  std::format("Entidade deixada em estado inválido: {}", os.str()));
    //       }
    //
    // (2) CFileASN::WriteToFile(const CFile&, const T&):
    //       std::vector<char> conteudo;
    //       const std::string contexto = "WriteToFile de " + arquivo.GetName();     // CFile +4
    //
    // (3) CFileASN::CodeObjectFunction(conteudo, entidade, contexto):
    //       if (!entidade.isValid()) {                                              // only isValid here
    //           std::ostringstream os;
    //           ASN1::trace_invalid(os, "", entidade);
    //           throw api::CUeIoError(EUeIoError{5955},
    //                 std::format("Objeto com conteúdo inválido para {}: {}", contexto, os.str()));   // :161
    //       }
    //       ASN1::CoderEnv env;  env.setBER();
    //       if (!ASN1::encodeBER(env, entidade, conteudo))                          // CoderEnv::encodeBER (1532)
    //           throw api::CUeIoError(EUeIoError{5956}, "Arquivo não foi codificado para " + contexto);   // :171
    //
    // (4) arquivo.RawWrite(conteudo.data(), conteudo.size());                        // func 1886
    //
    // The file is BER: EntidadeVersaoArquivos ::= SEQUENCE { versaoTag, arquivos SEQUENCE OF
    // ArquivoAssinatura { nome = module name, versão } } in std::map order (sorted by module name).
    // It is later signed through SAVD with the other result files (CAssinador::AssinaArquivosResultado,
    // SAVD file id 70) and copied to the MR (mídia de resultado).
}

} // namespace comum
