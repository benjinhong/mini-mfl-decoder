#include <iostream>
#include <ctime>
#include <stdio.h>
using namespace std;

bool disableFlag = false;
int msg = 0x1D6;

int main() {
    for (int i = 0; i < 20; i++) {
        switch(i) {
            case 0:
                msg = 0x3;
                break;
            case 1:
                msg = 0x1D6;
                break;
            case 2:
                msg = 0x1;
                break;
            case 3:
                msg = 0x2;
                break;
            case 4:
                msg = 0x3;
                break;
            case 5:
                msg = 0x44C;
                break;
            case 6:
                msg = 0x1;
                break;
            case 7:
                msg = 0x1;
                break;
            case 8:
                msg = 0x44C;
                break;
            case 9:
                msg = 0x2;
                break;
            case 10:
                msg = 0x3;
                break;
            case 11:
                msg = 0x1D6;
                break;
            case 12:
                msg = 0x1;
                break;
            case 13:
                msg = 0x2;
                break;
            case 14:
                msg = 0x3;
                break;
            case 15:
                msg = 0x44C;
                break;
            case 16:
                msg = 0x1;
                break;
            case 17:
                msg = 0x1;
                break;
            case 18:
                msg = 0x44C;
                break;
            case 19:
                msg = 0x2;
                break;
        }

        printf("%#03x was called\n", msg);

        if(disableFlag) {
            if(msg == 0x44C) {
                cout << "home display called, clearing disable flag" << endl;
                disableFlag = false;
            } else {
                cout << "home display has not been called, still disabled. ignoring." << endl;
            }
        } else {
            if(msg == 0x1D6) {
                disableFlag = true;
                cout << "phone button called, setting disable flag" << endl;
            } else {
                cout << "something else was called." << endl;
                if(msg == 0x1)
                    cout << "UP" << endl;
                else if (msg == 0x2)
                    cout << "OK" << endl;
                else if (msg == 0x3)
                    cout << "DOWN" << endl;
                else
                    cout << "nothing matched. exiting" << endl;
            }
        }
    }

    return 0;
}
