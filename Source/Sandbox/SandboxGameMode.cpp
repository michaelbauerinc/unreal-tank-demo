#include "SandboxGameMode.h"
#include "Pawns/TankPawn.h"
#include "UI/TankHUD.h"

ASandboxGameMode::ASandboxGameMode()
{
	DefaultPawnClass = ATankPawn::StaticClass();
	HUDClass = ATankHUD::StaticClass();
}
