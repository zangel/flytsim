#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "Config.hpp"

namespace srv { namespace cmd {

	struct Vector3
	{
		float x, y, z;
	};

	struct Common
	{
		optional<bool> async;
	};

	struct Arm
	{
		Common common;
	};

	struct Disarm
	{
		Common common;
	};

	struct TakeOff
	{
		Common common;
		float altitude;
	};

	struct Land
	{
		Common common;
	};

	struct PositionSetpoint
	{
		Common common;
		Vector3 position;
		optional<float> yaw;
		optional<bool> relative;
		optional<bool> body_frame;
	};

	struct VelocitySetpoint
	{
		Common common;
		Vector3 velocity;
		optional<float> yaw_rate;
		optional<bool> relative;
		optional<bool> body_frame;
	};

	struct AttitudeSetpoint
	{
		Common common;
		Vector3 rpy;
		float thrust;
	};

	struct GetImage
	{
		Common common;
	};

	typedef boost::variant
	<
		Arm,
		Disarm,
		TakeOff,
		Land,
		PositionSetpoint,
		VelocitySetpoint,
		AttitudeSetpoint,
		GetImage
	> Command;


} //namespace cmd
} //namespace srv


#endif //COMMANDS_HPP
