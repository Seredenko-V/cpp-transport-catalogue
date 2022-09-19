#include "request_handler.h"

using namespace std;
using namespace svg;
using namespace renderer;

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db, const MapRenderer& renderer)
	: db_(db)
	, renderer_(renderer) {
}

Document RequestHandler::RenderMap() const {
	return renderer_.GetImageSVG(db_.GetAllBusesSorted());
}