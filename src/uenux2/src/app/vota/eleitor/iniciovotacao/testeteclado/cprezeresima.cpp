// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). std::source_location record: :56 (GetEstadoPassouNoTeste).
// The two question screens (funcs 6580 / 6581) are CTelasVota methods: see ctelasvota.u26.cpp.
#include "vota/eleitor/iniciovotacao/testeteclado/cprezeresima.h"

#include <string>

#include "api/pattern/cpolysingletonlist.h"
#include "api/util/isystemdatetime.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "ecourna/api/util/cstringutils.h"
#include "vota/comum/votadefs.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/iniciovotacao/cgeradadosdinamicos.h"
#include "vota/eleitor/iniciovotacao/cverificahorariozeresima.h"
#include "vota/log/clogvota.h"

namespace vota::testeteclado {

using comum::md::estadoaplicacao::EEstadoVota;

namespace {
// The day before the zerésima date: `i64.load offset=544` of CConfiguracaoEleicao, i.e. the CDate part of the
// data/hora da zerésima (+544..+555, the same field CVerificaHorarioZeresima copies in func 5947), then
// comum_f3648 = copy + func 5477 (subtract N days; negative N -> func 5478 adds). The zerésima is printed on
// election day, so in practice this is the eve of the election.                          name inferred
api::CDate VesperaZeresima()
{
    api::CDate vespera = comum::CConfiguracaoEleicao::GetInst().GetHorarioZeresima().GetDate();   // cfg +544
    vespera -= 1;
    return vespera;
}
} // namespace

// wasm func 11862 (vtable slot 10)
CFormInterativoTelaVota CPreZeresima::CriaTela()
{
    m_agora = api::CDateTime(api::CPolySingletonList::instance<api::ISystemDateTime>().GetDataHora());   // ConvertFromLocalTime
    const bool antesDaVespera = m_agora.GetDate().Compare(VesperaZeresima()) < 0;                   // unknown_f1261
    return antesDaVespera ? CTelasVota::CriaTelaTesteTecladoOpcional()     // func 6581 "Quer testar o teclado?"
                          : CTelasVota::CriaTelaTesteTeclado();            // func 6580 "Por favor, teste o teclado"
}

// wasm func 5945 (vtable slot 11, srcloc :56). Where the start-of-day flow continues after the test.
comum::CAppState* CPreZeresima::GetEstadoPassouNoTeste()
{
    const EEstadoVota estado = comum::CAppInfo::GetInst().GetVota(comum::EUrnaTurno::Atual).GetEstadoVota();
    switch (estado) {
    case EEstadoVota::EAVINICIAL:                     // '1'
    case EEstadoVota::EAVGERADADOSDINAMICOS:          // '2'
        return &CGeraDadosDinamicos::GetInst();      // lazy singleton @1834716 (12-byte CAppState, flags 0)
    case EEstadoVota::EAVAGUARDAHORAZERESIMA:         // '3'
        return &CVerificaHorarioZeresima::GetInst(); // func 5947
    default:
        CLogVota::GetInst().LogaErroEstadoAplicativoNaoEsperado();   // func 5881 (severity 3)
        throw CUeVotaError(9377, "Estado nao esperado " +
                                 std::to_string(static_cast<int>(estado)));   // :56 (ecourna_f296: e.g. "52" for '4')
    }
}

// wasm func 11861 (vtable slot 12). The mesário chose "Não testar": allowed only before the eve of the zerésima
// date (cfg +544 - 1 day); otherwise stay on the question (CBase then logs nothing).
comum::CAppState* CPreZeresima::GetEstadoSemTeste()
{
    if (m_agora.GetDate().Compare(VesperaZeresima()) < 0)
        return GetEstadoPassouNoTeste();              // direct (non-virtual) call in the binary
    return this;
}

} // namespace vota::testeteclado
