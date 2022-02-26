
#include "parameterValidation.h"
#include <sstream>

std::string constructVariationName(const parameterInstanceMap_t& paramInstance)
{
	std::string variationName;
	for (auto [paramName, paramValue] : paramInstance) {

		if (paramName == "inputFile")
		{
			variationName += std::get<std::string>(paramValue) + "_";
		}
		else if (paramName == "algo")
		{
			variationName += paramName + "_" + std::get<std::string>(paramValue) + "_";
		} 
		else if (paramName == "magTol")
		{
			std::stringstream ss;
			ss << std::get<my_float>(paramValue) << std::scientific;
			variationName += paramName + "_" + ss.str() + "_";
		}
		else
		{
			variationName += paramName + "_" + std::to_string(std::get<int>(paramValue)) + "_";
		}
	}

	// Remove trailing "_"
	size_t lastIndex = variationName.find_last_of("_");
	variationName = variationName.substr(0, lastIndex);

	return variationName;

}

void CartesianRecurse(std::vector<std::vector<int64_t>> &accum, std::vector<int64_t> stack,
	std::vector<std::vector<int64_t>> sequences,int64_t index)
{
	std::vector<int64_t> sequence = sequences[index];
	for (int64_t i : sequence)
	{
		stack.push_back(i);
		if (index == 0) {
			accum.push_back(stack);
		}
		else {
			CartesianRecurse(accum, stack, sequences, index - 1);
		}
		stack.pop_back();
	}
}

std::vector<std::vector<int64_t>> CartesianProduct(std::vector<std::vector<int64_t>>& sequences)
{
	std::vector<std::vector<int64_t>> accum;
	std::vector<int64_t> stack;
	if (sequences.size() > 0) {
		CartesianRecurse(accum, stack, sequences, sequences.size() - 1);
	}
	return accum;
}

parameterInstanceSet_t generateInstanceSet(parameterCombinations_t& paramCombs, dontCares_t& dontCares)
{
	// Convert parameterCombinations_t to a vector of vector of ints
	std::vector<std::vector<int64_t>> sequences;
	for (auto& param : paramCombs) {
		std::vector<int64_t> seq;
		if (param.first == "inputFile" || param.first == "algo")
		{
			for (auto& val : param.second) {
				std::string* strAddress = &std::get<std::string>(val);
				seq.push_back(reinterpret_cast<int64_t&>(strAddress));
			}
		}
		else
		{
			for (auto& val : param.second) {
				seq.push_back(reinterpret_cast<int64_t&>(val));
			}
		}
		sequences.push_back(seq);
	}

	std::vector<std::vector<int64_t>> res = CartesianProduct(sequences);

	// Eliminate duplicates
	std::set<std::vector<int64_t>> resSet;
	for (auto& v : res) {
		resSet.insert(v);
	}

	// Create parameter combinations with the combinatorial generated above
	parameterCombinations_t newParamCombs;
	for (auto& set : resSet) {
		int i = 0;
		for (auto& param : paramCombs)
		{
			// Reverse the set and insert
			if (param.first == "magTol")
			{
				int64_t a = static_cast<int64_t>(set[set.size() - i - 1]);
				my_float b = reinterpret_cast<my_float&>(a);
				newParamCombs[param.first].push_back(b);
			}
			else if (param.first == "inputFile" || param.first == "algo")
			{
				int64_t a = static_cast<int64_t>(set[set.size() - i - 1]);
				std::string* b = reinterpret_cast<std::string*>(a);
				newParamCombs[param.first].push_back(*b);
			}
			else
			{
				auto a = static_cast<int>(set[set.size() - i - 1]);
				newParamCombs[param.first].push_back(a);
			}
			i++;
		}
	}
	
	// Remove repeated combinations taking into account don't care parameters
	ParameterInstanceSetCompare cmp(dontCares);
	std::set<parameterInstanceMap_t, ParameterInstanceSetCompare> paramInstanceSet{cmp};

	for (size_t paramIdx = 0; paramIdx < newParamCombs["buflen"].size(); paramIdx++)
	{
		// Select the parameters to use for this iteration of the test
		parameterInstanceMap_t paramInstance;
		for (auto& [paramName, paramValues] : newParamCombs) {
			paramInstance[paramName] = paramValues[paramIdx];
		}
		paramInstanceSet.insert(paramInstance);
	}

	return paramInstanceSet;
}
