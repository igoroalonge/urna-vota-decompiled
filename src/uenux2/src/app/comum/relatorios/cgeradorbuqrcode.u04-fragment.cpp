// Reconstructed from vota_web_wasm.wasm by unit u04 - FRAGMENT to be merged into
//   uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp   (file owned by unit u25)
//   uenux2/src/app/comum/relatorios/cgeradorbuqrcodevota.cpp (path inferred; class owned by u35)
//
// Why here: the pipeline attributed these functions to comum/dados/ccargos.cpp because the main one
// (wasm 5604, 22.8 KB) inlines CCargos::GetCurrentEleicaoVersaoPacote (ccargos.cpp:93). Its other
// srcloc records are cgeradorbuqrcode.cpp:405 (RetornaBlocoAssinado) and cpleito.cpp:158.
//
// What it does: builds the text payloads of the QR codes printed at the bottom of the Boletim de
// Urna (BU) and shown on screen by vota::CMostraQRCodeBU. See docs/modules/u04-... §6 for the full
// field list and an example.
//
// Functions: 5603 (CGeradorBUQRCodeVota ctor), 5604 (GeraQRCodes), 5605 (std::sort helper),
//            5606 (TotaisVotosCargo), 5608 (CCabecalhoQRCode copy ctor), 5616/5617/6028 (ORIG tests).
#include "cgeradorbuqrcode.h"

#include <algorithm>
#include <format>
#include <string>
#include <vector>

#include "../dados/ccandidaturas.h"
#include "../dados/ccargos.h"
#include "../dados/cconfiguracaoeleicao.h"
#include "../dados/celeitores.h"
#include "../dados/clocal.h"
#include "../dados/crdvvota.h"
#include "ccabecalhoqrcodebuilder.h"

