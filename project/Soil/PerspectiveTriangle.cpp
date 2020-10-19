#include<windows.h>
#include<stdio.h>
#include<GL\gl.h>
#include<GL\Glu.h> // graphic library utility
#include<time.h>
#include<stdio.h>
#include<math.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Glu32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define M_PI 3.14159292035
#define ROT_SPEED 0.8f

// global fuctions declaration
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

// global variables declaration
DWORD grdwStyle;
WINDOWPLACEMENT grgwpPrev = { sizeof(WINDOWPLACEMENT) };
bool grgbFullScreen = false;
HWND grghwnd = NULL;
bool grgbActiveWindow = false;
HDC grghdc = NULL;
HGLRC grghrc = NULL;
FILE* grgpFile = NULL;

// hut (deep)
void DL_DrawHut();
void DL_InitializePotGeometry();
void DL_DrawPot(GLfloat, GLint, GLint, GLfloat*, GLfloat*);
void DL_AnimatePot();

//tree(tejswini)
void TN_ThreeDCubeStem(GLfloat, GLfloat, GLfloat);
void TN_PyramidTree(GLfloat);
void TN_Circle();
void DrawTree();

bool gbAnim = false;
GLfloat trans = 0.0f;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////// Code To Copy Start ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Constants For Defining Size Of Pot
#define DL_STACKS 40
#define DL_SLICES 40

//Constant For Normalizing Shaping of Pot
#define DL_GROW_SPEED 1000.0f

//Current radii of cylinder
//Outer
GLfloat dl_pRadiiOuter[DL_STACKS];
//Inner
GLfloat dl_pRadiiInner[DL_STACKS];

//Final radii of cylinder
//Outer
GLfloat dl_pFinalRadiiOuter[DL_STACKS];
//Inner
GLfloat dl_pFinalRadiiInner[DL_STACKS];

//Normalized values for radii growth
GLfloat dl_pRadiiGrowth[DL_STACKS];

//Color Vertex Outer
GLfloat dl_pColorsOuter[DL_STACKS * DL_SLICES * 3];

