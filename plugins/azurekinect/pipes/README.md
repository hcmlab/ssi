# Azure Kinect pipelines
This directory contains example SSI pipelines that use the Azure Kinect DK. As the `azurekinect` plugin is currently in an experimental/alpha stage there are a couple things to be aware of:

## Building the plugin
After cloning the SSI repo and checking out the `plugin/azure-kinect` branch you will have to build the dll of this plugin in order to use it with SSIs `xmlpipe`:
* Open the plugin's sln (`<repo-root>/plugins/azurekinect/build/azurekinect.v14.sln`)
* Upon the first time a dialog to update sln settings will open:
    - Use Windows SDK 8.1
    - Do __NOT__ upgrade the toolkit version! MSVC v140 must be used.
* Make sure the project is set to `Release` and `x64`
* Right click the `azurekinect` project in the Solution Explorer
* Click `Build`
    > This will generate the `ssiazurekinect.dll` in the `<repo-root>/bin/x64/vc140` directory

---
> __IMPORTANT:__ The following sections assume that you have also correctly installed the necessary Azure Kinect SDKs as outline in `<repo-root>\plugins\azurekinect\INSTALL`
---

## Running the "Tests"
The solution includes a `test` project, which contains a single `Main.cpp` file. This file contains a couple of examples for using the plugin directly from code.
* Switch to `Debug` and `x64`
* Build (or Rebuild to be safe) the `azurekinect` project (this creates a debug version fo the dll: `ssiazurekinectd.dll`)
* Run the test via the VS "Play" button (Start Windows Debugger)

## Running the xml pipelines
If you set up SSI according to its installation documentation, you should have the executable `xmlpipe` on the path.
* Open a terminal in this folder
* Run `xmlpipe <filename>.pipeline`
* Follow the instructions in the terminal if there are any (normally pipelines start automatically)
* >__IMPORTANT__: If the xmlpipe process does not terminate after the pipeline is finished, kill it manually via the Windows Task Manaer. Due to a (probable) unresolved bug within the k4abt library, ssi blocks when it attemps to free the `ssiazurekinect.dll`. A fix and rebuild of ssi was introduced in `commit 8812137147af1fa9cf98bcd8abc9d4e1cc58acda`, so this should not be a problem anymore.