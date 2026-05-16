#!/bin/bash
# Usage: ./scripts/ghidra_info.sh 0x405090
ADDR=$1
curl -s -X POST http://127.0.0.1:8081/query --data "
SELECT 'FUNC' as kind, name || ' (' || printf('0x%X', address) || ' size=' || CAST(size AS TEXT) as info FROM funcs WHERE address = $ADDR
UNION ALL
SELECT 'BOOKMARK', type || ' | ' || category || ' | ' || comment FROM bookmarks WHERE address = $ADDR
UNION ALL
SELECT 'COMMENT', comment FROM comments WHERE address = $ADDR;
" --max-time 10 2>/dev/null | python -c "
import sys, json
d = json.load(sys.stdin)
for r in d['results'][0]['rows']:
    print(f'[{r[0]:8s}] {r[1]}')
" 2>/dev/null
