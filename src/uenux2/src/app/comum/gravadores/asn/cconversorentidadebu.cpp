// uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// comum::asn::CConversorEntidadeBU : IConversorASN<ModuloBoletimUrna::EntidadeBoletimUrna, md::CEntidadeBU>
// (vtable @1597536). Only DoConverte (C++ -> ASN.1, slot 2, func 10273) exists: the urna never reads a BU back.
// DoConverte is where the BU's integrity data is produced: the per-election SHA-512 hash chain over every
// TotalVotosVotavel ("hash encadeado") and the signature of the last hash (CAssinaVotavelBu, cassinavotavelbu.cpp,
// whose functions were all inlined here).
//
// md::CEntidadeBU layout (read by this function; built by CGravadorBU::MontaEntidadeBU):
//   +0   CCabecalhoEntidade (+12 id, +16 tipo do id)   +20 EUrnaFase    +24 CUrna (136 bytes; CCarga @+48,
//        codigoCarga @+76)
//   +160 município  +164 zona  +168 local  +172 seção  +174 qtdComparecimento (u16)
//   +176 vector<CResultadoVotacaoPorEleicao>   +188 vector<string> historicoCodigosCarga
//   +200 optional<CDadosBUVota>{abertura +200, encerramento +212, optional desligamentoVotoImpresso +224 (flag +236)} flag +240
//   +244 optional<CDadosBUSA>{junta u16 +244, turma u16 +246, numeroInternoUrnaOrigem +248} flag +252
//   +256 CDateTime dataHoraEmissao    +268 vector<CHistoricoVotoImpresso> (20-byte items)
//   +280 optional<CDetalhamentoComparecimento>{3 x u16} flag +286
#include "comum/gravadores/asn/cconversorentidadebu.h"

#include <format>

#include "api/pattern/cpolysingletonlist.h"
#include "api/pkcs11/ipkcs11.h"
#include "comum/asn/cconversorcabecalhoentidade.h"
#include "comum/asn/util.h"                              // Utils::ConverteFase, ConverteDataHora (comum_f1080)
#include "comum/gravadores/asn/cconversorurna.h"
#include "comum/gravadores/iresultado.h"
#include "ecourna/api/security/csha512.h"
#include "ecourna/app/dados/asn/cconversordetalhamentocomparecimento.h"
#include "ModuloBoletimUrna.h"

namespace comum::asn {

// =================================================================================================================
// cassinavotavelbu.cpp (path uenux2/src/app/comum/gravadores/asn/cassinavotavelbu.cpp inferred from the srcloc
// "cassinavotavelbu.cpp:89"). Everything was inlined into DoConverte; the singleton is a 1-byte object
// (comum_f3603 -> comum_f2900(mutex @1909976, &ptr @1910000)) and the chain state is a static vector @1910004.
// =================================================================================================================
namespace {
std::vector<uebyte> s_hashAnterior;                                                    // @1910004 (+8 +12)

// wasm func 3602 (name inferred): SHA-512 of a text (ecourna CSha512, slots 2 Init / 3 Update / 4 Final).
std::vector<uebyte> CalculaHash(const std::string& texto)
{
    std::shared_ptr<ecourna::api::security::CSha512> sha(new ecourna::api::security::CSha512());   // ecourna_f2684;
                                                                                       // __shared_ptr_pointer @1597356
    sha->Inicializa();
    sha->Atualiza(std::vector<uebyte>(texto.begin(), texto.end()));
    return sha->Finaliza();
}
}  // namespace

class CAssinaVotavelBu {
public:
    static CAssinaVotavelBu& GetInst();                                                // wasm func 3603 (name inferred)

