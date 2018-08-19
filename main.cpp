#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include "frame.h"
#include "CommandLineToArgvA.h"

int WINAPI WinMain(HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument, /// This string contains any CLI arguments that were used to run the .exe
                     int nCmdShow){
    MSG messages;            /* Here messages to the application are saved */

    std::cout << lpszArgument;
    if(!std::string(lpszArgument).empty()){
        /*/
        Warning("You have run mdledit with arguments, but this is not supported yet!\n\n"
                "These are the arguments: " + std::string(lpszArgument));
        /** /

        AllocConsole();
        std::string sRep;

        std::cout << "Text to new console!\n";
        std::cin >> sRep;
        int argc = 0;
        LPSTR * argv = CommandLineToArgvA(lpszArgument, &argc);

        ProcessCommandLineCall(argc, argv);

        LocalFree(argv);
        FreeConsole();

        return 0; // Exit the program.
        /**/
    }

    //Creation of window
    Frame *winMain = new Frame(hThisInstance);
    if(!winMain->Run(nCmdShow)){
        delete winMain;
        return 1; // error
    }

    /// Load the accelerator table:
    HACCEL hAccel = LoadAccelerators(hThisInstance, "MdleditAccel");

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while(GetMessage (&messages, NULL, 0, 0)){

        /// Translate Accelerators
        if(!TranslateAccelerator(winMain->GetHwnd(), hAccel, &messages)){
            /* Translate virtual-key messages into character messages */
            TranslateMessage(&messages);
            /* Send message to WindowProcedure */
            DispatchMessage(&messages);
        }
    }
    //Will only get this far after while is terminated
    delete winMain;

    //Gdiplus::GdiplusShutdown(gdiplusToken);

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}
