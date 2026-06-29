// TranspersonalCharacter.cpp
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260629_013
// Prehistoric survival game — human primitive character with WASD movement + survival stats

#include "TranspersonalCharacter.h"
#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// === Capsule ===
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// === Movement defaults ===
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = JumpZVelocity;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// === Camera Boom ===
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// === Follow Camera ===
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// === Survival Component ===
	SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));
}

void ATranspersonalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Apply movement speeds from properties
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		GetCharacterMovement()->JumpZVelocity = JumpZVelocity;
	}
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Movement axes
	PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);

	// Camera axes
	PlayerInputComponent->BindAxis("Turn",   this, &ATranspersonalCharacter::TurnCamera);
	PlayerInputComponent->BindAxis("LookUp", this, &ATranspersonalCharacter::LookUpCamera);

	// Jump
	PlayerInputComponent->BindAction("Jump", IE_Pressed,  this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Sprint
	PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &ATranspersonalCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprint);
}

// === Movement ===

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

void ATranspersonalCharacter::TurnCamera(float Value)
{
	AddControllerYawInput(Value);
}

void ATranspersonalCharacter::LookUpCamera(float Value)
{
	AddControllerPitchInput(Value);
}

void ATranspersonalCharacter::StartSprint()
{
	bIsSprinting = true;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
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

// === Survival Accessors ===

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
