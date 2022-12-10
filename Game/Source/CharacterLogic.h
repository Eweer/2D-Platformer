#ifndef __CHARACTERLOGIC_H__
#define __CHARACTERLOGIC_H__

#include "Defs.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <any>

/*
template<class UnaryPred>
std::any Find2(std::string_view action, UnaryPred predicate)
{
	auto findVertex = [action](std::unique_ptr<Vertex> const &v)
	{
		return StrEquals(v->action, action);
	}
}
*/

template <typename T>
struct Vertex
{
public:
	explicit Vertex(T name) : action(name) {};

	int AddEdge(int destination)
	{
		edges.push_back(destination);
		return static_cast<int>(edges.size()) - 1;
	}

	T action;
	std::vector<int> edges;
};

template <typename T>
struct graphvector : std::vector<std::unique_ptr<Vertex<T>>>
{
	std::vector<std::unique_ptr<Vertex<T>>> &operator[](std::size_t i)
	{

	}
};

template <typename T>
class AdjacencyList
{
public:
	using graph2d = std::vector<std::unique_ptr<Vertex<T>>>;

	AdjacencyList() = default;
	~AdjacencyList() = default;

	int AddVertex(T action)
	{
		if(auto i = GetIndex(action); i >= 0)
			return i;

		graph.push_back(std::make_unique<Vertex<T>>(action));
		return static_cast<int>(graph.size()) - 1;
	}

	int AddEdge(T origin, T destination)
	{
		Vertex<T> *v = At(origin);
		if(!v)
		{
			AddVertex(origin);
			v = At(origin);
		}

		// If destination exists
		if(auto i = GetIndex(destination); i != -1)
		{
			return v->AddEdge(i);
		}
		// If destination doesn't exist we add it.
		return v->AddEdge(AddVertex(destination));

	}

	int AddVertexAndEdge(T origin, T destination, int first, int second)
	{

	}

	// Returns index if action is on the array, -1 if no match
	int GetIndex(T action) const
	{
		for(int i = 0; auto const &elem : graph)
		{
			if(elem.get()->action == action)
				return i;
		}
		return -1;
	}

	// Returns iterator to element, iterator == graph.last() if no match
	graph2d::iterator Find(T action)
	{
		for(auto it = graph.begin(); it != graph.end(); ++it)
		{
			if((*it)->action == action)
			{
				return it;
			}
		}
		return graph.end();
	}

	// Returns vertex of action, nullptr if no match
	Vertex<T> *At(T action)
	{
		if(auto it = std::ranges::find_if(graph.begin(), graph.end(),
										  [action](std::unique_ptr<Vertex<T>> const &v)
										  {
											  return v->action == action;
										  }
		); it != graph.end())
			return it->get();

		return nullptr;
	}

private:
	int currentAction = 0;
	std::vector<std::unique_ptr<Vertex<T>>> graph;
};
#endif	// __CHARACTERLOGIC_H__
