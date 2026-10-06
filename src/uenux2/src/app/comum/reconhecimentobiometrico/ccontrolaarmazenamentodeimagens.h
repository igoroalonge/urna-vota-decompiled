// uenux2/src/app/comum/reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.h
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// comum::CControlaArmazenamentoDeImagens = storage of fingerprint images (WSQ) captured at the urna
// (voters enabled by biometrics, voters not recognised, poll workers "mesários"): each image is
// encrypted with CEPESC under the TSE public key "wsq.pk1", wrapped in an EntidadeEnvelopeGenerico
// (tipoEnvelope = imagem de biometria) and written under a random name to the internal flash, then
// copied to the external one. The files end up in the result packages wsqbio.jez / wsqman.jez / wsqmes.jez.
//
// Static-only class: no RTTI, no instance. Only one out-of-line function exists (wasm 2725 = Armazena);
// GerarCaminhosUnicos (line 73), LeChavePublica (117..135) and CifrarWsq (149..152) are inlined into it.
// The tools named 2725 "LeChavePublica" after one of those inlined srclocs.
#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

#include "comum/comumtypes.h"                                    // uebyte   (header name ?)
#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/api/security/cepesc/ccipheredout.hpp"

namespace comum {

// CBaseError<EUeComumReconhecimentoBiometricoError, SErrorLimits{9000, 9050}> (typeinfo @1595996,
// vtable @1596080, constructor thunk = wasm 5369). Codes: 9000 key file missing, 9001 key empty.
enum class EUeComumReconhecimentoBiometricoError : int {};
using CUeComumReconhecimentoBiometricoError =
    ecourna::api::exception::CBaseError<EUeComumReconhecimentoBiometricoError,
                                        ecourna::api::exception::SErrorLimits{9000, 9050}>;

class CControlaArmazenamentoDeImagens {
public:
    // Returned instead of an id when nothing was stored (ids are random % 999999, i.e. 0..999998).
    static constexpr std::uint32_t ID_NAO_ARMAZENADA = 999999;                     // name inferred

    // wasm 2725. Returns the id NNNNNN of "<prefixo>NNNNNN.wsq", or ID_NAO_ARMAZENADA.
    // Callers: vota::CMostraEleitorVotando::SalvaHabilitacaoEleitor (10425, prefix "") and
    // comum::CPedeDigitalMesario::GetControlador (5382, prefix "me", static std::string @543548).
    static std::uint32_t Armazena(const std::vector<uebyte>& wsq, const std::string& dirInterno,
                                  const std::string& dirExterno, const std::string& prefixo);   // name inferred

private:
    static std::tuple<std::uint32_t, std::string, std::string>
    GerarCaminhosUnicos(const std::string& dirInterno, const std::string& dirExterno,
                        const std::string& prefixo);                                 // line 73 (inlined)
    static std::vector<uebyte> LeChavePublica();                                     // lines 117..135 (inlined)
    static ecourna::api::cepesc::CCipheredOut CifrarWsq(const std::vector<uebyte>& wsq);   // 149..152 (inlined)
};

} // namespace comum
