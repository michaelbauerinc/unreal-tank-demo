#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TankHUD.generated.h"

/**
 * Minimal HUD - 3D reticle on tank handles aiming.
 */
UCLASS()
class SANDBOX_API ATankHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
