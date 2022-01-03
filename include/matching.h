//
// Created by caronkey on 28/12/2021.
//
//

#ifndef SEQGRAPH_MATCHING_H
#define SEQGRAPH_MATCHING_H
#include "Graph.h"
extern int VERBOSE;
class matching {
private:
    seqGraph::Graph* graph;
    double** currentMatrix;
    seqGraph::Graph* originalGraph;

    int N;

    double* lx;
    double* ly;
    int* matched;
    void init_labels();
    void update_labels();
    void bfs(int i, double ex[], double ey[], bool visity[],int pre[], double []);
public:
    explicit matching(seqGraph::Graph* graph1);
    void printM(int i);
    inline int* getMatched() const {
        return matched;
    }
    inline int getN() const {
        return N;
    }
    inline double** getMatrix() const {
        return this->currentMatrix;
    };
    ~matching() {
        free(this->graph);
    }
    void hungarian();
    bool dfs(int u, bool visity[], std::vector<int>* pre);
    bool kmDfs(int u, bool visity[],bool visitx[], std::vector<int>* pre, double ex[], double ey[], double slack[]);
    void main_steps();

    std::map<int, std::vector<int>*>* resolvePath(std::map<int, std::vector<int>*>* prevPaths);

    int checkConjugateMatch();

    std::string idx2Str(int idx);
    void reconstructMatrix(std::map<int, std::vector<int>*>* paths);
    void resetGraph(seqGraph::Graph* g);
    std::vector<int>* addPrevPath(std::map<int, std::vector<int>*>* prevPath, std::vector<int>* curPath);

};

int conjugateIdx(int idx);
//seqGraph::Graph* reconstructMatrix(double** matrix, std::map<int, std::vector<int>*>*);
double* mergePath(std::vector<int>* p1, std::vector<int>* p2, double** matrix);
std::vector<int>* addPrevPath(std::map<int, std::vector<int>*>* prevPath, std::vector<int>* curPath);
#endif //SEQGRAPH_MATCHING_H