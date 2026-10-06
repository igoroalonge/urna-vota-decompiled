// uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// CGravadorRCSecao (60 bytes, vtable @1554748) writes "...-jufa.dat" = ModuloResultadoUrnaCadastro::
// EntidadeResultadoUrnaCadastro ("RC da seção": the attendance record sent back to the voter registry):
// every voter of the section with his/her attendance state and how (s)he was enabled, the justifications of
// absence, and the mesários registered at opening/closing. With the parameter criptografarJUFA the attendance
// data is encrypted with CEPESC under the public key /dsk/fi/estatico/chave/jufa.pk1.
//   IResultado (+0..+39)   +40 api::CDateTime m_dhGeracao (day +40, month +42, year +44, seconds +48)
//   +52 int m_faseEcourna (CGravadorUtil::ConverteFaseEcourna: 'o'->2, 's'->1, 't'->3)
// Its destructor folded into IResultado's (func 12090); func 11615 is the deleting destructor.
// All the ecourna::app::dados classes used here are reconstructed in src/ecourna/app/dados/resultadournacadastro/.
#include "comum/gravadores/cgravadorrcsecao.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <format>

#include "api/pattern/cpolysingletonlist.h"
#include "api/persistencia/cdaorepositorio.h"
#include "comum/cpath.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/cjustificativas.h"
#include "ecourna/api/cepesc/ccepesccipher.h"
#include "ecourna/api/cepesc/cinfosalt.h"
#include "ecourna/api/security/irng.h"
#include "ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.h"
#include "ecourna/app/dados/cnumerocpf.h"
#include "ecourna/app/dados/cnumeroidentificacaolivre.h"
#include "ecourna/app/dados/cnumeroinscricaoeleitoral.h"
#include "ecourna/app/dados/resultadournacadastro/cresultadournacadastro.h"

