// TranspersonalCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Full implementation: movement, survival stats, biome modifiers, SurvivalComponent wiring

#include "TranspersonalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // === SURVIVAL COMPONENT ===
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // === CAMERA BOOM ===
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // === FOLLOW CAMERA ===
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // === MOVEMENT DEFAULTS ===
    WalkSpeed = 300.0f;
    RunSpeed = 600.0f;
    StaminaDrainPerSecond = 10.0f;
    StaminaRegenPerSecond = 5.0f;
    bIsRunning = false;

    // === CAMERA DEFAULTS ===
    BaseTurnRate = 45.0f;
    BaseLookUpRate = 45.0f;

    // === SURVIVAL STAT DEFAULTS ===
    CurrentHealth = 100.0f;
    CurrentHunger = 100.0f;
    CurrentThirst = 100.0f;
    CurrentStamina = 100.0f;
    CurrentFear = 0.0f;

    // === BIOME MODIFIER DEFAULTS ===
    BiomeMovementModifier = 1.0f;
    BiomeHungerDrainModifier = 1.0f;
    BiomeThirstDrainModifier = 1.0f;

    // Character movement config
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Sync initial stats from SurvivalComp if available
    if (SurvivalComp)
    {
        CurrentHealth = SurvivalComp->GetHealth();
        CurrentHunger = SurvivalComp->GetHunger();
        CurrentThirst = SurvivalComp->GetThirst();
        CurrentStamina = SurvivalComp->GetStamina();
        CurrentFear = SurvivalComp->GetFear();
    }

    UpdateMovementSpeedFromStats();
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TickSurvivalStats(DeltaTime);
    UpdateMovementSpeedFromStats();
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ATranspersonalCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("TurnRate", this, &ATranspersonalCharacter::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("LookUpRate", this, &ATranspersonalCharacter::LookUpAtRate);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ATranspersonalCharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ATranspersonalCharacter::StopJumping);
    PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ATranspersonalCharacter::StartRunning);
    PlayerInputComponent->BindAction("Run", IE_Released, this, &ATranspersonalCharacter::StopRunning);
}

// === MOVEMENT ===

void ATranspersonalCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ATranspersonalCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
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

void ATranspersonalCharacter::StartRunning()
{
    // Only run if stamina > 10%
    if (CurrentStamina > 10.0f)
    {
        bIsRunning = true;
    }
}

void ATranspersonalCharacter::StopRunning()
{
    bIsRunning = false;
}

void ATranspersonalCharacter::Jump()
{
    // Only jump if stamina > 5%
    if (CurrentStamina > 5.0f)
    {
        Super::Jump();
        if (SurvivalComp)
        {
            SurvivalComp->DrainStamina(8.0f);
        }
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - 8.0f);
    }
}

void ATranspersonalCharacter::StopJumping()
{
    Super::StopJumping();
}

// === SURVIVAL STAT TICK ===

