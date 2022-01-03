//
// Created by caronkey on 10/5/2021.
//
#include "include/Graph.h"
#include "include/matching.h"
#include <iostream>
#include <fstream>
#include <map>
int VERBOSE = 0;
void checkMatrixConjugate(double** matrix, int n) {
    for(int i = 1; i < n+1; i++) {
        for(int j = 1; j < n+1; j++) {
            auto t1 = matrix[i][j];
            auto t2 = matrix[conjugateIdx(j)][conjugateIdx(i)];
            if (matrix[i][j] != matrix[conjugateIdx(j)][conjugateIdx(i)]) {
                std::cout<<"Error\n"<<std::endl;
                break;
            }
        }
    }
}
void tokenize(const std::string &str, std::vector<std::string> &tokens, const std::string &delimiters)
{
    std::string::size_type pos, lastPos = 0, length = str.length();

    using value_type = typename std::vector<std::string>::value_type;
    using size_type  = typename std::vector<std::string>::size_type;

    while (lastPos < length + 1)
    {
        pos = str.find_first_of(delimiters, lastPos);
        if (pos == std::string::npos)
        {
            pos = length;
        }

        if (pos != lastPos)
            tokens.emplace_back(str.data() + lastPos, (size_type) pos - lastPos);

        lastPos = pos + 1;
    }
}
//recall paths from iteration result paths
//void pathsRecall(std::vector<std::map<int, std::vector<int>*>*> & recallPaths) {
//    for (auto iterPaths: *recallPaths.back()) {
//        for (auto item : *iterPaths.second) {
//
//        }
//    }
//}

int main(int argc, char *argv[]) {
    auto md = argv[1];
    std::ifstream infile(argv[1]);
    std::ofstream resultFile(argv[2]);
    int iterRounds = std::atoi(argv[3]);
    VERBOSE = std::atoi(argv[4]);

    std::string source, target;
    char sDir, tDir;
    double weight;
    auto* g = new seqGraph::Graph;

//    this used for path backtrack

    while (infile>>source>>sDir>>target>>tDir>>weight) {
        seqGraph::Vertex* v1;
        seqGraph::Vertex* v2;
        auto v1t = g->getVertexById(source);
        auto v2t = g->getVertexById(target);
        v1 = v1t == nullptr ? g->addVertex(source,"xx",1,2,1,1,2) : v1t;
        v2 = v2t == nullptr ? g->addVertex(target,"xx",1,2,1,1,2) : v2t;
        g->addJunction(v1, v2, sDir, tDir, weight, 1 , 1);
    }
    auto* m = new matching(g);
    checkMatrixConjugate(m->getMatrix(), m->getN());
//    m->main_steps();
    m->hungarian();
    std::cout<<"final matched relation\n";
    for(int i = 0; i < m->getN() + 1; i++) {
        std::cout<<m->getMatched()[i]<<"\t";
    }
    std::cout<<"\nresolve path";
    auto paths = m->resolvePath(nullptr);
//    recallPaths.push_back(paths);
    while (iterRounds !=0) {
        m->reconstructMatrix(paths);
        checkMatrixConjugate(m->getMatrix(), m->getN());
        m->hungarian();
        paths = m->resolvePath(paths);
        if (paths->size() == 1) break;
        iterRounds--;
    }
    for(auto item : *paths) {
        for(const auto& v: *item.second) {
            if (v == -1) {
                resultFile<<'c';
                continue;
            }
            resultFile<<m->idx2Str(v)<<"\t";
        }
        resultFile<<"\n";
    }
    infile.close();
    resultFile.close();
}