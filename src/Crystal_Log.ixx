/**
 * @file Crystal_Log.ixx
 * @module Crystal_Log
 * @author XZUXuVanyu
 * @brief A lightweight, asynchronous, multi-threaded logging library built with C++20 Modules.
 * * ### Physical Architecture:
 * - **Decoupled Tiers**: Separates public API from internal logic using module partitions (:Types, :Internal).
 * - **Lock-Free Perception**: Main threads remain non-blocking by offloading I/O tasks to a background worker.
 * * ### Metadata:
 * - Project: Crystal Log
 * - Standards: C++20 or higher
 * - Dependencies: Win32 API (for path resolution), Standard Library.
 */
module;
#include <string>
#include <optional>
export module Crystal_Log;
export import :Types;
import :Internal;

export namespace Crystal
{
	void Crystal_Log_Init(std::optional<std::string> path = std::nullopt) {
		Logger::Instance().Init(path);
	}
	void Crystal_Log_Set_Level(Log_Level level) {
		Logger::Instance().Set_Log_Level(level);
	}
	void Crystal_Debug_Log(const std::string_view msg) {
		if (Logger::Instance().Get_Log_Level() > Log_Level::Debug) {
			return;
		}
		Logger::Instance().Log(msg, Log_Level::Debug);
	}
	void Crystal_Info_Log(const std::string_view msg) {
		if (Logger::Instance().Get_Log_Level() > Log_Level::Info) {
			return;
		}
		Logger::Instance().Log(msg, Log_Level::Info);
	}
	void Crystal_Warning_Log(const std::string_view msg) {
		if (Logger::Instance().Get_Log_Level() > Log_Level::Warning) {
			return;
		}
		Logger::Instance().Log(msg, Log_Level::Warning);
	}
	void Crystal_Error_Log(const std::string_view msg) {
		if (Logger::Instance().Get_Log_Level() > Log_Level::Error) {
			return;
		}
		Logger::Instance().Log(msg, Log_Level::Error);
	}
	void Crystal_Critical_Log(const std::string_view msg) {
		Logger::Instance().Log(msg, Log_Level::Critical);
	}
}