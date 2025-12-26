#include "ExplosiveBarrel.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

AExplosiveBarrel::AExplosiveBarrel()
{
	// Use cylinder mesh for barrel
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BigFireFX(
		TEXT("/Game/Vefects/Free_Fire/Shared/Particles/NS_Fire_Big_Smoke.NS_Fire_Big_Smoke"));

	if (CylinderFinder.Object && Mesh)
	{
		Mesh->SetStaticMesh(CylinderFinder.Object);
	}

	if (BigFireFX.Succeeded())
	{
		ExplosionEffect = BigFireFX.Object;
	}

	// Barrel properties
	MaxHealth = 50.f;  // Easier to destroy
	DebrisCount = 8;
	DebrisScale = 0.25f;
	DebrisForce = 1200.f;
	DebrisColor = FLinearColor(0.6f, 0.15f, 0.1f);  // Reddish barrel color

	// Scale to barrel proportions
	SetActorScale3D(FVector(0.8f, 0.8f, 1.2f));
}

void AExplosiveBarrel::OnDestroyed()
{
	UWorld* World = GetWorld();
	FVector Location = GetActorLocation();

	// Big explosion effect
	if (ExplosionEffect)
	{
		UNiagaraComponent* FireComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			ExplosionEffect,
			Location,
			FRotator::ZeroRotator,
			FVector(2.f),  // Bigger scale
			true, true,
			ENCPoolMethod::None
		);

		if (FireComp)
		{
			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(TimerHandle, [FireComp]()
			{
				if (FireComp && FireComp->IsValidLowLevel())
				{
					FireComp->Deactivate();
				}
			}, 2.0f, false);
		}
	}

	// Apply radial damage to nearby destructibles (chain reaction!)
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	
	UGameplayStatics::ApplyRadialDamage(
		World,
		ExplosionDamage,
		Location,
		ExplosionRadius,
		nullptr,  // DamageType
		IgnoreActors,
		this,     // DamageCauser
		nullptr,  // Instigator controller
		true,     // DoFullDamage
		ECC_Visibility
	);
}
