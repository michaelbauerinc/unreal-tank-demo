#include "TankProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"

ATankProjectile::ATankProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// Simple visible mesh - no collision, just visual
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));

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
	if (World)
	{
		// Visual explosion - layered debug spheres
		DrawDebugSphere(World, Location, 50.f, 16, FColor::White, false, 0.3f, 0, 4.f);
		DrawDebugSphere(World, Location, 120.f, 16, FColor::Yellow, false, 0.6f, 0, 3.f);
		DrawDebugSphere(World, Location, 200.f, 12, FColor::Orange, false, 0.9f, 0, 2.f);
		DrawDebugSphere(World, Location, 280.f, 10, FColor::Red, false, 1.2f, 0, 1.f);
	}

	Destroy();
}
