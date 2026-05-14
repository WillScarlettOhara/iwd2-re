# Resource loading / CDimm / CResCache

## GOG IWD2 data layout
- `chitin.key` has 133 BIF entries.
- `AR1000.are` is in `data\ARE.bif` (BIFF V1, `bDrives=0x1`, HD0).
- `AR1000.wed` is in `data\AR1000.bif` (`bDrives=0x8`, CD2). Actual file path in GOG install: `CD2\Data\AR1000.bif`.
- `CD2\Data\AR1000.bif` starts with `BIFCV1.0` and must be decompressed into `hd0:\cache\data\AR1000.bif` before `CResFile` can read it as `BIFFV1  `.

## Bug fixed: BIFC cache setup
- `CDimm::WriteSetUp` originally attempted to create cache path components using a single-character `CString` (`sTemp = sDirName[nPos - 1]`), so missing `cache\data` dirs were not created.
- Result: `CResFile::OpenFile` failed on BIFC area BIFs; WED resources existed in KEY but `CResWED::Demand()` returned `NULL`, triggering `CInfinity.cpp:1075` (`"Demand for WED file failed."`).
- Fix: recursively create full directory prefixes before opening cache output file.

## CResCache::CopyFile notes
- BIFC format: header `BIFCV1.0` + DWORD uncompressed size, then chunks of `<DWORD uncompressedSize, DWORD compressedSize, compressedData>` using zlib.
- Buffers allocated with `new BYTE[]`; must be freed with `delete[]`.
- Compressed-buffer allocation null check must test `pCompressedBuffer`, not `pBuffer`.
