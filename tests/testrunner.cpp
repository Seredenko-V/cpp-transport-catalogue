#include "testrunner.h"
#include "../transport-catalogue/transport_catalogue.h"
#include "../transport-catalogue/json_reader.h"
#include "../transport-catalogue/map_renderer.h"
#include "../transport-catalogue/transport_router.h"
#include "../transport-catalogue/serialization.h"
#include "../transport-catalogue/request_handler.h"

#include <fstream>
#include <cassert>
#include <iostream>

using namespace std;

namespace tests {
    namespace run_test {
        void MakeBase(string&& file_name) {
            cerr << "Making database...\n"s;
            ifstream fin(file_name);
            assert(fin.is_open());
            transport_catalogue::TransportCatalogue t_catalogue;
            read::JsonReader reader(fin, t_catalogue);
            renderer::MapRenderer map_renderer;
            transport_router::TransportRouter t_router(t_catalogue);
            Serializator serializator(reader.ExtractSerializatorSettings(), t_catalogue, map_renderer, t_router);
            reader.FillCatalogue();
            map_renderer.SetSettings(reader.ExtractRenderSettings());
            t_router.Initialization(reader.ExtractRoutingSettings());
            serializator.SerializeDataBase();
        }

        ostringstream ProcessRequest(string&& file_name_request) {
            cerr << "Request processing...\n"s;
            ifstream fin(file_name_request);
            assert(fin.is_open());
            transport_catalogue::TransportCatalogue t_catalogue;
            read::JsonReader reader(fin, t_catalogue);

            renderer::MapRenderer map_renderer;
            transport_router::TransportRouter t_router(t_catalogue);
            Serializator serializator(reader.ExtractSerializatorSettings(), t_catalogue, map_renderer, t_router);

            serializator.DeserializeDataBase();
            handler::RequestHandler request_handler(t_catalogue, map_renderer, t_router);

            ostringstream sout;
            json::Print(request_handler.FormResponsesToRequests(reader.ExtractRequests()), sout);
            return sout;
        }

        ostringstream ReadFromBenchmarkFile(string&& file_name_benchmark) {
            ifstream fin(file_name_benchmark);
            assert(fin.is_open());
            ostringstream str_stream;
            str_stream << fin.rdbuf();
            return str_stream;
        }

        bool RunTest(string&& file_name_make_base, string&& file_name_request, string&& file_name_benchmark) {
            MakeBase(std::move(file_name_make_base));
            ostringstream request_result = ProcessRequest(std::move(file_name_request));
            ostringstream benchmark = ReadFromBenchmarkFile(std::move(file_name_benchmark));
            return request_result.str() == benchmark.str();
        }
    } // namespace run_test

    void RunAllTests() {
        constexpr int kNumAllTest = 1;
        const string kPrefixName = "../tests/test_"s;
        const string kMakeBase = "_make_base"s;
        const string kProcessRequests = "_process_requests"s;
        const string kOut = "_out"s;
        const string kExtension = ".json"s;
        string make_base = kPrefixName + "1" + kMakeBase + kExtension;

        int count_passed_tests = 0;
        for (int i = 1; i <= kNumAllTest; ++i) {
            string str_i = to_string(i);
            string process_requests = kPrefixName + str_i + kProcessRequests + kExtension;
            string out = kPrefixName + str_i + kOut + kExtension;
            bool is_test_passed = run_test::RunTest(std::move(make_base), std::move(process_requests), std::move(out));
            if (is_test_passed) {
                cerr << ">>> Test "s << i << " passed <<<\n"s;
            } else {
                cerr << ">>> Test "s << i << " failed <<<\n"s;
            }
            count_passed_tests += is_test_passed;
        }
        cerr << "Passed " << count_passed_tests << '/' << kNumAllTest << " tests\n"s;
    }
} // namespace tests
