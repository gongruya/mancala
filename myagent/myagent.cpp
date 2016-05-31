#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cassert>
#include <iomanip>
#include <numeric>
#include <cmath>
#include <unordered_map>

using namespace std;

template<typename T>
inline ostream &operator<<(ostream &out, const vector <T> &v) {     //Overloading vector <<
    out << "[";
    if (v.empty()) {
        return out << "]";
    }
    for (int i = 0; i < v.size() - 1; ++i) {
        out << v[i] << " ";
    }
    out << v.back();
    out << "]";
    return out;
}


inline void line_tokenization(const string s, vector <int> &arr) {
    stringstream ss(s);
    string t;
    while (getline(ss, t, ' ')) {
        arr.push_back(stoi(t));
    }
}

/*
 This is a immutable class!
 */
class board {
#define TASK_GREEDY       1
#define TASK_MINIMAX      2
#define TASK_ALPHA_BETA   3
#define TASK_COMPETITION  4
    
#define OUTPUT_WIDTH 5
#define MIN_INT -2147483648
#define MAX_INT 2147483647

#define WINNING 2147483646
#define LOSING -2147483646

private:
    enum player{player1 = 1, player2 = 2};
    enum node{MAX_NODE = 0, MIN_NODE = 1};
    player who;             //Who TO move!
    player whom;            //From whom!
    int man1, man2;         //Player's mancalas
    int n;                  //Board size
    int m;                  //Total size (n * 2)
    int task;               //The task to make the move
    int cutoff;             //Search cutoff depth
    int val;                //Minimax val
    int last_move;          //Last move  -1 means root
    bool is_root;           //Is this a root node?
    int alpha, beta;
    vector <int> history;   //How we get here? Only modified by MINIMAX
    vector <int> position;  //Position
    vector <int> candidate; //Candidate move
    /*
     Example of the position index:
     9 8 7 6 5
     0 1 2 3 4
     */
public:
    board() {
    }
    board(const int task1, const int cutoff1, const int who1, const vector <int> &p2, const vector <int> &p1, const int m2, const int m1) {
        
        assert(!p1.empty() && !p2.empty() && p1.size() == p1.size());
        
        
        alpha = MIN_INT;
        beta = MAX_INT;
        
        is_root = true;
        last_move = -1;
        task = task1;
        cutoff = cutoff1;
        n = (int) p1.size();
        m = n << 1;
        position = vector <int> (m);



        

        if (who1 == 1) {
            who = player1;
            whom = player2;
        } else {
            who = player2;
            whom = player1;
        }
        
        man1 = m1; man2 = m2;
        
        //Initializing the board
        for (int i = 0; i < n; ++i) {           //Player 1
            position[i] = p1[i];
        }
        for (int i = n - 1; i >= 0; --i) {
            position[m - 1 - i] = p2[i];
        }


        if (task == TASK_GREEDY) {
            cutoff = 1;
        } else if (task == TASK_COMPETITION) {
            cutoff = compute_cutoff(n, accumulate(position.begin(), position.end(), 0));
        }

        update_candidate();
        assert(!game_over());
    }
    
    int compute_cutoff(const int n, const int tot) const {      //size, total on board
        /*
            n: 3 ~ 10
            tot: 1 ~ 10000
        */

        return (int) round(12 - sqrt(n) - log(tot) / log(15));
    }


    friend inline ostream &operator<<(ostream &out, const board &my_board) {
        //print the position
        //return out << my_board.position;
        int n = my_board.n;
        int m = my_board.m;
        out << my_board.man2 << endl;
        for (int i = m - 1; i >= n + 1; --i) {
            out << setw(OUTPUT_WIDTH) << my_board.position[i];
        }
        out << setw(OUTPUT_WIDTH) << my_board.position[n] << endl;
        for (int i = 0; i < n - 1; ++i) {
            out << setw(OUTPUT_WIDTH) << my_board.position[i];
        }
        out << setw(OUTPUT_WIDTH) << my_board.position[n-1];
        out << endl << setw(OUTPUT_WIDTH * (n + 1)) << my_board.man1;
        
        
        out << endl << "Player " << my_board.who << " to move";
        out << endl << "Cutoff depth: " << my_board.cutoff;
        int current_eval = my_board.eval();
        out << endl << "Position analysis: " << ((current_eval >= 0)? "+": "") << current_eval;
        current_eval = my_board.eval(my_board.who);
        out << endl << "Candidate move: " << my_board.candidate << "; " << my_board.get_pit_name(my_board.candidate);
        out << endl << "----------------";
        return out;
    }
    inline string to_str() const {                //The output
        string ret = "";
        for (int i = m - 1; i >= n + 1; --i) {
            ret += to_string(position[i]) + " ";
        }
        ret += to_string(position[n]) + "\n";
        for (int i = 0; i < n - 1; ++i) {
            ret += to_string(position[i]) + " ";
        }
        ret += to_string(position[n-1]) + "\n" + to_string(man2) + "\n" + to_string(man1);
        return ret;
    }
    
