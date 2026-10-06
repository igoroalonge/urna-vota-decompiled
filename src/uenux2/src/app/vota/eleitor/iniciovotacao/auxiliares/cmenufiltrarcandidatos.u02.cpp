// uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/*.cpp  --  FRAGMENT written by unit u02
// (cmenufiltrarcandidatosporcargo/porpartido.cpp are owned by u09; the other classes have no known
// file). Candidate viewing menus reachable from "Mais informações" -> "Visualizar candidatos".

namespace vota {

// =============================================================================================
// cmenufiltrarcandidatospornumero.cpp (path inferred)

// wasm func 11891 (vtable vota::CMenuFiltrarCandidatosPorNumero slot 7) // name inferred
void CMenuFiltrarCandidatosPorNumero::ProcessInput()
{
    switch (m_form->Read()) {                                   // inlined, cinteractiveform.h:57
    case api::EInputResult::CONFIRMA: {                         // 9
        const int numero = ecourna::api::util::CStringUtils::ToInt32(m_form->Entrada(0).Texto());  // +24 of input 0
        const auto cargo = CVisualizarCandidatos::GetInst().GetCargo();          // wasm 1279 -> 5951
        if (comum::CCandidaturas::GetInst().Existe(cargo, numero)) {             // wasm 1273
            auto& visualizar = CVisualizarCandidatos::GetInst();
            visualizar.m_filtroPorNumero = true;                // byte +32
            visualizar.m_numero = numero;                       // +28
            m_proximoEstado = &visualizar;
            break;
        }
        for (auto* entrada : m_form->Entradas())
            entrada->Limpa();                                   // vtable slot 11
        m_form->SetFocus(0);                                    // wasm 1401
        // transient message box for 2 s:
        api::CWait espera(2000);
        api::CFormBuilder builder;
        builder.AddFill({0, 210}, {639, 310}, 30);
        builder.AddText("Candidato não encontrado!", {320, 242}, FONTE_MENSAGEM /*@520148*/, 2, 2, 30);
        api::CriaForm(builder, std::make_shared<api::CPreShowNothing<api::IScreen>>(), "")->Show();   // slot 3
        espera.Espera();                                        // wasm 5446
        break;                                                  // stays in this state
    }
    case api::EInputResult::CORRIGE:                            // 5
        m_proximoEstado = &CMenuVisualizarCandidatos::GetInst();   // wasm 2288 (name inferred)
        break;
    default:
        m_proximoEstado = this;
        break;
    }
}

// =============================================================================================
// cmenufiltrarcandidatosporpartido.cpp / cmenufiltrarcandidatosporcargo.cpp (owner u09)

// wasm func 5953                                                       // name inferred
// Called by both StartState()s: fills once (when the vector at +12/+16 is empty) the list of
// candidacies (72-byte records built by wasm 5952 from the CCandidatura) whose office is present in
// CCargos::GetMapaCargos() and is a candidate office (map node +104 = CCargo +84, the engaged flag of
// its optional<CDetalheCandidato>; the same flag gives "não é de candidato" in wasm 5747).
void CMenuFiltrarCandidatosBase::CarregaCandidaturas()
{
    if (!m_candidaturas.empty())
        return;
    const auto cargos = comum::CCargos::GetInst().GetMapaCargos();          // wasm 2834
    for (const auto& [chave, candidatura] : comum::CCandidaturas::GetInst().GetMapa()) {   // wasm 521
        const auto it = cargos.find(candidatura.GetCargo());               // byte +20 of the node value
        if (it != cargos.end() && it->second.TemDetalheCandidato() /*node +104*/)
            m_candidaturas.emplace_back(candidatura);                      // wasm 5952 / 5950 (relocation)
    }
}

// =============================================================================================
// citemvisualizarcandidatosvota.cpp (path inferred)

// wasm func 12244 (vtable vota::CItemVisualizarCandidatosVota slot 2, pure virtual in comum::CItemMenu)
// "Visualizar candidatos" is offered only when at least one candidacy belongs to a candidate office
// (not only consultas/referendum questions).
bool CItemVisualizarCandidatosVota::Disponivel() const                    // name inferred
{
    const auto cargos = comum::CCargos::GetInst().GetMapaCargos();
    for (const auto& [chave, candidatura] : comum::CCandidaturas::GetInst().GetMapa()) {
        const auto it = cargos.find(candidatura.GetCargo());
        if (it != cargos.end() && it->second.TemDetalheCandidato())
            return true;
    }
    return false;
}

} // namespace vota
