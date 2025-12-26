#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TankHUD.generated.h"

/**
 * Tank HUD - fixed crosshair at screen center.
 */
UCLASS()
class SANDBOX_API ATankHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawCrosshairAt(float X, float Y);
};
