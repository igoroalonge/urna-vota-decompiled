// uenux2/src/app/comum/relatorios/csigverifier.cpp
// Reconstructed from vota_web_wasm.wasm (unit u25).
// srcloc :32  virtual void comum::CSigVerifier::Verify() const
// srcloc iinterfacesavd.cpp:898  void comum::ValidarUE(IInterfaceSavd&, ESavdAplic, const std::string&,
//        const std::string&)   (4-argument overload of ValidarUE, inlined here; the 3-argument overload is
//        wasm 5890, iinterfacesavd.cpp:886)
// Not executed in the recorded votes (no report is printed). In the web build the SAVD is
// (anonymous)::CWasmSavd, which answers "OK" to every request (func 10949), so Verify() can never fail.

#include "comum/relatorios/csigverifier.h"

#include <filesystem>

#include "api/pattern/cpolysingleton.h"

namespace comum {

// wasm 11178 (complete) / 11176 (deleting): destroy the three strings.
CSigVerifier::~CSigVerifier() = default;

// wasm func 11179 (vtable slot 2).
void CSigVerifier::Verify() const
{
    auto& savd = api::CPolySingleton<IInterfaceSavd>::instance();                    // func 1822, :32
    ValidarUE(savd, m_aplicacao, (std::filesystem::path(m_diretorio) / m_assinatura).string(), m_arquivo);
}

// iinterfacesavd.cpp:898 — inlined into 11179 (shown here for reference; it belongs to iinterfacesavd.cpp).
// The request itself is wasm 5892 (tools: "api::CGenericTags::WalkTreeTLV"; it builds the TLV
// "sup{pkg{<pacote>} key{53} fil{<arquivo>}}", "Argumento inválido assinar verificar arquivo." if an argument
// is empty, and returns false on failure after storing the SAVD error code at savd +16).
//
//   void ValidarUE(IInterfaceSavd& savd, ESavdAplic aplicacao, const std::string& pacote,
//                  const std::string& arquivo)
//   {
//       if (!AssinarVerificarArquivo(savd, aplicacao, 53, pacote, arquivo))                   // wasm 5892 ?
//           throw CUeComumError(7324, std::format("Falha ao validar assinatura UE\npacote: ({})\n"
//                                                 "arquivo: ({})\nErro SAVD: ({})\n{}", pacote, arquivo,
//                                                 savd.GetCodigoErro() /*+16*/, savd.GetUltimoErro() /*+4*/));
//   }

} // namespace comum
