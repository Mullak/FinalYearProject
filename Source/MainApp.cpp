/*******************************************
	MainApp.cpp

	Windows functions and DirectX setup
********************************************/

#include <windows.h>
#include <windowsx.h>
#include <d3dx9.h>

#include "Defines.h"
#include "Input.h"
#include "CTimer.h"
#include "ThirdYearProject.h"

#include <al.h>      // Main OpenAL functions
#include <alc.h>     // OpenAL "context" functions (also part of main OpenAL API)
#include <AL/alut.h> // OpenAL utility library - helper functions
void soundLoader();
namespace gen
{
	

/*Sound Variables*/
const int maxSounds = 5;
ALuint buffer[maxSounds];
ALuint source[maxSounds];

// Each source has several properties, see the code for examples. Here we store position and velocity of
// the sound source above (x, y & z)
ALfloat sourcePos[3] = { 0.0, 5.0, 100.0 };
ALfloat sourceVel[3] = { 0.0, 0.0, 0.0 };
ALfloat sourceMusicPos[3] = { 0.0, 0.0, 0.0 };
ALfloat sourceMusicVel[3] = { 0.0, 0.0, 0.0 };
// There is always assumed to be a listener in an OpenAL application. We don't need a specific listener
// variable. However, listeners also have properties (examples in code). Here we store the position and
// velocity of the listener
ALfloat listenerPos[3] = { 0.0, 5.0, 100.0 };
ALfloat listenerVel[3] = { 0.0, 0.0, 0.0 };

// The listener may be at an angle (which may affect the perception of sound). Here we store the 
// orientation of the listener. The first three values are the facing direction (x, y, z) of the
// listener - called "at" in the documentation. The next three values are the upward direction
// of the listener, called "up". These vectors can be extracted from a world or view matrix
// NOTE: OpenAL (like OpenGL) uses a right-handed system for 3D coordinates. To convert from the
// left-handed system  we have used, we must negate all Z values (facing direction has -ve Z below)
ALfloat listenerOri[6] = { 0.0, 0.0, -1.0,
                           0.0, 1.0, 0.0 };
float volume = 0.5f;
const float maxVolume = 2.0f;
const float minVolume = 0.0f;

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

// Core DirectX interface
LPDIRECT3D9       g_pD3D       = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL; // Our rendering device
LPD3DXFONT        g_pFont      = NULL;  // D3DX font

// Window rectangle (dimensions) & client window rectangle - used for toggling fullscreen
RECT ClientRect;
RECT WindowRect;
bool Fullscreen;

// Actual viewport dimensions (fullscreen or windowed)
TUInt32 ViewportWidth;
TUInt32 ViewportHeight;

// Current mouse position
TUInt32 MouseX;
TUInt32 MouseY;

// Game timer
CTimer Timer;


//-----------------------------------------------------------------------------
// D3D management
//-----------------------------------------------------------------------------

// Initialise Direct3D
bool D3DSetup( HWND hWnd )
{
	// Get initial window and client window dimensions
	GetWindowRect( hWnd, &WindowRect );
	GetClientRect( hWnd, &ClientRect );

    // Create the D3D object.
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
	if (!g_pD3D)
	{
        return false;
	}

    // Set up the structure used to create the D3DDevice
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;  // Don't wait for vertical sync
	d3dpp.BackBufferCount = 1;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.Windowed = TRUE;
	d3dpp.BackBufferWidth = ClientRect.right;
	d3dpp.BackBufferHeight = ClientRect.bottom;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	Fullscreen = false;
	ViewportWidth = d3dpp.BackBufferWidth;
	ViewportHeight = d3dpp.BackBufferHeight;

    // Create the D3DDevice
    if (FAILED(g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                     D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                     &d3dpp, &g_pd3dDevice )))
    {
        return false;
    }
	
	// Turn on tri-linear filtering (for up to three simultaneous textures)
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	// Create a font using D3DX helper functions
    if (FAILED(D3DXCreateFont( g_pd3dDevice, 12, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pFont )))
    {
        return false;
    }
	soundLoader();
	return true;
}


