// uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by std::source_location records
// :200 (ConverteEstadoBiometria) and :232 (ConverteDedoHabilitacao).
//
// SQL through ecourna::api::sql: ISqlConnection::Prepare = slot 5; ISqlStatement SetInt = 2,
// SetInt64 = 3, SetTimestamp = 6, SetNull = 9, Execute = 10 (returns the result set), Close = 12;
// ISqlResultSet (by column name) GetInt = 3, GetInt64 = 5, IsNull = 11, GetTimestamp = 13,
// GetBool = 15, Next = 18. Timestamps are stored as microseconds since the Julian-day epoch
// (µs = unix_seconds * 1'000'000 + 210866803200000000, i.e. JD 2440587.5).
//
// Web build: the table is created at start-up (the recorded MEMFS uenux.db has it, empty); nothing in
// the simulator inserts or reads rows (the operator thread never runs).
#include "comum/comparecimentomesario/dao/ccomparecimentomesariodao.h"

#include <format>

#include "comum/comparecimentomesario/comparecimentomesariodefs.h"   // CComparecimentoMesarioError
#include "ecourna/api/util/cstringutils.h"

namespace comum::dao {

namespace {
using md::CComparecimentoMesario;
using md::CEleitorIdentidade;
using ecourna::api::util::CStringUtils;
// CBaseError<comum::EUeComumComparecimentoMesarioError, SErrorLimits{7750, 7800}> (vtable @1593524,
// constructor thunk func 3611)
using CComparecimentoMesarioError = ecourna::api::exception::CBaseError<
    EUeComumComparecimentoMesarioError, ecourna::api::exception::SErrorLimits{7750, 7800}>;

constexpr std::int64_t EPOCA_JULIANA_US = 210866803200000000LL;

std::int64_t ParaTimestamp(const api::CDateTime& dataHora)             // inlined; vota_f1381 = timegm
{
    // The sentinel test is on the MICROSECOND value (the product), as in the binary:
    // `(us = secs * 1000000) == INT64_MIN ? INT64_MIN : us + epoch`.
    const std::int64_t microssegundos = dataHora.ToTimeT() * 1000000;
    return microssegundos == INT64_MIN ? INT64_MIN : microssegundos + EPOCA_JULIANA_US;
}

api::CDateTime DeTimestamp(std::int64_t us)                            // inlined; api_f1000
{
    // the three sentinel values INT64_MAX-1, INT64_MAX, INT64_MIN map to +/- 9223372036854 s
    std::int64_t segundos;
    if (static_cast<std::uint64_t>(us - (INT64_MAX - 1)) <= 2)
        segundos = (us == INT64_MIN) ? -9223372036854LL : 9223372036854LL;
    else
        segundos = (us - EPOCA_JULIANA_US) / 1000000;
    return api::CDateTime::FromTimeT(segundos);
}

// Row -> entity, inlined into Recuperar and RecuperarTodos.
template <class RESULTSET>
CComparecimentoMesario LeLinha(RESULTSET& rs, const CComparecimentoMesarioDAO& dao)
{
    const std::string titulo = std::format("{}", rs.GetInt64("numero_titulo"));
    const int  tipo          = rs.GetInt("tipo_identificador");
    const auto periodo       = static_cast<md::EPeriodoPresente>(rs.GetInt("periodo_presente"));
    const bool pertence      = rs.GetBool("pertence_secao");
    const auto estado        = dao.ConverteEstadoBiometria(rs.GetInt("estado_biometria"));
    const auto dedo          = dao.ConverteDedoHabilitacao(rs.GetInt("dedo_habilitacao"));
    const auto dataHora      = DeTimestamp(rs.GetTimestamp("data_hora_registro"));
    if (!rs.IsNull("id_arquivo")) {
        const auto id = static_cast<std::uint32_t>(rs.GetInt("id_arquivo"));
        return CComparecimentoMesario(CEleitorIdentidade(titulo, tipo), periodo, pertence, estado, dedo,
                                      dataHora, id);                          // func 2730
    }
    return CComparecimentoMesario(CEleitorIdentidade(titulo, tipo), periodo, pertence, estado, dedo,
                                  dataHora);                                  // func 2731
}
} // namespace

// wasm func 5391 (srcloc :200)
CComparecimentoMesario::EstadoReconhecimentoBiometrico
CComparecimentoMesarioDAO::ConverteEstadoBiometria(std::uint32_t valor) const
{
    if (valor < 1 || valor > 3)                                               // `valor - 1 >= 3`
        throw CComparecimentoMesarioError(EUeComumComparecimentoMesarioError{7755},
                                          std::format("Estado biometria inválido: {}", valor));   // :200
    return static_cast<CComparecimentoMesario::EstadoReconhecimentoBiometrico>(valor);
}

// wasm func 5390 (srcloc :232) - fingers 0..10
md::CDedo::TipoDedo CComparecimentoMesarioDAO::ConverteDedoHabilitacao(const std::uint32_t valor) const
{
    if (valor > 10)
        throw CComparecimentoMesarioError(EUeComumComparecimentoMesarioError{7756},
                                          std::format("Dedo habilitação inválido: {}", valor));   // :232
    return static_cast<md::CDedo::TipoDedo>(valor);
}

// wasm func 10399 (slot 2) - func 3894 is the body shared with CJustificadorDAO / CEleitorDinamicoDAO
// (copy the shared connection into a new 12-byte DAO, vtable passed as a parameter).
api::persistencia::IDAO* CComparecimentoMesarioDAO::Clone() const
{
    return new CComparecimentoMesarioDAO(m_conexao);
}

// wasm func 10403 (slot 3)
void CComparecimentoMesarioDAO::Inserir(const TDado& c) const
{
    auto stmt = m_conexao->Prepare(
        " INSERT INTO comparecimento_mesario (numero_titulo, tipo_identificador, periodo_presente, "
        "pertence_secao, estado_biometria, dedo_habilitacao, data_hora_registro, id_arquivo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?) \n");
    stmt->SetInt64(1, CStringUtils::ToQWord(c.GetIdentidade().GetIdentidade()));
    stmt->SetInt(2, static_cast<int>(c.GetIdentidade().GetTipo()));
    stmt->SetInt(3, static_cast<int>(c.GetPeriodo()));
    stmt->SetInt(4, c.PertenceSecao());
    stmt->SetInt(5, static_cast<int>(c.GetEstadoBiometria()));
    stmt->SetInt(6, static_cast<int>(c.GetDedo()));
    stmt->SetTimestamp(7, ParaTimestamp(c.GetDataHora()));
    if (c.GetIdArquivo().has_value())
        stmt->SetInt(8, static_cast<int>(*c.GetIdArquivo()));
    else
        stmt->SetNull(8);
    stmt->Execute();          // the returned result set is released immediately
    stmt->Close();
}

// wasm func 10402 (slot 7): one row by (título, período), nullptr if absent.
std::shared_ptr<CComparecimentoMesarioDAO::TDado> CComparecimentoMesarioDAO::Recuperar(const TDadoID& chave) const
{
    auto stmt = m_conexao->Prepare(
        " SELECT numero_titulo, tipo_identificador, periodo_presente, pertence_secao, estado_biometria, "
        "dedo_habilitacao, data_hora_registro, id_arquivo FROM comparecimento_mesario "
        "WHERE numero_titulo = ? and periodo_presente = ? \n");
    stmt->SetInt64(1, CStringUtils::ToQWord(chave.GetTitulo()));
    stmt->SetInt(2, static_cast<int>(chave.GetPeriodo()));
    auto rs = stmt->Execute();
    std::shared_ptr<TDado> resultado;
    if (rs->Next())
        resultado = std::shared_ptr<TDado>(new TDado(LeLinha(*rs, *this)));   // not make_shared (@1593484)
    stmt->Close();
    return resultado;
}

// wasm func 10401 (slot 8): every row, ordered.
std::vector<CComparecimentoMesarioDAO::TDado> CComparecimentoMesarioDAO::RecuperarTodos() const
{
    auto stmt = m_conexao->Prepare(
        " SELECT numero_titulo, tipo_identificador, periodo_presente, pertence_secao, estado_biometria,  "
        "dedo_habilitacao, data_hora_registro, id_arquivo FROM comparecimento_mesario  "
        "ORDER BY pertence_secao ASC, numero_titulo ASC, periodo_presente ASC, dedo_habilitacao ASC \n");
    auto rs = stmt->Execute();
    std::vector<TDado> resultado;
    while (rs->Next())
        resultado.push_back(LeLinha(*rs, *this));                               // 52-byte elements
    stmt->Close();
    return resultado;
}

} // namespace comum::dao
