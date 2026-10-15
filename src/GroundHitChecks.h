// AutoUI scaffolding; included inside its private namespace.
#include "GroundHitGolden.h"

void GroundHitChecks(INT nStep)
{
    auto* game=g_pBaldurChitin->GetObjectGame(); auto* objects=game->GetObjectArray(); auto* area=game->GetVisibleArea();
    auto* handler=g_pBaldurChitin->GetMessageHandler(); auto* video=static_cast<CVidInf*>(g_pChitin->GetCurrentVideoMode());
    if (!area || !area->m_visibility.m_pMap || g_pChitin->cVideo.m_bIs3dAccelerated) { Fail(nStep,"groundhitchecks","requires software area"); return; }
    INT checks=0,draws=0,particles=0; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"ground-hit-case\",\"case\":\"%s\",\"passed\":false}",nStep,name); };
    auto stop=game->m_nTimeStop,caster=game->m_nTimeStopCaster; ULONG clock=game->GetWorldTimer()->m_gameTime;
    INT messages=handler->m_messageList.GetCount(),front=area->m_lVertSortAdd.GetCount();
    auto unpend=[&](CGameObject* p) { POSITION at=area->m_lVertSortAdd.Find(reinterpret_cast<int*>(p->m_id)); if (at) area->m_lVertSortAdd.RemoveAt(at); };
    auto retire=[&](CGameObject* p) { unpend(p); p->CGameObject::RemoveFromArea(); objects->Delete(p->m_id,CGameObjectArray::THREAD_ASYNCH,NULL,INFINITE); delete p; };
    IcewindCSpellHitEmission empty;
    IcewindCSpellHitEmissionRanged emptyRanged;
    for (const auto& row : GROUND_FANS) {
        auto* p=new IcewindCSpellHitVisual(empty,empty,emptyRanged,area,CPoint(160,row.y),row.range,row.speed,0,20);
        check("native-ground-fan-geometry",p->m_coverHalfW==row.geometry[0] && p->m_coverHalfH==row.geometry[1]
            && p->m_arcLen1==row.geometry[2] && p->m_arcLen2==row.geometry[3]);
        INT cells=(row.geometry[2]+row.geometry[3])*4;
        check("native-ground-rounded-velocities",memcmp(p->m_fanCells,row.fan,cells*16)==0);
        check("native-ground-axis-flags",memcmp(p->m_fanVel,row.flags,cells)==0);
        check("native-ground-registration",p->m_pArea==area && p->m_pos==CPoint(160,row.y) && p->m_posZ==0
            && p->m_frameCount==BYTE((row.range-1)/row.speed+1)); retire(p);
    }
    // Four independently counted name slots; frozen/saturated buffers must detach.
    for (BYTE count : {BYTE(0),BYTE(1),BYTE(0xFD),BYTE(0xFE),BYTE(0xFF)}) {
        char storage[4][34]; IcewindCSpellHitEmission e1; IcewindCSpellHitEmissionRanged e2;
        for (INT i=0;i<4;++i) { storage[i][0]=char(count); strcpy(storage[i]+1,"RNG_M03"); }
        e1.m_resref0=storage[0]+1; e1.m_resref1=storage[1]+1; e2.m_resref0=storage[2]+1; e2.m_resref1=storage[3]+1;
        e1.m_resref0Len=e1.m_resref1Len=e2.m_resref0Len=e2.m_resref1Len=7;
        e1.m_resref0Cap=e1.m_resref1Cap=e2.m_resref0Cap=e2.m_resref1Cap=31;
        e1.m_resref0Flags=0x77; e2.m_resref1Flags=0x88;
        auto* p=new IcewindCSpellHitVisual(empty,e1,e2,area,CPoint(160,96),200,10,0,20);
        char* names[]={p->m_emission1.m_resref0,p->m_emission1.m_resref1,p->m_emission2.m_resref0,p->m_emission2.m_resref1};
        for (INT i=0;i<4;++i) check("ground-name-retain-or-detach",strcmp(names[i],storage[i]+1)==0
            && (names[i]==storage[i]+1)==(count<0xFE) && BYTE(storage[i][0])==BYTE(count+(count<0xFE)));
        check("ground-name-flags-not-copied",p->m_emission1.m_resref0Flags==0 && p->m_emission2.m_resref1Flags==0);
        retire(p); for (INT i=0;i<4;++i) check("ground-name-release-balanced",BYTE(storage[i][0])==count);
    }
    // Real BAM predicates with a pinned single sequence; no offsets into our build.
    IcewindCSpellHitEmission explosion; char bam[]="FirebaX"; explosion.m_resref0=bam; explosion.m_resref0Len=7;
    auto* fixture=new IcewindCSpellHitVisual(explosion,empty,emptyRanged,area,CPoint(160,96),200,10,0,20);
    check("ground-impact-bam-available",fixture->m_cell.pRes!=NULL);
    if (fixture->m_cell.pRes) {
        ProjectileRenderFrameAnchor anchor(&fixture->m_cell); auto* resource=fixture->m_cell.pRes; auto* header=fixture->m_cell.m_header.pRes;
        SEQUENCEENTRY seq=resource->m_pSequences[0]; seq.nFrames=4;
        auto* original=resource->m_pSequences; auto* originalHeader=header?header->m_pSequences:NULL;
        resource->m_pSequences=&seq; if (header) header->m_pSequences=&seq;
        struct Sample : IcewindCSpellHitVisual {
            BOOL retired=FALSE;
            Sample(const IcewindCSpellHitEmission& a,const IcewindCSpellHitEmission& b,const IcewindCSpellHitEmissionRanged& c,CGameArea* area)
                : IcewindCSpellHitVisual(a,b,c,area,CPoint(160,96),200,10,0,20) {}
            void RemoveFromArea() override { retired=TRUE; }
        };
        for (const auto& row : GROUND_GATES) {
            auto* p=new Sample(explosion,empty,emptyRanged,area); p->m_frameCount=row.count; p->m_bamLoaded=row.loaded;
            p->m_arcLen1=p->m_arcLen2=0; p->m_cell.m_nCurrentFrame=row.end?3:0;
            game->m_nTimeStop=row.stop; game->m_nTimeStopCaster=row.stop==1?p->m_id+1:p->m_id;
            p->AIUpdate(); check("native-ground-time-stop-byte-countdown",p->m_age==row.age && p->m_frameCount==row.after
                && p->m_cell.m_nCurrentFrame==(row.end?3:0)+row.frame && p->retired==row.retired); retire(p);
        }
        game->m_nTimeStop=0;
        auto& search=area->m_search; auto* dynamic=search.m_pDynamicCost; CSize dimensions=search.m_GridSquareDimensions;
        std::vector<BYTE> costs(128*128,0x80); search.m_pDynamicCost=costs.data(); search.m_GridSquareDimensions=CSize(128,128);
        for (const auto& row : GROUND_COLLISIONS) {
            auto* p=new Sample(explosion,empty,emptyRanged,area); p->m_collision=row.collision; p->m_arcLen1=1; p->m_arcLen2=0;
            p->m_terrainTable[0]=row.wall?255:0; auto* cell=static_cast<LONG*>(p->m_fanCells); auto* flags=static_cast<BYTE*>(p->m_fanVel);
            cell[0]=-1025; cell[1]=-1367; cell[2]=row.velocity[1]; cell[3]=row.velocity[0]; memset(flags,2,4); flags[0]=row.flag;
            p->AIUpdate(); check("native-ground-wall-axis-rebound",memcmp(cell,row.cell,16)==0 && flags[0]==row.afterFlag && p->retired==row.retired); retire(p);
        }
        for (const auto& row : GROUND_SPAWNS) {
            char stationary[34]={},moving[34]={}; strcpy(stationary+1,"FirebaR"); strcpy(moving+1,"RNG_M03");
            IcewindCSpellHitEmission e1; IcewindCSpellHitEmissionRanged e2;
            if (row.moving) { e1.m_resref0=moving+1; e1.m_resref0Len=7; e1.m_resref0Cap=31; }
            e2.m_resref0=stationary+1; e2.m_resref0Len=7; e2.m_resref0Cap=31;
            e2.m_cellPool=new IcewindCSpellHitCellPool; e2.m_cellPool->m_refCount=1; // fixture retains pool across child destruction
            e2.m_densityBase=row.density; auto* p=new Sample(explosion,e1,e2,area); unpend(p);
            p->m_frameCount=3; p->m_arcLen1=1; p->m_arcLen2=0; p->m_age=row.age; p->m_emitCooldown=row.cooldown; p->m_terrainTable[0]=0;
            auto* cell=static_cast<LONG*>(p->m_fanCells); memset(cell,0,64); cell[2]=-3; cell[3]=-1;
            auto* flags=static_cast<BYTE*>(p->m_fanVel); memset(flags,2,4); flags[0]=0;
            std::vector<LONG> before; POSITION at=area->m_lVertSortAdd.GetHeadPosition(); while (at) before.push_back(reinterpret_cast<LONG>(area->m_lVertSortAdd.GetNext(at)));
            srand(1); p->AIUpdate();
            check("native-ground-age-density-cooldown",p->m_age==row.afterAge && p->m_emitCooldown==row.afterCooldown && p->m_movingSpawnCount==row.movingCount);
            const auto& cells=e2.m_cellPool->m_cells; check("native-ground-10-byte-cell-pool",row.poolFlag<0?cells.empty():
                (cells.size()==1 && cells[0].x==163839 && cells[0].y==98302 && cells[0].flag==row.poolFlag));
            std::vector<CGameObject*> children; at=area->m_lVertSortAdd.GetHeadPosition();
            while (at) { LONG id=reinterpret_cast<LONG>(area->m_lVertSortAdd.GetNext(at)); if (std::find(before.begin(),before.end(),id)!=before.end()) continue;
                CGameObject* object=NULL; if (objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS) {
                    children.push_back(object); objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE); }
            }
            check("native-ground-emission-child-count",children.size()==size_t(INT(row.stationary)+INT(row.moving)));
            for (auto* child : children) { auto* particle=dynamic_cast<IcewindCSpellHitParticle*>(child); ++particles;
                check("native-ground-particle-launch",particle && particle->m_posExact==CPoint(163839,98302)
                    && particle->m_posDelta==(particle->m_cellPool?CPoint(0,0):CPoint(-1,-2))
                    && particle->m_duration==(particle->m_cellPool?24:2) && particle->m_collision==0);
                retire(child);
            }
            retire(p); check("ground-pool-reference-balanced",e2.m_cellPool->m_refCount==1); delete e2.m_cellPool;
            check("ground-emission-names-restored",stationary[0]==0 && moving[0]==0);
        }
        // Detonate the real carrier, destroy it immediately, and drive every
        // resulting visual/particle through its own registration and retirement.
        // This catches the formerly borrowed emission strings after parent death.
        struct Detonation : CProjectileFireball {
            void Prepare(CGameArea* a) {
                m_pArea=a; CGameObject::m_pArea=a; m_pos=CPoint(232,1002);
                m_arrivalSoundRef=CResRef(""); m_visual1.m_soundResRef.Release(); m_visual3.m_soundResRef.Release();
            }
            IcewindCSpellHitCellPool* Pool() { return reinterpret_cast<IcewindCSpellHitCellPool*>(m_visual3CellPool); }
        };
        std::vector<LONG> baseline; POSITION pending=area->m_lVertSortAdd.GetHeadPosition();
        while (pending) baseline.push_back(reinterpret_cast<LONG>(area->m_lVertSortAdd.GetNext(pending)));
        auto* detonation=new Detonation; detonation->Prepare(area); detonation->OnArrival();
        auto* pool=detonation->Pool(); check("ground-real-detonation-pool",pool!=NULL);
        if (pool) ++pool->m_refCount;
        detonation->CGameObject::m_pArea=NULL; delete detonation;
        std::vector<LONG> live; INT ticks=0,total=0; BOOL intact=TRUE;
        auto collect=[&]() {
            POSITION pos=area->m_lVertSortAdd.GetHeadPosition();
            while (pos) { POSITION node=pos; LONG id=reinterpret_cast<LONG>(area->m_lVertSortAdd.GetNext(pos));
                if (std::find(baseline.begin(),baseline.end(),id)==baseline.end()) {
                    area->m_lVertSortAdd.RemoveAt(node); if (std::find(live.begin(),live.end(),id)==live.end()) { live.push_back(id); ++total; }
                }
            }
        };
        collect(); check("ground-real-detonation-one-visual",live.size()==1);
        srand(1);
        while (!live.empty() && ticks++<512) {
            std::vector<LONG> snapshot=live;
            for (LONG id : snapshot) {
                CGameObject* object=NULL;
                if (objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) continue;
                auto* visual=dynamic_cast<IcewindCSpellHitVisual*>(object);
                if (visual) { intact=intact && strcmp(visual->m_emission1.m_resref0,"FirebaR")==0 && strcmp(visual->m_emission2.m_resref0,"FirebaA")==0;
                    visual->m_terrainTable[0]=0; }
                auto* particle=dynamic_cast<IcewindCSpellHitParticle*>(object); if (particle) particle->m_terrainTable[0]=0;
                objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE); object->AIUpdate();
            }
            collect();
            live.erase(std::remove_if(live.begin(),live.end(),[&](LONG id) {
                CGameObject* object=NULL; if (objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)!=CGameObjectArray::SUCCESS) return true;
                objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE); return false;
            }),live.end());
        }
        check("ground-parent-independent-full-lifecycle",intact && live.empty() && total>20 && ticks<512);
        Emit("{\"step\":%d,\"op\":\"ground-hit-lifecycle\",\"ticks\":%d,\"objects\":%d,\"remaining\":%d}",nStep,ticks,total,INT(live.size()));
        for (LONG id : live) { CGameObject* object=NULL; if (objects->GetShare(id,CGameObjectArray::THREAD_ASYNCH,&object,INFINITE)==CGameObjectArray::SUCCESS) {
            objects->ReleaseShare(id,CGameObjectArray::THREAD_ASYNCH,INFINITE); retire(object); }
        }
        if (pool) { check("ground-full-lifecycle-pool-balanced",pool->m_refCount==1); delete pool; }
        search.m_pDynamicCost=dynamic; search.m_GridSquareDimensions=dimensions;
        auto& infinity=area->m_cInfinity; CRect view=infinity.rViewPort; INT viewX=infinity.nCurrentX,viewY=infinity.nCurrentY;
        auto* wed=infinity.pResWED; auto areaY=infinity.nAreaY,areaX=infinity.nAreaX;
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
        BOOL backed=pixels(&savedPixels,NULL,FALSE); check("ground-pixels-backed-up",backed);
        infinity.pResWED=NULL; infinity.rViewPort=CRect(80,80,400,320); infinity.nCurrentX=72; infinity.nCurrentY=882; infinity.nAreaY=10000;
        CResWED occluder; struct ScreenPolyData { WED_WEDHEADER header; WORD index; } screenPolys={};
        WED_POLYHEADER polygonHeader={}; polygonHeader.nOffsetToScreenPolyList=offsetof(ScreenPolyData,index);
        WED_POLYLIST polygon={}; polygon.nNumPoints=4; polygon.nType=8; polygon.nLeft=232; polygon.nRight=400; polygon.nTop=950; polygon.nBottom=1060;
        WED_POLYPOINT vertices[]={{232,950},{400,950},{400,1060},{232,1060}};
        WED_SCREENSECTIONLIST sections[]={{0,1},{0,1},{0,1}};
        occluder.m_bParsed=TRUE; occluder.m_pHeader=&screenPolys.header; occluder.m_pPolyHeader=&polygonHeader;
        occluder.m_pScreenSectionList=sections; occluder.m_pPolyList=&polygon; occluder.m_pPolyPoints=vertices;
        memset(area->m_visibility.m_pMap,CVisibilityMap::VISIBILITY_MASK,visibility.size());
        seq.nFrames=2;
        std::vector<BYTE> unobstructed;
        for (BOOL occluded : {FALSE,TRUE}) for (LONG z : {-17L,0L,17L}) for (DWORD flags : {0UL,0x20000UL}) for (BYTE loaded : {BYTE(0),BYTE(1),BYTE(2)}) for (BOOL hidden : {FALSE,TRUE}) {
            infinity.pResWED=occluded?&occluder:NULL; infinity.nAreaX=640; infinity.nAreaY=1440;
            fixture->m_pos=CPoint(232,1002); fixture->m_posZ=z; fixture->m_listType=CGameObject::LIST_FLIGHT;
            fixture->m_cell.m_nCurrentFrame=0; fixture->m_visualEffect.m_dwFlags=flags; fixture->m_bamLoaded=loaded;
            SHORT tile=area->m_visibility.PointToTile(fixture->m_pos); if (hidden) area->m_visibility.m_pMap[tile]=0;
            std::vector<BYTE> expected,actual; BOOL ok=backed && pixels(NULL,NULL,TRUE);
            if (!hidden && loaded) { CPoint center; CSize size; fixture->m_cell.GetCurrentCenterPoint(center,FALSE); fixture->m_cell.GetCurrentFrameSize(size,FALSE);
                CRect fx(0,0,size.cx,size.cy); CPoint point(232,1002); DWORD f=flags|CInfinity::FXPREP_COPYFROMBACK;
                infinity.FXPrep(fx,f,CVIDINF_SURFACE_BACK,point,center); ok=infinity.FXLock(fx,f);
                if (ok) { infinity.FXRender(&fixture->m_cell,center.x,center.y,flags,0);
                    CRect bounds(point.x-center.x,point.y-center.y,point.x-center.x+size.cx,point.y-center.y+size.cy);
                    infinity.FXRenderClippingPolys(point.x,point.y-z,z,center,bounds,FALSE,flags);
                    infinity.FXUnlock(f,NULL,CPoint(0,0));
                    ok=infinity.FXBltFrom(CVIDINF_SURFACE_BACK,fx,point.x,point.y,center.x,center.y,flags|1); }
            }
            ok=pixels(&expected,NULL,TRUE) && ok; fixture->Render(NULL,video,CVIDINF_SURFACE_BACK); ++draws; ok=pixels(&actual,NULL,FALSE) && ok;
            check("native-ground-render-full-pixels",ok && actual==expected);
            if (!hidden && loaded==1 && flags==0 && z==0) {
                if (!occluded) unobstructed=actual;
                else check("ground-obstacle-really-clips",actual!=unobstructed);
            }
            if (!hidden && loaded==1 && flags==0) check("ground-really-renders",std::any_of(actual.begin(),actual.end(),[](BYTE value){return value!=0;}));
            area->m_visibility.m_pMap[tile]=CVisibilityMap::VISIBILITY_MASK;
        }
        resource->m_pSequences=original; if (header) header->m_pSequences=originalHeader;
        memcpy(area->m_visibility.m_pMap,visibility.data(),visibility.size()); infinity.pResWED=wed; infinity.rViewPort=view;
        infinity.nCurrentX=viewX; infinity.nCurrentY=viewY; infinity.nAreaY=areaY; infinity.nAreaX=areaX;
        check("ground-pixels-restored",backed && pixels(NULL,&savedPixels,FALSE)); check("ground-resource-restored",anchor.Restore());
    }
    retire(fixture); game->m_nTimeStop=stop; game->m_nTimeStopCaster=caster; srand(1);
    check("ground-fixtures-restored",clock==game->GetWorldTimer()->m_gameTime && handler->m_messageList.GetCount()==messages && area->m_lVertSortAdd.GetCount()==front);
    Emit("{\"step\":%d,\"op\":\"groundhitchecks\",\"checks\":%d,\"renderCalls\":%d,\"particles\":%d,\"nativeCases\":465,\"restored\":true,\"valid\":%s}",nStep,checks,draws,particles,valid?"true":"false");
    if (!valid) Fail(nStep,"groundhitchecks","ground impact native state/render mismatch");
}
