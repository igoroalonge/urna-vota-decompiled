// FRAGMENT of uenux2/src/app/comum/appinfo/cappinfo.cpp (attested; owner of the GetEstado<> helpers).
// Reconstructed from vota_web_wasm.wasm (unit u35).                                              (file inferred)
#include "comum/appinfo/cappinfo.h"

namespace comum {

// wasm func 1485 - name as used by units u07/u10/u27. Not observed executing by the sampler.
// "Fase de treinamento": the urna was loaded with training data (EUrnaFase '3'; '1' oficial, '2' simulado), read
// from eg.bin (CEstadoGeral +48). In training the urna skips some steps: vota::CGravaResultado does not encrypt the
// WSQ fingerprint packages (cifra = !EhFaseTreinamento()), CMostraEleitorVotando does not store the recognised
// fingerprint, the end-of-voting time lock (CPedeIdentidade / VotacaoBloqueadaPorHorario) is never armed.
// The simulator's scenarios are all training loads (every package name in upstream/fs starts with 't'), so this
// is always true in the web build.
bool EhFaseTreinamento()
{
    return GetEstado<md::estadoaplicacao::CEstadoGeral>(CAppInfo::GetInst()).GetFase() == EUrnaFase('3');   // 291
}

} // namespace comum
