#include "stat_reader.h"
#include "input_reader.h"

using namespace std;

namespace statr {
	namespace getinf {
		string_view GetNameBus(string_view text_query) {
			return text_query.substr(text_query.find_first_of(' ') + 1);
		}

		TypeQuery GetTypeQuery(string_view text_query) {
			text_query.remove_suffix(text_query.size() - text_query.find_first_of(' '));
			return text_query == "Stop"sv ? TypeQuery::STOP : TypeQuery::BUS;
		}

		string_view GetNameStop(string_view text_query) {
			return text_query.substr(text_query.find_first_of(' ') + 1);
		}
	}

	namespace print {
		void PrintInformBus(const tc::BusInfo& stat_bus) {
			if (stat_bus.is_empty) {
				cout << "Bus "s << stat_bus.name << ": not found"s << endl;
			} else {
				cout << "Bus "s << stat_bus.name << ": "s << stat_bus.number_stops << " stops on route, "s <<
					stat_bus.number_unique_stops << " unique stops, "s << setprecision(6) << stat_bus.road_distance
					<< " route length, "s << stat_bus.tortuosity << " curvature" << endl;
			}
		}

		void PrintBusesStop(const tc::StopBuses& stop_buses) {
			cout << "Stop "sv << stop_buses.name << ": "sv;
			if (stop_buses.is_empty) {
				cout << "not found"sv << endl;
				return;
			}
			if (stop_buses.buses_stop.empty()) {
				cout << "no buses"sv << endl;
			} else {
				cout << "buses "sv;
				for (const tc::Bus* const& bus : stop_buses.buses_stop) {
					cout << bus->name << " "sv;
				}
				cout << endl;
			}
		}
	}

	StatReader::StatReader(tc::TransportCatalogue& transport_catalogue)
		: transport_catalogue_(transport_catalogue) {
	}

	void StatReader::GetStat(istream& input) {
		using namespace input::read;
		using namespace getinf;
		using namespace print;

		size_t num_string = ReadLineWithNumber(input);
		vector<string> output_queries(num_string);

		for (size_t i = 0; i < num_string; ++i) {
			output_queries[i] = move(ReadLine(input));
		}

		for (const string& query : output_queries) {
			switch (GetTypeQuery(query)) {
			case TypeQuery::BUS:
				PrintInformBus(transport_catalogue_.GetBusInfo(GetNameBus(query)));
				break;
			case TypeQuery::STOP:
				PrintBusesStop(transport_catalogue_.GetListBusesStop(GetNameStop(query)));
				break;
			default:
				break;
			}
		}
	}

}