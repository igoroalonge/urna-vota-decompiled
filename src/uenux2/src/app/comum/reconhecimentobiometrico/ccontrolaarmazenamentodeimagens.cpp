// uenux2/src/app/comum/reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// srclocs: :73 GerarCaminhosUnicos (IRng lookup), :117 / :127 / :135 LeChavePublica, :149 / :152 CifrarWsq
// (IUrna and IRng lookups). All of them are inlined into wasm 2725 (Armazena, name inferred).
// Functions of the unit: 2725, 5369 (error thunk), 6236 (musl utimes, library).
// Not observed executing. In the web build the method cannot store anything: the WSQ reaching it is empty
// (the fingerprint pipeline is compiled out; SalvaHabilitacaoEleitor throws 9392 first), and even with a
// non-empty image LeChavePublica throws 9000 because /dsk/fi/estatico/chave/ is not shipped (and no
// api::IKernelHSM implementation is registered).
#include "comum/reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.h"

#include <array>
#include <format>
#include <optional>
#include <string_view>

#include <sys/statfs.h>
#include <sys/time.h>

#include "api/hwil/iurna.h"
#include "api/io/asn/cfileasn.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/util/csystem.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/cpath.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/clocal.h"
#include "comum/gravadores/md/cenvelopegenerico.h"
#include "comum/md/ccabecalhoentidade.h"
#include "comum/md/cseguranca.h"
#include "ecourna/api/io/asn/serialization.hpp"
#include "ecourna/api/io/cfile.hpp"
#include "ecourna/api/security/cepesc/cplaintext.hpp"
#include "ecourna/api/security/csymmetriccipherfactory.hpp"
#include "ecourna/api/security/irng.hpp"
#include "ModuloEnvelopeChave.h"

