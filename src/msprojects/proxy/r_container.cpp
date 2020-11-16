#include "stdafx.h"
#include "r_container.h"
#include <unordered_map>
#include "misc.h"

using namespace rd;
const char* container::class_name = "++container++";
SEXP container::get_children()
{
  std::unordered_map<std::string, std::vector<std::wstring>> map;
  {
    const auto ret = m_dataset->get_children();
    for (size_t i = 0, n = ret.first.size(); i < n; i++)
      map[ret.second[i]].push_back(ret.first[i]);
  }
  tools::listGeneric list(map.size());
  for (const auto &it : map)
    list.push_back(tools::newVal(it.second), it.first);
  return list.get();
}
