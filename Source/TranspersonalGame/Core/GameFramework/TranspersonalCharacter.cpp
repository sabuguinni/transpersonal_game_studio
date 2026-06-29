#include "TranspersonalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/Survival/SurvivalComponent.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// === CAPSULE ===
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// === MOVEMENT DEFAULTS ===
	WalkSpeed = 300.f;
	RunSpeed = 600.f;
	JumpVelocity = 420.f;
	bIsSprinting = false;
	bIsCrouching = false;

	StaminaDrainRate = 10.f;   // per second while sprinting
	StaminaRegenRate  = 5.f;   // per second while not sprinting
	HungerDrainRate   = 0.5f;  // per second
	ThirstDrainRate   = 0.8f;  // per second

	// === CHARACTER MOVEMENT ===
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Don't rotate when the controller rotates
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	// === CAMERA BOOM ===
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 60.f);

	// === FOLLOW CAMERA ===
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// === SURVIVAL COMPONENT ===
	SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

	// === INITIAL SURVIVAL STATS ===
	Health  = 100.f;
	Hunger  = 100.f;
	Thirst  = 100.f;
	Stamina = 100.f;
	Fear    = 0.f;
}

void ATranspersonalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Sync stats from SurvivalComp if available
	if (SurvivalComp)
	{
		Health  = SurvivalComp->GetHealth();
		Hunger  = SurvivalComp->GetHunger();
		Thirst  = SurvivalComp->GetThirst();
		Stamina = SurvivalComp->GetStamina();
		Fear    = SurvivalComp->GetFear();
	}
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickSurvivalStats(DeltaTime);
}

void ATranspersonalCharacter::TickSurvivalStats(float DeltaTime)
{
	// Hunger and thirst drain over time
	Hunger  = FMath::Max(0.f, Hunger  - HungerDrainRate  * DeltaTime);
	Thirst  = FMath::Max(0.f, Thirst  - ThirstDrainRate  * DeltaTime);

	// Stamina: drain while sprinting, regen while idle/walking
	if (bIsSprinting && GetVelocity().SizeSquared() > 100.f)
	{
		Stamina = FMath::Max(0.f, Stamina - StaminaDrainRate * DeltaTime);
		if (Stamina <= 0.f)
		{
			StopSprint();
		}
	}
	else
	{
		Stamina = FMath::Min(100.f, Stamina + StaminaRegenRate * DeltaTime);
	}

	// Health drain if starving or dehydrated
	if (Hunger <= 0.f || Thirst <= 0.f)
	{
		Health = FMath::Max(0.f, Health - 2.f * DeltaTime);
	}

	// Sync to SurvivalComp
	if (SurvivalComp)
	{
		SurvivalComp->SetHealth(Health);
		SurvivalComp->SetHunger(Hunger);
		SurvivalComp->SetThirst(Thirst);
		SurvivalComp->SetStamina(Stamina);
		SurvivalComp->SetFear(Fear);
	}
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp",      this, &ATranspersonalCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookRight",   this, &ATranspersonalCharacter::LookRight);

	// Action bindings
	PlayerInputComponent->BindAction("Jump",   IE_Pressed,  this, &ATranspersonalCharacter::OnJumpPressed);
	PlayerInputComponent->BindAction("Jump",   IE_Released, this, &ATranspersonalCharacter::OnJumpReleased);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &ATranspersonalCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprint);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed,  this, &ATranspersonalCharacter::OnCrouchToggle);
}

// === MOVEMENT ===

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

void ATranspersonalCharacter::OnJumpPressed()
{
	Jump();
}

void ATranspersonalCharacter::OnJumpReleased()
{
	StopJumping();
}

void ATranspersonalCharacter::OnCrouchToggle()
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

// === SPRINT ===

void ATranspersonalCharacter::StartSprint()
{
	if (Stamina > 10.f)
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
}

void ATranspersonalCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

// === SURVIVAL ACTIONS ===

void ATranspersonalCharacter::Eat(float NutritionValue)
{
	Hunger = FMath::Min(100.f, Hunger + NutritionValue);
	if (SurvivalComp)
	{
		SurvivalComp->SetHunger(Hunger);
	}
}

void ATranspersonalCharacter::Drink(float HydrationValue)
{
	Thirst = FMath::Min(100.f, Thirst + HydrationValue);
	if (SurvivalComp)
	{
		SurvivalComp->SetThirst(Thirst);
	}
}

float ATranspersonalCharacter::GetHealthPercent() const
{
	return Health / 100.f;
}

float ATranspersonalCharacter::GetStaminaPercent() const
{
	return Stamina / 100.f;
}

bool ATranspersonalCharacter::IsAlive() const
{
	return Health > 0.f;
}
