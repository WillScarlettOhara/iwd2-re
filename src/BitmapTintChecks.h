// AutoUI-only 24-bit tinted bitmap blits on private buffers; no resource or surface is touched.

void BitmapTintChecks(INT nStep)
{
    CVidMode* mode=g_pChitin->GetCurrentVideoMode();
    if (mode==NULL) { Fail(nStep,"bitmaptintchecks","requires video mode"); return; }
    INT checks=0,index=-1; BOOL valid=TRUE;
    auto check=[&](const char* name,BOOL ok) { ++checks; valid=valid && ok;
        if (!ok) Emit("{\"step\":%d,\"op\":\"bitmap-tint-case\",\"row\":%d,\"case\":\"%s\",\"passed\":false}",nStep,index,name); };
    BYTE brightness=mode->m_nBrightnessCorrection,gamma=mode->m_nGammaCorrection;
    mode->m_nBrightnessCorrection=0; mode->m_nGammaCorrection=0;
    DWORD rs=static_cast<BYTE>(mode->m_dwRBitShift),gs=static_cast<BYTE>(mode->m_dwGBitShift),bs=static_cast<BYTE>(mode->m_dwBBitShift);
    auto pack=[&](DWORD r,DWORD g,DWORD b) { return (r<<rs)|(g<<gs)|(b<<bs); };
    CVidBitmap bitmap; CSize one(1,1);
    auto blit=[&](BYTE r,BYTE g,BYTE b,DWORD flags,DWORD surface) {
        BYTE data[3]={b,g,r}; DWORD pixel=surface;
        bitmap.BltBmp24To32Tint(&pixel,0,data,one,0,flags); return pixel; };
    auto reset=[&]() { memset(&bitmap.m_paletteAffects,0,sizeof(bitmap.m_paletteAffects)); };

    ++index; reset(); bitmap.m_paletteAffects.rgbTintColor=RGB(128,64,255);
    check("plain-tint-scales-channels",blit(200,100,50,0x20000,0)==pack(100,25,49));
    ++index; reset(); bitmap.m_paletteAffects.rgbLightColor=RGB(16,0,0);
    check("plain-light-overflow-keeps-high-bits",blit(200,100,50,0x200000,0)==pack(0x2FF,100,50));
    ++index;
    check("keyed-light-clamps-and-sets-alpha",blit(200,100,50,0x200001,0)==(pack(255,100,50)|0xFF000000));
    ++index;
    check("keyed-transparent-key-untouched",blit(0,255,0,0x200001,0x12345678)==0x12345678);
    ++index; reset();
    COLORREF bc=mode->ApplyBrightnessContrast(RGB(0,0,0)); DWORD under=pack(200,100,50);
    check("keyed-shadow-blends-with-brightness-contrast",blit(0,0,0,0x5,under)
        ==(pack((200+GetRValue(bc))>>1,(100+GetGValue(bc))>>1,(50+GetBValue(bc))>>1)|0x80000000));
    ++index; reset(); bitmap.m_paletteAffects.rgbAddColor=RGB(0,0,0);
    check("plain-add-inverts-low-byte",blit(200,100,50,0x40000,0)==pack(201,((155*255)>>8)^0xFF,((205*255)>>8)^0xFF));
    ++index; reset(); COLORREF range=RGB(0,255,255);
    bitmap.m_paletteAffects.pRangeTints[0]=&range; bitmap.m_paletteAffects.aRangeTintPeriods[0]=1;
    check("plain-range-tint-without-base-tint-overflows",blit(200,100,50,0,0)==pack(0,(100*255*255)>>8,(50*255*255)>>8));
    reset();

    mode->m_nBrightnessCorrection=brightness; mode->m_nGammaCorrection=gamma;
    Emit("{\"step\":%d,\"op\":\"bitmaptintchecks\",\"checks\":%d,\"passed\":%s}",nStep,checks,valid?"true":"false");
    if (!valid) Fail(nStep,"bitmaptintchecks","24-bit tinted bitmap blit mismatch");
}
