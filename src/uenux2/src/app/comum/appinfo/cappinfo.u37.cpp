// uenux2/src/app/comum/appinfo/cappinfo.cpp (path inferred, as for its twin SalvaEstado, wasm 491, unit u20)
// -- FRAGMENT written by unit u37.
//
// comum::GravaEstadoGeral(): persists eg.bin (EstadoGeralUrna, the urna-wide state: carga, local, fase,
// AJUSTE DE DATA/HORA...) on both flashes and signs it. Name used by the callers' reconstructions (u06, u10):
//   vota::CAjusteInicial::StartState (7160, after moving the clock to the election day in training/demo),
//   vota::CIniciodeCiclo::AjustaDataHora (7306), vota::CEncerramentoHorarioInvalido::StartState (10710).
// None of them runs in the web build (its first state is CAguardaMensagem).
// NOTE on the location: the body uses VOTA-only classes (the signer is vota::CAssinadorVota, wasm 1501; the
// shutdown flag is vota::CSincronizaVota's). The same holds for its twin SalvaEstado (491). The namespace
// comum / cappinfo.cpp follows the callers' reconstructions. vota/comum/csincronizavota.cpp is an equally
// good candidate for the original file.                                                                 ?
#include <filesystem>
#include <optional>
#include <string>

#include "api/gui/capplicationcontextstack.u15.h"
#include "api/util/csynchronizer.h"
#include "api/util/csystem.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/appinfo/servicos/iservicoestado.u29.h"   // CServicoEstadoGeral
#include "comum/carquivossavd.h"
#include "comum/gravadores/cassinador.h"
#include "vota/comum/cassinadorvota.h"
#include "vota/comum/csincronizavota.h"

namespace comum {

// wasm func 3333 (tools: vota_f3333)                                                    name inferred
void GravaEstadoGeral()
{
    // csincronizavota.cpp:47 inlined: flag @1832936 -> throw api::CUeDesligandoError (object built by 1685)
    vota::CSincronizaVota::VerificaUrnaDesligando();

    // DEAD VALUE (as compiled): the detail text of the current application context, or "Erro de
    // sincronização" when the context stack is empty, is computed exactly like in SalvaEstado (func 491)...
    std::optional<api::CApplicationContext> contextoAtual;
    auto& pilha = api::CApplicationContextStack::GetInst();                          // vector @1839212
    if (!pilha.Empty())
        contextoAtual = pilha.Top();                                                  // copy ctor wasm 1841
    const std::string detalhe = contextoAtual ? contextoAtual->m_detalhe : "Erro de sincronização";
    (void)detalhe;    // ...but, unlike SalvaEstado, never passed to the guards below (they get "").

    const auto& arquivos = CArquivosSavd::GetInst();                                  // wasm 1164
    auto& app = CAppInfo::GetInst();                                                  // wasm 185

    // 1) internal flash (MI, /dsk/fi): write dinamico/eg.bin and sign it into dinamico/eg.vsu
    {
        api::CApplicationContextGuard contexto(api::Actions::ReinicieOuSubstituaUrna /*2*/, "",
            "Gravando o estado geral da urna na MI",
            "Ocorreu um erro durante a sincronização do estado geral da urna na MI.");   // wasm 676
        vota::CAssinadorVota assinador(ESavdPacote{120});   // wasm 1501 (stores CAssinadorVota's vptr): eg.vsu (MI)
        if (app.TemGeral())                                                            // optional flag +180
            CServicoEstadoGeral(EFlashOrigem::INTERNA).Salva(app.GetGeral());          // 1941 + 3592
        assinador.Assina(ESavdArquivoUE{25});                                          // wasm 1277: "eg.bin"
        api::CSynchronizer::GetInst().Sync();                                          // wasm 600 + 620
    }                                                          // ~CAssinador 1007 (called directly), ~guard 675

    // 2) external flash (MV, /dsk/fe): write eg.bin again and copy the MI signature package
    {
        api::CApplicationContextGuard contexto(api::Actions::ReinicieOuSubstituaMidiaVotacao /*4*/, "",
            "Gravando o estado geral da urna na MV",
            "Ocorreu um erro durante a sincronização do estado geral da urna na MV.");
        if (app.TemGeral())
            CServicoEstadoGeral(EFlashOrigem::EXTERNA).Salva(app.GetGeral());
        api::CSystem::CopyFile(arquivos[ESavdPacote{120}],                             // /dsk/fi/dinamico/eg.vsu
                               arquivos[ESavdPacote{121}],                             // /dsk/fe/dinamico/eg.vsu
                               false);                                                 // wasm 378
        api::CSynchronizer::GetInst().Sync();
    }
}

} // namespace comum
