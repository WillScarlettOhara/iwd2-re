@echo off
set JAVA_HOME=C:\Program Files\Eclipse Adoptium\jdk-21.0.11.10-hotspot
set GHIDRA_INSTALL_DIR=C:\ghidra_dist\ghidra_12.0.4_PUBLIC
"C:\ghidrasql-repo\build\bin\Release\ghidrasql.exe" --ghidra "C:\ghidra_dist\ghidra_12.0.4_PUBLIC" --project "C:\ghidra_projects\IWD2" --project-name IWD2 --program IWD2.exe --initial-program IWD2.exe --no-analyze --keep-host --http --port 8081 --max-runtime 0 > "C:\ghidra_projects\IWD2\ghidrasql.log" 2>&1
