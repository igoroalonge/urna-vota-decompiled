// FRAGMENT reconstructed by unit u09 from vota_web_wasm.wasm.
// Original file (path inferred): uenux2/src/app/vota/comum/crelvotautil.cpp
// vota::CRelVotaUtil: helpers of the VOTA reports (RTTI only through its lambdas:
// std::function<CRelVotaUtil::CortaPapel()::$_0> and <CRelVotaUtil::CriaCabecalho(const std::string&)::$_0>).
// CortaPapel (func 2882) belongs to another unit.

#include "vota/comum/crelvotautil.h"

#include <functional>
#include <string>

#include "api/gui/cpaperformbuilder.h"
#include "api/util/cdatetime.h"
#include "comum/cappinfo.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/clocal.h"
#include "comum/relatorios/crelutil.h"

namespace vota {

// wasm func 5977 — header of the zerésima and of its summary (callers: funcs 11943, 11946).
api::SharedPaperForm CRelVotaUtil::CriaCabecalho(const std::string& titulo)
{
    const auto& estadoGeral = comum::CAppInfo::GetInst().GetEstadoGeral();       // func 291
    const api::CDateTime agora;                                                  // api_f479 (emission time)
    auto& local = comum::CLocal::GetInst();

    api::CPaperFormBuilder b;
    b.AddNewLine(2);
    comum::CRelUtil::IncluiCabecalhoEleicoesMZS(b, titulo, estadoGeral.GetMunicipio() /*+20*/,
        estadoGeral.GetZona() /*+24*/, estadoGeral.GetSecao() /*+26*/, local.GetNomeMunicipio(), 0, false);  // func 1543
    // "Eleitores aptos {:04}" (+ "Originais/Temporários na seção" lines): func 1921 over the lambda
    // CriaCabecalho::$_0 returning comum::SQtdeAptos (vtable @1543304)
    const std::function<comum::SQtdeAptos()> aptos = [] { return comum::CEleitores::GetInst().GetQtdAptos(); };
    b.AddText(comum::CRelUtil::FormataQtdAptos(aptos()), 1, 0);                 // funcs 1922/1921  name inferred
    b.AddNewLine(1);
    b.AddData(&DS_CodigoIdentificacaoUE, 0);                                     // slot 1567 -> func 1942
    comum::CRelUtil::IncluiDataHora(b, agora, "Data", "Hora");                   // func 1542
    b.AddNewLine(1);
    comum::CRelUtil::IncluiResumoCorrespondencia(b, estadoGeral.GetCorrespondencia());   // func 1541 (+60)
    return b.Build();
}

}  // namespace vota
