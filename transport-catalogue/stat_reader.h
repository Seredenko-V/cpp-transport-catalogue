#pragma once

#include "transport_catalogue.h"
#include "domain.h"

#include <iomanip>
#include <iostream>
#include <istream>
#include <ostream>
#include <string_view>
#include <vector>
#include <string>

using namespace std::literals;

namespace statr {
	enum TypeQuery {
		BUS,
		STOP
	};

	namespace get_inform {
		TypeQuery GetTypeQuery(std::string_view text_query);
		std::string_view GetNameBus(std::string_view text_query);
		std::string_view GetNameStop(std::string_view text_query);

	}

	namespace print {
		void PrintInformBus(const BusInfo& stat_bus, std::ostream& out = std::cout);
		void PrintBusesStop(const StopBuses& stop_buses, std::ostream& out = std::cout);
	}

	class StatReader {
	public:
		StatReader(transport_catalogue::TransportCatalogue& transport_catalogue);
		void GetStat(std::istream& input);

	private:
		transport_catalogue::TransportCatalogue& transport_catalogue_;
	};
}