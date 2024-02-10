#include "pch.h"
#include "Animation.h"

namespace Toad
{
	json Animation::Serialize() const
	{
		json data;
		json frames_data;

		for (const AnimationFrame& frame : frames)
		{
			json frame_data;

			frame_data["key"] = frame.is_key;
			frame_data["px"] = frame.position.x;
			frame_data["py"] = frame.position.y;
			frame_data["r"] = frame.rotation;
			frame_data["sx"] = frame.scale.x;
			frame_data["sy"] = frame.scale.y;

			frames_data.emplace_back(frame_data);
		}

		data["name"] = name;
		data["frame_rate"] = frame_rate;
		data["frames"] = frames_data;
		data["looping"] = is_looping;
		return data;
	}
}