namespace comum {

// ------------------------------------------------------------------------------------------------
// CCabecalhoQRCode (ccabecalhoqrcodebuilder.h, path inferred): 33 std::string fields, 396 bytes.
// Each field already holds "TAG:value " (with the trailing space) or is empty.
struct CCabecalhoQRCode {                                       // name inferred
    std::string orig;   // +0   "ORIG:VOTA " / "ORIG:RED " / "ORIG:SA "   (field name "Origem")
    std::string orlc;   // +12  "ORLC:LEG " or "ORLC:COM "                  ("OrigemProcessoEleitoral")
    std::string proc;   // +24  "PROC:<id processo eleitoral> "              ("ProcessoEleitoral")
    std::string dtpl;   // +36  "DTPL:<YYYYMMDD> "                          ("DataPleito")
    std::string plei;   // +48  "PLEI:<id pleito> "                          ("Pleito")
    std::string turn;   // +60  "TURN:<1|2> "                                ("Turno")
    std::string fase;   // +72  "FASE:<O|S|T> "                              ("Fase")
    std::string unfe;   // +84  "UNFE:<UF> " (upper-cased)                   ("Uf")
    std::string muni;   // +96  "MUNI:<cód. município> "                     ("Municipio")
    std::string zona;   // +108 "ZONA:<zona> "                               ("Zona")
    std::string seca;   // +120 "SECA:<seção> "                              ("Secao")
    std::string agre;   // +132 "AGRE:<seções agregadas> " (optional: appended only if not empty)
    std::string idue;   // +144 "IDUE:<id da urna> "                         ("IdUrna")
    std::string idca;   // +156 "IDCA:<código de carga, max 24 chars> "      ("CodigoCarga")
    std::string hica;   // +168 "HICA:<n>:<carga, max 24> ..." (+ "HIQT:")   ("HistoricoCarga")
    std::string vers;   // +180 "VERS:<versão do software> "                 ("VersaoSoftware")
    std::string loca;   // +192 "LOCA:<local de votação> "                   ("Local")
    std::string apto;   // +204 "APTO:<aptos> "                              ("QtdeAptos")
    std::string apts;   // +216 "APTS:<aptos da seção> "                     ("QtdeAptosDaSecao")
    std::string aptt;   // +228 "APTT:<aptos em TTE> "                       ("QtdeAptosTTE")
    std::string comp;   // +240 "COMP:<comparecimento> "                     ("QtdeCompareceram")
    std::string falt;   // +252 "FALT:<faltosos> "                           ("QtdeFaltosos")
    std::string hbbm;   // +264 "HBBM:<votaram habilitados por biometria> "
    std::string hbbg;   // +276 "HBBG:<votaram habilitados pelo código do mesário> "
    std::string hbsb;   // +288 "HBSB:<votaram sem biometria> "
    std::string dtab;   // +300 "DTAB:<data abertura YYYYMMDD> "
    std::string hrab;   // +312 "HRAB:<hora abertura> "
    std::string dtfc;   // +324 "DTFC:<data fechamento> "
    std::string hrfc;   // +336 "HRFC:<hora fechamento> "
    std::string junt;   // +348 (SA only, field name "Junta")
    std::string turm;   // +360 (SA only, field name "Turma")
    std::string dtem;   // +372 "DTEM:<data emissão> "                       ("DataEmissao")
    std::string hrem;   // +384 "HREM:<hora emissão> "                       ("HoraEmissao")
    // wasm func 5608 = implicitly-defined copy constructor CCabecalhoQRCode(const CCabecalhoQRCode&)
    //                  (33 string copies); comum_f2885 (not in u04) = its destructor.
};

namespace {

// wasm func 6028 - shared body produced by wasm-opt "merge similar functions" for 5616 and 5617:
// the origin value is passed as a packed string_view.
bool OrigemIgual(const CCabecalhoQRCode& cab, std::string_view origem)
{
    return cab.orig == std::format("ORIG:{} ", origem);
}
bool EhOrigemRED(const CCabecalhoQRCode& cab) { return OrigemIgual(cab, "RED"); }   // wasm 5616, name inferred
bool EhOrigemSA(const CCabecalhoQRCode& cab)  { return OrigemIgual(cab, "SA");  }   // wasm 5617, name inferred

// RetornaBlocoAssinado helpers (cgeradorbuqrcode.cpp:405, inlined in 5604)
std::string ParaHex(const std::vector<uebyte>& bytes);          // comum_f1243: upper-case hex ("0123456789ABCDEF")
std::string Trim(std::string s);                                // comum_f1374 (ecourna_f9406)

}  // namespace

// ------------------------------------------------------------------------------------------------
// wasm func 5603 - constructor of the VOTA flavour; the base-class constructor is inlined.
//   base:    vptr CGeradorBUQRCode (@1575920), m_cabecalho (+4, copy via 5608), m_rdv = CRdvVota
//            (+400), m_aptos = CEleitores::GetInst().GetQtdAptos() (+404, std::map moved in)
//   derived: vptr CGeradorBUQRCodeVota (@1575984), m_comparecimento (+416), two flags (+418, +419)
//            false, m_dataEmissao (+420, 12 bytes)
// Caller: vota::CGeraBU::StartState (12110) with comparecimento = max over the elections of
// CVotosEleicoesVota::Comparecimento (shared_f1269).
CGeradorBUQRCodeVota::CGeradorBUQRCodeVota(const CCabecalhoQRCode& cabecalho, TQtdVoto comparecimento,
                                           const api::CDateTime& dataEmissao)
    : CGeradorBUQRCode(cabecalho, CRdvVota::GetInst(), CEleitores::GetInst().GetQtdAptos())
    , m_comparecimento(comparecimento)
    , m_origemRED(false)        // +418: when true 11242 writes "ORIG:RED " instead of "ORIG:VOTA "
    , m_incluiEmissao(false)    // +419: when true 11242 writes DTEM/HREM from m_dataEmissao
    , m_dataEmissao(dataEmissao)
{
}

// ------------------------------------------------------------------------------------------------
// wasm func 5606 - name inferred. Totals block of a majoritarian or referendum cargo:
//   <aptos block of the cargo's abrangência (3696: "APTA:{} " [+ "APTS:{} APTT:{} "])>
//   "NOMI:{} BRAN:{} NULO:{} TOTC:{} "
std::string CGeradorBUQRCode::TotaisVotosCargo() const
{
    const TCargoID cargo = CCargos::GetInst().GetCurrent().GetCodigo();
    const TQtdVoto nominais = m_rdv.Nominais(cargo);    // vtable slot 6
    const TQtdVoto brancos  = m_rdv.Brancos(cargo);     // slot 9
    const TQtdVoto nulos    = m_rdv.Nulos(cargo);       // slot 8
    const TQtdVoto total    = m_rdv.Cargo(cargo);       // slot 10
    std::string s = AptosCargo();                       // wasm 3696 (misnamed "GetQtdAptos" in the db)
    s += std::format("NOMI:{} ", nominais);
    s += std::format("BRAN:{} ", brancos);
    s += std::format("NULO:{} ", nulos);
    s += std::format("TOTC:{} ", total);
    return s;
}

// ------------------------------------------------------------------------------------------------
// wasm func 5604 - name inferred (GeraQRCodes). Called by vota::CGeraBU::StartState with
// tamanhoMaximo = 1100 (and by api::CPolySingletonList::instance@1956, a misnamed caller).
// Returns every QR payload plus the signature (hex) appended to the last one.
struct SQRCodesBU {                                   // name inferred
    std::vector<std::string> conteudos;               // +0
    std::string              assinatura;              // +12
};

SQRCodesBU CGeradorBUQRCode::GeraQRCodes(std::size_t tamanhoMaximo) const
{
    // ---- 1. header ---------------------------------------------------------------------------
    const CConfiguracaoEleicao& cfg = CConfiguracaoEleicao::GetInst();
    const CLocal& local = CLocal::GetInst();
    const auto& eg = GetEstado<md::estadoaplicacao::CEstadoGeral>(ESTADO_GERAL);   // comum::GetEstado@291
    CCabecalhoQRCode cab = m_cabecalho;                                   // copy (5608)

    cab.orlc = std::format("ORLC:{} ", cfg.GetOrigem() == md::EOrigemConfiguracao::COMUNITARIA ? "COM" : "LEG");
    cab.proc = std::format("PROC:{} ", cfg.GetIdProcessoEleitoral());
    cab.dtpl = std::format("DTPL:{} ", cfg.GetPleito().GetData().Format("YYYYMMDD"));   // CDate at cfg +44
    cab.plei = std::format("PLEI:{} ", cfg.GetPleito().GetId());
    cab.turn = std::format("TURN:{} ", eg.GetDadoCarga().GetTurno());      // char '1'/'2' (eg +32)
    cab.fase = std::format("FASE:{:c} ", char(std::toupper(eg.GetDadoCarga().GetFaseChar())));
    cab.unfe = std::format("UNFE:{:2s} ", local.GetUF());
    ParaMaiusculas(cab.unfe);                                             // comum_f3509 (accent-aware)
    cab.vers = std::format("VERS:{} ", api::CStringUtils::GetVersionNumber("10.23.0.1 - DESENVOLVIMENTO"));

    PreencheCabecalho(cab);            // virtual slot 2: CGeradorBUQRCodeVota (11242) fills ORIG, MUNI,
                                       // AGRE, LOCA, APTO/APTS/APTT, COMP, FALT, HBBM/HBBG/HBSB,
                                       // DTAB/HRAB, DTFC/HRFC, DTEM/HREM
    // CCabecalhoQRCodeBuilder::preBuild()::lambda (2791): every listed field must be non-empty,
    // else EUeComumRelatoriosError 9050 "Campo ({}) não informado." (ccabecalhoqrcodebuilder.cpp:284)
    ValidaCampos({{cab.orig, "Origem"}, {cab.orlc, "OrigemProcessoEleitoral"}, {cab.proc, "ProcessoEleitoral"},
                  {cab.dtpl, "DataPleito"}, {cab.plei, "Pleito"}, {cab.turn, "Turno"}, {cab.fase, "Fase"},
                  {cab.unfe, "Uf"}, {cab.muni, "Municipio"}, {cab.zona, "Zona"}, {cab.seca, "Secao"},
                  {cab.idue, "IdUrna"}, {cab.idca, "CodigoCarga"}, {cab.hica, "HistoricoCarga"},
                  {cab.vers, "VersaoSoftware"}});
    if (EhOrigemSA(cab)) {
        ValidaCampos({{cab.junt, "Junta"}, {cab.turm, "Turma"}, {cab.dtem, "DataEmissao"}, {cab.hrem, "HoraEmissao"}});
    } else {
        ValidaCampos({{cab.loca, "Local"}, {cab.apto, "QtdeAptos"}, {cab.apts, "QtdeAptosDaSecao"},
                      {cab.aptt, "QtdeAptosTTE"}, {cab.comp, "QtdeCompareceram"}, {cab.falt, "QtdeFaltosos"}});
        if (EhOrigemRED(cab))
            ValidaCampos({{cab.dtem, "DataEmissao"}, {cab.hrem, "HoraEmissao"}});
    }

    std::string texto = cab.orig + cab.orlc + cab.proc + cab.dtpl + cab.plei + cab.turn + cab.fase +
                        cab.unfe + cab.muni + cab.zona + cab.seca;
    if (!cab.agre.empty())
        texto += cab.agre;
    texto += cab.idue + cab.idca + cab.hica + cab.vers;
    if (EhOrigemSA(cab)) {
        texto += cab.junt + cab.turm;
        texto += cab.dtem + cab.hrem;
    } else {
        texto += cab.loca + cab.apto + cab.apts + cab.aptt + cab.comp + cab.falt +
                 cab.hbbm + cab.hbbg + cab.hbsb + cab.dtab + cab.hrab + cab.dtfc + cab.hrfc;
        if (EhOrigemRED(cab))
            texto += cab.dtem + cab.hrem;
    }

    // ---- 2. body: one block per election and per cargo, in print order ----------------------
    CCargos& cargos = CCargos::GetInst();
    const CCandidaturas& candidaturas = CCandidaturas::GetInst();         // wasm_entry_f521
    cargos.OrdenaPorOrdemImpressao();                                      // comum_f3782
    std::string corpo;
    cargos.First();                                                        // 2269
    TEleicaoID eleicaoAnterior = 0;
    while (!cargos.IsEnd()) {
        const TEleicaoID eleicao = cargos.GetCurrentEleicao().GetId();
        if (eleicao != eleicaoAnterior) {
            corpo += std::format("IDEL:{} ", cargos.GetCurrentEleicaoID()) + DadosEleicao(eleicao); // slot 3 ("" for VOTA)
            eleicaoAnterior = eleicao;
        }
        const md::CCargo& cargo = cargos.GetCurrent();
        const std::vector<TCandidatoID> aptos = candidaturas.GetNumerosCandidatosAptos(cargo.GetCodigo()); // ecourna_f2840
        corpo += std::format("CARG:{} ", cargo.GetCodigo());
        corpo += std::format("TIPO:{} ", std::to_underlying(cargo.GetTipo()));  // 0 maj., 1 prop., 2 consulta

        if (aptos.empty() && cargo.TemDetalheCandidato()) {
            // candidate cargo without any apt candidate
            corpo += std::format("VERC:{} ", candidaturas.RecuperaVersaoPacote(cargo.GetCodigo()));  // 5810
            corpo += AptosCargo() + std::format("CSEC:{} ", m_rdv.Cargo(cargo.GetCodigo()));
            cargos.Next();
            continue;
        }
        if (cargo.TemDetalheCandidato())                                   // CCargo +84
            corpo += std::format("VERC:{} ", candidaturas.RecuperaVersaoPacote(cargo.GetCodigo()));
        if (cargo.TemDetalheConsulta())                                    // CCargo +136
            corpo += std::format("VERC:{} ", cargos.GetCurrentEleicaoVersaoPacote());   // ccargos.cpp:93

        if (cargo.TemDetalheCandidato() && cargo.GetTipo() == md::ETipoCargo::PROPORCIONAL) {
            // Parties in order; for each party with votes in this cargo:
            //   "PART:{p} " + "{numero}:{votos} " per candidate with votes + "LEGP:{} TOTP:{} "
            std::string partidos;
            CPartidos& lista = CPartidos::GetInst();                       // ecourna_f819 (CDataMap<uint16, CPartido>)
            for (lista.First(); !lista.IsEnd(); lista.Next()) {            // Next = 3697
                const TPartidoID p = lista.GetCurrent()->GetNumero();     // 1283
                if (m_rdv.Partido(cargo.GetCodigo(), p) == 0)              // slot 5
                    continue;
                std::string bloco = std::format("PART:{} ", p);
                std::string candidatos;
                for (const TCandidatoID numero : candidaturas.GetNumerosCandidatosAptos(cargo.GetCodigo(), p)) { // 2272
                    const md::CCandidatura* c = candidaturas.Localiza(cargo.GetCodigo(), numero);    // shared_f1273
                    const TQtdVoto votos = m_rdv.Candidato(cargo.GetCodigo(), c->GetNumero(), cargo.GetNumeroDigitos());
                    if (votos != 0)
                        candidatos += std::format("{}:{} ", c->GetNumero(), votos);
                }
                bloco += candidatos;
                bloco += std::format("LEGP:{} ", m_rdv.Legenda(cargo.GetCodigo(), p));   // slot 4
                bloco += std::format("TOTP:{} ", m_rdv.Partido(cargo.GetCodigo(), p));   // slot 5
                partidos += bloco;
            }
            std::string totais = AptosCargo();
            totais += std::format("NOMI:{} ", m_rdv.Nominais(cargo.GetCodigo()));      // slot 6
            totais += std::format("LEGC:{} ", m_rdv.Legendas(cargo.GetCodigo()));      // slot 7
            totais += std::format("BRAN:{} ", m_rdv.Brancos(cargo.GetCodigo()));       // slot 9
            totais += std::format("NULO:{} ", m_rdv.Nulos(cargo.GetCodigo()));         // slot 8
            totais += std::format("TOTC:{} ", m_rdv.Cargo(cargo.GetCodigo()));         // slot 10
            corpo += partidos + totais;
        } else if (cargo.TemDetalheCandidato() && cargo.GetTipo() == md::ETipoCargo::MAJORITARIO) {
            // every candidacy of this cargo (all situations) with at least one vote
            std::string candidatos;
            for (auto& c : candidaturas.Todas()) {                         // CDataMap<uint32, CCandidatura>
                if (c.GetCargo() != cargo.GetCodigo())
                    continue;
                const TQtdVoto votos = m_rdv.Candidato(cargo.GetCodigo(), c.GetNumero(), cargo.GetNumeroDigitos());
                if (votos != 0)
                    candidatos += std::format("{}:{} ", c.GetNumero(), votos);
            }
            corpo += candidatos + TotaisVotosCargo();                      // 5606
        } else {
            // referendum ("consulta"): answers sorted by number (5605 = std::__introsort on 28-byte
            // md::CRespostaConsulta{int numero; string resposta; string textoFonetico})
            std::vector<md::CRespostaConsulta> respostas = cargo.GetDetalheConsulta().GetRespostas();   // 1923
            std::ranges::sort(respostas, {}, &md::CRespostaConsulta::numero);
            std::string opcoes;
            for (const auto& r : respostas) {
                const TQtdVoto votos = m_rdv.Candidato(cargo.GetCodigo(), r.numero, cargo.GetNumeroDigitos());
                if (votos != 0)
                    opcoes += std::format("{}:{} ", r.numero, votos);
            }
            corpo += opcoes + TotaisVotosCargo();                          // 5606
        }
        cargos.Next();                                                     // 1708
    }
    texto += corpo;

    // ---- 3. split into QR-sized parts ----------------------------------------------------------
    // 277 characters are reserved for "QRBU:i:n VRQR:6.0 ", " HASH:<128 hex>" and " ASSI:...".
    const std::size_t limite = tamanhoMaximo - 277;      // 823 for the BU (tamanhoMaximo = 1100)
    std::vector<std::string> partes;
    for (std::size_t pos = 0; pos != texto.size();) {
        std::string parte = texto.substr(pos, limite);
        std::size_t tam = parte.size();
        if (tam >= limite && parte.back() != ' ') {
            // cut at the last space (the space goes to the next part and is trimmed there)
            tam = parte.rfind(' ');                        // hand-written backwards scan in the binary
            parte = texto.substr(pos, tam);
            // No guard (doc §9.2): tam == npos -> the whole remainder is taken and pos += npos moves
            // pos back by one (at pos 0: substr throws out_of_range); tam == 0 (the window's only
            // space is its first character, which is what follows every cut) -> an empty part is
            // pushed and pos does not move: endless loop.
        }
        partes.push_back(Trim(parte));
        pos += tam;
    }

    // ---- 4. hash chain, numbering and signature (RetornaBlocoAssinado, line 405) --------------
    SQRCodesBU resultado;
    std::vector<std::string> blocos;                     // "<parte> HASH:<hash>" of every part so far
    std::string hash;
    for (std::size_t i = 0; i < partes.size(); ++i) {
        // part 0: SHA-512(parte0);  part i>0: SHA-512(join(blocos, " ") + " " + parte_i)
        const std::string entrada = (i == 0) ? partes[0] : std::format("{} {}", Join(blocos, " "), partes[i]);
        ecourna::api::security::CSha sha("SHA2-512");     // ecourna_f2684 (EVP, 64-byte digest)
        sha.Reset();
        for (std::size_t off = 0; off < entrada.size(); off += 512)
            sha.Update(std::vector<uebyte>(entrada.begin() + off,
                                           entrada.begin() + std::min(off + 512, entrada.size())));
        hash = ParaHex(sha.Finish());                     // 128 upper-case hex characters
        blocos.push_back(std::format("{} HASH:{}", partes[i], hash));

        std::string qr = std::format("QRBU:{}:{} VRQR:{} {}", i + 1, partes.size(), "6.0", blocos.back());
        if (i == partes.size() - 1) {
            // RetornaBlocoAssinado: sign the raw bytes of the last hash with the urna's key through
            // the PKCS#11 singleton (api::pkcs11::IPkcs11: slot 23 open/login?, slot 6 sign,
            // slot 24 close?). NOTE: no IPkcs11 implementation is registered in the web build: the
            // first check in 3704 (cpolysingleton.h:78) throws EPatternErr 1301 "PolySingleton -
            // solicitada uma instancia nao criada N3api6pkcs117IPkcs11E ...".
            auto& pkcs11 = api::CPolySingletonList::instance<api::pkcs11::IPkcs11>();   // 3704
            pkcs11.Abre();                                                              // slot 23 ?
            const std::vector<uebyte> assinatura =
                pkcs11.Assina(ecourna::api::util::CStringUtils::HexStringToBytes(hash)); // slot 6
            pkcs11.Fecha();                                                             // slot 24 ?
            resultado.assinatura = ParaHex(assinatura);
            qr += std::format(" ASSI:{}", resultado.assinatura);
        }
        resultado.conteudos.push_back(std::move(qr));
    }
    return resultado;
}

}  // namespace comum
