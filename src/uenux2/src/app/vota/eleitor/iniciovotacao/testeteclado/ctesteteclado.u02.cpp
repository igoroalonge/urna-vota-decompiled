// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/*.cpp  --  FRAGMENT written by unit u02
// (ctesteteclado.cpp is owned by u26; the classes below have no known file, paths inferred:
//  ctestefalhou.cpp, cenviarmanutencao.cpp, cesperaretestar.cpp in the same directory).
// Keyboard test ("teste do teclado") done before the zerésima.

namespace vota::testeteclado {

// wasm func 11809 (vtable vota::testeteclado::CTesteFalhou slot 7)       // name inferred
// Screen "Deseja enviar a urna para manutenção?" with keys CONFIRMA = "Repetir teste",
// CORRIGE = "Enviar para manutenção".
void CTesteFalhou::ProcessInput()
{
    switch (m_form->Read()) {                              // inlined CInteractiveForm::Read (line 57)
    case api::EInputResult::CORRIGE:                       // 5
        // lazy @1835092, 36 bytes, CAppState(2) (wasm 1285), vtable CEnviarManutencao, form at +28:
        //   header (wasm 3054), "Deseja enviar a urna para manutenção?" (37 chars, {320,230}, FONTE_30),
        //   keys {'C', "Repetir teste"}, {'D', "Enviar para manutenção"}, control input,
        //   name "telaEnviarManutencaoTesteTeclado" (vota wasm 576)
        m_proximoEstado = &CEnviarManutencao::GetInst();
        break;
    case api::EInputResult::CONFIRMA:                      // 9
        m_proximoEstado = &CEsperaRetestar::GetInst();     // wasm 5936
        break;
    default:
        break;
    }
}

// wasm func 11812 (vtable vota::testeteclado::CEnviarManutencao slot 7)  // name inferred
void CEnviarManutencao::ProcessInput()
{
    switch (m_form->Read()) {
    case api::EInputResult::CORRIGE:                       // 5: send to maintenance -> final error screen
        // lazy @1835064, 36 bytes, CAppState(0), vtable CErroTesteTecladoFim, form at +28 (non-interactive,
        // comum wasm 886): status header; "Erro no teste do teclado" ({320,140}, FONTE_30);
        // "Por favor, solicite ajuda dos técnicos do TRE" ({320,220}, FONTE_TEXTO);
        // "para as devidas providências." ({320,260}); name "telaErroTesteTeclado"
        m_proximoEstado = &CErroTesteTecladoFim::GetInst();
        break;
    case api::EInputResult::CONFIRMA:                      // 9: repeat the test
        m_proximoEstado = &CEsperaRetestar::GetInst();     // wasm 5936
        break;
    default:
        break;
    }
}

// wasm func 5936                                                       // name inferred
// Lazy singleton @1835036 of CEsperaRetestar (44 bytes, CAppState(4)). Its screen shows
// "Por favor, espere {}s para a" (CDataTextFmt<std::function<std::string()>>, the function being
// table slot 1862, refreshed by a CTextFieldUpdate every 200 ms at {..,170}), then
// "realização de uma nova tentativa" ({..,220}) and "de execução do teste."; form name
// "telaEsperaRepetirTesteTeclado".
CEsperaRetestar& CEsperaRetestar::GetInst();

} // namespace vota::testeteclado
