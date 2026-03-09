module;
#include <windows.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <regex>
#include <source_location>
#include <string>
#include <sstream>
#include <thread>
#include <unordered_map>
export module Types;
export namespace Crystal
{
	enum Log_Level
	{
		Critical = 0,
		Error,
		Warning,
		Info,
		Debug
	};
	struct Log_Event
	{
		Log_Level level;
		std::string message;
		std::source_location location;
		std::chrono::system_clock::time_point timestamp;
	};
	const std::unordered_map<std::string, bool> default_settings = {
		{"if_colored_output",	true},  //If enable ANSI Colored console output
		{"if_to_file",			true}, //If record log message to file 
		{"if_to_console",		true},  //If record log message to std::iostream (cout/clog)
		{"if_time_stamp",		true},  //If add time stamps at begin
		{"if_file_location",	true},  //If show source file location (filename and line)
		{"if_threaded",			true},  //If enable unsynchornized log system
	};
	class Output_Formatter
	{
	public:
		Output_Formatter() = default;
		Output_Formatter(std::optional<std::string> custom_path) 
		{
			Init(custom_path);
		}
		~Output_Formatter()
		{
			ofs.close();
		}
		void Init(std::optional<std::filesystem::path> custom_path)
		{
			//Get customized path for *.ini
			if (custom_path.has_value()) 
			{
				if (std::filesystem::exists(custom_path.value()) and \
					std::filesystem::is_regular_file(custom_path.value()) and \
					(std::filesystem::path(custom_path.value()).extension() == ".ini"))
				{
					setting_file_path = custom_path.value();
					Parse_Settings();
				}
				std::cerr << "[Crystal_Log System] ERROR: Invalid custom path." << std::endl;
				return;
			}

			//Get the default path for Log_Settings.ini
			wchar_t path[MAX_PATH];
			if (GetModuleFileNameW(NULL, path, MAX_PATH) == 0) {
				std::cerr << std::format("[Crystal_Log System] ERROR: Failed to parse module path via Win32 API.") \
					<< std::endl;
				return;
			}

			setting_file_path = std::filesystem::path(path).parent_path();
			setting_file_path /= "Log_Settings.ini";

			if (!std::filesystem::exists(setting_file_path)) {
				std::cout << std::format("[Crystal_Log System] WARNING: Default .ini setting file '{}' not found. Reverting to internal defaults.",
					setting_file_path.string()) << std::endl;
			}
			Parse_Settings();
		}
		void Parse_Settings()
		{
			settings = default_settings;
			if (!setting_file_path.empty() and std::filesystem::exists(setting_file_path)) 
			{ 
				std::ifstream setting_file(setting_file_path);
				if (!setting_file.is_open()) {
					std::cout << "Unable to open file \"" << setting_file_path << "\" for reading." << std::endl;
					return;
				}

				std::string line;
				std::regex pattern(R"(^\s*([a-zA-Z0-9_-]+)\s*=\s*([^#\r\n\t]+?)\s*$)");
				std::smatch matches;

				while (std::getline(setting_file, line))
				{
					if (std::regex_match(line, matches, pattern)) {
						std::string key = matches[1].str();
						std::string value = matches[2].str();

						if (key == "log_output_path")
						{
							log_file_path = value;
							if (!log_file_path.empty() && !std::filesystem::exists(log_file_path))
							{
								std::cout << std::format("[Crystal_Log System] INFO: Directory '{}' not found. Attempting to create...",
									log_file_path.string()) << std::endl;
								if (!std::filesystem::create_directories(log_file_path))
								{
									log_file_path = "./Crystal_Logs/";
									std::cerr << std::format("[Crystal_Log System] ERROR: Failed to create directory. Falling back to default: '{}'",
										log_file_path.string()) << std::endl;
									std::filesystem::create_directories(log_file_path);
								}
							}
						}
						else
						{
							if (!settings.contains(key))
							{
								std::cout << "Invalid setting name: \"" << key << "\"." << std::endl;
								continue;
							}

							std::string lower_val = value;
							std::transform(lower_val.begin(), lower_val.end(), lower_val.begin(),
								[](unsigned char c) { return std::tolower(c); });
							settings[key] = (lower_val == "true");
						}
					}
				}
			}

			if (settings.at("if_to_file"))
			{
				if (!std::filesystem::exists(log_file_path)) {
					
					std::cout << std::format("[Crystal_Log System] INFO: Directory '{}' doesn't exists. Creating now...",\
						log_file_path.string()) << std::endl;
					std::filesystem::create_directories(log_file_path);
					std::cout << std::format("[Crystal_Log System] INFO: Directory '{}' created. Logs will be saved here.", \
						log_file_path.string()) << std::endl;
				}

				std::chrono::zoned_time time{ std::chrono::current_zone(), \
					std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()) };
				std::filesystem::path log_file_name = log_file_path / std::format("{:%Y%m%d}.log", time);
				ofs.open(log_file_name, std::ios::app);
			}
		}
		void Full_Output_To_Console(Log_Event event) 
		{
			std::string time_part = Time_Part(event);
			std::string level_part = Level_Part(event);
			std::string loc_part = Location_Part(event);

			std::cout << std::format("{}{}{}: {}", time_part, level_part, loc_part, event.message) << std::endl;
		}
		void Full_Output_To_File(Log_Event event)
		{
			auto to_sec = std::chrono::time_point_cast<std::chrono::seconds>(event.timestamp);
			std::chrono::zoned_time time_stamp{ std::chrono::current_zone(), to_sec };
			std::string log_file_name = log_file_path.string() + std::format("{:%Y%m%d}.log", time_stamp);
			
			if (!ofs.is_open()) {
				static bool error_notified = false;
				if (!error_notified) 
				{
					std::cerr << std::format(\
						"[Crystal_Log System] ERROR: Persistent file handle is closed. File logging failed.")\
						<< std::endl;
					error_notified = true;
				}
				return;
			}

			std::string time_part = Time_Part(event);
			std::string level_part = Level_To_String(event);
			std::string loc_part = Location_Part(event);
			ofs << std::format("{}{}{}: {}", time_part, level_part, loc_part, event.message) << std::endl;
		}
	public:
		std::string Level_To_String(const Log_Event& e)
		{
			switch (e.level)
			{
			case Crystal::Critical:
				return "[CRITICAL]";
			case Crystal::Error:
				return "[Error]";
			case Crystal::Warning:
				return "[Warning]";
			case Crystal::Info:
				return "[Info]";
			case Crystal::Debug:
				return "[Debug]";
			default:
				return "[Unknown]";
			}
		}
		std::string Colored_Level_To_String(const Log_Event& e)
		{
			switch (e.level)
			{
			case Crystal::Critical:
				return "\033[1;31m[CRITICAL]\033[0m";
			case Crystal::Error:
				return "\033[31m[Error]\033[0m";
			case Crystal::Warning:
				return "\033[33m[Warning]\033[0m";
			case Crystal::Info:
				return "\033[32m[Info]\033[0m";
			case Crystal::Debug:
				return "\033[37m[Debug]\033[0m";
			default:
				return "[Unknown]";
			}
		}

		std::string Time_Part(const Log_Event& e)
		{
			if (!settings.at("if_time_stamp")) return "";

			auto to_sec = std::chrono::time_point_cast<std::chrono::seconds>(e.timestamp);
			std::chrono::zoned_time time_stamp{ std::chrono::current_zone(), to_sec };
			return std::format("[{:%H:%M:%S}] ", time_stamp);
		}
		std::string Level_Part(const Log_Event& e) 
		{
			bool use_color = settings.at("if_colored_output");
				return use_color ? Colored_Level_To_String(e) : Level_To_String(e);
		}
		std::string Location_Part(const Log_Event& e) 
		{
			if (!settings.at("if_file_location")) return "";

				std::filesystem::path p(e.location.file_name());
			return std::format("[{}:{}]", p.filename().string(), e.location.line());
		}
	public:
		std::filesystem::path log_file_path = "./Crystal_Logs/";
		std::filesystem::path setting_file_path;
		std::unordered_map<std::string, bool> settings;
		std::ofstream ofs;
	};
} 