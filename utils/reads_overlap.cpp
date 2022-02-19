//
// Created by caronkey on 27/11/2020.
//

#include "reads_overlap.h"
// arg1 输入bam，arg2输出jaccard
double DEPTH = -1;
int main(int argc, char **argv) {
    htsFile *in;

    if ((in = hts_open(argv[1], "r")) == nullptr) {
        fprintf(stderr, "Error opening '%s'\n", argv[1]);
        return -1;
    }
    bool selfLoop = false;
    if (argc == 4) {
        selfLoop = atoi(argv[3]) == 1;
    }
    int cutoff = 300;
    if (argc == 5) {
        cutoff = atoi(argv[4]);
    }
    readBAM(in, argv[2], 100, selfLoop, cutoff);
    if (argc == 6) {
        DEPTH = std::stod(argv[5]);
    }
}

void initIMap (sam_hdr_t *hdr,Interactions& iMap) {
    int nRef = sam_hdr_nref(hdr);
    std::string refName, revRef;
    for(int i = 0; i< nRef; i++) {
        refName = sam_hdr_tid2name(hdr, i);
        revRef = refName+" -";
        std::unordered_map<std::string, int> tmp;
        std::unordered_map<std::string, int> rvtmp;
        iMap[refName] = tmp;
        iMap[revRef]= rvtmp;
    }
}

void readBAM(htsFile *in, const char* out_file, int readsLen, bool selfLoop, int cutoff) {
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
    initIMap(hdr, iMap);
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
        if (DEPTH != -1) {
            std::stringstream ssf(refName);
            std::string item;
            while (std::getline(ssf,item,'_'));
            double cov = std::stod(item);
            int copy = int(cov/DEPTH) == 0 ? 1 : int(cov/DEPTH);
            refCopys.emplace(refName, copy);
        } else {
            refCopys.emplace(refName, 1);
        }
        mRefName = std::string(sam_hdr_tid2name(hdr, b->core.mtid));
        pos = b->core.pos;
        mpos = b->core.mpos;
        auto flags = b->core.flag;
        if (flags & 0x80) continue;
        auto rev = flags & 0x10;
        auto mrev = flags & 0x20;
        if (pos < refLen - cutoff && pos > cutoff) continue;
        if (mpos < mRefLen - cutoff && pos > cutoff) continue;
//        refLen = sam_hdr_tid2len(hdr, b->core.tid);
//        mRefLen = sam_hdr_tid2len(hdr, b->core.mtid);
        if (refName == mRefName) {
            if (!selfLoop) continue;
            else if(rev == !mrev) continue;
        }
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

