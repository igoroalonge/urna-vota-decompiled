// uenux2/src/app/comum/comparecimentomesario/estados/igestordadomesario.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :65
// (GetControlador). Func 10337 also inlines CRegistradorMesario::SaveCurrentInternal
// (cregistradormesario.cpp:85) and the constructor + GetInst of CMesarioRegistrado.
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include "api/pattern/cpolysingletonlist.h"
#include "api/util/cdatetime.h"
#include "comum/comparecimentomesario/ccontroladorreconhecimetomesario.h"
#include "comum/comparecimentomesario/cregistradormesario.h"

namespace comum {

namespace {
IControladorRegistraMesarios& GetControlador()                                     // srcloc :65
{
    return api::CPolySingletonList::instance<IControladorRegistraMesarios>();
}
} // namespace

// wasm func 10337 - vtable slot 2 of CGestorDadoMesarioInicial / Votacao / Final (the tools named it
// IGestorDadoMesario::GetControlador after the inlined srcloc :65).
// Builds the md::CComparecimentoMesario of the mesário being registered, persists it and moves on.
void IGestorDadoMesario::StartState()
{
    auto& registrador = CRegistradorMesario::GetInst();                           // func 815
    const auto& reconhecimento = CControladorReconhecimentoMesario::GetInst();    // func 1149
    const bool pertenceSecao = GetControlador().MesarioEhEleitorDaSecao();        // slot 13
    const md::CEleitorIdentidade identidade(GetControlador().GetTituloMesario(), ETipoIdentificador::TITULO);   // slot 17

    // NOTE: the recognition data are whatever CPedeDigitalMesario left in the controller singleton.
    // On the biometric path m_estado (2/3) and m_dedo (matched finger, or 0 via SalvaDigitalMesario)
    // are rewritten for every mesário, but m_idArquivo is only ever SET (func 5371), never cleared: a
    // mesário whose print matched (no image stored) or whose image could not be stored inherits the
    // id_arquivo of an earlier mesário (see the u22 doc, §11). Without biometrics (exterior / cfg
    // +665) the singleton keeps its initial values (estado 1, dedo 0, no id).
    if (reconhecimento.m_idArquivo.has_value()) {
        registrador.Update(md::CComparecimentoMesario(identidade, GetPeriodoPresente(), pertenceSecao,   // slot 9
                                                      reconhecimento.m_estado, reconhecimento.m_dedo,
                                                      api::CDateTime::Now(), *reconhecimento.m_idArquivo));   // funcs 479, 2730
    } else {
        registrador.Update(md::CComparecimentoMesario(identidade, GetPeriodoPresente(), pertenceSecao,
                                                      reconhecimento.m_estado, reconhecimento.m_dedo,
                                                      api::CDateTime::Now()));                                 // func 2731
    }
    // api::CDataMap<CComparecimentoMesarioPK, CComparecimentoMesario>::Update puts the row in the map and
    // positions the "current" iterator on it; then the inlined CRegistradorMesario::SaveCurrentInternal:
    //   if (map.empty()) throw CComparecimentoMesarioError(7754, "O contêiner estava vazio");   // cregistradormesario.cpp:85
    //   m_servico->Salva(current->second);          // CComparecimentoMesarioServico slot 3 -> DAO Inserir
    registrador.SaveCurrent();
    GetControlador().SincronizaBancoDados();                                      // slot 16 (VOTA: sign uenux.db, copy to MV)

    m_proximoEstado = &CMesarioRegistrado::GetInst();                             // ctor inlined here
}

} // namespace comum
