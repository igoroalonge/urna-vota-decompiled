// ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   ParametrosUrna (47 fields, see src/asn1/ModuloParametrizacaoUrna.asn)  <->  CParametrosUrna
// DoDeconverte (func 9156) and DesconverterTitulos (func 5102) were observed executing during the
// recorded votes: VOTA's start-up function (func 7787) reads t00000br-pu.dat / t<pleito><uf>-pu.dat.
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// The two title-list helpers are const member functions, not file-local free functions: their wasm
// signature is (sret, this, list) with the middle argument never read, and their callers pass a
// junk value for it (9156 passes its stack frame, 9164 passes its frame too), exactly as 9164 does
// for the srcloc-named members ConverterFormaSuspender{Sem,Com}Voto. That is LLVM dead-argument
// elimination replacing an unused `this` by poison at the call sites of an externally visible
// function; a function in an anonymous namespace (internal linkage) would have lost the parameter.

// wasm func 5102 (no srcloc; name inferred). Index loop over the SEQUENCE OF, one
// CConversorTituloRelatorio::Deconverte per element, emplace_back (slow path func 9152).
// On a throw the partial vector is destroyed (func 1673).
TVectorTituloRelatorio CConversorParametrosUrna::DesconverterTitulos(
    const ASN1::SEQUENCE_OF<ModuloParametrizacaoUrna::TituloRelatorio>& titulos) const
{
    const CConversorTituloRelatorio conversor;
    TVectorTituloRelatorio resultado;
    for (std::size_t i = 0; i < titulos.size(); ++i) {
        resultado.push_back(conversor.Deconverte(titulos[i]));
    }
    return resultado;
}

// wasm func 9161 (no srcloc; name inferred). The inverse, into a temporary SEQUENCE OF (each
// element converted then cloned into the container). The caller then copies it into the field
// through the generated setter, which builds SEQUENCE_OF<TituloRelatorio>(first, last)
// (func 9160 -> func 2926) and swaps it in.
ASN1::SEQUENCE_OF<ModuloParametrizacaoUrna::TituloRelatorio>
CConversorParametrosUrna::ConverterTitulos(const TVectorTituloRelatorio& titulos) const
{
    const CConversorTituloRelatorio conversor;
    ASN1::SEQUENCE_OF<ModuloParametrizacaoUrna::TituloRelatorio> resultado;
    for (std::size_t i = 0; i < titulos.size(); ++i) {
        resultado.push_back(conversor.Converte(titulos[i]));
    }
    return resultado;
}

// wasm func 9162 (srcloc line 147): thunk into the shared body func 2306 (count 3): 0-based -> 1-based.
ModuloParametrizacaoUrna::FormaSuspenderComVoto
CConversorParametrosUrna::ConverterFormaSuspenderComVoto(CParametrosUrna::EFormaSuspenderComVoto forma) const
{
    if (static_cast<unsigned>(forma) >= 3) {
        throw CAsnParametrizacaoUrnaError(2566, "Forma de suspender com voto inválido.");   // line 147
    }
    return ModuloParametrizacaoUrna::FormaSuspenderComVoto(
        static_cast<ModuloParametrizacaoUrna::FormaSuspenderComVoto::NamedNumber>(forma + 1));
}

// wasm func 9163 (srcloc line 183): thunk into func 2306.
ModuloParametrizacaoUrna::FormaSuspenderSemVoto
CConversorParametrosUrna::ConverterFormaSuspenderSemVoto(CParametrosUrna::EFormaSuspenderSemVoto forma) const
{
    if (static_cast<unsigned>(forma) >= 3) {
        throw CAsnParametrizacaoUrnaError(2569, "Forma de suspender sem voto inválido.");   // line 183
    }
    return ModuloParametrizacaoUrna::FormaSuspenderSemVoto(
        static_cast<ModuloParametrizacaoUrna::FormaSuspenderSemVoto::NamedNumber>(forma + 1));
}

// inlined into func 9156 (srcloc lines 163, 167)
CParametrosUrna::EFormaSuspenderComVoto CConversorParametrosUrna::DesconverterFormaSuspenderComVoto(const TEntidade& p) const
{
    switch (p.get_formaSuspensaoComVoto().asInt()) {
    case ModuloParametrizacaoUrna::FormaSuspenderComVoto::tornaOutrosBranco:    return CParametrosUrna::TornaOutrosBranco;
    case ModuloParametrizacaoUrna::FormaSuspenderComVoto::tornaOutrosNulo:      return CParametrosUrna::TornaOutrosNulo;
    case ModuloParametrizacaoUrna::FormaSuspenderComVoto::tornaNaoVotouParcial: return CParametrosUrna::TornaNaoVotouParcial;
    case -1:   // ? explicit "invalid" enumerator
        throw CAsnParametrizacaoUrnaError(2567, "Forma de suspender com voto inválido.");   // line 163
    }
    throw CAsnParametrizacaoUrnaError(2568, "Forma de suspender com voto inválido.");       // line 167
}

