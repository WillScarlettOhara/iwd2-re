#!/bin/bash
# Save Ghidra DB + backup zip + commit
set -e

echo "Saving Ghidra database..."
curl -s -X POST http://127.0.0.1:8081/query --data "SELECT save_database();" > /dev/null
echo "  DB saved."

echo "Creating backup zip..."
cd "C:/ghidra_projects/IWD2"
C:/Users/wills/AppData/Local/Programs/Python/Python312/python.exe -c "
import zipfile, os
z = zipfile.ZipFile('C:/iwd2-re/data/ghidra/IWD2_rep.zip', 'w', zipfile.ZIP_DEFLATED)
rep = 'C:/ghidra_projects/IWD2/IWD2.rep'
for root, dirs, files in os.walk(rep):
    for f in files:
        if f.endswith('.lock') or f.endswith('.lock~') or f.endswith('.gpr') or f.endswith('.bak'):
            continue
        full = os.path.join(root, f)
        arc = os.path.relpath(full, 'C:/ghidra_projects/IWD2')
        z.write(full, arc)
z.close()
print(f'  Zip done: {os.path.getsize(\"C:/iwd2-re/data/ghidra/IWD2_rep.zip\")} bytes')
"
echo "Done."
