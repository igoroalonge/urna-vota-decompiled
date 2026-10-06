// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp
//
// SQLite persistence of the per-voter "dynamic" state (comparecimento / habilitação: how the voter
// was enabled to vote, biometric attempts, accessibility audio, photo presentation, poll-worker
// override). Table eleitor_dinamico lives in uenux.db next to rdv.dat in the dynamic work area.
// SQL is executed through ecourna::api::sql (ISqlConnection::Prepare = slot 5,
// ISqlStatement::SetInt=2/SetInt64=3/SetNull=9/ExecuteQuery=10/Close=12,
// ISqlResultSet::GetInt=2/GetInt64=4/IsNull=10/Next=18).
//
// Web build: the table is never created in the recorded sessions (MEMFS uenux.db only has
// comparecimento_mesario and registro_justificativa): this DAO is only instantiated by
// CEleitores::DynamicCreate/CompleteLoad and by the native CSincronismoVotoEleitor, which the
// simulator replaces with CSincronismoVotoEleitorWeb; CGeraDadosDinamicos also skips
// CEleitores::DynamicCreate in voter-training mode (u04 analysis), the mode of all scenarios.
#include "celeitordinamicodao.h"

#include <format>

#include "ecourna/api/util/cstringutils.h"
#include "md/cvalidadoridentidade.h"   // md::CEleitorIdentidade(std::string, tipo) validates (func 566)

