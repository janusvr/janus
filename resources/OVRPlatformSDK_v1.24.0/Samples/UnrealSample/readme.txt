Once the sample is downloaded and extracted:
Right-click the .uproject file, and set the correct version of UE using the Switch Unreal Engine option.  This sample should run on 4.16 and greater.
This should have generated the VS .sln file.  If not, right click the .uproject again and select Generate Visual Studio files.
Open the .sln file in Visual Studio
Set the Target to Development Editor and build the OculusPlatformSample project under "Games" in the solution explorer
Once that builds correctly, open the .uproject file (double click).
The Sample can be run in editor
NOTE:  To test the netdriver functionality, you will have to package the sample in UE and run two instances of the app.  Running two instances in editor will probably use the built in UE net driver (IPNETDRIVER).


Other Setup:
You will need to edit the Config\DefaultEngine.ini file and update the AppID with one that you have an Entitlement for.   Look for this section:
[OnlineSubsystemOculus]
bEnabled=true
OculusAppId=1025875484134790

You will then need to configure your AppID on the dashboard to have the correct Matchmaking Pool (https://dashboard.oculus.com/application/<appid>/matchmaking:
Matchmaking Pool Name:  PoolTest (default quickmatch)


Achievements:
You can define your achievements on the portal and the sample should be able to display each achievement in the UI

Leaderboards:
There are currently hardcoded names for the leaderboards in the OSSLeaderboardWidget.cpp file in the SetupLB_UI() function.  Those names should match your Leaderboards that are defined on the developer portal.



