// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/reports/csubreport.cpp (path inferred). Method names inferred.
#include "api/gui/reports/csubreport.h"

namespace api {

// wasm func 12014 (slot 0): thunk into shared_f3904 (release m_cabecalho, free m_via, free m_nome).
// wasm func 12013 (slot 1): the same, inlined, + operator delete.
CSubReport::~CSubReport() = default;

// wasm func 12012 (slot 3)
std::string CSubReport::GetVia() const
{
    return m_via;
}

// wasm func 12011 (slot 4)
bool CSubReport::TemNome() const
{
    return !m_nome.empty();
}

// wasm func 12010 (slot 5)
bool CSubReport::TemCabecalho() const
{
    return m_cabecalho != nullptr;
}

// wasm func 12009 (slot 6) - calls slot 2 of the part (IReportPart::Imprime)
void CSubReport::ImprimeCabecalho() const
{
    m_cabecalho->Imprime();
}

// wasm func 12008 (slot 7)
bool CSubReport::EhRelatorio() const
{
    return m_relatorio;
}

} // namespace api
