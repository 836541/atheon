1- TOOLS

---------------------------------------------------------

A- Beholder: User-Mode Memory Scanner. A Memory Scanner that detects Windows Shadow Backup Deletion and Ransomware's Extension Whitelist strings in a process' virtual memory. Not very effective because it's full User-Mode. However, a Kernel-Mode version will be developed soon.

B- Behelit: Kernel-Mode Honeypot (a tool which you spread decoy files so when they are encrypted you can detect a possible ransomware). Better than most honeypots because: 1- Kernel Mode (harder to evade and faster), 2- Decoys are immovable objects (can't be renamed, moved, deleted ... so false positives dont happen), 3- Decoys start with "aa" or "zz" substring so win32 apis list them quicker, 4- Entropy/Filetype/Fuzzy Hash/Header etc... are not used as a way to detect if a file is encrypted or not, because its super easy to bypass these type of mechanisms, rather than that the tool uses write/delete irps, which are operations that all crpyto ransomware needs to do.

C- Sacerdote: Kernel-mode Local Backup. Backup files to a kernel-mode protected folder everytime a read operation happens. Safer than most local backup solutions because the malware commonly needs to be a rootkit to access the kernel-mode folder instead of just using lateral movement and getting privileges. Also, backuping files only before read operations also means that the backup doesnt need as large as other solutions.

-------------------------------------------------------------------------------------

2- UPGRADES

-------------------------------------------------------------------------------------

0- Sacerdote doesnt backup PDF in all systems. I already know how to fix it, just need time.

A- Blocking traditional Driver-Unload.

B- Making a Decoy Generator

C- Tracking the Group Id of a Process so the entire tree can be killed.

D- Making communication with the driver so you can choose the extensions you want to backup, the max size etc...

E- Backup Syncro/Updating

F- Code has some hardcodes, like backuping only to "C:\sacerdotebackupdir".


----------------------------------------------------

3- INSTALL/USE (TRANSLATE)

---------------------------------------------------

A- Beholder: cmd -> "beholder.exe -l" 

B- Setup básico para as ferramentas Kernel-mode: desativar a proteção do Windows contra drivers sem assinatura via comando "bcdedit.exe /set testsigning on", o que exige também reiniciar a máquina; instalação do aplicativo DbgView64 (abrir como admin -> Capture -> Capture Kernel | Kernel Verbose Output para visualização dos logs, pois não há interface integrada ainda); botão direito em cada arquivo .inf -> install; "fltmc load behelit" e "fltmc load Sacerdote". Para desativar depois basta dar o comando anterior, mas com "unload" no lugar de "load" (essa feature será substituida por um modo menos fácil de dar bypass futuramente).

C- Setups específicos após realização do passo B -> Para o behelit, crie uma pasta com as substrings zzbehelit ou aabehelit no nome. Adicione 5 arquivos decoys com a substring "aabehelit" e "zzbehelit" (de preferência .pdf, .docx, .pptx ...) em cada pasta e as espalhe pelo filesystem (minimamente pelo dir do root, de cada user, downloads, documentos e desktop). Para o Sacerdote, apenas crie a pasta "C:\sacerdotebackupdir\".




