#include <memory>
#include <random>
#include <iostream>

#include "mcts.hpp"

struct TicTacToeState
{
	enum BoardElement {
		EMPTY,
		X,
		O
	};
	BoardElement board[3][3];

	TicTacToeState()
	{
		for(int x = 0; x < 3; x ++)
			for(int y = 0; y < 3; y ++)
				board[x][y] = EMPTY;
	}

	TicTacToeState(TicTacToeState& b)
	{
		for(int x = 0; x < 3; x ++)
			for(int y = 0; y < 3; y ++)
				board[x][y] = b.board[x][y];
	}

	bool operator!=(TicTacToeState& rhs)
	{
		for(int x = 0; x < 3; x ++) {
			for(int y = 0; y < 3; y ++) {
				if(board[x][y] != rhs.board[x][y])
					return true;
			}
		}

		return false;
	}
};

typedef TicTacToeState::BoardElement TicTacToePlayer;

template<>
std::vector<std::shared_ptr<MCTS::Tree<TicTacToeState>>>
MCTS::TreeSearch<TicTacToeState, TicTacToePlayer>::getChildren(std::shared_ptr<MCTS::Tree<TicTacToeState>> tree_ptr)
{
	std::vector<std::shared_ptr<MCTS::Tree<TicTacToeState>>> output;
	auto tree = tree_ptr.get();

	// Retrieve the next player
	int x_count = 0;
	int o_count = 0;
	for(int x = 0; x < 3; x ++) {
		for(int y = 0; y < 3; y ++) {
			switch (tree->state().board[x][y]) {
			case TicTacToeState::X: x_count ++; break;
			case TicTacToeState::O: o_count ++; break;
			case TicTacToeState::EMPTY: break;
			}
		}
	}

	// Get the next player
	auto player =
		x_count <= o_count
			? TicTacToeState::X
			: TicTacToeState::O;
	
	for(int x = 0; x < 3; x ++) {
		for(int y = 0; y < 3; y ++) {
			if(tree->state().board[x][y] != TicTacToeState::EMPTY)
				continue;
			
			TicTacToeState new_state(tree->state());
			new_state.board[x][y] = player;
			MCTS::Tree<TicTacToeState> child(new_state, tree_ptr);
			output.push_back(std::make_shared<MCTS::Tree<TicTacToeState>>(child));
		}
	}

	return output;
}

template<>
bool MCTS::TreeSearch<TicTacToeState, TicTacToePlayer>::isWinning(TicTacToeState& state)
{
	TicTacToePlayer player = this->m_info;

	// Checking rows and collumns
	for(int i = 0; i < 3; i ++) {
		bool is_row_winning = true;
		bool is_col_winning = true;
		
		for(int j = 0; j < 3; j ++) {
			if(state.board[i][j] != player)
				is_col_winning = false;

			if(state.board[j][i] != player)
				is_row_winning = false;
		}

		if(is_col_winning || is_row_winning)
			return true;
	}

	// Checking diagonals
	bool winning = true;
	for(int i = 0; i < 3; i ++) {
		if(state.board[i][i] != player)
			winning = false;
	}
	if(winning)
		return true;

	winning = true;
	for(int i = 0; i < 3; i ++) {
		if(state.board[2-i][i] != player)
			winning = false;
	}
	if(winning)
		return true;

	return false;
}

void printTree(std::shared_ptr<MCTS::Tree<TicTacToeState>> tree_ptr, int ident = 0)
{
	MCTS::Tree<TicTacToeState> *tree = tree_ptr.get();

	if(tree->simulations() > 0) {
		for(int i = 0; i < ident; i ++)
			std::cout << ' ';
		std::cout << '(' << tree->winRate() << ')' << std::endl;
	}

	for(auto child : tree->children())
		printTree(child, ident+1);
}

int main(int argc, char *argv[])
{
	std::random_device rd;
	TicTacToeState board;
	MCTS::TreeSearch<TicTacToeState, TicTacToePlayer> ai(
		TicTacToeState(),
		TicTacToePlayer::O,
		rd
	);

	for(int i = 0; i < 100000; i ++)
		ai.explore();

	try {
		board.board[1][1] = TicTacToeState::X;
		ai.playMove(board);
		std::cout << "Made a move !\n";
		printTree(ai.tree());
	} catch (std::exception& e) {
		std::cout << e.what() << '\n';
	}

	return 0;
}