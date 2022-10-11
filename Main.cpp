#include <stdio.h>
#include "stdafx.h"
#include "lua.hpp"
#include "DirectCompositeSample.h"
//#include "LuaGlobals.cpp"

HINSTANCE hInstanceSvd;
int nCmdShowSvd;
HWND conWND;

int (*LuaCFuncs) (lua_State*) = {};

namespace LuaGlobals {
    static int l_wcout(lua_State* L) {
        //int strlen;
        const char* input = luaL_checkstring(L, 1);
        //int strlen = luaL_checkint(L, 2);
        size_t strlen = lua_objlen(L, 1);
        WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), input, strlen, NULL, NULL);
        return 0;
    }
    static int l_newwnd(lua_State* L) {
        DirectCompositeSample sample(512, 512, L"Lua Overlay");
        int wnderr = Win32Application::Run(&sample, hInstanceSvd, nCmdShowSvd);
        return 0;
    }
    static int l_quit(lua_State* L) {
        FreeConsole();
        return 0;
    }
}

bool CheckLua(lua_State* L, int r) {
    if (r != 0) {
        std::string errmsg = "LUA ERROR: ";
        errmsg += lua_tostring(L, -1);
        errmsg += "\n";
        WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), (errmsg).c_str(), errmsg.length(), NULL, NULL);
        return false;
    }
    return true;
}


_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    hInstanceSvd = hInstance;
    nCmdShowSvd = nCmdShow;
    // Console
    AllocConsole();
    conWND = FindWindow(L"ConsoleWindowClass", NULL);
    ShowWindow(conWND, nCmdShow); //0 - hide, 1 - show
    // Create VM state
    lua_State* L = luaL_newstate();
    // run config
    //WCHAR path[MAX_PATH];
    //GetModuleFileName(NULL, path, MAX_PATH);
    if (!L)
        return 1;
    luaL_openlibs(L); // Open standard libraries
    lua_pushcfunction(L, LuaGlobals::l_wcout);
    lua_setglobal(L, "ConsoleWrite");
    lua_pushcfunction(L, LuaGlobals::l_quit);
    lua_setglobal(L, "ProgramClose");
    int Rcfg = luaL_dofile(L, "cfg.lua");
    if (!CheckLua(L, Rcfg)) {
        std::string errmsg = "\nPlease check if cfg.lua is near overlay's executable and restart application!\n";
        WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), (errmsg).c_str(), errmsg.length(), NULL, NULL);
        system("pause");
        return 1;
    }
    else {
        while (GetConsoleWindow());
        lua_close(L); //close lua state
        return 0;
    }
}
