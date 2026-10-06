// FRAGMENT reconstructed by unit u07 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/comum/csincronizavota.cpp (attested by srcloc records :47, :290, :303, :392).
// The rest of the file belongs to another unit (u26); only the code that u07 saw is here:
//   * wasm func 4682 (in u07), and
//   * the two anonymous-namespace functions whose bodies are inlined into
//     vota::impl::CSincronismoVotoEleitor::SincronizaVoto (wasm func 7174, csincronismovotoeleitor.cpp).
// Merge into csincronizavota.cpp.
//
// Vocabulary: MI = "memória interna" (internal flash, /dsk/fi), MV = "memória de votação" (removable flash
// card, /dsk/fe). RDV = Registro Digital do Voto (rdv.dat, encrypted). vota.bin = EstadoGeralVota (BER).
// uenux.db = SQLite database with the dynamic voter data. *.vsu = signature packages (CArquivosSavd).
//
// ESavdArquivoUE ids used here (from the CArquivosSavd tables built in wasm func 1164):
//   31 = "vota.bin", 83 = "rdv.dat", 110 = "uenux.db" (MI), 111 = "uenux.db" (MV)
// ESavdPacote ids (signature files; the pair is chosen by the current turno, CEstadoGeral +32 == '1'):
//   122/123 = vota.vsu  (MI, turno 1/2)   124/125 = vota.vsu  (MV, turno 1/2)
//   134/135 = rdv.vsu   (MI, turno 1/2)   136/137 = rdv.vsu   (MV, turno 1/2)
//   199/200 = uenux.vsu (MI, turno 1/2)   201/202 = uenux.vsu (MV, turno 1/2)
#include "vota/comum/csincronizavota.h"

#include <string>
#include <vector>

#include "api/io/cencryptedfile.h"                 // api::CEncryptedFile
#include "api/util/csystem.h"                      // api::CSystem::CopyFile / ReplaceFile
#include "api/util/csynchronizer.h"                // api::CSynchronizer
#include "api/gui/capplicationcontextstack.h"      // api::CApplicationContextGuard
#include "comum/appinfo/cappinfo.h"                // comum::CAppInfo, comum::GetEstadoGeral
#include "comum/carquivossavd.h"                   // comum::CArquivosSavd
#include "comum/cassinador.h"                      // comum::CAssinador
#include "comum/cpath.h"                           // comum::CPath
#include "comum/dados/celeitores.h"                // comum::CEleitores
#include "comum/dados/crdvvota.h"                  // comum::CRdvVota
#include "comum/dados/dao/celeitordinamicodao.h"   // comum::dao::CEleitorDinamicoDAO
#include "vota/comum/votadefs.h"               // vota::VerificaAssinaturaMI / MV, CUeVotaError

