#!/bin/bash

"/Users/Shared/Epic Games/UE_5.7/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
  -project="/Users/$USER/Dev/sandbox/Sandbox/Sandbox.uproject" \
  -platform=Mac \
  -clientconfig=Development \
  -build -cook -stage -package -archive \
  -archivedirectory="/Users/$USER/Dev/sandbox/Sandbox/Build" \
  -unattended \
  -utf8output \
  -IgnoreCookErrors
