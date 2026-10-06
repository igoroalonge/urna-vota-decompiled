// Compiler-generated functions of the voting application (uenux2/src/app/vota/eleitor/...) that the tools
// attributed to the web platform (unit u30) only because they sit next to web-entry functions in the
// function table ("data-table neighbours"). Reconstructed/identified by unit u30.
//
// The voter states are lazily created singletons. Their GetInst() is (merged bodies 764 / 6051 / 1961 / 2901):
//
//     static std::mutex s_mutex;                         // lock() vanished (no pthreads), unlock residue = func 150
//     static std::unique_ptr<CEstado> s_inst;
//     std::lock_guard lock(s_mutex);
//     if (!s_inst) s_inst = std::make_unique<CEstado>(...);
//     return *s_inst;
//
// and each static gets an atexit destructor ("__dtor_s_mutex", "__dtor_s_inst"). Emscripten never runs
// atexit handlers in this build (the runtime is kept alive; there is no exit), so all of these are dead code.
// The unique_ptr ones call vota_f1286 (reset: IConfereVotoEmCargo::~ via vtable slot 0, then free) or
// vota_f906 (reset: CVotacaoStateAudio::~, then free); the mutex ones are the folded
// "noexcept pthread stub" body func 150 applied to the mutex address.
//
//   func | static                          | singleton (found in the GetInst inlined into ...)
//   -----+---------------------------------+------------------------------------------------------------------
//  11681 | std::mutex          @1838344    | CConfereVotoEmCargo<CMajoritarioBranco, ...>  (CPedeMajoritario::
//  11680 | unique_ptr          @1838372    |   ProcessInputAudio, func 11683, cpedemajoritario.cpp)
//  11679 | std::mutex          @1838380    | CConfereVotoEmCargo<CMajoritarioNulo, ...>     (same)
//  11678 | unique_ptr          @1838408    |
//  11677 | std::mutex          @1838416    | CConfereVotoEmCargo<CMajoritarioRepetido, ...> (same)
//  11676 | unique_ptr          @1838444    |
//  11685 | std::mutex          @1838316    | CPedeMajoritario::GetInst (func 5921 -> 6051)
//  11686 | unique_ptr          @1838340    |
//  11743 | std::mutex          @1837792    | CConfereVotoEmCargo<CCandidatoInapto, ...>     (CPedeNulo::
//  11742 | unique_ptr          @1837820    |   GetProximoEstado 11744, CPedeNominal slot 16 11724)
//  11741 | std::mutex          @1837828    | CConfereVotoEmCargo<CProporcionalNulo, ...>    (CPedeNulo, 11744)
//  11740 | unique_ptr          @1837856    |
//  11746 | std::mutex          @1837764    | CPedeNulo::GetInst (inlined in CPedeProporcional::
//  11747 | unique_ptr          @1837788    |   ProcessInputAudio, func 11711)
//
// Written out, e.g. for func 11680 / 11681:
//
//     // wasm func 11680 - atexit destructor of CConfereVotoEmCargo<CMajoritarioBranco, ...>::GetInst()::s_inst
//     static void __dtor_s_inst_MajoritarioBranco() { s_inst.reset(); }          // vota_f1286(_, &s_inst)
//     // wasm func 11681 - atexit destructor of the same GetInst()::s_mutex
//     static void __dtor_s_mutex_MajoritarioBranco() { s_mutex.~mutex(); }       // pthread_mutex_destroy stub
