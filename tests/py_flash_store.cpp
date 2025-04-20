/*****************************************************************************
 * Copyright [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include <algorithm>
#include <ctime>
#include <thread>
#include <iostream>
#include <random>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include "yijinjing/journal/JournalReader.h"
#include "yijinjing/journal/JournalWriter.h"
#include "yijinjing/journal/Timer.h"

namespace py = pybind11;

struct TestData
{
    double last_price;
    double open_price;
    double highest_price;
    double lowest_price;
    int64_t local_timestamp_us;
};

struct TestD
{
    int last_price;
    int open_price;
};

yijinjing::JournalWriter create_writer(const string& dir, const string& jname,const string& writerName)
{
    yijinjing::JournalWriterPtr writer = yijinjing::JournalWriter::create(dir, jname, writerName);
    return *writer;
}

class MyClass {
public:
    template <typename T>
    T add(T a, T b) {
        return a + b;
    }
};

PYBIND11_MODULE(ShareMemory,m)
{
    m.doc() = "pybind11 example plugin";

    m.def("create_writer", &create_writer, "yjj Writer_create");
    
    py::class_<yijinjing::JournalWriter>(m, "JournalWriter")
        .def("init", &yijinjing::JournalWriter::init)
        .def("getPageNum", &yijinjing::JournalWriter::getPageNum)
        .def("seekEnd", &yijinjing::JournalWriter::seekEnd)
        .def("write_str", &yijinjing::JournalWriter::write_str)
        .def("write_frame", &yijinjing::JournalWriter::write_frame)
        .def("write_data_int", py::overload_cast<const int*, yijinjing::FH_TYPE_MSG_TP, yijinjing::FH_TYPE_LASTFG>(&yijinjing::JournalWriter::write_data<int>))
        .def("write_data_double", py::overload_cast<const double&, yijinjing::FH_TYPE_MSG_TP, yijinjing::FH_TYPE_LASTFG>(&yijinjing::JournalWriter::write_data<double>))
        .def("write_data_TestD", py::overload_cast<const TestD&, yijinjing::FH_TYPE_MSG_TP, yijinjing::FH_TYPE_LASTFG>(&yijinjing::JournalWriter::write_data<TestD>))
        .def("locateFrame", &yijinjing::JournalWriter::locateFrame)
        .def("passFrame", &yijinjing::JournalWriter::passFrame);

    py::class_<MyClass>(m, "MyClass")
        .def("add_int", &MyClass::add<int>, "Add two integers")
        .def("add_float", &MyClass::add<float>, "Add two floats");

    py::class_<TestD>(m, "TestD")
        .def(py::init<>())
        .def_readwrite("last_price", &TestD::last_price)
        .def_readwrite("open_price", &TestD::open_price);
}
