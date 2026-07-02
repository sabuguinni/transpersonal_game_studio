#include "TranspersonalCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/Survival/SurvivalComponent.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ── Capsule ──────────────────────────────────────────────────────────────
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// ── Movement defaults ────────────────────────────────────────────────────
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate              = FRotator(0.0f, 540.0f, 0.0f);
		MoveComp->JumpZVelocity             = JumpZVelocity;
		MoveComp->AirControl                = 0.2f;
		MoveComp->MaxWalkSpeed              = WalkSpeed;
		MoveComp->GravityScale              = 1.2f;
	}

	// ── Camera boom ──────────────────────────────────────────────────────────
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength         = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset            = FVector(0.0f, 0.0f, 60.0f);

	// ── Follow camera ────────────────────────────────────────────────────────
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// ── Survival component ───────────────────────────────────────────────────
	SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));
}

void ATranspersonalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Apply movement speed from editable properties
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed  = WalkSpeed;
		MoveComp->JumpZVelocity = JumpZVelocity;
	}
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn",        this, &ATranspersonalCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp",      this, &ATranspersonalCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump",   IE_Pressed,  this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump",   IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &ATranspersonalCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprint);
}

// ── Movement ─────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
		const FVector  Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATranspersonalCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
		const FVector  Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ATranspersonalCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ATranspersonalCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ATranspersonalCharacter::StartSprint()
{
	bIsSprinting = true;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = SprintSpeed;
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

// ── Survival queries ──────────────────────────────────────────────────────────

float ATranspersonalCharacter::GetHealth() const
{
	if (SurvivalComp)
	{
		return SurvivalComp->GetHealth();
	}
	return 100.0f;
}

float ATranspersonalCharacter::GetHunger() const
{
	if (SurvivalComp)
	{
		return SurvivalComp->GetHunger();
	}
	return 100.0f;
}

float ATranspersonalCharacter::GetThirst() const
{
	if (SurvivalComp)
	{
		return SurvivalComp->GetThirst();
	}
	return 100.0f;
}

float ATranspersonalCharacter::GetStamina() const
{
	if (SurvivalComp)
	{
		return SurvivalComp->GetStamina();
	}
	return 100.0f;
}

bool ATranspersonalCharacter::IsAlive() const
{
	if (SurvivalComp)
	{
		return SurvivalComp->IsAlive();
	}
	return true;
}

void ATranspersonalCharacter::ApplyDamage_Survival(float DamageAmount)
{
	if (SurvivalComp)
	{
		SurvivalComp->ApplyDamage(DamageAmount);
	}
}
