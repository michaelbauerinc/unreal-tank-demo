#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TankBodyComponent.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Visual tank assembly - hull, turret, barrel, animated treads.
 * All meshes have collision disabled (physics handled by parent).
 */
UCLASS()
class SANDBOX_API UTankBodyComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UTankBodyComponent();

	void SetTurretYaw(float WorldYaw);
	void SetBarrelPitch(float Pitch);
	void UpdateTreads(float ForwardSpeed, float TurnRate);
	FVector GetMuzzleLocation() const;

private:
	void CreateTreadSegments(bool bLeftSide, UStaticMesh* Mesh);
	void UpdateTreadPositions(bool bLeftSide);

	// Hull
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Hull;

	// Turret
	UPROPERTY(VisibleAnywhere)
	USceneComponent* TurretPivot;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Turret;

	// Barrel
	UPROPERTY(VisibleAnywhere)
	USceneComponent* BarrelPivot;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Barrel;

	// Tread segments (conveyor belt style)
	static constexpr int32 TreadSegments = 16;
	
	UPROPERTY()
	TArray<UStaticMeshComponent*> LeftTreads;
	
	UPROPERTY()
	TArray<UStaticMeshComponent*> RightTreads;

	// Materials
	UPROPERTY()
	UMaterialInstanceDynamic* HullMat;

	UPROPERTY()
	UMaterialInstanceDynamic* TreadMat;

	UPROPERTY()
	UMaterialInstanceDynamic* BarrelMat;

	// Tread animation
	float LeftTreadOffset = 0.f;
	float RightTreadOffset = 0.f;
	float SmoothedLeftSpeed = 0.f;
	float SmoothedRightSpeed = 0.f;

	// Tread geometry constants
	const float TreadLength = 280.f;
	const float TreadY = 90.f;
	const float TreadTopZ = 15.f;
	const float TreadBottomZ = -15.f;
};
