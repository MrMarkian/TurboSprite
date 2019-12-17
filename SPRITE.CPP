#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include "graphics.h"
#include <iostream.h>
#include <fstream.h>
#include <string.h>


typedef unsigned char colour_t;

//#define MK_FP(seg,off) \ ((void FAR *)(((long)(seg) << 16)|(unsigned)(off)))

#define WND_WIDTH (unsigned long)320
#define WND_HEIGHT (unsigned long)200
#define SCREEN ((colour_t far *)0xa0000000L)
#define set_pix(x,y,c) SCREEN [y * WND_WIDTH + x]=c
#define ARRAYLEN(ar) (sizeof(ar) / sizeof(ar[0]))

union REGS i,o;
struct SREGS s;

int MouseX,MouseY =0;
double latched,latchedprevious = 0;
//int *sprArray = NULL;

const int SCREENX = 200;
const int SCREENY = 320;

unsigned char far *screen;
unsigned char far *off_screen;
unsigned int screen_size;

	ofstream saveFile;
	ifstream loadFile;

int GridX=0;
int GridY=0;

int colorpallet[10];

class Sprite
{
  private:
  char filename[];


  public:
	int* Data;
    int Height, Width;
    int AlphaColor;


Sprite(int width, int height, int Alpha)
  {
	Height = height;
	Width = width;
	AlphaColor = Alpha;
	Data = new int[width*height];
  }

  void Sprite::LoadSprite (char * filename)
	{	loadFile.open(filename);

		char line[50];
		int value;

        delete [] Data;
		loadFile >> Width;
		GridX = Width;
		loadFile >> Height;
		GridY = Height;
		printf("%i : %i", Width, Height);
        getch();
        Data = new int[Width*Height];

			for (int count = 0 ; count < (Width * Height); count ++)
			{	loadFile >> value;
                Data[count] = value;
			}
			loadFile.close();
	}

	void SaveSprite(char *filename)
	{		saveFile.open(filename);
			saveFile << Width << endl;
			saveFile << Height << endl;

			   for (int count = 0 ; count < (Width * Height); count ++)
			   {
				 saveFile << Data[count] << endl;
			   }

			saveFile.close();
	}

};




void halt();
void DrawPalletBox(int x, int y, int page,int boxsize, int padding,int maxx,int maxy, Sprite ColArr);



class Disp_Controller
{
 #define MOUSE_INTERRUPT 0x51
private:
	unsigned int isActive;
	union REGS in,out;

public:
	void enter_mode13(void)
	{	in.h.ah = 0x00;
		in.h.al = 0x13;
		int86(0x10, &in, &out);
		isActive = 1;
	}

	void leave_mode13(void)
	{   in.h.ah = 0x00;
		in.h.al = 0x02;
		int86(0x10, &in,&out);
		isActive = 0;
	}
/*
	int CreateSecondBuffer(void)
	{
		off_screen=farmalloc(64000u);

		if ( off_screen)
			{
				//screen=MK_FP(0xa000, 0);
				screen_size=64000u;
				_fmemset(off_screen, 0, screen_size);
				return 0;
			}
		else{
		printf("out of mem or other errror");
		return 1;

		}


	}
 */

	//--------mouse section

	int detect_mouse(void)
	 { in.x.ax = 0;
	   int86(MOUSE_INTERRUPT, &in, &out);
	   return out.x.ax;
	 }

	 int show_mouse(void)
	 { in.x.ax = 1;
	   int86(MOUSE_INTERRUPT, &in, &out);
	   return 1;
	 }

	 void get_mouse_status(int *xpos, int *ypos,int *click)
	 { in.x.ax = 3;
	   int86(MOUSE_INTERRUPT, &in, &out);
	   *click = out.x.bx;
	   *xpos = out.x.cx;
	   *ypos = out.x.dx;
	 }

	int hide_mouse(void)
	 { in.x.ax = 2;
	   int86(MOUSE_INTERRUPT, &in, &out);
	   return 1;
	 }

    //--- Graphics
	void DrawLine (int x, int y, int len, int direction, int color)
	{	int a;
		switch (direction)
		{	case 1: //Downwards
				for (a=0; a<len; a++)
				{
						set_pix(x, (y+a), color);
				}
			break;

			case 2:
				for (a=0; a<len; a++)
				{
					set_pix((x+a), y , color);
				}
				break;

			case 3:
				for (a=0; a<len; a++)
				{
					set_pix(x, (y-a), color);
				}
				break;

			case 4:
				for (a=0; a<len; a++)
				{
					set_pix((x-a),y,color);
				}
				break;

			default:
				printf("Error, shouldn't hit here");
				break;
		}
	}

	void RandomFill(int x, int y, int wid, int hei)
	{
		int a,b; // temp vars

		srand(time(NULL));

		for (a=0; a<wid;++a)
		{
			for (b=0; b<hei; ++b)
			{
				set_pix((a+x),(b+y),(colour_t)rand());
			}
		}
	}


