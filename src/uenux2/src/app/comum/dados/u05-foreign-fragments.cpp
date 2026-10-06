// Reconstructed from vota_web_wasm.wasm by unit u05 - FRAGMENTS of other original files.
//
// The unit builder attached these functions to u05 (uenux2/src/app/comum/dados) because they call or
// inline u05 code (CRespostas::GetInst, CRdvVota, CEleitor, CMunicipio, ...). Their real homes are
// other files, most of them owned by other units. Each section names the original file (or "(path
// inferred)") so the code can be merged there. Library template instantiations are listed at the end.
// See docs/modules/u05-uenux2-src-app-comum-dados.md §12 and §15.

// =================================================================================================
// uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp  (path inferred; class of u27)
// =================================================================================================
namespace vota {

// wasm func 10625 - vtable slot 2 (StartState) of IEleitorImpedidoVotar and of its six subclasses
// CEleitorNaoEncontrado, CEleitorOptouPorVotarEmTransito, CEleitorImpedidoJustificar,
// CEleitorNaoTemIdadeMinima, CEleitorNaoPossuiCargosParaVotar, CEleitorImpedidoJustificarVotoTransito.
void IEleitorImpedidoVotar::StartState()
{
    m_form->Exibe();          // ? member at +12, its vtable slot 2
    this->AoIniciar();        // ? own vtable slot 9 (icf_nop in the base)
    ApresentaFotoEleitor();   // func 3616 (vota/operador/comum/capresentacaofotoeleitor.cpp)
    m_proximoEstado = this;   // CState "next state" (+4) = stay in this state
}

}  // namespace vota

// =================================================================================================
// uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h  (unit u25)
// =================================================================================================
namespace comum {

// wasm func 11261 - name inferred. BU text source (table slot 2921, wrapped by comum_f604 in
// CGeraBU::StartState): header line of a party inside a proportional office.
template <typename RDV, typename DSAptos>
std::string CDataSourcesRelatorio<RDV, DSAptos>::HeaderProporcionalPartido()
{
    const TCargoID cargo = CCargos::GetInst().GetCurrent().GetCodigo();         // byte +0
    const md::CPartido& partido = CPartidos::GetInst().GetCurrent();            // ecourna_f819 cursor
    std::string linha = std::format("{}: {} - {}\n", md::CTradutorFrase::Traduz("<PLSN>"),
                                    partido.GetNumero(), partido.GetNome());     // +0 u16, +4 string
    const RDV& rdv = RDV::GetInst();
    if (rdv.Legenda(cargo, partido.GetNumero()) != rdv.Partido(cargo, partido.GetNumero())) {
        linha += CCargoDSLabelRelatorio::HeaderDetalhe();                          // two separate appends,
        linha += "\n";                                                             // 450183 = "\n" (no operator+ temp)
    }
    return linha;
}

}  // namespace comum

// =================================================================================================
// uenux2/src/app/comum/relatorios/crelutil.cpp  (unit u25; slot neighbours DSCodigoVerificador 3048,
// TituloPartido 3050)
// =================================================================================================
namespace comum {

// wasm func 11478 - name inferred. Zeresima line of one consulta answer (table slot 3051, used by
// vota::CGeraZeresimaBase::StartState). Inlines CRespostas::GetInst (crespostas.cpp:28).
std::string CRelUtil::DSLinhaRespostaZE()
{
    const uebyte digitos = CCargos::GetInst().GetCurrent().GetQtdDigitos();       // byte +12
    const md::CRespostaConsulta& resposta = CRespostas::GetInst().GetCurrent();
    const std::string numero = std::format("{:0{}}", resposta.GetNumero(), digitos);
    return "  " + api::CStringUtils::PadRight(resposta.GetNome(), 26)            // comum_f1881
               + api::CStringUtils::PadLeft(numero, ' ', 10);                   // api_f753
}

}  // namespace comum

// =================================================================================================
// uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp
// =================================================================================================
namespace vota { namespace {

// wasm func 11479 - name inferred. Text source (table slot 1096) passed by
// adicionaBaseTelaCompletaConsulta (ctelasvota.cpp:942) to api_f1191: the current answer's name.
std::string NomeRespostaCorrente()
{
    return comum::CRespostas::GetInst().GetCurrent().GetNome();   // CRespostaConsulta +4
}

} }  // namespace vota::(anonymous)

