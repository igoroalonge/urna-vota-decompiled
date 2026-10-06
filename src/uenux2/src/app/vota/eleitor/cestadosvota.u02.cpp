// FRAGMENTS written by unit u02 for VOTA voter-side states (classes without a known source file:
// paths inferred from the TSE naming convention, class CFooBar -> cfoobar.cpp). They were pulled
// into u02 by call-graph proximity to wasm func 7787.
//
// comum::CAppState (vtable @1551248): +4 m_proximoEstado (the state the machine switches to),
// slots: 0/1 dtor, 2 StartState(), 3 ?, 6 ProcessMessage(int) (name inferred), 7 ProcessInput(),
// 8 ProcessTick(uebyte). State objects are lazy singletons; the shared body of the simple ones is
// wasm 764 (`if (!*p) *p = new CAppState-derived{vtable, arg}`), shared_f224 = CAppState(arg) ctor.
// api::EInputResult values seen: 5 = CORRIGE, 9 = CONFIRMA (names inferred from the key labels).

namespace vota {

// =============================================================================================
// uenux2/src/app/vota/eleitor/caguardamensagem.cpp (path inferred)
// "Aguarda mensagem": the voter terminal waits for a command from the operator terminal (mesário).

// wasm func 7481 (vtable vota::CAguardaMensagem slot 6)                  // name inferred
void CAguardaMensagem::ProcessMessage(int mensagem)
{
    switch (mensagem) {
    case 1:     // voter enabled by the operator
        CLogVota::GetInst().Loga("Eleitor foi habilitado");                           // wasm 233
        m_proximoEstado = &CEleitorVotando::GetInst();                                 // wasm 3229 (@1833020)
        break;
    case 7:     // operator started the closing of the vote ("encerramento")
        comum::CAppInfo::GetInst().GetVota(EUrnaTurno::ATUAL).SetEstadoVota(EEstadoVota::GERARBU);  // ';'
        comum::SalvaEstadoVota();                     // wasm 491: "Gravando o estado da urna", signs + syncs vota.bin
        CLogVota::GetInst().Loga("Inicio do Encerramento");
        m_proximoEstado = &CGeraBU::GetInst();                                          // wasm 6129 (@1833496)
        break;
    case 11:
        m_proximoEstado = &CIniciodeCiclo::GetInst();                                   // wasm 4433
        break;
    case 12:    // urna inspection requested
        m_proximoEstado = &CInspecionaUrna::GetInst();  // lazy @1837704, 20 bytes, CAppState(2), form at +12:
        //   CTelasVota::CriaTelaTextoConfirmaCorrige("Por favor, inspecione cabina e urna.",
        //                                           std::string("Continuar"), std::nullopt)
        break;
    case 14:
        m_proximoEstado = &CMostraTelaContinuaVotacao::GetInst();   // lazy @1833104, form at +12 =
        //   CTelasVota::CriaTelaContinuacaoVotacao() (wasm 6557)
        break;
    default:
        break;                                         // other messages: state unchanged
    }
}

// wasm func 4433: CIniciodeCiclo::GetInst() = wasm 764(mutex @1833024, &ms_instancia @1833048,
// vtable vota::CIniciodeCiclo @1533432, 0).

// =============================================================================================
// uenux2/src/app/vota/eleitor/cinspecionaurna.cpp (path inferred)

// wasm func 11797 (vtable vota::CInspecionaUrna slot 7)                 // name inferred
void CInspecionaUrna::ProcessInput()
{
    // CInteractiveForm<IScreen, IInputKbd>::Read() inlined (cinteractiveform.h:57): current input
    // field (index +84) of the form at +12 reads from CPolySingleton<api::IInputKbd>; `.at()`
    // semantics (vector::__throw_out_of_range when the index is past the end).
    if (m_form->Read() != api::EInputResult::CONFIRMA /*9*/)
        return;
    // next state: CUrnaInspecionada (lazy @1837732, 28 bytes, CAppState(1)) with two screens:
    //   +12 CTelasVota::CriaTelaNeutra("Inspeção terminada.&Siga as instruções&no terminal do mesário.", 35, false)
    //   +20 CTelasVota::CriaTelaContinuacaoVotacao()
    m_proximoEstado = &CUrnaInspecionada::GetInst();
    CLogVota::GetInst().Loga("Inspeção da urna confirmada");
    // tell the operator terminal: message 11 on the priority queue (CPriorityMessageQueue +36)
    api::CPriorityMessageQueue<api::SMessage>::GetInst().Envia(api::SMessage{11}, 1);   // wasm 270 + 501
}

// =============================================================================================
// uenux2/src/app/vota/eleitor/fimvotacao/cretirarmr.cpp (owner u09)

// wasm func 2291                                                       // name inferred
// Lazy singleton of the "retirar MR" state (MR = mídia de resultado, the result memory card),
// 36 bytes @1833932, CAppState(2), with three screens. Every text is centred (alignment 2) at
// x = 320 (packed positions 0x00AA0140 etc.):
//   +12 "telaRetireMR" (non-interactive, comum 886): "Retire a mídia de resultado" (320,170),
//        "e faça a entrega" (320,220), "conforme as instruções." (320,270), FONTE_30
//   +20 "telaLacreTampa" (vota 576): "Lacre a tampa do compartimento" (320,145), "da mídia de
//        resultado conforme" (320,195), "as instruções" (320,245), FONTE_30, key {'C', "Continuar"}
//   +28 "telaMRModoDemo" (demonstration mode, vota 576): "Substitua a mídia de resultado por outra
//        vazia" (320,90), "para deixar a urna pronta para votação" (320,130), "ou" (320,210),
//        "Mantenha a mídia de resultado" (320,290), "para voltar para a verificação pré-eleição"
//        (320,330), FONTE_TEXTO, key {'C', "Prosseguir"}
// Callers: CImprimirBUOutrasObrigatorias/CImprimirBJust/CImprimindoBim/CImprimindoBEHB::StartState
// and CAjusteInicial.
CRetirarMR& CRetirarMR::GetInst();

// =============================================================================================
// uenux2/src/app/vota/eleitor/cinstrucaovotacaoacessibilidade.cpp (owner u06)
// Inlined into wasm func 7787 (dcmp 25777-25928); reconstructed here because no other copy exists.

// srcloc cinstrucaovotacaoacessibilidade.cpp:157 (throw) and :161 (CPolySingleton<ITextToSpeech>)
// Web build: when 7787 runs this, the ITextToSpeech singleton is still the
// simulador::CWasmNullTextToSpeech pushed at the start of votaInit (the CRHVoiceTextToSpeech is
// pushed only after 7787 returns), so nothing is synthesised here (votaInit: 76 ms with --audio,
// 74 ms without, headless run 2026-09-23).
void CInstrucaoVotacaoAcessibilidade::CacheInstructionAudio() const
{
    const std::string instrucao = GetKeyboardPosition();                               // wasm 4407
    // "A urna está pronta para receber o seu voto. Use o teclado numérico ... abaixo/à direita da urna
    //  para digitar o seu voto. Aperte a tecla 4 para fala mais lenta. ... 6 ... mais rápida ..."
    if (instrucao.find('{') != std::string::npos)
        throw CVotaError(EUeVotaError{9315}, "Não é permitido fazer cache de mensagem dinâmica");   // line 157

    auto& tts = api::CPolySingleton<api::ITextToSpeech>::instance();                   // line 161, wasm 2079
    const int velocidadeOriginal = tts.GetVelocidade();                                // vtable slot 5
    std::vector<int> velocidades;
    for (int v = 0; v <= 4; ++v)                                                       // 60/80/100/120/140 %
        velocidades.push_back(v);
    for (const int velocidade : velocidades) {
        tts.SetVelocidade(velocidade);                                                 // vtable slot 4
        for (const char tecla : {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'B', 'D', 'C'})
            tts.CacheMessage(api::KeyName(tecla));                                     // wasm 744 + 921
        tts.CacheMessage(instrucao);
    }
    tts.SetVelocidade(velocidadeOriginal);
}

} // namespace vota
