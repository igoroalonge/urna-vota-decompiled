// uenux2/src/app/comum/dados/asn/eleitor/cconversorentidadeeleitores.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include <vector>

#include "comum/asn/iconversorparcialasn.h"
#include "comum/dados/asn/eleitor/cvisitanteeleitor.h"
#include "comum/dados/md/eleitor/celeitor.h"
#include "comum/dados/md/eleitor/centidadeeleitores.h"
#include "ModuloEleitores.h"

namespace comum::asn {

// Free function (srcloc signature "std::vector<md::CEleitorIdentidade> comum::asn::GetInscricoesEleitor(
// ModuloEleitores::EleitorSequencia)" — note: the SEQUENCE is taken BY VALUE, i.e. copied per voter).
std::vector<md::CEleitorIdentidade> GetInscricoesEleitor(ModuloEleitores::EleitorSequencia eleitor);   // inlined in 11411

// RTTI: CConversorEntidadeEleitores
//         : IConversorParcialASN<ModuloEleitores::EntidadeEleitores, md::CEntidadeEleitores, CVisitanteEleitor>
// vtable @1566776: [0] 174 [1] 144 [2] 11411 DoDesconverte(const TEntidade&, const TVisitor&)
// The partial converter has no "Converte" direction: the urna never writes a voter file.
//
// md::CEntidadeEleitores (60 bytes): +0 md::CCabecalhoEntidade (20) +20 municipio +24 short zona +26 short secao
//   +28 std::optional<md::CSeguranca> (CSeguranca = {2 x uebyte; std::vector<uebyte> chave}, flag +44)
//   +48 std::vector<md::CEleitor>
// md::CEleitor (104 bytes), from its two constructors (funcs 5666 / 5668):
//   +0 sequencial  +4 ushort secao  +8 std::vector<CEleitorIdentidade> identidades (16-byte {string numero; int tipo})
//   +20 ENecessidadeEspecial  +24 std::string nome  +36 std::string nomeSocial
//   +48 CTransferenciaTemporaria {+48 ETransferenciaTemporaria tipo; +52 std::string uf; +64 int codigoMunicipio}
//   +68 CDataJE dataNascimento ("YYYYMMDD")  +80 std::vector<uebyte> chaveArquivo (Seguranca.idArquivoChave)
//   +92 std::optional<std::pair<size_t,size_t>> posicaoBiometria (flag +100)
class CConversorEntidadeEleitores
    : public IConversorParcialASN<ModuloEleitores::EntidadeEleitores, md::CEntidadeEleitores, CVisitanteEleitor>
{
public:
    static md::CEleitor::ENecessidadeEspecial ConverteNecessidadeEspecial(ModuloEleitores::EleitorUrna eleitor);   // inlined
    md::CTransferenciaTemporaria::ETransferenciaTemporaria
    DesconverteTipoTransferenciaTemporaria(const ModuloEleitores::EleitorSequencia& eleitor) const;               // inlined
    md::CTransferenciaTemporaria DesconverteTransferenciaTemporaria(const ModuloEleitores::EleitorSequencia& eleitor) const;   // inlined

protected:
    TDado DoDesconverte(const TEntidade& entidade, const TVisitor& visitante) const override;   // wasm func 11411
};

} // namespace comum::asn