namespace comum::dao {

namespace {
using ecourna::api::util::CStringUtils;
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;

constexpr const char* COLUNAS_SELECT =
    "SELECT titulo, tipo_identificador, identidade_habilitacao, tipo_identidade_habilitacao, "
    "estado_comparecimento, tipo_habilitacao, dedo_habilitacao, score_habilitacao, numero_tentativa, "
    "erro_decifrar_biometria, tipo_ativacao_audio, estado_apresentacao_foto, resultado_decifracao_foto, "
    "titulo_mesario, tipo_identificador_mesario from ";

// Row -> entity, as inlined in RecuperarTodos (3766). Column order is the SELECT order above.
// Recuperar (11542) has its own copy of this code with one difference (see there).
md::CEleitorDinamico LeLinha(ecourna::api::sql::ISqlResultSet& rs)
{
    const std::string titulo      = std::format("{}", rs.GetInt64(0));
    const auto        tipoTitulo  = rs.GetInt(1);
    const std::string identidade  = std::format("{}", rs.GetInt64(2));
    const auto        tipoIdent   = rs.GetInt(3);
    const auto estadoComparecimento = rs.GetInt(4);
    const auto tipoHabilitacao      = rs.GetInt(5);
    const auto dedoHabilitacao      = rs.GetInt(6);
    const auto scoreHabilitacao     = rs.GetInt(7);
    const auto numeroTentativa      = rs.GetInt(8);
    const auto erroDecifrarBiometria= rs.GetInt(9);
    const auto tipoAtivacaoAudio    = rs.GetInt(10);
    const auto apresentacaoFoto     = std::pair{rs.GetInt(11), rs.GetInt(12)};   // shared_f1081

    if (!rs.IsNull(13)) {
        const std::string tituloMesario = std::format("{}", rs.GetInt64(13));
        const auto tipoMesario = rs.GetInt(14);
        return md::CEleitorDinamico(md::CEleitorIdentidade(titulo, tipoTitulo),
                                    md::CEleitorIdentidade(identidade, tipoIdent),
                                    estadoComparecimento, tipoHabilitacao, dedoHabilitacao,
                                    static_cast<std::uint16_t>(scoreHabilitacao),
                                    static_cast<std::uint8_t>(numeroTentativa),
                                    tipoAtivacaoAudio, erroDecifrarBiometria,
                                    md::CEleitorIdentidade(tituloMesario, tipoMesario),
                                    apresentacaoFoto);                                  // func 5663
    }
    return md::CEleitorDinamico(md::CEleitorIdentidade(titulo, tipoTitulo),
                                md::CEleitorIdentidade(identidade, tipoIdent),
                                estadoComparecimento, tipoHabilitacao, dedoHabilitacao,
                                static_cast<std::uint16_t>(scoreHabilitacao),
                                static_cast<std::uint8_t>(numeroTentativa),
                                tipoAtivacaoAudio, erroDecifrarBiometria, apresentacaoFoto); // func 5664
}
}  // namespace

const std::string CEleitorDinamicoDAO::TABELA = "eleitor_dinamico";   // dtor registered: func 11545

// wasm func 3768
CEleitorDinamicoDAO::CEleitorDinamicoDAO(const std::string& caminhoBanco)
    // new(24) + a separate control block (vtable __shared_ptr_pointer<CSqlConnection*> @1559008),
    // i.e. shared_ptr(new ...), not make_shared (which would use __shared_ptr_emplace)
    : m_conexao(new ecourna::api::sql::sqlite::CSqlConnection(caminhoBanco))
{
    // NOTE (original bug, kept): the CHECK of resultado_decifracao_foto tests estado_apresentacao_foto,
    // and the CHECK of tipo_identificador_mesario tests tipo_identificador.
    const std::string sql = "CREATE TABLE IF NOT EXISTS " + TABELA +
        "( titulo BIGINT PRIMARY KEY UNIQUE NOT NULL, "
        "tipo_identificador INTEGER CHECK(tipo_identificador IN (1,2,3) ) NOT NULL, "
        "identidade_habilitacao BIGINT NOT NULL, "
        "tipo_identidade_habilitacao INTEGER CHECK(tipo_identidade_habilitacao IN (1,2,3) ) NOT NULL, "
        "estado_comparecimento INTEGER NOT NULL, tipo_habilitacao INTEGER NOT NULL, "
        "dedo_habilitacao INTEGER NOT NULL, score_habilitacao INTEGER NOT NULL, "
        "tipo_ativacao_audio INTEGER CHECK(tipo_ativacao_audio IN (0,1,2) ) NOT NULL, "
        "numero_tentativa INTEGER CHECK(numero_tentativa IN (0,1,2,3,4) ) NOT NULL, "
        "erro_decifrar_biometria INTEGER NOT NULL, "
        "estado_apresentacao_foto INTEGER CHECK(estado_apresentacao_foto IN (0,1,2) ) NOT NULL, "
        "resultado_decifracao_foto INTEGER CHECK(estado_apresentacao_foto IN (0,1,2,3,4,5,6,7,8) ) NOT NULL, "
        "titulo_mesario BIGINT, "
        "tipo_identificador_mesario INTEGER CHECK(tipo_identificador IN (1,2,3) ) ) \n";
    auto stmt = m_conexao->Prepare(sql);
    stmt->ExecuteQuery();
    stmt->Close();
}

// wasm func 11541 (slot 2)  name inferred: comum_f3894(this, vtable) = new CEleitorDinamicoDAO(*this)
api::persistencia::IDAO* CEleitorDinamicoDAO::Clone() const
{
    return new CEleitorDinamicoDAO(*this);   // copies the vptr and the shared connection (use_count+1)
}

// wasm func 5776 (slot 3)  name inferred
void CEleitorDinamicoDAO::Inserir(const md::CEleitorDinamico& e) const
{
    auto stmt = m_conexao->Prepare("INSERT INTO " + TABELA +
        " (titulo, tipo_identificador, identidade_habilitacao, tipo_identidade_habilitacao, "
        "estado_comparecimento, tipo_habilitacao, dedo_habilitacao, score_habilitacao, "
        "tipo_ativacao_audio, numero_tentativa, erro_decifrar_biometria, estado_apresentacao_foto, "
        "resultado_decifracao_foto, titulo_mesario, tipo_identificador_mesario ) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)\n");
    stmt->SetInt64(1, CStringUtils::ToQWord(e.GetTitulo().GetIdentidade()));
    stmt->SetInt(2, e.GetTitulo().GetTipo());
    stmt->SetInt64(3, CStringUtils::ToQWord(e.GetIdentidadeHabilitacao().GetIdentidade()));
    stmt->SetInt(4, e.GetIdentidadeHabilitacao().GetTipo());
    stmt->SetInt(5, e.GetEstadoComparecimento());
    stmt->SetInt(6, e.GetTipoHabilitacao());
    stmt->SetInt(7, e.GetDedoHabilitacao());
    stmt->SetInt(8, e.GetScoreHabilitacao());
    stmt->SetInt(9, e.GetTipoAtivacaoAudio());
    stmt->SetInt(10, e.GetNumeroTentativa());
    stmt->SetInt(11, e.GetErroDecifrarBiometria());
    stmt->SetInt(12, e.GetApresentacaoFoto().first);
    stmt->SetInt(13, e.GetApresentacaoFoto().second);
    if (e.HabilitadoManualmente()) {                 // optional<CEleitorIdentidade> engaged (+72)
        stmt->SetInt64(14, CStringUtils::ToQWord(e.GetTituloMesarioHabilitacao().GetIdentidade()));
        stmt->SetInt(15, e.GetTituloMesarioHabilitacao().GetTipo());
    } else {
        stmt->SetNull(14);
        stmt->SetNull(15);
    }
    stmt->ExecuteQuery();
    stmt->Close();
}

// wasm func 11543 (slot 5) (srcloc line 130)
void CEleitorDinamicoDAO::ExcluirID(const std::string& /*titulo*/) const
{
    throw CUeComumDadosError(7983, "Exclusão de eleitor dinâmico");
}

// wasm func 5775 (slot 6)
void CEleitorDinamicoDAO::Atualizar(const md::CEleitorDinamico& e) const
{
    auto stmt = m_conexao->Prepare("UPDATE " + TABELA +
        " SET identidade_habilitacao = ?, tipo_identidade_habilitacao = ?, estado_comparecimento = ?, "
        "tipo_ativacao_audio = ?, tipo_habilitacao = ?,  dedo_habilitacao = ?, score_habilitacao = ?, "
        "numero_tentativa = ?, erro_decifrar_biometria = ?, estado_apresentacao_foto = ?, "
        "resultado_decifracao_foto = ?, titulo_mesario = ?, tipo_identificador_mesario = ? "
        "WHERE titulo = ?");
    stmt->SetInt64(1, CStringUtils::ToQWord(e.GetIdentidadeHabilitacao().GetIdentidade()));
    stmt->SetInt(2, e.GetIdentidadeHabilitacao().GetTipo());
    stmt->SetInt(3, e.GetEstadoComparecimento());
    stmt->SetInt(4, e.GetTipoAtivacaoAudio());
    stmt->SetInt(5, e.GetTipoHabilitacao());
    stmt->SetInt(6, e.GetDedoHabilitacao());
    stmt->SetInt(7, e.GetScoreHabilitacao());
    stmt->SetInt(8, e.GetNumeroTentativa());
    stmt->SetInt(9, e.GetErroDecifrarBiometria());
    stmt->SetInt(10, e.GetApresentacaoFoto().first);
    stmt->SetInt(11, e.GetApresentacaoFoto().second);
    if (e.HabilitadoManualmente()) {
        stmt->SetInt64(12, CStringUtils::ToQWord(e.GetTituloMesarioHabilitacao().GetIdentidade()));
        stmt->SetInt(13, e.GetTituloMesarioHabilitacao().GetTipo());
    } else {
        stmt->SetNull(12);
        stmt->SetNull(13);
    }
    stmt->SetInt64(14, CStringUtils::ToQWord(e.GetTitulo().GetIdentidade()));
    stmt->ExecuteQuery();
    stmt->Close();
}

// wasm func 11542 (slot 7)  name inferred
std::shared_ptr<md::CEleitorDinamico> CEleitorDinamicoDAO::Recuperar(const std::string& titulo) const
{
    auto stmt = m_conexao->Prepare(COLUNAS_SELECT + TABELA + " WHERE titulo = ?\n");
    stmt->SetInt64(1, CStringUtils::ToQWord(titulo));
    auto rs = stmt->ExecuteQuery();
    std::shared_ptr<md::CEleitorDinamico> resultado;
    if (rs->Next()) {
        // Same column reads as LeLinha, but the object is built directly in new(84) memory and owned
        // through shared_ptr(new ...) (control block vtable __shared_ptr_pointer @1559048).
        const std::string tituloLido  = std::format("{}", rs->GetInt64(0));
        const auto        tipoTitulo  = rs->GetInt(1);
        const std::string identidade  = std::format("{}", rs->GetInt64(2));
        const auto        tipoIdent   = rs->GetInt(3);
        const auto estadoComparecimento = rs->GetInt(4);
        const auto tipoHabilitacao      = rs->GetInt(5);
        const auto dedoHabilitacao      = rs->GetInt(6);
        const auto scoreHabilitacao     = rs->GetInt(7);
        const auto numeroTentativa      = rs->GetInt(8);
        const auto erroDecifrarBiometria= rs->GetInt(9);
        const auto tipoAtivacaoAudio    = rs->GetInt(10);
        const auto apresentacaoFoto     = std::pair{rs->GetInt(11), rs->GetInt(12)};
        if (!rs->IsNull(13)) {
            const std::string tituloMesario = std::format("{}", rs->GetInt64(13));
            const auto tipoMesario = rs->GetInt(14);
            resultado.reset(new md::CEleitorDinamico(
                md::CEleitorIdentidade(tituloLido, tipoTitulo), md::CEleitorIdentidade(identidade, tipoIdent),
                estadoComparecimento, tipoHabilitacao, dedoHabilitacao,
                static_cast<std::uint16_t>(scoreHabilitacao), static_cast<std::uint8_t>(numeroTentativa),
                tipoAtivacaoAudio, erroDecifrarBiometria,
                md::CEleitorIdentidade(tituloMesario, tipoMesario), apresentacaoFoto));   // func 5663
        } else {
            // NOTE: in this branch the wasm builds the voter's CEleitorIdentidade from the lookup
            // PARAMETER `titulo` (local 2 of func 11542, untouched on this path), not from the
            // formatted column 0 (which is computed and then only freed). Same value in practice
            // (WHERE titulo = ?), but the two branches of the original code differ here.
            resultado.reset(new md::CEleitorDinamico(
                md::CEleitorIdentidade(titulo, tipoTitulo), md::CEleitorIdentidade(identidade, tipoIdent),
                estadoComparecimento, tipoHabilitacao, dedoHabilitacao,
                static_cast<std::uint16_t>(scoreHabilitacao), static_cast<std::uint8_t>(numeroTentativa),
                tipoAtivacaoAudio, erroDecifrarBiometria, apresentacaoFoto));              // func 5664
        }
    }
    stmt->Close();
    return resultado;
}

// wasm func 3766 (slot 8)  name inferred
std::vector<md::CEleitorDinamico> CEleitorDinamicoDAO::RecuperarTodos() const
{
    auto stmt = m_conexao->Prepare(COLUNAS_SELECT + TABELA + "\n");
    auto rs = stmt->ExecuteQuery();
    std::vector<md::CEleitorDinamico> eleitores;
    while (rs->Next()) {
        // the row object is built in a local (5663/5664) and then COPIED into the vector (the inlined
        // fast path deep-copies the strings; slow path = func 5759, vector<CEleitorDinamico>, 84 B)
        const md::CEleitorDinamico eleitor = LeLinha(*rs);
        eleitores.push_back(eleitor);
    }
    stmt->Close();
    return eleitores;
}

}  // namespace comum::dao
