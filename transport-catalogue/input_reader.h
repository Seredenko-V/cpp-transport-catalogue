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

		void ReadQuery(std::istream& input);
		void FillCatalogue(std::vector<std::string>&& queries);

	private:
		std::vector<std::string> SplitIntoInformBlocks(std::string&& text, const char symbol);
		std::pair<size_t, std::string> GetDistanceToStop(std::string&& text);

		// разделение названий остановок маршрута
		std::vector<std::string> SplitIntoStops(std::string&& text, const char separator_symbol);

		tc::TransportCatalogue& transport_catalogue_;
	};
}
