# Restore external data from compressed archives

# PDB extracted text files (used for BG2EE field/function name lookups)
# Source: C:\projects\bg2-symbols\baldur.pdb → llvm-pdbutil dump
Expand-Archive -LiteralPath "data\pdb\bg2_pdb_extracted.zip" -DestinationPath "C:\projects\bg2-symbols" -Force

# IWD2 Ghidra project repository (source of truth for all function names)
# Place in C:\ghidra_projects\IWD2\IWD2.rep\
Expand-Archive -LiteralPath "data\ghidra\IWD2_rep.zip" -DestinationPath "C:\ghidra_projects\IWD2" -Force

# BG2EE Ghidra project repository (cross-reference for field/function names)
# Place in C:\ghidra_projects\BG2EE\BG2EE.rep\
Expand-Archive -LiteralPath "data\ghidra\BG2EE_rep.zip" -DestinationPath "C:\ghidra_projects\BG2EE" -Force

Write-Host "All data restored. Submodules (refs/) must be initialized with:"
Write-Host "  git submodule update --init --recursive"
