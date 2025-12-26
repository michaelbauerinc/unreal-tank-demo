#pragma once

#include "CoreMinimal.h"
#include "DestructibleTarget.h"
#include "ExplosiveBarrel.generated.h"

/**
 * Explosive barrel - cylinder shape, creates secondary explosion on destruction.
 */
UCLASS()
class SANDBOX_API AExplosiveBarrel : public ADestructibleTarget
{
	GENERATED_BODY()

public:
	AExplosiveBarrel();

protected:
	virtual void OnDestroyed() override;

	UPROPERTY(EditAnywhere, Category = "Explosive")
	float ExplosionRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Explosive")
	float ExplosionDamage = 80.f;

	// Bigger fire effect for barrel explosion
	UPROPERTY()
	UNiagaraSystem* ExplosionEffect;
};
