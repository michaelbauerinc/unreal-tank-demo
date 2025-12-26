#include "TankPawn.h"
#include "Components/TankBodyComponent.h"
#include "Input/TankInputConfig.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ATankPawn::ATankPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// === PHYSICS CHASSIS ===
	Chassis = CreateDefaultSubobject<UBoxComponent>(TEXT("Chassis"));
	Chassis->SetBoxExtent(FVector(300.f, 200.f, 60.f));  // 2x size
	Chassis->SetCollisionProfileName(TEXT("Pawn"));
	Chassis->SetSimulatePhysics(true);
	Chassis->SetEnableGravity(true);
	Chassis->SetMassOverrideInKg(NAME_None, 8000.f);  // Heavier for bigger tank
	Chassis->SetLinearDamping(2.f);
	Chassis->SetAngularDamping(2.f);
	Chassis->SetCenterOfMass(FVector(0.f, 0.f, -80.f));  // Lower CoM for stability
	RootComponent = Chassis;

	// === TANK BODY (visuals only) ===
	TankBody = CreateDefaultSubobject<UTankBodyComponent>(TEXT("TankBody"));
	TankBody->SetupAttachment(Chassis);
	TankBody->SetRelativeLocation(FVector(0.f, 0.f, -25.f));  // Adjusted to sit on ground
	TankBody->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));  // 2x visual size

	// === CAMERA ===
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(Chassis);
	CameraBoom->TargetArmLength = 1200.f;  // Further back for bigger tank
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->SetUsingAbsoluteRotation(true);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom);
}

void ATankPawn::BeginPlay()
{
	Super::BeginPlay();
	AimYaw = GetActorRotation().Yaw;
}

void ATankPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ApplyMovement();
	UpdateTurret();

	// Tread animation from input (consistent speed regardless of terrain)
	float TreadForward = ThrottleInput * 1000.f;  // Constant rate based on input
	float TreadTurn = TurnInput * 500.f;
	TankBody->UpdateTreads(TreadForward, TreadTurn);
}

void ATankPawn::ApplyMovement()
{
	if (!Chassis->IsSimulatingPhysics()) return;

	FVector Velocity = Chassis->GetPhysicsLinearVelocity();
	float Speed = Velocity.Size();

	// Drive force - apply along current forward (follows terrain tilt)
	if (FMath::Abs(ThrottleInput) > 0.01f && Speed < MaxSpeed)
	{
		FVector Force = GetActorForwardVector() * ThrottleInput * DriveForce;
		Chassis->AddForce(Force);
	}

	// Turn - directly set angular velocity (more reliable than torque)
	float TargetAngularVel = TurnInput * 120.f; // 120 degrees per second at full input (faster turn)
	FVector CurrentAngVel = Chassis->GetPhysicsAngularVelocityInDegrees();
	// Only control yaw, let physics handle pitch/roll
	Chassis->SetPhysicsAngularVelocityInDegrees(FVector(CurrentAngVel.X, CurrentAngVel.Y, TargetAngularVel));

	// Brake when no throttle input
	if (FMath::Abs(ThrottleInput) < 0.01f)
	{
		FVector Brake = -Velocity * 5000.f;
		Brake.Z = 0.f;
		Chassis->AddForce(Brake);
	}
	
	// Anti-flip stabilization - apply corrective torque if tilting too much
	FRotator Rot = GetActorRotation();
	float MaxTilt = 45.f;
	
	if (FMath::Abs(Rot.Pitch) > MaxTilt)
	{
		float Correction = -Rot.Pitch * 30000.f;
		Chassis->AddTorqueInDegrees(FVector(0.f, Correction, 0.f));
	}
	
	if (FMath::Abs(Rot.Roll) > MaxTilt)
	{
		float Correction = -Rot.Roll * 30000.f;
		Chassis->AddTorqueInDegrees(FVector(Correction, 0.f, 0.f));
	}
}

void ATankPawn::UpdateTurret()
{
	// Camera follows aim
	CameraBoom->SetWorldRotation(FRotator(AimPitch, AimYaw, 0.f));

	// Turret tracks aim
	TankBody->SetTurretYaw(AimYaw);

	// Barrel elevation
	float BarrelPitch = FMath::GetMappedRangeValueClamped(
		FVector2D(-50.f, 0.f), FVector2D(20.f, -5.f), AimPitch);
	TankBody->SetBarrelPitch(BarrelPitch);
}

void ATankPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputConfig = NewObject<UTankInputConfig>(this);
	InputConfig->Setup();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			InputConfig->AddMappingContext(Subsystem);
		}
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(InputConfig->MoveAction, ETriggerEvent::Triggered, this, &ATankPawn::HandleMove);
		EIC->BindAction(InputConfig->MoveAction, ETriggerEvent::Completed, this, &ATankPawn::HandleMove);
		EIC->BindAction(InputConfig->TurnAction, ETriggerEvent::Triggered, this, &ATankPawn::HandleTurn);
		EIC->BindAction(InputConfig->TurnAction, ETriggerEvent::Completed, this, &ATankPawn::HandleTurn);
		EIC->BindAction(InputConfig->LookAction, ETriggerEvent::Triggered, this, &ATankPawn::HandleLook);
	}
}

void ATankPawn::HandleMove(const FInputActionValue& Value)
{
	ThrottleInput = Value.Get<float>();
}

void ATankPawn::HandleTurn(const FInputActionValue& Value)
{
	TurnInput = Value.Get<float>();
}

void ATankPawn::HandleLook(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	AimYaw = FMath::UnwindDegrees(AimYaw + Input.X * 0.5f);
	AimPitch = FMath::Clamp(AimPitch - Input.Y * 0.5f, -50.f, 0.f);
}
