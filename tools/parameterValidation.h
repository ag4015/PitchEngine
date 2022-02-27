#pragma once
#include <unordered_map>
#include <string>
#include <map>
#include <set>
#include <variant>

#ifdef USE_DOUBLE
typedef double my_float;
#else
typedef float my_float;
#endif


using var_t = std::variant<int, my_float, std::string>;
using parameterInstanceMap_t  = std::map<std::string, var_t >;
using parameterCombinations_t = std::unordered_map<std::string, std::vector<var_t> >;
using dontCares_t = std::unordered_map<std::string, std::set<std::string> >;

struct ParameterInstanceSetCompare
{
	const dontCares_t& dontCares_;
	ParameterInstanceSetCompare(const dontCares_t& dontCares) : dontCares_(dontCares) {};
	bool operator()(const parameterInstanceMap_t& a, const parameterInstanceMap_t& b) const
	{
		for (auto& param : a)
		{
			if (dontCares_.at(std::get<std::string>(b.at("algo"))).count(param.first))
				continue;
			else
			{
				if (param.first == "algo" || param.first == "inputFile")
				{
					if (std::get<std::string>(param.second) == std::get<std::string>(b.at(param.first)))
						continue;
					else
						return std::get<std::string>(param.second) < std::get<std::string>(b.at(param.first));
				}
				else if (param.first == "magTol")
				{
					if (std::get<my_float>(param.second) == std::get<my_float>(b.at(param.first)))
						continue;
					else
						return std::get<my_float>(param.second) < std::get<my_float>(b.at(param.first));
				}
				else
				{
					if (std::get<int>(param.second) == std::get<int>(b.at(param.first)))
						continue;
					else
						return std::get<int>(param.second) < std::get<int>(b.at(param.first));
				}
			}
		}
		return false;
	}
};

using parameterInstanceSet_t = std::set<parameterInstanceMap_t, ParameterInstanceSetCompare>;

parameterInstanceSet_t generateParameterInstanceSet(parameterCombinations_t& paramCombs, dontCares_t& dontCares);
std::string constructVariationName(const parameterInstanceMap_t& paramInstance);

