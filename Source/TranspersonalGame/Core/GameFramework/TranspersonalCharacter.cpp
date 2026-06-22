// TranspersonalCharacter.cpp
// Player character — primitive human survivor
// Core Systems Programmer — Agent #3

#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule — human proportions
    GetCapsuleComponent()->InitCapsuleSize(34.0f, 88.0f);

    // Movement — primitive human
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 300.0f;          // Walk
        MoveComp->MaxWalkSpeedCrouched = 150.0f;
        MoveComp->JumpZVelocity = 420.0f;
        MoveComp->AirControl = 0.2f;
        MoveComp->GravityScale = 1.0f;
        MoveComp->Mass = 75.0f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        MoveComp->NavAgentProps.bCanCrouch = true;
    }

    // Camera boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 8.0f;
    CameraBoom->CameraLagMaxDistance = 200.0f;

    // Follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Survival stats — initial values
    Health = 100.0f;
    MaxHealth = 100.0f;
    Hunger = 100.0f;
    MaxHunger = 100.0f;
    Thirst = 100.0f;
    MaxThirst = 100.0f;
    Stamina = 100.0f;
    MaxStamina = 100.0f;
    Fear = 0.0f;
    MaxFear = 100.0f;
    Temperature = 37.0f;  // Body temperature Celsius

    // Drain rates per second
    HungerDrainRate = 0.02f;   // ~83 minutes to starve at rest
    ThirstDrainRate = 0.04f;   // ~42 minutes to die of thirst
    StaminaDrainRate = 5.0f;   // Drains while sprinting
    StaminaRegenRate = 3.0f;   // Regens while not sprinting
    FearDecayRate = 0.5f;      // Fear decays over time

    bIsSprinting = false;
    bIsDead = false;
    SprintSpeedMultiplier = 2.0f;
    NormalWalkSpeed = 300.0f;
    SprintSpeed = 600.0f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Survival stat tick — every 1 second
    GetWorldTimerManager().SetTimer(
        SurvivalTickHandle,
        this,
        &ATranspersonalCharacter::TickSurvivalStats,
        1.0f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("TranspersonalCharacter [%s] spawned — survival systems active"),
        *GetActorLabel());
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead) return;

    // Stamina management
    if (bIsSprinting)
    {
        Stamina = FMath::Max(0.0f, Stamina - StaminaDrainRate * DeltaTime);
        if (Stamina <= 0.0f)
        {
            StopSprinting();
        }
    }
    else
    {
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenRate * DeltaTime);
    }

    // Fear decay
    if (Fear > 0.0f)
    {
        Fear = FMath::Max(0.0f, Fear - FearDecayRate * DeltaTime);
    }
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Movement axes
    PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ATranspersonalCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

    // Actions
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ATranspersonalCharacter::StartSprinting);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprinting);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ATranspersonalCharacter::ToggleCrouch);
}

void ATranspersonalCharacter::MoveForward(float Value)
{
    if (bIsDead || FMath::IsNearlyZero(Value)) return;

    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
    const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    AddMovementInput(Direction, Value);
}

void ATranspersonalCharacter::MoveRight(float Value)
{
    if (bIsDead || FMath::IsNearlyZero(Value)) return;

    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
    const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    AddMovementInput(Direction, Value);
}

void ATranspersonalCharacter::StartSprinting()
{
    if (bIsDead || Stamina <= 10.0f) return;

    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    UE_LOG(LogTemp, Verbose, TEXT("TranspersonalCharacter: SPRINT start"));
}

void ATranspersonalCharacter::StopSprinting()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
    UE_LOG(LogTemp, Verbose, TEXT("TranspersonalCharacter: SPRINT stop"));
}

void ATranspersonalCharacter::ToggleCrouch()
{
    if (bIsCrouched)
    {
        UnCrouch();
    }
    else
    {
        Crouch();
    }
}

void ATranspersonalCharacter::TickSurvivalStats()
{
    if (bIsDead) return;

    // Drain hunger
    Hunger = FMath::Max(0.0f, Hunger - HungerDrainRate * 60.0f); // per minute equivalent

    // Drain thirst
    Thirst = FMath::Max(0.0f, Thirst - ThirstDrainRate * 60.0f);

    // Starvation damage
    if (Hunger <= 0.0f)
    {
        ApplyDamage_Internal(2.0f, TEXT("Starvation"));
    }

    // Dehydration damage
    if (Thirst <= 0.0f)
    {
        ApplyDamage_Internal(5.0f, TEXT("Dehydration")); // Dehydration kills faster
    }

    // Log survival state every 30 seconds (every 30 ticks)
    static int32 TickCount = 0;
    TickCount++;
    if (TickCount % 30 == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Survival [%s] — HP:%.0f Hunger:%.0f Thirst:%.0f Stamina:%.0f Fear:%.0f"),
            *GetActorLabel(), Health, Hunger, Thirst, Stamina, Fear);
    }
}

void ATranspersonalCharacter::ApplyDamage_Internal(float Amount, const FString& Cause)
{
    Health = FMath::Max(0.0f, Health - Amount);

    UE_LOG(LogTemp, Warning, TEXT("TranspersonalCharacter [%s] took %.0f damage from %s — HP: %.0f/%.0f"),
        *GetActorLabel(), Amount, *Cause, Health, MaxHealth);

    if (Health <= 0.0f)
    {
        OnDeath();
    }
}

float ATranspersonalCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // Fear spike when attacked by dinosaur
    if (DamageCauser)
    {
        Fear = FMath::Min(MaxFear, Fear + DamageAmount * 0.5f);
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalCharacter attacked by [%s] — Fear: %.0f"),
            *DamageCauser->GetActorLabel(), Fear);
    }

    ApplyDamage_Internal(ActualDamage, TEXT("Combat"));
    return ActualDamage;
}

void ATranspersonalCharacter::OnDeath()
{
    if (bIsDead) return;
    bIsDead = true;

    UE_LOG(LogTemp, Error, TEXT("TranspersonalCharacter [%s] has DIED"), *GetActorLabel());

    // Stop survival tick
    GetWorldTimerManager().ClearTimer(SurvivalTickHandle);

    // Disable movement
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Notify game mode for respawn logic (future)
    // Future: ATranspersonalGameMode::OnPlayerDeath()
}

void ATranspersonalCharacter::Eat(float NutritionValue)
{
    Hunger = FMath::Min(MaxHunger, Hunger + NutritionValue);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalCharacter [%s] ate — Hunger: %.0f/%.0f"),
        *GetActorLabel(), Hunger, MaxHunger);
}

void ATranspersonalCharacter::Drink(float HydrationValue)
{
    Thirst = FMath::Min(MaxThirst, Thirst + HydrationValue);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalCharacter [%s] drank — Thirst: %.0f/%.0f"),
        *GetActorLabel(), Thirst, MaxThirst);
}

void ATranspersonalCharacter::Heal(float HealAmount)
{
    Health = FMath::Min(MaxHealth, Health + HealAmount);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalCharacter [%s] healed — HP: %.0f/%.0f"),
        *GetActorLabel(), Health, MaxHealth);
}
