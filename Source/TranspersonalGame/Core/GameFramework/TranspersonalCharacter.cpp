// TranspersonalCharacter.cpp — Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260630_002
// Prehistoric survival game character — full implementation
// Hunger/thirst/stamina drain over time, camera boom, follow camera, WASD movement

#include "TranspersonalCharacter.h"
#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Capsule ──────────────────────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    // ── Movement defaults ────────────────────────────────────────────────────
    WalkSpeed  = 300.f;
    RunSpeed   = 600.f;
    CrouchSpeed = 150.f;
    bIsSprinting = false;
    bIsExhausted = false;
    SprintStaminaDrainRate = 20.f;   // stamina units per second while sprinting
    StaminaRecoveryRate    = 10.f;   // stamina units per second while not sprinting

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->JumpZVelocity = 420.f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    // ── Camera boom ──────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 8.f;
    CameraBoom->CameraLagMaxDistance = 100.f;

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

    // Apply initial movement speed
    UpdateMovementSpeed();

    if (SurvivalComp)
    {
        UE_LOG(LogTemp, Log, TEXT("ATranspersonalCharacter: SurvivalComp active — Health=%.1f Hunger=%.1f Thirst=%.1f"),
            SurvivalComp->Health, SurvivalComp->Hunger, SurvivalComp->Thirst);
    }
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!SurvivalComp) return;

    // ── Survival stats drain over time ───────────────────────────────────────
    // Hunger drains at 1 unit/min (0.0167/s), thirst at 1.5 units/min (0.025/s)
    const float HungerDrainRate  = 0.0167f;
    const float ThirstDrainRate  = 0.025f;

    SurvivalComp->Hunger = FMath::Clamp(SurvivalComp->Hunger - HungerDrainRate * DeltaTime, 0.f, 100.f);
    SurvivalComp->Thirst = FMath::Clamp(SurvivalComp->Thirst - ThirstDrainRate * DeltaTime, 0.f, 100.f);

    // Health drains when starving or dehydrated
    if (SurvivalComp->Hunger <= 0.f || SurvivalComp->Thirst <= 0.f)
    {
        const float HealthDrainRate = 0.05f; // 3 units/min when starving/dehydrated
        SurvivalComp->Health = FMath::Clamp(SurvivalComp->Health - HealthDrainRate * DeltaTime, 0.f, 100.f);

        if (SurvivalComp->Health <= 0.f)
        {
            UE_LOG(LogTemp, Warning, TEXT("ATranspersonalCharacter: Player died from starvation/dehydration"));
        }
    }

    // Stamina drain/recovery
    HandleStaminaDrain(DeltaTime);

    // Fear decays over time (no threat nearby)
    SurvivalComp->FearLevel = FMath::Clamp(SurvivalComp->FearLevel - 2.f * DeltaTime, 0.f, 100.f);
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
    PlayerInputComponent->BindAction("Jump",   IE_Pressed,  this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump",   IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &ATranspersonalCharacter::StartSprinting);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprinting);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed,  this, &ATranspersonalCharacter::StartCrouching);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ATranspersonalCharacter::StopCrouching);
}

// ── Movement ─────────────────────────────────────────────────────────────────

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

void ATranspersonalCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ATranspersonalCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ATranspersonalCharacter::StartSprinting()
{
    if (bIsExhausted) return;
    bIsSprinting = true;
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::StopSprinting()
{
    bIsSprinting = false;
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::StartCrouching()
{
    Crouch();
}

void ATranspersonalCharacter::StopCrouching()
{
    UnCrouch();
}

void ATranspersonalCharacter::UpdateMovementSpeed()
{
    if (!GetCharacterMovement()) return;

    if (bIsSprinting && !bIsExhausted)
    {
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    }
    else if (GetCharacterMovement()->IsCrouching())
    {
        GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void ATranspersonalCharacter::HandleStaminaDrain(float DeltaTime)
{
    if (!SurvivalComp) return;

    if (bIsSprinting && GetVelocity().SizeSquared() > 100.f)
    {
        // Drain stamina while sprinting and moving
        SurvivalComp->Stamina = FMath::Clamp(
            SurvivalComp->Stamina - SprintStaminaDrainRate * DeltaTime, 0.f, 100.f);

        if (SurvivalComp->Stamina <= 0.f && !bIsExhausted)
        {
            bIsExhausted = true;
            StopSprinting();
            UE_LOG(LogTemp, Log, TEXT("ATranspersonalCharacter: Exhausted — stamina depleted"));
        }
    }
    else
    {
        // Recover stamina when not sprinting
        SurvivalComp->Stamina = FMath::Clamp(
            SurvivalComp->Stamina + StaminaRecoveryRate * DeltaTime, 0.f, 100.f);

        // Clear exhaustion when stamina recovers above 20%
        if (bIsExhausted && SurvivalComp->Stamina >= 20.f)
        {
            bIsExhausted = false;
            UE_LOG(LogTemp, Log, TEXT("ATranspersonalCharacter: Recovered from exhaustion"));
        }
    }
}

// ── Survival Actions ──────────────────────────────────────────────────────────

void ATranspersonalCharacter::AttemptEat()
{
    if (!SurvivalComp) return;
    // Restore 25 hunger units when eating (requires food item in inventory — simplified here)
    SurvivalComp->Hunger = FMath::Clamp(SurvivalComp->Hunger + 25.f, 0.f, 100.f);
    UE_LOG(LogTemp, Log, TEXT("ATranspersonalCharacter: Ate food — Hunger now %.1f"), SurvivalComp->Hunger);
}

void ATranspersonalCharacter::AttemptDrink()
{
    if (!SurvivalComp) return;
    // Restore 30 thirst units when drinking (requires water source nearby — simplified here)
    SurvivalComp->Thirst = FMath::Clamp(SurvivalComp->Thirst + 30.f, 0.f, 100.f);
    UE_LOG(LogTemp, Log, TEXT("ATranspersonalCharacter: Drank water — Thirst now %.1f"), SurvivalComp->Thirst);
}

// ── Stat Accessors ────────────────────────────────────────────────────────────

float ATranspersonalCharacter::GetHealthPercent() const
{
    return SurvivalComp ? SurvivalComp->Health / 100.f : 0.f;
}

float ATranspersonalCharacter::GetHungerPercent() const
{
    return SurvivalComp ? SurvivalComp->Hunger / 100.f : 0.f;
}

float ATranspersonalCharacter::GetThirstPercent() const
{
    return SurvivalComp ? SurvivalComp->Thirst / 100.f : 0.f;
}

float ATranspersonalCharacter::GetStaminaPercent() const
{
    return SurvivalComp ? SurvivalComp->Stamina / 100.f : 0.f;
}

float ATranspersonalCharacter::GetFearLevel() const
{
    return SurvivalComp ? SurvivalComp->FearLevel : 0.f;
}

// ── Combat / Damage ───────────────────────────────────────────────────────────

void ATranspersonalCharacter::TakeSurvivalDamage(float DamageAmount, AActor* DamageCauser)
{
    if (!SurvivalComp || DamageAmount <= 0.f) return;

    SurvivalComp->Health = FMath::Clamp(SurvivalComp->Health - DamageAmount, 0.f, 100.f);

    // Fear spikes when taking damage from a dinosaur
    SurvivalComp->FearLevel = FMath::Clamp(SurvivalComp->FearLevel + 30.f, 0.f, 100.f);

    UE_LOG(LogTemp, Warning, TEXT("ATranspersonalCharacter: Took %.1f damage from %s — Health=%.1f Fear=%.1f"),
        DamageAmount,
        DamageCauser ? *DamageCauser->GetName() : TEXT("Unknown"),
        SurvivalComp->Health,
        SurvivalComp->FearLevel);

    if (SurvivalComp->Health <= 0.f)
    {
        UE_LOG(LogTemp, Error, TEXT("ATranspersonalCharacter: Player is DEAD"));
        // TODO: trigger death sequence, respawn, or game over screen
    }
}

bool ATranspersonalCharacter::IsAlive() const
{
    return SurvivalComp ? SurvivalComp->Health > 0.f : false;
}