void ATranspersonalCharacter::TickSurvivalStats(float DeltaTime)
{
    if (!SurvivalComp) return;

    // Hunger drain — scales with biome modifier and running
    float hungerDrain = 0.5f * BiomeHungerDrainModifier * DeltaTime;
    if (bIsRunning) hungerDrain *= 2.0f;
    SurvivalComp->DrainHunger(hungerDrain);

    // Thirst drain — scales with biome modifier and running
    float thirstDrain = 0.8f * BiomeThirstDrainModifier * DeltaTime;
    if (bIsRunning) thirstDrain *= 1.5f;
    SurvivalComp->DrainThirst(thirstDrain);

    // Stamina drain/regen
    if (bIsRunning && GetVelocity().SizeSquared() > 100.0f)
    {
        float staminaDrain = StaminaDrainPerSecond * DeltaTime;
        SurvivalComp->DrainStamina(staminaDrain);

        // Auto-stop running if stamina depleted
        if (SurvivalComp->GetStamina() <= 0.0f)
        {
            bIsRunning = false;
        }
    }
    else
    {
        // Regen stamina when not running
        float staminaRegen = StaminaRegenPerSecond * DeltaTime;
        // Hunger/thirst penalty on stamina regen
        if (CurrentHunger < 20.0f) staminaRegen *= 0.3f;
        if (CurrentThirst < 20.0f) staminaRegen *= 0.3f;
        SurvivalComp->RegenStamina(staminaRegen);
    }

    // Health drain from starvation/dehydration
    if (SurvivalComp->GetHunger() <= 0.0f)
    {
        SurvivalComp->DrainHealth(2.0f * DeltaTime);
    }
    if (SurvivalComp->GetThirst() <= 0.0f)
    {
        SurvivalComp->DrainHealth(3.0f * DeltaTime);
    }

    // Fear decay over time
    if (SurvivalComp->GetFear() > 0.0f)
    {
        SurvivalComp->DrainFear(5.0f * DeltaTime);
    }

    // Sync local cached values for Blueprint access
    CurrentHealth = SurvivalComp->GetHealth();
    CurrentHunger = SurvivalComp->GetHunger();
    CurrentThirst = SurvivalComp->GetThirst();
    CurrentStamina = SurvivalComp->GetStamina();
    CurrentFear = SurvivalComp->GetFear();
}

void ATranspersonalCharacter::UpdateMovementSpeedFromStats()
{
    float targetSpeed = bIsRunning ? RunSpeed : WalkSpeed;

    // Apply biome movement modifier
    targetSpeed *= BiomeMovementModifier;

    // Hunger/thirst penalty on movement
    if (CurrentHunger < 20.0f)
    {
        targetSpeed *= FMath::Lerp(0.5f, 1.0f, CurrentHunger / 20.0f);
    }
    if (CurrentThirst < 20.0f)
    {
        targetSpeed *= FMath::Lerp(0.4f, 1.0f, CurrentThirst / 20.0f);
    }

    // Fear boost — adrenaline gives brief speed boost
    if (CurrentFear > 70.0f)
    {
        targetSpeed *= 1.2f;
    }

    GetCharacterMovement()->MaxWalkSpeed = targetSpeed;
}

// === SURVIVAL ACTIONS ===

void ATranspersonalCharacter::ConsumeFood(float NutritionValue)
{
    if (SurvivalComp)
    {
        SurvivalComp->AddHunger(NutritionValue);
        CurrentHunger = SurvivalComp->GetHunger();
    }
}

void ATranspersonalCharacter::ConsumeWater(float HydrationValue)
{
    if (SurvivalComp)
    {
        SurvivalComp->AddThirst(HydrationValue);
        CurrentThirst = SurvivalComp->GetThirst();
    }
}

void ATranspersonalCharacter::TakeDamage_Survival(float DamageAmount, AActor* DamageCauser)
{
    if (SurvivalComp)
    {
        SurvivalComp->DrainHealth(DamageAmount);
        CurrentHealth = SurvivalComp->GetHealth();

        // Fear spike from taking damage
        ApplyFear(DamageAmount * 0.5f);
    }
}

void ATranspersonalCharacter::ApplyFear(float FearAmount)
{
    if (SurvivalComp)
    {
        SurvivalComp->AddFear(FearAmount);
        CurrentFear = SurvivalComp->GetFear();
    }
}

bool ATranspersonalCharacter::IsAlive() const
{
    if (SurvivalComp)
    {
        return SurvivalComp->GetHealth() > 0.0f;
    }
    return CurrentHealth > 0.0f;
}

// === BIOME INTEGRATION ===

void ATranspersonalCharacter::SetBiomeModifiers(float MovementMod, float HungerMod, float ThirstMod)
{
    BiomeMovementModifier = FMath::Clamp(MovementMod, 0.1f, 2.0f);
    BiomeHungerDrainModifier = FMath::Clamp(HungerMod, 0.5f, 3.0f);
    BiomeThirstDrainModifier = FMath::Clamp(ThirstMod, 0.5f, 3.0f);
}
