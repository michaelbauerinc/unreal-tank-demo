#include "TankProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

ATankProjectile::ATankProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// Simple visible mesh - no collision, just visual
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	// Load explosion effect from Vefects pack
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> ExplosionFX(
		TEXT("/Game/Vefects/Free_Fire/Shared/Particles/NS_Fire_Big_Smoke.NS_Fire_Big_Smoke"));
	if (ExplosionFX.Succeeded())
	{
		ExplosionEffect = ExplosionFX.Object;
	}

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (SphereMesh.Object)
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}
	Mesh->SetRelativeScale3D(FVector(0.3f));
	Mesh->CastShadow = false;
	RootComponent = Mesh;

	// Projectile movement - handles the flying, no physics collision
	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->SetUpdatedComponent(Mesh);
	Movement->InitialSpeed = Speed;
	Movement->MaxSpeed = Speed;
	Movement->bRotationFollowsVelocity = true;
	Movement->ProjectileGravityScale = 0.15f;
	Movement->bShouldBounce = false;
}

void ATankProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Orange glowing material
	if (Mesh && Mesh->GetMaterial(0))
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(Mesh->GetMaterial(0), this);
		Mat->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.f, 0.5f, 0.f));
		Mesh->SetMaterial(0, Mat);
	}

	PrevLocation = GetActorLocation();
}

void ATankProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	if (Age > LifeTime)
	{
		Destroy();
		return;
	}

	// Line trace from previous position to current to detect hits
	FVector CurrentLocation = GetActorLocation();
	
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit, PrevLocation, CurrentLocation, ECC_Visibility, Params))
	{
		Explode(Hit.ImpactPoint);
		return;
	}

	PrevLocation = CurrentLocation;
}

void ATankProjectile::Explode(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Find all actors in explosion radius using overlap
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(ExplosionRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	if (World->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, ECC_WorldDynamic, Sphere, Params))
	{
		TSet<AActor*> DamagedActors;  // Avoid damaging same actor twice
		
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* HitActor = Overlap.GetActor();
			if (HitActor && !DamagedActors.Contains(HitActor))
			{
				DamagedActors.Add(HitActor);
				
				// Apply damage directly
				FDamageEvent DamageEvent;
				HitActor->TakeDamage(ExplosionDamage, DamageEvent, 
					GetOwner() ? GetOwner()->GetInstigatorController() : nullptr, this);
			}
		}
	}

	// Spawn Niagara explosion effect
	if (ExplosionEffect)
	{
		UNiagaraComponent* ExplosionComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			ExplosionEffect,
			Location,
			FRotator::ZeroRotator,
			FVector(1.5f),
			true, true,
			ENCPoolMethod::None
		);

		if (ExplosionComp)
		{
			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(TimerHandle, [ExplosionComp]()
			{
				if (ExplosionComp && ExplosionComp->IsValidLowLevel())
				{
					ExplosionComp->Deactivate();
				}
			}, 1.5f, false);
		}
	}

	Destroy();
}