    int size() const {
        return n;
    }
    int turn() const {
        return who;
    }
    const vector<int> &get_candidate() const {
        return candidate;
    }
    const vector<int> &get_position() const {
        return position;
    }

    board move(vector <string> &history_moves, vector <string> &traverse_log, const bool save_log = true) const { //NOT a mutator!
        //assert(has_legal_move());
        assert(!game_over());
        
        vector<string> the_move;
        board next;
        switch (task) {
            case TASK_GREEDY:
                next = move_minimax(history_moves, traverse_log, false, save_log);
                return next;
            case TASK_MINIMAX:
                next = move_minimax(history_moves, traverse_log, false, save_log);
                return next;
            case TASK_ALPHA_BETA:
                next = move_minimax(history_moves, traverse_log, true, save_log);
                return next;
            case TASK_COMPETITION:
                next = move_minimax(history_moves, traverse_log, true, save_log);
                return next;
            default:
                return *this;
        }
        
    }
    
    board move(const int k) const {                    //Choose the k th pit
        assert(is_own_pit(k) && position[k] > 0);
        board t = *this;
        t.whom = who;
        t.last_move = k;
        t.is_root = false;
        //Move
        int i = (k + 1) % m;              //next pos
        bool last = false;          //Fall into own mancala?
        int tot = t.position[k];
        t.position[k] = 0;
        while (tot--) {
            if (who == player1) {
                if (!last && i == n) {
                    last = true;
                    ++t.man1;
                } else {
                    ++t.position[i];
                    i = (i + 1) % m;
                    last = false;
                }
            } else {
                if (!last && i == 0) {
                    last = true;
                    ++t.man2;
                } else {
                    ++t.position[i];
                    i = (i + 1) % m;
                    last = false;
                }
            }
        }
        
        
        if (!last) {    //Not in own mancala
            
            //If end up in own zero pit, capture this (i) and the opposite (m-i-1)
            
            /*
             CAPTURE!!!!!
             */
            i = (i - 1 + m) % m;
            //cout << "Last: " << get_pit_name(i) << " " << i << endl;
            //cout << t.position << endl;
            if (is_own_pit(i) && t.position[i] == 1/* && t.position[m-i-1] > 0*/) {
                int tmp = t.position[i] + t.position[m-i-1];
                if (who == player1) {
                    t.man1 += tmp;
                } else {
                    t.man2 += tmp;
                }
                t.position[i] = t.position[m-i-1] = 0;
            }
            
            //If NOT end up in own mancala, FLIP
            t.flip_player();
        }
        t.end_game_collect();
        t.update_candidate();
        return t;                               //Change turn;
    }
    
    bool game_over() const {                                //Game over
        //return accumulate(position.begin(), position.end(), 0) == 0;
        int m1, m2;
        m1 = accumulate(position.begin(), position.begin() + n, 0);
        m2 = accumulate(position.begin() + n, position.end(), 0);
        return (m1 == 0) || (m2 == 0);
    }
    void end_game_collect() {
        if (game_over()) {
            man1 += accumulate(position.begin(), position.begin() + n, 0);
            man2 += accumulate(position.begin() + n, position.end(), 0);
            position.clear();
            position = vector<int>(m, 0);
        }
    }
    
private:                        //Private methods
    
    int eval(player p) const {                    //Evaluation from p's perspective
        if (p == player1) {
            return eval();
        } else {
            return -eval();
        }
    }
    int eval() const {                            //Who stands better?
        if (task != TASK_COMPETITION) {
            return man1 - man2;
        } else {
            return (int) round(eval_comp());
        }
    }
    double eval_comp() const {                       //From Player1's perspective
        double score = 0;
        int piece1 = accumulate(position.begin(), position.begin() + n, 0);
        int piece2 = accumulate(position.begin() + n, position.end(), 0);
        int t = piece1 + piece2;
        if (man1 - man2 > t) {                       //Winning position for 1
            return WINNING;
        }
        if (man2 - man1 > t) {                       //Winning position for 2
            return LOSING;
        }
        double man_diff = (man1 - man2) * 2.22;
        double piece_diff = -(piece1 - piece2) * 0.38;   //Kick as many pieces to the opposite

        vector<int> weight = {2, 1, 0, 0, -1, -2};

        double weight1;
        double weight2;
        /*
            Weight:
            2 1 0 0 -1 -2
            2 1 0 0 -1 -2
        */

        score = man_diff + piece_diff;
        return score;
    }

