#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <string>
#include <algorithm>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <thread>
#include <atomic>
#include <conio.h>
#include "picosha2.h"

using namespace std;

// --- VARIÁVEIS DO LOADER ---
HANDLE handle = NULL;
bool erro = FALSE;
bool masterToggle = false;

uintptr_t addrNoDelay    = 0x0075CDB3;
uintptr_t addrMouseBoost = 0x00E9ABB0;
uintptr_t addrMayaPlayer = 0x00832E3E;
uintptr_t addrMayaMob    = 0x0082B7C6;

// --- LÓGICA DA SENHA ---
string generateTodayKey() {
    time_t t = time(nullptr);
    tm now;
    localtime_s(&now, &t);

    stringstream date;
    date << setw(2) << setfill('0') << now.tm_mday
         << setw(2) << setfill('0') << (now.tm_mon + 1)
         << (now.tm_year + 1900)
         << "DEVIRUCHI";

    string input = date.str();
    return picosha2::hash256_hex_string(input);
}

bool validatePassword(const string& password) {
    return password == generateTodayKey();
}

// --- FUNÇÕES DO PROCESSO ---
DWORD GetProcId(const char* procName) {
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);
        if (Process32First(hSnap, &procEntry)) {
            do {
                if (!_stricmp(procEntry.szExeFile, procName)) {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return procId;
}

void ApplyAllPatches(bool enable) {
    if (handle == NULL) return;
    if (enable) {
        BYTE nopND[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
        WriteProcessMemory(handle, (LPVOID)addrNoDelay, &nopND, sizeof(nopND), 0);
        DWORD valMB = 0xFFFFFFFF;
        WriteProcessMemory(handle, (LPVOID)addrMouseBoost, &valMB, sizeof(valMB), 0);
        BYTE jne = 0x75;
        WriteProcessMemory(handle, (LPVOID)addrMayaPlayer, &jne, 1, 0);
        WriteProcessMemory(handle, (LPVOID)addrMayaMob, &jne, 1, 0);
        cout << "\a>>> TUDO ATIVADO!" << endl;
    } else {
        BYTE origND[] = { 0xC6, 0x86, 0x9D, 0x00, 0x00, 0x00, 0x00 };
        WriteProcessMemory(handle, (LPVOID)addrNoDelay, &origND, sizeof(origND), 0);
        int origMB = 500;
        WriteProcessMemory(handle, (LPVOID)addrMouseBoost, &origMB, sizeof(origMB), 0);
        BYTE je = 0x74;
        WriteProcessMemory(handle, (LPVOID)addrMayaPlayer, &je, 1, 0);
        WriteProcessMemory(handle, (LPVOID)addrMayaMob, &je, 1, 0);
        cout << ">>> TUDO DESATIVADO!" << endl;
    }
}

// --- THREADS ---
DWORD WINAPI findProcess(void* data) {
    while (true) {
        if (handle == NULL) {
            DWORD procID = GetProcId("Ragexe.exe");
            if (procID != 0) {
                handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
                if (handle == NULL) {
                    cout << "Erro: Rode como Administrador!" << endl;
                    erro = TRUE;
                    break;
                }
                cout << "Ragexe.exe vinculado!" << endl;
            }
        }
        Sleep(1000);
    }
    return EXIT_SUCCESS;
}

DWORD WINAPI HotkeysThread(void* data) {
    while (true) {
        if (handle != NULL) {
            // ALTERADO PARA F12
            if (GetAsyncKeyState(VK_F12) & 0x8000) {
                masterToggle = !masterToggle;
                ApplyAllPatches(masterToggle);
                // Espera soltar a tecla para não bugar o toggle
                while (GetAsyncKeyState(VK_F12) & 0x8000) Sleep(100);
            }
        }
        Sleep(50);
    }
    return EXIT_SUCCESS;
}

// --- MAIN ---
int main() {
    SetConsoleTitleA("Autenticacao Requerida");

    string inputPass;
    cout << "===============================" << endl;
    cout << "   SISTEMA DE SEGURANCA       " << endl;
    cout << "===============================" << endl;
    cout << "Digite a chave do dia: ";
    cin >> inputPass;

    if (!validatePassword(inputPass)) {
        cout << "\n[!] Senha incorreta. Encerrando..." << endl;
        Sleep(2000);
        return 0;
    }

    system("cls");
    SetConsoleTitleA("Honesto - Ragexe");
    cout << "--- Honesto Ativo ---" << endl;
    cout << "Acesso concedido!" << endl;
    cout << "[F12] Toggle Master (NoDelay, MouseBoost, Maya)" << endl; // Texto atualizado

    CreateThread(NULL, 0, findProcess, NULL, 0, NULL);
    CreateThread(NULL, 0, HotkeysThread, NULL, 0, NULL);

    while (!erro) {
        Sleep(1000);
    }

    if (handle) CloseHandle(handle);
    return EXIT_SUCCESS;
}