namespace comum {

using namespace ecourna::app::dados;

// wasm func 5845 (srcloc cgravadorrcsecao.cpp:230)
TSharedIdentificadorEleitor CGravadorRCSecao::CriaIdentidadeEleitor(const md::CEleitorIdentidade& identidade) const
{
    switch (identidade.GetTipo()) {                                                    // +12
    case md::ETipoIdentificadorEleitor::TITULO:          // 1
        return std::make_shared<CNumeroInscricaoEleitoral>(CNumeroInscricaoEleitoral(identidade));
    case md::ETipoIdentificadorEleitor::CPF:             // 2
        return std::make_shared<CNumeroCPF>(CNumeroCPF(identidade));
    case md::ETipoIdentificadorEleitor::LIVRE:           // 3
        return std::make_shared<CNumeroIdentificacaoLivre>(CNumeroIdentificacaoLivre(identidade));
    }
    throw CUeComumGravadoresError(8654, std::format("Tipo de identidade invalida: {}",
                                                    static_cast<int>(identidade.GetTipo())));   // :230
}

// Inlined into 11616 (srcloc :65, :75, :83). Same scheme as CGravadorBU::LeChavePublica, file "jufa.pk1".
std::vector<uebyte> CGravadorRCSecao::LeChavePublica() const
{
    const std::filesystem::path caminho = std::filesystem::path(CPath::GetPathChaves()) / "jufa.pk1";   // @353405
    if (!api::CSystem::FileExists(caminho))
        throw CUeComumGravadoresError(8655, "O arquivo " + caminho.string() + " não existe");    // :65
    ModuloEnvelopeChave::EntidadeChave entidade;
    ecourna::api::io::DeserializeFromBuffer(entidade, caminho.string());
    auto& hsm = api::CPolySingletonList::instance<api::IKernelHSM>();                          // :75
    std::vector<uebyte> chave;
    ecourna::api::security::CSymmetricCipherFactory().Cria(hsm.GetChaveCifracao())->Decifra(entidade.GetChave(), chave);
    if (chave.empty())
        throw CUeComumGravadoresError(8656, "O arquivo " + caminho.string() + " está vazio");     // :83
    return chave;
}

// Inlined into 11616 (name inferred): attendance state of one voter.
std::optional<CEstadoComparecimento> CGravadorRCSecao::CriaEstadoComparecimento(
    const md::CEleitorIdentidade& identidade, const CEleitorDetalhe& eleitor) const
{
    const md::CEleitorDinamico& dinamico = eleitor.GetDinamico();                     // CEleitorDetalhe::GetDinamico
    const CRegistroIdentificacaoEleitor id(CriaIdentidadeEleitor(identidade),                  // map key (node +16)
                                           CriaIdentidadeEleitor(dinamico.GetIdentidade()));    // dinâmico +16; func 1878
    const auto situacao = static_cast<CEstadoComparecimento::ESituacaoComparecimento>(dinamico.GetSituacao());   // +32
    const CApresentacaoFotoEleitor& foto = dinamico.GetApresentacaoFoto();              // +76
    const auto audio = static_cast<CEstadoComparecimento::ESituacaoHabilitacaoAudio>(dinamico.GetHabilitacaoAudio());  // +48

    switch (dinamico.GetSituacao()) {
    case 0:                                                           // did not come
        return CEstadoComparecimento(id, situacao);                   // ecourna_f2658
    case 1:
        return CEstadoComparecimento(id, situacao, foto, audio);      // func 2193 (situation passed as literal 1)
    default:
        break;
    }
    switch (dinamico.GetTipoHabilitacao()) {                          // +36
    case 0:                                                           // enabled without biometrics
        return CEstadoComparecimento(id, situacao, foto, audio);      // func 2193
    case 1: {                                                         // enabled by fingerprint
        const CHabilitacaoBiometrica bio(dinamico.GetTentativas() /*+184*/, CBaseType<uint16_t, 0, 999>(dinamico.GetDedo() /*+88*/),
                                         dinamico.GetScore() /*+40*/, CErroLeituraBiometria(dinamico.GetErroLeitura() /*+52*/));  // func 5087
        return CEstadoComparecimento(id, situacao, foto, audio, bio); // func 2192
    }
    case 2: {                                                         // enabled by a mesário's code
        CEstadoHabilitacaoPorCodigo porCodigo(CEstadoHabilitacaoPorCodigo::ESituacaoReconhecimentoMesario(0));   // func 5090
        if (dinamico.PossuiTituloMesarioHabilitacao()) {                                  // +72
            const auto mesario = std::make_shared<CNumeroInscricaoEleitoral>(dinamico.GetTituloMesarioHabilitacao());
            // Was that mesário registered (comparecimento_mesario table) and recognised?
            int situacaoMesario = 0;                                                      // not found
            if (PersistenciaInicializada()) {                                             // comum_f2221
                // func 815 returns the repository, iterated directly as an ordered tree (no RecuperarTodos call)
                const auto& registros = api::persistencia::CDAORepositorio::Entregar<ICompareceimentoMesarioDAO>();
                const auto it = std::ranges::find_if(registros, [&](const auto& r) {
                    // node +16 string and +28 int compared with the título's {+0 string, +12 int} (the whole
                    // identity, not a constant tipo); node +32 = período (opening)
                    return r.GetIdentidade() == dinamico.GetTituloMesarioHabilitacao() && r.GetPeriodo() == 1;
                });
                if (it != registros.end())
                    situacaoMesario = !it->PossuiBiometria() ? 2 : (it->GetEstadoBiometria() ? 1 : 2);
            }
            porCodigo = CEstadoHabilitacaoPorCodigo(situacaoMesario, CRegistroIdentificacaoEleitor(mesario));  // 5088/1675
        }
        const CHabilitacaoBiometrica bio(dinamico.GetTentativas(), CBaseType<uint16_t, 0, 999>(dinamico.GetDedo()),
                                         dinamico.GetScore(), CErroLeituraBiometria(dinamico.GetErroLeitura()),
                                         porCodigo);                                      // func 2657
        return CEstadoComparecimento(id, situacao, foto, audio, bio); // func 2192
    }
    }
    return std::nullopt;                                              // other enabling types are skipped
}

// wasm func 11616 (vtable slot 7). The tools named it after the inlined LeChavePublica; srcloc :110 is
// GravaResultado. Not reached in the web page (encerramento); executed by the harness of docs/bu/codepath.md.
void CGravadorRCSecao::GravaResultado(api::CFile& arquivo) const
{
    // header: the generation time goes through text and boost (ISO "YYYYMMDDThhmmss")
    const std::string iso = std::format("{:04}{:02}{:02}T{:02}{:02}{:02}", m_dhGeracao.Ano(), m_dhGeracao.Mes(),
                                        m_dhGeracao.Dia(), m_dhGeracao.Segundos() / 3600,
                                        (m_dhGeracao.Segundos() % 3600) / 60, m_dhGeracao.Segundos() % 60);   // @8913
    const CCabecalhoEntidade cabecalho(boost::posix_time::from_iso_string(iso),
                                       CConfiguracaoEleicao::GetInst().GetPleito(), CCabecalhoEntidade::ETipoId(1));   // 5112
    const CIdentificacaoSecaoEleitoral secao(CMunicipioZona(CBaseType<uint32_t, 0, 99999>(m_municipio),
                                                            CBaseType<uint16_t, 0, 9999>(m_zona)),
                                             CBaseType<uint16_t, 0, 9999>(m_local),
                                             CBaseType<uint16_t, 0, 9999>(m_secao));                           // 5110

    // 1. every voter of the section's roll that is "apto" in the current turn (comum_f2266)
    std::vector<CEstadoComparecimento> eleitores;
    for (const auto& [identidade, eleitor] : CEleitores::GetInst())                    // map<CEleitorIdentidade, CEleitorDetalhe>
        if (eleitor.EhApto())                                                           // comum_f2266: impedimento(turno) == 0
            if (auto estado = CriaEstadoComparecimento(identidade, eleitor))
                eleitores.push_back(*estado);                                           // 5843/5844 slow paths
    const CComparecimentoSecao comparecimento(secao, eleitores);                        // func 5092

    // 2. justifications of absence (registro_justificativa table): título + ano de nascimento.
    //    Only when the repository singleton ALREADY exists (global @1839012 != 0, read before comum_f1391):
    //    the writer never creates it.
    std::vector<CIdentificacaoJustificativa> justificativas;
    if (CJustificativas::Existe())                                                      // @1839012 != nullptr
        for (const auto& [titulo, ano] : CJustificativas::GetInst())                    // comum_f1391
            justificativas.push_back({CRegistroIdentificacaoEleitor(std::make_shared<CNumeroInscricaoEleitoral>(titulo)),
                                      CBaseType<uint16_t, 0, 9999>(ano)});               // unknown_f5842

    // 3. mesários registered at the opening (período 1) and at the closing (período 2). Two separate passes over
    //    the repository returned by CDAORepositorio::Entregar (func 815), which is iterated directly as an ordered
    //    tree (begin +0, end node +4; período = node +32): copy_if(período == N) into 52-byte rows (5375), then
    //    CriaComparecimentoMesario (5841) for each. Rows with other períodos are ignored.
    //    Both lists are ALWAYS present in the entity (the optional flags are set to 1 unconditionally), empty when
    //    the persistence is not initialised or has no row for that period.
    auto listaMesarios = [](int periodo) {
        TVectorComparecimentoMesario lista;
        if (PersistenciaInicializada()) {                                               // comum_f2221
            const auto& registros = api::persistencia::CDAORepositorio::Entregar<ICompareceimentoMesarioDAO>();
            std::vector<SRegistroMesario> filtrados;                                    // 52-byte DAO rows
            std::ranges::copy_if(registros, std::back_inserter(filtrados),
                                 [&](const auto& r) { return r.GetPeriodo() == periodo; });
            for (const auto& registro : filtrados)
                lista.push_back(CriaComparecimentoMesario(registro));                   // 5841
        }
        return lista;
    };
    const std::optional<TVectorComparecimentoMesario> abertura = listaMesarios(1);        // flag byte f+780 = 1
    const std::optional<TVectorComparecimentoMesario> encerramento = listaMesarios(2);    // flag byte f+764 = 1
    const CDadosComparecimento dados(justificativas, comparecimento, abertura, encerramento);   // unknown_f3485

    const std::string versao = "10.23.0.1 - DESENVOLVIMENTO";
    if (CConfiguracaoEleicao::GetInst().GetParametros().GetCriptografarJUFA()) {       // parâmetro +161
        // Encrypted attendance: CEPESC(BER(DadosComparecimento)) with a random key/salt and jufa.pk1
        std::vector<uebyte> aleatorio(32);
        api::CPolySingletonList::instance<ecourna::api::security::IRng>().Gera(aleatorio);   // func 2030, slot 4 (:110)
        const auto salt = std::make_shared<ecourna::api::cepesc::CInfoSalt>(/*...*/);
        const std::vector<uebyte> chavePublica = LeChavePublica();
        std::vector<char> claro;
        api::CFileASN::CodeObjectFunction(claro, asn::CConversorDadosComparecimento().Converte(dados),
                                          "N27ModuloResultadoUrnaCadastro19DadosComparecimentoE");
        const auto cifrado = ecourna::api::cepesc::CCepescCipher().Cifra(/* key, salt, chavePublica, claro */);
        const CDadosComparecimentoCifrado dadosCifrados(
            CDadosCifracao(cifrado.GetChave(), cifrado.GetSalt(), cifrado.GetInformacaoAdicional()),   // 5091
            cifrado.GetConteudo());                                                                    // 3484
        const CResultadoUrnaCadastro rc(cabecalho, m_faseEcourna, versao, CResultadoUrnaCadastro::ArquivoFinal,
                                        dadosCifrados);                                                // 3482
        api::CFileASN::CodeObjectFunction(arquivo, rc);                                                // 5366
    } else {
        const CResultadoUrnaCadastro rc(cabecalho, m_faseEcourna, versao, CResultadoUrnaCadastro::ArquivoFinal,
                                        dados);                                                        // 3483
        api::CFileASN::CodeObjectFunction(arquivo, rc);
    }
}

// wasm func 11615 (slot 1): deleting destructor (IResultado part only).

// Library / helper instantiations of this file:
//   2278  std::uninitialized_copy for CEstadoComparecimento (96-byte items; also used by ecourna_f5092)
//   2845  std::uninitialized_move for CEstadoComparecimento (vector relocation; element dtor ecourna_f1006)
//   5375  std::vector<comparecimento_mesario DAO row (52 bytes)>::__push_back_slow_path (the copy_if buffer of
//         the mesário passes; NOT the ecourna CComparecimentoMesario, which 5841 builds and pushes inline)
//   5841  CriaComparecimentoMesario(registro) (name inferred): título -> CNumeroInscricaoEleitoral,
//         data/hora da coleta, estado da coleta digital
//   5843  std::vector<CEstadoComparecimento>::__emplace_back_slow_path(id, situacao, foto, audio, biometria) (2192)
//   5844  std::vector<CEstadoComparecimento>::__emplace_back_slow_path(id, situacao, foto, audio)            (2193)

}  // namespace comum