    void update_candidate() {
        candidate = vector<int>(0);
        if (who == player1) {
            for (int k = n - 1; k >= 0; --k) {
                if (position[k] > 0)
                    candidate.push_back(k);
            }
        } else {
            for (int k = m - 1; k >= n; --k) {
                if (position[k] > 0)
                    candidate.push_back(k);
            }
        }
    }
    board move_minimax(vector<string>& the_move, vector <string> &traverse_log, const bool prune, const bool save_log) const {              //prune==false -> no alpha beta
        board start = *this;
        board ultimate = max_node(start, traverse_log, 0, prune, MIN_NODE, save_log);//Find optimal
        cout << "After move: " << get_pit_name(ultimate.history) << ", expected analysis: " << ((ultimate.eval() >= 0)? "+": "") << ultimate.eval() << endl;


        board next = *this;    //Make the decision
        for (auto e: ultimate.history) {
            next = next.move(e);
            the_move.push_back(get_pit_name(e));
            if (who != next.turn()) {
                break;
            }
        }

        next.reset();                                   //Reset the new node as a root
        return next;
    }
    /*
     MINIMAX function
     */
    board max_node(board &current, vector <string> &traverse_log, const int depth, const bool prune, const node from, const bool save_log) const {
        assert(depth >= 0 && depth <= cutoff);
        //cout << get_node_name(current) << endl;
        
        int v = MIN_INT;
        
        /*if (depth == cutoff) {
            v = current.eval(who);
        }*/
        
        board best = current;
        
        if ((current.who != current.whom && depth == cutoff) || !current.has_legal_move() || (!current.is_root && abs(current.eval(who)) == WINNING)) {
            best.val = current.eval(who);
            if (save_log) {
                traverse_log.push_back(log_string(current, depth, best.val, prune));
            }
            return best;
        }
        if (save_log) {
            traverse_log.push_back(log_string(current, depth, v, prune));
        }
        
        for (auto e: current.candidate) {
            board next = current.move(e);
            next.history.push_back(e);
            
            int delta_depth = current.whom != current.who;
            
            if (depth == cutoff && delta_depth) {
                best.val = current.eval(who);
                return best;
            }
            
            if (next.whom == next.who) {            //Still player's turn
                next = max_node(next, traverse_log, depth + delta_depth, prune, MAX_NODE, save_log);
            } else {
                next = min_node(next, traverse_log, depth + delta_depth, prune, MAX_NODE, save_log);
            }
            if (next.val > v) {
                v = next.val;
                best = next;
            }
            if (prune) {
                if (best.val >= current.beta) {

                    if (save_log) {
                        traverse_log.push_back(log_string(current, depth, v, prune));
                    }
                    return best;
                }
                current.alpha = max(current.alpha, best.val);
            }
            //cout << get_node_name(current) << endl;

            if (save_log) {
                traverse_log.push_back(log_string(current, depth, v, prune));
            }
        }
        return best;
    }
    board min_node(board &current, vector <string> &traverse_log, const int depth, const bool prune, const node from, const bool save_log) const {
        assert(depth >= 0 && depth <= cutoff);
        //cout << get_node_name(current) << endl;
        int v = MAX_INT;
        
        /*if (depth == cutoff) {
            v = current.eval(who);
        }*/
        
        board best = current;
        
        if ((current.who != current.whom && depth == cutoff) || !current.has_legal_move() || (!current.is_root && abs(current.eval(who)) == WINNING)) {
            best.val = current.eval(who);
            if (save_log) {
                traverse_log.push_back(log_string(current, depth, best.val, prune));
            }
            return best;
        }
        if (save_log) {
            traverse_log.push_back(log_string(current, depth, v, prune));
        }

        for (auto e: current.candidate) {
            board next = current.move(e);
            next.history.push_back(e);
            
            int delta_depth = current.whom != current.who;
            
            if (depth == cutoff && delta_depth) {
                best.val = current.eval(who);
                return best;
            }
            
            if (next.whom == next.who) {          //Still player's turn
                next = min_node(next, traverse_log, depth + delta_depth, prune, MIN_NODE, save_log);
            } else {
                next = max_node(next, traverse_log, depth + delta_depth, prune, MIN_NODE, save_log);
            }
            
            if (next.val < v) {
                v = next.val;
                best = next;
            }
            if (prune) {
                if (best.val <= current.alpha) {
                    if (save_log) {
                        traverse_log.push_back(log_string(current, depth, v, prune));
                    }
                    return best;
                }
                current.beta = min(current.beta, best.val);
            }
            if (save_log) {
                traverse_log.push_back(log_string(current, depth, v, prune));
            }
            //cout << get_node_name(current) << endl;
        }
        return best;
    }

