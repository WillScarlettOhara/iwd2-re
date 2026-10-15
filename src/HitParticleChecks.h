// AutoUI scaffolding; typed state only, no original offsets into this build.
#include "HitParticleGolden.h"

struct HitParticleBitmapFixture {
    CVidBitmap& bitmap;
    CResBitmap resource;
    CResBitmap* saved;
    SHORT bits;
    BOOL doubleSize;
    BITMAPINFOHEADER header={};
    RGBQUAD palette[256]={};
    std::vector<BYTE> pixels;
    HitParticleBitmapFixture(CVidBitmap& target,BOOL height) : bitmap(target),saved(target.pRes),bits(target.m_nBitCount),doubleSize(target.m_bDoubleSize) {
        header.biSize=sizeof(header); header.biWidth=512; header.biHeight=512; header.biPlanes=1; header.biBitCount=8;
        INT stride=512; pixels.resize(stride*512+4);
        if (height) std::fill(pixels.begin(),pixels.end(),BYTE(3));
        else for (INT y=0;y<512;++y) for (INT x=0;x<512;++x) {
            pixels[(511-y)*stride+x]=BYTE(y*16);
        }
        for (INT i=0;i<256;++i) palette[i].rgbBlue=palette[i].rgbGreen=palette[i].rgbRed=BYTE(i);
        resource.pBitmapInfoHeader=&header; resource.pBitmapData=pixels.data(); resource.nXWords=stride/4; resource.bParsed=TRUE;
        resource.pColorTable=palette; resource.nColors=256;
        bitmap.pRes=&resource; bitmap.m_nBitCount=8; bitmap.m_bDoubleSize=FALSE;
    }
    ~HitParticleBitmapFixture() { bitmap.pRes=saved; bitmap.m_nBitCount=bits; bitmap.m_bDoubleSize=doubleSize; }
};

struct HitParticleTestAnimation : IcewindCGameAnimationTypeEffect {
    BOOL ended=FALSE,drawn=FALSE;
    INT increments=0;
    SHORT direction=-999;
    LONG draw[14]={};
    HitParticleTestAnimation() : IcewindCGameAnimationTypeEffect(IcewindCSpellHitEmission(),0) {}
    BOOL IsEndOfSequence() override { return ended; }
    void IncrementFrame() override { ++increments; }
    void ChangeDirection(SHORT value) override { direction=value; }
    void CalculateFxRect(CRect& r,CPoint& p,LONG) override { r=CRect(0,0,10,12); p=CPoint(3,4); }
    void CalculateGCBoundsRect(CRect& r,const CPoint& p,const CPoint& a,LONG z,LONG w,LONG h) override {
        r.left=LONG(DWORD(p.x)-DWORD(a.x)); r.top=LONG(DWORD(p.y)-DWORD(a.y)+DWORD(z));
        r.right=LONG(DWORD(r.left)+DWORD(w)); r.bottom=LONG(DWORD(r.top)+DWORD(h));
    }
    void Render(CInfinity*,CVidMode*,INT,const CRect&,const CPoint& p,const CPoint& a,DWORD flags,COLORREF tint,const CRect& r,BOOL dither,BOOL fade,LONG z,BYTE transparency) override {
        drawn=TRUE; LONG state[]={p.x,p.y,a.x,a.y,LONG(flags),LONG(tint),r.left,r.top,r.right,r.bottom,dither,fade,z,transparency};
        memcpy(draw,state,sizeof(draw));
    }
};

void HitParticleChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray(); auto* area=game->GetVisibleArea();
    auto* handler=g_pBaldurChitin->GetMessageHandler(); auto* video=static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
    if (!area || !area->m_visibility.m_pMap || g_pChitin->cVideo.m_bIs3dAccelerated) { Fail(nStep,"hitparticlechecks","requires software area"); return; }
    INT checks=0,draws=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"hit-particle-case\",\"case\":\"%s\",\"passed\":false}",nStep,name); };
    auto stop=game->m_nTimeStop,caster=game->m_nTimeStopCaster; ULONG clock=game->GetWorldTimer()->m_gameTime;
    INT messages=handler->m_messageList.GetCount(),front=area->m_lVertSortAdd.GetCount();
    auto unpend=[&](CGameObject* p) { POSITION at=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(p->m_id)); if (at) area->m_lVertSortAdd.RemoveAt(at); };
    auto retire=[&](CGameObject* p) { unpend(p); p->CGameObject::RemoveFromArea(); objects->Delete(p->m_id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE); delete p; };
    IcewindCSpellHitEmission empty;
    for (const auto& row : HIT_PARTICLE_CTORS) {
        IcewindCSpellHitEmission e; IcewindCSpellHitEmissionRanged r;
        e.m_resref0=r.m_resref0=const_cast<char*>("FirebaR"); e.m_resref0Len=r.m_resref0Len=7;
        r.m_cellPool=new IcewindCSpellHitCellPool; r.m_cellPool->m_refCount=9; r.m_lastCellIndex=3; r.m_respawnFlag=2; r.m_cloudFlag=255;
        auto* p=row.ranged?new IcewindCSpellHitParticle(r,area,CPoint(-1025,98302),-17,CPoint(-1024,2048),row.life,255,row.collision)
            :new IcewindCSpellHitParticle(e,area,CPoint(-1025,98302),-17,CPoint(-1024,2048),row.life,255,row.collision);
        auto* anim=dynamic_cast<IcewindCGameAnimationTypeEffect*>(p->m_animation.m_animation);
        check("native-particle-constructor-launch",p->m_pos==CPoint(row.position[0],row.position[1]) && p->m_posExact==CPoint(-1025,98302)
            && p->m_posDelta==CPoint(-1024,2048) && p->m_duration==row.life && p->m_durationFade==255 && p->m_collision==row.collision
            && p->m_posZ==-17 && p->m_animationRunning==1 && p->m_animTick==0 && anim && USHORT(anim->m_currentBamDirection)==row.direction);
        check("native-particle-constructor-pool",r.m_cellPool->m_refCount==row.references
            && (row.ranged?(p->m_cellPool==r.m_cellPool && p->m_cellIndex==3 && p->m_respawnFromPool==2 && p->m_hasCloud==255):
                (!p->m_cellPool && p->m_cellIndex==0 && p->m_respawnFromPool==0 && p->m_hasCloud==0)));
        retire(p); check("particle-constructor-reference-balanced",r.m_cellPool->m_refCount==9); delete r.m_cellPool;
    }
    struct Sample : IcewindCSpellHitParticle {
        BOOL retired=FALSE;
        Sample(const IcewindCSpellHitEmission& e,CGameArea* a) : IcewindCSpellHitParticle(e,a,CPoint(0,0),0,CPoint(0,0),10,0,0) {}
        void RemoveFromArea() override { retired=TRUE; }
    };
    auto& search=area->m_search; auto* oldCosts=search.m_pDynamicCost; CSize dimensions=search.m_GridSquareDimensions;
    // Width zero makes the search index just X; centering the fixture storage
    // covers every signed tile coordinate reached after DWORD wrap.
    std::vector<BYTE> costs(262145,0x80); search.m_pDynamicCost=costs.data()+131072; search.m_GridSquareDimensions=CSize(0,0);
    for (const auto& row : HIT_PARTICLE_AI) {
        auto* p=new Sample(empty,area); unpend(p); delete p->m_animation.m_animation;
        auto* anim=new HitParticleTestAnimation; p->m_animation.m_animation=anim; anim->ended=row.end; anim->m_animResName=row.name;
        p->m_pos=CPoint(row.position[0],row.position[1]); p->m_posExact=CPoint(row.exact[0],row.exact[1]); p->m_posDelta=CPoint(row.velocity[0],row.velocity[1]);
        p->m_duration=row.life; p->m_collision=row.collision; p->m_hasCloud=row.cloud; p->m_respawnFromPool=row.respawn; p->m_animTick=row.tick; p->m_animationRunning=row.visible;
        p->m_cellPool=new IcewindCSpellHitCellPool; p->m_cellPool->m_refCount=1; p->m_cellIndex=row.index;
        for (INT i=0;i<row.count;++i) { IcewindCSpellHitCell cell={i*1024-1025,i*2048-1,row.flags[i],0xab}; p->m_cellPool->m_cells.push_back(cell); }
        p->m_terrainTable[0]=p->m_terrainTable[8]=row.wall?255:0;
        game->m_nTimeStop=row.stop; game->m_nTimeStopCaster=row.stop==1?p->m_id+1:p->m_id;
        srand(row.seed); p->AIUpdate();
        LONG state[]={p->m_pos.x,p->m_pos.y,p->m_posExact.x,p->m_posExact.y,p->m_posDelta.x,p->m_posDelta.y,p->m_duration,p->m_animationRunning,p->m_cellIndex,p->m_animTick};
        check("native-particle-life-wrap-motion",memcmp(state,row.state,sizeof(state))==0 && p->retired==row.retired);
        check("native-particle-animation-and-direction",anim->increments==row.increments && anim->direction==row.direction);
        check("native-particle-cloud-name",anim->m_animResName.Compare(0,anim->m_animResName.Length(),row.afterName,INT(strlen(row.afterName)))==0);
        check("native-particle-rng-consumption",rand()==row.nextRandom);
        BOOL flags=TRUE; for (INT i=0;i<row.count;++i) flags=flags && p->m_cellPool->m_cells[i].flag==row.afterFlags[i] && p->m_cellPool->m_cells[i].padding==0xab;
        check("native-particle-shared-cell-byte-flags",flags); retire(p);
    }
    search.m_pDynamicCost=oldCosts; search.m_GridSquareDimensions=dimensions; game->m_nTimeStop=0;
    IcewindCSpellHitEmission named; named.m_resref0=const_cast<char*>("ICloudB"); named.m_resref0Len=7;
    IcewindCGameAnimationTypeEffect cloudFixture(named,0);
    auto* cloudRes=cloudFixture.m_g1VidCell.pRes; check("particle-cloud-bam-available",cloudRes!=NULL);
    if (cloudRes) {
        ProjectileRenderFrameAnchor anchor(&cloudFixture.m_g1VidCell); auto* head=cloudFixture.m_g1VidCell.m_header.pRes;
        struct Count { BAMHEADER* p; BYTE n; }; std::vector<Count> saved;
        for (auto* h : {cloudRes->m_pBamHeader,cloudRes->m_pBamHeaderCopy,head?head->m_pBamHeader:NULL,head?head->m_pBamHeaderCopy:NULL}) if (h) {
            BOOL found=FALSE; for (const auto& s : saved) if (s.p==h) found=TRUE; if (!found) saved.push_back({h,h->nSequences});
        }
        for (const auto& row : HIT_ANIMATION_CTORS) {
            for (const auto& s : saved) s.p->nSequences=BYTE(row.count);
            IcewindCSpellHitEmission e; IcewindCSpellHitEmissionRanged r;
            e.m_resref0=r.m_resref0=const_cast<char*>(row.name); e.m_resref0Len=r.m_resref0Len=row.name?INT(strlen(row.name)):7;
            e.m_animMode=r.m_animMode=row.mode; r.m_animFlag36=2;
            srand(1); IcewindCGameAnimationTypeEffect* p=row.ranged?new IcewindCGameAnimationTypeEffect(r,row.facing):new IcewindCGameAnimationTypeEffect(e,row.facing);
            check("native-ember-constructor-state",p->m_currentBamSequence==row.sequence && USHORT(p->m_currentBamDirection)==row.direction
                && p->m_animMode==row.afterMode && p->m_animFlag36==row.single && p->m_bRender==row.visible
                && BYTE(p->m_animResName.m_buf[0])==BYTE(row.facing) && p->m_currentVidCell==&p->m_g1VidCell && !p->m_currentVidCellShadow);
            const char* expected=row.name && *row.name?row.name:"";
            check("native-ember-constructor-keeps-name",p->m_animResName.Length()==INT(strlen(expected)) && (p->m_animResName.Length()==0 || strcmp(p->m_animResName.Data(),expected)==0));
            check("native-ember-constructor-rng",rand()==(row.randomCalls?18467:41)); delete p;
        }
        for (const auto& s : saved) s.p->nSequences=s.n;
        check("particle-cloud-resource-restored",anchor.Restore());
    }
    IcewindCSpellHitEmission effect; effect.m_resref0=const_cast<char*>("FirebaR"); effect.m_resref0Len=7;
    IcewindCGameAnimationTypeEffect animation(effect,0); CVidCell alternate(CResRef("FirebaA"),FALSE);
    check("particle-animation-bams-available",animation.m_g1VidCell.pRes && alternate.pRes);
    if (animation.m_g1VidCell.pRes && alternate.pRes) {
        ProjectileRenderFrameAnchor mainAnchor(&animation.m_g1VidCell),otherAnchor(&alternate);
        animation.m_g1VidCellShadow.SetResRef(CResRef("FirebaR"),FALSE,TRUE,TRUE);
        auto* main=animation.m_g1VidCell.pRes; auto* head=animation.m_g1VidCell.m_header.pRes;
        auto* other=alternate.pRes; auto* otherHead=alternate.m_header.pRes;
        auto* oldOther=other->m_pSequences; auto* oldOtherHead=otherHead?otherHead->m_pSequences:NULL;
        SEQUENCEENTRY sequence=oldOther[0]; sequence.nFrames=4; other->m_pSequences=&sequence; if (otherHead) otherHead->m_pSequences=&sequence;
        struct Count { BAMHEADER* p; BYTE n; }; std::vector<Count> counts;
        for (auto* h : {main->m_pBamHeader,main->m_pBamHeaderCopy,head?head->m_pBamHeader:NULL,head?head->m_pBamHeaderCopy:NULL}) if (h) {
            BOOL found=FALSE; for (const auto& s : counts) if (s.p==h) found=TRUE; if (!found) counts.push_back({h,h->nSequences});
        }
        for (const auto& row : HIT_ANIMATION_ADVANCE) {
            for (const auto& s : counts) s.p->nSequences=BYTE(row.count);
            animation.m_currentVidCell=&alternate; animation.m_currentVidCellShadow=row.shadow?&animation.m_g1VidCellShadow:NULL;
            animation.m_currentBamSequence=row.sequence; animation.m_animMode=row.mode; animation.m_animFlag36=row.single; animation.m_bRender=1;
            alternate.m_nCurrentFrame=row.end?3:0; alternate.m_nCurrentSequence=0;
            animation.m_g1VidCell.m_nCurrentFrame=7; animation.m_g1VidCell.m_nCurrentSequence=0;
            animation.m_g1VidCellShadow.m_nCurrentFrame=9; animation.m_g1VidCellShadow.m_nCurrentSequence=0;
            srand(1); animation.IncrementFrame(); LONG state[]={animation.m_currentBamSequence,animation.m_bRender,alternate.m_nCurrentFrame,
                animation.m_g1VidCell.m_nCurrentFrame,animation.m_g1VidCell.m_nCurrentSequence,animation.m_g1VidCellShadow.m_nCurrentFrame,animation.m_g1VidCellShadow.m_nCurrentSequence};
            check("native-ember-byte-sequence-wrap-and-shadow",memcmp(state,row.state,sizeof(state))==0);
            check("native-ember-advance-rng",rand()==row.nextRandom);
        }
        for (const auto& s : counts) s.p->nSequences=s.n;
        other->m_pSequences=oldOther; if (otherHead) otherHead->m_pSequences=oldOtherHead;
        animation.m_currentVidCell=&animation.m_g1VidCell; animation.m_currentVidCellShadow=NULL;
        check("particle-main-resource-restored",mainAnchor.Restore()); check("particle-alternate-resource-restored",otherAnchor.Restore());
    }
    // Deterministic height and luminosity maps make the tint coordinate observable.
    {
        HitParticleBitmapFixture height(area->m_bmHeight,TRUE),light(area->m_bmLum,FALSE);
        auto& infinity=area->m_cInfinity; CRect viewport=infinity.rViewPort; INT cameraX=infinity.nCurrentX,cameraY=infinity.nCurrentY;
        auto areaType=infinity.m_areaType; auto renderCode=infinity.m_renderDayNightCode; infinity.m_areaType|=0x40; infinity.m_renderDayNightCode=1;
        BYTE* map=area->m_visibility.m_pMap; auto size=area->m_visibility.m_nMapSize; auto width=area->m_visibility.m_nWidth;
        std::vector<BYTE> visibility(32768,CVisibilityMap::VISIBILITY_MASK); area->m_visibility.m_pMap=visibility.data(); area->m_visibility.m_nMapSize=INT(visibility.size()); area->m_visibility.m_nWidth=128;
        infinity.nCurrentX=80; infinity.nCurrentY=80; infinity.rViewPort=CRect(0,0,320,240);
        auto* p=new Sample(empty,area); unpend(p); delete p->m_animation.m_animation;
        auto* anim=new HitParticleTestAnimation; p->m_animation.m_animation=anim; p->m_listType=CGameObject::LIST_FLIGHT;
        for (const auto& row : HIT_PARTICLE_RENDER) {
            p->m_pos=CPoint(row.position[0],row.position[1]); p->m_posZ=17; p->m_animationRunning=row.visible; p->m_duration=row.life; p->m_durationFade=row.fade; anim->drawn=FALSE;
            memset(visibility.data(),row.hidden?0:CVisibilityMap::VISIBILITY_MASK,visibility.size()); p->Render(NULL,video,CVIDINF_SURFACE_BACK);
            check("native-particle-render-gates-and-complete-bounds",anim->drawn==row.draws && (!row.draws || memcmp(anim->draw,row.draw,sizeof(row.draw))==0));
        }
        memset(visibility.data(),CVisibilityMap::VISIBILITY_MASK,visibility.size()); p->m_pos=CPoint(79,96); p->m_listType=CGameObject::LIST_FRONT; p->m_animationRunning=1;
        for (LONG z : {-17L,0L,17L}) { p->m_posZ=z; anim->drawn=FALSE; p->Render(NULL,video,CVIDINF_SURFACE_BACK);
            COLORREF expected=area->GetTintColor(CPoint(79,101+z),CGameObject::LIST_FRONT),wrong=area->GetTintColor(CPoint(79,96+z),CGameObject::LIST_FRONT);
            check("particle-tint-uses-terrain-height",anim->drawn && DWORD(anim->draw[5])==expected && expected!=wrong);
        }
        retire(p); area->m_visibility.m_pMap=map; area->m_visibility.m_nMapSize=size; area->m_visibility.m_nWidth=width;
        infinity.rViewPort=viewport; infinity.nCurrentX=cameraX; infinity.nCurrentY=cameraY; infinity.m_areaType=areaType; infinity.m_renderDayNightCode=renderCode;
    }
    // Full software pixels for every native blend/transparency/shadow/fade plan.
    {
        ProjectileRenderFrameAnchor anchor(&animation.m_g1VidCell),shadowAnchor(&animation.m_g1VidCellShadow);
        auto& infinity=area->m_cInfinity; CRect view=infinity.rViewPort; INT cameraX=infinity.nCurrentX,cameraY=infinity.nCurrentY; auto* wed=infinity.pResWED;
        CRect patch(64,64,416,336); std::vector<BYTE> saved;
        auto pixels=[&](std::vector<BYTE>* read,const std::vector<BYTE>* write,BOOL clear)->BOOL {
            if (!video->BKLock(patch)) return FALSE; DWORD bytes=video->m_SurfaceDesc.ddpfPixelFormat.dwRGBBitCount/8; INT row=patch.Width()*bytes;
            BOOL ok=bytes>=2 && bytes<=4 && video->m_SurfaceDesc.lpSurface!=NULL; if (ok && read) read->resize(row*patch.Height());
            if (ok) for (INT y=0;y<patch.Height();++y) { BYTE* p=static_cast<BYTE*>(video->m_SurfaceDesc.lpSurface)+y*video->m_SurfaceDesc.lPitch;
                if (read) memcpy(read->data()+y*row,p,row); if (write) memcpy(p,write->data()+y*row,row); if (clear) memset(p,0,row); }
            video->BKUnlock(); return ok;
        };
        BOOL backed=pixels(&saved,NULL,FALSE); check("ember-pixels-backed-up",backed);
        infinity.pResWED=NULL; infinity.rViewPort=CRect(80,80,400,320); infinity.nCurrentX=72; infinity.nCurrentY=882;
        animation.m_currentVidCell=&animation.m_g1VidCell; animation.m_currentBamDirection=0; animation.m_g1VidCell.m_nCurrentFrame=0; animation.m_g1VidCell.m_nCurrentSequence=0;
        animation.m_g1VidCellShadow.m_nCurrentFrame=0; animation.m_g1VidCellShadow.m_nCurrentSequence=0;
        for (const auto& row : HIT_EMBER_RENDER) {
            animation.m_currentVidCellShadow=row.shadow?&animation.m_g1VidCellShadow:NULL; animation.m_bRender=1;
            auto& visual=animation.m_visualEffect; visual.m_bCopyFromBack=row.copy; visual.m_bTransparent=row.transparent; visual.m_bTintEnabled=row.tint; visual.field_2=row.extra;
            visual.m_dwFlags=0x20000; visual.m_nTransValue=73;
            CRect fx; CPoint center; animation.CalculateFxRect(fx,center,17); CPoint foot(232,1002),point(232,1019); CRect bounds(220,990,244,1030);
            std::vector<BYTE> expected,actual; BOOL ok=backed && pixels(NULL,NULL,TRUE);
            animation.m_g1VidCell.SetTintColor(RGB(255,255,255)); infinity.FXPrep(fx,row.prep,CVIDINF_SURFACE_BACK,point,center); ok=infinity.FXLock(fx,row.prep) && ok;
            if (ok) { if (row.tint==1) animation.m_g1VidCell.SetTintColor(0x123456);
                if (row.shadow) infinity.FXRender(&animation.m_g1VidCellShadow,center.x,center.y-17,0x20004,0);
                infinity.FXRender(&animation.m_g1VidCell,center.x,center.y,0x20000,row.transparency);
                infinity.FXUnlock(row.prep,row.fade?&fx:NULL,row.fade?point+center:CPoint(0,0));
                ok=infinity.FXBltFrom(CVIDINF_SURFACE_BACK,fx,point.x,point.y,center.x,center.y,0x20001) && ok;
            }
            ok=pixels(&expected,NULL,TRUE) && ok; animation.m_g1VidCell.SetTintColor(RGB(255,255,255));
            animation.Render(&infinity,video,CVIDINF_SURFACE_BACK,fx,foot,center,0x20000,0x123456,bounds,FALSE,row.fade,17,255); ++draws;
            ok=pixels(&actual,NULL,FALSE) && ok; check("native-ember-full-software-pixels",ok && actual==expected);
            if (!row.copy && !row.transparent && !row.tint && !row.extra && !row.shadow && !row.fade)
                check("ember-really-renders",std::any_of(actual.begin(),actual.end(),[](BYTE value){return value!=0;}));
        }
        animation.m_currentVidCellShadow=NULL; infinity.pResWED=wed; infinity.rViewPort=view; infinity.nCurrentX=cameraX; infinity.nCurrentY=cameraY;
        // Both cells use FirebaR; unwind nested metadata/demand snapshots in
        // reverse order so the outer anchor restores the original resource.
        check("ember-pixels-restored",backed && pixels(NULL,&saved,FALSE)); check("ember-shadow-restored",shadowAnchor.Restore()); check("ember-main-restored",anchor.Restore());
    }
    game->m_nTimeStop=stop; game->m_nTimeStopCaster=caster; srand(1);
    check("particle-fixtures-restored",clock==game->GetWorldTimer()->m_gameTime && handler->m_messageList.GetCount()==messages && area->m_lVertSortAdd.GetCount()==front);
    Emit("{\"step\":%d,\"op\":\"hitparticlechecks\",\"checks\":%d,\"renderCalls\":%d,\"nativeCases\":2604,\"restored\":true,\"valid\":%s}",nStep,checks,draws,valid?"true":"false");
    if (!valid) Fail(nStep,"hitparticlechecks","particle or ember animation differs from original capture");
}
