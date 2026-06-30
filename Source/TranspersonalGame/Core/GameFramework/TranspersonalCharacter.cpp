// TranspersonalCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Full implementation: movement, survival stats tick, SurvivalComponent integration

#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->GravityScale = 1.5f;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Camera boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = FVector(0.f, 0.f, 60.f);

    // Follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Survival stats defaults
    Health = 100.f;
    MaxHealth = 100.f;
    Hunger = 100.f;
    MaxHunger = 100.f;
    Thirst = 100.f;
    MaxThirst = 100.f;
    Stamina = 100.f;
    MaxStamina = 100.f;
    Fear = 0.f;
    MaxFear = 100.f;

    // Depletion rates (per second)
    HungerDepletionRate = 0.5f;
    ThirstDepletionRate = 0.3f;
    StaminaDepletionRate = 10.f;   // when sprinting
    StaminaRecoveryRate = 5.f;     // when idle/walking
    FearDecayRate = 2.f;           // fear decays over time

    bIsSprinting = false;
    bIsAlive = true;

    // Survival tick interval
    SurvivalTickInterval = 1.0f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Add input mapping context
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    // Start survival tick timer
    GetWorldTimerManager().SetTimer(
        SurvivalTickHandle,
        this,
        &ATranspersonalCharacter::TickSurvivalStats,
        SurvivalTickInterval,
        true  // looping
    );
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Real-time stamina management (per frame, not per timer)
    if (bIsAlive)
    {
        UpdateStamina(DeltaTime);
    }
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (JumpAction)
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
        if (MoveAction)
        {
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Move);
        }
        if (LookAction)
        {
            EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Look);
        }
        if (SprintAction)
        {
            EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::StartSprint);
            EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopSprint);
        }
    }
}

void ATranspersonalCharacter::Move(const FInputActionValue& Value)
{
    if (!bIsAlive) return;

    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ATranspersonalCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ATranspersonalCharacter::StartSprint()
{
    if (!bIsAlive || Stamina <= 5.f) return;
    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = 900.f;
}

void ATranspersonalCharacter::StopSprint()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
}

void ATranspersonalCharacter::TickSurvivalStats()
{
    if (!bIsAlive) return;

    // Hunger depletion
    Hunger = FMath::Clamp(Hunger - HungerDepletionRate * SurvivalTickInterval, 0.f, MaxHunger);
    if (Hunger <= 0.f)
    {
        // Starvation: deal 2 HP/s damage
        ApplyDamage(2.f, nullptr);
    }

    // Thirst depletion
    Thirst = FMath::Clamp(Thirst - ThirstDepletionRate * SurvivalTickInterval, 0.f, MaxThirst);
    if (Thirst <= 0.f)
    {
        // Dehydration: deal 3 HP/s damage (more urgent than starvation)
        ApplyDamage(3.f, nullptr);
    }

    // Fear decay over time
    if (Fear > 0.f)
    {
        Fear = FMath::Clamp(Fear - FearDecayRate * SurvivalTickInterval, 0.f, MaxFear);
    }

    // Blueprint event for UI updates
    OnSurvivalStatsUpdated();
}

void ATranspersonalCharacter::UpdateStamina(float DeltaTime)
{
    if (bIsSprinting && GetVelocity().SizeSquared() > 100.f)
    {
        // Drain stamina while sprinting
        Stamina = FMath::Clamp(Stamina - StaminaDepletionRate * DeltaTime, 0.f, MaxStamina);
        if (Stamina <= 0.f)
        {
            // Force stop sprint when exhausted
            StopSprint();
        }
    }
    else
    {
        // Recover stamina when not sprinting
        Stamina = FMath::Clamp(Stamina + StaminaRecoveryRate * DeltaTime, 0.f, MaxStamina);
    }
}

float ATranspersonalCharacter::ApplyDamage(float DamageAmount, AActor* DamageCauser)
{
    if (!bIsAlive) return 0.f;

    Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);

    // Increase fear when taking damage from a causer (e.g., dinosaur attack)
    if (DamageCauser)
    {
        Fear = FMath::Clamp(Fear + 20.f, 0.f, MaxFear);
    }

    if (Health <= 0.f)
    {
        Die();
    }

    OnHealthChanged(Health, MaxHealth);
    return DamageAmount;
}

void ATranspersonalCharacter::HealCharacter(float HealAmount)
{
    if (!bIsAlive) return;
    Health = FMath::Clamp(Health + HealAmount, 0.f, MaxHealth);
    OnHealthChanged(Health, MaxHealth);
}

void ATranspersonalCharacter::ConsumeFood(float NutritionValue)
{
    Hunger = FMath::Clamp(Hunger + NutritionValue, 0.f, MaxHunger);
}

void ATranspersonalCharacter::ConsumeWater(float HydrationValue)
{
    Thirst = FMath::Clamp(Thirst + HydrationValue, 0.f, MaxThirst);
}

void ATranspersonalCharacter::AddFear(float FearAmount)
{
    Fear = FMath::Clamp(Fear + FearAmount, 0.f, MaxFear);
    // High fear reduces movement speed
    if (Fear > 70.f)
    {
        GetCharacterMovement()->MaxWalkSpeed = 600.f; // panic sprint
    }
}

void ATranspersonalCharacter::Die()
{
    if (!bIsAlive) return;
    bIsAlive = false;

    // Stop survival tick
    GetWorldTimerManager().ClearTimer(SurvivalTickHandle);

    // Disable input
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        DisableInput(PC);
    }

    // Enable ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    OnCharacterDied();
}

void ATranspersonalCharacter::OnSurvivalStatsUpdated_Implementation()
{
    // Blueprint override for UI updates
}

void ATranspersonalCharacter::OnHealthChanged_Implementation(float NewHealth, float NewMaxHealth)
{
    // Blueprint override for health bar updates
}

void ATranspersonalCharacter::OnCharacterDied_Implementation()
{
    // Blueprint override for death screen / respawn logic
}
