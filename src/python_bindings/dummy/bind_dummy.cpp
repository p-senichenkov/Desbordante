#include <pybind11/pybind11.h>

#include "core/algorithms/dummy/dummy_algorithm/dummy_algorithm.h"
#include "python_bindings/py_util/bind_primitive.h"

namespace py = pybind11;

namespace python_bindings {
void BindDummy(py::module_& main_module) {
    using namespace algos;

    auto dummy_module = main_module.def_submodule("dummy");
    BindPrimitiveNoBase<algos::dummy::DummyAlgorithm>(dummy_module, "DummyAlgorithm");
}
}  // namespace python_bindings
