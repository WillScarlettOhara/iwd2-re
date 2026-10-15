// AutoUI-only typed replay; native offsets are confined to the original probe.
#include "EmberGeometryGolden.h"

struct EmberStringAccess : TString {
    using TString::PrepareAssign;
    using TString::Grow;
    using TString::Tidy;
};

void EmberGeometryChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* area=game->GetVisibleArea(); auto* objects=game->GetObjectArray();
    auto* handler=g_pBaldurChitin->GetMessageHandler();
    if (!area) { Fail(nStep,"embergeometrychecks","requires loaded area"); return; }
    INT checks=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"ember-geometry-case\",\"case\":\"%s\",\"passed\":false}",nStep,name); };
    ULONG clock=game->GetWorldTimer()->m_gameTime; auto stop=game->m_nTimeStop,caster=game->m_nTimeStopCaster;
    INT messages=handler->m_messageList.GetCount(),pending=area->m_lVertSortAdd.GetCount();
    EnterCriticalSection(&g_pChitin->m_critSectDimm); EnterCriticalSection(&g_pChitin->m_critSectService);
    struct KeyCount { CDimmKeyTableEntry* key; INT references; }; std::vector<KeyCount> keys;
    for (const char* name : {"GreaseA","FirebaR","FirebaA","ICloudA","ICloudB"}) for (USHORT type : {USHORT(1000),USHORT(1100)}) {
        auto* key=g_pChitin->cDimm.m_cKeyTable.FindKey(CResRef(name),type,FALSE); if (key) keys.push_back({key,key->field_10});
    }
    auto storage=[](TString& text,INT capacity,INT length,BYTE rc)->char* {
        text=""; text.m_buf[0]=0x77;
        if (!capacity) return NULL;
        char* block=new char[capacity+2]; block[0]=static_cast<char>(rc); memset(block+1,'O',length); block[length+1]=0;
        text.m_pData=block+1; text.m_nLen=length; text.m_nRes=capacity; return block;
    };
    auto retained=[](BYTE rc,BOOL freed,BOOL same) { return (rc!=0 && rc!=255) || (!freed && !same); };
    auto stringState=[&](const EmberStringGolden& row,const TString& text,char* old) {
        check("native-string-length-capacity-and-bytes",text.m_nLen==row.afterLength && text.m_nRes==row.afterCapacity
            && (text.m_pData==NULL)==row.null && (!text.m_pData || strcmp(text.m_pData,row.value)==0) && BYTE(text.m_buf[0])==0x77
            && (text.m_pData?INT(BYTE(text.m_pData[-1])):-1)==row.afterRc);
        // A released owned allocation may be reused at the same address. Only
        // compare identities/refcounts while the native old buffer stays alive.
        check("native-string-sharing-and-identity",!old || row.freed ||
            (BYTE(old[0])==row.oldRc && (text.m_pData==old+1)==row.same));
    };
    auto stringRows=[&](const auto& rows,INT operation) {
        for (const auto& row : rows) {
            EmberStringAccess text; char* old=storage(text,row.capacity,row.length,row.rc);
            if (operation==0) check("native-string-prepare-result",text.PrepareAssign(row.size,row.flag)==row.returned);
            else if (operation==1) text.Grow(row.size); else text.Tidy(row.flag);
            stringState(row,text,old); text.Tidy(TRUE);
            if (old && retained(row.rc,row.freed,row.same)) delete[] old;
        }
    };
    stringRows(EMBER_STRING_RESERVE,0); stringRows(EMBER_STRING_GROW,1); stringRows(EMBER_STRING_TIDY,2);
    {
        IcewindCSpellHitEmission emission; emission.m_resref0=const_cast<char*>("FirebaR"); emission.m_resref0Len=7;
        IcewindCGameAnimationTypeEffect animation(emission,0); CVidCell other(CResRef("FirebaA"),FALSE);
        check("ember-geometry-resources-available",animation.m_g1VidCell.pRes && other.pRes);
        if (animation.m_g1VidCell.pRes && other.pRes) {
            ProjectileRenderFrameAnchor mainAnchor(&animation.m_g1VidCell),otherAnchor(&other);
            animation.m_g1VidCell.m_nCurrentSequence=0; animation.m_g1VidCell.m_nCurrentFrame=0;
            other.m_nCurrentSequence=0; other.m_nCurrentFrame=0;
            auto setFrame=[](ProjectileRenderFrameAnchor& anchor,const LONG* input) {
                for (auto* f : {anchor.frame,anchor.headerFrame}) if (f) {
                    f->nCenterX=SHORT(input[0]); f->nCenterY=SHORT(input[1]); f->nWidth=WORD(input[2]); f->nHeight=WORD(input[3]);
                }
            };
            for (const auto& row : EMBER_GEOMETRY) {
                setFrame(mainAnchor,row.main); setFrame(otherAnchor,row.other);
                animation.m_currentVidCell=&animation.m_g1VidCell; animation.m_currentVidCellShadow=row.shadow?&other:NULL;
                CRect rect; CPoint anchor; animation.CalculateFxRect(rect,anchor,row.z);
                LONG state[]={rect.left,rect.top,rect.right,rect.bottom},point[]={anchor.x,anchor.y};
                check("native-ember-fx-shadow-and-height-wrap",memcmp(state,row.rect,sizeof(state))==0 && memcmp(point,row.anchor,sizeof(point))==0);
            }
            for (const auto& row : EMBER_BOUNDS) {
                animation.m_currentBamDirection=row.direction; animation.m_extendDirectionTest=row.threshold;
                CRect rect; animation.CalculateGCBoundsRect(rect,CPoint(row.pos[0],row.pos[1]),CPoint(row.anchor[0],row.anchor[1]),row.z,row.size[0],row.size[1]);
                LONG state[]={rect.left,rect.top,rect.right,rect.bottom}; check("native-ember-bounds-signed-facing-and-wrap",memcmp(state,row.rect,sizeof(state))==0);
            }
            struct Sequence { SEQUENCEENTRY* p; SEQUENCEENTRY saved; }; std::vector<Sequence> sequences;
            for (auto* p : {mainAnchor.resource->m_pSequences,mainAnchor.header?mainAnchor.header->m_pSequences:NULL,
                    otherAnchor.resource->m_pSequences,otherAnchor.header?otherAnchor.header->m_pSequences:NULL}) if (p) {
                BOOL found=FALSE; for (const auto& s : sequences) if (s.p==p) found=TRUE; if (!found) sequences.push_back({p,*p});
            }
            for (const auto& row : EMBER_QUERIES) {
                animation.m_currentVidCell=row.alternate?&other:&animation.m_g1VidCell; animation.m_currentBamSequence=7;
                animation.m_g1VidCell.m_nCurrentFrame=11; other.m_nCurrentFrame=13;
                for (const auto& s : sequences) {
                    BOOL isOther=s.p==otherAnchor.resource->m_pSequences || (otherAnchor.header && s.p==otherAnchor.header->m_pSequences);
                    s.p->nFrames=WORD((isOther?13:11)+(row.end?1:2));
                }
                animation.ChangeDirection(row.direction);
                LONG state[]={animation.m_currentBamDirection,animation.m_currentBamSequence,animation.m_g1VidCell.m_nCurrentFrame,other.m_nCurrentFrame,
                    animation.IsEndOfSequence(),animation.GetCurrentFrame()};
                check("native-ember-direction-end-and-frame-contract",memcmp(state,row.state,sizeof(state))==0);
                // 55D210 is folded across the base and ember vtables in the
                // original; Debug keeps separate source symbols for both bodies.
                check("native-folded-base-frame-contract",animation.CGameAnimationType::GetCurrentFrame()==row.state[5]);
            }
            for (const auto& s : sequences) *s.p=s.saved;
            animation.m_currentVidCell=&animation.m_g1VidCell; animation.m_currentVidCellShadow=NULL;
            check("ember-geometry-other-restored",otherAnchor.Restore()); check("ember-geometry-main-restored",mainAnchor.Restore());
        }
    }
    {
        CVidCell a(CResRef("ICloudA"),FALSE),b(CResRef("ICloudB"),FALSE),other(CResRef("FirebaA"),FALSE);
        check("ember-cloud-resources-available",a.pRes && b.pRes && other.pRes);
        if (a.pRes && b.pRes && other.pRes) {
            ProjectileRenderFrameAnchor anchorA(&a),anchorB(&b),otherAnchor(&other);
            struct Header { BAMHEADER* p; BYTE saved; }; std::vector<Header> headers;
            struct Sequence { SEQUENCEENTRY* p; SEQUENCEENTRY saved; }; std::vector<Sequence> sequences;
            for (auto* anchor : {&anchorA,&anchorB}) {
                for (auto* h : {anchor->resource->m_pBamHeader,anchor->resource->m_pBamHeaderCopy,
                        anchor->header?anchor->header->m_pBamHeader:NULL,anchor->header?anchor->header->m_pBamHeaderCopy:NULL}) if (h) {
                    BOOL found=FALSE; for (const auto& v : headers) if (v.p==h) found=TRUE; if (!found) headers.push_back({h,h->nSequences});
                }
                for (auto* s : {anchor->resource->m_pSequences,anchor->header?anchor->header->m_pSequences:NULL}) if (s) {
                    BOOL found=FALSE; for (const auto& v : sequences) if (v.p==s) found=TRUE; if (!found) sequences.push_back({s,*s});
                }
            }
            for (const auto& row : EMBER_REBIND) {
                for (const auto& h : headers) h.p->nSequences=BYTE(row.count);
                IcewindCSpellHitEmission empty; IcewindCGameAnimationTypeEffect animation(empty,0);
                char* old=storage(animation.m_animResName,row.capacity,row.length,row.rc);
                animation.m_currentVidCell=&other; animation.m_currentVidCellShadow=&other; animation.m_animMode=row.mode; animation.m_bRender=2;
                animation.m_currentBamDirection=-123; animation.m_g1VidCell.m_nCurrentFrame=11; other.m_nCurrentFrame=13;
                srand(1); animation.OverrideAnimation(row.name);
                LONG state[]={animation.m_currentBamSequence,animation.m_g1VidCell.m_nCurrentFrame,animation.m_g1VidCell.m_nCurrentSequence,other.m_nCurrentFrame,
                    animation.m_bRender,animation.m_currentBamDirection};
                check("native-cloud-rebind-main-not-current-cell",memcmp(state,row.state,sizeof(state))==0
                    && animation.m_currentVidCell==&other && animation.m_currentVidCellShadow==&other);
                BOOL present=strncmp(row.name,"ICloud",6)==0;
                check("native-cloud-rebind-name-and-resource",animation.m_animResName.Length()==INT(strlen(row.name))
                    && (animation.m_animResName.Length()==0 || strcmp(animation.m_animResName.Data(),row.name)==0)
                    && animation.m_animResName.m_nRes==row.afterCapacity && BYTE(animation.m_animResName.m_buf[0])==0x77
                    && (animation.m_g1VidCell.pRes!=NULL)==present
                    && animation.m_g1VidCell.GetResRef()==CResRef(present?row.name:"") && !animation.m_g1VidCell.m_bDoubleSize);
                check("native-cloud-rebind-rng",rand()==(row.randomCalls?18467:41));
                check("native-cloud-rebind-shared-storage",!old || row.freed || (BYTE(old[0])==row.oldRc && (animation.m_animResName.Data()==old+1)==row.same));
                animation.m_animResName="";
                if (old && retained(row.rc,row.freed,row.same)) delete[] old;
                animation.m_currentVidCell=&animation.m_g1VidCell; animation.m_currentVidCellShadow=NULL;
            }
            // Real particle AI now drives both cloud changes through actual BAMs.
            for (const auto& h : headers) h.p->nSequences=1;
            for (const auto& s : sequences) s.p->nFrames=1;
            IcewindCSpellHitEmission cloud; cloud.m_resref0=const_cast<char*>("ICloudA"); cloud.m_resref0Len=7;
            auto* p=new IcewindCSpellHitParticle(cloud,area,CPoint(160<<10,96<<10),0,CPoint(0,0),10,0,0);
            POSITION at=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(p->m_id)); if (at) area->m_lVertSortAdd.RemoveAt(at);
            auto* visual=static_cast<IcewindCGameAnimationTypeEffect*>(p->m_animation.m_animation); p->m_hasCloud=1;
            game->m_nTimeStop=0;
            for (INT i=0;i<4;++i) {
                p->m_animTick=99+100*i; p->AIUpdate();
                check("real-cloud-century-selects-B",visual->m_animResName.Compare(0,visual->m_animResName.Length(),"ICloudB",7)==0
                    && visual->m_g1VidCell.GetResRef()==CResRef("ICloudB"));
                p->AIUpdate(); check("real-cloud-end-restores-A",visual->m_animResName.Compare(0,visual->m_animResName.Length(),"ICloudA",7)==0
                    && visual->m_g1VidCell.GetResRef()==CResRef("ICloudA"));
            }
            p->CGameObject::RemoveFromArea(); BYTE deleted=objects->Delete(p->m_id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE);
            check("real-cloud-object-array-retired",deleted==CGameObjectArray::SUCCESS); if (deleted==CGameObjectArray::SUCCESS) delete p;
            for (const auto& h : headers) h.p->nSequences=h.saved;
            for (const auto& s : sequences) *s.p=s.saved;
            check("ember-cloud-other-restored",otherAnchor.Restore()); check("ember-cloud-B-restored",anchorB.Restore()); check("ember-cloud-A-restored",anchorA.Restore());
        }
    }
    // SetResRef's native inline path retains old KEY references; disposable
    // rebind fixtures restore those counters after every helper has been destroyed.
    for (const auto& v : keys) v.key->field_10=v.references;
    LeaveCriticalSection(&g_pChitin->m_critSectService); LeaveCriticalSection(&g_pChitin->m_critSectDimm);
    game->m_nTimeStop=stop; game->m_nTimeStopCaster=caster; srand(1);
    check("ember-fixtures-restored",clock==game->GetWorldTimer()->m_gameTime && handler->m_messageList.GetCount()==messages && area->m_lVertSortAdd.GetCount()==pending);
    Emit("{\"step\":%d,\"op\":\"embergeometrychecks\",\"checks\":%d,\"nativeCases\":1260,\"cloudChanges\":8,\"restored\":true,\"valid\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"embergeometrychecks","ember geometry, cloud rebind or string ownership differs from original capture");
}
