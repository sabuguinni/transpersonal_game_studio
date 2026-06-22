#include "TranspersonalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "World/BiomeManager.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// --- Camera Boom ---
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 8.f;
	CameraBoom->CameraLagMaxDistance = 80.f;

	// --- Follow Camera ---
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// --- Survival Component ---
	SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

	// --- Movement defaults ---
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
		MoveComp->JumpZVelocity = 600.f;
		MoveComp->AirControl = 0.2f;
		MoveComp->MaxWalkSpeed = WalkSpeed;
		MoveComp->GravityScale = 1.0f;
	}

	// Biome integration defaults
	BiomeManagerRef = nullptr;
	BiomeTickAccumulator = 0.f;
}

void ATranspersonalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Find BiomeManager in the world (placed as singleton actor)
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABiomeManager::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		BiomeManagerRef = Cast<ABiomeManager>(FoundActors[0]);
		if (BiomeManagerRef)
		{
			UE_LOG(LogTemp, Log, TEXT("TranspersonalCharacter: BiomeManager found — survival modifiers active."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TranspersonalCharacter: No BiomeManager in level — biome modifiers disabled."));
	}

	// Set initial walk speed
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Sprint stamina drain
	HandleSprintStamina(DeltaTime);

	// Biome modifier tick (throttled)
	BiomeTickAccumulator += DeltaTime;
	if (BiomeTickAccumulator >= BiomeTickInterval)
	{
		BiomeTickAccumulator = 0.f;
		ApplyBiomeModifiers();
	}
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent) return;

	// Axis bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp",      this, &ATranspersonalCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookRight",   this, &ATranspersonalCharacter::LookRight);

	// Action bindings
	PlayerInputComponent->BindAction("Jump",   IE_Pressed,  this, &ATranspersonalCharacter::Jump_Input);
	PlayerInputComponent->BindAction("Jump",   IE_Released, this, &ATranspersonalCharacter::StopJump_Input);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &ATranspersonalCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprint);
}

// --- Movement ---

void ATranspersonalCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATranspersonalCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ATranspersonalCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ATranspersonalCharacter::LookRight(float Value)
{
	AddControllerYawInput(Value);
}

void ATranspersonalCharacter::Jump_Input()
{
	Jump();
}

void ATranspersonalCharacter::StopJump_Input()
{
	StopJumping();
}

// --- Sprint ---

void ATranspersonalCharacter::StartSprint()
{
	if (!SurvivalComp) return;

	// Only allow sprint if stamina > 10
	if (SurvivalComp->GetStamina() > 10.f)
	{
		bIsSprinting = true;
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		}
	}
}

void ATranspersonalCharacter::StopSprint()
{
	bIsSprinting = false;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void ATranspersonalCharacter::HandleSprintStamina(float DeltaTime)
{
	if (!SurvivalComp) return;

	if (bIsSprinting)
	{
		// Drain stamina while sprinting: 15 units/sec
		SurvivalComp->ModifyStamina(-15.f * DeltaTime);

		// Auto-stop sprint when stamina hits 0
		if (SurvivalComp->GetStamina() <= 0.f)
		{
			StopSprint();
		}
	}
	else
	{
		// Recover stamina when not sprinting: 8 units/sec
		SurvivalComp->ModifyStamina(8.f * DeltaTime);
	}
}

// --- Biome Integration ---

void ATranspersonalCharacter::ApplyBiomeModifiers()
{
	if (!BiomeManagerRef || !SurvivalComp) return;

	FVector PlayerLoc = GetActorLocation();
	FCore_BiomeSurvivalModifiers Mods = BiomeManagerRef->GetSurvivalModifiers(PlayerLoc);

	// Apply biome hunger/thirst drain multipliers
	// Base drain rates: hunger 2/min, thirst 3/min — scaled by biome
	float HungerDrain = 2.f * Mods.HungerDrainMultiplier * (BiomeTickInterval / 60.f);
	float ThirstDrain = 3.f * Mods.ThirstDrainMultiplier * (BiomeTickInterval / 60.f);

	SurvivalComp->ModifyHunger(-HungerDrain);
	SurvivalComp->ModifyThirst(-ThirstDrain);

	// Apply temperature stress as health drain if extreme
	if (Mods.TemperatureModifier > 40.f || Mods.TemperatureModifier < -10.f)
	{
		float TempStress = FMath::Abs(Mods.TemperatureModifier) * 0.05f * (BiomeTickInterval / 60.f);
		SurvivalComp->ModifyHealth(-TempStress);
	}

	// Danger level increases fear
	if (Mods.DangerLevel > 0.5f)
	{
		float FearIncrease = Mods.DangerLevel * 5.f * (BiomeTickInterval / 60.f);
		SurvivalComp->ModifyFear(FearIncrease);
	}

	UE_LOG(LogTemp, Verbose, TEXT("TranspersonalCharacter: BiomeModifiers applied — HungerDrain=%.2f ThirstDrain=%.2f Temp=%.1f Danger=%.2f"),
		HungerDrain, ThirstDrain, Mods.TemperatureModifier, Mods.DangerLevel);
}

// --- Survival Accessors ---

float ATranspersonalCharacter::GetHealth() const
{
	return SurvivalComp ? SurvivalComp->GetHealth() : 100.f;
}

float ATranspersonalCharacter::GetHunger() const
{
	return SurvivalComp ? SurvivalComp->GetHunger() : 100.f;
}

float ATranspersonalCharacter::GetThirst() const
{
	return SurvivalComp ? SurvivalComp->GetThirst() : 100.f;
}

float ATranspersonalCharacter::GetStamina() const
{
	return SurvivalComp ? SurvivalComp->GetStamina() : 100.f;
}

float ATranspersonalCharacter::GetFear() const
{
	return SurvivalComp ? SurvivalComp->GetFear() : 0.f;
}

void ATranspersonalCharacter::ApplyDamage_Survival(float DamageAmount)
{
	if (SurvivalComp)
	{
		SurvivalComp->ModifyHealth(-DamageAmount);
	}
}