// inlined into func 9156 (srcloc lines 199, 203)
CParametrosUrna::EFormaSuspenderSemVoto CConversorParametrosUrna::DesconverterFormaSuspenderSemVoto(const TEntidade& p) const
{
    switch (p.get_formaSuspensaoSemVoto().asInt()) {
    case ModuloParametrizacaoUrna::FormaSuspenderSemVoto::tornaNaoVotou:    return CParametrosUrna::TornaNaoVotou;
    case ModuloParametrizacaoUrna::FormaSuspenderSemVoto::tornaTodosBranco: return CParametrosUrna::TornaTodosBranco;
    case ModuloParametrizacaoUrna::FormaSuspenderSemVoto::tornaTodosNulo:   return CParametrosUrna::TornaTodosNulo;
    case -1:   // ? explicit "invalid" enumerator
        throw CAsnParametrizacaoUrnaError(2570, "Forma de suspender sem voto inválido.");   // line 199
    }
    throw CAsnParametrizacaoUrnaError(2571, "Forma de suspender sem voto inválido.");       // line 203
}

// wasm func 9164 (vtable slot 2). No range check of its own: the ASN constraints are checked by
// IConversorASN::Converte on the result.
CConversorParametrosUrna::TEntidade CConversorParametrosUrna::DoConverte(const TDado& p) const
{
    const CConversorLabelParametrizado conversorLabel;
    ModuloParametrizacaoUrna::ParametrosUrna e;
    e.set_formaSuspensaoSemVoto(ConverterFormaSuspenderSemVoto(p.m_formaSuspensaoSemVoto));
    e.set_formaSuspensaoComVoto(ConverterFormaSuspenderComVoto(p.m_formaSuspensaoComVoto));
    e.set_numZeresimas(p.m_numZeresimas);
    e.set_numBUVotaObrigatorios(p.m_numBUVotaObrigatorios);
    e.set_numBUVotaAdicionais(p.m_numBUVotaAdicionais);
    e.set_numBUFinalREDObrigat(p.m_numBUFinalREDObrigat);
    e.set_numBUFinalREDAdic(p.m_numBUFinalREDAdic);
    e.set_numBUParcialREDObrigat(p.m_numBUParcialREDObrigat);
    e.set_numBUParcialREDAdic(p.m_numBUParcialREDAdic);
    e.set_numBUFinalSAObrigat(p.m_numBUFinalSAObrigat);
    e.set_numBUFinalSAAdic(p.m_numBUFinalSAAdic);
    e.set_numBUParcialSAObrigat(p.m_numBUParcialSAObrigat);
    e.set_numTentativasHabilitacao(p.m_numTentativasHabilitacao);
    e.set_numTentativasVerificacao(p.m_numTentativasVerificacao);
    e.set_numRelatorioEstado(p.m_numRelatorioEstado);
    e.set_numRelatorioEleitores(p.m_numRelatorioEleitores);
    e.set_numRelatorioVersoesDados(p.m_numRelatorioVersoesDados);
    e.set_numRelatorioPU(p.m_numRelatorioPU);
    e.set_criptografarBU(p.m_criptografarBU);
    e.set_criptografarJUFA(p.m_criptografarJUFA);
    e.set_numDigitosPartido(p.m_numDigitosPartido);
    e.set_tempoDispararSuspensao(p.m_tempoDispararSuspensao);
    e.set_tempoDispararSuspensaoTE(p.m_tempoDispararSuspensaoTE);
    e.set_tempoConfirmacaoVoto(p.m_tempoConfirmacaoVoto);
    e.set_tempoDesligamentoAutomatico(p.m_tempoDesligamentoAutomatico);
    e.set_tempoAvisoDesligamentoAutomatico(p.m_tempoAvisoDesligamentoAutomatico);
    e.set_cabecalho(ConverterTitulos(p.m_cabecalho));
    e.set_rodapeZEVOTA(ConverterTitulos(p.m_rodapeZEVOTA));
    e.set_rodapeBUVOTA(ConverterTitulos(p.m_rodapeBUVOTA));
    e.set_rodapeZESA(ConverterTitulos(p.m_rodapeZESA));
    e.set_rodapeBUSA(ConverterTitulos(p.m_rodapeBUSA));
    e.set_rodapeBUParcial(ConverterTitulos(p.m_rodapeBUParcial));
    e.set_telaEmissaoBU(p.m_telaEmissaoBU);
    e.set_labelMunicipio(conversorLabel.Converte(p.m_labelMunicipio));
    e.set_labelZona(conversorLabel.Converte(p.m_labelZona));
    e.set_labelSecao(conversorLabel.Converte(p.m_labelSecao));
    e.set_labelPartido(conversorLabel.Converte(p.m_labelPartido));
    e.set_imprimirZeradosBU(p.m_imprimirZeradosBU);
    e.set_gravarZeradosBU(p.m_gravarZeradosBU);
    e.set_aceitarJustificativa(p.m_aceitarJustificativa);
    e.set_aceitarBrancoNulo(p.m_aceitarBrancoNulo);
    e.set_apresentarPartido(p.m_apresentarPartido);
    e.set_permitirHabManualAudio(p.m_permitirHabManualAudio);
    e.set_imprimirQrCodeNoBU(p.m_imprimirQrCodeNoBU);
    e.set_exibirScoreBiometria(p.m_exibirScoreBiometria);
    e.set_registrarMesarios(p.m_registrarMesarios);
    e.set_pedeAnoNascimentoEleitor(p.m_pedeAnoNascimentoEleitor);
    return e;
}

