// AutoUI test scaffolding, included inside its private namespace.
#include "SchoolHitGolden.h"
#include "SunrayGolden.h"

struct SunrayAccess : CProjectileSunray {
    using CProjectileSunray::m_vidCell;
    using CProjectileSunray::m_palette;
    using CProjectileSunray::m_phase;
    using CProjectileSunray::m_lifetime;
    using CProjectileSunray::m_renderFlag;
    using CProjectileSunray::m_animFlag;
    SunrayAccess(BYTE color=255,LONG flags=0,LONG animation=0,SHORT life=20)
        : CProjectileSunray(CResRef("SunrayX"),CResRef(""),color,flags,animation,life) {}
};

struct SchoolHitAccess : CProjectileSummonVFX {
    using CProjectileSummonVFX::m_pVidCell;
    using CProjectileSummonVFX::m_visualEffect;
    using CProjectileSummonVFX::m_offsetAboveTarget;
    using CProjectileSummonVFX::m_loopSequence;
    using CProjectileSummonVFX::m_loopSequenceIndex;
    using CProjectileSummonVFX::m_loopCount;
    using CProjectileSummonVFX::m_randomSequence;
    using CProjectileSummonVFX::m_singleSequence;
    using CProjectileSummonVFX::m_visible;
    using CProjectileSummonVFX::m_hasShadowCell;
    using CProjectileSummonVFX::m_dirCount;
    using CProjectileSummonVFX::m_useHeightOffset;
    SchoolHitAccess() : CProjectileSummonVFX(CResRef("InvocH"),IcewindCVisualEffect()) {}
};

struct SchoolSoundBuffer : IDirectSoundBuffer {
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID,void**) override { return E_NOINTERFACE; }
    ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }
    HRESULT STDMETHODCALLTYPE GetCaps(LPDSBCAPS) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetCurrentPosition(LPDWORD,LPDWORD) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetFormat(LPWAVEFORMATEX,DWORD,LPDWORD) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetVolume(LPLONG) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetPan(LPLONG) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetFrequency(LPDWORD) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetStatus(LPDWORD status) override { *status=DSBSTATUS_PLAYING; return S_OK; }
    HRESULT STDMETHODCALLTYPE Initialize(LPDIRECTSOUND,LPCDSBUFFERDESC) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE Lock(DWORD,DWORD,LPVOID*,LPDWORD,LPVOID*,LPDWORD,DWORD) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE Play(DWORD,DWORD,DWORD) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE SetCurrentPosition(DWORD) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE SetFormat(LPCWAVEFORMATEX) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE SetVolume(LONG) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE SetPan(LONG) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE SetFrequency(DWORD) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE Stop() override { return S_OK; }
    HRESULT STDMETHODCALLTYPE Unlock(LPVOID,DWORD,LPVOID,DWORD) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE Restore() override { return E_NOTIMPL; }
};

void SunrayChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray(); auto* area=game->GetVisibleArea();
    auto* handler=g_pBaldurChitin->GetMessageHandler(); auto* video=static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
    if (!area || !area->m_visibility.m_pMap || g_pChitin->cVideo.m_bIs3dAccelerated) { Fail(nStep,"sunraychecks","requires software area"); return; }
    INT checks=0,draws=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"sunray-case\",\"case\":\"%s\",\"passed\":false}",nStep,name); };
    ULONG clock=game->GetWorldTimer()->m_gameTime; auto stop=game->m_nTimeStop,caster=game->m_nTimeStopCaster;
    INT messages=handler->m_messageList.GetCount(),front=area->m_lVertSortAdd.GetCount();
    auto retire=[&](CGameObject* p) {
        POSITION pending=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(p->m_id));
        if (pending) area->m_lVertSortAdd.RemoveAt(pending);
        p->RemoveFromArea(); objects->Delete(p->m_id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE); delete p;
    };
    MasterPaletteFixture master(game->GetMasterBitmap()); check("sunray-master-palette-pinned",master.active);
    for (BYTE color : {BYTE(0),BYTE(1),BYTE(73),BYTE(127),BYTE(254),BYTE(255)}) for (LONG flags : {0L,1L,-1L}) for (LONG anim : {0L,-1L}) {
        SunrayAccess p(color,flags,anim,-32768);
        check("native-sunray-constructor",p.m_phase==0 && p.m_lifetime==-32768 && p.m_renderFlag==flags && p.m_animFlag==anim
            && p.m_vidCell.m_nCurrentFrame==0 && !p.m_bSparkleTrail);
        check("native-sunray-palette",color==255?p.m_vidCell.m_cPalette.m_nType==CVidPalette::TYPE_RESOURCE:
            (p.m_palette.m_rangeColors[0]==color && p.m_vidCell.m_cPalette.m_rangeColors[0]==color
                && p.m_palette.m_pPalette==p.m_vidCell.m_cPalette.m_pPalette && p.m_palette.m_pPalette[4].rgbGreen==color));
    }
    SunrayAccess fixture; check("sunray-bam-available",fixture.m_vidCell.pRes!=NULL);
    if (fixture.m_vidCell.pRes) {
        ProjectileRenderFrameAnchor anchor(&fixture.m_vidCell); auto* resource=fixture.m_vidCell.pRes; auto* header=fixture.m_vidCell.m_header.pRes;
        auto* original=resource->m_pSequences; auto* originalHeader=header?header->m_pSequences:NULL;
        SEQUENCEENTRY sequence=original[0]; sequence.nFrames=4; std::vector<SEQUENCEENTRY> sequences(12,sequence);
        struct Count { BAMHEADER* p; BYTE count; }; std::vector<Count> counts;
        for (auto* h : {resource->m_pBamHeader,resource->m_pBamHeaderCopy,header?header->m_pBamHeader:NULL,header?header->m_pBamHeaderCopy:NULL}) if (h) {
            BOOL seen=FALSE; for (const auto& saved : counts) if (saved.p==h) seen=TRUE;
            if (!seen) { counts.push_back({h,h->nSequences}); h->nSequences=12; }
        }
        resource->m_pSequences=sequences.data(); if (header) header->m_pSequences=sequences.data();
        for (const auto& row : SUNRAY_AI) {
            BOOL destroyed=FALSE;
            struct Sample : SunrayAccess { BOOL* destroyed=NULL; ~Sample() override { if (destroyed) *destroyed=TRUE; } };
            auto* p=new Sample; p->destroyed=&destroyed; p->m_lifetime=row.lifetime; p->m_animFlag=row.random; p->m_phase=row.phase;
            p->m_targetId=-1; p->m_vidCell.m_nCurrentFrame=row.frame; objects->Add(&p->m_id,p,INFINITE);
            game->m_nTimeStop=row.stop; game->m_nTimeStopCaster=row.stop==1?p->m_id+1:p->m_id;
            srand(1); p->AIUpdate(); check("native-sunray-rng",rand()==(row.stop!=1 && row.random?18467:41));
            check("native-sunray-retirement",destroyed==row.retired);
            if (!destroyed) { check("native-sunray-life-phase-frame",p->m_lifetime==row.afterLife && p->m_phase==row.afterPhase
                && p->m_vidCell.m_nCurrentFrame==row.afterFrame && p->m_vidCell.m_nCurrentSequence==row.afterSequence); retire(p); }
        }
        game->m_nTimeStop=0;
        struct Origin : CGameObject { BYTE GetObjectType() override { return TYPE_NONE; } };
        auto* target=new Origin; target->m_pArea=area; target->m_pos=CPoint(160,96); objects->Add(&target->m_id,target,INFINITE);
        struct Parent : CProjectile { INT callbacks=0; void CallBack() override { ++callbacks; } };
        auto* parent=new Parent; objects->Add(&parent->m_id,parent,INFINITE);
        for (BOOL point : {FALSE,TRUE}) for (BOOL missing : {FALSE,TRUE}) for (BOOL callback : {FALSE,TRUE}) {
            struct Sample : SunrayAccess { INT deliveries=0,removed=0; void DeliverEffects() override { ++deliveries; }
                void RemoveSelf() override { ++removed; m_targetId=-777; } };
            auto* p=new Sample; LONG id=point?-1:missing?-123:target->m_id;
            p->m_callBackProjectile=callback?parent->m_id:-1; INT calls=parent->callbacks;
            p->Fire(area,11,id,CPoint(256,400),31,13);
            check("native-sunray-fire-payload-and-ids",p->deliveries==1 && p->m_sourceId==11 && p->m_targetId==id);
            BOOL placed=point || !missing;
            check("native-sunray-one-target-share",p->CGameObject::m_pArea==(placed?area:NULL)
                && parent->callbacks==calls+(placed && callback));
            if (placed) check("native-sunray-y-plus-thirteen",p->m_pos==CPoint(point?256:160,point?413:109) && p->m_posZ==0);
            if (!point) { target->m_pArea=NULL; p->AIUpdate(); check("native-sunray-stale-target-removal",p->removed==1 && p->m_targetId==-777); target->m_pArea=area; }
            retire(p);
        }
        target->m_pArea=NULL; retire(target); retire(parent);
        auto& infinity=area->m_cInfinity; CRect view=infinity.rViewPort; INT viewX=infinity.nCurrentX,viewY=infinity.nCurrentY;
        auto* wed=infinity.pResWED; auto areaY=infinity.nAreaY;
        std::vector<BYTE> visibility(area->m_visibility.m_pMap,area->m_visibility.m_pMap+area->m_visibility.m_nMapSize);
        CRect patch(64,64,416,336); std::vector<BYTE> savedPixels;
        auto pixels=[&](std::vector<BYTE>* read,const std::vector<BYTE>* write,BOOL clear)->BOOL {
            if (!video->BKLock(patch)) return FALSE; DWORD bytes=video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount/8;
            INT row=patch.Width()*bytes; BOOL ok=bytes>=2 && bytes<=4 && video->m_SurfaceDesc.lpSurface!=NULL;
            if (ok && read) read->resize(row*patch.Height()); if (ok && write) ok=write->size()==static_cast<size_t>(row*patch.Height());
            if (ok) for (INT y=0;y<patch.Height();++y) { BYTE* p=static_cast<BYTE*>(video->m_SurfaceDesc.lpSurface)+y*video->m_SurfaceDesc.lPitch;
                if (read) memcpy(read->data()+y*row,p,row); if (write) memcpy(p,write->data()+y*row,row); if (clear) memset(p,0,row); }
            video->BKUnlock(); return ok;
        };
        BOOL backed=pixels(&savedPixels,NULL,FALSE); check("sunray-pixels-backed-up",backed);
        infinity.pResWED=NULL; infinity.rViewPort=CRect(80,80,400,320); infinity.nCurrentX=72; infinity.nCurrentY=882; infinity.nAreaY=10000;
        memset(area->m_visibility.m_pMap,CVisibilityMap::VISIBILITY_MASK,visibility.size());
        for (auto& s : sequences) s.nFrames=1;
        if (anchor.frame) anchor.frame->nCenterY=0; if (anchor.headerFrame) anchor.headerFrame->nCenterY=0;
        for (BYTE color : {BYTE(73),BYTE(255)}) for (LONG phase : {0L,1L,2L,0x4000L,0x10001L}) for (LONG flags : {0L,1L,-1L}) for (BOOL hidden : {FALSE,TRUE}) {
            SunrayAccess p(color,flags),reference(color,flags); p.m_phase=phase;
            p.m_pArea=area; p.CGameObject::m_pArea=area; p.m_pos=CPoint(232,1002); p.m_posZ=17; p.m_listType=CGameObject::LIST_FLIGHT;
            SHORT tile=area->m_visibility.PointToTile(p.m_pos); if (hidden) area->m_visibility.m_pMap[tile]=0;
            std::vector<BYTE> expected,actual; BOOL ok=backed && pixels(NULL,NULL,TRUE);
            if (!hidden) for (INT layer=0;layer<4 && ok;++layer) {
                reference.m_vidCell.SequenceSet(static_cast<SHORT>(static_cast<USHORT>(static_cast<USHORT>(phase)<<2)+layer));
                CPoint center; CSize size; reference.m_vidCell.GetCurrentCenterPoint(center,FALSE); reference.m_vidCell.GetCurrentFrameSize(size,FALSE);
                CRect fx(0,0,size.cx,size.cy); CPoint point(232,1002); DWORD f=flags?0x20280:0x20101;
                infinity.FXPrep(fx,f,CVIDINF_SURFACE_BACK,point,center); ok=infinity.FXLock(fx,f);
                if (ok) { infinity.FXRender(&reference.m_vidCell,center.x,center.y,f,0); infinity.FXUnlock(f,NULL,CPoint(0,0));
                    ok=infinity.FXBltFrom(CVIDINF_SURFACE_BACK,fx,point.x,point.y,center.x,center.y,f|1); }
            }
            ok=pixels(&expected,NULL,TRUE) && ok; p.Render(NULL,video,CVIDINF_SURFACE_BACK); ++draws; ok=pixels(&actual,NULL,FALSE) && ok;
            check("native-sunray-four-slice-full-pixels",ok && actual==expected);
            check("native-sunray-phase-word-shift",p.m_vidCell.m_nCurrentSequence==(hidden?0:static_cast<SHORT>(static_cast<USHORT>(static_cast<USHORT>(phase)<<2)+3))
                && p.m_vidCell.m_nCurrentFrame==0 && p.m_phase==phase);
            if (!hidden && phase==0 && flags==0 && color==255) check("sunray-really-renders",std::any_of(actual.begin(),actual.end(),[](BYTE value){return value!=0;}));
            area->m_visibility.m_pMap[tile]=CVisibilityMap::VISIBILITY_MASK; p.CGameObject::m_pArea=NULL;
        }
        resource->m_pSequences=original; if (header) header->m_pSequences=originalHeader;
        for (const auto& saved : counts) saved.p->nSequences=saved.count;
        memcpy(area->m_visibility.m_pMap,visibility.data(),visibility.size()); infinity.pResWED=wed; infinity.rViewPort=view;
        infinity.nCurrentX=viewX; infinity.nCurrentY=viewY; infinity.nAreaY=areaY;
        check("sunray-pixels-restored",backed && pixels(NULL,&savedPixels,FALSE)); check("sunray-resource-restored",anchor.Restore());
    }
    game->m_nTimeStop=stop; game->m_nTimeStopCaster=caster; srand(1); check("sunray-master-restored",master.Restore());
    check("sunray-fixtures-restored",clock==game->GetWorldTimer()->m_gameTime && handler->m_messageList.GetCount()==messages && area->m_lVertSortAdd.GetCount()==front);
    Emit("{\"step\":%d,\"op\":\"sunraychecks\",\"checks\":%d,\"renderCalls\":%d,\"nativeCases\":658,\"restored\":true,\"valid\":%s}",nStep,checks,draws,valid?"true":"false");
    if (!valid) Fail(nStep,"sunraychecks","SunrayX native state/render mismatch");
}

void SchoolHitChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray(); auto* area=game->GetVisibleArea();
    auto* handler=g_pBaldurChitin->GetMessageHandler();
    if (!area) { Fail(nStep,"schoolhitchecks","requires loaded area"); return; }
    INT checks=0,replays=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"school-hit-case\",\"case\":\"%s\",\"passed\":false}",nStep,name); };
    ULONG clock=game->GetWorldTimer()->m_gameTime; auto stop=game->m_nTimeStop,caster=game->m_nTimeStopCaster;
    INT messages=handler->m_messageList.GetCount(),front=area->m_lVertSortAdd.GetCount();
    auto retire=[&](CGameObject* p) {
        POSITION pending=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(p->m_id));
        if (pending) area->m_lVertSortAdd.RemoveAt(pending);
        p->RemoveFromArea(); objects->Delete(p->m_id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE); delete p;
    };
    struct Recipient : CGameAIBase {
        INT calls=0; DWORD color=0,flags=0; BYTE list=0; BOOL save=FALSE,immediate=FALSE;
        void AddEffect(CGameEffect* e,BYTE l,BOOL s,BOOL i) override {
            ++calls; color=e->m_effectAmount; flags=e->m_dwFlags; list=l; save=s; immediate=i; delete e;
        }
    } recipient;
    objects->Add(&recipient.m_id,&recipient,INFINITE);
    auto requested=[&](const char* name)->CResRef { CRes* r=g_pChitin->cDimm.GetResObject(CResRef(name),1000,FALSE);
        BOOL available=r!=NULL; if (r) g_pChitin->cDimm.ReleaseResObject(r); return CResRef(available?name:""); };
    for (const auto& row : SCHOOL_HIT_FACTORY) {
        recipient.calls=0;
        CProjectile* p=CProjectileSummonVFX::DecodeSpellHitProjectile(row.type,row.caster?&recipient:NULL,row.positive);
        check("native-factory-nonnull-and-sound",p && p->m_fireSoundRef==CResRef(row.fire));
        if (auto* overlay=dynamic_cast<CProjectileSummonVFX*>(p)) {
            check("native-overlay-resource",(overlay->*(&SchoolHitAccess::m_pVidCell))->GetResRef()==requested(row.visual));
            const auto& v=overlay->*(&SchoolHitAccess::m_visualEffect);
            check("native-overlay-render-config",v.m_bTintEnabled==row.tint && v.m_dwFlags==row.flags);
            check("native-overlay-sequence-config",overlay->*(&SchoolHitAccess::m_offsetAboveTarget)==row.offset
                && overlay->*(&SchoolHitAccess::m_loopSequence)==row.loop && overlay->*(&SchoolHitAccess::m_loopSequenceIndex)==row.loopIndex
                && overlay->*(&SchoolHitAccess::m_loopCount)==row.loopCount && overlay->*(&SchoolHitAccess::m_randomSequence)==row.random
                && overlay->*(&SchoolHitAccess::m_singleSequence)==row.single);
            check("native-inherited-layout-defaults",!(overlay->*(&SchoolHitAccess::m_hasShadowCell))
                && overlay->*(&SchoolHitAccess::m_dirCount)==1 && !p->m_bHasHeight && !(overlay->*(&SchoolHitAccess::m_useHeightOffset)));
        } else check("native-exotic-carrier-kind",row.type==0 || (row.type==57?dynamic_cast<CProjectileSunray*>(p)!=NULL:dynamic_cast<CProjectileCallLightning*>(p)!=NULL));
        INT queued=0;
        while (handler->m_messageList.GetCount()>messages) {
            CMessage* m=handler->m_messageList.RemoveTail(); auto* glow=dynamic_cast<CMessageAddEffect*>(m);
            check("native-fire-glow-message-ownership",glow && row.queued && m->m_sourceId==recipient.m_id && m->m_targetId==recipient.m_id);
            if (glow) { ++queued; glow->Run(); } delete m;
        }
        check("native-direct-versus-queued-glow",queued==row.queued && recipient.calls==row.glow);
        if (row.glow) check("native-glow-color-and-apply-flags",recipient.color==(DWORD(row.r)<<8 | DWORD(row.g)<<16 | DWORD(row.b)<<24)
            && recipient.flags==(30u<<16) && recipient.list==CGameAIBase::EFFECT_LIST_TIMED && recipient.save==!row.queued && recipient.immediate);
        delete p;
    }
    objects->Delete(recipient.m_id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE); recipient.m_id=-1;
    for (INT positive : {2,255,257}) {
        auto* p=CProjectileSummonVFX::DecodeSpellHitProjectile(3,NULL,positive);
        check("native-positive-is-exact-low-byte-one",p->m_fireSoundRef==CResRef(positive==257?"EFF_P05":"EFF_M06")); delete p;
    }
    SchoolHitAccess fixture; CVidCell* cell=fixture.m_pVidCell;
    check("invocation-animation-available",cell->pRes!=NULL);
    if (cell->pRes) {
        ProjectileRenderFrameAnchor anchor(cell); auto* resource=cell->pRes; auto* header=cell->m_header.pRes;
        SEQUENCEENTRY sequence=resource->m_pSequences[0]; sequence.nFrames=4;
        std::vector<SEQUENCEENTRY> sequences(3,sequence);
        auto* original=resource->m_pSequences; auto* originalHeader=header?header->m_pSequences:NULL;
        struct Count { BAMHEADER* p; BYTE count; }; std::vector<Count> counts;
        for (auto* h : {resource->m_pBamHeader,resource->m_pBamHeaderCopy,header?header->m_pBamHeader:NULL,header?header->m_pBamHeaderCopy:NULL}) if (h) {
            BOOL seen=FALSE; for (const auto& saved : counts) if (saved.p==h) seen=TRUE;
            if (!seen) { counts.push_back({h,h->nSequences}); h->nSequences=3; }
        }
        resource->m_pSequences=sequences.data(); if (header) header->m_pSequences=sequences.data();
        SchoolSoundBuffer sound;
        for (const auto& row : SCHOOL_HIT_AI) {
            BOOL destroyed=FALSE;
            struct Sample : SchoolHitAccess { BOOL* destroyed=NULL; ~Sample() override { m_sound.pSoundBuffer=NULL; if (destroyed) *destroyed=TRUE; } };
            auto* p=new Sample; p->destroyed=&destroyed;
            p->m_pVidCell->m_nCurrentFrame=row.frame; p->m_pVidCell->m_nCurrentSequence=row.sequence;
            p->m_loopSequence=row.loop; p->m_loopSequenceIndex=1; p->m_loopCount=row.count; p->m_singleSequence=row.single;
            p->m_sound.pSoundBuffer=row.sound?&sound:NULL;
            objects->Add(&p->m_id,p,INFINITE); game->m_nTimeStop=row.stop; game->m_nTimeStopCaster=row.stop==1?p->m_id+1:p->m_id;
            p->AIUpdate(); ++replays; check("native-overlay-retirement",destroyed==row.retired);
            if (!destroyed) {
                check("native-overlay-frame-loop-and-sound-wait",p->m_pVidCell->m_nCurrentFrame==row.afterFrame
                    && p->m_pVidCell->m_nCurrentSequence==row.afterSequence && p->m_loopCount==row.afterCount && p->m_visible==row.visible);
                p->m_sound.pSoundBuffer=NULL; retire(p);
            }
        }
        game->m_nTimeStop=0;
        // Fire keeps pre-existing payload metadata and delivers BEFORE registration.
        for (BYTE random : {BYTE(0),BYTE(1),BYTE(2)}) for (BYTE offset : {BYTE(0),BYTE(1),BYTE(2)}) for (BOOL failure : {FALSE,TRUE}) {
            BOOL destroyed=FALSE;
            struct Launch : SchoolHitAccess {
                BOOL* destroyed=NULL; BOOL delivered=FALSE; BOOL before=FALSE;
                void DeliverEffects() override { delivered=TRUE; before=m_id==CGameObjectArray::INVALID_INDEX && m_sourceId==71 && m_targetId==72; }
                ~Launch() override { if (destroyed) *destroyed=TRUE; }
            };
            auto* p=new Launch; p->destroyed=&destroyed; p->m_randomSequence=random; p->m_offsetAboveTarget=offset;
            p->m_sourceId=71; p->m_targetId=72; p->m_posZ=17;
            SHORT oldLimit=objects->m_nNextObjectId; if (failure) objects->m_nNextObjectId=0x7FFF;
            srand(1); p->Fire(area,11,22,CPoint(256,LONG(0x80000000)),31,13); objects->m_nNextObjectId=oldLimit;
            check("native-fire-registration-failure",destroyed==failure); check("native-fire-rng",rand()==(random==1?18467:41));
            if (!destroyed) {
                check("native-fire-order-and-preserved-ids",p->delivered && p->before && p->m_sourceId==71 && p->m_targetId==72);
                check("native-fire-offset-and-random-sequence",p->m_pos==CPoint(256,static_cast<LONG>(0x80000000u+(offset==1?-100:1)))
                    && p->m_posZ==0 && p->m_pVidCell->m_nCurrentSequence==(random==1?2:0));
                retire(p);
            }
        }
        resource->m_pSequences=original; if (header) header->m_pSequences=originalHeader;
        for (const auto& saved : counts) saved.p->nSequences=saved.count;
        check("overlay-bam-pointers-and-demands-restored",anchor.Restore());
    }
    LONG actorId=game->GetFixedOrderCharacterId(1); CGameObject* raw=NULL;
    BYTE rc=objects->GetDeny(actorId,CGameObjectArray::THREAD_ASYNCH,&raw,INFINITE);
    check("opcode-233-party-target-available",rc==CGameObjectArray::SUCCESS);
    if (rc==CGameObjectArray::SUCCESS) {
        auto* sprite=static_cast<CGameSprite*>(raw); LONG hp=sprite->m_baseStats.m_hitPoints;
        INT timed=sprite->m_timedEffectList.GetCount(); CPoint pos=sprite->GetPos();
        ITEM_EFFECT item={}; item.effectID=ICEWIND_CGAMEEFFECT_VISUALSPELLHIT;
        for (DWORD type : {9u,18u,21u,45u,57u,58u,69u,73u,74u,75u,76u,91u,98u,104u,110u,111u}) {
            item.dwFlags=type; auto* effect=CGameEffect::DecodeEffect(&item,pos,actorId,pos); effect->m_sourceRes="SPPTEST";
            check("opcode-233-decoder",dynamic_cast<CGameEffectVisualSpellHitIWD*>(effect)!=NULL);
            auto* copy=effect->Copy(); check("opcode-233-copy",copy->m_dwFlags==type && copy->m_sourceRes==effect->m_sourceRes); delete copy;
            check("opcode-233-virtual-completion",!effect->ApplyEffect(sprite) && effect->m_done);
            check("opcode-233-one-local-carrier",area->m_lVertSortAdd.GetCount()==front+1);
            if (area->m_lVertSortAdd.GetCount()>front) {
                LONG id=reinterpret_cast<LONG>(area->m_lVertSortAdd.GetTail()); CGameObject* child=NULL;
                BYTE shared=objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&child,INFINITE);
                check("opcode-233-registered-carrier",shared==CGameObjectArray::SUCCESS);
                if (shared==CGameObjectArray::SUCCESS) {
                    auto* overlay=dynamic_cast<CProjectileSummonVFX*>(child);
                    LONG y=pos.y+(type==45 || type==57?13:overlay && overlay->*(&SchoolHitAccess::m_offsetAboveTarget)==1?-100:1);
                    check("opcode-233-native-position",child->m_pos==CPoint(pos.x,y) && child->m_posZ==0);
                    objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE); retire(child);
                }
            }
            INT launches=0;
            while (handler->m_messageList.GetCount()>messages) {
                CMessage* message=handler->m_messageList.RemoveTail(); auto* launch=dynamic_cast<CMessageFireProjectile*>(message);
                auto* group=dynamic_cast<CMessage105*>(message);
                check("opcode-233-owned-messages",launch || (group && group->m_nCount==0));
                if (launch) {
                    ++launches; check("opcode-233-replication-type-and-source",launch->m_projectileType==type+0x1001 && launch->m_projectileTargetId==actorId
                        && launch->m_sourceId==actorId && launch->m_targetId==actorId && launch->m_projectileTarget==pos && launch->m_height==0 && launch->field_1E==0);
                    SHORT next=objects->m_nNextObjectId; launch->Run();
                    check("opcode-233-owner-gate-no-duplicate",objects->m_nNextObjectId==next && area->m_lVertSortAdd.GetCount()==front);
                }
                delete message;
            }
            check("opcode-233-single-replication-and-no-damage",launches==1 && sprite->m_baseStats.m_hitPoints==hp && sprite->m_timedEffectList.GetCount()==timed);
            delete effect;
        }
        CGameArea* savedArea=sprite->m_pArea; sprite->m_pArea=NULL; item.dwFlags=0xFFFFFFFFu;
        auto* effect=CGameEffect::DecodeEffect(&item,pos,actorId,pos);
        check("opcode-233-no-area-before-selector",!effect->ApplyEffect(sprite) && effect->m_done
            && handler->m_messageList.GetCount()==messages && area->m_lVertSortAdd.GetCount()==front);
        delete effect; sprite->m_pArea=savedArea; objects->ReleaseDeny(actorId,CGameObjectArray::THREAD_ASYNCH,INFINITE);
    }
    game->m_nTimeStop=stop; game->m_nTimeStopCaster=caster; srand(1);
    check("school-factory-and-ai-restored",clock==game->GetWorldTimer()->m_gameTime && handler->m_messageList.GetCount()==messages && area->m_lVertSortAdd.GetCount()==front);
    Emit("{\"step\":%d,\"op\":\"schoolhitchecks\",\"checks\":%d,\"nativeReplays\":%d,\"nativeCases\":3044,\"restored\":true,\"valid\":%s}",nStep,checks,replays,valid?"true":"false");
    if (!valid) Fail(nStep,"schoolhitchecks","school-hit factory/overlay native state mismatch");
}
