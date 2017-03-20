#include "geoview.h"

char GeoView::cClassName[] = "geoview";
LRESULT CALLBACK GeoViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
bool GeoView::bRegistered = false;

GeoView::GeoView(){
    // #1 Basics
    WindowClass.cbSize = sizeof(WNDCLASSEX); // Must always be sizeof(WNDCLASSEX)
    WindowClass.lpszClassName = cClassName; // Name of this class
    WindowClass.hInstance = GetModuleHandle(NULL); // Instance of the application
    WindowClass.lpfnWndProc = GeoViewProc; // Pointer to callback procedure

    // #2 Class styles
    WindowClass.style = CS_DBLCLKS; // Class styles

    // #3 Background
    WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 255, 255)); //(HBRUSH) (COLOR_WINDOW); // Background brush

    // #4 Cursor
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW); // Class cursor

    // #5 Icon
    WindowClass.hIcon = NULL; // Class Icon
    WindowClass.hIconSm = NULL; // Small icon for this class

    // #6 Menu
    WindowClass.lpszMenuName = NULL; // Menu Resource

    // #7 Other
    WindowClass.cbClsExtra = 0; // Extra bytes to allocate following the wndclassex structure
    WindowClass.cbWndExtra = 0; // Extra bytes to allocate following an instance of the structure
}

bool GeoView::Run(){
    if(!bRegistered){
        if(!RegisterClassEx(&WindowClass)){
            std::cout<<string_format("Registering Window Class %s failed!\n", WindowClass.lpszClassName);
            return false;
        }
        std::cout<<string_format("Class %s registered!\n", WindowClass.lpszClassName);
        bRegistered = true;
    }
    //HMENU HAS to be NULL!!!!! Otherwise the function fails to create the window!
    hMe = CreateWindowEx(NULL, WindowClass.lpszClassName, "Geometry Viewer", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                         CW_USEDEFAULT, CW_USEDEFAULT, 600, 500,
                         HWND_DESKTOP, NULL, GetModuleHandle(NULL), NULL);
    if(!hMe) return false;
    ShowWindow(hMe, true);
    return true;
}

extern std::vector<GeoView> GeoViews;

