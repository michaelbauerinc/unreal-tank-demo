#include "TankInputConfig.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "InputModifiers.h"

void UTankInputConfig::Setup()
{
	// === ACTIONS ===
	MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis1D;

	TurnAction = NewObject<UInputAction>(this, TEXT("IA_Turn"));
	TurnAction->ValueType = EInputActionValueType::Axis1D;

	LookAction = NewObject<UInputAction>(this, TEXT("IA_Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;

	// === MAPPING CONTEXT ===
	MappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Tank"));

	// W/S for forward/back
	{
		FEnhancedActionKeyMapping& W = MappingContext->MapKey(MoveAction, EKeys::W);
		FEnhancedActionKeyMapping& S = MappingContext->MapKey(MoveAction, EKeys::S);
		S.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	}

	// A/D for turning
	{
		FEnhancedActionKeyMapping& A = MappingContext->MapKey(TurnAction, EKeys::A);
		A.Modifiers.Add(NewObject<UInputModifierNegate>(this));
		FEnhancedActionKeyMapping& D = MappingContext->MapKey(TurnAction, EKeys::D);
	}

	// Mouse for look
	{
		MappingContext->MapKey(LookAction, EKeys::Mouse2D);
	}
}

void UTankInputConfig::AddMappingContext(UEnhancedInputLocalPlayerSubsystem* Subsystem)
{
	if (Subsystem && MappingContext)
	{
		Subsystem->AddMappingContext(MappingContext, 0);
	}
}
