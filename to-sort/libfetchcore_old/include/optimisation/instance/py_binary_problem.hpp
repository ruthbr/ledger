#pragma once
#include"serializers/referenced_byte_array.hpp"
#include"service/service_client.hpp"

#include<memory>

namespace fetch
{
namespace optimisers
{

void BuildClient(pybind11::module &m)
{
  namespace py = pybind11;
  
  py::class_<BinaryProblem>(m, "BinaryProblem")
    .def(py::init<>())    
    .def("Resize", &BinaryProblem::Resize)
    .def("Insert", &BinaryProblem::Insert)
    .def("ProgramSpinGlassSolver", &BinaryProblem::ProgramSpinGlassSolver);


}
  
};
};