    // hash inicial = SHA-512("{pleito:05}|{eleicao:05}|{municipio:05}|{zona:04}|{secao:04}|{codigoCarga:24}")
    void IniciaEleicao(int pleito, TEleicaoID eleicao, TMunicipioID municipio, TZonaID zona, TSecaoID secao,
                       const std::string& codigoCarga)
    {
        s_hashAnterior = CalculaHash(std::format("{:05}|{:05}|{:05}|{:04}|{:04}|{:24}",
                                                 pleito, eleicao, municipio, zona, secao, codigoCarga));   // @8483
    }
    // hash(n) = SHA-512("<HEX(hash(n-1))>|ordem|cargo|tipoVoto|quantidade[|codigo|partido]")
    std::vector<uebyte> Encadeia(ueword ordem, TCargoID cargo, int tipoVotoAsn, TQtdEleitor quantidade,
                                 const std::optional<md::CIdentificacaoVotavel>& id)
    {
        const std::string anterior = util::BytesToHex(s_hashAnterior);                 // comum_f1243 (upper case)
        s_hashAnterior = id ? CalculaHash(std::format("{}|{}|{}|{}|{}|{}|{}", anterior, ordem, cargo, tipoVotoAsn,
                                                      quantidade, id->codigo, id->partido))      // @1120
                            : CalculaHash(std::format("{}|{}|{}|{}|{}", anterior, ordem, cargo, tipoVotoAsn,
                                                      quantidade));                              // @1126
        return s_hashAnterior;
    }
    const std::vector<uebyte>& GetUltimoHash() const { return s_hashAnterior; }

