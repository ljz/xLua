# -*- encoding:utf8 -*-
import json
import os
import sys
import traceback
import types
import shutil
        
def main():
    try:
        oldname = "E:\BladeX_Git_Client/Unity/UnityProject/build/plugin_lua53/Plugins/x86_64/xlua.dll"
        newname = "E:\BladeX_Git_Client/Unity/UnityProject/Assets/Plugins/x86_64/xlua.dll"
        shutil.copyfile(oldname,newname)
        print "copy succ"
    except:
        print traceback.format_exc()
    
    os.system("pause")

if __name__ == '__main__':
    main()