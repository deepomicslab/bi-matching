//
// Created by caronkey on 28/12/2021.
//
//

#ifndef SEQGRAPH_MATCHING_H
#define SEQGRAPH_MATCHING_H
#include "Graph.h"
#include "algorithm"
#include <deque>
#include "util.h"
extern int VERBOSE;
extern bool BREAK_C;
class matching {
private:
    seqGraph::Graph* graph;
//    float** currentMatrix;
//    float** originalMatrix;
    std::vector<seqGraph::Vertex*>* originalVertices;
//    std::vector<seqGraph::Junction* >* originalJunctions;
    seqGraph::Graph* originalGraph;

    int N;

    float* lx;
    float* ly;
    int* matched;

    std::vector<int> cyclePaths;
    void init_labels();
    void update_labels();
    void bfs(int i, float ex[], float ey[], bool visity[],int pre[], std::set<int>& skipped, float []);
public:
    explicit matching(seqGraph::Graph* graph1);
    ~matching();

    void printM(int i);
    inline int* getMatched() const {
        return matched;
    }
    static bool cmpVertex (int i,int j);
    inline int getN() const {
        return N;
    }
    inline bool isCycle(int i) {
        return std::find(cyclePaths.begin(), cyclePaths.end(), i) != cyclePaths.end();
    }
//    inline float** getMatrix() const {
//        return this->currentMatrix;
//    };
    inline seqGraph::SparseMatrix& getMatrix() const {
        return this->graph->getConjugateMatrix();
    }
    void hungarian();
    bool dfs(int u, bool visity[], std::vector<int>* pre);
    bool kmDfs(int u, bool visity[],bool visitx[], std::set<int>* pre, float ex[], float ey[], float slack[]);
    void main_steps();

    std::map<int, std::vector<int>*>* resolvePath(std::map<int, std::vector<int>*>* prevPaths);

    int checkConjugateMatch();
    void checkConjugateMatrix();
    inline float getIRowMax(int i) {
        return this->graph->getIRowMaxV(i);
    }

    float getIJ(int i,int j);

//
    std::string idx2StrDir(int idx, const std::string& token= std::string(""));
    std::string idx2Str(int idx);
//    idx to current graph vertex
    seqGraph::Vertex* idx2VertexInCurrentGraph(int idx);
    seqGraph::Vertex* idx2VertexInOriginalGraph(int idx);
    void writeMatchResult(std::ofstream& outS);
    void reconstructMatrix(std::map<int, std::vector<int>*>* paths, seqGraph::Graph* originGraph);
    void resetGraph(seqGraph::Graph* g);
    std::vector<int>* addPrevPath(std::map<int, std::vector<int>*>* prevPath, std::vector<int>* curPath);
    void breakResolvedPaths(std::vector<int>* cur, std::deque<int> & zereBK, std::map<int,std::vector<int>* >* result);
    void breakAndMergeCycle(std::map<int,std::vector<int>*> *result);
    std::vector<int>* breakCycle(std::vector<int> *);
    float* mergePath(std::vector<int>* p1, std::vector<int>* p2, seqGraph::SparseMatrix& matrix, float* result);
};

//seqGraph::Graph* reconstructMatrix(float** matrix, std::map<int, std::vector<int>*>*);
std::vector<int>* addPrevPath(std::map<int, std::vector<int>*>* prevPath, std::vector<int>* curPath);
#endif //SEQGRAPH_MATCHING_H
