// uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp  --  FRAGMENT written by unit u36 (file owned by
// u02/u07). Reconstructed from vota_web_wasm.wasm.
//
// Placement: the only caller is wasm 6737 (CInformacaoEleitor::GerarDadosDinamicos, name inferred by u02),
// which calls it twice (training branch and normal branch), right after the RDV was (re)created.  (path ?)
#include "vota/eleitor/comum/cinformacaoeleitor.h"

#include "comum/appinfo/cappinfo.h"
#include "comum/iinterfacesavd.h"
#include "vota/comum/cassinadorvota.h"             // vota::CAssinadorVota : comum::CAssinador   (header ?)

namespace vota {

namespace {

// wasm func 4668 - observed executing (votaInit)                                        // name inferred
// Asks SAVD to sign the two dynamic files that exist before the first vote:
//   id 83  "RDV MI"       dinamico/trab<turno>/rdv.dat   (registro digital do voto)
//   id 110 "UENUXDB INT"  dinamico/trab<turno>/uenux.db  (SQLite: mesários, justificativas, ...)
// with the signing package 122 in the 1st turno and 123 otherwise (vota::CAssinadorVota ctor = wasm 1501,
// aplicação 1 = VOTA). Same pattern as comum::GravaBancoDadosNaMI (4657), without the fsync/copy.
// Web build: IInterfaceSavd is (anonymous)::CWasmSavd, which answers "OK" without doing anything, so no
// .vsu is produced here; votaInit writes the fake "assinatura simulada para vota_web_wasm" files itself.
void AssinaDadosDinamicos()
{
    const bool primeiroTurno = comum::CAppInfo::GetInst().GetGeral().GetTurno() == '1';   // CEstadoGeral +32
    const CAssinadorVota assinador(primeiroTurno ? 122 : 123);                          // wasm 1501
    assinador.Assina(comum::ESavdArquivoUE{83});          // wasm 1277 = CAssinador::Assina (cassinador.cpp:114)
    assinador.Assina(comum::ESavdArquivoUE{110});
}                                                                                        // ~CAssinador (1007)

}  // namespace

}  // namespace vota
