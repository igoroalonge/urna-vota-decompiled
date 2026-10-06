// uenux2/src/app/vota/comum/csincronizavota.cpp  --  FRAGMENT written by unit u02 (owner u26)
// + comum helper of unknown file. Names inferred.

namespace vota {

// wasm func 4662                                                       // name inferred
// Persists uenux.db unless the urna is shutting down. Callers: CInformacaoEleitor::InicializarPersistencia,
// inlined into wasm func 7787 (vota::CInformacaoEleitor::Inicializar), and
// vota::CControladorRegistraMesariosVota vtable slot 16 (wasm 10786, a 5-byte tail call to this).
void CSincronizaVota::SincronizaBancoDados()
{
    // inlined static VerificaUrnaDesligando() (csincronizavota.cpp:47): `if (desligando) throw
    // api::CUeDesligandoError("Urna desligando", srcloc :47)`. The test of byte @1832936 is inlined
    // here; only the exception construction stayed out of line (wasm 1685, which carries the
    // srcloc and therefore the name VerificaUrnaDesligando).
    VerificaUrnaDesligando();
    comum::GravaBancoDadosNaMI();                                // wasm 4657
}

// wasm func 10786 (vtable vota::CControladorRegistraMesariosVota slot 16)
void CControladorRegistraMesariosVota::SincronizaBancoDados()   // name inferred
{
    CSincronizaVota::SincronizaBancoDados();
}

} // namespace vota

namespace comum {

// wasm func 4657                                                       // name inferred
// "Gravando o banco de dados na MI": signs uenux.db and copies it from the internal to the external
// flash, inside an application-context guard whose error text is
// "Ocorreu um erro durante a persitência dos dados na MI." [sic: "persitência"].
void GravaBancoDadosNaMI()
{
    // argument order from the stack slots: (2, a+92 = "" (empty SSO string), a+80 = "Gravando ...",
    // a+68 = "Ocorreu ...")
    api::CApplicationContextGuard guarda(2, "", "Gravando o banco de dados na MI",
                                         "Ocorreu um erro durante a persitência dos dados na MI.");  // wasm 676
    {
        // key/"arquivo" id 122 in the 1st turn (turno '1'), 123 otherwise
        CAssinador assinador(CAppInfo::GetInst().GetGeral().GetTurno() == '1' ? 122 : 123);  // wasm 1501
        AssinarUE(assinador, 110);                      // wasm 1277: signs file id 110 = uenux.db (-> uenux.vsu;
                                                        // simulated by (anon)::CWasmSavd in the web build)
        api::CSynchronizer::CreateInst().Sincroniza();  // wasm 600 + 620 (fsync)
    }                                                    // ~CAssinador (wasm 1007)
    const auto& arquivos = CArquivosSavd::GetInst();                                // wasm 1164
    // ids 110 and 111 both map to "uenux.db" in the CArquivosSavd table (wasm 1164)
    const std::filesystem::path origem  = CPath::GetPathTrab(EFlashOrigem::INTERNA) / arquivos[110];  // wasm 680
    const std::filesystem::path destino = CPath::GetPathTrab(EFlashOrigem::EXTERNA) / arquivos[111];
    api::CSystem::CopyFile(origem, destino, false);                                  // wasm 378
    // ~guarda (wasm 675)
    // wasm 4682 (name inferred): same pattern for the MV (mídia de votação): guard(4, "",
    // "Gravando o banco de dados na MV", "Ocorreu um erro durante a persitência dos dados na MV."),
    // then CSystem::CopyFile(CArquivosSavd[199], CArquivosSavd[201]) in the 1st turn ('1'),
    // CArquivosSavd[200] -> [202] otherwise.
    GravaBancoDadosNaMV();
    api::CSynchronizer::CreateInst().Sincroniza();
}

} // namespace comum
