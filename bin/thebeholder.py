# /undead_warlock
# GPL3.0-or-foward

from win32security import LookupPrivilegeValue, SE_DEBUG_NAME
from ctypes import wintypes
from setup_apis import *
from ctypes import *
import atexit 
import subprocess
import time
from subprocess import CREATE_NO_WINDOW
import optparse
import re 
import os 


def winERROR(msg, malware_alert = None):
    if malware_alert:
       msg = "Ransomware pid (" + malware_alert + ") no sistema"
       windll.user32.MessageBoxA(0, msg.encode(), b"Ransomware", 0)
       return 

       windll.user32.MessageBoxA(0, msg.encode(), b"Error", 0)


def arguments():
    parser = optparse.OptionParser() 

    parser.add_option("-l", "--live", dest= "live", action= "store_true", default=False, help= "-r for real time process monitoring")
    parser.add_option("-p", "--pid", dest= "processid", default=False, type= "int", help= "pid to scan, only use if you want to scan only one process")
    (inputs, args) = parser.parse_args() 

    if inputs.live and inputs.processid:
        parser.error("[x] Please use only one scan method")

    if not inputs.live and not inputs.processid:
        parser.error("[x] Please choose a scan method")

    return (inputs.live, inputs.processid)

def setDebugPriv():    # Getting Debug Privileges for Memory Reading

   token_handle = wintypes.HANDLE()

   if not OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,byref(token_handle)):
      print("Error:",kernel32.GetLastError())
      return False    


   luidvalue = LookupPrivilegeValue ( None, SE_DEBUG_NAME )
   if not luidvalue:
      return False

   se_debug_name_value = LUID(luidvalue)   
   LAA                 = LUID_AND_ATTRIBUTES (se_debug_name_value,SE_PRIVILEGE_ENABLED)
 
   tkp = TOKEN_PRIVILEGES (1, LAA)


   if not AdjustTokenPrivileges(token_handle, False, byref(tkp), sizeof(tkp), None, None):
       print("Error:",GetLastError)
       CloseHandle(token_handle)       
       return False

   return True        

def checkString(stringlist, boolist, buffer):
    for x in stringlist:
        if x in buffer:
            index = stringlist.index(x)
            boolist[index] = 1
    if boolist != [1,1,1,1,1]:
        boolist = [0,0,0,0,0]
        return False 
    return True 

def killProcess(hProcess):
    exitCode = wintypes.DWORD()
    status = GetExitCodeProcess(hProcess, byref(exitCode))
    if not status:
        return False 
    status = TerminateProcess(hProcess, exitCode)
    if not status:
        return False 
    return True 

def memscanner(pid):          
    ransom1 = [b"vssadmin", b"delete", b"shadows", b"all", b"quiet"]
    ransom2 = [b"bcdedit", b"set", b"default", b"recoveryenabled", b"no"]
    ransom3 = [b"bcdedit", b"set", b"default", b"bootstatuspolicy"]
    '''extensions = [b".txt", b".exe", b".zip", b".pdf", b".doc", b".docx", b".pptx", b".ppt", b".pps", b".xls", b".xlsx", b".jpg", b".png", b".gif", b".html", b".mp3",
    b".mp4", b".avi", b".mpg", b".wmv", b".mov", b".css", b".js", b".php", b".mpeg", b".jpeg", b".rar", b".7z", b".rtf", b".sql"]'''

    ransomcheck1 = [0,0,0,0,0]
    ransomcheck2 = [0,0,0,0,0]
    ransomcheck3 = [0,0,0,0,0]
    # extensionscheck = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0] # should put around 100 extensions if u want to detect ransomwares that use whitelist

    if pid == os.getpid():
        return False

    setDebugPriv()  

    process = OpenProcess (PROCESS_TERMINATE|PROCESS_VM_READ|PROCESS_QUERY_INFORMATION, False, pid)                        
    if not process: 
      return False
          
   
    system_info = SYSTEM_INFO()
    GetSystemInfo ( byref(system_info) )        
    MaxAppAdress = system_info.lpMaximumApplicationAdress  
    
    VirtualQueryEx = VirtualQueryEx64
    mbi            = MEMORY_BASIC_INFORMATION64()

    memset (byref(mbi), 0, sizeof(mbi))
    Adress      = 0
    BytesRead   = c_size_t (0)
    old_protect = wintypes.DWORD()
 
   
    while MaxAppAdress > Adress:           

        if not VirtualQueryEx(process, Adress, byref(mbi), sizeof(mbi)):
               return False
        
        if mbi.State == MEM_COMMIT:   
                 if mbi.Protect == 0x10:
                    try:
                       VirtualProtectEx(process, Adress, mbi.RegionSize, 0x20, byref(old_protect) )  
                    except:
                        pass          
                 try:
                     ContentsBuffer = create_string_buffer(mbi.RegionSize)
                     
                 except:
                     pass


                 if not ReadProcessMemory(process, Adress, ContentsBuffer, mbi.RegionSize, byref(BytesRead)):              
                    Adress += mbi.RegionSize          
                    continue

                 else:
                    if checkString(ransom1, ransomcheck1, ContentsBuffer.raw):
                        killProcess(process)
                        return checkString 
                    if checkString(ransom2, ransomcheck2, ContentsBuffer.raw):
                        killProcess(process)
                        return checkString 
                    if checkString(ransom3, ransomcheck3, ContentsBuffer.raw):
                        killProcess(process)
                        return checkString 
                    ''' for extension in extensions:
                        if extension in ContentsBuffer.raw:
                            index = extensions.index(extension)
                            extensionscheck[index] = 1
                    if extensionscheck.count(1) < 20:
                          extensionscheck = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]

                    if extensionscheck.count(1) >= 20: 
                        # killProcess(process)
                        print(extensionscheck)
                        print("cuzin")
                        return True '''


        Adress += mbi.RegionSize
        # if malware: killProcess(handle)         

    return False

def cleanAfterExit(hprocess):
    hprocess.kill()

def Endianess(string):
        endian = str()
        while len(endian) != len(string):
            endian += string[len(string)-1 - len(endian)]

        return endian

def main():
    realtime, userpid = arguments()

    try:
            watcher = subprocess.Popen("watcher.exe", creationflags= CREATE_NO_WINDOW)
    except:
            winERROR("Nao foi possível encontrar o arquivo watcher na mesma pasta")
            quit()   
    atexit.register(cleanAfterExit, hprocess = watcher)


    if userpid:
        if not memscanner(userpid):
            print("\n[!] String not found in Process Virtual Memory")
        else: 
            winERROR("Oi", str(userpid))



    if realtime:
        index = 0 
        line  = 0
        time.sleep(3)
        while True:
            with open("WATCHER", "r") as file: 
                content = file.readlines()
            try:
                regex = re.findall(r"(([0-9]*),)", content[line])
                pids  = [int(processid[1]) for processid in regex]
            except:
                continue

            try: 
                pid = pids[index]
                print(pid)
                index += 1 
                if index > 9:
                   index = 0
                   line += 1
                if memscanner(pid):
                   with open("RESULT", "a") as file:
                      file.write(f"{pid} is Malware\n")
                   winERROR("oi", str(pid))

            except: 
                continue 

    return 1 


if __name__ == "__main__":

   main()


        

        

       
          


    




        
        
                

        

                  
                
                

                

                







            
















    







  

 














  



