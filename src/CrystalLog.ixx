module;
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <string>
#include <optional>
#include <filesystem>
#include <source_location>
#include <iostream>
#include <format>
#include <queue>
#include <condition_variable>

export module Crystal_Log;
import Types;
namespace Crystal
{
	export class Logger
	{
	public:
		static void Init(std::optional<std::string> custom_path = std::nullopt)
		{
			auto& logger = Instance();
			if (custom_path.has_value()) {
				logger.formatter.Init(custom_path.value());
			}
			else {
				logger.formatter.Init(std::nullopt);
			}

			if (logger.formatter.settings.at("if_threaded") and !logger.is_active) {
				logger.is_active = true;
				logger.background_worker = std::thread(&Logger::Background_Output_Loop, &logger);

				logger.Log("Crystal Log initialized with .ini file.", Log_Level::Info);
			}
			else {
				logger.Log(".ini file not found or invalid. Initialized with default settings.", Log_Level::Info);
			}
		}
		void Shutdown() 
		{
			is_active = false;
			cv.notify_all();
			if (background_worker.joinable()) {
				background_worker.join();
			}
		}
		static Logger& Instance()
		{
			static Logger instance;
			return instance;
		}
		~Logger()
		{
			Shutdown();
		}
		Logger() {};
		Logger(Logger&& other) = delete;
		Logger(const Logger& other) = delete;
		Logger& operator=(const Logger& other) = delete;
		Logger& operator=(Logger&& other) = delete;

	public:
		void Log(const std::string_view message, Log_Level level,
			const std::source_location location = std::source_location::current())
		{
			Log_Event event{ .level = level, .message = message.data(), .location = location,
			.timestamp = std::chrono::system_clock::now() };
			if (formatter.settings.at("if_threaded"))
			{
				{
					std::lock_guard<std::mutex> lock(log_queue_lock);
					log_queue.push(event);
				}
				cv.notify_one();
			}
			else
			{
				if (formatter.settings.at("if_to_file"))
				{
					formatter.Full_Output_To_File(event);
				}
				formatter.Full_Output_To_Console(event);
			}		
		};
		void Background_Output_Loop()
		{
			while (is_active or !log_queue.empty())
			{
				std::unique_lock<std::mutex> lock(log_queue_lock);
				//Thread excuting this function should be wakend when:
				//queue not empty -> log the messages out
				//after Logger is terminated -> check if there are any remained messages, and notify user that Logger is terminated.
				cv.wait(lock, [this]() {return !log_queue.empty() or !is_active; });

				if (!log_queue.empty())
				{
					Log_Event e = std::move(log_queue.front());
					log_queue.pop();
					lock.unlock();
					formatter.Full_Output_To_Console(e);
					if (formatter.settings.at("if_to_file"))
					{
						formatter.Full_Output_To_File(e);
					}

				}
			}
			std::cout << "Crystal Logger terminated." << std::endl;
		}

	private:
		std::filesystem::path setting_file_path;
		Output_Formatter formatter;
		std::queue<Log_Event> log_queue;
		std::mutex log_queue_lock;
		std::condition_variable cv;
		std::thread background_worker;

		bool is_active = false;
	};
}