// Reset the Direct3D device to resize window or toggle fullscreen/windowed
bool ResetDevice( HWND hWnd, bool ToggleFullscreen = false )
{
	// If currently windowed...
	if (!Fullscreen)
	{
		// Get current window and client window dimensions
		RECT NewClientRect;
		GetWindowRect( hWnd, &WindowRect );
		GetClientRect( hWnd, &NewClientRect );

		// If not switching to fullscreen, then we must ensure the window is changing size, if
		// it isn't then return without doing anything
		if (!ToggleFullscreen && NewClientRect.right == ClientRect.right && 
			NewClientRect.bottom == ClientRect.bottom)
		{
			return true;
		}
		ClientRect = NewClientRect;
	}

	// Toggle fullscreen if requested
	if (ToggleFullscreen)
	{
		Fullscreen = !Fullscreen;
	}

	// Reset the structure used to create the D3DDevice
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;  // Don't wait for vertical sync
	d3dpp.BackBufferCount = 1;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	if (!Fullscreen)
	{
		// Set windowed parameters - need to set the back buffer dimensions when reseting,
		// match them to the window client area
	    d3dpp.Windowed = TRUE;
		d3dpp.BackBufferWidth = ClientRect.right;
		d3dpp.BackBufferHeight = ClientRect.bottom;
	    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	}
	else
	{
		// Get current dimensions of primary monitor
		MONITORINFO monitorInfo;
		monitorInfo.cbSize = sizeof(MONITORINFO);
		if (GetMonitorInfo( g_pD3D->GetAdapterMonitor( D3DADAPTER_DEFAULT ), &monitorInfo ))
		{
			d3dpp.BackBufferWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
			d3dpp.BackBufferHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
		}
		else
		{
			d3dpp.BackBufferWidth = 1280;
			d3dpp.BackBufferHeight = 1024;
		}

		// Set other fullscreen parameters
		d3dpp.Windowed = FALSE;
	    d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	}
	ViewportWidth = d3dpp.BackBufferWidth;
	ViewportHeight = d3dpp.BackBufferHeight;

	// Need to recreate resources when reseting. Any resources (vertex & index buffers, textures) created
	// using D3DPOOL_MANAGED rather than D3DPOOL_DEFAULT will be recreated automatically. Dynamic resources
	// must be in D3DPOOL_DEFAULT, so they must be recreated manually. D3DX fonts are such an example.
	// Other dynamic resources are those that are updated during the game loop, e.g. procedural textures,
	// or dynamic terrain
	if (g_pFont != NULL)
		g_pFont->Release();

	// Reset the Direct3D device with the new settings
    if (FAILED(g_pd3dDevice->Reset( &d3dpp )))
    {
        return false;
    }

	// If reseting to windowed mode, we need to reset the window size
	if (!Fullscreen)
	{
		SetWindowPos( hWnd, HWND_NOTOPMOST, WindowRect.left, WindowRect.top,
			          WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, 0 );
	}

	// Need to set up states again after reset
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	// Recreate the font
    if (FAILED(D3DXCreateFont( g_pd3dDevice, 12, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pFont )))
    {
        return false;
    }

	return true;
}


// Uninitialise D3D
void D3DShutdown()
{
	alutExit();
	// Release D3D interfaces
	if (g_pFont != NULL)
		g_pFont->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}


} // namespace gen


//-----------------------------------------------------------------------------
// Windows functions - outside of namespace
//-----------------------------------------------------------------------------

// Window message handler
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
		{
            PostQuitMessage( 0 );
            return 0;
		}

        case WM_SIZE:
		{
			// Resized window - reset device to match back buffer to new window size
			if (gen::g_pd3dDevice && !gen::ResetDevice( hWnd ))
			{
				DestroyWindow( hWnd );
			}
            return 0;
		}

		case WM_KEYDOWN:
		{
			gen::EKeyCode eKeyCode = static_cast<gen::EKeyCode>(wParam);
			gen::KeyDownEvent( eKeyCode );
			break;
		}

		case WM_KEYUP:
		{
			gen::EKeyCode eKeyCode = static_cast<gen::EKeyCode>(wParam);
			gen::KeyUpEvent( eKeyCode );
			break;
		}
		case WM_MOUSEMOVE:
		{
			gen::MouseX = GET_X_LPARAM(lParam); 
			gen::MouseY = GET_Y_LPARAM(lParam);
		}
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}