namespace comum {

// wasm func 5369 (tools: comum_f5369) - "CUeComumReconhecimentoBiometricoError(code, msg, where)"
// constructor thunk: shared CBaseError body (ecourna func 710) with vtable @1596080.

// Line 73 - inlined into 2725. Picks a random 6-digit id whose file does not exist yet in the INTERNAL
// directory (the external path is not checked). The IRng singleton registered by main is CPrng(seed 0),
// which re-seeds a local mt19937 on every call: in this build Gera() always returns the same value.
std::tuple<std::uint32_t, std::string, std::string>
CControlaArmazenamentoDeImagens::GerarCaminhosUnicos(const std::string& dirInterno, const std::string& dirExterno,
                                                     const std::string& prefixo)
{
    auto& rng = api::CPolySingletonList::instance<ecourna::api::security::IRng>();       // line 73 (func 2030)
    std::uint32_t id;
    std::string interno, externo;
    do {
        id = static_cast<std::uint32_t>(rng.Gera()) % 999999;                             // IRng slot 3, rem_u
        const std::string nome = std::format("{}{:06}.wsq", prefixo, id);
        interno = std::format("{}{}", dirInterno, nome);
        externo = std::format("{}{}", dirExterno, nome);
    } while (api::CSystem::FileExists(interno));                                          // func 412
    return {id, interno, externo};
}

// Lines 117..135 - inlined into 2725. Same code as CGravadorBU::LeChavePublica (cgravadorbu.cpp:542..560)
// with file "wsq.pk1": an EntidadeChave whose `chave` is enciphered with a key derived from the HSM secret.
// The EntidadeChave.cifrado flag is NOT consulted: the key is always deciphered.
//
// The file name is NOT readable from this binary. The format arguments are packed as type 428 =
// (const char*, std::string_view): the directory is passed as .c_str(), and the name is a constant
// string_view {data = 117, size = 7} (i64.const 30065124469). Address 117 lies below the first data
// segment (1024) and holds 7 zero bytes at run time (read after votaInit), so in this build the path is
// "/dsk/fi/estatico/chave/" followed by seven NULs. "wsq.pk1" is inferred from the length (7) and from
// CGravadorWSQ::ValidaTipoBiometria (3796), which builds a std::string "wsq.pk1" (@353397). The real urna
// strongly supports it: /dsk/fi/estatico/chave/wsq.pk1 is in every UF block of all four 2026 urna file lists,
// and bio.sk1 is the only other 7-character key name there (2026 urna data: investigation/README.md, finding A3).
std::vector<uebyte> CControlaArmazenamentoDeImagens::LeChavePublica()
{
    static constexpr std::string_view NOME_CHAVE = "wsq.pk1";                           // see above ({117, 7}); strongly supported by the 2026 urna lists (A3)
    const std::string arquivo = std::format("{}{}", CPath::GetPathChaves().c_str(), NOME_CHAVE);  // func 1948: "/dsk/fi/estatico/chave/"
    if (!api::CSystem::FileExists(arquivo))
        throw CUeComumReconhecimentoBiometricoError(EUeComumReconhecimentoBiometricoError{9000},
                                                    "O arquivo " + arquivo + " não existe");      // line 117

    ModuloEnvelopeChave::EntidadeChave entidade;                                         // SEQUENCE @1146940
    ecourna::api::io::DeserializeFromBuffer(entidade, arquivo);      // func 2280 = DeserializeFromFile<EntidadeChave>
    const std::vector<uebyte> chaveCifrada(entidade.get_chave().begin(), entidade.get_chave().end());   // field 4

    ecourna::api::security::CSymmetricCipherFactory fabrica;                             // vptr @1113840, on the stack
    const std::vector<uebyte> segredo =
        api::CPolySingletonList::instance<api::IKernelHSM>().GetChaveCifracao();         // line 127, slot 3
    const auto cifrador = fabrica.Cria(std::string(segredo.begin(), segredo.end()));     // factory slot 2 (func 1884)
    std::vector<uebyte> chave;
    cifrador->Decifra(chaveCifrada, chave);                                              // cipher slot 3
    if (chave.empty())
        throw CUeComumReconhecimentoBiometricoError(EUeComumReconhecimentoBiometricoError{9001},
                                                    "O arquivo " + arquivo + " está vazio");      // line 135
    return chave;
}

// Lines 149..152 - inlined into 2725. CEPESC encryption of the image with the public key; identical
// sequence to the encrypted-BU branch of CGravadorBU::GravaResultado.
ecourna::api::cepesc::CCipheredOut CControlaArmazenamentoDeImagens::CifrarWsq(const std::vector<uebyte>& wsq)
{
    const CLocal& local = CLocal::GetInst();                                             // func 401
    std::array<uebyte, 1024> tabela;
    api::CPolySingletonList::instance<api::IUrna>().GetTabelaCripto(tabela.data());      // line 149, slot 4
    std::vector<uebyte> aleatorio(32, 0);
    api::CPolySingletonList::instance<ecourna::api::security::IRng>().Gera(aleatorio);  // line 152, slot 4
    const std::vector<uebyte> chavePublica = LeChavePublica();

    // comum_f5168: CPlainText(tipoArquivo 0, idCriptografia 1, zona, seção, tabela, aleatório, chave,
    // conteúdo, no CInfoSalt) - every vector passed as a fresh copy.
    const ecourna::api::cepesc::CPlainText claro(local.GetZonaID(), local.GetSecaoID(),
                                                 std::vector<uebyte>(tabela.begin(), tabela.end()),
                                                 aleatorio, chavePublica, wsq);
    return ecourna::api::cepesc::CCepescCipher().Cifra(claro);                           // vtable @1115004 slot 2 (2681)
}

// wasm func 2725 (tools: comum::CControlaArmazenamentoDeImagens::LeChavePublica).       name inferred
std::uint32_t CControlaArmazenamentoDeImagens::Armazena(const std::vector<uebyte>& wsq, const std::string& dirInterno,
                                                         const std::string& dirExterno, const std::string& prefixo)
{
    // At least 5 MiB free in the dynamic area of the EXTERNAL flash (roots table @1838576: [0] "/dsk/fi/",
    // [1] "/dsk/fe/"). If statfs itself fails the image is stored anyway.
    struct statfs info;
    if (::statfs(CPath::GetPathDinamico(EFlashOrigem::EXTERNA).c_str(), &info) == 0 &&     // wasm_entry_f1082(…, 1)
        static_cast<double>(info.f_bavail) * static_cast<double>(info.f_bsize) < 5242880.0)
        return ID_NAO_ARMAZENADA;

    const auto [id, caminhoInterno, caminhoExterno] = GerarCaminhosUnicos(dirInterno, dirExterno, prefixo);

    // comum_f2742 = singleton of a stateless 1-byte helper (the WSQ codec, see u10); the call it makes
    // was reduced to a plain copy of `wsq` in this build.                                       ?
    const std::vector<uebyte> imagem = CCodecWsq::GetInst().Prepara(wsq);                     // ? names
    if (imagem.empty())
        return ID_NAO_ARMAZENADA;

    const ecourna::api::cepesc::CCipheredOut cifrado = CifrarWsq(imagem);

    {
        ecourna::api::io::CFile arquivo(caminhoInterno, "w+b",
                                        ecourna::api::io::CFile::FM_NOATIME);            // func 517, mode 2

        // Envelope header: the pleito id and the pleito DATE at 00:00:00 - not the capture time.
        const auto& configuracao = CConfiguracaoEleicao::GetInst();                     // func 187
        const api::CDateTime dataGeracao(configuracao.GetPleito().GetData(),            // cfg +44
                                         api::EncodeTime(0, 0, 0));                     // func 3642
        const md::CCabecalhoEntidade cabecalho(dataGeracao, configuracao.GetPleito().GetId(),   // cfg +28
                                               md::ETipoCabecalho::Pleito);             // func 1945

        const EUrnaFase fase = CAppInfo::GetInst().GetEstadoGeral().GetFase();          // CEstadoGeral +48
        const CLocal& local = CLocal::GetInst();
        const std::optional<TLocalID> idLocal =
            local.EstaCarregado() ? std::optional<TLocalID>(local.GetLocalID()) : std::nullopt;   // 5739 / 1933
        const md::CSeguranca seguranca(0, 1, cifrado.GetChave());                       // func 3823

        const md::CEnvelopeGenerico envelope(cabecalho, fase, local.GetMunicipio(), local.GetZonaID(), idLocal,
                                             local.GetSecaoID(), md::CEnvelopeGenerico::Tipo::ImagemBiometria /*3*/,
                                             seguranca, cifrado.GetConteudo(), EUrnaTipo{'1'});   // comum_f3821
        api::CFileASN::CodeObjectFunction(arquivo, envelope);                           // func 2724
    }                                                                                    // CFile::Close (336)

    // Access and modification times of the stored image are reset to the epoch (1970-01-01).
    const std::array<struct timeval, 2> zero{};
    ::utimes(caminhoInterno.c_str(), zero.data());                                       // func 6236 (result ignored)
    api::CSystem::CopyFile(caminhoInterno, caminhoExterno, true /*preserva atributos: the zero times too*/);   // func 378

    return id;
}

// wasm func 6236 (tools: comum_f6236) - library: musl utimes(path, const struct timeval tv[2]) ->
// __futimesat: rejects tv_usec >= 1000000 with EINVAL, converts to timespec (usec * 1000) and calls
// utimensat(AT_FDCWD, path, ts, 0). Callers: 2725 above and SQLite's dotlockLock (utimes(path, 0)).

} // namespace comum
