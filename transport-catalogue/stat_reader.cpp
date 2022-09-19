#include "stat_reader.h"
#include "input_reader.h"

using namespace std;

namespace statr {
	namespace get_inform {
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
		void PrintInformBus(const BusInfo& stat_bus, ostream& out) {
			if (stat_bus.is_empty) {
				out << "Bus "s << stat_bus.name << ": not found"s << endl;
			} else {
				out << "Bus "s << stat_bus.name << ": "s << stat_bus.number_stops << " stops on route, "s <<
					stat_bus.number_unique_stops << " unique stops, "s << setprecision(6) << stat_bus.road_distance
					<< " route length, "s << stat_bus.curvature << " curvature" << endl;
			}
		}

		void PrintBusesStop(const StopBuses& stop_buses, ostream& out) {
			using namespace transport_catalogue;
			vector <const Bus*> stop_buses_vect(stop_buses.buses_stop.size());
			copy(stop_buses.buses_stop.begin(), stop_buses.buses_stop.end(), stop_buses_vect.begin());
			sort(stop_buses_vect.begin(), stop_buses_vect.end(),
				[](const Bus*& lhs, const Bus*& rhs) {
					return lhs->name < rhs->name;
				});

			out << "Stop "sv << stop_buses.name << ": "sv;
			if (stop_buses.is_empty) {
				out << "not found"sv << endl;
				return;
			}
			if (stop_buses.buses_stop.empty()) {
				out << "no buses"sv << endl;
			} else {
				out << "buses "sv;
				for (const Bus* const& bus : stop_buses_vect) {
					out << bus->name << " "sv;
				}
				out << endl;
			}
		}
	}

	StatReader::StatReader(transport_catalogue::TransportCatalogue& transport_catalogue)
		: transport_catalogue_(transport_catalogue) {
	}

	void StatReader::GetStat(istream& input) {
		using namespace input::read;
		using namespace get_inform;
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
			case TypeQuery::STOP: {
				PrintBusesStop(transport_catalogue_.GetListBusesStop(GetNameStop(query)));
				break;
			}
			default:
				break;
			}
		}
	}

}