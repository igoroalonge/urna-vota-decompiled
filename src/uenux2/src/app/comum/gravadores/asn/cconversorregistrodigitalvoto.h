// uenux2/src/app/comum/gravadores/asn/cconversorregistrodigitalvoto.h   (path inferred; this is the include path
// already used by u05's crdvvota.h)
// Reconstructed from vota_web_wasm.wasm (unit u21: DoConverte 11483 and DoDesconverte 11482).
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "comum/asn/iconversorasn.h"
#include "comum/asn/util.h"
#include "comum/dados/md/rdv/cvotoseleicoesvota.h"
#include "ModuloRegistroDigitalVoto.h"

namespace comum::asn {

// RDV = Registro Digital do Voto (rdv.dat): the list of every vote cast, per election and office, kept sorted by
// (tipo, digitado) by comum::CRdvPosicionadorVota (func 11496), so the voting order is not recorded.
//   EntidadeRegistroDigitalVoto ::= SEQUENCE { pleito INTEGER (0..99999), fase Fase,
//        identificacao IdentificacaoSecaoEleitoral, historicoCodigosCarga SEQUENCE OF GeneralString,
//        eleicoes Eleicoes CHOICE { [1] eleicoesVota SEQUENCE OF EleicaoVota, [2] eleicoesSA SEQUENCE OF EleicaoSA } }
// The header fields are NOT part of md::CVotosEleicoesVota: they are members of this converter, set once by the
// start-up code (wasm 7787) that creates CRdvVota. CONVERSOR converts the "eleicoes" CHOICE (CConversorEleicoesVota,
// unit u03).
//
// RTTI: CConversorRegistroDigitalVoto<CConversorEleicoesVota>
//         : IConversorASN<ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto, md::CVotosEleicoesVota>
// vtable @1560304: [0] 11485 ~T  [1] 11484 deleting ~T  [2] 11483 DoConverte  [3] 11482 DoDesconverte.
// Both methods were named by the tool after the inlined IConversorASN<Eleicoes, CVotosEleicoesVota>::Converte (:56) /
// Desconverte (:71) of the member converter.
template <typename CONVERSOR>
class CConversorRegistroDigitalVoto
    : public IConversorASN<ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto, md::CVotosEleicoesVota>
{
public:
    CConversorRegistroDigitalVoto(CONVERSOR conversor, TPleitoID pleito, EUrnaFase fase, TMunicipioID municipio,
                                  TZonaID zona, TLocalID local, TSecaoID secao,
                                  std::vector<std::string> historicoCodigosCarga)          // built inline in 7787
        : m_conversor(std::move(conversor)), m_pleito(pleito), m_fase(fase), m_municipio(municipio), m_zona(zona),
          m_local(local), m_secao(secao), m_historicoCodigosCarga(std::move(historicoCodigosCarga)) {}
    ~CConversorRegistroDigitalVoto() override = default;                                   // wasm 11485 / 11484

protected:
    // wasm func 11483 (vtable slot 2). Observed executing: the (empty) RDV is serialised during votaInit
    // (CRdvVota::Converte -> rdv.dat, encrypted). Order: identification first, then the votes (checked with
    // CHOICE::isValid / CHOICE::isStrictlyValid, code 7653), then the header fields.
    TEntidade DoConverte(const TDado& votos) const override
    {
        ModuloTiposEleitorais::MunicipioZona municipioZona;
        municipioZona.set_municipio(m_municipio);
        municipioZona.set_zona(m_zona);
        ModuloTiposEleitorais::IdentificacaoSecaoEleitoral identificacao;
        identificacao.set_municipioZona(municipioZona);
        identificacao.set_local(m_local);
        identificacao.set_secao(m_secao);

        const ModuloRegistroDigitalVoto::Eleicoes eleicoes = m_conversor.Converte(votos);   // inlined (:56)

        TEntidade entidade;
        entidade.set_pleito(m_pleito);                                  // Constrained_INTEGER<0, 99999> temporary
        entidade.set_fase(Utils::ConverteFase(m_fase));
        entidade.set_identificacao(identificacao);
        ASN1::SEQUENCE_OF<ASN1::GeneralString> historico;
        for (const std::string codigo : m_historicoCodigosCarga) {    // by value: the wasm copies each string first
            historico.push_back(ASN1::GeneralString(codigo));          // AbstractString(info @1148180, std::string)
        }
        entidade.set_historicoCodigosCarga(historico);                 // wasm 5731 (SEQUENCE_OF copy-assign)
        entidade.set_eleicoes(eleicoes);                               // wasm 2188 (CHOICE assignment)
        return entidade;
    }

    // wasm func 11482 (vtable slot 3). Not observed executing in the recorded sessions (rdv.dat is read back by
    // CRdvVota::Desconverte / ConfereConteudo through IConversorASN::Desconverte(std::vector<uebyte>), wasm 5680).
    // Only "eleicoes" is converted: pleito, fase, identificacao and historicoCodigosCarga of the file are ignored.
    TDado DoDesconverte(const TEntidade& entidade) const override
    {
        return m_conversor.Desconverte(entidade.get_eleicoes());       // inlined (:71, CHOICE checks)
    }

private:
    CONVERSOR m_conversor;                                  // +4  (CConversorEleicoesVota: vptr + std::map, 16 bytes)
    TPleitoID m_pleito;                                     // +20
    EUrnaFase m_fase;                                       // +24
    TMunicipioID m_municipio;                               // +28
    TZonaID m_zona;                                         // +32 (uint16)
    TLocalID m_local;                                       // +36
    TSecaoID m_secao;                                       // +40 (uint16)
    std::vector<std::string> m_historicoCodigosCarga;       // +44 (codes of the load media used, "códigos de carga")
};

} // namespace comum::asn
