#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TankInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;

/**
 * Tank input configuration - creates Enhanced Input actions in C++.
 */
UCLASS()
class SANDBOX_API UTankInputConfig : public UObject
{
	GENERATED_BODY()

public:
	void Setup();
	void AddMappingContext(UEnhancedInputLocalPlayerSubsystem* Subsystem);

	UPROPERTY()
	UInputAction* MoveAction;

	UPROPERTY()
	UInputAction* TurnAction;

	UPROPERTY()
	UInputAction* LookAction;

	UPROPERTY()
	UInputAction* FireAction;

private:
	UPROPERTY()
	UInputMappingContext* MappingContext;
};
