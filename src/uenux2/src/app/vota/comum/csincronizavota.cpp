// uenux2/src/app/vota/comum/csincronizavota.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). std::source_location records:
//   :47  static void CSincronizaVota::VerificaUrnaDesligando()             (wasm func 1685)
//   :290 void (anonymous namespace)::SincronizaRDVInternoSeguro()          (see csincronizavota.u07.cpp)
//   :303 void (anonymous namespace)::SincronizaRDVExternoSeguro()          (see csincronizavota.u07.cpp)
//   :392 static void CSincronizaVota::SincronizaRelatorios(const string&)  (wasm func 1836)
//
// Vocabulary: MI = memória interna (/dsk/fi, internal flash), MV = memória de votação (/dsk/fe, memory card),
// trab<N> = work directory of the current turno (CPath::GetPathTrab), SAVD = the urna's signing service
// (comum::IInterfaceSavd; in the web build (anonymous)::CWasmSavd, which signs nothing).
#include "vota/comum/csincronizavota.h"

#include <filesystem>

#include "api/hwil/cuedesligandoerror.h"        // api::CUeDesligandoError = CBaseError<api::EUeDesligandoError,
                                                //   SErrorLimits{4200, 4250}> (typeinfo @1532348, vtable @1599940)
#include "api/util/csynchronizer.h"
#include "api/util/csystem.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/carquivossavd.h"
#include "comum/cpath.h"
#include "vota/comum/votadefs.h"                 // vota::CUeVotaError
#include "vota/comum/cassinadorvota.h"           // vota::CAssinadorVota (ctor func 1501, path inferred)

namespace vota {

using comum::ESavdArquivoUE;
using comum::ESavdPacote;

namespace {

bool TurnoUm()                                    // CEstadoGeral +32 == '1'; inlined everywhere   name inferred
{
    return comum::CAppInfo::GetInst().GetGeral().GetTurno() == '1';
}

// Report files accepted by SincronizaRelatorios, with the signature package of each one in the work
// directory of the MI / MV, per turno (ids from comum::CArquivosSavd, table in carquivossavd.u22.cpp).
struct SRelatorioSincronizavel {                 // name inferred
    ESavdArquivoUE arquivo;
    ESavdPacote pacoteMI[2];                      // [turno 1, turno 2]
    ESavdPacote pacoteMV[2];
};
constexpr SRelatorioSincronizavel RELATORIOS[] = {
    {ESavdArquivoUE{86},  {ESavdPacote{126}, ESavdPacote{127}}, {ESavdPacote{128}, ESavdPacote{129}}},  // bu.dat   / bu.vsu
    {ESavdArquivoUE{84},  {ESavdPacote{130}, ESavdPacote{131}}, {ESavdPacote{132}, ESavdPacote{133}}},  // buj.dat  / buj.vsu
    {ESavdArquivoUE{88},  {ESavdPacote{138}, ESavdPacote{139}}, {ESavdPacote{140}, ESavdPacote{141}}},  // ze.dat   / ze.vsu
    {ESavdArquivoUE{89},  {ESavdPacote{142}, ESavdPacote{143}}, {ESavdPacote{144}, ESavdPacote{145}}},  // rze.dat  / rze.vsu
    {ESavdArquivoUE{106}, {ESavdPacote{191}, ESavdPacote{192}}, {ESavdPacote{193}, ESavdPacote{194}}},  // bim.dat  / bim.vsu
    {ESavdArquivoUE{108}, {ESavdPacote{195}, ESavdPacote{196}}, {ESavdPacote{197}, ESavdPacote{198}}},  // behb.dat / behb.vsu
};

// Inlined into func 1836 (name inferred). Copies the signature package of `arquivo` from the MI to the MV.
// For a name that is none of the six reports ALL six packages are copied (dead branch here: the caller has
// already rejected such names).
void CopiaAssinaturaRelatorioParaMV(const std::string& arquivo)
{
    const auto& savd = comum::CArquivosSavd::GetInst();                            // func 1164
    const int t = TurnoUm() ? 0 : 1;
    for (const auto& r : RELATORIOS) {
        if (arquivo == savd[r.arquivo]) {                                           // func 680
            api::CSystem::CopyFile(savd[r.pacoteMI[t]], savd[r.pacoteMV[t]], false);   // func 275, func 378
            return;
        }
    }
    for (const auto& r : RELATORIOS)
        api::CSystem::CopyFile(savd[r.pacoteMI[t]], savd[r.pacoteMV[t]], false);
}

} // namespace

// wasm func 1685 is the out-of-line part of the throw-expression (constructor call with srcloc :47 and the
// vptr of api::CUeDesligandoError); the flag test is inlined at every call site.
void CSincronizaVota::VerificaUrnaDesligando()
{
    if (ms_desligando)                                                                    // byte @1832936
        throw api::CUeDesligandoError(api::EUeDesligandoError(4201), "Urna desligando");  // :47
}

// wasm func 1836 (srcloc :392). Called right after a report file was written into <MI>/dinamico/trab<N>/.
void CSincronizaVota::SincronizaRelatorios(const std::string& arquivo)
{
    VerificaUrnaDesligando();
    api::CSynchronizer::CreateInst().Sync();                                      // fsync; empty in this build (shared_f620)

    const auto& savd = comum::CArquivosSavd::GetInst();
    {
        CAssinadorVota assinador(TurnoUm() ? ESavdPacote{122} : ESavdPacote{123});   // func 1501 (vota.vsu of the MI)

        ESavdArquivoUE id{};
        bool encontrado = false;
        for (const auto& r : RELATORIOS) {                                        // unrolled: 86, 84, 88, 89, 106, 108
            if (arquivo == savd[r.arquivo]) {
                id = r.arquivo;
                encontrado = true;
                break;
            }
        }
        if (!encontrado)   // literal @124100 is "O arquivo" with NO trailing space: "O arquivox.dat não é ..." (sic)
            throw CUeVotaError(9302, "O arquivo" + arquivo +
                                     " não é um relatório válido para sincronização");   // :392
        assinador.Assina(id);                                                      // func 1277 -> SAVD (id 66 = sign)
        api::CSynchronizer::CreateInst().Sync();

        const std::string origem  = comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA) / arquivo;   // func 436(0)
        const std::string destino = comum::CPath::GetPathTrab(comum::EFlashOrigem::EXTERNA) / arquivo;   // func 436(1)
        api::CSystem::CopyFile(origem, destino, false);                            // func 378

        CopiaAssinaturaRelatorioParaMV(arquivo);
        api::CSynchronizer::CreateInst().Sync();
    }                                                                              // ~CAssinadorVota (vtable slot 0)
}

} // namespace vota