LRESULT CALLBACK GeoViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    //FACT: this is a static function. It cannot be made into variants depending on what window calls it.
    //Therefore, we have to check for that manually, here. We can only go by one thing - hwnd.
    //So, I need to make sure that I can access all the window handles here.
    GeoView * ptr_me = nullptr;
    int nOurGV = -1;
    for(int n = 0; n < GeoViews.size(); n++){
        if(GeoViews.at(n).hMe == hwnd){
            ptr_me = &GeoViews.at(n);
            nOurGV = n;
            n = GeoViews.size();
        }
    }
    /**
    Now, actually it takes a while for the handles to recognize that they're the same, so we are going to skip some messages, including WM_CREATE,
    if we just return default when the pointer is null. Though on the other hand, as long as the pointer is null, we have no way to access the data
    that would be used in WM_CREATE anyway.
    /**/
    if(ptr_me == nullptr) return DefWindowProc (hwnd, message, wParam, lParam);
    //From here on, I know exactly what object I belong to and have access to all its data
    GeoView & me = GeoViews.at(nOurGV);

    //Prepare custom cursor for use
    static HCURSOR hcDrag = (HCURSOR) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_CUR_DRAG), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);

	PAINTSTRUCT ps;
	HDC hdc;

    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);
    POINT ptCurrent;
    ptCurrent.x = xPos;
    ptCurrent.y = yPos;

    if(!me.bInit){
        //This is the first time we got through with a valid object pointer. Initialize stuff now!
        me.fZoomDistance = 8.0;
        //POINT ptMeshCenter = me.GetCenter();
        GetClientRect(hwnd, &me.rcClient);
        me.ptBasicOffset.x = me.rcClient.right/2;// + ptMeshCenter.x;
        me.ptBasicOffset.y = me.rcClient.bottom/2;// - ptMeshCenter.y;
        //me.ptCenter.x = me.ptBasicOffset.x;// + ptMeshCenter.x;
        //me.ptCenter.y = me.ptBasicOffset.y;// - ptMeshCenter.y;
        me.ptScroll.x = 0;
        me.ptScroll.y = 0;
        me.bDrag = false;
        me.bInit = true;
    }

    /* handle the messages */
    switch(message){
        case WM_MOUSEMOVE:
        {
            GetClientRect(hwnd, &me.rcClient);

            if(me.bDrag){
                //std::cout<<"Dragging.\n";
                me.ptScroll.x += ptCurrent.x - me.ptHover.x;
                me.ptScroll.y += ptCurrent.y - me.ptHover.y;
                //std::cout<<"Dragging x: "<<me.ptScroll.x<<", "<<ptCurrent.x<<" - "<<me.ptHover.x<<".\n";
                //me.ptCenter.x = me.ptBasicOffset.x - me.ptClick.x + me.ptHover.x;
                //me.ptCenter.y = me.ptBasicOffset.y - me.ptClick.y + me.ptHover.y;
                me.ptHover = ptCurrent;
                InvalidateRect(hwnd, &me.rcClient, false);
            }
            else{
                me.ptHover = ptCurrent;
                //std::cout<<"Determining current hover.\n";
                POINT Trigon [3];
                bool bFound = false;
                int nFace = -1;
                int nEdge = -1;
                for(int f = 0; f < me.Data.Mesh.Faces.size() && !bFound; f++){
                    //std::cout<<"Checking face "<<f<<".\n";
                    Face & face = me.Data.Mesh.Faces.at(f);
                    HRGN Region;
                    Trigon[0] = me.GetVertPOINT(face.nIndexVertex[0]);
                    Trigon[1] = me.GetVertPOINT(face.nIndexVertex[1]);
                    Trigon[2] = me.GetVertPOINT(face.nIndexVertex[2]);
                    Region = CreatePolygonRgn(Trigon, 3, WINDING);
                    if(PtInRegion(Region, me.ptHover.x, me.ptHover.y)){
                        //std::cout<<"Found the face we are hovering over: "<<f<<".\n";
                        bFound = true;
                        nFace = f;
                        if(bCursorOnLine(me.ptHover, Trigon[0], Trigon[1], 7)){
                            nEdge = 0;
                        }
                        else if(bCursorOnLine(me.ptHover, Trigon[1], Trigon[2], 7)){
                            nEdge = 1;
                        }
                        else if(bCursorOnLine(me.ptHover, Trigon[2], Trigon[0], 7)){
                            nEdge = 2;
                        }
                        else nEdge = -1;
                    }
                    DeleteObject(Region);
                }
                if((nEdge != me.nEdge && nFace == me.nFace) || nFace != me.nFace){
                    //std::cout<<"Detected a change of hover object, invalidating client now.\n";
                    if(me.nFace != -1){
                        RECT rcInvalidate = me.GetFaceRect(me.nFace);
                        InvalidateRect(hwnd, &rcInvalidate, false);
                        //std::cout<<"Invalidated rect ("<<rcInvalidate.left<<", "<<rcInvalidate.top<<", "<<rcInvalidate.right<<", "<<rcInvalidate.bottom<<"). Our cursor location is ("<<me.ptHover.x<<", "<<me.ptHover.y<<").\n";
                    }
                    me.nEdge = nEdge;
                    me.nFace = nFace;
                    if(me.nFace != -1){
                        RECT rcInvalidate = me.GetFaceRect(me.nFace);
                        InvalidateRect(hwnd, &rcInvalidate, false);
                    }
                }
            }
        }
        break;
        case WM_LBUTTONDOWN:
        {
            GetClientRect(hwnd, &me.rcClient);
            RECT rcClip = me.rcClient;
            MapWindowPoints(hwnd, NULL, (LPPOINT) &rcClip, 2);
            ClipCursor(&rcClip);
            me.ptClick.x = ptCurrent.x + me.ptScroll.x;
            me.ptClick.y = ptCurrent.y + me.ptScroll.y;
            me.bDrag = true;
            SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR) hcDrag);
        }
        break;
        case WM_LBUTTONUP:
        {
            GetClientRect(hwnd, &me.rcClient);
            //Release cursor
            ClipCursor((LPRECT) NULL);
            if(me.bDrag){
                me.bDrag = false;
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR) LoadCursor(NULL, IDC_ARROW));
            }
        }
        break;
        case WM_SIZE:
        {
            int nCenterOldx = me.ptScroll.x - me.rcClient.right/2;
            int nCenterOldy = me.ptScroll.y - me.rcClient.bottom/2;
            GetClientRect(hwnd, &me.rcClient);
            me.ptScroll.x = nCenterOldx + me.rcClient.right/2;
            me.ptScroll.y = nCenterOldy + me.rcClient.bottom/2;

            InvalidateRect(hwnd, &me.rcClient, false);
        }
        break;
        case WM_PAINT:
        {
            if(DEBUG_LEVEL > 50) std::cout<<"GeoView: WM_PAINT. Start.\n";

            //Setup memory hdc
            HDC hdcReal = BeginPaint(hwnd, &ps);
            hdc = CreateCompatibleDC(hdcReal);
            HBITMAP memBM = CreateCompatibleBitmap(hdcReal, me.rcClient.right, me.rcClient.bottom);
            HBITMAP hBMold = (HBITMAP) SelectObject(hdc, memBM);
            //Done, now draw

            GetClientRect(hwnd, &me.rcClient);
            HBRUSH hFill = CreateSolidBrush(RGB(200, 200, 200));
            FillRect(hdc, &me.rcClient, hFill);
            DeleteObject(hFill);

            Gdiplus::Point Trigon [3];
            Color MaterialColor;
            Gdiplus::Graphics graphics(hdc);
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias8x8);
            Gdiplus::Pen penEdgeSel(Gdiplus::Color(255, 255, 0, 0), 3.0);
            Gdiplus::Pen penEdge(Gdiplus::Color(255, 180, 0, 0), 1.0);
            Gdiplus::SolidBrush brushEdge(Gdiplus::Color(255, 150, 0, 0));
            Gdiplus::Pen penFace(Gdiplus::Color(255, 220, 0, 0), 1.0);
            Gdiplus::SolidBrush brushFace(Gdiplus::Color(255, 250, 0, 0));
            //std::cout<<"Start painting faces.\n";
            for(int f = 0; f < me.Data.Mesh.Faces.size(); f++){
                Face & face = me.Data.Mesh.Faces.at(f);
                if(f != me.nFace){
                    Trigon[0] = me.GetVertPoint(face.nIndexVertex[0]);
                    Trigon[1] = me.GetVertPoint(face.nIndexVertex[1]);
                    Trigon[2] = me.GetVertPoint(face.nIndexVertex[2]);
                    MaterialColor = me.GetMaterialBaseColor(face.nMaterialID);
                    MaterialColor.ConvertToByte();
                    Gdiplus::Pen pen(Gdiplus::Color(255, std::max(0, MaterialColor.nR - 30), std::max(0, MaterialColor.nG - 30), std::max(0, MaterialColor.nB - 30)));
                    Gdiplus::SolidBrush solidBrush(Gdiplus::Color(255, std::min(255, MaterialColor.nR + 5), std::min(255, MaterialColor.nG + 5), std::min(255, MaterialColor.nB + 5)));
                    graphics.FillPolygon(&solidBrush, Trigon, 3);
                    graphics.DrawPolygon(&pen, Trigon, 3);
                }
            }
            //std::cout<<"Paint selected face.\n";
            if(me.nFace != -1){
                Face & face = me.Data.Mesh.Faces.at(me.nFace);
                Trigon[0] = me.GetVertPoint(face.nIndexVertex[0]);
                Trigon[1] = me.GetVertPoint(face.nIndexVertex[1]);
                Trigon[2] = me.GetVertPoint(face.nIndexVertex[2]);
                if(me.nEdge == -1){
                    graphics.FillPolygon(&brushFace, Trigon, 3);
                    graphics.DrawPolygon(&penFace, Trigon, 3);
                }
                else if(me.nEdge == 0){
                    graphics.FillPolygon(&brushEdge, Trigon, 3);
                    graphics.DrawLine(&penEdgeSel, Trigon[0], Trigon[1]);
                    graphics.DrawLine(&penEdge, Trigon[1], Trigon[2]);
                    graphics.DrawLine(&penEdge, Trigon[2], Trigon[0]);
                }
                else if(me.nEdge == 1){
                    graphics.FillPolygon(&brushEdge, Trigon, 3);
                    graphics.DrawLine(&penEdge, Trigon[0], Trigon[1]);
                    graphics.DrawLine(&penEdgeSel, Trigon[1], Trigon[2]);
                    graphics.DrawLine(&penEdge, Trigon[2], Trigon[0]);
                }
                else if(me.nEdge == 2){
                    graphics.FillPolygon(&brushEdge, Trigon, 3);
                    graphics.DrawLine(&penEdge, Trigon[0], Trigon[1]);
                    graphics.DrawLine(&penEdge, Trigon[1], Trigon[2]);
                    graphics.DrawLine(&penEdgeSel, Trigon[2], Trigon[0]);
                }
            }

            //std::cout<<"Done painting.\n";
            //Done drawing, finish up
            BitBlt(hdcReal, 0, 0, me.rcClient.right, me.rcClient.bottom, hdc, 0, 0, SRCCOPY);
            SelectObject(hdc, hBMold);
            DeleteObject(memBM);
            DeleteDC(hdc);
            EndPaint(hwnd, &ps);

            if(DEBUG_LEVEL > 50) std::cout<<"GeoView: WM_PAINT. End.\n";
        }
        break;
        case WM_MOUSEWHEEL:
        {
            bool bZoom = false;
            int nRoll = GET_WHEEL_DELTA_WPARAM(wParam);
            double fOld = me.fZoomDistance;
            double fZoomDistanceMax = 10.0;
            double fZoomDistanceMin = 0.5;
            if(wParam != NULL && nRoll%120==0){
                double fNew = me.fZoomDistance - me.fZoomStep * (double)(nRoll/120);
                if(!(nRoll < 0 && fNew > fZoomDistanceMax) && !(nRoll > 0 && fNew < fZoomDistanceMin)){
                    std::cout<<"We have a new Zoom Distance: "<<fNew<<".\n";
                    me.fZoomDistance = fNew;
                    bZoom = true;
                }
            }
            if(bZoom){
                std::cout<<"Conversion is now: "<< 1.0 / (me.fBaseConversion * powf(2.0, me.fZoomDistance / me.fZoomStep) + (2.0 * me.fZoomDistance * tanf(rad(me.fFieldOfViewAngle / 2.0))) / (double) me.rcClient.right) <<".\n";
                std::cout<<"Old conversions: "<< 1.0 / (me.fBaseConversion * powf(1.5, fOld / me.fZoomStep) + (2.0 * fOld * tanf(rad(me.fFieldOfViewAngle / 2.0))) / (double) me.rcClient.right) <<".\n";
                std::cout<<"Quotient: "<< (1.0 / (me.fBaseConversion * powf(1.5, me.fZoomDistance / me.fZoomStep) + (2.0 * me.fZoomDistance * tanf(rad(me.fFieldOfViewAngle / 2.0))) / (double) me.rcClient.right))/(1.0 / (me.fBaseConversion * powf(1.5, fOld / me.fZoomStep) + (2.0 * fOld * tanf(rad(me.fFieldOfViewAngle / 2.0))) / (double) me.rcClient.right)) <<".\n";
                double fConversion = 1.0 / (me.fBaseConversion * powf(2.0, me.fZoomDistance / me.fZoomStep) + (2.0 * me.fZoomDistance * tanf(rad(me.fFieldOfViewAngle / 2.0))) / (double) me.rcClient.right);
                me.ptScroll.x = (int) roundf(me.fZoomDistance/fOld * ((double) me.ptScroll.x - (double) me.rcClient.right/2.0) + (double)me.rcClient.right/2.0);
                me.ptScroll.y = (int) roundf(me.fZoomDistance/fOld * ((double) me.ptScroll.y - (double) me.rcClient.bottom/2.0) + (double)me.rcClient.bottom/2.0);
                //me.ptBasicOffset.x = (int) roundf(me.fZoomDistance/fOld * ((double) me.ptBasicOffset.x + (double) me.rcClient.right/2.0) - (double)me.rcClient.right/2.0);
                //me.ptBasicOffset.y = (int) roundf(me.fZoomDistance/fOld * ((double) me.ptBasicOffset.y + (double) me.rcClient.bottom/2.0) - (double)me.rcClient.bottom/2.0);
                me.ptBasicOffset.x = (int) roundf(me.fZoomDistance/fOld * (double) me.ptBasicOffset.x);
                me.ptBasicOffset.y = (int) roundf(me.fZoomDistance/fOld * (double) me.ptBasicOffset.y);
                //POINT ptMeshCenter = me.GetCenter();
                //me.ptCenter.x = me.rcClient.right/2 + ptMeshCenter.x;
                //me.ptCenter.y = me.rcClient.bottom/2 - ptMeshCenter.y;
                InvalidateRect(hwnd, &me.rcClient, false);
            }
        }
        break;
        case WM_DESTROY:
            DestroyWindow(hwnd);
        break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        default:
        {
            return DefWindowProc (hwnd, message, wParam, lParam);
        }
    }
    return 0;
}