// wasm func 9156 (vtable slot 3; the tool named it after the inlined DesconverterFormaSuspenderComVoto).
// Evaluation order in the binary: sem-voto rule, com-voto rule, the scalars, the six title lists,
// telaEmissaoBU, the four labels, the ten booleans; then the 47-argument constructor (func 5093).
CConversorParametrosUrna::TDado CConversorParametrosUrna::DoDeconverte(const TEntidade& p) const
{
    const CConversorLabelParametrizado conversorLabel;
    const auto semVoto = DesconverterFormaSuspenderSemVoto(p);
    const auto comVoto = DesconverterFormaSuspenderComVoto(p);
    return CParametrosUrna(
        semVoto, comVoto, p.get_numZeresimas(), p.get_numBUVotaObrigatorios(), p.get_numBUVotaAdicionais(),
        p.get_numBUFinalREDObrigat(), p.get_numBUFinalREDAdic(), p.get_numBUParcialREDObrigat(),
        p.get_numBUParcialREDAdic(), p.get_numBUFinalSAObrigat(), p.get_numBUFinalSAAdic(),
        p.get_numBUParcialSAObrigat(), p.get_numTentativasHabilitacao(), p.get_numTentativasVerificacao(),
        p.get_numRelatorioEstado(), p.get_numRelatorioEleitores(), p.get_numRelatorioVersoesDados(),
        p.get_numRelatorioPU(), p.get_criptografarBU(), p.get_criptografarJUFA(), p.get_numDigitosPartido(),
        p.get_tempoDispararSuspensao(), p.get_tempoDispararSuspensaoTE(), p.get_tempoConfirmacaoVoto(),
        p.get_tempoDesligamentoAutomatico(), p.get_tempoAvisoDesligamentoAutomatico(),
        DesconverterTitulos(p.get_cabecalho()), DesconverterTitulos(p.get_rodapeZEVOTA()),
        DesconverterTitulos(p.get_rodapeBUVOTA()), DesconverterTitulos(p.get_rodapeZESA()),
        DesconverterTitulos(p.get_rodapeBUSA()), DesconverterTitulos(p.get_rodapeBUParcial()),
        p.get_telaEmissaoBU(),
        conversorLabel.Deconverte(p.get_labelMunicipio()), conversorLabel.Deconverte(p.get_labelZona()),
        conversorLabel.Deconverte(p.get_labelSecao()), conversorLabel.Deconverte(p.get_labelPartido()),
        p.get_imprimirZeradosBU(), p.get_gravarZeradosBU(), p.get_aceitarJustificativa(),
        p.get_aceitarBrancoNulo(), p.get_apresentarPartido(), p.get_permitirHabManualAudio(),
        p.get_imprimirQrCodeNoBU(), p.get_exibirScoreBiometria(), p.get_registrarMesarios(),
        p.get_pedeAnoNascimentoEleitor());
}

} // namespace ecourna::app::dados::asn
