// uenux2/src/app/comum/gravadores/cassinador.cpp  (+ .h below as a comment)
// Reconstructed from vota_web_wasm.wasm (unit u23). AssinaArquivosResultado (cassinador.cpp:141..149) was
// inlined into vota::CGravaResultado::StartState and is reconstructed in unit u07 (cgravaresultado.cpp).
//
//   class CAssinador {                      // vtable @1553260; vota::CAssinadorVota (vtable @1532312) derives from it
//   public:
//       CAssinador(ESavdAplic aplicacao, ESavdPacote pacote, ...);   // comum_f1501 (other unit)
//       virtual ~CAssinador();                                      // slot 0 func 1007, slot 1 func 4694
//       void Assina(const ESavdArquivoUE arquivo) const;            // func 1277 (srcloc :114)
//       void AssinaArquivosResultado(const std::vector<ESavdArquivoUE>& arquivos) const;   // :141..149 (u07)
//   private:
//       ESavdAplic  m_aplicacao;     // +4   1 = VOTA
//       ESavdPacote m_pacote;        // +8   158 (1º turno) / 159 (2º turno) for the result files
//       std::string m_arquivo;       // +12  "...-vota.vsc"
//       std::string m_diretorio;     // +24
//       std::string m_local;         // +36  "<uf><mun:05><zona:04><secao:04>" (15 chars)
//   };
#include "comum/gravadores/cassinador.h"

#include "api/gui/capplicationcontextstack.h"
#include "api/pattern/cpolysingletonlist.h"
#include "comum/carquivossavd.h"
#include "comum/iinterfacesavd.h"

namespace comum {

// wasm func 1007 (slot 0; also vota::CAssinadorVota::~CAssinadorVota = func 7766, a 7-byte thunk to it)
CAssinador::~CAssinador() = default;          // destroys m_local, m_diretorio, m_arquivo

// wasm func 4694 (slot 1, shared with CAssinadorVota): deleting destructor = ~CAssinador() + operator delete.

// wasm func 1277 (srcloc cassinador.cpp:114 and iinterfacesavd.cpp:1004). The tools named it after the second
// srcloc (comum::AssinarUE, inlined). Observed executing: votaInit and every "Gravando o estado da urna"
// (comum_f491), CSincronizaVota::SincronizaRelatorios (1836), CSincronismoVotoEleitor (7174) ... call it to have
// SAVD sign a file of dinamico/ (vota.bin, rdv.dat, reports...). In the web build CWasmSavd answers OK and no
// .vsu is produced by this path (votaInit writes the fake "assinatura simulada para vota_web_wasm" files itself).
void CAssinador::Assina(const ESavdArquivoUE arquivo) const
{
    auto& savd = api::CPolySingletonList::instance<IInterfaceSavd>();                 // func 1822, srcloc :114
    const api::CApplicationContextGuard contexto(
        api::Actions::ReinicieOuFotografeQRCode /*11*/, "Erro na assinatura", "",
        "Ocorreu um erro durante a assinatura do arquivo: " + NomeArquivoSavd(arquivo));   // comum_f5905
    AssinarUE(savd, m_aplicacao, 66 /*0x42*/, static_cast<ueint32>(arquivo));          // iinterfacesavd.cpp:1004
}                                                                                      // ~guard: comum_f675

}  // namespace comum