    void flip_player() {                //The only mutable variable
        who = (who == player1)? player2: player1;
    }
    bool has_legal_move() const {
        return !candidate.empty();
    }
    bool is_own_pit(const int k) const {
        if (who == player1) {
            return k < n;
        } else {
            return k >= n;
        }
    }
    string get_pit_name(const int k) const {
        if (k < n) {
            return "B" + to_string(k + 2);
        } else {
            return "A" + to_string(m - k + 1);
        }
    }
    vector <string> get_pit_name(const vector <int> &v) const {
        vector <string> ret;
        for (auto e: v) {
            ret.push_back(get_pit_name(e));
        }
        return ret;
    }
    /*
    string get_node_name(const int k) const {
        if (k < n) {
            return "B" + to_string(k + 2);
        } else {
            return "A" + to_string(m - k + 1);
        }
    }

    */
    string get_pit_name(const board &t) const {
        if (t.is_root) {
            return "root";
        }
        return get_pit_name(t.last_move);
    }

    string disp(const int x) const {
        if (x == MIN_INT) {
            return "-Infinity";
        }
        if (x == MAX_INT) {
            return "Infinity";
        }
        return to_string(x);
    }
    string log_string(const board &t, const int depth, const int val, const bool prune) const {
        string my_log = get_pit_name(t) + "," + disp(depth) + "," + disp(val);
        if (prune) {
            my_log += "," + disp(t.alpha) + "," + disp(t.beta);
        }
        return my_log;
    }
    void reset() {                  //Reset a node' alpha beta val...
        alpha = MIN_INT;
        beta = MAX_INT;
        is_root = true;
        history.clear();
        last_move = -1;
        whom = (who == player1)? player2: player1;
    }
};

int main(int argc, char *argv[]) {
    string inp_file = "input.txt";
    if (argc == 3) {
        inp_file = argv[2];
    }
    ifstream fin(inp_file);
    
    int task, who, cutoff, man1, man2;
    double time_remain;
    fin >> task >> who;



    if (task == 4) {
        //Competition
        fin >> time_remain;
        cutoff = 1;
    } else {
        fin >> cutoff;
    }




    string line;
    getline(fin, line);             //consuming the line
    
    vector <int> p1, p2;
    
    getline(fin, line);             //player2
    line_tokenization(line, p2);
    
    getline(fin, line);             //player1
    line_tokenization(line, p1);
    
    fin >> man2 >> man1;
    fin.close();


    board my_board(task, cutoff, who, p2, p1, man2, man1);
    vector <string> history_moves;
    vector <string> traverse_log;
    if (0) {
        /*
         Test mode
         */
         
        int step = 1;
        cout << my_board << endl;
        while (!my_board.game_over()) {
            cout << "#" << (++step >> 1) << ":" << endl;
            vector <string> traverse_log;
            my_board = my_board.move(history_moves, traverse_log, false);
            cout << my_board << endl;
            cout << "History moves: " << history_moves << endl << "---" << endl;
        }
        cout << "Game Over" << endl << "History moves: " << history_moves << endl;
        
        //cout << my_board.move(3) << endl;
    } else {
        if (task != 4) {
            /*
             Start of the homework
             */
            ofstream fnext("next_state.txt");
            ofstream flog("traverse_log.txt");
            if (task == 2) {
                flog << "Node,Depth,Value" << endl;
            } else if (task == 3) {
                flog << "Node,Depth,Value,Alpha,Beta" << endl;
            }

            cout << my_board << endl;

            my_board = my_board.move(history_moves, traverse_log);
            cout << my_board << endl;

            for (auto e: traverse_log) {
                flog << e << endl;
            }

            
            fnext << my_board.to_str() << endl;
            
            fnext.close();
            flog.close();
            cout << "History moves: " << history_moves << endl;
        } else {
            //Competition!
            cout << my_board << endl;
            my_board = my_board.move(history_moves, traverse_log, false);
            cout << traverse_log << endl;
            cout << my_board << endl;
            cout << "History moves: " << history_moves << endl;
            ofstream fmove("output.txt");
            for (auto e: history_moves) {
                fmove << e << endl;
            }
            fmove.close();
        }
    }
    return 0;
}