	void DrawBox(int x, int y, int wid, int hei, int color)
	{

	  int a,b; // temp vars

		for (a=0; a<wid;++a)
		{
			for (b=0; b<hei; ++b)
			{
				set_pix((a+x), (b+y), color);
			}

		}

	}
	//------ Flood fill the screen with a color
	void FullWipe(int color)
	{
		int a,b;

		for (a=0;a<WND_WIDTH; ++a)
		{
			for (b=0;b<WND_HEIGHT; ++b)
			{
				set_pix(a,b,color);
			}
		}

	}
	//--  Sprite Renderer
	void RenderSprite (Sprite sp, int x, int y)
	{
		int a,b,c=0;

		for (a=0; a < sp.Height; a++)
		{
			for(b=0; b< sp.Width; b++)
			{
			    if (sp.Data[c]!=sp.AlphaColor){
				set_pix((x+b),(y+a),sp.Data[c]);
			    }

				c++;
			}
		}

	}





};


int main(int argc, char *argv[])
{
	char * temp;
	double x,y,a,button,mx,my,answer;

	int b;
	int boxsize,padding;



	char Filename[100];
	Disp_Controller screen;
	srand(time(NULL));
	system("cls");

	screen.show_mouse();

    Sprite NewSprite(0,0,0);

	if (argc == 2) // Commandline options will go here..
	{
        printf("Arguments::%s\n",argv[1]);
        NewSprite.LoadSprite(argv[1]);
        boxsize = 5;
        padding = 2;
	}else {

	printf("Mouse Support:%i\n",screen.detect_mouse());
	cout << "Spriter 1.3" << endl << "By Markian Rutkowskyj" <<  endl << endl << "Keys.." << endl << "I:Enter Color C:Copy P:Paste B:Zoom" << endl << "S:Save L:Load R:Render N:Cell Padding." << endl << endl ;


    cout <<"(L)oad File or New?";
    cin >> temp;



    if (!strcmp(temp, "l"))
    {

        cout << endl << "FileName? :";
        cin >> Filename;
        NewSprite.LoadSprite(Filename);
		GridX = NewSprite.Width;
		GridY = NewSprite.Height;
    }
    else
    {
        cout << "Max X?";
        cin >> GridX;
        cout << "Max Y?";
        cin >> GridY;
            if ((GridX + GridY) < 2)
                {
                GridX = SCREENX;
                GridY = SCREENY;
                }

                    NewSprite.Width = GridX;
                    NewSprite.Height = GridY;

                    delete [] NewSprite.Data;
                    NewSprite.Data = new int[GridX*GridY];


            int color=0;
                for (a=0;a<(GridX*GridY);a++)
                {
                    if (color > 256)
                    {color = 0;}

                NewSprite.Data[a]=color;
                color++;
                }

    }
        cout << "Calculated Grid Size:" << ((double)GridX * (double)GridY);
        cout << endl <<"BoxSize?";
        cin >> boxsize;
        cout << endl << "Padding?";
        cin >> padding;
	}

	screen.enter_mode13();
//	screen.CreateSecondBuffer();
	screen.FullWipe(0);

	unsigned int key=0;
	int offset = 20;
	double temppos,tempcol,CopyPaste;

	// sprArray = new int[GridX,GridY];



	 DrawPalletBox(padding,padding,boxsize,boxsize,padding,GridX,GridY,NewSprite);

	while ((key = getch()) != 27)
	{
	  switch(key)
	  {
		case 109: //clear screen (m)


			DrawPalletBox(padding,padding,boxsize,boxsize,padding,GridX,GridY,NewSprite);
			break;

		 case 102: //f key Fill
			printf("\033[%d;%dH",2,30);
			cout <<"!";
			cin >> tempcol;
			printf("\033[%d;%dH",2,30);
			cout << "         ";

			for(a=0;a<(GridX*GridY);a++)
			{
				NewSprite.Data[a]=tempcol;
				}

			break;

			case 99:
				CopyPaste = NewSprite.Data[latched];
				break;
			case 112:
				NewSprite.Data[latched] = CopyPaste;
				break;

			case 98:
				printf("\033[%d;%dH",2,30);
				cout <<"!";
				cin >> boxsize;
	   //			printf("\033[%d;%dH",2,30);
		 //		cout << "         ";
				screen.FullWipe(0);
				break;

		  case 110:
				printf("\033[%d;%dH",2,30);
				cout <<"!";
				cin >> padding;
		   //		printf("\033[%d;%dH",2,30);
			 //	cout << "         ";
				screen.FullWipe(0);
				break;



		 case 105: // "i" - set one pixel to color
			printf("\033[%d;%dH",2,30);
				cout <<"!";
			cin >> tempcol;
			printf("\033[%d;%dH",2,30);
			cout << "         ";

			NewSprite.Data[latched]=tempcol;
			DrawPalletBox(padding,padding,boxsize,boxsize,padding,GridX,GridY,NewSprite);
			break;
   /*
		 case 108: //"l for insert"

			printf("\033[%d;%dH",0,30);
			cout <<"Select?";
			cin >> latched;
			printf("\033[%d;%dH",0,30);
			cout << "           ";
			break;
   */

		 case 43: // "+" colour up

			NewSprite.Data[latched]++;
			if ((NewSprite.Data[latched] > 255))
				NewSprite.Data[latched] = 255;
			break;

		 case 45: // "-" key colour down
			latchedprevious = latched;
			NewSprite.Data[latched]--;
			if ((NewSprite.Data[latched] < 0 ))
				NewSprite.Data[latched] = 0;

			break;

		 case 72: //upkey
			latchedprevious = latched;
			latched = (latched - GridX);
				if ((latched <0))
					latched =0;
			break;

		 case 80: //down
			latchedprevious = latched;
			if ((latched + GridX) < (GridX * GridY))
			latched =(latched + GridX);
			break;


		 case 75: // left
			latchedprevious = latched;
			latched --;
				if ((latched <0))
				latched =0;
			break;

		 case 77: // right

			latchedprevious = latched;
			if (latched + 1 < (GridX * GridY))
			latched++;

			break;

		case 115: //s save
			printf("\033[%d;%dH",2,30);
			cout <<"SVE!";
			cin >> Filename;
			NewSprite.SaveSprite(Filename);
			printf("\033[%d;%dH",0,30);
			cout << "                                    ";
			break;

		case 108: //l for load
			printf("\033[%d;%dH",2,30);
			cout <<"LOAD?";
			cin >> Filename;
			NewSprite.LoadSprite(Filename);
			printf("\033[%d;%dH",0,30);
			cout << "                                    ";
			screen.FullWipe(0);
			break;

        case 114: // R for Render
            screen.FullWipe(0);
            screen.RenderSprite(NewSprite,10,10);
            getch();
            screen.FullWipe(0);
            break;

            // --- This section is for color pallet - numbers 1 thru 0
        case 33: // Number 1
            colorpallet[1] = NewSprite.Data[latched];
            break;

        case 49:
           NewSprite.Data[latched]  = colorpallet[1];
           break;

        case 34: // Number 2
            colorpallet[2] = NewSprite.Data[latched];
            break;

        case 50:
           NewSprite.Data[latched]  = colorpallet[2];
           break;

        case 156: // Number 3
            colorpallet[3] = NewSprite.Data[latched];
            break;

        case 51:
           NewSprite.Data[latched]  = colorpallet[3];
           break;
        case 36: // Number 4
            colorpallet[4] = NewSprite.Data[latched];
            break;

        case 52:
           NewSprite.Data[latched]  = colorpallet[4];
           break;

        case 37: // Number 5
            colorpallet[5] = NewSprite.Data[latched];
            break;

        case 53:
           NewSprite.Data[latched]  = colorpallet[5];
           break;
        case 94: // Number 6
            colorpallet[6] = NewSprite.Data[latched];
            break;

        case 54:
           NewSprite.Data[latched]  = colorpallet[6];
           break;

        case 38: // Number 7
            colorpallet[7] = NewSprite.Data[latched];
            break;

        case 55:
           NewSprite.Data[latched]  = colorpallet[7];
           break;
        case 42: // Number 8
            colorpallet[8] = NewSprite.Data[latched];
            break;

        case 56:
           NewSprite.Data[latched]  = colorpallet[8];
           break;

        case 41: // Number 9
            colorpallet[9] = NewSprite.Data[latched];
            break;

        case 57:
           NewSprite.Data[latched]  = colorpallet[9];
           break;

        default:
            printf("%i",key);
            break;


	 }

		DrawPalletBox(padding,padding,boxsize,boxsize,padding,GridX,GridY,NewSprite);
		halt();
	}



	screen.leave_mode13();

	delete [] NewSprite.Data;

 return 0;

}

