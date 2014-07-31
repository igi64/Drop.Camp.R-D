This documentation repository holds [node-webrtc](https://github.com/js-platform/node-webrtc) Windows build instructions notes.

1. Install [MS Visual Studio Express 2013 for Desktop](http://www.visualstudio.com/downloads/download-visual-studio-vs#d-express-windows-desktop)

2. Download [depot_tools for windows](https://src.chromium.org/svn/trunk/tools/depot_tools.zip)

3. Right-click on depot_tools.zip and click on extract all.
   Do not extract to a path containing spaces.
   
4. Add depot_tools folder to your environmental variables PATH
   set PATH=%PATH%;<path-to-depot_tools-directory>\depot_tools

5. Run gclient from the cmd shell.

6. Set necessary environmental variables
   You can create a shortcut to the environment variable dialog with the path: %windir%\System32\rundll32.exe sysdm.cpl,EditEnvironmentVariables

GYP_DEFINES -> component=shared_library build_with_chromium=0
GYP_GENERATORS -> msvs
GYP_MSVS_VERSION -> 2013
WDK_DIR -> c:\WinDDK\7600.16385.1 (or somewhere else if you chose a custom dir)

7. Clone the repository using gclient config http://webrtc.googlecode.com/svn/trunk.

8. Get the source code -> gclient sync --nohooks.

9. Create project files with gyp (overwriting any existing ones!) -> gclient runhooks --force.

10. in nss.gyp comment out "USE_HW_AES" and "INTEL_GCM" and rerun gclient runhooks --force

11. in video_render_direct3d9.h comment out #include <d3dx9.h>, in video_render_direct3d9.cc comment out D3DXMATRIX...

12. do not build for x64 platform!

13. Before wrtc.node deployment Install MS Visual C++ 2013 Redistributable and copy crssl.dll, crnss.dll, protobuf_lite.dll, crnspr.dll, icui18n.dll, icuuc.dll into binding directory


For overall information on the Drop.Camp project, please refer to the [Drop.Camp project Web site](https://github.com/igi64/Drop.Camp)