namespace vota {

void CopiaAssinaturaBancoDadosParaMV();                               // wasm func 4682, defined below

namespace {

bool TurnoUm()                                                      // inlined everywhere; name inferred
{
    return comum::GetEstadoGeral(comum::CAppInfo::GetInst()).GetTurno() == '1';   // CEstadoGeral +32
}

// Encrypts the in-memory RDV to "<arquivo>.tmp", reads it back and asks the RDV to compare the
// decrypted content with itself. Inlined twice into wasm func 7174.                  // name inferred
bool GravaEConfereRDV(comum::CRdvVota& rdv, const std::string& arquivo)
{
    const std::string temporario = arquivo + ".tmp";
    {
        api::CEncryptedFile cifrado(rdv.GetCifrador());               // func 1692; shared_ptr<ISymmetricCipher> at CRdvVota+8
        const std::vector<uebyte> conteudo = rdv.Converte();          // CRdv slot 11
        cifrado.MemWrite(conteudo);                                    // func 2766
        cifrado.Save(temporario);                                      // func 2767 (cencryptedfile.cpp:88, CFile::Flush inlined)
    }                                                                  // func 1691 (~CEncryptedFile)
    std::vector<uebyte> lido;
    {
        api::CEncryptedFile cifrado(rdv.GetCifrador());
        cifrado.Load(temporario);                                      // func 3653
        cifrado.MemRead(lido);                                         // func 3652
    }
    return rdv.ConfereConteudo(lido);                                  // CRdv slot 13
}

// csincronizavota.cpp:290 (the throw). Inlined into wasm func 7174.
void SincronizaRDVInternoSeguro()
{
    CSincronizaVota::VerificaUrnaDesligando();       // flag @1832936; throws api::CUeDesligandoError (csincronizavota.cpp:47)

    comum::CRdvVota& rdv = comum::CRdvVota::GetInst();                          // func 555
    if (!GravaEConfereRDV(rdv, comum::CPath::ArquivoRdvInterno()))              // func 1551: <trab MI>/rdv.dat
        throw CUeVotaError(9300, "Falha na gravação do RDV na MI");             // csincronizavota.cpp:290

    comum::VerificaIntegridadeReferencial();         // func 2543: CIntegridadeReferencial over CRdvVota, CCargos, CRespostas
    api::CSystem::ReplaceFile(comum::CPath::ArquivoRdvInterno() + ".tmp",
                              comum::CPath::ArquivoRdvInterno(), "");          // func 5455 ("CSystem::ReplaceFile")
    comum::CAppInfo::GetInst().SalvaVotaInterno();    // func 3790: vota.bin of the current turno, MI

    if (!comum::EhTreinamentoEleitor()) {            // func 697: fase '3' && EstadoGeralVota.treinamentoEleitor
        auto& eleitores = comum::CEleitores::GetInst();
        {
            comum::dao::CEleitorDinamicoDAO dao(comum::CPath::BancoInterno());  // func 2826: <trab MI>/uenux.db
            dao.Salva(eleitores.GetEleitorAtual().GetDinamico());               // DAO slot 6 ("já votou" etc.)
        }
        api::CSynchronizer::CreateInst().Sync();      // compiled to nothing in this build (func 620 = dead load)
        comum::CAssinador assinador(TurnoUm() ? 122 : 123);                   // func 1501 (vota.vsu, MI)
        assinador.Assina(110);                                                  // uenux.db (func 1277)
        api::CSynchronizer::CreateInst().Sync();
    }
    {
        comum::CAssinador assinador(TurnoUm() ? 122 : 123);
        assinador.Assina(83);                                                   // rdv.dat
        assinador.Assina(31);                                                   // vota.bin
    }
    api::CSynchronizer::CreateInst().Sync();

    VerificaAssinaturaMI("vota.vsu");                // votadefs.cpp:103 (func 3290)
    VerificaAssinaturaMI("rdv.vsu");
    if (!comum::EhTreinamentoEleitor())
        VerificaAssinaturaMI("uenux.vsu");
}

// csincronizavota.cpp:303 (the throw). Inlined into wasm func 7174.
void SincronizaRDVExternoSeguro()
{
    CSincronizaVota::VerificaUrnaDesligando();

    comum::CRdvVota& rdv = comum::CRdvVota::GetInst();
    if (!GravaEConfereRDV(rdv, comum::CPath::ArquivoRdvExterno()))              // func 1701: <trab MV>/rdv.dat
        throw CUeVotaError(9301, "Falha na gravação do RDV na MV");             // csincronizavota.cpp:303

    comum::VerificaIntegridadeReferencial();
    api::CSystem::ReplaceFile(comum::CPath::ArquivoRdvExterno() + ".tmp",
                              comum::CPath::ArquivoRdvExterno(), "");
    comum::CAppInfo::GetInst().SalvaVotaExterno();    // func 3789: vota.bin of the current turno, MV

    auto& savd = comum::CArquivosSavd::GetInst();     // func 1164
    if (!comum::EhTreinamentoEleitor()) {
        // The voter database is NOT rewritten on the MV: the MI copy is duplicated, then its signature.
        const std::string origem  = comum::CPath::GetPathTrab(comum::MI) / savd[110];   // func 436(0) / "uenux.db"
        const std::string destino = comum::CPath::GetPathTrab(comum::MV) / savd[111];   // func 436(1) / "uenux.db"
        api::CSystem::CopyFile(origem, destino, false);                                // func 378
        CopiaAssinaturaBancoDadosParaMV();                                             // func 4682
    }

    // The MV files are not re-signed either: the MI signature packages are copied. (Inference: they can
    // only validate on the MV if rdv.dat / vota.bin written there are byte-identical to the MI ones, i.e.
    // if the RDV encryption is deterministic - the CAesKey of u01 carries its own IV.)
    if (TurnoUm()) {
        api::CSystem::CopyFile(savd[122], savd[124], false);         // vota.vsu
        api::CSystem::CopyFile(savd[134], savd[136], false);         // rdv.vsu
    } else {
        api::CSystem::CopyFile(savd[123], savd[125], false);
        api::CSystem::CopyFile(savd[135], savd[137], false);
    }
    api::CSynchronizer::CreateInst().Sync();

    // VerificaAssinaturaMV(n) = IInterfaceSavd::GetInst().ValidarUE(CPath::GetPathTrab(MV, turno) / n)
    VerificaAssinaturaMV("vota.vsu");                // votadefs.cpp:110 (func 3288)
    VerificaAssinaturaMV("rdv.vsu");
    if (!comum::EhTreinamentoEleitor())
        VerificaAssinaturaMV("uenux.vsu");
}

} // namespace

// ---------------------------------------------------------------------------------------------------
// wasm func 4682 (in unit u07)                                                       // name inferred
// Copies the uenux.db signature package of the current turno from the MI to the MV, inside an application
// context that turns any failure into the "persistência na MV" error screen. Callers: func 7174 above and
// func 4657 (the MI counterpart: "Gravando o banco de dados na MI", which signs uenux.db and copies it).
void CopiaAssinaturaBancoDadosParaMV()
{
    auto& savd = comum::CArquivosSavd::GetInst();
    api::CApplicationContextGuard contexto(4, "",
                                           "Gravando o banco de dados na MV",
                                           "Ocorreu um erro durante a persitência dos dados na MV.");   // sic "persitência"
    if (TurnoUm())
        api::CSystem::CopyFile(savd[199], savd[201], false);         // uenux.vsu MI -> MV (turno 1)
    else
        api::CSystem::CopyFile(savd[200], savd[202], false);         // uenux.vsu MI -> MV (turno 2)
}                                                                     // ~CApplicationContextGuard (func 675)

// The public entry points called from CSincronismoVotoEleitor::SincronizaVoto (names inferred; the
// wrappers themselves are fully inlined):
void CSincronizaVota::SincronizaRDVInterno() { SincronizaRDVInternoSeguro(); }
void CSincronizaVota::SincronizaRDVExterno() { SincronizaRDVExternoSeguro(); }

} // namespace vota
