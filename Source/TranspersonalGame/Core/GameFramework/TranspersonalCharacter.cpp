// TranspersonalCharacter.cpp
// Prehistoric survival game — Transpersonal Game Studio
// Agent #03 — Core Systems Programmer
// Implements ATranspersonalCharacter: WASD movement, camera boom, SurvivalComponent integration

#include "TranspersonalCharacter.h"
#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ── Survival Component ──────────────────────────────────────────────────
	SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

	// ── Camera Boom ─────────────────────────────────────────────────────────
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 8.0f;
	CameraBoom->CameraLagMaxDistance = 50.0f;

	// ── Follow Camera ────────────────────────────────────────────────────────
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// ── Character Movement Defaults ──────────────────────────────────────────
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 420.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Don't rotate character with camera — camera follows independently
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;
}

void ATranspersonalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Ensure movement speed is initialised from property values
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ── Sprint stamina drain / recovery ─────────────────────────────────────
	if (SurvivalComp)
	{
		if (bIsSprinting && GetVelocity().SizeSquared() > 1.0f)
		{
			// Drain stamina while actively moving and sprinting
			const float CurrentStamina = SurvivalComp->GetStamina();
			if (CurrentStamina > 0.0f)
			{
				SurvivalComp->ModifyStamina(-SprintStaminaDrainRate * DeltaTime);
			}
			else
			{
				// Force stop sprint when stamina is empty
				OnStaminaDepleted();
			}
		}
		else if (!bIsSprinting)
		{
			// Recover stamina while not sprinting
			const float CurrentStamina = SurvivalComp->GetStamina();
			if (CurrentStamina < 100.0f)
			{
				SurvivalComp->ModifyStamina(StaminaRecoveryRate * DeltaTime);
			}
		}
	}
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Movement axes
	PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);

	// Camera axes
	PlayerInputComponent->BindAxis("Turn",    this, &ATranspersonalCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp",  this, &ATranspersonalCharacter::LookUp);

	// Jump
	PlayerInputComponent->BindAction("Jump", IE_Pressed,  this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Sprint
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
	if (SurvivalComp && SurvivalComp->GetStamina() > 5.0f)
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

// ── Survival helpers ─────────────────────────────────────────────────────────

void ATranspersonalCharacter::OnStaminaDepleted()
{
	StopSprint();
}

float ATranspersonalCharacter::GetHealth() const
{
	return SurvivalComp ? SurvivalComp->GetHealth() : 0.0f;
}

float ATranspersonalCharacter::GetHunger() const
{
	return SurvivalComp ? SurvivalComp->GetHunger() : 0.0f;
}

float ATranspersonalCharacter::GetThirst() const
{
	return SurvivalComp ? SurvivalComp->GetThirst() : 0.0f;
}

float ATranspersonalCharacter::GetStamina() const
{
	return SurvivalComp ? SurvivalComp->GetStamina() : 0.0f;
}

float ATranspersonalCharacter::GetFear() const
{
	return SurvivalComp ? SurvivalComp->GetFear() : 0.0f;
}