// =================================================================================================
// uenux2/src/app/comum/dados/asn/municipiozona/cconversormunicipio.cpp  (path inferred; DoConverte
// 11379 is in unit u21)
// =================================================================================================
namespace comum::asn {

// wasm func 11378 - vtable CConversorMunicipio[3]. Observed executing (data load).
md::CMunicipio CConversorMunicipio::DoDesconverte(const ModuloTiposCadastro::Municipio& entidade) const
{
    const bool comBiometria = entidade.hasOptionalField(1) ? bool(entidade.comBiometria) : false;
    return md::CMunicipio(entidade.codigo, entidade.nome, comBiometria);   // func 3712 validates
}

}  // namespace comum::asn

// =================================================================================================
// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.cpp  (class of u35)
// =================================================================================================
namespace comum::asn {

// wasm func 11389 - vtable CConversorLocalidadeEleitoral[3]
md::estadoaplicacao::CLocalidadeEleitoral CConversorLocalidadeEleitoral::DoDesconverte(const TEntidade& e) const
{
    return md::estadoaplicacao::CLocalidadeEleitoral(e.municipio, e.zona, e.secao);   // func 5631 validates
}

}  // namespace comum::asn

// =================================================================================================
// uenux2/src/app/comum/dados/celeitores.cpp  (unit u04)
// =================================================================================================
namespace comum {

namespace {
// wasm func 5771 - name inferred. Observed executing (votaInit).
// wasm signature (result, first, last, function*): the path list arrives as a begin/end pair (a
// `const vector&` of an internal function promoted by LLVM ArgumentPromotion, or an iterator pair) and
// the std::function BY VALUE (the caller clones its std::function into a temporary before each call and
// destroys the clone afterwards).
std::vector<md::CEleitor> ConcatenaEleitores(
    const std::vector<std::string>& arquivos,
    std::function<std::vector<md::CEleitor>(const std::string&)> carrega)
{
    std::vector<md::CEleitor> eleitores;
    for (const std::string& arquivo : arquivos) {
        const std::vector<md::CEleitor> parte = carrega(arquivo);   // bad_function_call if empty
        eleitores.insert(eleitores.end(), parte.begin(), parte.end());   // func 5770
    }
    return eleitores;
}
}  // namespace

// wasm func 5772. The name and the full signature come from RTTI: the lambda's typeinfo is
// comum::CEleitores::GetEleitoresEstaticos(std::string const&)::$_0 (std::function vtable @1559500,
// typeinfo @1559556), and 5772 is the function that builds that std::function. wasm params
// (result, this, dirEstatico): a const member function of CEleitores (u04 declares it the same way in
// celeitores.h). Observed executing (votaInit). Callers: CEleitores::CompleteLoad and the start-up
// code (7787).
std::vector<md::CEleitorDecorator> CEleitores::GetEleitoresEstaticos(const std::string& dirEstatico) const
{
    const std::function<std::vector<md::CEleitor>(const std::string&)> carrega =
        [](const std::string& arquivo) -> std::vector<md::CEleitor> {
            // $_0 (func 11523): partial ASN.1 decode of ModuloEleitores::EntidadeEleitores through
            // CVisitanteEleitor (iconversorparcialasn.h:52, cpartialfileasn.h:182/277/285)
            return {};
        };
    std::vector<md::CEleitor> eleitores =
        ConcatenaEleitores(MontaCaminhos(dirEstatico, m_arquivosEl), carrega);        // this+76 *-el.dat, api_f1936
    const std::vector<md::CEleitor> agregados =
        ConcatenaEleitores(MontaCaminhos(dirEstatico, m_arquivosTte), carrega);       // this+64 *-tte.dat (u04 doc)
    eleitores.insert(eleitores.end(), agregados.begin(), agregados.end());            // func 5770

    std::vector<md::CEleitorDecorator> resultado;
    resultado.reserve(eleitores.size());                                              // func 5768
    for (const md::CEleitor& eleitor : eleitores)
        // a temporary decorator (CEleitor copy-constructed in place by 946, +104 = this+100) is
        // move-appended: push_back(T&&), not emplace_back (which would construct at end() directly)
        resultado.push_back(md::CEleitorDecorator(eleitor, m_tipoIdentificadorPrincipal));
    std::sort(resultado.begin(), resultado.end());   // introsort 5769 with operator<=> (456)
    return resultado;
}

}  // namespace comum

