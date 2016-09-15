#include "CommandsParser.hpp"
#include "Commands.hpp"

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
    srv::cmd::Vector3,
    (float, x)
    (float, y)
    (float, z)
)

BOOST_FUSION_ADAPT_STRUCT(
    srv::cmd::Common,
    (boost::optional<bool>, async)
)

BOOST_FUSION_ADAPT_STRUCT(
    srv::cmd::Arm,
    (srv::cmd::Common, common)
)

BOOST_FUSION_ADAPT_STRUCT(
    srv::cmd::Disarm,
    (srv::cmd::Common, common)
)

BOOST_FUSION_ADAPT_STRUCT(
    srv::cmd::TakeOff,
    (srv::cmd::Common, common)
    (float, altitude)
)

BOOST_FUSION_ADAPT_STRUCT(
    srv::cmd::Land,
    (srv::cmd::Common, common)
)

BOOST_FUSION_ADAPT_STRUCT(
    srv::cmd::PositionSetpoint,
    (srv::cmd::Common, common)
    (srv::cmd::Vector3, position)
    (boost::optional<float>, yaw)
    (boost::optional<bool>, relative)
    (boost::optional<bool>, body_frame)
)

BOOST_FUSION_ADAPT_STRUCT(
    srv::cmd::VelocitySetpoint,
    (srv::cmd::Common, common)
    (srv::cmd::Vector3, velocity)
    (boost::optional<float>, yaw_rate)
    (boost::optional<bool>, relative)
    (boost::optional<bool>, body_frame)
)

BOOST_FUSION_ADAPT_STRUCT(
    srv::cmd::AttitudeSetpoint,
    (srv::cmd::Common, common)
    (srv::cmd::Vector3, rpy)
    (float, thrust)
)

BOOST_FUSION_ADAPT_STRUCT(
    srv::cmd::GetImage,
    (srv::cmd::Common, common)
)



namespace srv { namespace cmd {

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
	struct Rules : qi::grammar<Iterator, Command(), ascii::space_type >
	{
		Rules()
			: Rules::base_type(r_Command)
		{
			r_Command =
				r_Arm 				|
				r_Disarm 			|
				r_TakeOff			|
				r_Land				|
				r_PositionSetpoint	|
				r_VelocitySetpoint	|
				r_AttitudeSetpoint	|
				r_GetImage;

			r_Arm =
				qi::lit("arm") >>
				r_Common;

			r_Disarm =
				qi::lit("disarm") >>
				r_Common;

			r_TakeOff =
				qi::lit("take_off") >>
				r_Common >>
				qi::lit("altitude:") >> qi::float_;

			r_Land =
				qi::lit("land") >>
				r_Common;

			r_PositionSetpoint =
				qi::lit("position_setpoint") >>
				r_Common >>
				qi::lit("position:") 		>> r_Vector3 	>>
				-( qi::lit("yaw:") 			>> qi::float_ ) >>
				-( qi::lit("relative:") 	>> qi::bool_ ) 	>>
				-( qi::lit("body_frame:") 	>> qi::bool_ );

			r_VelocitySetpoint =
				qi::lit("velocity_setpoint") >>
				r_Common >>
				qi::lit("velocity:") 		>> r_Vector3 	>>
				-( qi::lit("yaw_rate:") 	>> qi::float_ ) >>
				-( qi::lit("relative:") 	>> qi::bool_ ) 	>>
				-( qi::lit("body_frame:") 	>> qi::bool_ );

			r_AttitudeSetpoint =
				qi::lit("attitude_setpoint") 	>>
				r_Common >>
				qi::lit("rpy:") 				>> r_Vector3 	>>
				qi::lit("thrust:") 				>> qi::float_;

			r_GetImage =
				qi::lit("get_image") >>
				r_Common;

			r_Common =
				-( qi::lit("async:") >> qi::bool_ );

			r_Vector3 =
				qi::lit("{") >>
				qi::float_ >> qi::lit(",") >>
				qi::float_ >> qi::lit(",") >>
				qi::float_ >>
				qi::lit("}");

		}

		qi::rule<Iterator, Command(), ascii::space_type > r_Command;
		qi::rule<Iterator, Arm(), ascii::space_type > r_Arm;
		qi::rule<Iterator, Disarm(), ascii::space_type > r_Disarm;
		qi::rule<Iterator, TakeOff(), ascii::space_type > r_TakeOff;
		qi::rule<Iterator, Land(), ascii::space_type > r_Land;
		qi::rule<Iterator, PositionSetpoint(), ascii::space_type > r_PositionSetpoint;
		qi::rule<Iterator, VelocitySetpoint(), ascii::space_type > r_VelocitySetpoint;
		qi::rule<Iterator, AttitudeSetpoint(), ascii::space_type > r_AttitudeSetpoint;
		qi::rule<Iterator, GetImage(), ascii::space_type > r_GetImage;
		qi::rule<Iterator, Common(), ascii::space_type > r_Common;
		qi::rule<Iterator, Vector3(), ascii::space_type > r_Vector3;
	};

} //namespace grammar

system::error_code parse(std::string const &str, Command &command)
{
	std::string::const_iterator begin = str.begin();
	std::string::const_iterator end = str.end();
	grammar::Rules<std::string::const_iterator> rules;

	if(!grammar::qi::phrase_parse(begin, end, rules, grammar::ascii::space, command))
		return make_error_code(system::errc::invalid_argument);

	return system::error_code();
}

} //namespace cmd
} //namespace srv
