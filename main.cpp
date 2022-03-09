//
// Created by caronkey on 10/5/2021.
//
#include "include/Graph.h"
#include "include/matching.h"
#include <iostream>
#include <fstream>
#include <map>
#include <cstring>
#include <sstream>
#include "include/cxxopts.hpp"
int VERBOSE = 0;
const double ZERO = 0.00000001;
const double M_WEIGHT = 1000000;
bool BREAK_C = false;
int TYPE = 0;
bool SELF_L = false;
int MIN_L = -1;

void checkMatrixConjugate(double** matrix, int n) {
    for(int i = 1; i < n+1; i++) {
        std::cout<<i<<" ";
        for(int j = 1; j < n+1; j++) {
            auto t1 = matrix[i][j];
            auto t2 = matrix[conjugateIdx(j)][conjugateIdx(i)];
            if (std::abs(matrix[i][j] - matrix[conjugateIdx(j)][conjugateIdx(i)]) > ZERO) {
                std::cout<<"Error\n"<<i<<j<<std::endl;
                break;
            }
            std::cout<<matrix[i][j]<<" ";
        }
        std::cout<<"\n"<<std::endl;
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
//void print_help() {
//    std::cout<<"matching is a conjugated assembly algorithm that constructs a conjugate\n"
//               "map according to Barcode connection information between Contig,\n"
//               "and finds the most likely connection relationship between Contig\n";
//    std::cout<<"usage:\n\tmatching input.graph result.txt result.c.txt 0 1"<<std::endl;
//    std::cout<<"parameters:\n";
//    std::cout<<"\tinput.graph: graph generated by bDistance\n";
//    std::cout<<"\tresult.txt: linear output result\n";
//    std::cout<<"\tresult.c.txt: cycle output result\n";
//    std::cout<<"\t0: iteration matching times, 0 means only matching once, no more iteration\n";
//    std::cout<<"\t1: if output verbose information\n";
//}

void parse_tgs(std::string& f_name, seqGraph::Graph* g){
    std::ifstream tgs(f_name);
    std::string line;
    seqGraph::Vertex* vertex1 = nullptr;
    seqGraph::Vertex* vertex2 = nullptr;
    while( std::getline(tgs,line) )
    {
        std::stringstream ss(line);

        std::string v1, v2;
        char dir1, dir2;
        std::string course;
        int i = 1;
        while( std::getline(ss,course,' ') )
        {
            course.erase(std::remove(course.begin(), course.end(), '\r'), course.end());
            course.erase(std::remove(course.begin(), course.end(), '\n'), course.end());
            if (i == 1) {
                dir1 = course[course.size()-1];
                course.pop_back();
                v1 = course;
                vertex1 = g->addVertex(v1,"xx",1,2,1,1,2);
            } else {
                dir2 = course[course.size()-1];
                course.pop_back();
                v2 = course;
//                add to grap
                vertex2 = g->addVertex(v2,"xx",1,2,1,1,2);
                g->addJunction(vertex1, vertex2, dir1, dir2, M_WEIGHT, 1 , 1);
                vertex1 = vertex2;
                vertex2 = nullptr;
                dir1 = dir2;
//              make
            }
            i++;
        }
        std::cout<<"parse tgs down"<<"\n";
    }
    tgs.close();
}
//TODO add args support
// 1 graph 2 result 3 result.c 4 iteration 5 verbose 7 tgs
int main(int argc, char *argv[]) {

//    parse options
    cxxopts::Options options("matching", "Matching graph with conjugate bi-matching algorithm");

    options.add_options()
            ("g,graph", "Graph file", cxxopts::value<std::string>())
            ("r,result","Result", cxxopts::value<std::string>())
            ("c,result_c", "Cycle result", cxxopts::value<std::string>())
            ("i,iteration", "Iteration times", cxxopts::value<int>()->default_value("0"))
            ("v,verbose", "Verbose", cxxopts::value<int>()->default_value("0"))
            ("l,local_order", "Local order information", cxxopts::value<std::string>())
            ("m,result_m","Matching result", cxxopts::value<std::string>())
            ("b,break_c", "Whether break and merge cycle into line paths", cxxopts::value<bool>()->default_value("false"))
            ("s,self_l", "Cycle result", cxxopts::value<bool>()->default_value("true"))
            ("min_l", "Min length to print", cxxopts::value<int>()->default_value("-1"))
            ("h,help", "Print usage");
    auto result = options.parse(argc,argv);
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    std::string graphF = result["graph"].as<std::string>();
    std::string resultF = result["result"].as<std::string>();
    std::string resultCF = result["result_c"].as<std::string>();
    int iterRounds = result["iteration"].as<int>();
    VERBOSE = result["verbose"].as<int>();


    std::ifstream infile(graphF);
    std::ofstream resultFile(resultF);
    std::ofstream cyclePathsFile(resultCF);
    std::ofstream matchResultFile;
    int iterRoundsBK = iterRounds;
    if (result.count("result_m")) {
        std::string resultM = result["result_m"].as<std::string>();
        matchResultFile.open(resultM);
    }
    if (result.count("break_c")) {
        BREAK_C = true;
    }
    if (result.count("self_l")) {
        SELF_L = true;
    }

    std::string source, target, startTag, originalSource, originalTarget;
    char sDir, tDir;
    int copyNum;
    double coverage = 1;
    double weight;
    auto* g = new seqGraph::Graph;

//
    if (result.count("local_order")) {
        std::string localOrder = result["local_order"].as<std::string>();
        parse_tgs(localOrder, g);
    }

//    this used for path backtrack
    std::string line;
    while (getline(infile, line)) {
        std::istringstream iss(line);
        if (line.rfind("SEG", 0) == 0) {
//            print result match
            if (result.count("result_m")) {
                matchResultFile<<line<<std::endl;
            }
//            TODO, take coverage into consideration
            iss>>startTag>>originalSource>>coverage>>copyNum;
//            iss>>startTag>>originalSource>>copyNum;

//            TODO consider optimize the copied vertices.
            source = originalSource;
            source = source.append("_0");
//            if(source == "EDGE_1499493_length_56_cov_55.000000_0") {
//                int mm = 9;
//            }
            g->addVertex(source,"xx",1,2,coverage,1,copyNum);
            for (int i = 1; i < copyNum; i++) {
                source.pop_back();
                if (i>=11)
                    source.pop_back();
                if (i>=101)
                    source.pop_back();
                source.append(std::to_string(i));
                g->addVertex(source,"xx",1,2,coverage,1,copyNum);
//                source = originalSource;
            }
        } else {
            iss>>startTag>>originalSource>>sDir>>originalTarget>>tDir>>weight;
            if(SELF_L && originalSource == originalTarget) {
                cyclePathsFile<<originalSource<<"\n";
                continue;
            }
//            seqGraph::Vertex* v1;
//            seqGraph::Vertex* v2;
            source = originalSource;
            target = originalTarget;
            auto v1t = g->getVertexById(source+"_0");
            auto v2t = g->getVertexById(target+"_0");
//            v1 = v1t == nullptr ? g->addVertex(source,"xx",1,2,1,1,2) : v1t;
//            v2 = v2t == nullptr ? g->addVertex(target,"xx",1,2,1,1,2) : v2t;
            double c1 = v1t->getWeight()->getCopyNum();
            double c2 = v2t->getWeight()->getCopyNum();
            auto avg_weight = weight / (v1t->getWeight()->getCopyNum() * v2t->getWeight()->getCopyNum());
//            g->addJunction(v1t, v2t, sDir, tDir, avg_weight, 1 , 1);
            for (int i = 0; i <  c1; i++) {
                v1t = g->getVertexById(source.append("_").append(std::to_string(i)));
//                g->addJunction(v1t, v2t, sDir, tDir, avg_weight, 1 , 1);
                for (int j = 0; j <  c2; j++) {
                    v2t = g->getVertexById(target.append("_").append(std::to_string(j)));
                    g->addJunction(v1t, v2t, sDir, tDir, avg_weight, 1 , 1);
                    target = originalTarget;
                }
                source = originalSource;
            }
//            g->addJunction(v1t, v2t, sDir, tDir, weight, 1 , 1);
        }
    }
//    matching for each connected graph
    int n = 0;
    g->parseConnectedComponents();
    std::cout<<"total nodes"<<g->getVertices()->size()<<std::endl;
    int maxI = g->subGraphCount();
    while (n< maxI) {
//        if(n==6){
//            int m = 9;
//        }
        std::cout<<"process subgraph "<<n<<"\n";
        auto subGraph = g->getSubgraph(n);
        std::cout<<"sub graph nodes: "<<subGraph->getVertices()->size()<<std::endl;
        if(subGraph->getVertices()->size() == 1) {
            resultFile<<subGraph->getVertices()->front()->getOriginId()<<"+"<<"\n";
            n++;
            continue;
        }
        if (subGraph->getJuncSize() == 1) {
            auto sV = subGraph->getJunctions()->front()->getSource()->getOriginId();
            auto tV = subGraph->getJunctions()->front()->getTarget()->getOriginId();
            auto sD = subGraph->getJunctions()->front()->getSourceDir();
            auto tD = subGraph->getJunctions()->front()->getTargetDir();
            resultFile<<sV<<sD<<"\t"<<tV<<tD<<"\n";
            n++;
            continue;
        }
        auto* m = new matching(subGraph);
        if (VERBOSE >= 2)
            checkMatrixConjugate(m->getMatrix(), m->getN());
//    m->main_steps();
        m->hungarian();
        if (VERBOSE >= 2) {
            std::cout<<"final matched relation\n";
            for(int i = 0; i < m->getN() + 1; i++) {
                std::cout<<m->getMatched()[i]<<"\t";
            }
        }
        std::cout<<"\nresolve path"<<std::endl;
//    TODO , make the path and cycles info into mathicng class
        if (result.count("result_m")) {
            m->writeMatchResult(matchResultFile);
        }
        auto paths = m->resolvePath(nullptr);

//        cyclePathsFile<<"sub\n";
//        cyclePathsFile<<"iter 0\n";
        for (auto item: *paths) {
            if (m->isCycle(item.first)) {
                cyclePathsFile<<"iter "<<0<<",graph"<<n<<"\n";
                for(const auto& v: *item.second) {
                    cyclePathsFile<<m->idx2StrDir(v)<<"\t";
                }
                cyclePathsFile<<"\n";
            }
        }
//    recallPaths.push_back(paths);
        int iterN = 0;
        iterRounds = iterRoundsBK;
        int prevPathSize = paths->size();
        if (paths->size() != 1) {
            while (iterRounds !=0) {
                std::cout<<"Iteration "<<iterN<<",nodes count"<<paths->size()<<"...\n";
//                TODO if v==1 or juncs ==1 continue
                m->reconstructMatrix(paths, subGraph);
//                checkMatrixConjugate(m->getMatrix(), m->getN());
                m->hungarian();
                paths = m->resolvePath(paths);
                if (paths->size() == prevPathSize || paths->size() == 1) {std::cout<<paths->begin()->second->size()<<"break"<<std::endl; break;}
                for (auto item: *paths) {
                    if (m->isCycle(item.first)) {
                        cyclePathsFile<<"iter "<<iterN<<",graph"<<n<<"\n";
                        for(const auto& v: *item.second) {
                            cyclePathsFile << m->idx2StrDir(v) << "\t";
                        }
                        cyclePathsFile<<"\n";
                    }
                }
                prevPathSize = paths->size();
                iterRounds--;
                iterN++;
            }
        }
        for(auto item : *paths) {
            if (BREAK_C && m->isCycle(item.first)) continue;
            int total_length = 0;
            for(const auto& v: *item.second) {
                std::vector<std::string> tokens;
                tokenize(m->idx2StrDir(v).substr(0, m->idx2StrDir(v).length()-1), tokens, "_");
                int length = std::stoi(tokens[3]);
                total_length+=length;
                if (v == -1) {
                    resultFile<<'c';
                    continue;
                }
                if (MIN_L != -1 && total_length < MIN_L) continue;
                resultFile << m->idx2StrDir(v) << "\t";
            }
            resultFile<<"\n";
        }
        free(m);
        n++;
    }
    infile.close();
    resultFile.close();
    cyclePathsFile.close();
    matchResultFile.close();
}
