#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Log.h"

#include "AssetManager.h"
#include "MiniConfig.h"

#include "easyhook.h"
#include <Windows.h>
#include <gl/GL.h>

#pragma comment(lib, "EasyHook64.lib")

using namespace ci;
using namespace ci::app;
using namespace std;

BOOL WINAPI myBeepHook(DWORD dwFreq, DWORD dwDuration)
{
    cout << "\n****All your beeps belong to us!\n\n";
    return Beep(dwFreq + 800, dwDuration);
}

GLenum APIENTRY myglGetError(void)
{
    cout << "\n****All your glGetError belong to us!\n\n";
    return glGetError();
}

// d3dukmdt.h
typedef ULONGLONG D3DGPU_VIRTUAL_ADDRESS;
typedef UINT D3DKMT_HANDLE;
#define D3DDDI_MAX_BROADCAST_CONTEXT        64
#define D3DDDI_MAX_WRITTEN_PRIMARIES 16

typedef struct _D3DKMT_SUBMITCOMMANDFLAGS
{
    UINT    NullRendering : 1;  // 0x00000001
    UINT    PresentRedirected : 1;  // 0x00000002
    UINT    Reserved : 30;  // 0xFFFFFFFC
} D3DKMT_SUBMITCOMMANDFLAGS;

typedef struct _D3DKMT_SUBMITCOMMAND
{
    D3DGPU_VIRTUAL_ADDRESS      Commands;
    UINT                        CommandLength;
    D3DKMT_SUBMITCOMMANDFLAGS   Flags;
    ULONGLONG                   PresentHistoryToken;                            // in: Present history token for redirected present calls
    UINT                        BroadcastContextCount;
    D3DKMT_HANDLE               BroadcastContext[D3DDDI_MAX_BROADCAST_CONTEXT];
    VOID* pPrivateDriverData;
    UINT                        PrivateDriverDataSize;
    UINT                        NumPrimaries;
    D3DKMT_HANDLE               WrittenPrimaries[D3DDDI_MAX_WRITTEN_PRIMARIES];
    UINT                        NumHistoryBuffers;
    D3DKMT_HANDLE* HistoryBufferArray;
} D3DKMT_SUBMITCOMMAND;

using FP = NTSTATUS (*)(const D3DKMT_SUBMITCOMMAND* Arg1);
FP addr;
NTSTATUS myD3DKMTSubmitCommand(const D3DKMT_SUBMITCOMMAND* Arg1)
{
    cout << "\n****All your myD3DKMTSubmitCommand belong to us!\n\n";
    return addr(Arg1);
}

struct FlyCameraRotateApp : public App
{
    HOOK_TRACE_INFO hHook = { NULL }; // keep track of our hook

    void tryHook()
    {
        cout << "\n";
        addr = (FP)GetProcAddress(GetModuleHandle(TEXT("gdi32")), "D3DKMTSubmitCommand");

        // Install the hook
        NTSTATUS result = LhInstallHook(
            addr,
            myD3DKMTSubmitCommand,
            NULL,
            &hHook);
        if (FAILED(result))
        {
            wstring s(RtlGetLastErrorString());
            wcout << "Failed to install hook: ";
            wcout << s;
            cout << "\n\nPress any key to exit.";
            cin.get();
            return;
        }

        cout << "Activating hook for current thread only.\n";
        // If the threadId in the ACL is set to 0, 
        // then internally EasyHook uses GetCurrentThreadId()
        ULONG ACLEntries[1] = { 0 };
        LhSetInclusiveACL(ACLEntries, 1, &hHook);
    }

    void setup() override
    {
        tryHook();

        log::makeLogger<log::LoggerFileRotating>(fs::path(), "IG.%Y.%m.%d.log");
        
        auto aabb = am::triMesh(MESH_NAME)->calcBoundingBox();
        mCam.lookAt(aabb.getMax() * 2.0f, aabb.getCenter());
        mCamUi = CameraUi( &mCam, getWindow(), -1 );
        
        createConfigUI({200, 400});
        gl::enableDepth();

        getWindow()->getSignalResize().connect([&] {
            APP_WIDTH = getWindowWidth();
            APP_HEIGHT = getWindowHeight();
            mCam.setAspectRatio( getWindowAspectRatio() );
        });

        getSignalCleanup().connect([&] { 
            writeConfig();

            cout << "Uninstall hook\n";
            LhUninstallHook(&hHook);

            cout << "\n\nRestore ALL entry points of pending removals issued by LhUninstallHook()\n";
            LhWaitForPendingRemovals();
        });

        getWindow()->getSignalKeyUp().connect([&](KeyEvent& event) {
            if (event.getCode() == KeyEvent::KEY_ESCAPE) quit();
        });
        
        mGlslProg = am::glslProg("lambert");

        getWindow()->getSignalDraw().connect([&] {

            float pitchAmount = std::sin(app::getElapsedSeconds()) * 0.5f;
            float yawAmount = std::cos(app::getElapsedSeconds() * 0.5f) * 0.5f;
            float rollAmount = std::cos(app::getElapsedSeconds() * 0.3f) * 0.5f;

            quat pitchYawRoll = glm::quat() * glm::angleAxis(toRadians(PITCH), vec3(1.0f, 0.0f, 0.0f))
                * glm::angleAxis(toRadians(YAW), vec3(0.0f, 1.0f, 0.0f))
                * glm::angleAxis(toRadians(ROLL), vec3(0.0f, 0.0f, 1.0f));
            mCam.setOrientation(pitchYawRoll);

            gl::setMatrices( mCam );
            gl::clear();
        
            gl::ScopedGlslProg glsl(mGlslProg);

            gl::draw(am::vboMesh(MESH_NAME));
        });
    }
    
    CameraPersp         mCam;
    CameraUi            mCamUi;
    gl::GlslProgRef     mGlslProg;
};

CINDER_APP( FlyCameraRotateApp, RendererGl, [](App::Settings* settings) {
    readConfig();
    settings->setWindowSize(APP_WIDTH, APP_HEIGHT);
    settings->setMultiTouchEnabled(false);
    settings->setConsoleWindowEnabled();
} )
