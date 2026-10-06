// uenux2/src/app/comum/gravadores/cgravadorbu.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// The whole BU file is produced by ONE wasm function, CGravadorBU::GravaResultado (func 11629, 15.9 KB): LTO
// inlined MontaEntidadeBU (:636), CEntidadeBU::ValidaCriacao (centidadebu.cpp:159..177), LeChavePublica
// (:542..:560), the CConversorEntidadeBU call (iconversorasn.h:56) and CFileASN::CodeObjectFunction.
// Out-of-line helpers of this file: 3819 (votes of the cargos), 3820 AcrescentaVotoVotavel, and the vector /
// constructor instantiations 1556, 1944, 2802, 2853, 2856, 3814..3818, 3824, 5859, 5863, 5864.
//
// Not reached in the web page: the encerramento (and so CGravaResultado) never runs there (docs/bu/codepath.md).
// The harness run of docs/bu/codepath.md executed it and produced a BU that the TSE's tools accept.
#include "comum/gravadores/cgravadorbu.h"

#include <algorithm>
#include <filesystem>
#include <format>

#include "api/io/asn/cfileasn.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/security/ikernelhsm.h"
#include "api/iurna.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/cpath.h"
#include "comum/dados/ccandidaturas.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/cpartidos.h"
#include "comum/gravadores/asn/cconversorentidadebu.h"
#include "comum/gravadores/md/cenvelopegenerico.h"
#include "ecourna/api/cepesc/ccepesccipher.h"
#include "ecourna/api/io/serialization.hpp"
#include "ecourna/api/security/csymmetriccipherfactory.h"
#include "ecourna/api/security/irng.h"
#include "ModuloEnvelopeChave.h"
#include "ModuloInformacaoMidia.h"

