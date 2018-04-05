#ifndef LIBFETCHCORE_SERVICE_PROTOCOL_HPP
#define LIBFETCHCORE_SERVICE_PROTOCOL_HPP
#include "service/protocol.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
namespace fetch
{
namespace service
{

void BuildProtocol(pybind11::module &module) {
  namespace py = pybind11;
  py::class_<Protocol>(module, "Protocol" )
    .def(py::init<>()) /* No constructors found */
    .def("Expose", &Protocol::Expose)
    .def("Subscribe", &Protocol::Subscribe)
    .def("Unsubscribe", &Protocol::Unsubscribe)
    .def("operator[]", &Protocol::operator[])
    .def("feeds", &Protocol::feeds)
    .def("RegisterFeed", &Protocol::RegisterFeed);

}
};
};

#endif