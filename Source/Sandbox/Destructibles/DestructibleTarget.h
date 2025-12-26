#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructibleTarget.generated.h"

class UStaticMeshComponent;
class UNiagaraSystem;

/**
 * Base class for destructible environment objects.
 * Takes damage, spawns debris and effects when destroyed.
 */
UCLASS()
class SANDBOX_API ADestructibleTarget : public AActor
{
	GENERATED_BODY()

public:
	ADestructibleTarget();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnDestroyed();
	virtual void SpawnDebris(const FVector& ImpactDir);

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Destructible")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Destructible")
	int32 DebrisCount = 6;

	UPROPERTY(EditAnywhere, Category = "Destructible")
	float DebrisScale = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Destructible")
	float DebrisForce = 800.f;

	UPROPERTY(EditAnywhere, Category = "Destructible")
	FLinearColor DebrisColor = FLinearColor(0.4f, 0.3f, 0.2f);

	// Fire effect on destruction
	UPROPERTY()
	UNiagaraSystem* DestructionEffect;

	float CurrentHealth;

	// Cached mesh for debris
	UPROPERTY()
	UStaticMesh* CubeMesh;

	UPROPERTY()
	UMaterial* BaseMaterial;
};
