// FRAGMENT of uenux2/mock/app/comum/cappinfobuilder.cpp reconstructed by unit u28 from vota_web_wasm.wasm.
// Attested by three std::source_location records of this file:
//   :112 col 37  CAppInfoBuilder &comum::teste::CAppInfoBuilder::SalvaGeral(bool, std::vector<EMidia>)
//   :277 col 28  CAppInfoBuilder &comum::teste::CAppInfoBuilder::SalvaApps(bool, std::vector<EMidia>, EUrnaTurno,
//                                                                         std::vector<EApp>)
//   :287 col 22  CAppInfoBuilder &comum::teste::CAppInfoBuilder::SalvaApp(bool, EFlashOrigem, EUrnaTurno, EApp)
// The three records are the default `std::source_location::current()` argument of the two Converte helpers,
// i.e. they locate the CALLS to Converte. SalvaApp has no function of its own: it is inlined into SalvaApps.
// Other functions of this file (constructor, setters, Converte(EMidia), EscreveArquivo, the CriaEstado helpers,
// the IServicoEstado<...>::Salva instantiations) are reconstructed by units u29/u30.
//
// Both functions run once, during votaInit, with assina == false (see cappinfobuilder.u28.h). Observed executing.

#include "mock/app/comum/cappinfobuilder.u28.h"

#include <filesystem>
#include <format>
#include <stdexcept>

#include "comum/appinfo/servicos/cservicoestadogeral.h"
#include "comum/appinfo/servicos/cservicoestadogeralgap.h"
#include "comum/appinfo/servicos/cservicoestadogeralsa.h"
#include "comum/appinfo/servicos/cservicoestadogeralvota.h"
#include "comum/cpath.h"

namespace comum::teste {

// Inlined into SalvaApps (no wasm function of its own). Maps the turno to the index of the per-turno arrays.
// The enum is formatted as its integer value (std::formatter over the underlying type, handle func 9868),
// e.g. "Converte - turno invalido [51] da linha [287]" for EUrnaTurno::Atual ('3').
std::size_t Converte(EUrnaTurno turno, const std::source_location& local)
{
    switch (turno) {
    case EUrnaTurno::Primeiro: return 0;                       // '1'
    case EUrnaTurno::Segundo:  return 1;                       // '2'
    default:
        throw std::logic_error(std::format("Converte - turno invalido [{}] da linha [{}]", turno, local.line()));
    }
}
// For reference, its sibling (func 5344, unit u29) is:
//   EFlashOrigem Converte(EMidia midia, const std::source_location& local)
//   {
//       switch (midia) {                          // compiled as one unsigned test (i32.ge_u 2): any value
//       case EMidia::FlashInterna: return EFlashOrigem::INTERNA;   // outside {0, 1}, negative ones included,
//       case EMidia::FlashExterna: return EFlashOrigem::EXTERNA;   // throws; 0/1 are returned unchanged
//       }
//       throw std::logic_error(std::format("Converte - midia invalida [{}] da linha [{}]", midia, local.line()));
//   }

// wasm func 10243 (srcloc line 112, the Converte call). Writes dinamico/eg.bin on every requested flash.
CAppInfoBuilder& CAppInfoBuilder::SalvaGeral(bool assina, std::vector<EMidia> midias)
{
    for (const EMidia midia : midias) {
        const EFlashOrigem origem = Converte(midia);                                   // :112  func 5344
        if (assina)                                                                    // dead: always false here
            EscreveArquivo((CPath::GetPathDinamico(origem) / "eg.vsu").string(),       // 1082, path ctor 11637,
                           "assinatura EG");                                          // operator/ 5968, 10293
        // CServicoEstadoGeral = {vptr @1558024, origem} built inline (func 1941); Salva = IServicoEstado<
        // CEstadoGeral, CConversorEstadoGeral>::Salva (func 3592 -> 2894): GetPathArquivo() (<dinamico>/eg.bin),
        // BER-encode with CConversorEstadoGeral, api::CFileASN::WriteToFile.
        CServicoEstadoGeral(origem).Salva(CriaEstado(m_geral));                        // 10205, then ~ 2793
    }
    return *this;
}

// Inlined into SalvaApps (srcloc line 287 is its Converte call). One state file of one application, for one
// flash memory and one turno.
CAppInfoBuilder& CAppInfoBuilder::SalvaApp(bool assina, EFlashOrigem origem, EUrnaTurno turno, EApp app)
{
    const std::size_t idx = Converte(turno);                                           // :287
    const std::filesystem::path trab = CPath::GetPathTrab(origem, turno);              // func 358: <flash>/dinamico/trabN/

    switch (app) {
    case EApp::Gap:
        if (assina)                                                                    // dead: always false here
            EscreveArquivo((trab / "gap.vsu").string(), "assinatura EG Gap");          // path ctor 10134
        CServicoEstadoGeralGap(origem, turno)                                          // func 5812 -> ctor 3897
            .Salva(CriaEstado(m_gap[idx]));                                            // 10123, Salva 10112 -> 2894,
        break;                                                                         // ~CEstadoGeralGap 1698
    case EApp::SA:
        if (assina)
            EscreveArquivo((trab / "sa.vsu").string(), "assinatura EG SA");            // path ctor 11637
        CServicoEstadoGeralSA(origem, turno)                                           // func 11566 -> 3897
            .Salva(CriaEstado(m_sa[idx]));                                             // 10101, Salva 10089 -> 2894
        break;
    case EApp::Vota:
        if (assina)
            EscreveArquivo((trab / "vota.vsu").string(), "assinatura EG Vota");        // path ctor 10078
        CServicoEstadoGeralVota(origem, turno)                                         // func 3787 -> 3897
            .Salva(CriaEstado(m_vota[idx]));                                           // 10067, Salva 5329 -> 2894
        break;
    }                                                   // any other EApp value: nothing written, no error
    return *this;
}

// wasm func 10212 (srcloc line 277, the Converte call). For every flash memory, every application's state file
// of the given turno: <flash>/dinamico/trab<turno>/{gap,sa,vota}.bin.
CAppInfoBuilder& CAppInfoBuilder::SalvaApps(bool assina, std::vector<EMidia> midias, EUrnaTurno turno,
                                            std::vector<EApp> apps)
{
    for (const EMidia midia : midias) {
        const EFlashOrigem flash = Converte(midia);                                    // :277  func 5344
        for (const EApp app : apps)
            SalvaApp(assina, flash, turno, app);                                       // inlined
    }
    return *this;
}

}  // namespace comum::teste
