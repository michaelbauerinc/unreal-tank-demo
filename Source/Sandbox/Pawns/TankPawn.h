#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TankPawn.generated.h"

class UBoxComponent;
class UTankBodyComponent;
class USpringArmComponent;
class UCameraComponent;
class UTankInputConfig;
struct FInputActionValue;

/**
 * Physics-based tank pawn.
 * 
 * Controls:
 * - W/S: Drive forward/backward
 * - A/D: Turn hull
 * - Mouse: Aim turret/camera
 */
UCLASS()
class SANDBOX_API ATankPawn : public APawn
{
	GENERATED_BODY()

public:
	ATankPawn();

	UTankBodyComponent* GetTankBody() const { return TankBody; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	// Root physics body
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* Chassis;

	// Visual assembly (no collision)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTankBodyComponent* TankBody;

	// Camera
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCameraComponent* Camera;

	// Input config
	UPROPERTY()
	UTankInputConfig* InputConfig;

	// Aim state (world-space, decoupled from hull)
	float AimYaw = 0.f;
	float AimPitch = -20.f;

	// Input state
	float ThrottleInput = 0.f;
	float TurnInput = 0.f;

	// Firing
	float FireCooldown = 0.f;

	UPROPERTY(EditAnywhere, Category = "Tank|Combat")
	float FireRate = 0.5f;

	// Tuning
	UPROPERTY(EditAnywhere, Category = "Tank|Movement")
	float DriveForce = 8000000.f;  // 2x+ for bigger tank

	UPROPERTY(EditAnywhere, Category = "Tank|Movement")
	float TurnTorque = 200000000.f;  // More torque for bigger tank

	UPROPERTY(EditAnywhere, Category = "Tank|Movement")
	float MaxSpeed = 2400.f;  // 2x speed

	// Input callbacks
	void HandleMove(const FInputActionValue& Value);
	void HandleTurn(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleFire(const FInputActionValue& Value);

	// Update functions
	void ApplyMovement();
	void UpdateTurret();
	void Fire();
};
