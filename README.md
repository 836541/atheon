# atheon
Anti-Ransomware Toolkit.

https://www.youtube.com/watch?v=qklVNKxWNHw (COMERCIAL)

--------------------------------------------

0 - Integrantes

------------------------------------------------------

Leonardo, Tarek, Thiago, Matheus Santos e João. 1TDCA.

-------------------------------------------------------

1- Ferramentas

---------------------------------------------------------

Todas ferramentas, salvo alguns ajustes, estão da mesma maneira que foram usadas na quarta mentoria. Os upgrades estão sendo feitos em OFF.
Por exemplo, as ferramentas de kernel-mode, em off, estão tendo seu unload desativado para que um atacante não consiga dar bypass através de unloads no filtermanager.
Há uma ferramenta que foi apresentada na terceira mentoria, o "shadowcloack", o qual trocava as extensões dos arquivos do sistema sem alterar a funcionalidade dos filetypes, de modo que Ransomwares que atacam através de whitelist de extensões não funcionassem, mas essa ferramenta foi descontinuada depois de críticas obtidas na apresentação a qual foi mostrada.

A- Beholder: User-Mode Memory Scanner. É uma ferramenta de detecção via análise dinâmica, leva de 1-12s para encontrar o Ransomware caso o mesma possua strings de deleção do backup shadow do windows em seu código. Contudo, por ser user-mode é extremamente vulnerável, então no final é apenas uma ferramenta extra do nosso grupo.

B- Behelit: Kernel-Mode Honeypot. É uma ferramenta de detecção via análise comportamental. Possui uma série de melhorias em relação aos honeypots tradicionais, como: kernel-mode para evitar bypasses a nível de usuário, decoys com prioridade de aparecimento na listagem de arquivos (win apis vão do A ao Z ou vice-versa), decoys que não podem ser movidos, deletados, renomeados etc... tudo para que seja evitado falso-positivos e intervenções do usuário. Espalhando apenas 6 pastas decoys no filesystem resultou na detecção de ransomwares em uma média de 2-3s. 

C- Sacerdote: Kernel-mode Local Backup. É uma ferramenta que realiza backup numa pasta local (protegida de qualquer acesso que não seja Kernel-Mode). Toda operação de Read é interceptada e, somente quando o backup é concluido, a operação é concluida, de modo que seja impossível uma criptografia ocorrer antes do backup. Por enquanto o backup é feito com flag de CREATE, o que faz com que não haja sincronização de backup.

-------------------------------------------------------------------------------------

2- Melhorias Futuras para as atuais ferramentas

-------------------------------------------------------------------------------------

A- Bloquear o Unload das ferramentas de kernel-mode para evitar bypass via comandos como "fltmc unload" ou apis como "FltUnregisterFilter". Dessa forma, o unloading vai começar a ser feito através de clients user-mode.

B- Melhoria da qualidade dos decoys: nomes diferentes um dos outros e filetypes diferentes. Além da diminuição de quantos decoys podem ser alterados antes que o processo seja considerado suspeito.

C- (Difícil, mas tentaremos): fazer algum sistema que simule o Group Id do Linux, para que seja possível matar a árvore de processos do Ransomware mesmo que no meio desta haja processos já mortos.

D- O Backup até agora só é feito para arquivos que estão em uma lista de tipos de arquivo que o user quer proteger. Iremos aumentar essa lista de 12 para 100+ ou simplesmente a substituir por uma lista de filetypes que o user NÃO quer proteger.

E- Atualização dos arquivos backupados (atualmente se já há um arquivo com o mesmo nome, não é feito o backup nem atualização), feito caso a última modificação que o arquivo sofreu já passou de 1 minuto.

F- Código menos Hardcoded: atualmente o código possui alguns hardcodes, por exemplo o backup do Sacerdote ser feito somente na pasta C:\sacerdotebackupdir.

G- Integração com Interface Gráfica.

----------------------------------------------------

3- Instalação e Uso das Tools

---------------------------------------------------

A- Beholder: cmd -> "beholder.exe -l" 

B- Setup básico para as ferramentas Kernel-mode: desativar a proteção do Windows contra drivers sem assinatura via comando "bcdedit.exe /set testsigning on", o que exige também reiniciar a máquina; instalação do aplicativo DbgView64 (abrir como admin -> Capture -> Capture Kernel | Kernel Verbose Output para visualização dos logs, pois não há interface integrada ainda); botão direito em cada arquivo .inf -> install; "fltmc load behelit" e "fltmc load Sacerdote". Para desativar depois basta dar o comando anterior, mas com "unload" no lugar de "load" (essa feature será substituida por um modo menos fácil de dar bypass futuramente).

C- Setups específicos após realização do passo B -> Para o behelit, crie uma pasta com as substrings zzbehelit ou aabehelit no nome. Adicione 5 arquivos decoys com a substring "aabehelit" e "zzbehelit" (de preferência .pdf, .docx, .pptx ...) em cada pasta e as espalhe pelo filesystem (minimamente pelo dir do root, de cada user, downloads, documentos e desktop). Para o Sacerdote, apenas crie a pasta "C:\sacerdotebackupdir\".




