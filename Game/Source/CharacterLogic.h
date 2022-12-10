#ifndef __CHARACTERLOGIC_H__
#define __CHARACTERLOGIC_H__

#include "Defs.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <any>

class CharacterLogic;


template<class UnaryPred>
std::any Find2(std::string_view action, UnaryPred predicate)
{
	auto findVertex = [action](std::unique_ptr<Vertex> const &v)
	{
		return StrEquals(v->action, action);
	}
}

struct Vertex
{
public:
	explicit Vertex(std::string_view name) : action(name) {};

	uint AddEdge(std::string_view destination)
	{
		edges.push_back(1);
		return static_cast<uint>(edges.size()) - 1;
	}

	uint ActionToIndex;

	std::string action = "";
	std::vector<uint> edges;
	friend class AdjacencyList;
};

class AdjacencyList
{
public:
	using graph2d = std::vector<std::unique_ptr<Vertex>>;

	AdjacencyList() = default;
	~AdjacencyList() = default;

	int AddVertex(std::string_view action)
	{
		if(auto i = GetIndex(action); i >= 0)
			return i;

		graph.push_back(std::make_unique<Vertex>(action));
		return graph.size() - 1;
	}

	int AddEdge(std::string_view origin, std::string_view destination)
	{
		if(auto v = At(origin); v)
			return v->AddEdge(destination);
	}

	// Returns index if action is on the array, -1 if no match
	int GetIndex(std::string_view action) const
	{
		for(int i = 0; auto const &elem : graph)
		{
			if(StrEquals(elem.get()->action, action))
				return i;
		}
		return -1;
	}

	// Returns iterator to element, iterator == graph.last() if no match
	graph2d::iterator Find(std::string_view action)
	{
		for(auto it = graph.begin(); it != graph.end(); ++it)
		{
			if(StrEquals((*it)->action, action))
			{
				return it;
			}
		}
		return graph.end();
	}

	// Returns vertex of action, nullptr if no match
	Vertex *At(std::string_view action)
	{
		if(auto it = std::ranges::find_if(graph.begin(), graph.end(),
										  [action](std::unique_ptr<Vertex> const &v)
										  {
											  return StrEquals(v->action, action);
										  }
		); it != graph.end())
			return it->get();

		return nullptr;
	}

private:
	uint currentAction = 0;
	graph2d graph;
};
#endif	// __CHARACTERLOGIC_H__
