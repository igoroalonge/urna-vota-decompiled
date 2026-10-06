// uenux2/src/app/comum/dados/asn/eleitor/cconversorbiometriaeleitorcifrada.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// Encrypted biometrics of a voter, stored inside the voter file (EleitorUrna.biometria):
//   BiometriaEleitorCifrada ::= SEQUENCE { composicaoBiometria ComposicaoBiometria, conteudo OCTET STRING,
//                                          salt OCTET STRING }
// Decryption chain (all inlined into one function):
//   1. LeChave(): read /dsk/fi/estatico/chave/bio.sk1 (ModuloEnvelopeChave::EntidadeChave), take its `chave`
//      OCTET STRING and decrypt it with a symmetric cipher keyed by a secret obtained from the api::IKernelHSM
//      singleton -> "chave secreta".
//   2. CEPESC (the government cipher of CEPESC/ABIN, library ecourna::api::cepesc): CCipheredIn(0, 1, 0,
//      chave de sessão (CParametro), chave secreta, conteudo, CInfoSalt(salt, info)) -> CCepescCipher::Decifra.
//   3. BER-decode the plaintext as ModuloEleitores::BiometriaEleitor and convert it with CConversorBiometriaEleitor.
// composicaoBiometria is never read.
// WEB BUILD: CCepescCipher::Decifra (func 5171, appended below) returns the "ciphertext" unchanged, and no
// scenario ships bio.sk1 or voters with biometrics, so this path is dead in the simulator.
#include "comum/dados/asn/eleitor/cconversorbiometriaeleitorcifrada.h"

#include <filesystem>
#include <memory>
#include <string>

#include "api/io/cfileasn.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/security/ikernelhsm.h"
#include "comum/cpath.h"
#include "comum/dados/asn/eleitor/cconversorbiometriaeleitor.h"
#include "ecourna/api/io/serialize.h"
#include "ecourna/api/security/cepesc/ccepesccipher.h"
#include "ecourna/api/security/cepesc/ccipheredin.h"
#include "ecourna/api/security/cepesc/cinfosalt.h"
#include "ecourna/api/security/csymmetriccipherfactory.h"
#include "ModuloEnvelopeChave.h"

namespace comum::asn {

// Inlined into wasm func 11419 (srcloc lines 91, 101, 109)
std::vector<uebyte> CConversorBiometriaEleitorCifrada::LeChave() const
{
    const std::filesystem::path arquivo = CPath::GetDiretorioChaves() / "bio.sk1";   // func 1948: "/dsk/fi/estatico/chave/"
    if (!api::CSystem::FileExists(arquivo)) {                                         // api_f412
        throw CDadosError(7894, "O arquivo " + arquivo.string() + " não existe");     // line 91
    }

    ModuloEnvelopeChave::EntidadeChave entidade;
    ecourna::api::io::DeserializeFromFile(entidade, arquivo.string());   // BER decode of the key envelope
    const std::vector<uebyte> chaveCifrada(entidade.get_chave().begin(), entidade.get_chave().end());
    // NOTE: the `cifrado` flag and `tipo` of the envelope are ignored: the key is always decrypted.

    auto& hsm = api::CPolySingletonList::instance<api::IKernelHSM>();   // srcloc line 101 (column 40)
    const std::vector<uebyte> segredo = hsm.GetChave();                 // IKernelHSM vtable slot 3   // name inferred
    const std::shared_ptr<ecourna::api::security::ISymmetricCipher> cifrador =
        ecourna::api::security::CSymmetricCipherFactory().Cria(std::string(segredo.begin(), segredo.end()));   // ?
    std::vector<uebyte> chave;
    cifrador->Decifra(chaveCifrada, chave);   // vtable slot 3 of the cipher   // name inferred
    if (chave.empty()) {
        throw CDadosError(7895, "O arquivo " + arquivo.string() + " está vazio");   // line 109
    }
    return chave;
}

// wasm func 11419 (vtable slot 3; the tool named it "vf3")
md::CBiometriaEleitor CConversorBiometriaEleitorCifrada::DoDesconverte(const TEntidade& biometria,
                                                                       const md::CParametro& parametro) const
{
    ModuloEleitores::BiometriaEleitor claro;

    const std::vector<uebyte> chaveSessao = parametro.GetChaveSessao();   // copy of CParametro +16
    std::vector<uebyte> textoClaro;
    {
        const std::vector<uebyte> conteudo(biometria.get_conteudo().begin(), biometria.get_conteudo().end());
        const std::vector<uebyte> chaveSecreta = LeChave();
        const std::vector<uebyte> salt(biometria.get_salt().begin(), biometria.get_salt().end());
        const std::vector<uebyte> info(parametro.GetInfo().begin(), parametro.GetInfo().end());   // CParametro +0 (?)

        // CCipheredIn(uebyte, uebyte, uebyte, sessão, secreta, conteúdo, const CInfoSalt&) (inlined,
        // ecourna/api/security/cepesc/ccipheredin.cpp):
        //   empty sessão  -> CBaseError<ESecurityError>(1501, "chave de sessão vazia.")   :79
        //   empty secreta -> CBaseError<ESecurityError>(1502, "chave secreta vazia.")     :82
        //   empty conteúdo-> CBaseError<ESecurityError>(1503, "conteúdo vazio.")          :85
        // The three leading bytes are {0, 1, 0}; the CInfoSalt is kept in a shared_ptr (+40).
        const ecourna::api::cepesc::CCipheredIn cifrado(0, 1, 0, chaveSessao, chaveSecreta, conteudo,
                                                        ecourna::api::cepesc::CInfoSalt(salt, info));
        textoClaro = ecourna::api::cepesc::CCepescCipher().Decifra(cifrado);   // func 5171
    }

    // api::CFileASN::DecodeObject<ModuloEleitores::BiometriaEleitor>(buffer, "DecodeObject de " + typeid name)
    // (inlined, cfileasn.h):
    //   BER decode fails -> CBaseError<api::EUeIoError>(5953, "Conteúdo não foi decodificado para {}: {}")  :135
    //   invalid result   -> CBaseError<api::EUeIoError>(5954, "Conteúdo inválido para {}: {}")             :143
    claro = api::CFileASN::DecodeObject<ModuloEleitores::BiometriaEleitor>(
        std::vector<char>(textoClaro.begin(), textoClaro.end()),
        std::string("DecodeObject de ") + typeid(ModuloEleitores::BiometriaEleitor).name());

    return CConversorBiometriaEleitor().Desconverte(claro);
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Emitted in this TU (the tool attributes it here because 11419 is its only caller):
//
// wasm func 5171 — ecourna::api::cepesc::CCepescCipher::Decifra(const CCipheredIn&)   // name inferred
// Real home: ecourna-lib/ecourna/api/security/cepesc/ccepesccipher.cpp (path inferred). Slot 3 of
// IAsymmetricCipher<CPlainText, CCipheredOut, CCipheredIn, std::vector<uebyte>>; slot 2 (func 2681, unit u40) is
// the encrypt direction and builds a CCipheredOut with a 32-byte ZERO session key around the unchanged plaintext.
// In this build CEPESC is therefore a pass-through: "decryption" is a copy of the input content (+28).
//
//   std::vector<uebyte> ecourna::api::cepesc::CCepescCipher::Decifra(const CCipheredIn& in) const
//   {
//       return in.GetConteudo();   // copy of the vector at CCipheredIn +28; no key, salt or algorithm used
//   }