// Windows main function
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    // Register the window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), LoadIcon( NULL, IDI_APPLICATION ),
					  LoadCursor( NULL, IDC_ARROW ), NULL, NULL,
                      "ThirdYearProject", NULL };
    RegisterClassEx( &wc );

    // Create the application's window
	HWND hWnd = CreateWindow( "ThirdYearProject", "ThirdYearProject",
                              WS_OVERLAPPEDWINDOW, 100, 100, 1024, 720,
                              NULL, NULL, wc.hInstance, NULL );
    // Initialize Direct3D
	if (gen::D3DSetup( hWnd ))
    {
        // Prepare the scene
        if (gen::SceneSetup())
        {
            // Show the window
            ShowWindow( hWnd, SW_SHOWDEFAULT );
            UpdateWindow( hWnd );

			// Reset the timer for a timed game loop
			gen::Timer.Reset();

            // Enter the message loop
            MSG msg;
            ZeroMemory( &msg, sizeof(msg) );
			alSourcePlay(gen::source[4]);
            while( msg.message != WM_QUIT )
            {
                if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
                else
				{
					// Render and update the scene - using variable timing
					float updateTime = gen::Timer.GetLapTime();
                    gen::RenderScene( updateTime );
					gen::UpdateScene( updateTime );

					// Toggle fullscreen / windowed
					if (gen::KeyHit( gen::Key_F1 ))
					{
						if (!gen::ResetDevice( hWnd, true ))
						{
							DestroyWindow( hWnd );
						}
					}

					// Quit on escape
					if (gen::KeyHit( gen::Key_Escape ))
					{
						DestroyWindow( hWnd );
					}
				}
            }
        }
	    gen::SceneShutdown();
		
    }
	alutExit();
	gen::D3DShutdown();

	UnregisterClass( "ThirdYearProject", wc.hInstance );
    return 0;
}

void soundLoader()
{
	// Initialise ALUT and hence OpenAL. If not using ALUT, we can initialise using core OpenAL functions.
	// However then we would need to consider what sound devices are available, with what features, and
	// select from those. ALUT just selects the default output device
	alutInit( 0, 0 );

	gen::buffer[0] = alutCreateBufferFromFile("Pain.wav");
	gen::buffer[1] = alutCreateBufferFromFile("HealSpell.wav");
	gen::buffer[2] = alutCreateBufferFromFile("MageAttack.wav");
	gen::buffer[3] = alutCreateBufferFromFile("MeleeAttack.wav");
	gen::buffer[4] = alutCreateBufferFromFile("Background.wav");
	alGenSources( 1, &gen::source[0] );
	alGenSources( 1, &gen::source[1] );
	alGenSources( 1, &gen::source[2] );
	alGenSources( 1, &gen::source[3] );
	alGenSources( 2, &gen::source[4] );
	for(int i = 0; i < gen::maxSounds-1; i++)
	{
		// Set the properties of the source. The full list of available properties can be found in the documentation
		// The last characters of each function name indicate the type of the second parameter (int, float, float vector etc.)
		alSourcei ( gen::source[i], AL_BUFFER,  gen::buffer[i] ); // Attach a buffer to the source (identify which sound to play)
		alSourcef ( gen::source[i], AL_PITCH,    1.0f );   // Pitch multiplier, doubling the pitch shifts the sound up 1 octave, halving
														 // the pitch shifts it down 1 octave. Will also shorten/lengthen the sound
		alSourcef ( gen::source[i], AL_GAIN,     1.0f );   // Effectively the volume of the sound - 0.0 = silent, 1.0 = as recorded. May
														// be able to increase volume over 1, but depends on sound
		alSourcefv( gen::source[i], AL_POSITION, gen::sourcePos ); // Position of sound relative to listener affects how it is reproduced through speakers
		alSourcefv( gen::source[i], AL_VELOCITY, gen::sourceVel ); // Velocity of sound relative to listener can cause Doppler effect
		alSourcei ( gen::source[i], AL_LOOPING,  AL_FALSE );  // Whether to loop the sound or just stop when it finishes
	}

	alSourcei ( gen::source[4], AL_BUFFER,  gen::buffer[4] );
	alSourcef ( gen::source[4], AL_PITCH,    1.0f );
    alSourcef ( gen::source[4], AL_GAIN,     1.0f ); 
    alSourcefv( gen::source[4], AL_POSITION, gen::sourcePos ); 
    alSourcefv( gen::source[4], AL_VELOCITY, gen::sourceVel ); 
	alSourcei ( gen::source[4], AL_LOOPING,  AL_TRUE );

	alListenerfv( AL_POSITION,    gen::listenerPos );
	alListenerfv( AL_VELOCITY,    gen::listenerVel ); 
	alListenerfv( AL_ORIENTATION, gen::listenerOri ); 
	alListenerf ( AL_GAIN,        gen::volume );
}
