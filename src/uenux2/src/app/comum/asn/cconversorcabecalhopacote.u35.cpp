// FRAGMENT of uenux2/src/app/comum/asn/cconversorcabecalhopacote.cpp (path inferred; file of unit u21, which wrote
// DoConverte = func 11438 and the class declaration in cconversorcabecalhopacote.h).
// Reconstructed from vota_web_wasm.wasm (unit u35: DoDesconverte = func 11437).
//
// The "*.pid" file next to every data package of the election (t02400-cp.pid, t02411ac-ce.pid, ...) and each
// entry of dadoscarga.dat (DadosDisponiveisCarga) is a CabecalhoPacote:
//   { tipoPacote TipoPacote, idPacote IDPacote, nomepacote GeneralString, versao NumericString (SIZE(12)),
//     abrangencia [1] Abrangencia OPTIONAL, origem Sistema }
//   IDPacote ::= { idPacoteEleitoral IDEleitoral, fase Fase, siglaUF OPTIONAL, codigoMunicipio OPTIONAL,
//                  numeroZona OPTIONAL }
// Everything below is inlined into the single function; the srclocs give the original functions and lines. The md
// constructors/validators are written out in src/uenux2/src/app/comum/md/ccabecalhopacote.u35.cpp, the Utils
// functions in comum/asn/util.u35.cpp.
#include "comum/asn/cconversorcabecalhopacote.h"

#include "comum/asn/util.h"
#include "comum/md/ccabecalhopacote.h"
#include "comum/md/cidpacote.h"

namespace comum::asn {

// wasm func 11437 - vtable slot 3. Not observed executing in the recorded votes (the .pid files are read by
// CPE/LePleito at start-up - funcs 5461 / 5706 - and by the "Versões de pacotes" report, vota::CImpressaoVersao-
// Pacotes 11905, through dadoscarga.dat).
md::CCabecalhoPacote CConversorCabecalhoPacote::DoDesconverte(const ModuloTiposEleitorais::CabecalhoPacote& cabecalho) const
{
    const md::ETipoPacote tipo = Utils::DesconverteTipoPacote(cabecalho.get_tipoPacote());         // util.cpp:329

    // ---- IDPacote -> md::CIDPacote (constructors of cidpacote.cpp inlined; :21 = CIDPacoteValidar(fase)) ------
    const auto& id = cabecalho.get_idPacote();
    const md::CIDEleitoral idEleitoral = Utils::DesconverteIdEleitoral(id.get_idPacoteEleitoral()); // util.cpp:89
    const EUrnaFase fase = Utils::DesconverteFase(id.get_fase());                                   // func 2843
    md::CIDPacote idPacote = [&] {
        if (!id.hasOptionalField(0))                                   // no siglaUF
            return md::CIDPacote(idEleitoral, fase);
        const std::string uf = id.get_siglaUF();
        if (!id.hasOptionalField(1))                                   // no codigoMunicipio
            return md::CIDPacote(idEleitoral, fase, uf);
        const TMunicipioID municipio = id.get_codigoMunicipio();
        if (!id.hasOptionalField(2))                                   // no numeroZona
            return md::CIDPacote(idEleitoral, fase, uf, municipio);
        return md::CIDPacote(idEleitoral, fase, uf, municipio, static_cast<TZonaID>(id.get_numeroZona()));
    }();
    // Every CIDPacote constructor starts with CIDPacoteValidar(fase) (cidpacote.cpp:21):
    //     if (fase == '0' || fase == '4') throw CUeComumMdError(8921, "Fase inválida.");
    // (the test is (fase & ~4) == '0'; DesconverteFase can only return '1'..'3', so it never fires here.)

    const std::string nome = cabecalho.get_nomepacote();
    const std::string versao = cabecalho.get_versao();
    const ESistemaJE origem = Utils::DesconverteIdSistema(cabecalho.get_origem());                 // util.cpp:451
    // The optional abrangencia ([1]) is not read: md::CCabecalhoPacote has no such member.

    // md::CCabecalhoPacote::CCabecalhoPacote (ccabecalhopacote.cpp) inlined:
    //   :33 nome.empty()          -> CUeComumMdError 8908 "Nome do pacote vazio."
    //   :36 versao.size() != 12   -> CUeComumMdError 8909 "Versão deve ter tamanho 12."
    //   :39 origem not in 1..12   -> CUeComumMdError 8910 "Valor de origem inválido."   (one unsigned compare:
    //                                   (unsigned)(origem - 13) <= 0xFFFFFFF3; never fires, see DesconverteIdSistema)
    return md::CCabecalhoPacote(tipo, idPacote, nome, versao, origem);
}

} // namespace comum::asn
