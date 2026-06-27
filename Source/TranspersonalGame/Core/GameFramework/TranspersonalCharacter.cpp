#include "TranspersonalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"

// Forward-declared SurvivalComponent — include only if module dependency exists
// #include "Core/Survival/SurvivalComponent.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ─── Camera Boom ──────────────────────────────────────────────────────
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// ─── Follow Camera ────────────────────────────────────────────────────
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// ─── Movement defaults ────────────────────────────────────────────────
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// ─── Survival defaults ────────────────────────────────────────────────
	SurvivalStats.Health    = 100.0f;
	SurvivalStats.MaxHealth = 100.0f;
	SurvivalStats.Hunger    = 100.0f;
	SurvivalStats.Thirst    = 100.0f;
	SurvivalStats.Stamina   = 100.0f;
	SurvivalStats.Fear      = 0.0f;
	SurvivalStats.Temperature = 37.0f;

	bIsSprinting = false;

	// SurvivalComp — create when module dependency is wired
	SurvivalComp = nullptr;
}

void ATranspersonalCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickSurvivalDecay(DeltaTime);
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn",        this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate",    this, &ATranspersonalCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp",      this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate",  this, &ATranspersonalCharacter::LookUpAtRate);

	// Action bindings
	PlayerInputComponent->BindAction("Jump",   IE_Pressed,  this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump",   IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &ATranspersonalCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprint);
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

void ATranspersonalCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATranspersonalCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ATranspersonalCharacter::StartSprint()
{
	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

	// Drain stamina faster while sprinting (handled in TickSurvivalDecay)
}

void ATranspersonalCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

// ─── Damage ───────────────────────────────────────────────────────────────────

float ATranspersonalCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	ApplyDamageToSurvival(ActualDamage);
	return ActualDamage;
}

void ATranspersonalCharacter::ApplyDamageToSurvival(float Amount)
{
	SurvivalStats.Health = FMath::Clamp(SurvivalStats.Health - Amount, 0.0f, SurvivalStats.MaxHealth);

	if (SurvivalStats.Health <= 0.0f)
	{
		// Trigger death — disable movement, notify game mode
		GetCharacterMovement()->DisableMovement();
		DisableInput(Cast<APlayerController>(GetController()));
	}
}

bool ATranspersonalCharacter::IsAlive() const
{
	return SurvivalStats.Health > 0.0f;
}

float ATranspersonalCharacter::GetHealthPercent() const
{
	if (SurvivalStats.MaxHealth <= 0.0f) return 0.0f;
	return SurvivalStats.Health / SurvivalStats.MaxHealth;
}

// ─── Survival Decay ───────────────────────────────────────────────────────────

void ATranspersonalCharacter::TickSurvivalDecay(float DeltaTime)
{
	if (!IsAlive()) return;

	// Hunger decays at 1 unit/min (0.0167/s)
	SurvivalStats.Hunger = FMath::Clamp(SurvivalStats.Hunger - (0.0167f * DeltaTime), 0.0f, 100.0f);

	// Thirst decays at 1.5 units/min (0.025/s)
	SurvivalStats.Thirst = FMath::Clamp(SurvivalStats.Thirst - (0.025f * DeltaTime), 0.0f, 100.0f);

	// Stamina: drain while sprinting (20/s), recover while idle (10/s)
	if (bIsSprinting)
	{
		SurvivalStats.Stamina = FMath::Clamp(SurvivalStats.Stamina - (20.0f * DeltaTime), 0.0f, 100.0f);
		if (SurvivalStats.Stamina <= 0.0f)
		{
			StopSprint();
		}
	}
	else
	{
		SurvivalStats.Stamina = FMath::Clamp(SurvivalStats.Stamina + (10.0f * DeltaTime), 0.0f, 100.0f);
	}

	// Starvation / dehydration damage
	if (SurvivalStats.Hunger <= 0.0f || SurvivalStats.Thirst <= 0.0f)
	{
		// 1 HP/s when starving or dehydrated
		ApplyDamageToSurvival(1.0f * DeltaTime);
	}
}
