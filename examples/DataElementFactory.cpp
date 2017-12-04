#include <functional>
#include <memory>
#include <string>

#include <DataElement.hpp>
#include <DataElementFactory.hpp>
#include <IsoTypeExample.hpp>

using namespace IsoLib::Example;

namespace
{
  using DebPtr = std::unique_ptr<DataElementBase>;

  
  std::unique_ptr<DataElement<Alpha>> makeDe1()
  {
    std::unique_ptr<DataElement<Alpha>> ret;
    ret.reset(new DataElement<Alpha>{Alpha(20)});
    return ret;
  }

  std::unique_ptr<DataElement<Alpha>> makeDe2()
  {
    std::unique_ptr<DataElement<Alpha>> ret;
    ret.reset(new DataElement<Alpha>{Alpha(10)});
    return ret;
  }

  std::unique_ptr<DataElement<Numeric>> makeDe3()
  {
    std::unique_ptr<DataElement<Numeric>> ret;
    ret.reset(new DataElement<Numeric>{Numeric(10)});
    return ret;
  }
}

const CreationMap ExampleFactory::creationMap_ = CreationMap{
  {"DE1", makeDe1},
  {"DE2", makeDe2},
  {"DE3", makeDe3},
};

std::unique_ptr<DataElementBase> ExampleFactory::createDataElement(const std::string& id)
{
  const auto& it = creationMap_.find(id);

  if (it == std::end(creationMap_))
    throw std::runtime_error("There is no data element with Id " + id);
  else
    return it->second();
}
