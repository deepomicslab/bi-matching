//
// Created by caronkey on 27/11/2020.
//

#include "reads_overlap.h"
#include "../include/cxxopts.hpp"
// arg1 输入bam，arg2输出jaccard
double DEPTH = -1;
int CUTOFF = 300;
bool SELFLOOP = false;
int THRESHOLD = 0;
int main(int argc, char **argv) {

    //    parse options
    cxxopts::Options options("Reads overlap", "Extract reads overlap from bam");

    options.add_options()
            ("b,bam", "Bam file", cxxopts::value<std::string>())
            ("o,out","Out file", cxxopts::value<std::string>())
            ("c,cut_off", "Cute off size", cxxopts::value<int>()->default_value("300"))
            ("d,depth", "Bam depth", cxxopts::value<float>())
            ("t,threshold", "Threshold for graph weigh filter", cxxopts::value<int>()->default_value("0"))
            ("s,self_l", "If self_loop take into consideration", cxxopts::value<bool>()->default_value("false"))
            ("h,help", "Print usage");
    auto result = options.parse(argc,argv);
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    std::string bamF = result["bam"].as<std::string>();
    std::string resultF = result["out"].as<std::string>();

    if(result.count("cut_off")) {
        CUTOFF = result["cut_off"].as<int>();
    }
    if(result.count("depth")) {
        DEPTH = result["depth"].as<float>();
    }
    if(result.count("self_l")) {
        SELFLOOP = result["self_l"].as<bool>();
    }
    if(result.count("threshold")) {
        THRESHOLD = result["threshold"].as<int>();
    }



    htsFile *in;

    if ((in = hts_open(bamF.c_str(), "r")) == nullptr) {
        fprintf(stderr, "Error opening '%s'\n", bamF.c_str());
        return -1;
    }

    readBAM(in, resultF, 100);

}

void initIMap (sam_hdr_t *hdr,Interactions& iMap, std::map<std::string, int>& refCopys) {
    int nRef = sam_hdr_nref(hdr);
    std::string refName, revRef;
    for(int i = 0; i< nRef; i++) {
        refName = sam_hdr_tid2name(hdr, i);

        if (DEPTH != -1) {
            std::stringstream ssf(refName);
            std::string item;
            while (std::getline(ssf,item,'_'));
            int idx = item.find(':');
            double cov = 0;
            if (idx != -1) {
                cov = std::stod(item.substr(0,idx));
            } else {
                cov = std::stod(item);
            }
            int copy = int(cov/DEPTH) == 0 ? 1 : int(cov/DEPTH);
            refCopys.emplace(refName, copy);
        } else {
            refCopys.emplace(refName, 1);
        }

        revRef = refName+" -";
        std::unordered_map<std::string, int> tmp;
        std::unordered_map<std::string, int> rvtmp;
        iMap[refName] = tmp;
        iMap[revRef]= rvtmp;
    }
}

void readBAM(htsFile *in, std::string& out_file, int readsLen) {
    Interactions iMap;
    sam_hdr_t *hdr;
    bam1_t *b;
    int ret;
    std::map<std::string, int> refCopys;
    if ((hdr = sam_hdr_read(in)) == nullptr) {
        fprintf(stderr, "[E::%s] couldn't read file header \n", __func__);
        return;
    }
    if ((b = bam_init1()) == nullptr) {
        fprintf(stderr, "[E::%s] Out of memory allocating BAM struct.\n", __func__);
    }
    std::string readName, refName, mRefName;
    initIMap(hdr, iMap, refCopys);
    int pos, mpos, refLen, mRefLen;
    while ((ret = sam_read1(in, hdr, b)) >= 0) {
        if (ret < -1) {
            fprintf(stderr, "[E::%s] Error parsing input.\n", __func__);
            if (b) bam_destroy1(b);
            if (hdr) sam_hdr_destroy(hdr);
        }
        if (b->core.tid == -1 || b->core.mtid == -1)
            continue;
        refLen = sam_hdr_tid2len(hdr, b->core.tid);
        mRefLen = sam_hdr_tid2len(hdr, b->core.mtid);
        refName = std::string(sam_hdr_tid2name(hdr, b->core.tid));

        mRefName = std::string(sam_hdr_tid2name(hdr, b->core.mtid));
        pos = b->core.pos;
        mpos = b->core.mpos;
        auto flags = b->core.flag;
        if (flags & 0x80) continue;
        auto rev = flags & 0x10;
        auto mrev = flags & 0x20;
        if (pos < refLen - CUTOFF && pos > CUTOFF) continue;
        if (mpos < mRefLen - CUTOFF && pos > CUTOFF) continue;
//        refLen = sam_hdr_tid2len(hdr, b->core.tid);
//        mRefLen = sam_hdr_tid2len(hdr, b->core.mtid);
//        if (refName == mRefName) {
//            if (!SELFLOOP) continue;
//            else if(rev == mrev) continue;
//        }
//        if (refName == mRefName && rev == !mrev)continue;
        if (rev)
            refName = refName.append(" -");
        if (!mrev)
            mRefName = mRefName.append(" -");
//        if (refLen <= readsLen) {
//            if (pos == 0)
//                refName = refName.append(" -");
//        } else if (pos < refLen/2 - readsLen/2) {
//            refName = refName.append(" -");
//        }
//        if (mRefLen <= readsLen) {
//            if (pos != 0)
//                mRefName = mRefName.append(" -");
//        } else if (pos >= mRefLen/2 - readsLen/2) {
//            mRefName = mRefName.append(" -");
//        }
        auto& refMap = iMap[refName];
        if (refMap.find(mRefName) == refMap.end()) {
            refMap[mRefName] = 1;
        } else {
            refMap[mRefName]+=1;
        }
    }
    std::ofstream fout(out_file);
    std::string prev;
    std::string next;
    for (const auto& item : refCopys ){
        fout<<"SEG "<<item.first<<" "<<item.second<<"\n";
    }
    for (auto& it: iMap) {
        for(auto& it2 : it.second) {
            if (it2.second < THRESHOLD) continue;
            if (it.first.back() != '-') prev = it.first + " +";
            else prev = it.first;
            if (it2.first.back() != '-') next = it2.first + " +";
            else next = it2.first;
            fout<<"JUNC "<<prev<<" "<<next<<" "<<it2.second<<"\n";
        }
//        if (it.second > 50) fout<<it.first.first<<"\t"<<it.first.second<<"\t"<<it.second<<"\t"<<"\n";
    }
    fout.close();
}

