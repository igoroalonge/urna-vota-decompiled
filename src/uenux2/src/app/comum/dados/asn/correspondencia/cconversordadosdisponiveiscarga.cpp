// uenux2/src/app/comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.h"

#include <string>
#include <vector>

#include "comum/asn/cconversorcabecalhopacote.h"
#include "comum/asn/util.h"
#include "comum/dados/asn/cconversorsecaoeleitoral.h"
#include "comum/dados/asn/municipiozona/cconversorcomplementomunicipio.h"
#include "comum/dados/asn/municipiozona/cconversormunicipio.h"
#include "comum/dados/md/cinfomunicipio.h"

namespace comum::asn {

// wasm func 11424 (vtable slot 2). The tool named it after the inlined IConversorASN<CabecalhoPacote>::Converte
// (iconversorasn.h:56, srcloc record 1562828). Not observed executing.
ModuloDadosDisponiveisCarga::DadosDisponiveisCarga
CConversorDadosDisponiveisCarga::DoConverte(const md::CDadosDisponiveisCarga& dados) const
{
    ModuloDadosDisponiveisCarga::DadosDisponiveisCarga entidade;
    entidade.set_fase(Utils::ConverteFase(dados.GetFase()));
    entidade.set_idPE(dados.GetIdPE());                                // Constrained_INTEGER<0, 99999> temporary
    entidade.set_idPleito(dados.GetIdPleito());
    entidade.set_serialFlashCarga(dados.GetSerialFlashCarga());
    entidade.set_pais(dados.GetPais());
    ModuloTiposEleitorais::UF uf;
    uf.set_sigla(dados.GetSiglaUF());
    uf.set_nome(dados.GetNomeUF());
    entidade.set_uf(uf);
    entidade.set_zona(dados.GetZona());

    const CConversorComplementoMunicipio conversorComplemento;
    const CConversorMunicipio conversorMunicipio;
    const CConversorSecaoEleitoral conversorSecao;
    ASN1::SEQUENCE_OF<ModuloDadosDisponiveisCarga::MunicipioDisponivel> municipios;
    for (const md::CMunicipioDisponivel& disponivel : dados.GetMunicipios()) {
        const md::CInfoMunicipio& info = disponivel;                   // CMunicipioDisponivel derives from / starts
                                                                       // with CInfoMunicipio   // ?
        ModuloDadosDisponiveisCarga::MunicipioDisponivel entidadeMunicipio;
        entidadeMunicipio.set_municipio(conversorMunicipio.Converte(
            md::CMunicipio(info.GetCodigo(), info.GetNome(), info.GetComBiometria())));   // func 3712
        entidadeMunicipio.ref_municipio().set_comBiometria(info.GetComBiometria());       // set again (redundant)

        md::CComplementoMunicipio complemento(info.GetCodigo(), info.GetFuso());           // func 3711
        if (info.GetHorarioVerao().has_value()) {
            complemento = md::CComplementoMunicipio(info.GetCodigo(), info.GetFuso(),
                                                    *info.GetHorarioVerao());              // func 3710
        }
        entidadeMunicipio.set_complementoMunicipio(conversorComplemento.Converte(complemento));   // thunk 3736

        ASN1::SEQUENCE_OF<ModuloLocal::SecaoEleitoral> secoes;
        for (const md::CSecaoEleitoral& secao : disponivel.GetSecoes()) {
            secoes.push_back(conversorSecao.Converte(secao));          // thunk 5713 (clone + push_back)
        }
        entidadeMunicipio.set_secoes(secoes);                          // copy-and-swap assignment
        municipios.push_back(entidadeMunicipio);
    }
    entidade.set_municipios(municipios);

    const CConversorCabecalhoPacote conversorPacote;
    ASN1::SEQUENCE_OF<ModuloTiposEleitorais::CabecalhoPacote> pacotes;
    for (const md::CCabecalhoPacote& pacote : dados.GetPacotesImportados()) {
        pacotes.push_back(conversorPacote.Converte(pacote));           // Converte inlined (:56, code 7653)
    }
    entidade.set_pacotesImportados(pacotes);
    return entidade;
}

// wasm func 11423 (vtable slot 3; 5864 bytes). Contains the inlined md::CDadosDisponiveisCarga constructor
// (cdadosdisponiveiscarga.cpp:41..69, EUeComumDadosError 8040..8048). Not observed executing in the recorded votes.
md::CDadosDisponiveisCarga
CConversorDadosDisponiveisCarga::DoDesconverte(const ModuloDadosDisponiveisCarga::DadosDisponiveisCarga& dados) const
{
    const EUrnaFase fase = Utils::DesconverteFase(dados.get_fase());   // func 2843
    const TPleitoID pleito = dados.get_idPleito();
    const TProcessoEleitoralID processo = dados.get_idPE();
    const std::string pais = dados.get_pais();
    const std::string serial = dados.get_serialFlashCarga();
    const std::string siglaUF = dados.get_uf().get_sigla();
    const std::string nomeUF = dados.get_uf().get_nome();
    const TZonaID zona = dados.get_zona();

    const CConversorMunicipio conversorMunicipio;
    const CConversorComplementoMunicipio conversorComplemento;
    const CConversorSecaoEleitoral conversorSecao;
    std::vector<md::CMunicipioDisponivel> municipios;
    for (const auto* item : dados.get_municipios()) {
        const md::CMunicipio municipio = conversorMunicipio.Desconverte(item->get_municipio());               // 5716
        const md::CComplementoMunicipio complemento =
            conversorComplemento.Desconverte(item->get_complementoMunicipio());                            // 3737
        // Municipio.comBiometria is "[2] BOOLEAN OPTIONAL" (optional index 1): absent means false.
        const bool comBiometria = item->get_municipio().comBiometria_isPresent()
                                  && item->get_municipio().get_comBiometria();
        md::CInfoMunicipio info(municipio.GetCodigo(), municipio.GetNome(), complemento.GetFuso(),
                                comBiometria);                                                             // func 5676
        if (complemento.GetHorarioVerao().has_value()) {                                                  // +28 flag
            info = md::CInfoMunicipio(municipio.GetCodigo(), municipio.GetNome(), complemento.GetFuso(),
                                      comBiometria, complemento.GetHorarioVerao().value());               // func 5674
        }
        std::vector<md::CSecaoEleitoral> secoes;
        for (const auto* secao : item->get_secoes()) {
            secoes.push_back(conversorSecao.Desconverte(*secao));                                          // 5715
        }
        municipios.push_back(md::CMunicipioDisponivel(info, secoes));                  // vector copy: func 2807
    }

    std::vector<md::CCabecalhoPacote> pacotes;
    const CConversorCabecalhoPacote conversorPacote;
    for (const auto* pacote : dados.get_pacotesImportados()) {
        pacotes.push_back(conversorPacote.Desconverte(*pacote));                                           // 5705
    }

    // Inlined constructor, checks in this order (all CUeComumDadosError):
    //   fase == '0'          8040 "Fase inválida."                      (:41) (unreachable: DesconverteFase)
    //   processo == 0        8041 "Processo eleitoral inválido."         (:44)
    //   pleito == 0          8042 "Pleito inválido."                     (:49)
    //   serial.size() != 8   8043 "Serial da mídia de carga inválido."   (:52)
    //   pais.empty()         8044 "País em branco."                      (:57)
    //   siglaUF.empty()      8045 "Sigla da uf em branco."               (:60)
    //   nomeUF.empty()       8046 "Nome da uf em branco."                (:63)
    //   zona == 0            8047 "Zona inválida."                       (:66)
    //   municipios.empty()   8048 "Deve existir no mínimo um município." (:69)
    return md::CDadosDisponiveisCarga(fase, processo, pleito, serial, pais, siglaUF, nomeUF, zona, municipios, pacotes);
}

} // namespace comum::asn
