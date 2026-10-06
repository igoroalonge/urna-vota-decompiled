// FRAGMENT reconstructed by unit u19 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp (srclocs :31 and :38 here).
// The rest of the file (CMostraQRCodeBU::StartState/ProcessInput/AjustaTela, TextoPagina) is in
// cmostraqrcodebu.cpp (unit u09). Merge into that file.
//
// "BU digital" on the urna's own screen, at the end of the day: the same QR payloads as the printed BU
// (comum::CGeradorBUQRCodeVota, docs/bu/qrcode.md) but cut for the screen: max 2500 characters per QR code
// (the paper BU uses 1100). The parts are produced ONCE, on the first call, and kept in the poly-singleton
// vota::IQRCodeBUDS together with the index of the part on display.
// None of this ran in the recorded sessions (the web build never reaches the encerramento).
#include "vota/eleitor/fimvotacao/cmostraqrcodebu.h"

#include <memory>
#include <source_location>
#include <string>
#include <vector>

#include "api/pattern/cpolysingleton.h"
#include "comum/appinfo/cappinfo.h"                 // comum::GetEstado<...>
#include "comum/dados/crdvvota.h"
#include "comum/relatorios/cgeradorbuqrcodevota.h"  // comum::CGeradorBUQRCodeVota, CCabecalhoQRCode
#include "vota/comum/votadefs.h"                    // CUeVotaError

namespace vota {

// Inlined into GetQRDSInst (func 1956).
IQRCodeBUDS::IQRCodeBUDS(std::vector<std::string> qrcodes)       // vtable @1542092
    : m_qrcodes(std::move(qrcodes))
    , m_indice(0)
{
    if (m_qrcodes.empty())
        throw CUeVotaError(9372, "Vetor de partes vazio", std::source_location::current());   // :38
}

namespace {

// Builds the QR payloads of the BU (inlined into func 1956).                        // name inferred
std::vector<std::string> GeraPartesQRCodeBU()
{
    using namespace comum::md::estadoaplicacao;

    const CEstadoGeral&    eg  = comum::GetEstado<CEstadoGeral>();        // func 291 (eg.bin)
    const CEstadoGeralGap& gap = comum::GetEstado<CEstadoGeralGap>();     // inlined; if the state was not
        // loaded: CUeComumAppInfoError 7600 std::format("O estado não foi carregado: {}", "GetGap")
        // (cappinfo.cpp:42)
    const TQtdVoto comparecimento = comum::CRdvVota::GetInst().GetComparecimento();   // shared_f1269:
                                                                         // max over the eleições
    const CEstadoGeralVota& vota = comum::GetEstado<CEstadoGeralVota>();  // func 261 (vota.bin)
    const api::CDateTime dhEmissao = vota.GetDtHrEmissaoBU();             // optional @+80; if empty:
        // CUeComumDadosError 8093 "A data/hora da emissão do BU não foi registrada" (cestadogeralvota.h:159)

    comum::CCabecalhoQRCode cabecalho;                                    // func 5624 (33 empty strings)
    cabecalho.SetZona(eg.GetZona());                                      // func 5618  "ZONA:"   eg +24
    cabecalho.SetSecao(eg.GetSecao());                                    // func 5620  "SECA:"   eg +26
    cabecalho.SetIdUrna(eg.GetCarga().GetNumeroInternoUrna());            // func 5621  "IDUE:"   eg +60
    cabecalho.SetCodigoCarga(eg.GetCarga().GetCodigoCarga());             // func 5623  "IDCA:"   eg +88
    std::vector<std::string> historico;
    historico.reserve(gap.GetCorrespondencias().size() + 1);
    for (const auto& c : gap.GetCorrespondencias())                       // 96-byte entries, string at +28
        historico.push_back(c.GetCodigoCarga());
    cabecalho.SetHistoricoCargas(historico);                              // func 5622  "HIQT:"/"HICA:"

    const comum::CGeradorBUQRCodeVota gerador(cabecalho, comparecimento, dhEmissao);   // func 5603
    comum::SQRCodesBU qr = gerador.GeraQRCodes(2500);                    // func 5604 (hash chain + ASSI)
    return std::move(qr.conteudos);                                      // the signature string is dropped
                                                                         // (it is already inside the last payload)
}

// wasm func 1956 (tools name "api::CPolySingletonList::instance@1956")
// srclocs: cmostraqrcodebu.cpp:31 (this function, passed as `loc`), :38 (IQRCodeBUDS ctor),
//          cpolysingletonlist.h:129 (push inlined), cpolysingleton.h:78, cestadogeralvota.h:159, cappinfo.cpp:42
IQRCodeBUDS& GetQRDSInst()
{
    if (!api::CPolySingleton<IQRCodeBUDS>::exists()) {                   // func 2886
        std::vector<std::string> partes = GeraPartesQRCodeBU();         // generated BEFORE the accessor call
        auto& info = api::GetPolySingletonsInfo();
        // register-or-replace wrapper inlined: a second exists<IQRCodeBUDS> (2886) + erase, then push
        api::CPolySingletonList::replace<IQRCodeBUDS>(std::make_unique<IQRCodeBUDS>(std::move(partes)), info);
    }
    return api::CPolySingleton<IQRCodeBUDS>::instance(api::GetPolySingletonsInfo(),
                                                      std::source_location::current());   // :31
}

// wasm func 12053 (table slot 1506; bound as std::function<std::string()> by StartState, func 12055)
std::string QRCodeAtual()
{
    const IQRCodeBUDS& ds = GetQRDSInst();
    return ds.m_qrcodes.at(ds.m_indice);                                 // out_of_range if the index is bad
}

// wasm func 12054 (table slot 1505)
std::string TextoInstrucaoQRCode()
{
    if (GetQRDSInst().m_qrcodes.size() == 1)
        return "O QR code ao lado contém o resultado da votação para esta urna.";
    return "O QR code ao lado contém o resultado da votação para esta urna. "
           "Use as teclas 3 e 9 para navegar pelas partes do BU.";
}

}  // namespace (anonymous: the srcloc names "vota::(anonymous namespace)::GetQRDSInst()")

}  // namespace vota
