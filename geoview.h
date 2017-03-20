#ifndef GEOVIEW_H_INCLUDED
#define GEOVIEW_H_INCLUDED

#include "general.h"
#include <windowsx.h>
#include <gdiplus.h>
#include "MDL.h"


class GeoView{
    WNDCLASSEX WindowClass;
    static char cClassName [];
    static bool bRegistered;

    //Window functionality
    bool bInit = false;
    Node Data;
    bool bDrag;
    RECT rcClient;
    POINT ptCenter;
    POINT ptBasicOffset;
    POINT ptClick;
    POINT ptHover;
    POINT ptScroll;
    int nFace = -1;
    int nEdge = -1;
    double fZoomDistance;
    double fZoomStep = 0.5;
    double fFieldOfViewAngle = 100.0; //in degrees

    /// Defined as 'a / b', 'a' is in game coords, 'b' in screen coords - the conversion if distance from us to the object is 0
    double fBaseConversion = 0.01/100.0;

public:
    HWND hMe;
    GeoView();
    bool Run();
    friend LRESULT CALLBACK GeoViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void SetData(Node & node){
        Data = node;
    }
    RECT GetFaceRect(int nIndex){
        //std::cout<<"Gettin rect. Our face is "<<nIndex<<".\n";
        Face & face = Data.Mesh.Faces.at(nIndex);
        //std::cout<<"Our verts are "<<face.nIndexVertex[0]<<", "<<face.nIndexVertex[1]<<" and "<<face.nIndexVertex[2]<<".\n";
        Vector & v1 = Data.Mesh.Vertices.at(face.nIndexVertex[0]);
        Vector & v2 = Data.Mesh.Vertices.at(face.nIndexVertex[1]);
        Vector & v3 = Data.Mesh.Vertices.at(face.nIndexVertex[2]);
        RECT rcReturn;
        rcReturn.left = ToScreenCoords(std::min(v1.fX, std::min(v2.fX, v3.fX))) - ptScroll.x + ptCenter.x - 10;
        rcReturn.right = ToScreenCoords(std::max(v1.fX, std::max(v2.fX, v3.fX))) - ptScroll.x + ptCenter.x + 10;
        rcReturn.top = 0-ToScreenCoords(std::min(v1.fY, std::min(v2.fY, v3.fY))) - ptScroll.y + ptCenter.y + 10;
        rcReturn.bottom = 0-ToScreenCoords(std::max(v1.fY, std::max(v2.fY, v3.fY))) - ptScroll.y + ptCenter.y - 10;
        return rcReturn;
    }
    int ToScreenCoords(double fCoords){
        /// This function should take the bare coords and do all the transformations on them
        //double fConversion = 6000.0/fZoomDistance / std::max(Data.Mesh.vBBmax.fX-Data.Mesh.vBBmin.fX, Data.Mesh.vBBmax.fY-Data.Mesh.vBBmin.fY);
        //double fConversion = fBaseConversion * powf(2.0, - ((double) rcClient.right / ((double) rcClient.right + 2.0 * tanf(fFieldOfViewAngle / 2.0) * fZoomDistance)) / ((double) rcClient.right / ((double) rcClient.right + 2.0 * tanf(fFieldOfViewAngle / 2.0) * fZoomStep)));
        //double fConversion = fBaseConversion * powf(1.5, - fZoomDistance / fZoomStep);
        double fConversion = 1.0 / (fBaseConversion * powf(1.5, fZoomDistance / fZoomStep) + (2.0 * fZoomDistance * tanf(rad(fFieldOfViewAngle / 2.0))) / (double) rcClient.right);
        double fScreenCoords = roundf(fCoords * fConversion);
        //std::cout<<"Converted coordinate "<<fCoords<<" to screen coordinate "<<fScreenCoords<<" (conversion factor "<<fConversion<<").\n";
        return (int) fScreenCoords;
    }
    Vector ApplyTranformations(const double & fx, const double & fy){
        Vector vWorking(fx, fy);
        //vWorking.print("Start.");
        double fBasic = 100.0 / powf(2.0, fZoomDistance / fZoomStep);
        double fConversion = 1.0 / (fBaseConversion * powf(2.0, fZoomDistance / fZoomStep) + (2.0 * fZoomDistance * tanf(rad(fFieldOfViewAngle / 2.0))) / (double) rcClient.right);
        //scaling by fBasic
        vWorking*=Matrix22(fBasic, 0.0,
                             0.0, fBasic);
       // vWorking.print("Scaled by basic.");
        //mirroring for y
        vWorking*=Matrix22(1.0, 0.0,
                             0.0, -1.0);
        //vWorking.print("Mirrored for y.");
        //std::cout<<"Basic offset: "<<ptBasicOffset.x<<", "<<ptBasicOffset.y<<".\n";
        //move by basic offset
        vWorking+=Vector(ptBasicOffset);
        //vWorking.print("Moved by basic offset.");
        //move by scroll
        vWorking+=Vector(ptScroll);
        //vWorking.print("Moved by scroll.");

        //done
        return vWorking;
    }
    POINT GetCenter(){
        POINT ptReturn;
        ptReturn.x = ToScreenCoords((Data.Mesh.vBBmax.fX+Data.Mesh.vBBmin.fX)/2);
        ptReturn.y = ToScreenCoords((Data.Mesh.vBBmax.fY+Data.Mesh.vBBmin.fY)/2);
        return ptReturn;
    }
    Gdiplus::Point GetVertPoint(int nIndex){
        Gdiplus::Point ptReturn;
        Vector v = ApplyTranformations(Data.Mesh.Vertices.at(nIndex).fX, Data.Mesh.Vertices.at(nIndex).fY);
        ptReturn.X = v.fX;
        ptReturn.Y = v.fY;
        //ptReturn.X = ToScreenCoords(Data.Mesh.Vertices.at(nIndex).fX);
        //ptReturn.Y = ToScreenCoords(Data.Mesh.Vertices.at(nIndex).fY);
        return ptReturn;
    }
    POINT GetVertPOINT(int nIndex){
        POINT ptReturn;
        Vector v = ApplyTranformations(Data.Mesh.Vertices.at(nIndex).fX, Data.Mesh.Vertices.at(nIndex).fY);
        ptReturn.x = v.fX;
        ptReturn.y = v.fY;
        //ptReturn.x = ToScreenCoords(Data.Mesh.Vertices.at(nIndex).fX) - ptScroll.x + ptCenter.x;
        //ptReturn.y = 0-ToScreenCoords(Data.Mesh.Vertices.at(nIndex).fY) - ptScroll.y + ptCenter.y;
        return ptReturn;
    }
    Color GetMaterialBaseColor(int nMaterial){
        if(nMaterial == MATERIAL_NONE) return Color(1.0, 1.0, 1.0);
        else if(nMaterial == MATERIAL_DIRT) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_OBSCURING) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_GRASS) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_STONE) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_WOOD) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_WATER) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_NONWALK) return Color(181.0/255.0, 70.0/255.0, 201.0/255.0);
        else if(nMaterial == MATERIAL_TRANSPARENT) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_CARPET) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_METAL) return Color(20.0/255.0, 20.0/255.0, 20.0/255.0);
        else if(nMaterial == MATERIAL_PUDDLES) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_SWAMP) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_MUD) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_LEAVES) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_LAVA) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_BOTTOMLESSPIT) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_DEEPWATER) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_DOOR) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_SNOW) return Color(0.0, 0.0, 0.0);
        else if(nMaterial == MATERIAL_SAND) return Color(0.0, 0.0, 0.0);
        else return Color(1.0, 1.0, 1.0);
    }
};

#endif // GEOVIEW_H_INCLUDED
