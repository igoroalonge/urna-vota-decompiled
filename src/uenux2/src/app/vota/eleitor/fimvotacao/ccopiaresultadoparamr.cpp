// Reconstructed from vota_web_wasm.wasm (unit u08).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/ccopiaresultadoparamr.cpp
//
// srcloc evidence:
//   :59   virtual void StartState()   Assert (vota.GetEstadoVota() == EAVCOPIARESULTADOSMR)      (3451)
//   :85   void CopiaResultado()       comum::IInterfaceInit lookup
//   :94   void CopiaResultado()       "A mídia de resultado não estava presente"                 (9367)
//   :112  void CopiaResultado()       "Mídia de resultado inválida para gravação de resultados"  (9368)
//   :126  void CopiaResultado()       "Mídia de resultado já contém arquivos de resultados."     (9369)
//   :174  void CopiaResultado()       "Nao copiou"                                               (9370)
//   api/util/csystem.cpp:724/730/739/745  static bool api::CSystem::AreFilesEqual(...), inlined
//
// Not executed in the recorded sessions (the web page never reaches the encerramento).

#include "vota/eleitor/fimvotacao/ccopiaresultadoparamr.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <format>
#include <memory>
#include <source_location>
#include <vector>

#include "api/util/csystem.h"
#include "comum/cappinfo.h"
#include "comum/cpath.h"
#include "comum/carquivosresultado.h"                // EExtensaoArquivoResultado
#include "comum/gravadores/ccopiadormr.h"            // comum::CCopiadorMR / CCopiadorWSQMR
#include "comum/gravadores/cgravadorutil.h"
#include "comum/iinterfaceinit.h"
#include "comum/validamidia/ivalidamidia.h"
#include "comum/dados/clocal.h"
#include "vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.h"
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;
using comum::md::estadoaplicacao::EEstadoEncerramento;
using comum::EExtensaoArquivoResultado;             // index of CArquivosResultado::operator[] (func 347)

// wasm func 12134 — vtable slot 2 (the tools named it CopiaResultado after the inlined srcloc)
void CCopiaResultadoParaMR::StartState()
{
    api::CApplicationContextGuard contexto(10, "Copiando resultado para MR", "Erro de gravação",
                                           "Ocorreu um erro durante a cópia do resultado para a MR.");

    auto& vota = comum::CAppInfo::GetInst().GetVota();                              // GetEstado<CEstadoGeralVota>
    UE_ASSERT(vota.GetEstadoVota() == EEstadoVota::EAVCOPIARESULTADOSMR);           // :59  (63)

    m_tela->Exibe();                                                                // "Gravando o resultado na mídia"
    CopiaResultado();

    vota.SetEstadoVota(EEstadoVota::EAVENCERRADA);                                  // 64
    vota.SetEstadoEncerramento(EEstadoEncerramento::EAEIMPRIMIROBRIGATORIABU);      // 50
    comum::SalvaEstado();                                                           // func 491
    m_proximoEstado = &CImprimirBUOutrasObrigatorias::GetInst();                    // func 5984
}

