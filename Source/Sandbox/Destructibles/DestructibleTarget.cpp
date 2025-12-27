#include "DestructibleTarget.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

ADestructibleTarget::ADestructibleTarget()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterial> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> FireFX(
		TEXT("/Game/Vefects/Free_Fire/Shared/Particles/NS_Fire_Small_Smoke.NS_Fire_Small_Smoke"));

	CubeMesh = CubeFinder.Object;
	BaseMaterial = MatFinder.Object;
	if (FireFX.Succeeded())
	{
		DestructionEffect = FireFX.Object;
	}

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	if (CubeMesh)
	{
		Mesh->SetStaticMesh(CubeMesh);
	}
	RootComponent = Mesh;
}

void ADestructibleTarget::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	if (Mesh)
	{
		Mesh->SetSimulatePhysics(CurrentBreakDepth > 0);  // Debris has physics
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionObjectType(ECC_WorldDynamic);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		Mesh->SetGenerateOverlapEvents(true);
	}

	// Apply color
	if (Mesh && BaseMaterial)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		Mat->SetVectorParameterValue(TEXT("Color"), DebrisColor);
		Mesh->SetMaterial(0, Mat);
	}
}

void ADestructibleTarget::SetDebrisMode(float Scale, const FLinearColor& Color, float Health)
{
	DebrisScale = Scale;
	DebrisColor = Color;
	MaxHealth = Health;
	
	// Fewer debris pieces as we get smaller
	DebrisCount = FMath::Max(2, DebrisCount / 2);
	DebrisForce *= 0.6f;
}

float ADestructibleTarget::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	CurrentHealth -= ActualDamage;

	if (CurrentHealth <= 0.f)
	{
		FVector ImpactDir = FVector::ZeroVector;
		if (DamageCauser)
		{
			ImpactDir = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
		}
		
		OnDestroyed();
		SpawnDebris(ImpactDir);
		Destroy();
	}

	return ActualDamage;
}

void ADestructibleTarget::OnDestroyed()
{
	// Only spawn fire effect for original objects or first break
	if (DestructionEffect && CurrentBreakDepth <= 1)
	{
		float EffectScale = CurrentBreakDepth == 0 ? 1.f : 0.5f;
		
		UNiagaraComponent* FireComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DestructionEffect,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector(EffectScale),
			true, true,
			ENCPoolMethod::None
		);

		if (FireComp)
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [FireComp]()
			{
				if (FireComp && FireComp->IsValidLowLevel())
				{
					FireComp->Deactivate();
				}
			}, 1.0f, false);
		}
	}
}

void ADestructibleTarget::SpawnDebris(const FVector& ImpactDir)
{
	// Don't spawn more debris if we've reached max break depth
	if (CurrentBreakDepth >= MaxBreakDepth) return;
	if (!CubeMesh || !BaseMaterial) return;

	UWorld* World = GetWorld();
	FVector Origin = GetActorLocation();
	FVector ActorScale = GetActorScale3D();

	// Calculate new debris scale (each break makes pieces ~40% smaller)
	float NewScale = (CurrentBreakDepth == 0) ? DebrisScale : DebrisScale * 0.5f;
	float NewHealth = MaxHealth * 0.3f;  // Debris is weaker

	for (int32 i = 0; i < DebrisCount; i++)
	{
		FVector Offset = FMath::VRand() * 30.f * ActorScale.GetMax();
		FVector SpawnLoc = Origin + Offset;
		FRotator SpawnRot = FRotator(FMath::RandRange(0.f, 360.f), FMath::RandRange(0.f, 360.f), 0.f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		// Spawn a new DestructibleTarget as debris
		ADestructibleTarget* Debris = World->SpawnActor<ADestructibleTarget>(
			GetClass(),  // Use same class (or base class for subclasses)
			SpawnLoc, 
			SpawnRot, 
			Params
		);
		
		if (!Debris) continue;

		// Configure as debris
		Debris->SetBreakDepth(CurrentBreakDepth + 1);
		
		// Vary the color slightly
		float Variation = FMath::RandRange(0.8f, 1.2f);
		FLinearColor VariedColor = DebrisColor * Variation;
		
		Debris->SetDebrisMode(NewScale, VariedColor, NewHealth);
		
		// Set scale
		float ScaleVariation = FMath::RandRange(0.7f, 1.3f);
		Debris->SetActorScale3D(ActorScale * NewScale * ScaleVariation);

		// Apply impulse after a tiny delay to let physics initialize
		FTimerHandle ImpulseTimer;
		FVector Impulse = FMath::VRand();
		Impulse.Z = FMath::Abs(Impulse.Z) + 0.5f;
		Impulse = Impulse.GetSafeNormal() * DebrisForce;
		Impulse += ImpactDir * DebrisForce * 0.5f;
		
		World->GetTimerManager().SetTimer(ImpulseTimer, [Debris, Impulse]()
		{
			if (Debris && Debris->Mesh)
			{
				Debris->Mesh->SetSimulatePhysics(true);
				Debris->Mesh->AddImpulse(Impulse, NAME_None, true);
				Debris->Mesh->AddAngularImpulseInDegrees(FMath::VRand() * 100.f, NAME_None, true);
			}
		}, 0.01f, false);
	}
}
