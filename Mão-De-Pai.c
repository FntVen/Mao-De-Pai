#include <SDL3/SDL_pixels.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <stdio.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <math.h>
SDL_Window *window = NULL;
SDL_Renderer *Render = NULL;
 //--------------------------//
//    Basic For Elements    //
enum RenderFunctions{
    RENDER_SQUARE,
    RENDER_RSQUARE,//NON-FUNCTIONAL
    RENDER_LINE,
    RENDER_DOT,
    RENDER_GEOMETRY,
    RENDER_BUTTON,
    RENDER_RBUTTON,
    RENDER_LABEL,
    RENDER_TEXTBOX,//NON-FUNCTIONAL
    RENDER_DROPDOWN,//NON-FUNCTIONAL
};
typedef struct{
    float A;
    float R;
    float G;
    float B;
}Color;
//--------------------------//
//    Basics for Shapes    //
enum ShapeTypes{
    PADDINGSHAPE_T,
    SQUARE_T,
    LINE_T,
    POINT_T
};
typedef struct{
    float x1;
    float y1;
    float x2;
    float y2;
}SDL_Line;
typedef union{
    SDL_FRect Square;
    SDL_Line Line;
    SDL_Point Point;
}ShapeU;
typedef struct{
    ShapeU Form;
    int SHTYPE;
    int RenderFunction;
    Color Color;
}ShapeElement;
typedef struct{
    ShapeU *Form;
    int *SHTYPE;
    int *RenderFunction;
    Color *Color;
    int TotalArrayLength;
    int Indexes;
}ShapeList;
//--------------------------//
//   Basics for Objects    //
enum Exec_Functions{
    BUTTONFUNCTION,
    RBUTTONFUNCTION,
    TEXTBOXFUNCTION
};            //Enum of Functions Objects can Execute
enum ObjectTypes{
  PADDINGOBJ_T,
  BUTTON_T,
  RBUTTON_T,
  LABEL_T,
  TEXTBOX_T
};               //All of the different Objects in the form of a enums for easy filtering
typedef struct{
    int PlaceHolder;
}TextBoxParam;     //Standard parameters for TextBoxes | Parameters Should change from Application to Application usually
typedef struct{
    int PlaceHolder;
}ButtonParam;      //Standard parameters for Buttons | Parameters Should change from Application to Application usually
typedef struct{
    int PlaceHolder;
}RButtonParam;     //Standard parameters for Radio Buttons | Parameters Should change from Application to Application usually
typedef union{
    ButtonParam Button_P;
    RButtonParam RButton_P;
    TextBoxParam TextBox_P;
}EventParams;       //Union of all Standard Parameters of existing Objects for the Object Type
typedef struct{
    char *String;
    int Length;
    float X;
    float Y;
    float Width;
    float Height;
    int Border_Thickness;
    Color ARGB_BackGround;
    Color ARGB_Border;
}SDL_TextBox;      //Standard data to render a TextBox
typedef struct{
    const char *String;
    int Lenght;
    Color ARGB;
    int FontSize;
    float X;
    float Y;
    SDL_Texture *Texture;
    float Width;
    float Height;
}SDL_Label;        //Standard data to render a Label
typedef struct{
    float X;
    float Y;
    Color ARGB_Used;
    Color ARGB_Normal;
    Color ARGB_HighLight;
    Color ARGB_Disabled;
}SDL_Button;       //Standard data to render a Button
typedef struct{
    float X;
    float Y;
    int Size;
    bool State;
    Color ARGB_Front;
    Color ARGB_Back;
}SDL_RadioButton;  //Standard data to render a Radio Button
typedef union{
    SDL_TextBox TextBox;
    SDL_Label Label;
    SDL_Button Button;
    SDL_RadioButton RadioButton;
}ObjectU;
typedef struct{
    ObjectU Object;
    int OBJTYPE;
    int RenderFunction;
    int ExecutingFunction;
    SDL_FRect HitBox;
    EventParams Parameter;
}ObjectElement;    //Union of all standard data to render an Object
typedef struct{
    ObjectU *Object;
    int *OBJTYPE;
    int *RenderFunction;
    int *ExectuingFunction;
    SDL_FRect *HitBox;
    EventParams *Parameter;
    int TotalArrayLength;
    int Indexes;
}ObjectList;       //An struct AoS that holds the data for every object | Every object should have every parameter, so all lists should be in sync
//--------------------------//
// Basics for Control Flow //
typedef struct{
    float X;
    float Y;
}MousePos;
typedef struct{
    MousePos XY;//Mouse Position
    SDL_Cursor *Pointer_Normal;//Normal Mouse State
    SDL_Cursor *Pointer_Text;//Mouse hovering a TextBox and the like
    SDL_Cursor *pointer_Select;//Mouse Hovering a selectable item
}MouseControl;
MouseControl MouseHandler;
bool _Update = true;                //The signal to manage when objects and shapes need to be recalculated
//--------------------------//
//         Helpers         //
typedef union{
    int NUMBER;
    char DIGIT;
}SimpleConvert;
typedef struct{
    bool _Error;
    SDL_AppResult SDL_State;
    char *Message;
    int MessageLength;
}Error;
int WnWidth = 1200;                 //Screen Standard Width
int WnHeight = 600;                 //Screen Standard Height
ObjectList OBJFront;
ShapeList SHFront;
ObjectList OBJBack;
ShapeList SHBack;
TTF_Font *MainFont;
SDL_AppResult SetFont()
{
    if(!TTF_Init())
    {
        SDL_Log("Error in the Font Loader");
        return SDL_APP_FAILURE;
    }
    TTF_Font *Font = TTF_OpenFont("Resources/Roboto-Regular.ttf", 24);
    if(!Font)
    {
        SDL_Log("Error Loading Font");
        return SDL_APP_FAILURE;
    }
    MainFont = Font;
    return  SDL_APP_CONTINUE;
}
Error ExpandShapeList(int Depth,int ExpandFactor)
{
    Error Err = (Error){
        ._Error = true,
        .SDL_State = SDL_APP_FAILURE,
    };
    if(Depth == 1)
    {
        ShapeU *NewForm = malloc(sizeof(ShapeU)*(SHFront.TotalArrayLength+ExpandFactor));
        int *NewSHTYPE = malloc(sizeof(int)*(SHFront.TotalArrayLength+ExpandFactor));
        int *NewRenderFunction = malloc(sizeof(int)*(SHFront.TotalArrayLength+ExpandFactor));
        Color *NewColor = malloc(sizeof(Color)*(SHFront.TotalArrayLength+ExpandFactor));
        if(NewForm == NULL ||NewSHTYPE == NULL || NewRenderFunction)
        {
            Err.Message = "Error In Memory Allocation to Expand ShapeU\n";
            Err.MessageLength = 45;
            return Err;
        }
        for(int i; i<=SHFront.Indexes;i++){
            NewForm[i] = SHFront.Form[i];
            NewSHTYPE[i] = SHFront.SHTYPE[i];
            NewRenderFunction[i] = SHFront.RenderFunction[i];
            NewColor[i] = SHFront.Color[i];
        }
        SHFront.Form = NewForm;//Reminder: These already are pointers since they are arrays
        SHFront.SHTYPE = NewSHTYPE;
        SHFront.RenderFunction = NewRenderFunction;
        SHFront.Color = NewColor;
        SHFront.TotalArrayLength++;
    }
    if(Depth == 0)
    {
        ShapeU *NewForm = malloc(sizeof(ShapeU)*(SHBack.TotalArrayLength+ExpandFactor));
        int *NewSHTYPE = malloc(sizeof(int)*(SHBack.TotalArrayLength+ExpandFactor));
        int *NewRenderFunction = malloc(sizeof(int)*(SHBack.TotalArrayLength+ExpandFactor));
        Color *NewColor = malloc(sizeof(Color)*(SHBack.TotalArrayLength+ExpandFactor));
        if(NewForm == NULL ||NewSHTYPE == NULL || NewRenderFunction)
        {
            Err.Message = "Error In Memory Allocation to Expand ShapeList\n";
            Err.MessageLength = 48;
            return Err;
        }
        for(int i; i<=SHBack.Indexes;i++){
            NewForm[i] = SHBack.Form[i];
            NewSHTYPE[i] = SHBack.SHTYPE[i];
            NewRenderFunction[i] = SHBack.RenderFunction[i];
            NewColor[i] = SHBack.Color[i];
        }
        SHBack.Form = NewForm;//Reminder: These already are pointers since they are arrays
        SHBack.SHTYPE = NewSHTYPE;
        SHBack.RenderFunction = NewRenderFunction;
        SHBack.Color = NewColor;
        SHBack.TotalArrayLength++;
    }
    return Err;
}
Error ExpandObjectList(int Depth,int ExpandFactor)
{
    Error Err = (Error){
        ._Error = true,
        .SDL_State = SDL_APP_FAILURE,
    };
    if(Depth == 1)
    {
        ObjectU *NewObjectU = malloc(sizeof(ObjectU)*(OBJFront.TotalArrayLength+ExpandFactor));
        int *NewOBJTYPE = malloc(sizeof(int)*(OBJFront.TotalArrayLength+ExpandFactor));
        int *NewRenderFunction = malloc(sizeof(int)*(OBJFront.TotalArrayLength+ExpandFactor));
        int *NewExecutingFunction = malloc(sizeof(int)*(OBJFront.TotalArrayLength+ExpandFactor));
        SDL_FRect *NewHitBox = malloc(sizeof(SDL_FRect)*(OBJFront.TotalArrayLength+ExpandFactor));
        EventParams *NewParam = malloc(sizeof(EventParams)*(OBJFront.TotalArrayLength+ExpandFactor));
        if(NewObjectU == NULL || NewOBJTYPE == NULL || NewRenderFunction == NULL || NewExecutingFunction == NULL || NewHitBox == NULL || NewParam == NULL)
        {
            Err.Message = "Error In Memory Allocation to Expand ObjectList\n";
            Err.MessageLength = 49;
            return Err;
        }
        for(int i = 0;i<OBJFront.Indexes;i++)
        {
            NewObjectU[i] = OBJFront.Object[i];
            NewOBJTYPE[i] = OBJFront.OBJTYPE[i];
            NewExecutingFunction[i] = OBJFront.ExectuingFunction[i];
            NewRenderFunction[i] = OBJFront.RenderFunction[i];
            NewHitBox[i] = OBJFront.HitBox[i];
            NewParam[i] = OBJFront.Parameter[i];
        }
        OBJFront.Object = NewObjectU;
        OBJFront.OBJTYPE = NewOBJTYPE;
        OBJFront.RenderFunction = NewRenderFunction;
        OBJFront.ExectuingFunction = NewExecutingFunction;
        OBJFront.HitBox = NewHitBox;
        OBJFront.Parameter = NewParam;
    }
    if(Depth == 0)
    {
        ObjectU *NewObjectU = malloc(sizeof(ObjectU)*(OBJBack.TotalArrayLength+ExpandFactor));
        int *NewOBJTYPE = malloc(sizeof(int)*(OBJBack.TotalArrayLength+ExpandFactor));
        int *NewRenderFunction = malloc(sizeof(int)*(OBJBack.TotalArrayLength+ExpandFactor));
        int *NewExecutingFunction = malloc(sizeof(int)*(OBJBack.TotalArrayLength+ExpandFactor));
        SDL_FRect *NewHitBox = malloc(sizeof(SDL_FRect)*(OBJBack.TotalArrayLength+ExpandFactor));
        EventParams *NewParam = malloc(sizeof(EventParams)*(OBJBack.TotalArrayLength+ExpandFactor));
        if(NewObjectU == NULL || NewOBJTYPE == NULL || NewRenderFunction == NULL || NewExecutingFunction == NULL || NewHitBox == NULL)
        {
            Err.Message = "Error In Memory Allocation to Expand ObjectList\n";
            Err.MessageLength = 49;
            return Err;
        }
        for(int i = 0;i<OBJFront.Indexes;i++)
        {
            NewObjectU[i] = OBJBack.Object[i];
            NewOBJTYPE[i] = OBJBack.OBJTYPE[i];
            NewExecutingFunction[i] = OBJBack.ExectuingFunction[i];
            NewRenderFunction[i] = OBJBack.RenderFunction[i];
            NewHitBox[i] = OBJBack.HitBox[i];
            NewParam[i] = OBJBack.Parameter[i];
        }
        OBJBack.Object = NewObjectU;
        OBJBack.OBJTYPE = NewOBJTYPE;
        OBJBack.RenderFunction = NewRenderFunction;
        OBJBack.ExectuingFunction = NewExecutingFunction;
        OBJBack.HitBox = NewHitBox;
        OBJBack.Parameter = NewParam;
    }

    return Err;
}
//--------------------------//
//      Renderers & CO     //
Error CleanALL()
{
    Error Err = (Error){
        ._Error = true,
        .SDL_State = SDL_APP_FAILURE
    };
    return Err;//Move this to the end when how to clean is figured out
    int CR_OBJ = 0;
    int CR_SH = 0;
    int CR = 0;
    if(OBJBack.TotalArrayLength > OBJFront.TotalArrayLength)
    {
        CR_OBJ = OBJBack.TotalArrayLength;
    }
    else
    {
        CR_OBJ = OBJFront.TotalArrayLength;
    }
    if(SHBack.TotalArrayLength > SHFront.TotalArrayLength)
    {
        CR_SH = SHBack.TotalArrayLength;
    }
    else
    {
        CR_SH = SHFront.TotalArrayLength;
    }
    if(CR_OBJ > CR_SH)
    {
        CR = CR_OBJ;
    }
    else
    {
        CR = CR_SH;
    }
    for(int i =0;i<=CR;i++)
    {
        //Pretty Sure this is freeing only the arrays, not the pointers in them
        free(OBJBack.Object);
        free(OBJBack.OBJTYPE);
        free(OBJBack.ExectuingFunction);
        free(OBJBack.RenderFunction);
        free(OBJBack.HitBox);
        free(OBJBack.Parameter);

        free(OBJFront.Object);
        free(OBJFront.OBJTYPE);
        free(OBJFront.ExectuingFunction);
        free(OBJFront.RenderFunction);
        free(OBJFront.HitBox);
        free(OBJFront.Parameter);
    }
}
Error Add_Shape(ShapeElement Sh, int Depth)
{
    Error Err = (Error){
        ._Error = true,
        .SDL_State = SDL_APP_CONTINUE
    };
    if(Depth != 0 || Depth != 1)
    {
        Err.Message = "Invalid Depth | 1 for Front or 0 for Back\n";
        Err.MessageLength = 43;
        return Err;
    }
    if(Depth == 1)
    {
        if(SHFront.Indexes+1 > SHFront.TotalArrayLength)
        {
            Error FunctionErr = ExpandShapeList(Depth,1);
            if(FunctionErr._Error == true)
            {
                return FunctionErr;
            }
        }
        else
        {
            SHFront.Form[SHFront.Indexes] = Sh.Form;
            SHFront.Color[SHFront.Indexes] = Sh.Color;
            SHFront.RenderFunction[SHFront.Indexes] = Sh.RenderFunction;
            SHFront.SHTYPE[SHFront.Indexes] = Sh.SHTYPE;
            SHFront.Indexes++;
        }
    }
    else
    {
        if(SHBack.Indexes+1 > SHBack.TotalArrayLength)
        {
            Error FunctionErr = ExpandShapeList(Depth,1);
            if(FunctionErr._Error == true)
            {
                return FunctionErr;
            }
        }
        else
        {
            SHBack.Form[SHBack.Indexes] = Sh.Form;
            SHBack.Color[SHBack.Indexes] = Sh.Color;
            SHBack.RenderFunction[SHBack.Indexes] = Sh.RenderFunction;
            SHBack.SHTYPE[SHBack.Indexes] = Sh.SHTYPE;
            SHBack.Indexes++;
        }
    }
    Err._Error = false;
    return Err;
}
Error Add_Object(ObjectElement Obj, int Depth)
{
    Error Err = (Error){
        ._Error = true,
        .SDL_State = SDL_APP_CONTINUE
    };
    if(Depth != 0 || Depth != 1)
    {
        Err.Message = "Invalid Depth | 1 for Front or 0 for Back\n";
        Err.MessageLength = 43;
        return Err;
    }
    if(Depth == 1)
    {
        if(OBJFront.Indexes+1 > OBJFront.TotalArrayLength)
        {
            Error FunctionErr = ExpandObjectList(Depth,1);
            if(FunctionErr._Error == true)
            {
                return FunctionErr;
            }
        }
        else
        {
            OBJFront.Object[OBJFront.Indexes] = Obj.Object;
            OBJFront.ExectuingFunction[OBJFront.Indexes] = Obj.ExecutingFunction;
            OBJFront.RenderFunction[SHFront.Indexes] = Obj.RenderFunction;
            OBJFront.OBJTYPE[OBJFront.Indexes] = Obj.OBJTYPE;
            OBJFront.HitBox[OBJFront.Indexes] = Obj.HitBox;
            OBJFront.Parameter[OBJFront.Indexes] = Obj.Parameter;
            OBJFront.Indexes++;
        }
    }
    else
    {
        if(OBJBack.Indexes+1 > OBJBack.TotalArrayLength)
        {
            Error FunctionErr = ExpandObjectList(Depth,1);
            if(FunctionErr._Error == true)
            {
                return FunctionErr;
            }
        }
        else
        {
            OBJBack.Object[OBJBack.Indexes] = Obj.Object;
            OBJBack.ExectuingFunction[OBJBack.Indexes] = Obj.ExecutingFunction;
            OBJBack.RenderFunction[OBJBack.Indexes] = Obj.RenderFunction;
            OBJBack.OBJTYPE[OBJBack.Indexes] = Obj.OBJTYPE;
            OBJBack.HitBox[OBJBack.Indexes] = Obj.HitBox;
            OBJBack.Parameter[OBJBack.Indexes] = Obj.Parameter;
            OBJBack.Indexes++;
        }
    }
    Err._Error = false;
    return Err;
}
Error Render_Polygon(float PosX, float PosY,int Radius, int Ver_Count, Color Color)
{
    Error Err = (Error){
        ._Error = true,
        .SDL_State = SDL_APP_FAILURE
    };
    if(Ver_Count<=2)
    {
        Err.Message = "Poligon Attempting to be rendered with less than 2 vertices";
        return Err;
    }

    Color.A = Color.A/255;
    Color.R = Color.R/255;
    Color.G = Color.G/255;
    Color.B = Color.B/255;
    float Angle_Vertices = (float)(180+(180*(Ver_Count - 3)))/Ver_Count;
    float AngleStep = 0.0174444444445;//One of these days i need to find an explanation for this
    float Spinning_Angle = AngleStep * (180 - Angle_Vertices);

    SDL_Vertex Vertices[Ver_Count+1];
    SDL_zeroa(Vertices);
    int Ind[Ver_Count*3] = {};//Need to find a way to be a const
    int Offset = 0;
    for(int i=2;i<=Ver_Count;i++)
    {
        if(i==2)
        {
            Vertices[0].position.x = PosX;
            Vertices[0].position.y = PosY;
            Vertices[1].position.x = PosX;
            Vertices[1].position.y = PosY - Radius;
            Vertices[2].position.x = ((Vertices[1].position.x - PosX)*cos(Spinning_Angle * (i-1))-(Vertices[1].position.y - PosY)*sin(Spinning_Angle * (i-1))) + PosX;
            Vertices[2].position.y = ((Vertices[1].position.x - PosX)*sin(Spinning_Angle * (i-1))+(Vertices[1].position.y - PosY)*cos(Spinning_Angle * (i-1))) + PosY;
            Ind[0] = 1;
            Ind[1] = 0;
            Ind[2] = 2;
            Vertices[0].color.a = Color.A;
            Vertices[0].color.r = Color.R;
            Vertices[0].color.g = Color.G;
            Vertices[0].color.b = Color.B;
            Vertices[1].color.a = Color.A;
            Vertices[1].color.r = Color.R;
            Vertices[1].color.g = Color.G;
            Vertices[1].color.b = Color.B;
            Vertices[2].color.a = Color.A;
            Vertices[2].color.r = Color.R;
            Vertices[2].color.g = Color.G;
            Vertices[2].color.b = Color.B;
            continue;
        }
        Vertices[i].position.x = ((Vertices[1].position.x - PosX)*cos(Spinning_Angle * (i-1))-(Vertices[1].position.y - PosY)*sin(Spinning_Angle * (i-1))) + PosX;
        Vertices[i].position.y = ((Vertices[1].position.x - PosX)*sin(Spinning_Angle * (i-1))+(Vertices[1].position.y - PosY)*cos(Spinning_Angle * (i-1))) + PosY;
        Vertices[i].color.a = Color.A;
        Vertices[i].color.r = Color.R;
        Vertices[i].color.g = Color.G;
        Vertices[i].color.b = Color.B;
        Ind[i + Offset] = i-1;
        Offset++;
        Ind[i + Offset] = 0;
        Offset++;
        Ind[i + Offset] = i;
        if(i==Ver_Count)
        {
            Offset++;
            Ind[i+Offset] = i;
            Offset++;
            Ind[i+Offset] = 0;
            Offset++;
            Ind[i+Offset] = 1;
        }
    }
    SDL_RenderGeometry(Render, NULL, Vertices, Ver_Count+1,Ind,Ver_Count*3);
    Err._Error = false;
    Err.SDL_State = SDL_APP_CONTINUE;
    return Err;
}
Error SDL_RenderRadioBtn(SDL_RadioButton *RD)
{
    Error Err;
    Err = Render_Polygon(RD->X, RD->Y, RD->Size, 200, RD->ARGB_Back);
    if(Err._Error)
    {
        return Err;
    }
    if(RD->State)
    {
        Err = Render_Polygon(RD->X, RD->Y, RD->Size*0.5, 200, RD->ARGB_Front);
        if(Err._Error)
        {
            return Err;
        }
    }
    Err._Error=false;
    Err.SDL_State = SDL_APP_CONTINUE;
    return Err;
}
Error Render_Label(SDL_Label *LB)
{
    Error Err = (Error){
        ._Error = true,
        .SDL_State = SDL_APP_FAILURE
    };
    SDL_Color CL = (SDL_Color){
        .a = LB->ARGB.A,
        .r = LB->ARGB.R,
        .g = LB->ARGB.G,
        .b = LB->ARGB.B,
    };
    SDL_Surface *SF = TTF_RenderText_Blended(MainFont,LB->String,0,CL);
    if(SF == NULL)
    {
        Err.Message = "Surface Creation of Label Texture Died\n";
        Err.MessageLength = 40;
        return Err;
    }
    LB->Width = SF->w;
    LB->Height = SF->h;
    LB->Texture = SDL_CreateTextureFromSurface(Render,SF);
    if(LB->Texture == NULL)
    {
        Err.Message = "Texture Creation of Label Texture Died\n";
        Err.MessageLength = 40;
        SDL_DestroySurface(SF);
        return Err;
    }
    SDL_DestroySurface(SF);
    SDL_SetRenderScale(Render, LB->FontSize/(float)24, LB->FontSize/(float)24);
    SDL_RenderTexture(
        Render,
        LB->Texture,
        NULL,
        &(SDL_FRect){
            .x = LB->X,
            .y = LB->Y,
            .w = LB->Width,
            .h = LB->Height
        }
    );
    SDL_SetRenderScale(Render, 1, 1);
    Err._Error = false;
    Err.SDL_State = SDL_APP_CONTINUE;
    return Err;
}
Error Render_TextBox(SDL_Texture *TXT)
{
    Error Err = (Error){
        ._Error = true,
        .SDL_State = SDL_APP_FAILURE
    };
    return Err;
}
Error Render_ObjectsNShapes()
{
    Error Err = (Error){
        ._Error = true,
        .SDL_State = SDL_APP_FAILURE
    };
    //What renders in the back need to come first so what comes on top can overlay
    for(int i = 0; i < SHBack.Indexes; i++)
    {
        switch(*SHBack.SHTYPE)
        {
            case PADDINGSHAPE_T:
                Err.Message = "SHAPE HAS NO TYPE\n";
                Err.MessageLength = 19;
            return Err;
            case SQUARE_T:
                SDL_RenderFillRect(Render, &SHBack.Form[i].Square);
            break;
            case LINE_T:
                SDL_RenderLine(Render,SHBack.Form[i].Line.x1, SHBack.Form[i].Line.y1, SHBack.Form[i].Line.x2, SHBack.Form[i].Line.y2);
            break;
            case POINT_T:
                SDL_RenderPoint(Render, SHBack.Form[i].Point.x, SHBack.Form[i].Point.y);
            break;
        }
    }
    for(int i = 0; i < OBJBack.Indexes; i++)
    {
        switch(*OBJBack.OBJTYPE)
        {
            case PADDINGOBJ_T:
                Err.Message = "OBJECT HAS NO TYPE\n";
                Err.MessageLength = 20;
            return Err;
            case BUTTON_T:
            break;
            case RBUTTON_T:
                Err = SDL_RenderRadioBtn(&OBJBack.Object[i].RadioButton);
                if(Err._Error)
                {
                    return Err;
                }
            break;
            case TEXTBOX_T:
            break;
            case LABEL_T:
                Render_Label(&OBJBack.Object[i].Label);
            break;
        }
    }
    //
    for(int i = 0; i < SHFront.Indexes; i++)
    {
        switch(*SHBack.SHTYPE)
        {
            case PADDINGSHAPE_T:
                Err.Message = "OBJECT HAS NO TYPE\n";
                Err.MessageLength = 19;
            return Err;
            case SQUARE_T:
                SDL_RenderFillRect(Render, &SHFront.Form[i].Square);
            break;
            case LINE_T:
                SDL_RenderLine(Render,SHFront.Form[i].Line.x1, SHFront.Form[i].Line.y1, SHFront.Form[i].Line.x2, SHFront.Form[i].Line.y2);
            break;
            case POINT_T:
                SDL_RenderPoint(Render, SHFront.Form[i].Point.x, SHFront.Form[i].Point.y);
            break;
        }
    }
    for(int i = 0; i < OBJFront.Indexes; i++)
    {
        switch(*OBJFront.OBJTYPE)
        {
            case PADDINGOBJ_T:
                Err.Message = "OBJECT HAS NO TYPE\n";
                Err.MessageLength = 20;
            return Err;
            case BUTTON_T:
            break;
            case RBUTTON_T:
                Err = SDL_RenderRadioBtn(&OBJFront.Object[i].RadioButton);
                if(Err._Error)
                {
                    return Err;
                }
            break;
            case TEXTBOX_T:
            break;
            case LABEL_T:
                Render_Label(&OBJFront.Object[i].Label);
            break;
        }
    }
    Err._Error = false;
    Err.SDL_State = SDL_APP_CONTINUE;
    return Err;
}
//Notes to Self
// Research what the different parameters of a surface are using the SDL Documentation
