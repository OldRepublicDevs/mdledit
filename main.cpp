#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include "frame.h"

int WINAPI WinMain(HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument, /// This string contains any CLI arguments that were used to run the .exe
                     int nCmdShow){
    MSG messages;            /* Here messages to the application are saved */

    //std::cout << "These are the arguments: " << lpszArgument << std::endl;

    //Creation of window
    Frame *winMain = new Frame(hThisInstance);
    if(!winMain->Run(nCmdShow)){
        delete winMain;
        return 1; // error
    }

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while(GetMessage (&messages, NULL, 0, 0)){
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }
    //Will only get this far after while is terminated
    delete winMain;

    //Gdiplus::GdiplusShutdown(gdiplusToken);

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}
