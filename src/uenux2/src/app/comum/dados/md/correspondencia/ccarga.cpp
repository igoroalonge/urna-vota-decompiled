// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/correspondencia/ccarga.cpp
//
// CCarga = identification of the "carga" (the loading of the election media into this urna),
// ModuloTiposEcoUrna::Carga. It is copied into every result file (BU, RDV, ...) through
// CCorrespondenciaResultado so that each file can be matched to the urna that produced it.
// Layout (76 bytes, copy = func 1557): +0 numeroInternoUrna, +4 serial da MC/FC (8 hex chars),
//   +16 dataHoraCarga ?, +28 codigoCarga (24 decimal digits), +40/+52/+64 identificador do
//   gerador de mídia (nome, serialCertificadoTPM, serialInstalacao) ?
#include "ccarga.h"

#include <algorithm>
#include <cctype>

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 5670 (srclocs lines 36, 41, 46, 51). Called from the CCarga constructor (comum_f2802).
void CCarga::ValidaCriacao() const
{
    if (m_serialMC.size() != 8)
        throw CUeComumDadosError(8019, "Serial da MC inválido [" + m_serialMC + "]");        // line 36
    if (m_serialMC.find_first_not_of("0123456789ABCDEFabcdef") != std::string::npos)
        throw CUeComumDadosError(8020, "Serial da MC inválido [" + m_serialMC + "]");        // line 41
    if (m_codigoCarga.size() != 24)
        throw CUeComumDadosError(8021, "Código da carga inválido [" + m_codigoCarga + "]");  // line 46
    if (!std::all_of(m_codigoCarga.begin(), m_codigoCarga.end(),
                     [](char c) { return c >= '0' && c <= '9'; }))
        throw CUeComumDadosError(8022, "Código da carga inválido [" + m_codigoCarga + "]");  // line 51
}

}  // namespace comum::md
