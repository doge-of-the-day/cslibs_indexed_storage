#pragma once

#include <cslibs_clustering/backend/simple/map_generic.hpp>
#include <unordered_map>

namespace cslibs_clustering
{
namespace backend
{
namespace simple
{

template<typename data_t_, typename index_wrapper_t_, typename... options_ts_>
class Map : public MapGeneric<
        std::unordered_map<typename index_wrapper_t_::type, data_t_>,
        data_t_, index_wrapper_t_, options_ts_...>
{
};

}
}}