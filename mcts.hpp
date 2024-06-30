#include <vector>
#include <memory>
#include <random>
#include <cassert>

namespace MCTS
{
template <typename State>
class Tree
{
public:
	Tree(State state) :
		m_children(), m_state(state), m_parent(),
		m_wins(0), m_simulations(0)
	{}

	Tree(State& state, std::weak_ptr<Tree> parent) :
		m_children(), m_state(state), m_parent(parent),
		m_wins(0), m_simulations(0)
	{}
	
	void addChild(std::shared_ptr<Tree> child)
	{ m_children.push_back(child); }
	
	std::vector<std::shared_ptr<Tree>>& children()
	{ return m_children; }

	bool isLeaf()
	{ return m_children.size() == 0; }

	State& state()
	{ return m_state; }

	int wins()
	{ return m_wins; }

	int simulations()
	{ return m_simulations; }

	void addWin()
	{ m_wins ++; }

	void addSimulation()
	{ m_simulations ++; }

	std::weak_ptr<Tree>& parent()
	{ return m_parent; }

private:
	std::vector<std::shared_ptr<Tree>> m_children;
	State m_state;
	std::weak_ptr<Tree> m_parent;
	unsigned int m_wins;
	unsigned int m_simulations;
};


class UnkownNextStateException: public std::exception
{
	virtual const char* what() const throw()
	{
		return "An unknown state has given as an argument";
	}
};

template <typename State, typename Information>
class TreeSearch
{
public:
	TreeSearch(State initial, Information info, std::random_device &rd) :
		m_rng_gen(rd()),
		m_info(info)
	{
		auto tree = Tree<State>(initial);
		m_tree = std::make_shared<Tree<State>>(tree);
	}

	std::shared_ptr<Tree<State>>& tree()
	{ return m_tree; }
	
	void explore()
	{
		std::uniform_int_distribution<int> distrib;
		std::weak_ptr<Tree<State>> tree_weak_ptr(m_tree);

		/* Selection */
		while(1) {
			if(auto tree_ptr = tree_weak_ptr.lock()) {
				Tree<State> *tree = tree_ptr.get();
				if(tree->isLeaf())
					break;

				/* TODO: Change that */
				distrib = std::uniform_int_distribution<int>(0, tree->children().size()-1);
				tree_weak_ptr = tree->children()[distrib(m_rng_gen)];
			}
		}

		/* Expansion */
		std::vector<std::shared_ptr<MCTS::Tree<State>>> outcomes;
		while(1) {
			auto tree_ptr = tree_weak_ptr.lock();
			assert(tree_ptr);

			outcomes = getChildren(tree_ptr);
			if(outcomes.size() == 0)
				break;
			
			Tree<State> *tree = tree_ptr.get();
			for(auto outcome : outcomes)
				tree->addChild(outcome);

			/* TODO: Change that */
			distrib = std::uniform_int_distribution<int>(0, outcomes.size()-1);
			tree_weak_ptr = tree->children()[distrib(m_rng_gen)];
		}

		/* Backpropagation */
		auto tree_ptr = tree_weak_ptr.lock();
		Tree<State> *tree = tree_ptr.get();
		bool winning = isWinning(tree->state());

		while(auto tree_ptr = tree_weak_ptr.lock()) {
			Tree<State> *tree = tree_ptr.get();
			if(winning)
				tree->addWin();
			tree->addSimulation();

			tree_weak_ptr = tree->parent();
		}
	}

	void playMove(State& move)
	{
		Tree<State> *tree = m_tree.get();

		for(auto child_ptr : tree->children()) {
			if(Tree<State>* child = child_ptr.get()) {
				if(child->state() != move)
					continue;

				m_tree = child_ptr;
				return;
			}
		}

		throw UnkownNextStateException();
	}

private:
	virtual std::vector<std::shared_ptr<Tree<State>>>
		getChildren(std::shared_ptr<Tree<State>>);
	virtual bool isWinning(State&);

	std::shared_ptr<Tree<State>> m_tree;
	std::mt19937 m_rng_gen;
	Information m_info;
};

};
