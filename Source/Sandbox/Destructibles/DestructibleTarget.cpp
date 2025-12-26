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

	// Default mesh - cube (collision setup deferred to BeginPlay to avoid CDO issues)
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

	// Setup collision (deferred from constructor to avoid CDO physics issues)
	if (Mesh)
	{
		Mesh->SetSimulatePhysics(false);
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

float ADestructibleTarget::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	CurrentHealth -= ActualDamage;

	if (CurrentHealth <= 0.f)
	{
		// Get impact direction from damage causer
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
	// Spawn fire effect
	if (DestructionEffect)
	{
		UNiagaraComponent* FireComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DestructionEffect,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector(1.f),
			true, true,
			ENCPoolMethod::None
		);

		// Auto-deactivate after short duration
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
	if (!CubeMesh || !BaseMaterial) return;

	UWorld* World = GetWorld();
	FVector Origin = GetActorLocation();
	FVector ActorScale = GetActorScale3D();
	FVector DebrisScaleVec = ActorScale * DebrisScale;

	for (int32 i = 0; i < DebrisCount; i++)
	{
		// Random offset from center of destroyed object
		FVector Offset = FMath::VRand() * 30.f * ActorScale.GetMax();
		FVector SpawnLoc = Origin + Offset;
		FRotator SpawnRot = FRotator(FMath::RandRange(0.f, 360.f), FMath::RandRange(0.f, 360.f), FMath::RandRange(0.f, 360.f));

		// Create debris actor at the correct location
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		AActor* Debris = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLoc, SpawnRot, Params);
		if (!Debris) continue;

		// Add mesh component
		UStaticMeshComponent* DebrisMesh = NewObject<UStaticMeshComponent>(Debris);
		DebrisMesh->SetStaticMesh(CubeMesh);
		DebrisMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DebrisMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
		Debris->SetRootComponent(DebrisMesh);
		DebrisMesh->RegisterComponent();
		
		// Set transform after registration
		DebrisMesh->SetWorldLocation(SpawnLoc);
		DebrisMesh->SetWorldRotation(SpawnRot);
		DebrisMesh->SetWorldScale3D(DebrisScaleVec * FMath::RandRange(0.5f, 1.2f));

		// Apply material with slight color variation
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, Debris);
		float Variation = FMath::RandRange(0.8f, 1.2f);
		Mat->SetVectorParameterValue(TEXT("Color"), DebrisColor * Variation);
		DebrisMesh->SetMaterial(0, Mat);

		// Enable physics after setup
		DebrisMesh->SetSimulatePhysics(true);

		// Apply impulse - outward + upward + impact direction
		FVector Impulse = FMath::VRand();
		Impulse.Z = FMath::Abs(Impulse.Z) + 0.5f; // Bias upward
		Impulse = Impulse.GetSafeNormal() * DebrisForce;
		Impulse += ImpactDir * DebrisForce * 0.5f; // Add impact direction
		DebrisMesh->AddImpulse(Impulse, NAME_None, true);

		// Random spin
		FVector Torque = FMath::VRand() * 100.f;
		DebrisMesh->AddAngularImpulseInDegrees(Torque, NAME_None, true);

		// Auto-destroy after delay
		Debris->SetLifeSpan(5.f);
	}
}
