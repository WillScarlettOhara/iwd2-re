# Ghidra rename / annotate workflow

Ghidra DB is the source of truth for function names, signatures, locals, params, comments, bookmarks, and tags.
Use `scripts/ghidra_re.py` against the running GhidraSQL HTTP server (`http://127.0.0.1:8081`).

## Start / health

```powershell
C:\ghidra_projects\IWD2\start_ghidrasql.bat
curl http://127.0.0.1:8081/health
```

The local server exposes `/query`; the current `ghidrasql --url ...` CLI may fail against it with `/rpc` errors.

## Inspect before mutating

```powershell
python scripts/ghidra_re.py decomp 0x5D2DE0
python scripts/ghidra_re.py locals 0x5D2DE0
python scripts/ghidra_re.py params 0x5D2DE0
```

## Rename function

```powershell
python scripts/ghidra_re.py func 0x5D2DE0 RenderFogOfWar
```

With signature:

```powershell
python scripts/ghidra_re.py func 0x5D2DE0 RenderFogOfWar --signature "void RenderFogOfWar(CVidMode* pVidMode)"
```

The tool verifies the row and runs `SELECT save_database();` unless `--no-save` is used.

## Rename/retype params and locals

```powershell
python scripts/ghidra_re.py param 0x5D2DE0 0 --name pVidMode --type "CVidMode *"
python scripts/ghidra_re.py local 0x5D2DE0 local_8 --name pArea --type "CGameArea *"
```

Locals use GhidraSQL helpers:

- `rename_local(func_addr, local_id, new_name)`
- `set_local_type(func_addr, local_id, new_type)`

This avoids the array-local rename failure path in direct `UPDATE decomp_lvars`.

## Comments / bookmarks / tags

```powershell
python scripts/ghidra_re.py comment 0x5D2DE0 plate "Renders fog of war overlay." --replace
python scripts/ghidra_re.py decomp-comment 0x5D2DE0 0x5D2E40 pre "alpha blend setup" --replace
python scripts/ghidra_re.py bookmark 0x5D2DE0 review "Check blend flags."
python scripts/ghidra_re.py tag 0x5D2DE0 reviewed --comment "manual RE pass complete"
```

Comment kinds: `plate`, `pre`, `post`, `eol`, `repeatable`.

## Import source TODO/FIXME as Ghidra bookmarks

```powershell
python scripts/ghidra_re.py source-notes --replace
```

This creates bookmarks with:

- category `TODO` or `FIXME`
- type `Note`
- comment prefix `[src] file:line: ...`

Current import: 716 source bookmarks (415 TODO, 301 FIXME).

## Batch file

JSON array or JSONL:

```json
[
  {"op":"func", "address":"0x5D2DE0", "name":"RenderFogOfWar"},
  {"op":"local", "func_addr":"0x5D2DE0", "local_id":"local_8", "name":"pArea", "type":"CGameArea *"},
  {"op":"comment", "address":"0x5D2DE0", "kind":"plate", "text":"Fog overlay renderer", "replace":true},
  {"op":"bookmark", "address":"0x5D2DE0", "category":"review", "text":"Compare against BG2 renderer"}
]
```

Apply:

```powershell
python scripts/ghidra_re.py batch my_annotations.json
```

## Source sync rule

After function rename in Ghidra:

```powershell
rg "sub_5D2DE0|FUN_005D2DE0|OldName" src/
```

Then update declaration, definition, and callsites atomically. Build before commit.

Fields are not stored in Ghidra DB; field names in C++ headers remain source-of-truth, and must be renamed class-scoped only.
