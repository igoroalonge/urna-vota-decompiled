// Unit u36 - "app:comum: classes without known file": the rest of the unit that is not a function of one
// original source file. Reconstructed from vota_web_wasm.wasm. See docs/modules/u36-app-comum-classes-
// without-known-file.md for the complete index -> symbol -> file map.
//
// Three kinds of code live here:
//   A. wasm-opt "merge-similar-functions" bodies: N source functions that differed only in constants (a
//      std::source_location, an error code, a vtable, a string literal) were folded into ONE body that takes
//      the constants as extra parameters; each source function survives as a 1-line thunk. The per-class
//      functions are reconstructed by the owner units (files named below); this file shows the shared body
//      once, as a helper, so that its exact behaviour is documented.
//   B. compiler-generated special members of comum types (bodies of `= default`).
//   C. static-storage destructors ("atexit" handlers) and C++ library template instances, as a list.
//
// Everything in A/B is plain translation of the wasm; names marked "name inferred" are ours.

#include <filesystem>
#include <format>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>

namespace comum {

// =========================================================================================================
// A. merge-similar bodies
// =========================================================================================================

namespace dao {
// wasm func 3894 - body of the three DAO Clonar() (IDAO vtable slot 2):             (name inferred: u20)
//   CComparecimentoMesarioDAO::Clonar (10399), CJustificadorDAO::Clonar (11468), CEleitorDinamicoDAO::Clonar
//   (11541), each = comum_f3894(this, <its vtable>).
// A DAO is 12 bytes {vptr, std::shared_ptr<conexão SQLite>}; the clone copies the vptr and the shared_ptr
// (use_count + 1). Source form (in each DAO .cpp):  IDAO* CXxxDAO::Clonar() const { return new CXxxDAO(*this); }
}  // namespace dao

// wasm func 3896 - body of the three IServicoEstado<...>::GetPathArquivo() (vtable slot 2):
//   CServicoEstadoGeralVota (11568: srcloc cservicoestadogeralvota.cpp:32, code 7607, "vota.bin"),
//   CServicoEstadoGeralGap (11567, "gap.bin"), CServicoEstadoGeralSA (11565, "sa.bin")
//   -> reconstructed in comum/appinfo/servicos/cservicoestadogeral{vota,gap,sa}.cpp (unit u20).
// Service object: +0 vptr, +4 EFlashOrigem m_midia, +8 EUrnaTurno m_turno.
// Executed: 18 calls in votaInit (entry counter, review run); 6030 below: 4..8 calls; 5552: none, 5556: 11..12.
template <class SERVICO>
std::filesystem::path GetPathArquivoTurno(const SERVICO& servico, const std::source_location& onde,
                                          const EUeComumAppInfoError codigo, std::string_view arquivo)
{
    if (servico.GetTurno() == EUrnaTurno{'0'})                                   // "sem turno"
        throw CUeComumAppInfoError(codigo, "Urna sem turno em contexto onde turno era esperado", onde);
    return CPath::GetPathTrab(servico.GetMidia(), servico.GetTurno()) / std::string(arquivo);   // func 358
}

// wasm func 6008 - body of the two CNomeMesariosUrnaDS::Text(formato) (table slots 4386 / 4379):
//   cpededigitalmesario.cpp (10319, srcloc :47) and cdigitalmesarionaoreconhecida.cpp (10323, srcloc :27),
//   reconstructed there by u22. Behaviour: IControladorRegistraMesarios from the poly-singleton list (func 356
//   with the srcloc); if the mesário is NOT a voter of this section (slot 13) the raw título (slot 17) is
//   returned WITHOUT applying `formato`; otherwise a copy of the voter's CEleitorDecorator (func 946) is made
//   and std::vformat(formato, first 40 BYTES of nome social, or of nome when the social name is empty).
//
// wasm func 6009 - body of CTituloMesarioJaRegistradoDS::Text (10344, :29) and CTituloMesarioInvalidoDS::Text
//   (10349, :28): std::vformat(formato, IControladorRegistraMesarios::GetTituloMesario()).
//
// wasm func 6010 - body of CTituloMesarioVazio::StartState (10353, srcloc :57) and
//   CTituloMesarioInvalido::StartState (10348, :75):
//       m_proximoEstado = this; m_formMT(+12)->Show(); m_formEleitor(+20)->Show();
//       CPolySingletonList::instance<IControladorRegistraMesarios>(onde).LogaTituloInvalido();   // slot 26
//   NB: because the two bodies were identical up to the srcloc, an EMPTY título is logged exactly like an
//   invalid one: in VOTA slot 26 is CControladorRegistraMesariosVota::vf26 (10776), CLoga::loga(level 2,
//   "Digitado título inválido para o registro de mesário").

namespace asn {
// wasm func 6030 - body of the enum converters' DoDesconverte (vtable slot 3):
//   CConversorTipoIdentificadorEleitor (11356: :51, code 8184, 3 values, "Tipo de identificador de eleitor
//   inválido") and CConversorOrigemConfiguracao (11366: :38, code 7953, 2 values, "Origem configuração
//   inválida"). Unsigned test: values < 1 or > quantidade throw CUeComumDadosError.
inline int DesconverteEnumerado(const int valor, const std::source_location& onde, const char* mensagem,
                                const EUeComumDadosError codigo, const unsigned quantidade)
{
    if (static_cast<unsigned>(valor - 1) >= quantidade)
        throw CUeComumDadosError(codigo, mensagem, onde);                       // func 170, typeinfo @1528076
    return valor;
}
}  // namespace asn

namespace md {
// wasm func 6044 - body of CVersoesContratos::GetValor (3825: cversoescontratos.cpp:42, code 8697) and
//   CDependenciasContratos::GetValor (3826) - reconstructed by u23 in gravadores/md/. Lookup through
//   api::CIniSection::Find (10877); a missing key throws CUeComumGravadoresError(code,
//   std::format("Propriedade inexistente: {}", chave), onde); else returns a copy of the key's VALUE string
//   (CIniKey +12, node +40). In the simulator both .properties files are 0-byte stubs.
}  // namespace md

// wasm func 6043 - body of the complete-object destructors ~CGravadorRDV (11586: member at +56) and
//   ~IGravadorEnvelope (11625: member at +76), each = comum_f6043(this, offset, own vtable):
//       vptr = own vtable; m_correspondencia.~CDadoCorrespondencia();   // func 857, 96-byte member
//       vptr = IResultado (@1541028); m_nome.~string();                  // IResultado +20
//   i.e. `~CGravadorRDV() = default;` / `~IGravadorEnvelope() = default;` with IGravador's empty dtor inlined.

// =========================================================================================================
// B. compiler-generated special members
// =========================================================================================================
// wasm func 5744  md::CLocal::~CLocal()  (dados/md/clocal.cpp) - implicit: resets
//     std::optional<CSecaoEleitoral> (+84, flag +120; frees its vector<CIdentificacaoAgregada> at +108),
//     then the strings nome do município (+44), nome da UF (+28), sigla da UF (+16), país (+4).
//     Callers: CLocal::~CLocal (2261) and api::CFileASN::ReadFromFile<md::CLocal> (5740). Executed: 5 calls in
//     votaInit (entry counter, review run).
// wasm func 5760  md::CEleitorDinamico::CEleitorDinamico(const CEleitorDinamico&)  - implicit copy (84 bytes:
//     two CEleitorIdentidade, 7 scalars, optional<CEleitorIdentidade> tituloMesario (flag +72), foto pair).
//     Callers: vector<CEleitorDinamico>::__push_back_slow_path (5759) and CRegistraDigitalOperador (3614).
// wasm func 5763  std::pair<const md::CEleitorIdentidade, CEleitorDetalhe>::pair(pair&&) - implicit move of a
//     CEleitores map value (220 bytes; the const key string is COPIED, everything else moved). Used when the
//     map nodes (236 bytes) are built: CEleitores::CompleteLoad (6734) and the start-up function (7787).
//     Executed: 1 call in votaInit (entry counter).
// wasm func 5552 / 5556  api::CApplicationContext move assignment / operator== (api/gui/capplicationcontextstack.u36.cpp)

// =========================================================================================================
// C1. static-storage destructors. The web build has EXIT_RUNTIME=0: __cxa_atexit is a no-op and no
//     registration survives, so NONE of these ever runs; they remain only because the function table keeps
//     their addresses. "mutex" = the no-op ~mutex residue (func 150 on the address); "unique_ptr" = reset.
// =========================================================================================================
//   10274  std::vector<uebyte> @1910004 (named s_hashAnterior by u23) - hash chain of the BU entities (CAssinaVotavelBU,
//          gravadores/asn/cassinavotavelbu.cpp; used by CConversorEntidadeBU::DoConverte 10273)
//   (no reference to any of these handlers' table slots exists in the code: nothing passes them to __cxa_atexit)
//   10320 / 10321  CPedeDigitalMesario::GetInst: mutex @1909880 / unique_ptr @1909904 (shared_f3886)
//   10334 / 10336  CGestorDadoMesarioInicial::GetInst: mutex @1909740 / unique_ptr @1909764
//   10346          CTituloMesarioJaRegistrado::GetInst: unique_ptr @1909708 (unknown_f763 = virtual delete)
//   10350 / 10351  CTituloMesarioInvalido::GetInst: mutex @1909656 / unique_ptr @1909680
//   10354 / 10355  CTituloMesarioVazio::GetInst: mutex @1909628 / unique_ptr @1909652
//   10371          CPedeTituloMesarioFinal::GetInst: mutex @1909544
//   10378          CPedeTituloMesarioInicial::GetInst: mutex @1909516
//   10381 / 10382  CEncerraRegistroMesarios::GetInst (api_f5388): mutex @1909488 / unique_ptr @1909512
//   10386 / 10387  CConfirmaFimRegistroMesarios::GetInst (5389): mutex @1909460 / unique_ptr @1909484
//   10389 / 10390  CPedeTituloMesario::GetInst (2729): mutex @1909432 / unique_ptr @1909456
//   10393 / 10394  CRegistrarMesarios::GetInst (3610): mutex @1909404 / unique_ptr @1909428
//   10519 / 10520  fingerprint-extractor configuration singleton (vota_f1903, 16 bytes {8 bpp?, 500 dpi,
//                  196.85 = 500/2.54 pixels per cm}): mutex @1908640 / unique_ptr @1908664
//   10521 / 10522  WSQ-encoder singleton (comum_f2742, 1-byte object): mutex @1908612 / unique_ptr @1908636
//   11174          CSubstituidorTitulo::s_mutex @1839116 (relatorios/csubstituidortitulo.cpp)
//   11268          static std::vector<uebyte> certificado @1839076 of CEstadoGeral::RecuperarCertificado (5635)
//   11480          CRespostas::GetInst mutex @1838960 (dados/crespostas.cpp)
//   11639 / 11640  CLogComum::s_mutex @1838612 / s_pInstancia @1838636 (log/clogcomum.cpp)
//   11648          CPath::ms_raiz @1838600 ("/")
//   11649          CPath::ms_raizesFlash[2] @1838576 ("/dsk/fi/", "/dsk/fe/"), destroyed last-to-first
//   11657          CArquivosSavd::GetInst mutex @1838520 (carquivossavd.cpp)
//   11659 / 11660  CArquivosResultado::GetInst: mutex @1838492 / unique_ptr @1838516 (carquivosresultado.cpp)
//
// =========================================================================================================
// C2. C++ library template instances attributed to app:comum by the tools (no TSE logic)
// =========================================================================================================
//   3368   std::__split_buffer<__dir_stream*, allocator<__dir_stream*>>::push_back(pointer&&)   (deque map)
//   4778   std::unique_ptr<__dir_stream, __allocator_destructor<allocator<__dir_stream>>>::~unique_ptr
//   4779   std::deque<__dir_stream>::__back_spare()          (block = 51 x 80-byte __dir_stream = 4080 B)
//   8097   std::deque<__dir_stream>::pop_back()              (+ free a spare block)
//   8099   std::deque<__dir_stream>::push_back(__dir_stream&&) (__add_back_capacity inlined)
//          -> the directory stack of std::filesystem::recursive_directory_iterator (8096/8101), used by
//             ecourna's ICompressor::Add glob (CGravadorWSQ::CompactaWsq, 5821)
//   3870   std::map<TEleicaoID, std::string>::__emplace_hint_unique_key_args (map copy: CPleito versions)
//   5681   std::map<md::SCargoInfo, md::CVotos>::__emplace_hint_unique_key_args (CVotosCargos map copy)
//   5717 / 5718 / 5719  std::__sort5 / __insertion_sort_incomplete / __introsort for
//          md::CIdentificacaoAgregada (8 bytes, key = uint16 seção at +4): std::sort of the aggregated
//          sections in CConversorSecaoEleitoral::DoDesconverte (11453)
//   5956   std::vector<md::CMunicipioDisponivel>::__destroy_vector::operator() (56-byte items with a
//          vector<CSecaoEleitoral>, whose items own a vector at +24) - CConversorDadosDisponiveisCarga,
//          vota::CImpressaoVersaoPacotes
//   9404   std::vector<std::string>::__emplace_back_slow_path<const std::string&, size_t&, size_t&>
//          (= emplace_back(str, pos, n), used by api::CStringUtils::Split, 1880)
//   9873   std::_AllocatorDestroyRangeReverse<allocator<CDadoCorrespondencia>, CDadoCorrespondencia*>::operator()
//          (exception rollback of a vector<CDadoCorrespondencia> copy; table slot 473)
//   11140  api::CriaForm<api::CPreShowClearScreen>(CFormBuilder&, const std::string& nome): merge-similar
//          thunk (body 6117) = std::shared_ptr<IForm<IScreen>>(new IForm(builder, shared_ptr<IPreShow>(
//          new CPreShowClearScreen)), nome) - a NON-interactive form (vota::CriaFormVota = 886 is the twin
//          with CPreShowFormVota). Used by CriaFormEleitorRegistroMesarios (1255).
//   7765 7768 7770 7772 7774 7776  libc++ locale.cpp static-array destructors of __time_get_c_storage:
//          <char>::__x() "%m/%d/%y" (7765), <wchar_t>::__am_pm (7768), <char>::__am_pm (7770),
//          <wchar_t>::__months (7772), <char>::__months (7774), <wchar_t>::__weeks (7776).

}  // namespace comum