namespace comum {

using md::CVoto;

// ---------------------------------------------------------------------------------------------------------------
// wasm func 3820 (srcloc cgravadorbu.cpp:221)
// Adds one TotalVotosVotavel to a cargo, unless it has no votes. Blank/null/"null because the cargo has no
// candidate" lines carry no identification; every other line is identified by (codigo, partido):
//   consulta      -> partido 0
//   candidate     -> party of the candidate (CCandidaturas)
//   party (legenda)-> the number must exist in CPartidos; partido = that number
void CGravadorBU::AcrescentaVotoVotavel(TVetorVotosVotaveis& votos, CVoto::ETipo tipo, TCandidatoID codigo,
                                        TQtdEleitor quantidade, const md::CCargo& cargo)
{
    if (quantidade == 0)          // unconditional: the PU parameter gravarZeradosBU (CParametrosUrna +393) is
        return;                   // never read by this writer, so zero-vote lines never reach the BU file

    if (tipo == CVoto::ETipo::BRANCO || tipo == CVoto::ETipo::NULO ||
        tipo == CVoto::ETipo::NULO_CARGO_SEM_CANDIDATO) {                              // bitmask 0x118
        votos.push_back(md::CVotosVotavel{tipo, codigo, quantidade, std::nullopt});    // func 2856 + 1944
        return;
    }

    if (cargo.EhConsulta()) {                                                          // optional<CDetalheConsulta> (+136)
        votos.push_back(md::CVotosVotavel{tipo, codigo, quantidade, md::CIdentificacaoVotavel{codigo, 0}});
        return;
    }

    TPartidoID partido;
    if (const auto* candidatura = CCandidaturas::GetInst().Busca(cargo.GetCodigo(), codigo))   // shared_f1273
        partido = candidatura->GetPartido();                                                  // +2
    else {
        auto& partidos = CPartidos::GetInst();                                                // ecourna_f819
        const auto it = partidos.find(static_cast<TPartidoID>(codigo));                       // inlined lower_bound
        if (it == partidos.end())
            throw CUeComumGravadoresError(8638, std::format("Candidato {} / cargo {} não existe",
                                                            codigo, cargo.GetCodigo()));      // :221
        partidos.SetCurrent(it);                                                              // cursor (+12) = node
        partido = it->second.GetNumero();
    }
    votos.push_back(md::CVotosVotavel{tipo, codigo, quantidade, md::CIdentificacaoVotavel{codigo, partido}});  // 5863/5864
}

// ---------------------------------------------------------------------------------------------------------------
// wasm func 3819 (name inferred). Votes of every cargo of one "tipo" of one election, read from the in-memory
// RDV (IRdv slots: 3 Candidato, 4 Legenda, 8 Nulos, 9 Brancos, 10 Cargo). Order of the lines of a cargo:
//   nominal votes (candidates in CCandidaturas order, or consulta answers) , branco, nulo, then legenda votes
//   (every party of CPartidos, proportional cargos only).
// wasm signature: (sret, &cargos, &m_ordemCargos (+284), m_rdv, &CCandidaturas, &CPartidos) - no `this`: the
// member reads and the two GetInst() calls were hoisted into the caller (11629).
std::vector<md::CVotosCargo> CGravadorBU::MontaVotosCargos(const std::vector<md::CCargo>& cargos) const
{
    const IRdv& rdv = *m_rdv;
    auto& candidaturas = CCandidaturas::GetInst();                                     // wasm_entry_f521
    auto& partidos = CPartidos::GetInst();                                             // ecourna_f819
    std::vector<md::CVotosCargo> resultado;

    for (const md::CCargo& cargo : cargos) {
        TVetorVotosVotaveis votos;

        if (!cargo.EhConsulta() && !candidaturas.PossuiCandidatos(cargo.GetCodigo())) {   // comum_f2271
            // cargo sem candidato: every vote of the cargo is a "nulo, cargo sem candidato" (ASN.1 cargoSemCandidato)
            if (const TQtdEleitor total = rdv.Cargo(cargo.GetCodigo()))
                votos.push_back(md::CVotosVotavel{CVoto::ETipo::NULO_CARGO_SEM_CANDIDATO, 0, total, std::nullopt});
        } else {
            TVetorVotosVotaveis nominais;
            std::vector<TCandidatoID> numeros;
            if (cargo.EhConsulta())
                for (const auto& resposta : cargo.GetDetalheConsulta().GetRespostas())   // func 1923, 28-byte items
                    numeros.push_back(resposta.GetNumero());
            else
                numeros = candidaturas.GetNumeros(cargo.GetCodigo(), false);            // comum_f2272
            for (const TCandidatoID numero : numeros)
                AcrescentaVotoVotavel(nominais, CVoto::ETipo::NOMINAL, numero,
                                      rdv.Candidato(cargo.GetCodigo(), numero, cargo.GetNumeroDigitos()), cargo);

            if (const TQtdEleitor brancos = rdv.Brancos(cargo.GetCodigo()))
                nominais.push_back(md::CVotosVotavel{CVoto::ETipo::BRANCO, 0, brancos, std::nullopt});
            if (const TQtdEleitor nulos = rdv.Nulos(cargo.GetCodigo()))
                nominais.push_back(md::CVotosVotavel{CVoto::ETipo::NULO, 0, nulos, std::nullopt});

            TVetorVotosVotaveis legendas;
            if (cargo.GetTipo() == md::CCargo::ETipo::PROPORCIONAL && cargo.PossuiDetalheCandidato())
                for (const auto& [numeroPartido, partido] : partidos)
                    AcrescentaVotoVotavel(legendas, CVoto::ETipo::LEGENDA, numeroPartido,
                                          rdv.Legenda(cargo.GetCodigo(), numeroPartido), cargo);

            votos = nominais;
            votos.insert(votos.end(), legendas.begin(), legendas.end());
        }

        resultado.push_back(md::CVotosCargo{cargo.GetCodigo(),
                                            m_ordemCargos.at(cargo.GetCodigo()),         // "map::at:  key not found"
                                            votos});
    }
    return resultado;
}

// ---------------------------------------------------------------------------------------------------------------
// Inlined into 11629 (name inferred): one CResultadoVotacaoPorEleicao per election of the pleito.
std::vector<md::CResultadoVotacaoPorEleicao> CGravadorBU::MontaResultados() const
{
    const auto& cfg = CConfiguracaoEleicao::GetInst();
    std::vector<md::CResultadoVotacaoPorEleicao> resultados;

    for (const auto& eleicao : cfg.GetEleicoes()) {                                   // 52-byte items (cfg +52)
        const std::vector<md::CCargo> cargos = cfg.GetCargos(eleicao.GetId(), true);   // comum_f3775
        std::vector<md::CCargo> majoritarios, proporcionais, consultas;               // comum_f3818 (copy_if)
        std::ranges::copy_if(cargos, std::back_inserter(majoritarios), [](auto& c) { return c.GetTipo() == md::CCargo::ETipo::MAJORITARIO; });
        std::ranges::copy_if(cargos, std::back_inserter(proporcionais), [](auto& c) { return c.GetTipo() == md::CCargo::ETipo::PROPORCIONAL; });
        std::ranges::copy_if(cargos, std::back_inserter(consultas), [](auto& c) { return c.GetTipo() == md::CCargo::ETipo::CONSULTA; });

        // Attendance = votes of the FIRST office of the group / number of choices of that office.
        // The majoritarian (or, without one, the consulta) figure is also used for consultas.
        // No guard on GetQtdEscolhas() (CCargo +13, a byte): 0 would be an integer division by zero (a wasm trap).
        TQtdEleitor comparecimento = 0;
        if (!majoritarios.empty())
            comparecimento = m_rdv->Cargo(majoritarios.at(0).GetCodigo()) / majoritarios.at(0).GetQtdEscolhas();
        else if (!consultas.empty())
            comparecimento = m_rdv->Cargo(consultas.at(0).GetCodigo()) / consultas.at(0).GetQtdEscolhas();
        const TQtdEleitor comparecimentoProporcional = proporcionais.empty() ? 0
            : m_rdv->Cargo(proporcionais.at(0).GetCodigo()) / proporcionais.at(0).GetQtdEscolhas();

        std::vector<md::CResultadoVotacao> porTipo;
        md::CResultadoVotacao r1{md::CCargo::ETipo::PROPORCIONAL, comparecimentoProporcional,
                                 MontaVotosCargos(proporcionais)};                      // func 3824
        if (!r1.votosCargos.empty()) porTipo.push_back(r1);                            // func 3817 (realloc)
        md::CResultadoVotacao r0{md::CCargo::ETipo::MAJORITARIO, comparecimento, MontaVotosCargos(majoritarios)};
        if (!r0.votosCargos.empty()) porTipo.push_back(r0);
        md::CResultadoVotacao r2{md::CCargo::ETipo::CONSULTA, comparecimento, MontaVotosCargos(consultas)};
        if (!r2.votosCargos.empty()) porTipo.push_back(r2);

        const auto& aptos = m_qtdAptos.at(eleicao.GetAbrangencia());                   // "map::at:  key not found"
        resultados.push_back(md::CResultadoVotacaoPorEleicao{eleicao.GetId(), aptos.secao, aptos.tte, porTipo});
    }
    return resultados;
}

// ---------------------------------------------------------------------------------------------------------------
// Inlined into 11629 (srcloc cgravadorbu.cpp:636). CEntidadeBU's constructor + ValidaCriacao inlined.
md::CEntidadeBU CGravadorBU::MontaEntidadeBU(const md::CCabecalhoEntidade& cabecalho, const md::CUrna& urna,
                                             const std::vector<md::CResultadoVotacaoPorEleicao>& resultados) const
{
    std::optional<ecourna::app::dados::CDetalhamentoComparecimento> detalhamento;
    if (m_urnaBiometrica)
        detalhamento.emplace(m_habilitacoes.semBiometria, m_habilitacoes.porBiometria, m_habilitacoes.porBiografia);

    if (m_dadosVota)
        return md::CEntidadeBU(cabecalho, m_fase, urna, m_municipio, m_zona, m_local, m_secao, m_comparecimento,
                               resultados, m_historicoCodigosCarga, *m_dadosVota, m_dhEmissao, detalhamento);
    if (m_dadosSA)
        return md::CEntidadeBU(cabecalho, m_fase, urna, m_municipio, m_zona, m_local, m_secao, m_comparecimento,
                               resultados, m_historicoCodigosCarga, *m_dadosSA, m_dhEmissao, detalhamento);
    throw CUeComumGravadoresError(8641, "Dados específicos do aplicativo não presentes");      // :636
}

// md::CEntidadeBU::ValidaCriacao (centidadebu.cpp:159..177), inlined into 11629:
//   fase == '0' || fase >= '4' (i32.ge_s) -> 8665 "Fase inválida"               :159
//                          (so it is NOT a '1'..'3' range check: a value below '0' passes)
//   município < 100000     else 8666 "Código de município inválido: {}"           :162
//   zona < 10000           else 8667 "Número de zona inválido: {}"                :167
//   local < 10000          else 8668 "Número de local inválido: {}"               :172
//   seção < 10000          else 8669 "Número de seção inválido: {}"               :177

// ---------------------------------------------------------------------------------------------------------------
// Inlined into 11629 (srcloc :542, :552, :560). The BU public key: /dsk/fi/estatico/chave/bu.pk1, an
// ModuloEnvelopeChave::EntidadeChave whose key material is itself enciphered with a key-encryption key
// obtained from the urna's HSM (api::IKernelHSM slot 3).
std::vector<uebyte> CGravadorBU::LeChavePublica() const
{
    const std::filesystem::path caminho = std::filesystem::path(CPath::GetPathChaves()) / m_arquivoChave;   // f1948
    if (!api::CSystem::FileExists(caminho))                                            // api_f412
        throw CUeComumGravadoresError(8639, "O arquivo " + caminho.string() + " não existe");   // :542

    ModuloEnvelopeChave::EntidadeChave entidade;
    ecourna::api::io::DeserializeFromBuffer(entidade, caminho.string());
    const std::vector<uebyte> chaveCifrada = entidade.GetChave();

    auto& hsm = api::CPolySingletonList::instance<api::IKernelHSM>();                 // func 2279, srcloc :552
    const std::string kek = hsm.GetChaveCifracao();                                   // slot 3
    std::vector<uebyte> chave;
    ecourna::api::security::CSymmetricCipherFactory().Cria(kek)->Decifra(chaveCifrada, chave);   // factory slot 2, cipher slot 3
    if (chave.empty())
        throw CUeComumGravadoresError(8640, "O arquivo " + caminho.string() + " está vazio");     // :560
    return chave;
}

// Inlined into 11629 (name inferred). Serial of the voting flash (MV) from its "infomidia.dat".
std::string CGravadorBU::LeSerialMV() const
{
    std::string serial = "00000000";
    const auto caminho = std::filesystem::path(CPath::GetPathEstatico(EFlashOrigem::EXTERNA)) / "infomidia.dat";  // wasm_entry_f762(1)
    if (api::CSystem::FileExists(caminho)) {
        const auto info = api::CFileASN::ReadFromFile<ModuloInformacaoMidia::InformacaoMidia>(caminho);   // func 3735
        serial = util::BytesToHex(info.GetNumeroSerie());                              // comum_f1243 (+32 vector)
    }                                                                                  // ~ comum_f3815
    return serial;
}

// ---------------------------------------------------------------------------------------------------------------
// wasm func 11629 (vtable slot 7; srcloc cgravadorbu.cpp:484, :485). Called by IGravador::Grava (11634).
void CGravadorBU::GravaResultado(api::CFile& arquivo) const
{
    const auto& estadoGeral = CAppInfo::GetInst().GetEstadoGeral();                    // GetEstado<CEstadoGeral> (291)
    const md::EUrnaTipo tipoUrna = estadoGeral.GetTipoUrna(estadoGeral.GetTurno());   // +36 ('1') / +40 (2º turno)
    const std::string serialMV = LeSerialMV();
    const auto& cfg = CConfiguracaoEleicao::GetInst();

    const std::vector<md::CResultadoVotacaoPorEleicao> resultados = MontaResultados();

    const md::CCabecalhoEntidade cabecalho(m_dhGeracao, cfg.GetPleito(), md::CCabecalhoEntidade::ETipoId(1));  // pleito
    const md::CCorrespondenciaResultado correspondencia(
        m_correspondencia.GetMunicipio(), m_correspondencia.GetZona(), m_correspondencia.GetSecao(),
        md::CCarga(m_correspondencia.GetNumeroInternoUrna(), m_correspondencia.GetSerialMC(),
                   m_correspondencia.GetDataHoraCarga(), m_correspondencia.GetCodigoCarga(),
                   m_correspondencia.GetIdentificadorGerador()),                         // func 2802 (+ ValidaCriacao)
        m_secao != 0 ? md::ETipoUrna::SECAO : md::ETipoUrna::CONTINGENCIA);            // '1' / '2': IResultado
                                                                                       // seção (+16), not the
                                                                                       // correspondência's (+118)
    const std::string versao = "10.23.0.1 - DESENVOLVIMENTO";                         // @326597
    const md::CUrna urna = m_motivoUtilizacaoSA
        ? md::CUrna(tipoUrna, versao, correspondencia, m_tipoArquivo, serialMV, *m_motivoUtilizacaoSA)   // func 2855
        : md::CUrna(tipoUrna, versao, correspondencia, m_tipoArquivo, serialMV);                          // func 2854

    const md::CEntidadeBU entidadeBU = MontaEntidadeBU(cabecalho, urna, resultados);   // + ValidaCriacao

    // EntidadeBoletimUrna -> BER bytes (CConversorEntidadeBU::DoConverte = func 10273, hash chain + signature)
    const auto asn = asn::CConversorEntidadeBU().Converte(entidadeBU);                // "Entidade deixada em estado inválido: {}" 7653
    std::vector<char> conteudo;
    api::CFileASN::CodeObjectFunction(conteudo, asn, "N17ModuloBoletimUrna19EntidadeBoletimUrnaE");   // cfileasn.h:161/171

    const TLocalID local = m_local;
    if (m_permiteCifrar && cfg.GetParametros().GetCriptografarBU()) {                // CGravadorBU +212, parâmetro +160
        // Encrypted BU: CEPESC over the BER bytes with the BU public key.
        std::vector<uebyte> tabela(1024);
        api::CPolySingletonList::instance<api::IUrna>().GetTabelaCripto(tabela);      // IUrna slot 4, srcloc :484
        std::vector<uebyte> semente(32, 0);
        api::CPolySingletonList::instance<ecourna::api::security::IRng>().Gera(semente);   // IRng slot 4, srcloc :485
        const std::vector<uebyte> chavePublica = LeChavePublica();
        const ecourna::api::cepesc::CPlainText claro(m_zona, m_secao, tabela, semente, chavePublica,
                                                     {conteudo.begin(), conteudo.end()});   // comum_f5168
        const auto cifrado = ecourna::api::cepesc::CCepescCipher().Cifra(claro);          // vf2
        const md::CSeguranca seguranca(0, 1, cifrado);
        const md::CEnvelopeGenerico envelope(cabecalho, m_fase, m_municipio, m_zona, local, m_secao,
                                             md::CEnvelopeGenerico::Tipo::BoletimUrna, seguranca,
                                             cifrado.GetConteudo(), urna.GetTipoUrna());   // comum_f3821
        api::CFileASN::CodeObjectFunction(arquivo, envelope);                           // func 2724
        return;
    }

    const md::CEnvelopeGenerico envelope(cabecalho, m_fase, m_municipio, m_zona, local, m_secao,
                                         md::CEnvelopeGenerico::Tipo::BoletimUrna,
                                         std::vector<uebyte>(conteudo.begin(), conteudo.end()),
                                         urna.GetTipoUrna());                          // comum_f5861
    api::CFileASN::CodeObjectFunction(arquivo, envelope);                               // func 2724
}

// ---------------------------------------------------------------------------------------------------------------
// wasm func 5857 (slot 0) / 11628 (slot 1 = 5857 + operator delete)
CGravadorBU::~CGravadorBU() = default;
// destroys: m_ordemCargos (3814 = __tree::destroy), m_arquivoChave, m_qtdAptos (1405), m_historicoCodigosCarga,
// m_correspondencia (857), IResultado.

// Library instantiations of this file (no TSE logic):
//   1556  std::vector<md::CVotosCargo>::vector(first, last, n)       (16-byte items, inner vector<CVotosVotavel>)
//   1944  std::vector<md::CVotosVotavel>::push_back(const&)         (24-byte items, trivially copyable)
//   2802  md::CCarga::CCarga(numeroInterno, serialMC, dataHora, codigoCarga, identificadorGerador) + ValidaCriacao
//   2853  std::uninitialized_copy for md::CResultadoVotacaoPorEleicao (20-byte items)
//   2856  md::CVotosVotavel::CVotosVotavel(tipo, codigo, quantidade)                    (no identification)
//   3814  std::__tree<std::pair<TCargoID, uebyte>>::destroy                               (m_ordemCargos)
//   3815  ~md::CInformacaoMidia (the infomidia.dat model read by LeSerialMV)
//   3816  std::uninitialized_move for md::CCargo (140-byte items; optional detalhes 365/374, 242/267)
//   3817  std::vector<md::CResultadoVotacao>::__push_back_slow_path
//   3818  std::copy_if(cargos, tipo == X)  (the three filters of MontaResultados)
//   3824  md::CResultadoVotacao::CResultadoVotacao(tipo, comparecimento, votosCargos)
//   5859  std::vector<md::CResultadoVotacaoPorEleicao>::__base_destruct_at_end
//   5863  md::CVotosVotavel::CVotosVotavel(tipo, codigo, quantidade, CIdentificacaoVotavel)
//   5864  md::CIdentificacaoVotavel::CIdentificacaoVotavel(codigo, partido)

}  // namespace comum
