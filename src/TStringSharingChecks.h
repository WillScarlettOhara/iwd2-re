// AutoUI-only replay of isolated original-binary string operations.
#include "TStringSharingGolden.h"

struct SharingStringAccess : TString {
    using TString::operator=;
    using TString::ConstructCopy;
    using TString::Detach;
    using TString::SetLength;
    using TString::MaxSize;
    using TString::Tidy;
};

void TStringSharingChecks(INT nStep)
{
    INT checks=0, index=-1, kind=-1; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) {
        ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"tstring-sharing-case\",\"row\":%d,\"kind\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,kind,name);
    };
    auto seed=[](SharingStringAccess& text,const SharingStorage& input,BYTE head)->char* {
        text.m_buf[0]=static_cast<char>(head);
        text.m_buf[1]=static_cast<char>(0xA1); text.m_buf[2]=static_cast<char>(0xB2); text.m_buf[3]=static_cast<char>(0xC3);
        if (!input.capacity) return NULL;
        char* block=new char[input.capacity+2]; block[0]=static_cast<char>(input.rc);
        memset(block+1,0xCC,input.capacity+1); memcpy(block+1,input.data,input.length); block[input.length+1]=0;
        text.m_pData=block+1; text.m_nLen=input.length; text.m_nRes=input.capacity; return block;
    };
    for (const auto& row : TSTRING_SHARING) {
        ++index; kind=row.kind;
        SharingStringAccess source,target;
        if (kind==7) { SharingStringAccess::ConstructCopy(NULL,NULL); check("null-construction",TRUE); continue; }
        if (kind==8) { check("native-maximum-size",source.MaxSize()==row.value); continue; }
        char* oldSource=seed(source,row.source,0x77); char* oldTarget=NULL;
        SharingStorage empty={}; seed(target,empty,0x99);
        SharingStringAccess* active=&source;
        if (kind==0) { SharingStringAccess::ConstructCopy(&target,&source); active=&target; }
        if (kind==1) { oldTarget=seed(target,row.target,0x99); active=&target; }
        if (kind==2) {
            target.m_pData=source.m_pData; target.m_nLen=source.m_nLen; target.m_nRes=source.m_nRes; active=&target;
        }
        if (kind==1 || kind==2 || kind==3) check("native-assignment-return",&active->AssignString(source,row.pos,row.count)==active && row.returned);
        if (kind==4) source.Detach();
        if (kind==5) source.SetLength(row.size);
        if (kind==6) {
            BYTE rhs[128]; memset(rhs,0xCC,sizeof(rhs)); memcpy(rhs,row.rhs,row.rhsBytes);
            check("native-unsigned-byte-comparison",source.Compare(INT(row.pos),INT(row.count),reinterpret_cast<const char*>(rhs),INT(row.rhsLength))==row.value);
        }
        auto state=[&](const SharingStringAccess& text,const SharingState& expected) {
            check("native-head-and-padding",BYTE(text.m_buf[0])==expected.head && BYTE(text.m_buf[1])==0xA1 && BYTE(text.m_buf[2])==0xB2 && BYTE(text.m_buf[3])==0xC3);
            BOOL dimensions=UINT(text.m_nLen)==expected.length && UINT(text.m_nRes)==expected.capacity;
            check("native-length-capacity",dimensions);
            check("native-bytes-and-terminator",(text.m_pData==NULL)==(expected.origin==0)
                && (!text.m_pData || (dimensions && memcmp(text.m_pData,expected.data,expected.length)==0 && (text.m_pData[expected.length]==0)==expected.terminated)));
            check("native-reference-count",(text.m_pData?INT(BYTE(text.m_pData[-1])):-1)==expected.rc);
            INT origin=!text.m_pData?0:oldSource && text.m_pData==oldSource+1?1:oldTarget && text.m_pData==oldTarget+1?2:3;
            // The CRT can reuse a released allocation at the same address.
            check("native-live-buffer-identity",origin==expected.origin || (expected.origin==3 &&
                ((origin==1 && row.oldSource[1]) || (origin==2 && row.oldTarget[1]))));
        };
        state(source,row.afterSource); state(*active,row.afterTarget);
        auto oldState=[&](char* block,const INT* expected) {
            check("native-retained-buffer-reference-count",!block || expected[1] || BYTE(block[0])==expected[0]);
        };
        oldState(oldSource,row.oldSource); oldState(oldTarget,row.oldTarget);
        if (kind==0) { target.Tidy(TRUE); oldState(oldSource,row.releasedSource); }
        BOOL sourceFreed=row.oldSource[1],targetFreed=row.oldTarget[1];
        auto release=[&](SharingStringAccess& text) {
            if (text.m_pData) {
                BYTE rc=BYTE(text.m_pData[-1]);
                if (rc==0 || rc==255) {
                    if (oldSource && text.m_pData==oldSource+1) sourceFreed=TRUE;
                    if (oldTarget && text.m_pData==oldTarget+1) targetFreed=TRUE;
                }
            }
            text.Tidy(TRUE);
        };
        release(target); release(source);
        // Seeded counts model absent external owners. Dispose their remaining
        // allocations after releasing both real objects, without stale reads.
        if (oldSource && !sourceFreed) delete[] oldSource;
        if (oldTarget && !targetFreed) delete[] oldTarget;
    }
    index=-1; kind=-1;
    {
        SharingStringAccess source,target; source="ICloudA"; target="FirebaR";
        char* originalSource=source.m_pData; char* originalTarget=target.m_pData;
        for (UINT pos : {8u,0x7FFFFFFFu,UINT_MAX}) {
            BOOL correct=FALSE;
            try { source.Compare(INT(pos),-1,"",0); }
            catch (const std::out_of_range& e) { correct=strcmp(e.what(),"invalid string position")==0; }
            catch (...) {}
            check("invalid-compare-position-throws-out-of-range",correct);
            correct=FALSE;
            try { target.AssignString(source,pos,UINT_MAX); }
            catch (const std::out_of_range& e) { correct=strcmp(e.what(),"invalid string position")==0; }
            catch (...) {}
            check("invalid-assignment-position-throws-out-of-range",correct);
            check("invalid-position-preserves-both-strings",source.m_pData==originalSource && target.m_pData==originalTarget
                && strcmp(source.Data(),"ICloudA")==0 && strcmp(target.Data(),"FirebaR")==0);
        }
    }
    {
        // Exercise the actual derived-stats immunity container and its comparator,
        // including allocator copy construction, duplicate insertion and release.
        TString key; key="SPWI304"; key.m_buf[0]=0x77;
        char* shared=key.m_pData; CSpellImmunitySet tree;
        tree.insert(key);
        check("immunity-set-shares-key",tree.size()==1 && tree.begin()->Data()==shared && BYTE(shared[-1])==1 && BYTE(tree.begin()->m_buf[0])==0x77);
        tree.insert(key); check("immunity-set-duplicate",tree.size()==1 && BYTE(shared[-1])==1);
        CSpellImmunitySet clone(tree);
        check("immunity-set-copy-shares-key",clone.size()==1 && clone.begin()->Data()==shared && BYTE(shared[-1])==2);
        key="SPWI305";
        check("immunity-set-key-mutation-detaches",key.Data()!=shared && BYTE(shared[-1])==1 && strcmp(tree.begin()->Data(),"SPWI304")==0
            && tree.find(key)==tree.end());
        TString lookup; lookup="SPWI304";
        check("immunity-set-original-membership",tree.find(lookup)!=tree.end() && clone.find(lookup)!=clone.end());
        clone.clear(); check("immunity-set-copy-release",BYTE(shared[-1])==0 && tree.find(lookup)!=tree.end());
        tree.clear(); check("immunity-set-clear",tree.empty());
    }
    Emit("{\"step\":%d,\"op\":\"tstringsharingchecks\",\"nativeCases\":%u,\"checks\":%d,\"valid\":%s}",nStep,UINT(sizeof(TSTRING_SHARING)/sizeof(TSTRING_SHARING[0])),checks,valid?"true":"false");
    if (!valid) Fail(nStep,"tstringsharingchecks","native string sharing/substring/compare replay failed");
}
