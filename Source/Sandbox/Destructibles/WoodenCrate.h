#pragma once

#include "CoreMinimal.h"
#include "DestructibleTarget.h"
#include "WoodenCrate.generated.h"

/**
 * Wooden crate - simple cube that breaks into pieces.
 */
UCLASS()
class SANDBOX_API AWoodenCrate : public ADestructibleTarget
{
	GENERATED_BODY()

public:
	AWoodenCrate();
};
