module;
#include <chrono>
#include <string>
#include <source_location>
export module Crystal_Log:Types;
namespace Crystal
{
	export enum class Log_Level : int
	{
		Debug = 0, Info, Warning, Error, Critical
	};
	constexpr bool operator>(Log_Level lhs, Log_Level rhs)
	{
		return static_cast<int>(lhs) > static_cast<int>(rhs);
	}
	export struct Log_Event
	{
		Log_Level level;
		std::string message;
		std::source_location location;
		std::chrono::system_clock::time_point timestamp;
	};
} 