void halt()
{
	asm hlt
}

void DrawPalletBox(int x, int y, int page,int boxsize, int padding, int maxx, int maxy, Sprite ColArr)
{
    //Sprite one,two,three,four,five,six,seven;


	Disp_Controller screen;
	int a,b,c=0;

	for (a=0;a<(boxsize+padding)*maxy;a+=(boxsize+padding))
	{
		for (b=0;b<(boxsize+padding)*maxx;b+=(boxsize+padding))
		{
		  if (padding > 0)
		   {

			if (c == latchedprevious)
			  {
				screen.DrawBox((b+x-(padding/2)),(a+y-(padding/2)),(boxsize+((padding/2)*2)),(boxsize+((padding/2)*2)),0);
			  }

			if (c == latched){
				screen.DrawBox((b+x-(padding/2)),(a+y-(padding/2)),(boxsize+((padding/2)*2)),(boxsize+((padding/2)*2)),15);
				}

		   }

		  screen.DrawBox((b+x),(a+y),boxsize,boxsize,ColArr.Data[c]);
		  c++;



		 }
	}
	printf("\033[%d;%dH",0,30);
	cout << "                ";
	printf("\033[%d;%dH",0,30);
	cout << latched << "["<< ColArr.Data[latched] << "]";


	//Draw pallet grid
	int starty = 20;

	//screen.RenderSprite(one,200,20);


	for (int tempcol =1 ;tempcol < 10 ;tempcol++)
    {
       screen.DrawBox(220,starty,10,10,colorpallet[tempcol]);
       starty +=13;

    }

}

