// TranspersonalCharacter.cpp
// Transpersonal Game Studio — Core Systems Programmer (#03)
// Prehistoric survival game: player character with full movement + survival stats

#include "TranspersonalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"

// SurvivalComponent forward-declared in header — include here for full definition
// #include "Core/Survival/SurvivalComponent.h"
// NOTE: SurvivalComponent is compiled in next build — using raw pointer until then

ATranspersonalCharacter::ATranspersonalCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ─── Capsule ──────────────────────────────────────────────────────────────
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// ─── Movement ─────────────────────────────────────────────────────────────
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate              = FRotator(0.0f, 540.0f, 0.0f);
		MoveComp->JumpZVelocity             = JumpVelocity;
		MoveComp->AirControl                = 0.2f;
		MoveComp->MaxWalkSpeed              = WalkSpeed;
		MoveComp->MaxWalkSpeedCrouched      = CrouchSpeed;
		MoveComp->NavAgentProps.bCanCrouch  = true;
	}

	// ─── Camera Boom ──────────────────────────────────────────────────────────
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength         = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset            = FVector(0.0f, 0.0f, 60.0f);

	// ─── Follow Camera ────────────────────────────────────────────────────────
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// ─── Survival Component ───────────────────────────────────────────────────
	// SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));
	// NOTE: Uncomment after SurvivalComponent is compiled into the module.
	// For now, survival stats are tracked directly on this class.
	SurvivalComp = nullptr;
}

void ATranspersonalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize survival stats
	CurrentHealth  = MaxHealth;
	CurrentStamina = MaxStamina;
	CurrentHunger  = MaxHunger;
	CurrentThirst  = MaxThirst;
	FearLevel      = 0.0f;
	bIsDead        = false;

	UE_LOG(LogTemp, Log, TEXT("TranspersonalCharacter: BeginPlay — survival stats initialized"));
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

	TickSurvivalStats(DeltaTime);
	TickStamina(DeltaTime);
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn",        this, &ATranspersonalCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp",      this, &ATranspersonalCharacter::LookUp);

	// Action bindings
	PlayerInputComponent->BindAction("Sprint",  IE_Pressed,  this, &ATranspersonalCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint",  IE_Released, this, &ATranspersonalCharacter::StopSprint);
	PlayerInputComponent->BindAction("Crouch",  IE_Pressed,  this, &ATranspersonalCharacter::ToggleCrouch);
	PlayerInputComponent->BindAction("Jump",    IE_Pressed,  this, &ATranspersonalCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump",    IE_Released, this, &ATranspersonalCharacter::StopJump);
}

// ─── Movement ─────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATranspersonalCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ATranspersonalCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ATranspersonalCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ATranspersonalCharacter::StartSprint()
{
	if (CurrentStamina > 5.0f)
	{
		bIsSprinting = true;
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = RunSpeed;
		}
	}
}

void ATranspersonalCharacter::StopSprint()
{
	bIsSprinting = false;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
	}
}

void ATranspersonalCharacter::ToggleCrouch()
{
	if (bIsCrouching)
	{
		UnCrouch();
		bIsCrouching = false;
	}
	else
	{
		Crouch();
		bIsCrouching = true;
	}
}

void ATranspersonalCharacter::StartJump()
{
	if (CurrentStamina > 10.0f)
	{
		Jump();
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - 10.0f);
	}
}

void ATranspersonalCharacter::StopJump()
{
	StopJumping();
}

// ─── Survival Functions ───────────────────────────────────────────────────────

void ATranspersonalCharacter::ApplyDamage(float DamageAmount)
{
	if (bIsDead) return;

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
	UE_LOG(LogTemp, Warning, TEXT("TranspersonalCharacter: Took %.1f damage — Health: %.1f/%.1f"),
		DamageAmount, CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		OnDeath();
		UE_LOG(LogTemp, Error, TEXT("TranspersonalCharacter: PLAYER DIED"));
	}
}

void ATranspersonalCharacter::Eat(float NutritionValue)
{
	CurrentHunger = FMath::Min(MaxHunger, CurrentHunger + NutritionValue);
	UE_LOG(LogTemp, Log, TEXT("TranspersonalCharacter: Ate — Hunger: %.1f/%.1f"), CurrentHunger, MaxHunger);
}

void ATranspersonalCharacter::Drink(float HydrationValue)
{
	CurrentThirst = FMath::Min(MaxThirst, CurrentThirst + HydrationValue);
	UE_LOG(LogTemp, Log, TEXT("TranspersonalCharacter: Drank — Thirst: %.1f/%.1f"), CurrentThirst, MaxThirst);
}

bool ATranspersonalCharacter::IsAlive() const
{
	return !bIsDead && CurrentHealth > 0.0f;
}

float ATranspersonalCharacter::GetHealthPercent() const
{
	return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ATranspersonalCharacter::GetStaminaPercent() const
{
	return (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
}

float ATranspersonalCharacter::GetHungerPercent() const
{
	return (MaxHunger > 0.0f) ? (CurrentHunger / MaxHunger) : 0.0f;
}

float ATranspersonalCharacter::GetThirstPercent() const
{
	return (MaxThirst > 0.0f) ? (CurrentThirst / MaxThirst) : 0.0f;
}

// ─── Private Tick Helpers ─────────────────────────────────────────────────────

void ATranspersonalCharacter::TickSurvivalStats(float DeltaTime)
{
	// Decay hunger and thirst
	CurrentHunger = FMath::Max(0.0f, CurrentHunger - HungerDecayRate * DeltaTime);
	CurrentThirst = FMath::Max(0.0f, CurrentThirst - ThirstDecayRate * DeltaTime);

	// Starvation damage
	if (CurrentHunger <= 0.0f)
	{
		ApplyDamage(StarvationDamageRate * DeltaTime);
		OnNearStarvation();
	}

	// Dehydration damage (faster than starvation)
	if (CurrentThirst <= 0.0f)
	{
		ApplyDamage(DehydrationDamageRate * DeltaTime);
		OnNearDehydration();
	}
}

void ATranspersonalCharacter::TickStamina(float DeltaTime)
{
	if (bIsSprinting && GetVelocity().SizeSquared() > 100.0f)
	{
		// Drain stamina while actively sprinting
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - StaminaDrainRate * DeltaTime);

		// Force stop sprint if stamina depleted
		if (CurrentStamina <= 0.0f)
		{
			StopSprint();
		}
	}
	else
	{
		// Recover stamina when not sprinting
		CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRecoveryRate * DeltaTime);
	}
}
