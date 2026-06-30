// TranspersonalCharacter.cpp — Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260630_001
// Full implementation of prehistoric survival character with SurvivalComponent

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

    // ── Capsule ──────────────────────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // ── Movement defaults ────────────────────────────────────────────────────
    WalkSpeed = 300.f;
    RunSpeed = 600.f;
    CrouchSpeed = 150.f;
    bIsSprinting = false;
    bIsExhausted = false;
    SprintStaminaDrainRate = 20.f;   // stamina units per second while sprinting
    StaminaRecoveryRate = 10.f;      // stamina units per second while resting

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->JumpZVelocity = 420.f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // ── Camera boom ──────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;

    // ── Follow camera ────────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ── Survival Component ───────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    HandleStaminaDrain(DeltaTime);
    UpdateMovementSpeed();
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
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ATranspersonalCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
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
    if (!bIsExhausted)
    {
        bIsSprinting = true;
        UpdateMovementSpeed();
    }
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
    float TargetSpeed = WalkSpeed;
    if (bIsSprinting && !bIsExhausted)
    {
        TargetSpeed = RunSpeed;
    }
    else if (GetCharacterMovement()->IsCrouching())
    {
        TargetSpeed = CrouchSpeed;
    }
    GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

void ATranspersonalCharacter::HandleStaminaDrain(float DeltaTime)
{
    if (!SurvivalComp) return;

    if (bIsSprinting && GetVelocity().SizeSquared() > 100.f)
    {
        // Drain stamina while sprinting
        float CurrentStamina = SurvivalComp->GetStamina();
        float NewStamina = FMath::Max(0.f, CurrentStamina - SprintStaminaDrainRate * DeltaTime);
        SurvivalComp->SetStamina(NewStamina);

        if (NewStamina <= 0.f)
        {
            bIsExhausted = true;
            bIsSprinting = false;
            UpdateMovementSpeed();
        }
    }
    else
    {
        // Recover stamina when not sprinting
        float CurrentStamina = SurvivalComp->GetStamina();
        float MaxStamina = SurvivalComp->GetMaxStamina();
        if (CurrentStamina < MaxStamina)
        {
            float NewStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRecoveryRate * DeltaTime);
            SurvivalComp->SetStamina(NewStamina);

            // Clear exhaustion when stamina recovers to 25%
            if (bIsExhausted && NewStamina >= MaxStamina * 0.25f)
            {
                bIsExhausted = false;
            }
        }
    }
}

// ── Survival Actions ──────────────────────────────────────────────────────────

void ATranspersonalCharacter::AttemptEat()
{
    if (SurvivalComp)
    {
        SurvivalComp->ConsumeFood(25.f);
    }
}

void ATranspersonalCharacter::AttemptDrink()
{
    if (SurvivalComp)
    {
        SurvivalComp->ConsumeWater(25.f);
    }
}

float ATranspersonalCharacter::GetHealthPercent() const
{
    if (!SurvivalComp) return 1.f;
    return SurvivalComp->GetHealth() / FMath::Max(1.f, SurvivalComp->GetMaxHealth());
}

float ATranspersonalCharacter::GetHungerPercent() const
{
    if (!SurvivalComp) return 1.f;
    return SurvivalComp->GetHunger() / FMath::Max(1.f, SurvivalComp->GetMaxHunger());
}

float ATranspersonalCharacter::GetThirstPercent() const
{
    if (!SurvivalComp) return 1.f;
    return SurvivalComp->GetThirst() / FMath::Max(1.f, SurvivalComp->GetMaxThirst());
}

float ATranspersonalCharacter::GetStaminaPercent() const
{
    if (!SurvivalComp) return 1.f;
    return SurvivalComp->GetStamina() / FMath::Max(1.f, SurvivalComp->GetMaxStamina());
}

float ATranspersonalCharacter::GetFearLevel() const
{
    if (!SurvivalComp) return 0.f;
    return SurvivalComp->GetFear();
}

// ── Combat / Damage ───────────────────────────────────────────────────────────

void ATranspersonalCharacter::TakeSurvivalDamage(float DamageAmount, AActor* DamageCauser)
{
    if (!SurvivalComp) return;

    float CurrentHealth = SurvivalComp->GetHealth();
    float NewHealth = FMath::Max(0.f, CurrentHealth - DamageAmount);
    SurvivalComp->SetHealth(NewHealth);

    // Increase fear when taking damage from a dinosaur
    if (DamageCauser)
    {
        float CurrentFear = SurvivalComp->GetFear();
        float NewFear = FMath::Min(100.f, CurrentFear + DamageAmount * 0.5f);
        SurvivalComp->SetFear(NewFear);
    }
}

bool ATranspersonalCharacter::IsAlive() const
{
    if (!SurvivalComp) return true;
    return SurvivalComp->GetHealth() > 0.f;
}
