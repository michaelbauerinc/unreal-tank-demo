#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructibleTarget.generated.h"

class UStaticMeshComponent;
class UNiagaraSystem;

/**
 * Base class for destructible environment objects.
 * Takes damage, spawns debris and effects when destroyed.
 * Debris can recursively break up to MaxBreakDepth times.
 */
UCLASS()
class SANDBOX_API ADestructibleTarget : public AActor
{
	GENERATED_BODY()

public:
	ADestructibleTarget();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// Set break depth for spawned debris
	void SetBreakDepth(int32 Depth) { CurrentBreakDepth = Depth; }
	void SetDebrisMode(float Scale, const FLinearColor& Color, float Health);

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

	// Maximum times debris can break (0 = original, 1 = first break, etc.)
	UPROPERTY(EditAnywhere, Category = "Destructible")
	int32 MaxBreakDepth = 3;

	// Fire effect on destruction
	UPROPERTY()
	UNiagaraSystem* DestructionEffect;

	float CurrentHealth;
	int32 CurrentBreakDepth = 0;  // 0 = original object

	// Cached mesh for debris
	UPROPERTY()
	UStaticMesh* CubeMesh;

	UPROPERTY()
	UMaterial* BaseMaterial;
};
