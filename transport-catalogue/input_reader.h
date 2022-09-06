#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <istream>
#include <string>
#include <string_view>
#include <deque>
#include <unordered_map>

using namespace std::literals;

namespace input {
	class InputReader
	{
	public:
		InputReader(tc::TransportCatalogue& transport_catalogue);
		void FillCatalogue(std::vector<std::string>&& queries);

	private:
		void FillDistanceStops(std::unordered_map<std::string, std::vector<std::string>>&& inform_blocks_stops);
		void FillBuses(std::vector<std::string>&& text_buses);

		tc::TransportCatalogue& transport_catalogue_;
	};

	namespace read {
		std::string ReadLine(std::istream& input);
		int ReadLineWithNumber(std::istream& input);
		std::vector<std::string> ReadQuery(std::istream& input);
	}

	namespace parse {
		std::vector<std::string> SplitIntoInformBlocks(std::string&& text, const char symbol);
		std::pair<size_t, std::string> GetDistanceToStop(std::string&& text);

		// разделение названий остановок маршрута
		std::vector<std::string> SplitIntoStops(std::string&& text, const char separator_symbol);
	}
}
