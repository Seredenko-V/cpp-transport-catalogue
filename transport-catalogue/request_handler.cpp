#include "request_handler.h"

using namespace std;

namespace detail {
	vector<cBusPtr> SortingBuses(const deque<Bus>& all_buses) {
		vector<cBusPtr> all_sorted_buses(all_buses.size());
		for (size_t i = 0; i < all_sorted_buses.size(); ++i) {
			all_sorted_buses[i] = &all_buses[i];
		}
		sort(all_sorted_buses.begin(), all_sorted_buses.end(),
			[](cBusPtr& lhs, cBusPtr& rhs) {
				return lhs->name < rhs->name;
			});
		return all_sorted_buses;
	}
}

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer)
	: db_(db)
	, renderer_(renderer) {
}

svg::Document RequestHandler::RenderMap() const {
	return renderer_.GetImageSVG(detail::SortingBuses(db_.GetAllBuses()));
}