// srcloc :85..:174 — inlined into StartState
void CCopiaResultadoParaMR::CopiaResultado()
{
    auto& init = comum::IInterfaceInit::GetInst();                                  // :85
    if (init.GetDemoMode())
        return;                     // demonstration mode ("DEMONSTRAÇÃO PRÉ-ELEIÇÃO"): nothing is copied

    HabilitaMR(init);                                                               // func 2863

    if (!init.IsMRPresenteSemHabilitar()) {
        CLogVota::GetInst().LogaMRNaoPresente();       // func 5885 "Mídia de resultado não estava presente"
        throw CUeVotaError(9367, "A mídia de resultado não estava presente",
                           std::source_location::current());                        // :94
    }
    if (init.IsMRMontadoSemHabilitar())
        init.DesmontarMRSemDesabilitar();
    init.MontarMRSemHabilitar();

    auto& appInfo = comum::CAppInfo::GetInst();

    // A result stick that already has files must be one accepted by the media validator.
    if (!api::CSystem::IsEmptyDir(comum::CPath::GetPathMR())) {                     // "/dsk/mr/" (func 949)
        const auto validacao = comum::impl::IValidaMidia::GetInst().ValidaMidiaResultado(   // slot 3
            init.GetDispositivoMR(), appInfo.GetEstadoGeral().GetTurno());          // IInterfaceInit slot 4, +32
        if (!validacao.valida) {
            CLogVota::GetInst().LogaMRInvalida();      // func 5883 (level 3)
            UENUX_LOG(LOG_INFO, "mídia inválida para gravação pelo motivo [%d]", validacao.motivo);
            // ^ inlined helper: syslog when /dev/urna exists (cached in a static), otherwise printf
            //   only if the environment variable DEBUG_UENUX is set
            throw CUeVotaError(9368, "Mídia de resultado inválida para gravação de resultados",
                               std::source_location::current());                    // :112
        }
    }

    auto& local = comum::CLocal::GetInst();                                         // func 401
    const auto municipio = local.GetMunicipio();
    const auto zona      = local.GetZonaID();
    const auto secao     = local.GetSecaoID();
    const char fase      = appInfo.GetEstadoGeral().GetDadoCarga().GetFaseChar();   // func 2253

    // Outside training (EUrnaFase '3'), the stick must not contain results of another urna/section.
    if (appInfo.GetEstadoGeral().GetFase() != comum::EUrnaFase::Treinamento &&      // +48 != 51
        comum::impl::IValidaMidia::GetInst().MidiaContemResultados()) {             // slot 6
        CLogVota::GetInst().LogaMRInvalida();
        throw CUeVotaError(9369, "Mídia de resultado já contém arquivos de resultados.",
                           std::source_location::current());                        // :126
    }

    // file = "<fase><pleito:05><uf?><municipio:05><zona:04><secao:04>-" + CArquivosResultado[tipo]
    // (comum::CGravadorUtil::DeterminaNomeArquivoSemLetra, func 3798)
    const auto copiador = [&](EExtensaoArquivoResultado tipo) {
        return std::make_shared<comum::CCopiadorMR>(municipio, zona, secao, fase, tipo);   // func 1276
    };
    std::vector<std::shared_ptr<comum::CCopiadorMR>> copiadores{
        copiador(EExtensaoArquivoResultado(4)),    // bu.dat      Boletim de Urna
        copiador(EExtensaoArquivoResultado(6)),    // rdv.dat     Registro Digital do Voto
        copiador(EExtensaoArquivoResultado(8)),    // jufa.dat    justificativas
        copiador(EExtensaoArquivoResultado(9)),    // imgbu.dat   image of the printed BU
        copiador(EExtensaoArquivoResultado(11)),   // imgze.dat   image of the zerésima
        copiador(EExtensaoArquivoResultado(12)),   // hash.dat    hashes of the result files
        copiador(EExtensaoArquivoResultado(13)),   // log.jez     application log
        copiador(EExtensaoArquivoResultado(1)),    // vota.vsc    signature file
        copiador(EExtensaoArquivoResultado(19)),   // mr.ver      result-media version
    };
    if (local.UrnaBiometrica()) {                                                   // func 820
        const auto wsq = [&](EExtensaoArquivoResultado tipo) {
            return std::make_shared<comum::CCopiadorWSQMR>(municipio, zona, secao, fase, tipo);  // func 3827
        };
        copiadores.push_back(wsq(EExtensaoArquivoResultado(16)));   // wsqbio.jez  fingerprints (voters)
        copiadores.push_back(wsq(EExtensaoArquivoResultado(17)));   // wsqman.jez
        copiadores.push_back(wsq(EExtensaoArquivoResultado(18)));   // wsqmes.jez  (mesários)
    }

    for (const auto& c : copiadores) {
        c->Copia();                                                                 // CCopiadorMR slot 2
        api::CSynchronizer::CreateInst()->Sincroniza();                            // fsync (shared_f620)
    }

    // Only the BU is verified: result directory copy == MR copy.
    const std::string nomeBU = comum::CGravadorUtil::DeterminaNomeArquivoSemLetra(
        municipio, zona, secao, fase, EExtensaoArquivoResultado(4));
    const auto origem  = comum::CPath::GetPathResult(0) / nomeBU;                  // func 1274
    const auto destino = comum::CPath::GetPathMR() / nomeBU;

    const bool copiou = api::CSystem::FileExists(origem) && api::CSystem::FileExists(destino) &&  // api_f412
                        api::CSystem::AreFilesEqual(origem, destino);               // csystem.cpp:724..745
    if (!copiou) {
        CLogVota::GetInst().Loga(3, "Erro na cópia dos resultados para MR");
        throw CUeVotaError(9370, "Nao copiou", std::source_location::current());    // :174
    }

    init.DesmontarMRSemDesabilitar();
    CLogVota::GetInst().Loga("Mídia de resultado gravada");
}

// ------------------------------------------------------------------------------------------------
// api::CSystem::AreFilesEqual (uenux2/src/api/util/csystem.cpp:720-750), as inlined here:
//   sizes must match; both files fopen(..., "rb") ("Falha ao abrir arquivo [{}]: {}", EUeUtilError
//   7059/7060); compared in 32 KiB chunks read into two static buffers (@1839328, @1872096) with
//   memcmp ("Falha ao ler arquivo [{}]: {}", 7061/7062 when fread returns 0 with errno set).
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// Functions of other files that the tools attributed to this one:
//
// wasm func 1276 — comum::CCopiadorMR::CCopiadorMR(TMunicipioID, TZonaID, TSecaoID, char fase, tipo)
//   (uenux2/src/app/comum/gravadores/ccopiadormr.cpp, path inferred):
//       vptr = CCopiadorMR; m_nome (+4) = CGravadorUtil::DeterminaNomeArquivoSemLetra(mun, zona, secao, fase, tipo)
// wasm func 3827 — comum::CCopiadorWSQMR::CCopiadorWSQMR(...) : CCopiadorMR(...) {}  (vptr = CCopiadorWSQMR)
// wasm func 5883 — vota::CLogVota::LogaMRInvalida()   name inferred
//       Loga(3, "Mídia de resultado inválida para gravação de resultados")      (api::CLoga::loga level 3)
// wasm func 2863 (not in this unit) — HabilitaMR(IInterfaceInit& init): name inferred
//       init.EnviarMensagemThrowVoid(10, "habilitando MR"); api::CWait::Sleep(500) (emscripten_sleep
//       guarded by the flag @1584624)
// ------------------------------------------------------------------------------------------------

}  // namespace vota
