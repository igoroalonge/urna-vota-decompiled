// Reconstructed from vota_web_wasm.wasm (unit u05). Original: uenux2/src/app/comum/dados/crdvvota.cpp
//
// The build is LTO + wasm-opt: every CRdvVota query method is a one-liner that forwards to the
// member m_votos (comum::md::CVotosEleicoesVota, cvotoseleicoesvota.cpp). The callee bodies were
// inlined into the CRdvVota vtable slots and then merged by wasm-opt ("merge-similar-functions")
// into three shared bodies that take the per-method constants as extra parameters:
//     func 2296  <- Nominais / Legendas / Nulos / Brancos / Cargo      (5 thunks)
//     func 6033  <- Legenda / Partido                                  (2 thunks)
//     func 3707  <- the "cargo -> eleição" lookup used by all of them
// The std::source_location records inside them therefore name CVotosEleicoesVota methods
// (cvotoseleicoesvota.cpp:113..190). They are reconstructed at the end of this file, in the form
// they had in cvotoseleicoesvota.cpp, so that the wasm functions of this unit are all accounted for.
#include "crdvvota.h"

#include <format>
#include <functional>
#include <typeinfo>

#include "api/io/asn/cfileasn.h"
#include "api/pattern/csingleton.h"      // comum_f1406: GetInst() helper ("<X> - instancia nao criada")
#include "cconfiguracaoeleicao.h"
#include "ecourna/api/exception/cbaseerror.h"