// =================================================================================================
// uenux2/src/api/util/cstringutils.cpp  (unit u20)
// =================================================================================================
namespace api {

// wasm func 753 - name inferred. Observed executing. 15 callers (CRegraCPF/CRegraTitulo::Formata,
// CRdvVota, reports, CPedeIdentidade, ...). The insert runs under invoke_iiiii (api_f2130).
std::string CStringUtils::PadLeft(const std::string& texto, char preenchimento, std::size_t tamanho)
{
    std::string r = texto;
    if (r.size() < tamanho)
        r.insert(0, tamanho - r.size(), preenchimento);
    return r;
}

}  // namespace api

// =================================================================================================
// uenux2/src/api/util/cdate.cpp  (unit u20)
// =================================================================================================
namespace api {

// wasm func 5478 - name inferred. Adds days (month-length table @532883, leap years by %4/%100/%400);
// a negative argument calls operator-=. Used by CTradutorFrase for "<DT+N>".
CDate& CDate::operator+=(int dias);
// wasm func 5477 - name inferred. Subtracts days; a negative argument calls operator+=.
CDate& CDate::operator-=(int dias);
// wasm func 3648 - name inferred. Copy + operator-= (callers: CTradutorFrase "<DT-N>",
// vota::testeteclado::CPreZeresima).
CDate operator-(const CDate& data, int dias) { CDate r = data; r -= dias; return r; }

}  // namespace api

// =================================================================================================
// uenux2/src/app/comum/cinfomtlcd.cpp  (unit u22)
// =================================================================================================
namespace comum {

// wasm func 2284 - name inferred. Lazy singleton of the micro-terminal LCD helper (object @1838572,
// 64 bytes). The constructor it calls, func 5904, is CInfoMTLCD::CInfoMTLCD(): it stores vtable
// comum::CInfoMTLCD @1551828 at +0; the tools named it after the srcloc of its inlined member
// constructor api::BatteryIconDataSource<Vertical> (member at +4, cpowerinformation.cpp:119).
// The returned object is the `this` of CInfoMTLCD::MostrarImagemNoLCD (3836, cinfomtlcd.cpp:84).
// Callers: func 3616/3617, vota::CMostraEleitorVotando::vf2, vota::CPedeIdentidade::StartState.
CInfoMTLCD& CInfoMTLCD::GetInst()
{
    static std::unique_ptr<CInfoMTLCD> s_instancia;
    if (!s_instancia)
        s_instancia.reset(new CInfoMTLCD());
    return *s_instancia;
}

}  // namespace comum

// =================================================================================================
// uenux2/src/app/comum/dados/md/rdv/cvotos.cpp  (path from srcloc cvotos.cpp:40; unit u22)
// =================================================================================================
namespace comum::md {

// wasm func 2794 (ICF body also used by an unrelated 16-byte-element vector helper)
CVotos::CVotos(std::vector<CVoto> votos)
    : m_votos(std::move(votos))
{
    m_votos.reserve(1000);
}

}  // namespace comum::md

// =================================================================================================
// Exception factory and library template instantiations (no source of their own)
// =================================================================================================
// func 655  CBaseError<api::EUeRdvError, SErrorLimits{4650,4850}> constructor thunk:
//           ecourna_f710(exc, code, std::move(msg), srcloc) + store vtable @1560120.
// func 1550 std::__tree<std::__value_type<TCargoID,TEleicaoID>, ...>::destroy(node)   (<__tree>)
// func 5157 std::set<int>::set(std::initializer_list<int>)                              (<set>)
// func 5759 std::vector<md::CEleitorDinamico>::__push_back_slow_path(const&)  (84-byte elements)
// func 5757 std::vector<md::CEleitor>::__move_range(from_s, from_e, to)       (104-byte elements)
// func 5770 std::vector<md::CEleitor>::__insert_with_size(pos, first, last, n)   observed
// func 5768 std::vector<md::CEleitorDecorator>::__swap_out_circular_buffer   (108-byte elements)
// func 945  std::__sort3<_ClassicAlgPolicy, __less<>&, md::CEleitorDecorator*>
// func 3761 std::__sort4<...>
// func 5755 std::__sort5<...>
// func 5756 std::__insertion_sort_incomplete<...>   (switch on n = 0..5, then insertion with limit 8)
// func 5769 std::__introsort<...>                   (depth limit = 2*floor(log2 n), leftmost flag)
// The element moves inside the sort helpers are the inlined md::CEleitor::operator=(CEleitor&&)
// (func 337) plus the +104 principal-type int of CEleitorDecorator.
