#include "ResponseParser.hpp"

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/qi_optional.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/qi_omit.hpp>
#include <boost/spirit/repository/include/qi_kwd.hpp>
#include <boost/spirit/repository/include/qi_keywords.hpp>
#include <boost/foreach.hpp>


BOOST_FUSION_ADAPT_STRUCT(
  cli::response::Result,
  (int, result)
  (std::string, message)
)

BOOST_FUSION_ADAPT_STRUCT(
  cli::response::Image,
  (int, width)
  (int, height)
  (int, size)
  (std::string, data)
)


namespace cli { namespace response {

  namespace grammar
  {
    namespace spirit = boost::spirit;
    namespace phoenix = boost::phoenix;
    namespace ascii = spirit::ascii;
    namespace qi
    {
      using namespace spirit::qi;
      using namespace spirit::repository::qi;
    }
    namespace labels = qi::labels;

    template <typename Iterator>
    struct ResultRule : qi::grammar < Iterator, Result(), ascii::space_type >
    {
      ResultRule()
        : ResultRule::base_type(r_Result)
      {
        r_Result =
          qi::lit("result:")  >> qi::int_ >>
          qi::lit("message:") >> r_QuotedMessage;

        r_QuotedMessage =
          qi::lexeme["\"" >> *(qi::char_ - '"') >> "\""];
      }

      qi::rule<Iterator, Result(), ascii::space_type > r_Result;
      qi::rule<Iterator, std::string(), ascii::space_type > r_QuotedMessage;
    };

    template <typename Iterator>
    struct ImageRule : qi::grammar < Iterator, Image(), ascii::space_type >
    {
      ImageRule()
        : ImageRule::base_type(r_Image)
      {
        r_Image =
          qi::lit("width:")   >> qi::int_ >>
          qi::lit("height:")  >> qi::int_ >>
          qi::lit("size:")    >> qi::int_ >>
          qi::lit("data:")    >> r_Base32;

        r_Base32 = *qi::char_("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567");
      }

      qi::rule<Iterator, Image(), ascii::space_type > r_Image;
      qi::rule<Iterator, std::string(), ascii::space_type > r_Base32;
    };

  } //namespace grammar

  system::error_code parseResult(std::string const &str, Result &result)
  {
    std::string::const_iterator begin = str.begin();
    std::string::const_iterator end = str.end();
    grammar::ResultRule<std::string::const_iterator> rule;

    if(!grammar::qi::phrase_parse(begin, end, rule, grammar::ascii::space, result))
      return make_error_code(system::errc::invalid_argument);

    return system::error_code();
  }

  system::error_code parseImage(std::string const &str, Image &image)
  {
    std::string::const_iterator begin = str.begin();
    std::string::const_iterator end = str.end();
    grammar::ImageRule<std::string::const_iterator> rule;

    if(!grammar::qi::phrase_parse(begin, end, rule, grammar::ascii::space, image))
      return make_error_code(system::errc::invalid_argument);

    return system::error_code();
  }


} //namespace response
} //namespace cli

