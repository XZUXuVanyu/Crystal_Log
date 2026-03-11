module;
#include <chrono>
#include <string>
#include <source_location>
export module Crystal_Log:Types;
export namespace Crystal
{
	export enum Log_Level
	{
		Debug = 0, Info, Warning, Error, Critical
	};
	export struct Log_Event
	{
		Log_Level level;
		std::string message;
		std::source_location location;
		std::chrono::system_clock::time_point timestamp;
	};
} 