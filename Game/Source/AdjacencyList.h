#ifndef __ADJACENCYLIST_H__
#define __ADJACENCYLIST_H__

#include "Defs.h"

#include <vector>
#include <memory>
#include <algorithm>

template <typename T>
struct Vertex
{
public:
	explicit Vertex() = default;
	explicit Vertex(T name) : value(name) {};

	int AddEdge(int destination)
	{
		// Check if destination is already on the array
		for(auto i = 0; auto const &elem : edges)
		{
			if(elem == destination)
				return i;
			i++;
		}
		// If it's not, add it
		edges.push_back(destination);
		return static_cast<int>(edges.size()) - 1;
	}

	T value;
	std::vector<int> edges;
};

/* Directed Graph */
template <typename T>
class AdjacencyList
{
public:
	using graph2d = std::vector<Vertex<T>>;

	AdjacencyList() = default;
	~AdjacencyList() = default;

	int AddVertex(T value)
	{
		if(auto i = GetIndex(value); i >= 0)
			return i;

		graph.push_back(Vertex<T>(value));
		return static_cast<int>(graph.size()) - 1;
	}

	int AddEdge(T origin, T destination)
	{
		auto iEdge = GetIndex(destination);

		// If destination does not exist in graph
		if(iEdge == -1)
		{
			// Add the Vertex and update iEdge value with the new index
			iEdge = AddVertex(destination);
		}

		auto v = At(origin);
		// Add the edge to the Vertex array
		return v->AddEdge(iEdge);
	}

	// Returns index if action is on the array, -1 if no match
	int GetIndex(T value) const
	{
		for(int i = 0; auto const &elem : graph)
		{
			if(elem.value == value)
				return i;
			i++;
		}
		return -1;
	}

	// Returns iterator to element, iterator == graph.last() if no match
	graph2d::iterator Find(T value)
	{
		if(auto it = std::ranges::find_if(graph.begin(), graph.end(),
										  [value](Vertex<T> const &v)
										  {
											  return v.value == value;
										  }
		); it != graph.end())
			return it;

		return graph.end();
	}

	// Returns vertex of action, adds vertex if no match
	Vertex<T> *At(T value)
	{
		for(int i = 0; auto const &elem : graph)
		{
			if(elem.value == value)
				return &graph[i];
			i++;
		}
		return &graph[AddVertex(value)];
	}

private:
	std::vector<Vertex<T>> graph;
};
#endif	// __ADJACENCYLIST_H__
