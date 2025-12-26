#include "TankBodyComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UTankBodyComponent::UTankBodyComponent()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterial> BaseMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	UStaticMesh* Cube = CubeMesh.Object;
	UStaticMesh* Cylinder = CylinderMesh.Object;

	if (BaseMat.Object)
	{
		HullMat = UMaterialInstanceDynamic::Create(BaseMat.Object, this);
		HullMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.28f, 0.35f, 0.22f));

		TreadMat = UMaterialInstanceDynamic::Create(BaseMat.Object, this);
		TreadMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.12f, 0.12f, 0.12f));

		BarrelMat = UMaterialInstanceDynamic::Create(BaseMat.Object, this);
		BarrelMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.15f, 0.15f, 0.12f));
	}

	auto SetupMesh = [](UStaticMeshComponent* M, UStaticMesh* SM, UMaterialInstanceDynamic* Mat) {
		if (SM) M->SetStaticMesh(SM);
		if (Mat) M->SetMaterial(0, Mat);
		M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		M->SetCastShadow(true);
	};

	// === HULL ===
	Hull = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hull"));
	Hull->SetupAttachment(this);
	SetupMesh(Hull, Cube, HullMat);
	Hull->SetRelativeScale3D(FVector(2.4f, 1.4f, 0.5f));
	Hull->SetRelativeLocation(FVector(0.f, 0.f, 20.f));

	// === TREAD SEGMENTS ===
	CreateTreadSegments(true, Cube);   // Left
	CreateTreadSegments(false, Cube);  // Right

	// === TURRET ===
	TurretPivot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretPivot"));
	TurretPivot->SetupAttachment(this);
	TurretPivot->SetRelativeLocation(FVector(-10.f, 0.f, 45.f));

	Turret = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Turret"));
	Turret->SetupAttachment(TurretPivot);
	SetupMesh(Turret, Cube, HullMat);
	Turret->SetRelativeScale3D(FVector(1.0f, 0.85f, 0.4f));
	Turret->SetRelativeLocation(FVector(0.f, 0.f, 20.f));

	// === BARREL ===
	BarrelPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelPivot"));
	BarrelPivot->SetupAttachment(TurretPivot);
	BarrelPivot->SetRelativeLocation(FVector(50.f, 0.f, 20.f));

	Barrel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Barrel"));
	Barrel->SetupAttachment(BarrelPivot);
	SetupMesh(Barrel, Cylinder, BarrelMat);
	Barrel->SetRelativeScale3D(FVector(0.12f, 0.12f, 0.9f));
	Barrel->SetRelativeLocation(FVector(45.f, 0.f, 0.f));
	Barrel->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
}

void UTankBodyComponent::CreateTreadSegments(bool bLeftSide, UStaticMesh* Cube)
{
	TArray<UStaticMeshComponent*>& Treads = bLeftSide ? LeftTreads : RightTreads;
	FString Side = bLeftSide ? TEXT("L") : TEXT("R");

	for (int32 i = 0; i < TreadSegments; i++)
	{
		FName Name(*FString::Printf(TEXT("Tread_%s_%d"), *Side, i));
		UStaticMeshComponent* Seg = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Seg->SetupAttachment(this);
		if (Cube) Seg->SetStaticMesh(Cube);
		if (TreadMat) Seg->SetMaterial(0, TreadMat);
		Seg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Seg->SetRelativeScale3D(FVector(0.22f, 0.28f, 0.1f));
		Treads.Add(Seg);
	}

	UpdateTreadPositions(bLeftSide);
}

void UTankBodyComponent::UpdateTreadPositions(bool bLeftSide)
{
	TArray<UStaticMeshComponent*>& Treads = bLeftSide ? LeftTreads : RightTreads;
	float Offset = bLeftSide ? LeftTreadOffset : RightTreadOffset;
	float Y = bLeftSide ? -TreadY : TreadY;

	float HalfLen = TreadLength * 0.5f;
	float Height = TreadTopZ - TreadBottomZ;
	float Perimeter = TreadLength * 2.f + Height * 2.f;

	for (int32 i = 0; i < Treads.Num(); i++)
	{
		float T = FMath::Fmod(Offset + (float)i / Treads.Num(), 1.f);
		if (T < 0.f) T += 1.f;
		float Dist = T * Perimeter;

		FVector Pos;
		// All segments stay horizontal (no rotation)

		if (Dist < TreadLength)
		{
			// Top (moving forward)
			Pos = FVector(-HalfLen + Dist, Y, TreadTopZ);
		}
		else if (Dist < TreadLength + Height)
		{
			// Front curve (moving down) - segments stay at front X, interpolate Z
			float D = Dist - TreadLength;
			float Alpha = D / Height;
			Pos = FVector(HalfLen, Y, FMath::Lerp(TreadTopZ, TreadBottomZ, Alpha));
		}
		else if (Dist < TreadLength * 2.f + Height)
		{
			// Bottom (moving backward)
			float D = Dist - TreadLength - Height;
			Pos = FVector(HalfLen - D, Y, TreadBottomZ);
		}
		else
		{
			// Back curve (moving up) - segments stay at back X, interpolate Z
			float D = Dist - TreadLength * 2.f - Height;
			float Alpha = D / Height;
			Pos = FVector(-HalfLen, Y, FMath::Lerp(TreadBottomZ, TreadTopZ, Alpha));
		}

		Treads[i]->SetRelativeLocation(Pos);
		Treads[i]->SetRelativeRotation(FRotator::ZeroRotator); // Always horizontal
	}
}

void UTankBodyComponent::SetTurretYaw(float WorldYaw)
{
	if (TurretPivot)
	{
		float LocalYaw = WorldYaw - GetComponentRotation().Yaw;
		TurretPivot->SetRelativeRotation(FRotator(0.f, LocalYaw, 0.f));
	}
}

void UTankBodyComponent::SetBarrelPitch(float Pitch)
{
	if (BarrelPivot)
	{
		BarrelPivot->SetRelativeRotation(FRotator(FMath::Clamp(Pitch, -10.f, 25.f), 0.f, 0.f));
	}
}

void UTankBodyComponent::UpdateTreads(float ForwardSpeed, float TurnRate)
{
	// Smooth the input speeds to prevent jerky animation
	float SmoothRate = 0.1f;
	float TargetLeftSpeed = ForwardSpeed + TurnRate;
	float TargetRightSpeed = ForwardSpeed - TurnRate;
	
	SmoothedLeftSpeed = FMath::FInterpTo(SmoothedLeftSpeed, TargetLeftSpeed, GetWorld()->GetDeltaSeconds(), 8.f);
	SmoothedRightSpeed = FMath::FInterpTo(SmoothedRightSpeed, TargetRightSpeed, GetWorld()->GetDeltaSeconds(), 8.f);
	
	float Rate = 0.0002f;  // Half speed
	LeftTreadOffset = FMath::Fmod(LeftTreadOffset + SmoothedLeftSpeed * Rate + 1.f, 1.f);
	RightTreadOffset = FMath::Fmod(RightTreadOffset + SmoothedRightSpeed * Rate + 1.f, 1.f);

	UpdateTreadPositions(true);
	UpdateTreadPositions(false);
}

FVector UTankBodyComponent::GetMuzzleLocation() const
{
	if (BarrelPivot)
	{
		return BarrelPivot->GetComponentLocation() + BarrelPivot->GetForwardVector() * 90.f;
	}
	return GetComponentLocation();
}