    // cassinavotavelbu.cpp:89. The signature is made by the urna's PKCS#11 token (hardware key).
    std::vector<uebyte> Assinar(const std::vector<uebyte>& hash) const
    {
        auto& pkcs11 = api::CPolySingletonList::instance<api::pkcs11::IPkcs11>();     // func 3704 (:89)
        pkcs11.AbreSessao();                                                           // slot 23   (names inferred)
        std::vector<uebyte> assinatura = pkcs11.Assina(hash);                          // slot 6
        pkcs11.FechaSessao();                                                          // slot 24
        return assinatura;
    }
};

// =================================================================================================================
// Anonymous helpers of cconversorentidadebu.cpp, all inlined into DoConverte (srclocs :87, :110, :132, :164, :175)
// =================================================================================================================
namespace {

// ConverteTipoVoto (srcloc :284): md::CVoto::ETipo -> ModuloBoletimUrna::TipoVoto (table @546352)
int ConverteTipoVoto(md::CVoto::ETipo tipo)
{
    static constexpr int tabela[9] = {
        4,   // 1 legenda                          -> legenda (4)
        1,   // 2 nominal                          -> nominal (1)
        2,   // 3 branco                           -> branco (2)
        3,   // 4 nulo                             -> nulo (3)
        2,   // 5 branco após suspensão            -> branco
        3,   // 6 nulo após suspensão              -> nulo
        3,   // 7 nulo por repetição               -> nulo
        5,   // 8 nulo cargo sem candidato         -> cargoSemCandidato (5)
        5,   // 9 nulo após suspensão, sem candidato-> cargoSemCandidato
    };
    const int indice = static_cast<int>(tipo) - 1;
    if (indice < 0 || indice >= 9)
        throw CUeComumGravadoresError(8609, std::format("Tipo inválido: {}", static_cast<int>(tipo)));   // :284
    return tabela[indice];
}

// ConverteTipoCargo (srcloc :299): 0 majoritário, 1 proporcional, 2 consulta -> TipoCargoConsulta 1/2/3
int ConverteTipoCargo(md::CCargo::ETipo tipo)
{
    if (static_cast<int>(tipo) >= 3)
        throw CUeComumGravadoresError(8610, std::format("Tipo inválido: {}", static_cast<int>(tipo)));   // :299
    return static_cast<int>(tipo) + 1;
}

// CriaVotosVotavel (srcloc :87)
ModuloBoletimUrna::TotalVotosVotavel CriaVotosVotavel(const md::CVotosVotavel& votos, const TCargoID cargo,
                                                      const ueword ordem)
{
    ModuloBoletimUrna::TotalVotosVotavel asn;
    const int tipoVoto = ConverteTipoVoto(votos.tipo);
    asn.set_tipoVoto(tipoVoto);
    asn.set_quantidadeVotos(votos.quantidade);                                         // Constrained_INTEGER<0,9999>
    if (votos.identificacao) {
        ModuloBoletimUrna::IdentificacaoVotavel id;
        id.set_codigo(votos.identificacao->codigo);                                    // 0..99999
        id.set_partido(votos.identificacao->partido);                                  // 0..99
        asn.set_identificacaoVotavel(id);
    }
    asn.set_ordemGeracaoHash(ordem);
    asn.set_hash(CAssinaVotavelBu::GetInst().Encadeia(ordem, cargo, tipoVoto, votos.quantidade, votos.identificacao));
    if (!asn.isValid() || !asn.isStrictlyValid())
        throw CUeComumGravadoresError(8603, "Entidade inválida");                      // :87
    return asn;
}

// CriaVotosCargo (srcloc :110). ordemGeracaoHash restarts at 1 for every cargo.
ModuloBoletimUrna::TotalVotosCargo CriaVotosCargo(const md::CVotosCargo& votosCargo)
{
    ModuloBoletimUrna::TotalVotosCargo asn;
    asn.set_codigoCargo(CConversorCodigoCargoConsulta().Converte(votosCargo.codigo));   // vtable @1555736
    asn.set_ordemImpressao(votosCargo.ordemImpressao);
    ueword ordem = 0;
    for (const auto& v : votosCargo.votos)
        asn.votosVotaveis().push_back(CriaVotosVotavel(v, votosCargo.codigo, ++ordem));
    if (!asn.isValid() || !asn.isStrictlyValid())
        throw CUeComumGravadoresError(8604, "Entidade inválida");                      // :110
    return asn;
}

// CriaResultadoVotacao (srcloc :132)
ModuloBoletimUrna::ResultadoVotacao CriaResultadoVotacao(const md::CResultadoVotacao& resultado)
{
    ModuloBoletimUrna::ResultadoVotacao asn;
    asn.set_tipoCargo(ConverteTipoCargo(resultado.tipoCargo));
    asn.set_qtdComparecimento(resultado.qtdComparecimento);
    for (const auto& vc : resultado.votosCargos)
        asn.totaisVotosCargo().push_back(CriaVotosCargo(vc));
    if (!asn.isValid() || !asn.isStrictlyValid())
        throw CUeComumGravadoresError(8605, "Entidade inválida");                      // :132
    return asn;
}

// CriaResultadoVotacaoPorEleicao (srcloc :164)
ModuloBoletimUrna::ResultadoVotacaoPorEleicao CriaResultadoVotacaoPorEleicao(
    const md::CResultadoVotacaoPorEleicao& resultado, const md::CEntidadeBU& bu)
{
    auto& assinador = CAssinaVotavelBu::GetInst();
    assinador.IniciaEleicao(bu.GetCabecalho().GetId(), resultado.idEleicao, bu.GetMunicipio(), bu.GetZona(),
                            bu.GetSecao(), bu.GetUrna().GetCorrespondencia().GetCarga().GetCodigoCarga());

    ModuloBoletimUrna::ResultadoVotacaoPorEleicao asn;
    asn.set_idEleicao(resultado.idEleicao);
    asn.set_qtdEleitoresAptos(static_cast<TQtdEleitor>(resultado.qtdAptosSecao + resultado.qtdAptosTTE));
    asn.set_qtdEleitoresAptosSecao(resultado.qtdAptosSecao);
    asn.set_qtdEleitoresAptosTTE(resultado.qtdAptosTTE);
    for (const auto& r : resultado.resultados)
        asn.resultadosVotacao().push_back(CriaResultadoVotacao(r));
    asn.set_ultimoHashVotosVotavel(assinador.GetUltimoHash());
    asn.set_assinaturaUltimoHashVotosVotavel(assinador.Assinar(assinador.GetUltimoHash()));   // :89
    if (!asn.isValid() || !asn.isStrictlyValid())
        throw CUeComumGravadoresError(8606, "Entidade inválida");                      // :164
    return asn;
}

// CriaResultadosVotacoes (srcloc :175)
void CriaResultadosVotacoes(ModuloBoletimUrna::EntidadeBoletimUrna& entidade, const md::CEntidadeBU& bu)
{
    // The chain's first field is the pleito: the entity header must be identified by a pleito (tipo 1).
    if (bu.GetCabecalho().GetTipoId() != md::CCabecalhoEntidade::ETipoId(1))
        throw CUeComumGravadoresError(8607, "Erro no hash encadeado: Tipo de identificador do cabeçalho da entidade "
                                            "BU precisa ser um pleito.");              // :175
    for (const auto& r : bu.GetResultados())
        entidade.resultadosVotacaoPorEleicao().push_back(CriaResultadoVotacaoPorEleicao(r, bu));
}

}  // namespace

// =================================================================================================================
// wasm func 10273 (vtable slot 2; srcloc cconversorentidadebu.cpp:232). Not observed in the recorded votes (it
// only runs at the encerramento); it did run in the harness of docs/bu/codepath.md.
// =================================================================================================================
CConversorEntidadeBU::TEntidade CConversorEntidadeBU::DoConverte(const TDado& bu) const
{
    TEntidade entidade;                                                                // ASN.1 EntidadeBoletimUrna
    entidade.set_cabecalho(CConversorCabecalhoEntidade().Converte(bu.GetCabecalho()));
    entidade.set_fase(Utils::ConverteFase(bu.GetFase()));
    entidade.set_urna(CConversorUrna().Converte(bu.GetUrna()));                       // func 10287

    auto& id = entidade.identificacaoSecao();
    id.municipioZona().set_municipio(bu.GetMunicipio());
    id.municipioZona().set_zona(bu.GetZona());
    id.set_local(bu.GetLocal());
    id.set_secao(bu.GetSecao());

    entidade.set_dataHoraEmissao(Utils::ConverteDataHora(bu.GetDataHoraEmissao()));  // comum_f1080: "YYYYMMDDThhmmss"
    entidade.set_qtdEleitoresCompareceram(bu.GetQtdComparecimento());

    if (bu.PossuiDetalhamentoComparecimento())                                          // flag +286
        entidade.set_detalhamentoComparecimento(                                        // GetDetalhamentoComparecimento (:206)
            ecourna::app::dados::asn::CConversorDetalhamentoComparecimento().Converte(bu.GetDetalhamentoComparecimento()));
    else
        entidade.omit_detalhamentoComparecimento();

    if (bu.PossuiDadosVota()) {                                                         // flag +240
        const md::CDadosBUVota& vota = bu.GetDadosVota();                               // centidadebu.cpp:186
        ModuloBoletimUrna::DadosSecao secao;
        secao.set_dataHoraAbertura(Utils::ConverteDataHora(vota.GetDataHoraAbertura()));
        secao.set_dataHoraEncerramento(Utils::ConverteDataHora(vota.GetDataHoraEncerramento()));
        if (vota.PossuiDataHoraDesligamentoVotoImpresso())
            secao.set_dataHoraDesligamentoVotoImpresso(
                Utils::ConverteDataHora(vota.GetDataHoraDesligamentoVotoImpresso()));   // cdadosbuvota.cpp:37
        else
            secao.omit_dataHoraDesligamentoVotoImpresso();
        entidade.dadosSecaoSA().set_dadosSecao(secao);                                  // CHOICE [0]
    } else if (bu.PossuiDadosSA()) {                                                    // flag +252
        const md::CDadosBUSA& sa = bu.GetDadosSA();                                     // centidadebu.cpp:196
        ModuloBoletimUrna::DadosSA dadosSA;
        dadosSA.set_juntaApuradora(sa.GetJunta());
        dadosSA.set_turmaApuradora(sa.GetTurma());
        if (sa.GetNumeroInternoUrnaOrigem() != 0)
            dadosSA.set_numeroInternoUrnaOrigem(sa.GetNumeroInternoUrnaOrigem());       // 0..99999999
        else
            dadosSA.omit_numeroInternoUrnaOrigem();
        entidade.dadosSecaoSA().set_dadosSA(dadosSA);                                   // CHOICE [1]
    } else {
        throw CUeComumGravadoresError(8608, "Objeto de dados do BU mal formado");       // :232
    }

    CriaResultadosVotacoes(entidade, bu);                                               // hash chain + signatures

    for (const std::string& codigo : bu.GetHistoricoCodigosCarga())                     // SEQUENCE OF GeneralString
        entidade.historicoCodigosCarga().push_back(ASN1::GeneralString(codigo));

    if (!bu.GetHistoricoVotoImpresso().empty()) {
        for (const auto& h : bu.GetHistoricoVotoImpresso())
            entidade.historicoVotoImpresso().push_back(CConversorHistoricoVotoImpresso().Converte(h));   // vtable @1597396
    } else {
        entidade.omit_historicoVotoImpresso();
    }
    return entidade;
}

}  // namespace comum::asn

// wasm func 2900 (comum_f2900, ICF body shared by comum_f348 / 948 / 2742 / 3603): lazy creation of a 1-byte
// (empty-class) singleton held in a static unique_ptr, under a static mutex (pthread stubs in this build):
//     std::lock_guard l(mutex); if (!ptr) ptr.reset(new T); return *ptr;
