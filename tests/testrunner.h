#pragma once

#include <string>
#include <ostream>

namespace tests {
    namespace run_test {
        /// Заполнение базы данных с последующей десериализацией
        void MakeBase(std::string&& file_name);

        /// Десериализация с последующей обработкой запроса
        std::ostringstream ProcessRequest(std::string&& file_name_request);

        /// Чтение из файла с последующим приведением к ostringstream
        std::ostringstream ReadFromBenchmarkFile(std::string&& file_name_benchmark);

        /// Запуск одного теста
        bool RunTest(std::string&& file_name_make_base, std::string&& file_name_request, std::string&& file_name_benchmark);
    } // namespace run_test

    void RunAllTests();
} // namespace tests

