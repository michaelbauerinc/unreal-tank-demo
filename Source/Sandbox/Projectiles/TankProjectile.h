#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TankProjectile.generated.h"

class UProjectileMovementComponent;
class UStaticMeshComponent;

/**
 * Simple tank shell - flies forward, explodes on hit.
 * Uses ProjectileMovementComponent for reliable physics-free movement.
 */
UCLASS()
class SANDBOX_API ATankProjectile : public AActor
{
	GENERATED_BODY()

public:
	ATankProjectile();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void Explode(const FVector& Location);

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* Movement;

	UPROPERTY(EditDefaultsOnly)
	float Speed = 8000.f;

	UPROPERTY(EditDefaultsOnly)
	float LifeTime = 5.f;

	float Age = 0.f;
	FVector PrevLocation;
};
