# Cloud Storage Example

This project demonstrates how to use Oculus Cloud Storage to save games.
For full documentation on Cloud Storage, see the PLATFORM SDK tutorials at:
https://developer.oculus.com/documentation/platform/latest/

Gameplay is the ancient game of chance - encountering the largest random integer.
See RandomGame::Tick for the details.  Three different saves are being stored in the cloud and loaded at startup.
1) The highest score ever recorded.
1) The latest high score found.
1) The highest score on each device.


## Setup
1) Create a new Rift Application on the Developer Dashboard.
1) Copy your App ID to the constant OCULUS_APP_ID in main.cpp
1) On the Cloud Storage section of the Dashboard, create the following Buckets:
--  "Latest High Score" with a "Latest Timestamp" conflict resolution policy
--  "Overall Highest Score" with a "Highest Counter" conflict resolution policy
--  "Local High Scores" with a "Manual" conflict resolution policy
--  (These names are defined in CloudStorageManager.cpp if you want to change them.)
1) Compile and upload a build to the ALPHA channel for your app.  Make sure you are entitled as a developer on the app.
1) Install the App from the Oculus Store (should be located under My Preview Apps).
1) Open the solution in Visual Studio 2015, build and run!

## Understanding the Example
- The PlatformManager class polls the LibOVRPlatform message queue for messages, and forwards all the Cloud Storage messages to the CloudStorageManager class.
- CloudStorageManager transitions the game to running over after it loads all the saved metadata and data.
- The data in the "Latest High Score" and "Overall Highest Score" buckets are fairly simple to deal with since you don't need to handle conflicts from multiple devices.
- The save stored in the "Local High Scores" bucket is fairly complicated because it demonstrations resolving conflicts by loading both saved files and merging them together.
