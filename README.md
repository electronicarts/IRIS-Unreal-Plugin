# About
The IRIS plugin allows use of the [IRIS library](https://github.com/electronicarts/iris) inside Unreal Engine 5 (UE5). 

When enabled, the plugin captures a game's frames during runtime and analyses them asynchronously in real-time (in order to minimize impact on game performance) to detect moments that could potentially cause photosensitivity risks.

For more information on IRIS and how it detects potential photosensitivity issues, please visit the [IRIS repository](https://github.com/electronicarts/iris#readme).

Neither IRIS nor the IRIS plugin for UE5 are intended to guarantee, certify or otherwise validate visual content’s compliance with legal, regulatory or other requirements.

# Features
- Real-time photosensitivity analysis of rendered content inside UE5.
- Real-time feedback of detected potential photosensitivity issues via the flash transition and results line graphs for luminance and red saturated flashes.
- Automatic video capture of detected incidents.

# Supported Platforms
- Windows
  
# Usage
Use the command line commands to activate/deactivate IRIS while the game/scene is running to automatically analyse the rendered frames.

## Command line commands
- Iris.StartSession: activates the frame capture and IRIS analysis. 
- Iris.EndSession: deactivates the frame capture and IRIS analysis. 
- Iris.TransitionGraph: toggles the luminance and red flash transition graph. When the analysis is active, it is updated with the flash transition data from the analysis results.
- Iris.ResultsGraph: toggles the luminance and red flash frame result graph. When the analysis is active, it is updated with the flash transition data from the analysis results.
- Iris.StartAll: starts the analysis and toggles both the transition and results graphs. 
- Iris.RecordFailsOnVideo: when a photosensitivity issue is detected a video is recorded. The video contains the 2s prior to the incident, the duration of the incident and 2s afterwards. 
  
# Set up
1. Clone this repository into your project's Plugins directory.
2. Regenerate visual studio project and build your project or launch project and accept compiling the plugin.

No further action needed, the plugin can now be activated from any scene (if active, IRIS will continue the analysis when transitioning to another scene).

  
# Dependencies
- PixelCapture plugin 

# CONTRIBUTING
Before you can contribute, EA must have a Contributor License Agreement (CLA) on file that has been signed by each contributor. You can sign [here](https://electronicarts.na1.echosign.com/public/esignWidget?wid=CBFCIBAA3AAABLblqZhByHRvZqmltGtliuExmuV-WNzlaJGPhbSRg2ufuPsM3P0QmILZjLpkGslg24-UJtek*).