//Color Vertex Outer
GLfloat dl_pColorsInner[DL_STACKS * DL_SLICES * 3];



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// Function declaration
	void Initialize(void);
	void Display(void);

	// variables declaration
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("OGL");
	int grDesktopWidth, grDesktopHeight;
	int grWndXPos, grWndYPos;
	bool grbDone = false;

	if (fopen_s(&grgpFile, "GRLog.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Cannot open desired file"), TEXT("Error"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	else
	{
		fprintf(grgpFile, "Log file created successfully. \n Program started successfully\n **** Logs ***** \n");
	}

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, IDC_ARROW);

	RegisterClassEx(&wndclass);

	// Get width and height of desktop screen
	grDesktopWidth = GetSystemMetrics(SM_CXSCREEN);
	grDesktopHeight = GetSystemMetrics(SM_CYSCREEN);

	// Get center horizontal point
	grDesktopWidth = grDesktopWidth / 2;
	// Get center vertical point
	grDesktopHeight = grDesktopHeight / 2;

	// X position = center horizontal coordinate of screen - center horizontal coordinate of window
	grWndXPos = grDesktopWidth - 400;

	// X position = center horizontal coordinate of screen - center horizontal coordinate of window
	grWndYPos = grDesktopHeight - 300;


	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("OpenGL"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		grWndXPos,
		grWndYPos,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	grghwnd = hwnd;

	Initialize();

	ShowWindow(hwnd, iCmdShow);

	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	while (grbDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				grbDone = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (grgbActiveWindow == true)
			{
				//update function

				//display function
				Display();
			}
		}

	}

	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function declaration
	void ToggleFullScreen(void);
	void Resize(int, int);
	void Uninitialize(void);

	switch (iMsg)
	{
	case WM_SETFOCUS:
		grgbActiveWindow = true;
		break;

	case WM_KILLFOCUS:
		grgbActiveWindow = false;
		break;

	case WM_ERASEBKGND:
		return(0);


	case WM_SIZE:
		Resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 0x46:
		case 0x66:
			ToggleFullScreen();
			break;

		default:
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		Uninitialize();
		PostQuitMessage(0);
		break;

	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen()
{
	MONITORINFO mi = { sizeof(MONITORINFO) };

	if (grgbFullScreen == false)
	{
		grdwStyle = GetWindowLong(grghwnd, GWL_STYLE);
		if (grdwStyle & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(grghwnd, &grgwpPrev) && GetMonitorInfo(MonitorFromWindow(grghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(grghwnd, GWL_STYLE, (grdwStyle & ~WS_OVERLAPPEDWINDOW));
				SetWindowPos(grghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
					(mi.rcMonitor.right - mi.rcMonitor.left), (mi.rcMonitor.bottom - mi.rcMonitor.top), SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(false);
		grgbFullScreen = true;
	}
	else
	{
		SetWindowLong(grghwnd, GWL_STYLE, (grdwStyle | WS_OVERLAPPEDWINDOW));
		SetWindowPlacement(grghwnd, &grgwpPrev);
		SetWindowPos(grghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(true);
		grgbFullScreen = false;
	}
}

void Initialize()
{
	// function declaration
	void Resize(int, int);

	//variable declarations
	PIXELFORMATDESCRIPTOR grpfd;
	int griPixelFormatIndex;

	//code
	grghdc = GetDC(grghwnd);

	ZeroMemory(&grpfd, sizeof(PIXELFORMATDESCRIPTOR));
	grpfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	grpfd.nVersion = 1;
	grpfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	grpfd.iPixelType = PFD_TYPE_RGBA;
	grpfd.cColorBits = 32;
	grpfd.cRedBits = 8;
	grpfd.cGreenBits = 8;
	grpfd.cBlueBits = 8;
	grpfd.cAlphaBits = 8;
	grpfd.cDepthBits = 32;

	griPixelFormatIndex = ChoosePixelFormat(grghdc, &grpfd);
	if (griPixelFormatIndex == 0)
	{
		fprintf(grgpFile, "ChoosePixelFormat() failed\n");
		DestroyWindow(grghwnd);
	}

	if (SetPixelFormat(grghdc, griPixelFormatIndex, &grpfd) == FALSE)
	{
		fprintf(grgpFile, "SetPixelFormat() failed\n");
		DestroyWindow(grghwnd);
	}

	grghrc = wglCreateContext(grghdc);
	if (grghrc == NULL)
	{
		fprintf(grgpFile, "wglCreateContext() failed\n");
		DestroyWindow(grghwnd);
	}

	if (wglMakeCurrent(grghdc, grghrc) == FALSE)
	{
		fprintf(grgpFile, "wglMakeCurrent() failed\n");
		DestroyWindow(grghwnd);
	}
	DL_InitializePotGeometry();
	// set clearcolor
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	// warm-up call to resize
	Resize(WIN_WIDTH, WIN_HEIGHT);
}

void Resize(int width, int height)
{
	if (height == 0)
		height = 1;

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

}

void Display(void)
{
	void DisplaySoil();
	void drawBlackBoardAndStand();
	void Draw_grass(GLfloat, GLfloat);
	void BirdsAnimation();
	void DrawPartialPar();
	void DisplaySky();
	void BirdsAnimation();

	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

#pragma region region Gauri display
	DisplaySky();
	DisplaySoil();
#pragma endregion Gauri display

#pragma region region Deep Display

	static GLfloat rot = 0.0f;

	/*
	glLoadIdentity();
	glTranslatef(1.0f, 0.0f, -6.0f);
	glScalef(1.5f, 1.5f, 1.5f);
	DL_DrawHut();
	*/
	/*
	glLoadIdentity();
	glTranslatef(-1.0f, -0.5f, -8.0f);
	glScalef(0.5f, 0.5f, 0.5f);
	DL_DrawHut();
	*/
	// pot
	glLoadIdentity();

	glTranslatef(2.0f, -1.5f, -8.0f);


	glRotatef(rot, 0.0f, 1.0f, 0.0f);

	DL_DrawPot(1.5f, DL_STACKS, DL_SLICES, dl_pRadiiOuter, dl_pColorsOuter);
	DL_DrawPot(1.5f, DL_STACKS, DL_SLICES, dl_pRadiiInner, dl_pColorsInner);

	if (gbAnim) {
		rot += ROT_SPEED;
		if (rot >= 360) {
			rot -= 360.0f;
		}
	}

	DL_AnimatePot();


#pragma endregion Deep Display

#pragma region region Pratik Display
	drawBlackBoardAndStand();
	DrawPartialPar();
#pragma endregion Pratik Display

#pragma region region Mehesh Display 
	Draw_grass(1.0f, -7.0f);
	Draw_grass(1.0f, -6.0f);
#pragma endregion Mahesh Display

#pragma region region Akhilesh display
	glLoadIdentity();

	BirdsAnimation();

#pragma endregion Akhilesh display

#pragma region region Tejswini display

	DrawTree();

#pragma endregion Tejswini display


	SwapBuffers(grghdc);
}

#pragma region region Gauri code

void DisplaySky()
{
	GLfloat gri;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 1.0f, -17.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.8f, 0.0f);			// orange-ish color
	glVertex3f(25.0f, -2.7f, 0.0f);			
	glColor3f(0.5290f, 0.8080f, 0.9220f);
	glVertex3f(-25.0f, -2.7f, 0.0f);
	glColor3f(0.5290f, 0.8080f, 0.9220f);
	glVertex3f(-25.0f, 6.0f, 0.0f);
	glVertex3f(25.0f, 6.0f, 0.0f);
	glEnd();

	/*
	// sun
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(2.0f, -1.6f, -17.0f);
	glScalef(4.5f, 0.5f, 1.0f);

	glBegin(GL_TRIANGLE_STRIP);
	for (gri = 0.0f; gri <= (3.142 * 1.0f); gri = gri + 0.01f)
	{
		glColor3f(0.8f, 0.0f, 0.0f);			// red (sun center)
		glVertex3f(0.0f, 0.0f, 0.0f);
		glColor3f(1.0f, 0.8f, 0.0f);			// orange-ish color
		glVertex3f(1.0f * cos(gri), 1.0f * sin(gri), 0.0f);
	}
	glEnd();
	*/
}

void DisplaySoil()
{
	glLoadIdentity();


	// Floor
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, -3.0f, -15.0f);
	//glColor3f(0.910f, 0.7140f, 0.310f);
	glColor3f(0.46270f, 0.36570f, 0.17160f); // dark brown
	glBegin(GL_QUADS);
	glVertex3f(25.0f, 0.0f, 0.0f);
	glVertex3f(-25.0f, 0.0f, 0.0f);
	glVertex3f(-25.0f, 0.0f, 15.0f);
	glVertex3f(25.0f, 0.0f, 15.0f);
	glEnd();

	// working code 1 for soil
	for (GLfloat i = 28.0f; i >= -30.0f; i = i - 0.5f)
	{

		for (GLfloat j = 15.0f; j >= -15.0f; j = j - 1.0f)
		{
			glBegin(GL_TRIANGLES);
			glColor3f(0.46270f, 0.36570f, 0.17160f); // dark brown
			//glColor3f(0.0f, 0.7140f, 0.310f);
			glVertex3f(i, 0.0f, j - 2.0f);
			glColor3f(0.38610f, 0.33660f, 0.27720f); // darkest brown
			//glColor3f(0.0f, 0.7140f, 0.310f);
			glVertex3f(i, 0.0f, j);
			glColor3f(0.570f, 0.4080f, 0.1610f); // brown
			//glColor3f(0.0f, 0.7140f, 0.310f);
			glVertex3f(i + 2.5f, 0.0f, j);
			glEnd();

		}

	}

}

#pragma endregion Gauri code

#pragma region region Tejswini code
void DrawTree()
{

	static GLfloat tn_trans = 1.9f;
	//stem
	glLoadIdentity();
	glTranslatef(-2.0f - tn_trans, -0.7f, -5.5f);
	TN_ThreeDCubeStem(0.11f, 1.9f, 0.09f); //stem


	glLoadIdentity();
	glTranslatef(-1.3f - tn_trans, 0.0f, -5.5f);
	glRotatef(270, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.2f - tn_trans, 0.2f, -5.5f);
	glRotatef(280, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.1f - tn_trans, 0.4f, -5.5f);
	glRotatef(270, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.0f - tn_trans, 0.6f, -5.5f);
	glRotatef(300, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.0f - tn_trans, 0.8f, -5.5f);
	glRotatef(310, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.0f - tn_trans, 1.0f, -5.5f);
	glRotatef(320, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.1f - tn_trans, 1.2f, -5.5f);
	glRotatef(330, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.2f - tn_trans, 1.4f, -5.5f);
	glRotatef(340, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.3f - tn_trans, 1.5f, -5.5f);
	glRotatef(350, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.4f - tn_trans, 1.6f, -5.5f);
	glRotatef(360, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.5f - tn_trans, 1.7f, -5.5f);
	glRotatef(370, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.6f - tn_trans, 1.8f, -5.5f);
	glRotatef(380, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.7f - tn_trans, 1.8f, -5.5f);
	glRotatef(390, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.8f - tn_trans, 1.9f, -5.5f);
	glRotatef(390, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);

	glLoadIdentity();
	glTranslatef(-1.9f - tn_trans, 2.0f, -5.5f);
	glRotatef(390, 0.0f, 0.0f, 1.0f);
	glScalef(0.7f, 0.7f, 0.7f);
	TN_PyramidTree(0.6f);


	glLoadIdentity();
	glTranslatef(-3.4f, 0.8f, -5.5f);
	TN_Circle();

}

void TN_PyramidTree(GLfloat dim) {

	glBegin(GL_TRIANGLES);


	//front
	//glColor3f(1.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 0.3921f, 0.0f);
	glVertex3f(0.0f, dim, 0.0f);
	glColor3f(0.0f, 0.3921f, 0.0f);
	//glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-dim, -dim, dim);
	glColor3f(0.0f, 0.3921f, 0.0f);
	//glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(dim, -dim, dim);


	//right
	//glColor3f(0.0f, 1.0f, 0.0f);
	//glColor3f(1.0f, 0.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, dim, 0.0f);
	//glColor3f(0.0f, 0.0f, 1.0f);  //b
	glColor3f(0.0f, 0.3921f, 0.0f);
	glVertex3f(dim, -dim, dim);
	//glColor3f(0.0f, 1.0f, 0.0f); //g
	glColor3f(0.0f, 0.3921f, 0.0f);
	glVertex3f(dim, -dim, -dim);



	//back
	//glColor3f(1.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 0.3921f, 0.0f);
	glVertex3f(0.0f, dim, 0.0f);
	//glColor3f(0.0f, 0.0f, 1.0f);  //b
	glColor3f(0.0f, 0.3921f, 0.0f);
	glVertex3f(-dim, -dim, -dim);
	//glColor3f(0.0f, 1.0f, 0.0f); //g
	glColor3f(0.0f, 0.3921f, 0.0f);
	glVertex3f(dim, -dim, -dim);


	//left
	//glColor3f(1.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 0.3921f, 0.0f);
	glVertex3f(0.0f, dim, 0.0f);
	//glColor3f(0.0f, 1.0f, 0.0f); //g
	glColor3f(0.0f, 0.3921f, 0.0f);
	glVertex3f(-dim, -dim, dim);
	//glColor3f(0.0f, 0.0f, 1.0f); //b
	glColor3f(0.0f, 0.3921f, 0.0f);
	glVertex3f(-dim, -dim, -dim);

	glEnd();

	//bottom
	glBegin(GL_QUADS);
	//glColor3f(1.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 0.3921f, 0.0f);
	glVertex3f(dim, -dim, -dim);
	//glColor3f(0.0f, 1.0f, 0.0f);
	glColor3f(0.0f, 0.3921f, 0.0f);
	//glVertex3f(-dim, -dim, -dim);
	glColor3f(0.0f, 0.3921f, 0.0f);
	//glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-dim, -dim, dim);
	//glColor3f(1.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 0.3921f, 0.0f);
	glVertex3f(dim, -dim, dim);

	glEnd();


}



void TN_Circle()
{
	GLdouble angle = 0.0f;
	GLfloat radius = 1.03f;

	glBegin(GL_LINE_LOOP);
	for (angle = 0.0f; angle < 2 * 3.14; angle = angle + 0.001f)
	{
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);

		glColor3f(0.0f, 0.3921f, 0.0f);
		glVertex3f(cos(angle) * radius, sin(angle) * radius, 1.0f);
	}

	glEnd();

}

void TN_ThreeDCubeStem(GLfloat x, GLfloat y, GLfloat z) { //stem

	glBegin(GL_QUADS);

	//front
	glColor3f(0.4196f, 0.2666f, 0.1372f);
	glVertex3f(x, y, z);
	glVertex3f(-x, y, z);
	glColor3f(0.4823f, 0.2470f, 0.0f);
	glVertex3f(-x, -y, z);
	glVertex3f(x, -y, z);

	//right
	glColor3f(0.4823f, 0.2470f, 0.0f);
	glVertex3f(x, y, -z);
	glVertex3f(x, y, z);
	glColor3f(0.4196f, 0.2666f, 0.1372f);
	glVertex3f(x, -y, z);
	glVertex3f(x, -y, -z);

	//back
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(x, y, -z);
	glVertex3f(-x, y, -z);
	glVertex3f(-x, -y, -z);
	glVertex3f(x, -y, -z);

	//cyan
	glColor3f(0.0f, 1.0f, 1.0f);
	glVertex3f(-x, y, -z);
	glVertex3f(-x, y, z);
	glVertex3f(-x, -y, z);
	glVertex3f(-x, -y, -z);

	//top
	glColor3f(0.4196f, 0.2666f, 0.1372f);
	glVertex3f(x, y, -z);
	glVertex3f(-x, y, -z);
	glVertex3f(-x, y, z);
	glVertex3f(x, y, z);

	//bottom
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(x, -y, -z);
	glVertex3f(-x, -y, -z);
	glVertex3f(-x, -y, z);
	glVertex3f(x, -y, z);

	glEnd();
}
#pragma endregion Tejswini code

#pragma region region Deep code
void DL_DrawHut() {
	//Randomly Generated End Points for Roof
	static GLfloat gfRoofH[] = {
	0.110150, 0.158661, 0.227001,
	0.190058, 0.283568, 0.153811,
	0.261100, 0.180799, 0.128181,
	0.139362, 0.317705, 0.132817,
	0.111781, 0.216222, 0.230252,
	0.152137, 0.210158, 0.121308,
	0.276663, 0.212977, 0.295195,
	0.311180, 0.217871, 0.211316,
	0.291338, 0.275921, 0.319259,
	0.231200, 0.172132, 0.288019,
	0.195172, 0.255331, 0.111758,
	0.175177, 0.227581, 0.287511,
	0.216988, 0.268747, 0.211591,
	0.268888, 0.229810, 0.267001,
	};

	//Body of Hut
	glBegin(GL_QUADS);
	glColor3f(0.705f, 0.431f, 0.0f);
	glVertex3f(0.3f, 0.3f, 0.0f);
	glVertex3f(-0.3f, 0.3f, 0.0f);
	glColor3f(0.391f, 0.201f, 0.0f);
	glVertex3f(-0.3f, -0.3f, 0.0f);
	glVertex3f(0.3f, -0.3f, 0.0f);
	glEnd();

	//Color Pallete for Roof
	GLfloat color_arr[][3] = {
	{1.0f, 0.68f, 0.0f},
	{0.509f, 0.372f, 0.078f},
	{0.7f, 0.45f, 0.01f}
	};

	//Roof Of Hut
	glLineWidth(4.0f);
	glBegin(GL_LINES);
	GLfloat x = -0.4f;
	GLfloat increment = 0.005f;
	for (int i = 0; i < 160; i++, x += increment) {
		glColor3fv(color_arr[i % 3]);
		glVertex3f(0.0f, 0.7f, 0.03f);
		glVertex3f(x, gfRoofH[i % 42], 0.01f);
	}
	glEnd();

	//Draw Warli 1
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.2f, -0.06f, 0.01f);
	for (int i = 0; i <= 40; i++) {
		GLfloat angle = (2 * 3.145 * i) / 40.0f;
		glVertex3f(sin(angle) * 0.02f + 0.2f, cos(angle) * 0.02f - 0.06f, 0.01f);
	}
	glEnd();
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glVertex3f(0.2f, -0.06f, 0.01f);
	glVertex3f(0.195f, -0.1f, 0.01f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(0.25f, -0.09f, 0.01f);
	glVertex3f(0.14f, -0.1f, 0.01f);
	glVertex3f(0.2f, -0.15f, 0.01f);

	glVertex3f(0.2f, -0.15f, 0.01f);
	glVertex3f(0.15f, -0.21f, 0.01f);
	glVertex3f(0.25f, -0.2f, 0.01f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(0.25f, -0.09f, 0.01f);
	glVertex3f(0.28f, -0.1f, 0.01f);
	glVertex3f(0.27f, -0.15f, 0.01f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(0.14f, -0.1f, 0.01f);
	glVertex3f(0.11f, -0.08f, 0.01f);
	glVertex3f(0.13f, -0.03f, 0.01f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(0.185f, -0.2f, 0.01f);
	glVertex3f(0.183f, -0.25f, 0.01f);
	glVertex3f(0.17f, -0.251f, 0.01f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(0.215f, -0.2f, 0.01f);
	glVertex3f(0.213f, -0.26f, 0.01f);
	glVertex3f(0.2f, -0.261f, 0.01f);
	glEnd();


	//Draw Warli 2
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, -0.06f, 0.01f);
	for (int i = 0; i <= 40; i++) {
		GLfloat angle = 2 * M_PI * i / 40.0f;
		glVertex3f(sin(angle) * 0.02f + 0.0f, cos(angle) * 0.02f - 0.06f, 0.01f);
	}
	glEnd();
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, -0.06f, 0.01f);
	glVertex3f(0.0f, -0.1f, 0.01f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(0.05f, -0.1f, 0.01f);
	glVertex3f(-0.05f, -0.1f, 0.01f);
	glVertex3f(0.0f, -0.15f, 0.01f);

	glVertex3f(0.0f, -0.15f, 0.01f);
	glVertex3f(-0.05f, -0.2f, 0.01f);
	glVertex3f(0.05f, -0.2f, 0.01f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(0.05f, -0.1f, 0.01f);
	glVertex3f(0.09f, -0.09f, 0.01f);
	glVertex3f(0.07f, -0.03f, 0.01f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(-0.05f, -0.1f, 0.01f);
	glVertex3f(-0.09f, -0.11f, 0.01f);
	glVertex3f(-0.07f, -0.16f, 0.01f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(0.02f, -0.2f, 0.01f);
	glVertex3f(0.02f, -0.25f, 0.01f);
	glVertex3f(0.029f, -0.25f, 0.01f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(-0.02f, -0.2f, 0.01f);
	glVertex3f(-0.02f, -0.25f, 0.01f);
	glVertex3f(-0.029f, -0.25f, 0.01f);
	glEnd();

	//Draw Warli 3
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(-0.2f, -0.06f, 0.01f);
	for (int i = 0; i <= 40; i++) {
		GLfloat angle = 2 * M_PI * i / 40.0f;
		glVertex3f(sin(angle) * 0.02f - 0.2f, cos(angle) * 0.02f - 0.06f, 0.01f);
	}
	glEnd();
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glVertex3f(-0.2f, -0.06f, 0.01f);
	glVertex3f(-0.21f, -0.1f, 0.01f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(-0.16f, -0.1f, 0.01f);
	glVertex3f(-0.26f, -0.08f, 0.01f);
	glVertex3f(-0.21f, -0.14f, 0.01f);

	glVertex3f(-0.21f, -0.14f, 0.01f);
	glVertex3f(-0.25f, -0.19f, 0.01f);
	glVertex3f(-0.17f, -0.2f, 0.01f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(-0.25f, -0.08f, 0.01f);
	glVertex3f(-0.28f, -0.12f, 0.01f);
	glVertex3f(-0.25f, -0.16f, 0.01f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(-0.16f, -0.1f, 0.01f);
	glVertex3f(-0.12f, -0.08f, 0.01f);
	glVertex3f(-0.13f, -0.03f, 0.01f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(-0.2f, -0.2f, 0.01f);
	glVertex3f(-0.21f, -0.25f, 0.01f);
	glVertex3f(-0.198f, -0.25f, 0.01f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(-0.23f, -0.19f, 0.01f);
	glVertex3f(-0.24f, -0.25f, 0.01f);
	glVertex3f(-0.23f, -0.25f, 0.01f);
	glEnd();
}
void DL_AnimatePot() {
	//Copy Call From Update or call in Update wihtout any parameters

	//Static Counnter for each of sir's rules
	static int count1 = 0;
	static int count2 = 0;
	static int count3 = 0;
	static int count4 = 0;
	static int count5 = 0;

	//Animation for each of sir's rules
	if (count1 < DL_GROW_SPEED) {
		for (int i = 0; i < 10; i++) {
			dl_pRadiiOuter[i] += dl_pRadiiGrowth[i];
			dl_pRadiiInner[i] += dl_pRadiiGrowth[i];
		}
		count1++;
	}
	else if (count2 < DL_GROW_SPEED) {
		for (int i = 20; i < 40; i++) {
			dl_pRadiiOuter[i] += dl_pRadiiGrowth[i];
			dl_pRadiiInner[i] += dl_pRadiiGrowth[i];
		}
		count2++;
	}
	else if (count3 < DL_GROW_SPEED) {
		for (int i = 10; i < 20; i++) {
			dl_pRadiiOuter[i] += dl_pRadiiGrowth[i];
			dl_pRadiiInner[i] += dl_pRadiiGrowth[i];
		}
		count3++;
	}
}
void DL_DrawPot(GLfloat height, GLint stacks, GLint slices, GLfloat* radii, GLfloat* colors) {
	//Copy Call From Display

	//Calculate Current Postion of Y
	GLfloat fCurrentY = height / 2;

	//Calculate normalized interval for incrementing Y
	const GLfloat fYInterval = height / stacks;

	//Vertex Array to store generated points
	GLfloat* points = (GLfloat*)malloc(sizeof(GLfloat) * stacks * slices * 3);

	//Loop to generate points for each stack
	for (int i = 0; i < stacks; i++) {
		//Loop to generate points for each slice in each stack
		for (int j = 0; j < slices; j++) {
			//Calculate angle for obtaining point on circle
			GLfloat angle = 2 * M_PI * j / slices;

			//Saving Generated Co-oridnates in Vertex Array Suitable Format
			//points[i][j][0]
			points[i * slices * 3 + j * 3 + 0] = cos(angle) * radii[i];
			//points[i][j][1]
			points[i * slices * 3 + j * 3 + 1] = fCurrentY;
			//points[i][j][2]
			points[i * slices * 3 + j * 3 + 2] = sin(angle) * radii[i];
		}
		//Decrementing Y for next Stack Layer
		fCurrentY -= fYInterval;
	}

	//Calculating Number of points required to draw geometry
	const GLint count = (slices + 1) * 2;

	//Vertex List to Draw Generated Vertices in correct order
	GLuint* geometry = (GLuint*)malloc(sizeof(GLuint) * count);

	//Enabling Vertex and Color Arrays for faster execution
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	//Setting VertexPointer and ColorPointer to respective arrays calculated earlier in this function and InitializePotGeomerty()
	glVertexPointer(3, GL_FLOAT, 0, points);
	glColorPointer(3, GL_FLOAT, 0, colors);

	//Loop to Draw a single Strip or Stack of Pot
	for (int i = 0; i < stacks - 1; i++) {
		//Loop to arrange vertices in correct order
		for (int j = 0; j < slices; j++) {
			//Setting Upper Vertex
			geometry[j * 2] = i * slices + j;
			//Setting Lower Vertex
			geometry[j * 2 + 1] = (i + 1) * slices + j;
		}
		//Setting Last Vertices same as the first to complete strip
		geometry[slices * 2] = geometry[0];
		geometry[slices * 2 + 1] = geometry[1];

		//Drawing a single strip as defined by above vertex array and order as above geometry array
		glDrawElements(GL_QUAD_STRIP, count, GL_UNSIGNED_INT, geometry);
	}

	//Freeing dynamically allocated memory
	free(points);
	free(geometry);

	//Disabling Vertex and Color Arrays for future drawing
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}
void DL_InitializePotGeometry() {
	//Copy Call From Initialize or call in Initialize wihtout any parameters

	//Color Pallete for Outside of the Pot
	GLfloat colors[][3] = {
	{0.808f, 0.457f, 0.32f},
	{0.838f, 0.487f, 0.35f},
	};

	//Color Pallete for Inside of the Pot
	GLfloat colors_s[][3] = {
	{0.608f, 0.257f, 0.12f},
	{0.638f, 0.287f, 0.15f},
	};

	//Filling Colors for Outside of Pot
	for (int i = 0; i < DL_STACKS * DL_SLICES; i++) {
		int r = rand() % 2;
		dl_pColorsOuter[i * 3] = colors[r][0];
		dl_pColorsOuter[i * 3 + 1] = colors[r][1];
		dl_pColorsOuter[i * 3 + 2] = colors[r][2];
	}

	//Filling Colors for Inside of Pot
	for (int i = 0; i < DL_STACKS * DL_SLICES; i++) {
		int r = rand() % 2;
		dl_pColorsInner[i * 3] = colors_s[r][0];
		dl_pColorsInner[i * 3 + 1] = colors_s[r][1];
		dl_pColorsInner[i * 3 + 2] = colors_s[r][2];
	}

	//Generating Co-ordinates for initial shape of Pot
	int  l = 1;

	dl_pRadiiOuter[0] = 0.01f;
	dl_pRadiiInner[0] = 0.0f;

	for (int i = 2; i < 8; i++) {
		GLfloat angle = 2.0f * M_PI * i / 30.0f;
		dl_pRadiiOuter[l] = sin(angle) * 0.55f;
		dl_pRadiiInner[l] = dl_pRadiiOuter[l] - 0.01f;
		l++;
	}

	for (; l < DL_STACKS; l++) {
		dl_pRadiiOuter[l] = 0.5f + l / 100.0f;
		dl_pRadiiInner[l] = dl_pRadiiOuter[l] - 0.01f;
	}

	//Generating Co-ordinates for final shape of Pot
	l = 0;

	for (int i = 0; i < 4; i++, l++) {
		GLfloat ang = 2.0f * M_PI * i / 20.0f;
		dl_pFinalRadiiOuter[l] = cos(ang) * 0.5f;
		dl_pFinalRadiiInner[l] = dl_pFinalRadiiOuter[l] - 0.01f;
	}

	for (int i = 5; i < 20; i++, l++) {
		GLfloat ang = 2.0f * M_PI * i / 80.0f;
		dl_pFinalRadiiOuter[l] = sin(ang) * 0.8f;
		dl_pFinalRadiiInner[l] = dl_pFinalRadiiOuter[l] - 0.01f;
	}

	for (int i = 28; i < 48; i++, l++) {
		GLfloat ang = 2.0f * M_PI * i / 115.0f;
		dl_pFinalRadiiOuter[l] = sin(ang) * 0.8f;
		dl_pFinalRadiiInner[l] = dl_pFinalRadiiOuter[l] - 0.01f;
	}

	//Genrating Normalized Speed for Growth
	for (int i = 0; i < DL_STACKS; i++) {
		dl_pRadiiGrowth[i] = (dl_pFinalRadiiOuter[i] - dl_pRadiiOuter[i]) / DL_GROW_SPEED;
	}
}

#pragma endregion Deep code

#pragma region Pratik code
void drawBlackBoardAndStand() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.5f, 0.0f, -9.0f);
	glBegin(GL_QUADS);

	GLfloat xBackFront = 1.8f;
	GLfloat xSides = 1.81f;
	GLfloat yBackFront = 1.0f;
	GLfloat ySides = 1.01f;
	GLfloat zVal = 0.1f;


	// Black-board
	// front - all z-positive
	glColor3f(0.0f, 0.0f, 0.0f); //B
	glVertex3f(xBackFront, yBackFront, zVal); // right
	glVertex3f(-xBackFront, yBackFront, zVal); // left
	glVertex3f(-xBackFront, -yBackFront, zVal); // left bottom
	glVertex3f(xBackFront, -yBackFront, zVal); //right bottom

	// right
	glColor3f(1.0f, 1.0f, 1.0f); //W
	glVertex3f(xSides, ySides, -zVal); // right
	glVertex3f(xSides, ySides, zVal); // left
	glVertex3f(xSides, -ySides, zVal); // left bottom
	glVertex3f(xSides, -ySides, -zVal); //right bottom

	//// back // not required
	//glColor3f(0.0f, 0.0f, 0.0f); //B
	//glVertex3f(-xBackFront, yBackFront, -zVal); // right
	//glVertex3f(xBackFront, yBackFront, -zVal); // left
	//glVertex3f(xBackFront, -yBackFront, -zVal); // left bottom
	//glVertex3f(-xBackFront, -yBackFront, -zVal); //right bottom

	// left
	glColor3f(1.0f, 1.0f, 1.0f); //W
	glVertex3f(-xSides, ySides, zVal); // right
	glVertex3f(-xSides, ySides, -zVal); // left
	glVertex3f(-xSides, -ySides, -zVal); // left bottom
	glVertex3f(-xSides, -ySides, zVal); //right bottom

	// top
	glColor3f(1.0f, 1.0f, 1.0f); //M
	glVertex3f(xSides, ySides, -zVal); // right
	glVertex3f(-xSides, ySides, -zVal); // left
	glVertex3f(-xSides, ySides, zVal); // left bottom
	glVertex3f(xSides, ySides, zVal); //right bottom

	// bottom
	glColor3f(1.0f, 1.0f, 1.0f); //Y
	glVertex3f(xSides, -ySides, -zVal); // right
	glVertex3f(-xSides, -ySides, -zVal); // left
	glVertex3f(-xSides, -ySides, zVal); // left bottom
	glVertex3f(xSides, -ySides, zVal); //right bottom


	glEnd();

	glLoadIdentity();
	glTranslatef(-1.8f, 0.0f, -9.3f);

	glBegin(GL_QUADS);
	// Black-board Stand
	glColor3f(0.90f, 0.40f, 0.0f);
	glVertex3f(0.5f, 1.5f, 0.0f); // right
	glVertex3f(0.4f, 1.5f, 0.0f); // left
	glVertex3f(-1.0f, -2.5f, 0.0f); // left bottom
	glVertex3f(-0.9f, -2.5f, 0.0f); //right bottom

	glVertex3f(-0.5f, 1.5f, 0.0f); // right
	glVertex3f(-0.4f, 1.5f, 0.0f); // left
	glVertex3f(1.0f, -2.5f, 0.0f); // left bottom
	glVertex3f(0.9f, -2.5f, 0.0f); //right bottom
	glEnd();

}

// par
void DrawPartialPar()
{
	// variables
	GLUquadric* pQuadric = NULL;
	GLfloat rotateDisk = 125.0f;
	GLfloat rotate = 25.0f;
	GLfloat translateY = -1.3f;

	void drawVerticalLinesOnPar(GLfloat x, GLfloat y1, GLfloat y2);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-4.0f, translateY, -5.5f);
	glRotatef(rotate, 0.0f, 1.0f, 0.0f);
	glRotatef(rotateDisk, 1.0f, 0.0f, 0.0f);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.8f, 0.8f, 0.8f); //light grey
	pQuadric = gluNewQuadric();
	gluPartialDisk(pQuadric,
		0.25, // inner radius
		1.0, // outer radius
		20, // slices
		10,
		0,
		180); // rings

	// var ty controls height of the par
	for (float ty = translateY; ty > -1.8f; ty = ty - 0.01f) {
		if ((int)(ty * 100) % 10 == 0)
		{
			glColor3f(1.0f, 1.0f, 1.0f); //white
		}
		else
		{
			glColor3f(0.5f, 0.5f, 0.5f); //grey
		}
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(-4.0f, ty, -5.5f);
		glRotatef(rotate, 0.0f, 1.0f, 0.0f);
		glRotatef(rotateDisk, 1.0f, 0.0f, 0.0f);
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		pQuadric = gluNewQuadric();
		gluPartialDisk(pQuadric,
			0.25, // inner radius
			1.0, // outer radius
			10, // slices
			10,
			0,
			180); // rings 
	}

	// Vertical Lines
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.3f, -0.3f, -1.5f);
	glRotatef(45.0f, 0.0f, 1.0f, 0.0f); // Mind this
	glBegin(GL_LINES);

	glColor3f(1.1f, 1.0f, 1.0f); //white

	GLfloat x = 0.05f;
	GLfloat y1 = -0.15f;
	GLfloat y2 = -0.45f;
	drawVerticalLinesOnPar(x, y1, y2);

	//glTranslatef(0.0f, 0.0f, -1.5f);
	x = 0.2f;	y1 = -0.32f;	y2 = -0.45f;
	drawVerticalLinesOnPar(x, y1, y2);

	x = 0.35f;	y1 = -0.20f;	y2 = -0.43f;
	drawVerticalLinesOnPar(x, y1, y2);

	x = 0.449f;	y1 = -0.15f;	y2 = -0.349f;
	drawVerticalLinesOnPar(x, y1, y2);

	glEnd();
}

void drawVerticalLinesOnPar(GLfloat x, GLfloat y1, GLfloat y2)
{
	glVertex3f(x, y1, 0.0f);
	glVertex3f(x, y2, 0.0f);
	glVertex3f(x + 0.001f, y1, 0.0f);
	glVertex3f(x + 0.001f, y2, 0.0f);
	glVertex3f(x + 0.002f, y1, 0.0f);
	glVertex3f(x + 0.002f, y2, 0.0f);
	glVertex3f(x + 0.004f, y1, 0.0f);
	glVertex3f(x + 0.004f, y2, 0.0f);
}

#pragma endregion Pratik code

#pragma region region Mahesh code
void Draw_grass(GLfloat x, GLfloat z)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(x, -2.0f, z);
	glScalef(0.2f, 0.2f, 0.2f);
	glLineWidth(1);
	glBegin(GL_LINES);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.5f, 0.0f);

	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.3f, 0.6f, 0.0f);

	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.4f, 0.7f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-0.2f, 0.5f, 0.0f);

	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-0.3f, 0.6f, 0.0f);

	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-0.4f, 0.7f, 0.0f);

	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.5f, 0.0f);

	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.1f, 0.2f, 0.0f);

	glEnd();

}

#pragma endregion Mahesh code

#pragma region region Akhilesh code

void BirdsAnimation()
{
	GLfloat static fly1 = -4;
	GLfloat static fly2 = -4.5;
	GLfloat static fly3 = -5;

	GLfloat static wingMove = 0;
	//1st Bird
	glMatrixMode(GL_MODELVIEW); //Most important
	glLoadIdentity();
	glTranslatef(fly1, 0.90, -8.0f);
	glRotatef(-10, 0, 0, 1);
	glColor3f(1.0f, 1.0f, 1.0f);
	//face
	glColor3f(0.647, 0.749, 0.764);
	glBegin(GL_POLYGON);
	glVertex2f(0.05, 0.05);
	glVertex2f(0.058, 0.055);
	glVertex2f(0.061, 0.065);

	glVertex2f(0.125, 0.10);
	glVertex2f(0.15, 0.07);
	glColor3f(0.729, 0.717, 0.662);
	glVertex2f(0.15, 0.05);
	glVertex2f(0.125, 0.025);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex2f(0.05, 0.025);
	glEnd();
	glColor3f(0.729, 0.717, 0.662);
	glBegin(GL_POLYGON);
	glVertex2f(0.125, 0.025);
	glVertex2f(0.05, 0.025);
	glVertex2f(0.05, 0);
	glVertex2f(0.10, 0);

	glEnd();
	glPointSize(3);
	glBegin(GL_POINTS);

	glColor3f(0, 0, 0);
	glVertex2f(0.12, 0.08);
	glEnd();
	glColor3f(1, 1, 1);
	//peek
	glBegin(GL_TRIANGLES);
	glVertex2f(0.15, 0.07);
	glVertex2f(0.15, 0.05);
	glVertex2f(0.20, 0.06);
	glEnd();

	//body

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_POLYGON);
	glVertex2f(0.10, 0);
	glVertex2f(0, 0);
	glVertex2f(-0.12, -0.05);
	glColor3f(0.647, 0.749, 0.764);
	glVertex2f(-0.25, -0.15);
	glVertex2f(0.05, -0.05);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);

	glVertex2f(-0.15, -0.12);
	glVertex2f(-0.18, -0.16);

	glVertex2f(-0.18, -0.16);
	glVertex2f(-0.20, -0.18);


	glVertex2f(-0.13, -0.10);
	glVertex2f(-0.14, -0.15);

	glVertex2f(-0.14, -0.15);
	glVertex2f(-0.16, -0.17);
	glEnd();


	//Wings
	glMatrixMode(GL_MODELVIEW); //Most important
	glLoadIdentity();
	glTranslatef(fly1, 0.90, -8.0f);
	glRotatef(-10, 0, 0, 1);
	glRotatef(wingMove, 1, 0, 0);

	glColor3f(0.647, 0.749, 0.764);
	glBegin(GL_POLYGON);

	glVertex2f(0.05, 0);
	glVertex2f(0.05, 0.025);
	glVertex2f(-0.25, 0.25);
	glVertex2f(-0.24, 0.24);
	glVertex2f(-0.23, 0.15);
	glColor3f(1, 1, 1);
	glVertex2f(-0.22, 0.11);
	glVertex2f(-0.20, 0.05);
	glVertex2f(-0.17, 0);
	glVertex2f(-0.15, -0.025);
	glVertex2f(-0.09, -0.019);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0, 0.07);
	glVertex2f(-0.10, 0.20);
	glVertex2f(-0.102, 0.18);
	glVertex2f(-0.10, 0.16);
	glVertex2f(-0.09, 0.14);
	glEnd();

	//2 bird
	glMatrixMode(GL_MODELVIEW); //Most important
	glLoadIdentity();
	glTranslatef(fly2, 0.60, -5.0f);
	glRotatef(-10, 0, 0, 1);
	glColor3f(1.0f, 1.0f, 1.0f);
	//face
	glColor3f(0.647, 0.749, 0.764);
	glBegin(GL_POLYGON);
	glVertex2f(0.05, 0.05);
	glVertex2f(0.058, 0.055);
	glVertex2f(0.061, 0.065);

	glVertex2f(0.125, 0.10);
	glVertex2f(0.15, 0.07);
	glColor3f(0.729, 0.717, 0.662);
	glVertex2f(0.15, 0.05);
	glVertex2f(0.125, 0.025);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex2f(0.05, 0.025);
	glEnd();
	glColor3f(0.729, 0.717, 0.662);
	glBegin(GL_POLYGON);
	glVertex2f(0.125, 0.025);
	glVertex2f(0.05, 0.025);
	glVertex2f(0.05, 0);
	glVertex2f(0.10, 0);

	glEnd();
	glPointSize(3);
	glBegin(GL_POINTS);

	glColor3f(0, 0, 0);
	glVertex2f(0.12, 0.08);
	glEnd();
	glColor3f(1, 1, 1);
	//peek
	glBegin(GL_TRIANGLES);
	glVertex2f(0.15, 0.07);
	glVertex2f(0.15, 0.05);
	glVertex2f(0.20, 0.06);
	glEnd();

	//body

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_POLYGON);
	glVertex2f(0.10, 0);
	glVertex2f(0, 0);
	glVertex2f(-0.12, -0.05);
	glColor3f(0.647, 0.749, 0.764);
	glVertex2f(-0.25, -0.15);
	glVertex2f(0.05, -0.05);
	glEnd();



	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);

	glVertex2f(-0.15, -0.12);
	glVertex2f(-0.18, -0.16);

	glVertex2f(-0.18, -0.16);
	glVertex2f(-0.20, -0.18);


	glVertex2f(-0.13, -0.10);
	glVertex2f(-0.14, -0.15);

	glVertex2f(-0.14, -0.15);
	glVertex2f(-0.16, -0.17);
	glEnd();


	//Wings
	glMatrixMode(GL_MODELVIEW); //Most important
	glLoadIdentity();
	glTranslatef(fly2, 0.60, -5.0f);
	glRotatef(-10, 0, 0, 1);
	glRotatef(wingMove, 1, 0, 0);

	glColor3f(0.647, 0.749, 0.764);
	glBegin(GL_POLYGON);

	glVertex2f(0.05, 0);
	glVertex2f(0.05, 0.025);
	glVertex2f(-0.25, 0.25);
	glVertex2f(-0.24, 0.24);
	glVertex2f(-0.23, 0.15);
	glColor3f(1, 1, 1);
	glVertex2f(-0.22, 0.11);
	glVertex2f(-0.20, 0.05);
	glVertex2f(-0.17, 0);
	glVertex2f(-0.15, -0.025);
	glVertex2f(-0.09, -0.019);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0, 0.07);
	glVertex2f(-0.10, 0.20);
	glVertex2f(-0.102, 0.18);
	glVertex2f(-0.10, 0.16);
	glVertex2f(-0.09, 0.14);
	glEnd();
	//3 bird
	glMatrixMode(GL_MODELVIEW); //Most important
	glLoadIdentity();
	glTranslatef(fly3, 0.30, -6.0f);
	glRotatef(-10, 0, 0, 1);
	glColor3f(1.0f, 1.0f, 1.0f);
	//face
	glColor3f(0.647, 0.749, 0.764);
	glBegin(GL_POLYGON);
	glVertex2f(0.05, 0.05);
	glVertex2f(0.058, 0.055);
	glVertex2f(0.061, 0.065);

	glVertex2f(0.125, 0.10);
	glVertex2f(0.15, 0.07);
	glColor3f(0.729, 0.717, 0.662);
	glVertex2f(0.15, 0.05);
	glVertex2f(0.125, 0.025);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex2f(0.05, 0.025);
	glEnd();
	glColor3f(0.729, 0.717, 0.662);
	glBegin(GL_POLYGON);
	glVertex2f(0.125, 0.025);
	glVertex2f(0.05, 0.025);
	glVertex2f(0.05, 0);
	glVertex2f(0.10, 0);

	glEnd();
	glPointSize(3);
	glBegin(GL_POINTS);

	glColor3f(0, 0, 0);
	glVertex2f(0.12, 0.08);
	glEnd();
	glColor3f(1, 1, 1);
	//peek
	glBegin(GL_TRIANGLES);
	glVertex2f(0.15, 0.07);
	glVertex2f(0.15, 0.05);
	glVertex2f(0.20, 0.06);
	glEnd();

	//body

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_POLYGON);
	glVertex2f(0.10, 0);
	glVertex2f(0, 0);
	glVertex2f(-0.12, -0.05);
	glColor3f(0.647, 0.749, 0.764);
	glVertex2f(-0.25, -0.15);
	glVertex2f(0.05, -0.05);
	glEnd();



	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);

	glVertex2f(-0.15, -0.12);
	glVertex2f(-0.18, -0.16);

	glVertex2f(-0.18, -0.16);
	glVertex2f(-0.20, -0.18);


	glVertex2f(-0.13, -0.10);
	glVertex2f(-0.14, -0.15);

	glVertex2f(-0.14, -0.15);
	glVertex2f(-0.16, -0.17);
	glEnd();


	//Wings
	glMatrixMode(GL_MODELVIEW); //Most important
	glLoadIdentity();
	glTranslatef(fly3, 0.30, -6.0f);
	glRotatef(-10, 0, 0, 1);
	glRotatef(wingMove, 1, 0, 0);

	glColor3f(0.647, 0.749, 0.764);
	glBegin(GL_POLYGON);

	glVertex2f(0.05, 0);
	glVertex2f(0.05, 0.025);
	glVertex2f(-0.25, 0.25);
	glVertex2f(-0.24, 0.24);
	glVertex2f(-0.23, 0.15);
	glColor3f(1, 1, 1);
	glVertex2f(-0.22, 0.11);
	glVertex2f(-0.20, 0.05);
	glVertex2f(-0.17, 0);
	glVertex2f(-0.15, -0.025);
	glVertex2f(-0.09, -0.019);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0, 0.07);
	glVertex2f(-0.10, 0.20);
	glVertex2f(-0.102, 0.18);
	glVertex2f(-0.10, 0.16);
	glVertex2f(-0.09, 0.14);
	glEnd();



	fly1 = fly1 + 0.001;
	fly2 = fly2 + 0.001;
	fly3 = fly3 + 0.001;
	if (fly1 > 4)
	{
		fly1 = -6;

	}
	if (fly2 > 5)
	{
		fly2 = -6.5;

	}
	if (fly3 > 5.5)
	{
		fly3 = -7;

	}
	wingMove = wingMove + 0.15;
	if (wingMove > 90)
	{
		wingMove = 0;
	}
}

#pragma endregion Akhilesh


void Uninitialize(void)
{
	//code
	if (grgbFullScreen == true)
	{
		grdwStyle = GetWindowLong(grghwnd, GWL_STYLE);
		SetWindowLong(grghwnd, GWL_STYLE, (grdwStyle | WS_OVERLAPPEDWINDOW));
		SetWindowPlacement(grghwnd, &grgwpPrev);
		SetWindowPos(grghwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(true);

	}
	if (wglGetCurrentContext() == grghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (grghrc)
	{
		wglDeleteContext(grghrc);
		grghrc = NULL;
	}

	if (grghdc)
	{
		ReleaseDC(grghwnd, grghdc);
		grghdc = NULL;
	}

	if (grgpFile)
	{
		fprintf(grgpFile, "\n **** End ****\nLog File closed successfully. \n Program terminated successfully");
		fclose(grgpFile);
		grgpFile = NULL;
	}
}