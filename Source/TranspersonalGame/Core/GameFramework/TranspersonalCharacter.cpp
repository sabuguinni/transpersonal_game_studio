// TranspersonalCharacter.cpp
// Transpersonal Game Studio — Core Systems Programmer #03
// Full implementation: movement, camera, survival stats, SurvivalComponent integration

#include "TranspersonalCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/Survival/SurvivalComponent.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ─── Capsule ──────────────────────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    // ─── Movement defaults ────────────────────────────────────────────────────
    WalkSpeed = 400.f;
    RunSpeed  = 800.f;
    bIsSprinting = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    // ─── Camera Boom ──────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;

    // ─── Follow Camera ────────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ─── Survival Component ───────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // ─── Survival stat defaults ───────────────────────────────────────────────
    MaxHealth  = 100.f;
    MaxHunger  = 100.f;
    MaxThirst  = 100.f;
    MaxStamina = 100.f;
    MaxFear    = 100.f;

    CurrentHealth  = MaxHealth;
    CurrentHunger  = MaxHunger;
    CurrentThirst  = MaxThirst;
    CurrentStamina = MaxStamina;
    CurrentFear    = 0.f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Passive hunger/thirst drain (realistic survival pacing)
    const float HungerDrainRate = 0.5f;   // per second
    const float ThirstDrainRate = 0.8f;   // per second
    const float StaminaRegenRate = 10.f;  // per second when not sprinting

    if (IsAlive())
    {
        CurrentHunger = FMath::Max(0.f, CurrentHunger - HungerDrainRate * DeltaTime);
        CurrentThirst = FMath::Max(0.f, CurrentThirst - ThirstDrainRate * DeltaTime);

        // Starvation / dehydration damage
        if (CurrentHunger <= 0.f || CurrentThirst <= 0.f)
        {
            CurrentHealth = FMath::Max(0.f, CurrentHealth - 1.f * DeltaTime);
            if (CurrentHealth <= 0.f)
            {
                HandleDeath();
            }
        }

        // Stamina regen when not sprinting
        if (!bIsSprinting)
        {
            CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * DeltaTime);
        }
        else
        {
            // Stamina drain while sprinting
            CurrentStamina = FMath::Max(0.f, CurrentStamina - 15.f * DeltaTime);
            if (CurrentStamina <= 0.f)
            {
                StopSprint();
            }
        }
    }
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn",        this, &ATranspersonalCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp",      this, &ATranspersonalCharacter::LookUp);

    PlayerInputComponent->BindAction("Jump",   IE_Pressed,  this, &ATranspersonalCharacter::StartJump);
    PlayerInputComponent->BindAction("Jump",   IE_Released, this, &ATranspersonalCharacter::StopJump);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &ATranspersonalCharacter::StartSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprint);
}

// ─── Movement ─────────────────────────────────────────────────────────────────

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

void ATranspersonalCharacter::StartSprint()
{
    if (CurrentStamina > 10.f)
    {
        bIsSprinting = true;
        UpdateMovementSpeed();
    }
}

void ATranspersonalCharacter::StopSprint()
{
    bIsSprinting = false;
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::StartJump()
{
    if (CurrentStamina > 5.f)
    {
        Jump();
        CurrentStamina = FMath::Max(0.f, CurrentStamina - 10.f);
    }
}

void ATranspersonalCharacter::StopJump()
{
    StopJumping();
}

// ─── Survival Actions ─────────────────────────────────────────────────────────

void ATranspersonalCharacter::ApplyDamage(float DamageAmount)
{
    if (!IsAlive()) return;
    CurrentHealth = FMath::Max(0.f, CurrentHealth - DamageAmount);
    // Increase fear on damage
    CurrentFear = FMath::Min(MaxFear, CurrentFear + DamageAmount * 0.5f);
    if (CurrentHealth <= 0.f)
    {
        HandleDeath();
    }
}

void ATranspersonalCharacter::ConsumeFood(float NutritionValue)
{
    CurrentHunger = FMath::Min(MaxHunger, CurrentHunger + NutritionValue);
    // Eating also slightly restores health
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + NutritionValue * 0.1f);
}

void ATranspersonalCharacter::ConsumeWater(float HydrationValue)
{
    CurrentThirst = FMath::Min(MaxThirst, CurrentThirst + HydrationValue);
}

bool ATranspersonalCharacter::IsAlive() const
{
    return CurrentHealth > 0.f;
}

float ATranspersonalCharacter::GetHealthPercent() const
{
    return (MaxHealth > 0.f) ? (CurrentHealth / MaxHealth) : 0.f;
}

float ATranspersonalCharacter::GetHungerPercent() const
{
    return (MaxHunger > 0.f) ? (CurrentHunger / MaxHunger) : 0.f;
}

float ATranspersonalCharacter::GetThirstPercent() const
{
    return (MaxThirst > 0.f) ? (CurrentThirst / MaxThirst) : 0.f;
}

float ATranspersonalCharacter::GetStaminaPercent() const
{
    return (MaxStamina > 0.f) ? (CurrentStamina / MaxStamina) : 0.f;
}

// ─── Private Helpers ──────────────────────────────────────────────────────────

void ATranspersonalCharacter::HandleDeath()
{
    // Disable input, trigger ragdoll
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->DisableInput(PC);
    }
    GetMesh()->SetSimulatePhysics(true);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    UE_LOG(LogTemp, Warning, TEXT("TranspersonalCharacter: Player died."));
}

void ATranspersonalCharacter::UpdateMovementSpeed()
{
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? RunSpeed : WalkSpeed;
    }
}