namespace comum {

namespace {
// CBaseError<api::EUeRdvError, SErrorLimits{4650, 4850}>; func 655 is its (merged) constructor
// thunk: ecourna_f710(exc, code, std::move(msg), srcloc, vtable CBaseError<EUeRdvError>).
using CUeRdvError = ecourna::api::exception::CBaseError<api::EUeRdvError,
                                                        ecourna::api::exception::SErrorLimits{4650, 4850}>;
using CUeComumAsnError = ecourna::api::exception::CBaseError<comum::EUeComumAsnError,
                                                             ecourna::api::exception::SErrorLimits{7650, 7750}>;

// Zero-left-pad of a number (api_f753 = "copy s, then insert(0, n - size, c)"; see u05 doc).
std::string NumeroComZeros(std::uint32_t numero, std::size_t digitos)
{
    return api::CStringUtils::PadLeft(std::to_string(numero), '0', digitos);   // api_f753
}
}  // namespace

// wasm func 555 (srcloc line 87)
CRdvVota& CRdvVota::GetInst()
{
    // comum_f1406(mutex @1838932, srcloc, msg, &s_instancia @1838956): locks, throws
    // CBaseError<EPatternError>(1303, msg) when the instance was never created.
    return api::CSingleton<CRdvVota>::GetInst("CRdvVota - instancia nao criada");
}

// (not a separate wasm function: inlined into the start-up function 7787)  (srcloc line 92)
void CRdvVota::CreateInst()
{
    if (s_instancia)
        throw CUeRdvError(4658, "Instancia ja criada");
    s_instancia.reset(new CRdvVota());
}

// wasm func 5733 (vtable slot 0)  name inferred
// Votes of every office of every election, keyed by office. Used by the zeresima's
// "-----------EXTRATO DO RDV-------------" (vota::CriaTituloExtratoRDV asserts every list is empty:
// "Assert (votosCargos.Total() == 0)").
std::map<TCargoID, md::CVotos> CRdvVota::GetVotos() const
{
    std::map<TCargoID, md::CVotos> votos;
    for (const auto& [eleicao, votosCargos] : m_votos.GetEleicoes())
        for (const auto& [cargo, dados] : votosCargos.GetCargos())       // pair<SCargoInfo, CVotos>
            votos.try_emplace(cargo, md::CVotos(dados.second.GetVotos())); // copy vector, CVotos(vector&&)
    return votos;
}

// wasm func 11490 (vtable slot 1)  name inferred (the inlined callee is
// CVotosEleicoesVota::GetVotos(TEleicaoID), cvotoseleicoesvota.cpp:113, error 4733)
std::map<TCargoID, md::CVotos> CRdvVota::GetVotos(TEleicaoID eleicao) const
{
    std::map<TCargoID, md::CVotos> votos;
    for (const auto& [cargo, dados] : m_votos.GetVotos(eleicao).GetCargos())
        votos.try_emplace(cargo, md::CVotos(dados.second.GetVotos()));
    return votos;
}

// wasm func 11492 (vtable slot 2)
TQtdVoto CRdvVota::Comparecimento(TEleicaoID eleicao) const
{
    return m_votos.Comparecimento(eleicao);   // func 5639 (u22): votes of the first office / qtdEscolhas
}

// (func 1269, NOT in unit u05 - classified rt:shared) non-virtual, name inferred:
// highest turnout among the configured elections; used by BU/zeresima/reinício code to decide
// whether votes were already cast (e.g. CQuerReimprimirZeresima, CReinicioVotacao, CGeraBU).
TQtdVoto CRdvVota::Comparecimento() const
{
    TQtdVoto maximo = 0;
    for (const TEleicaoID eleicao : CConfiguracaoEleicao::GetInst().GetEleicoes())   // api_f5791
        maximo = std::max(maximo, m_votos.Comparecimento(eleicao));
    return maximo;
}

// wasm func 1931 (vtable slot 3); inlined body = CVotosEleicoesVota::Candidato (cvotoseleicoesvota.cpp:126)
TQtdVoto CRdvVota::Candidato(TCargoID cargo, TCandidatoID numero, uebyte digitos) const
{
    return m_votos.Candidato(cargo, NumeroComZeros(numero, digitos));
}

// wasm func 3749 (vtable slot 4) -> merged body 6033; callee CVotosEleicoesVota::Legenda (:140)
TQtdVoto CRdvVota::Legenda(TCargoID cargo, TPartidoID partido) const
{
    return m_votos.Legenda(cargo, NumeroComZeros(partido, m_digitosPartido));
}

// wasm func 2814 (vtable slot 5) -> merged body 6033; callee CVotosEleicoesVota::Partido (:154)
TQtdVoto CRdvVota::Partido(TCargoID cargo, TPartidoID partido) const
{
    return m_votos.Partido(cargo, NumeroComZeros(partido, m_digitosPartido));
}

// wasm func 2813 (vtable slot 6) -> merged body 2296; callee CVotosEleicoesVota::Nominais (:166)
TQtdVoto CRdvVota::Nominais(TCargoID cargo) const { return m_votos.Nominais(cargo); }

// wasm func 5732 (vtable slot 7) -> merged body 2296; callee CVotosEleicoesVota::Legendas (:172)
TQtdVoto CRdvVota::Legendas(TCargoID cargo) const { return m_votos.Legendas(cargo); }

// wasm func 3748 (vtable slot 8) -> merged body 2296; callee CVotosEleicoesVota::Nulos (:178)
TQtdVoto CRdvVota::Nulos(TCargoID cargo) const { return m_votos.Nulos(cargo); }

// wasm func 3747 (vtable slot 9) -> merged body 2296; callee CVotosEleicoesVota::Brancos (:184)
TQtdVoto CRdvVota::Brancos(TCargoID cargo) const { return m_votos.Brancos(cargo); }

// wasm func 1930 (vtable slot 10) -> merged body 2296; callee CVotosEleicoesVota::Cargo (:190)
TQtdVoto CRdvVota::Cargo(TCargoID cargo) const { return m_votos.Cargo(cargo); }

// wasm func 11488 (vtable slot 11)  name inferred
// Serialises the RDV: CVotosEleicoesVota -> ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto
// -> BER. The caller (CRdv/CSincronismoVotoEleitor, funcs 5736/5737/7174) writes the bytes through
// api::CEncryptedFile::MemWrite to rdv.dat (".tmp" first), in /dsk/fi and /dsk/fe.
std::vector<uebyte> CRdvVota::Converte() const
{
    // comum::asn::IConversorASN<...>::Converte (iconversorasn.h:56), inlined:
    const auto entidade = m_conversor.DoConverte(m_votos);           // vtable slot 2 of the converter
    if (!entidade.isValid() || !entidade.isStrictlyValid()) {
        std::ostringstream trace;
        ASN1::trace_invalid(trace, typeid(ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto).name(), entidade);
        throw CUeComumAsnError(7653, std::format("Entidade deixada em estado inválido: {}", trace.str()));
    }
    // api::CFileASN::CodeObjectFunction (cfileasn.h:161/171), inlined:
    std::vector<char> ber;
    api::CFileASN::CodeObjectFunction(ber, entidade,
        "CodeObject de " + std::string(typeid(ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto).name()));
    //   -> "Objeto com conteúdo inválido para {}: {}" (EUeIoError 5955) if !isValid
    //   -> "Arquivo não foi codificado para " + nome   (EUeIoError 5956) if encodeBER fails
    return std::vector<uebyte>(ber.begin(), ber.end());
}

// wasm func 11487 (vtable slot 12) (srcloc line 237)
// Loads rdv.dat content (CEleitores::CompleteLoad reads it through CEncryptedFile::Load/MemRead).
void CRdvVota::Desconverte(const std::vector<uebyte>& conteudo)
{
    md::CVotosEleicoesVota votos = m_conversor.Desconverte(conteudo);   // func 5680 (u03/u21)
    if (!m_votos.EstruturaCompativel(votos))                             // func 5638
        throw CUeRdvError(4660, "Conteudo do arquivo eh incompatível com objeto");
    std::swap(m_votos, votos);   // both maps swapped (inlined __tree swaps + destroy 1159/1550)
}

// wasm func 11486 (vtable slot 13)  name inferred
// Read-back check after writing rdv.dat.tmp: vota::impl::CSincronismoVotoEleitor::vf2 (7174) decrypts
// the file it just wrote and only renames it over rdv.dat when this returns true.
bool CRdvVota::ConfereConteudo(const std::vector<uebyte>& conteudo) const
{
    const md::CVotosEleicoesVota votos = m_conversor.Desconverte(conteudo);
    return m_votos.EstruturaCompativel(votos) && m_votos == votos;
    // operator== (inlined): same number of elections; per election same key and, per office,
    // same key and the same vector<CVoto> (CVoto = {int tipo; std::string numero}, compared field by
    // field); then the map<TCargoID, TEleicaoID> must be equal too. SCargoInfo is NOT compared here
    // (EstruturaCompativel already did it).
}

// ======================================================================================
// Bodies inlined from cvotoseleicoesvota.cpp (class comum::md::CVotosEleicoesVota, unit u22)
// ======================================================================================
namespace md {

// wasm func 3707 (merged helper; the srcloc passed in is the caller's: cvotoseleicoesvota.cpp:126..190)
// m_cargoEleicao: std::map<TCargoID, TEleicaoID> at +12 of CVotosEleicoesVota.
TEleicaoID CVotosEleicoesVota::EleicaoDoCargo(TCargoID cargo, int codigoErro,
                                             const std::source_location& loc) const   // name inferred
{
    const auto it = m_cargoEleicao.find(cargo);
    if (it == m_cargoEleicao.end())
        throw CUeRdvError(codigoErro, "Cargo " + std::to_string(cargo) + " nao encontrado", loc);
    return it->second;
}

// wasm func 5638  name inferred. Same shape: identical office->election map, and for every election
// of *this the other side has the same offices with the same SCargoInfo
// {TEleicaoID/codigo (+20), uebyte (+24), uebyte qtdEscolhas (+25)}.
bool CVotosEleicoesVota::EstruturaCompativel(const CVotosEleicoesVota& outro) const
{
    if (m_cargoEleicao.size() != outro.m_cargoEleicao.size())
        return false;
    for (auto a = m_cargoEleicao.begin(), b = outro.m_cargoEleicao.begin(); a != m_cargoEleicao.end(); ++a, ++b)
        if (a->first != b->first || a->second != b->second)
            return false;

    for (const auto& [eleicao, cargos] : m_eleicoes) {
        const auto it = outro.m_eleicoes.find(eleicao);
        if (it == outro.m_eleicoes.end())
            return false;
        const auto& outrosCargos = it->second.GetCargos();
        if (cargos.GetCargos().size() != outrosCargos.size())
            return false;
        for (const auto& [cargo, dados] : outrosCargos) {           // iterates the OTHER side
            const auto meu = cargos.GetCargos().find(cargo);
            if (meu == cargos.GetCargos().end())
                return false;
            const SCargoInfo& a = dados.first;
            const SCargoInfo& b = meu->second.first;
            if (a.codigo != b.codigo || a.tipo != b.tipo || a.qtdEscolhas != b.qtdEscolhas)
                return false;
        }
    }
    return true;
}

// wasm func 2296 (merged body of the five TCargoID-only queries). The std::function holds the
// per-query lambda ($_0 of Nominais/Legendas/Nulos/Brancos/Cargo, vtables @1574324..1574588).
// cvotoseleicoesvota.cpp:166/172/178/184/190, error codes 4728/4729/4730/4731/4732.
TQtdVoto CVotosEleicoesVota::Consulta(TCargoID cargo,
                                      const std::function<TQtdVoto(const CVotosCargos&, TCargoID)>& f,
                                      int codigoErro, const std::source_location& loc) const   // name inferred
{
    const TEleicaoID eleicao = EleicaoDoCargo(cargo, codigoErro, loc);
    // NOTE: the result of find() is dereferenced without an end() check (see u05 doc §14 item 3)
    return f(m_eleicoes.find(eleicao)->second, cargo);
}

// wasm func 6033 (merged body of Legenda / Partido; also the shape of 1931 = Candidato).
// cvotoseleicoesvota.cpp:140/154 (126 for Candidato), errors 4726/4727 (4725).
TQtdVoto CVotosEleicoesVota::Consulta(TCargoID cargo, const std::string& numero,
                                      const std::function<TQtdVoto(const CVotosCargos&, TCargoID,
                                                                   const std::string&)>& f,
                                      int codigoErro, const std::source_location& loc) const   // name inferred
{
    const TEleicaoID eleicao = EleicaoDoCargo(cargo, codigoErro, loc);
    return f(m_eleicoes.find(eleicao)->second, cargo, numero);
}

TQtdVoto CVotosEleicoesVota::Candidato(TCargoID c, const std::string& n) const
{ return Consulta(c, n, [](const CVotosCargos& v, TCargoID c, const std::string& n) { return v.Candidato(c, n); }, 4725); }
TQtdVoto CVotosEleicoesVota::Legenda(TCargoID c, const std::string& n) const
{ return Consulta(c, n, [](const CVotosCargos& v, TCargoID c, const std::string& n) { return v.Legenda(c, n); }, 4726); }
TQtdVoto CVotosEleicoesVota::Partido(TCargoID c, const std::string& n) const
{ return Consulta(c, n, [](const CVotosCargos& v, TCargoID c, const std::string& n) { return v.Partido(c, n); }, 4727); }
TQtdVoto CVotosEleicoesVota::Nominais(TCargoID c) const
{ return Consulta(c, [](const CVotosCargos& v, TCargoID c) { return v.Nominais(c); }, 4728); }
TQtdVoto CVotosEleicoesVota::Legendas(TCargoID c) const
{ return Consulta(c, [](const CVotosCargos& v, TCargoID c) { return v.Legendas(c); }, 4729); }
TQtdVoto CVotosEleicoesVota::Nulos(TCargoID c) const
{ return Consulta(c, [](const CVotosCargos& v, TCargoID c) { return v.Nulos(c); }, 4730); }
TQtdVoto CVotosEleicoesVota::Brancos(TCargoID c) const
{ return Consulta(c, [](const CVotosCargos& v, TCargoID c) { return v.Brancos(c); }, 4731); }
TQtdVoto CVotosEleicoesVota::Cargo(TCargoID c) const
{ return Consulta(c, [](const CVotosCargos& v, TCargoID c) { return v.Total(c); }, 4732); }

// (func 3708, not in this unit) GetVotos(TEleicaoID): find or throw
//   CUeRdvError(4733/4734/..., "Eleicao (" + std::to_string(eleicao) + ") nao encontrada")

// wasm func 2794: CVotos(std::vector<CVoto> votos) : m_votos(std::move(votos)) { m_votos.reserve(1000); }
// (ICF-shared with an unrelated vector<16-byte> helper; the class lives in md/rdv, unit u22)

}  // namespace md

// wasm func 1550: std::__tree<...>::destroy(node*) for std::map<TCargoID, TEleicaoID> (library,
// recursive post-order free of the red-black tree; called from ~CVotosEleicoesVota and the swaps above).

}  // namespace comum
