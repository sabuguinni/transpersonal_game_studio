// TranspersonalCharacter.cpp
// Engine Architect #02 — Cycle AUTO_005
// Prehistoric survival game — player character with WASD movement, camera boom, survival stats
// Milestone 1: Walk Around — fully functional third-person character

#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule size for a primitive human
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Don't rotate character with camera — only camera rotates
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Character movement configuration — Milestone 1 requirements
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
        MoveComp->JumpZVelocity = 700.f;
        MoveComp->AirControl = 0.35f;
        MoveComp->MaxWalkSpeed = WalkSpeed;
        MoveComp->MinAnalogWalkSpeed = 20.f;
        MoveComp->BrakingDecelerationWalking = 2000.f;
        MoveComp->GravityScale = 1.75f;
    }

    // Camera boom (spring arm) — follows character at distance
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = FVector(0.f, 0.f, 60.f);

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

    // Movement speeds
    WalkSpeed = 300.0f;
    RunSpeed = 600.0f;
    CrouchSpeed = 150.0f;

    bIsRunning = false;
    bIsCrouching = false;
    bIsAlive = true;

    // Drain rates per second
    HungerDrainRate = 0.5f;
    ThirstDrainRate = 0.8f;
    StaminaDrainRate = 10.0f;
    StaminaRegenRate = 5.0f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Add input mapping context for Enhanced Input
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
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsAlive) return;

    UpdateSurvivalStats(DeltaTime);
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Move — WASD
        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Move);
        }

        // Look — mouse
        if (LookAction)
        {
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Look);
        }

        // Jump
        if (JumpAction)
        {
            EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
            EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }

        // Sprint
        if (SprintAction)
        {
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::StartSprint);
            EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopSprint);
        }

        // Crouch
        if (CrouchAction)
        {
            EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::ToggleCrouch);
        }
    }
}

void ATranspersonalCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
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

    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ATranspersonalCharacter::StartSprint()
{
    if (Stamina > 10.0f)
    {
        bIsRunning = true;
        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->MaxWalkSpeed = RunSpeed;
        }
    }
}

void ATranspersonalCharacter::StopSprint()
{
    bIsRunning = false;
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
    }
}

void ATranspersonalCharacter::ToggleCrouch()
{
    bIsCrouching = !bIsCrouching;
    if (bIsCrouching)
    {
        Crouch();
        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->MaxWalkSpeed = CrouchSpeed;
        }
    }
    else
    {
        UnCrouch();
        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->MaxWalkSpeed = bIsRunning ? RunSpeed : WalkSpeed;
        }
    }
}

void ATranspersonalCharacter::UpdateSurvivalStats(float DeltaTime)
{
    // Drain hunger and thirst over time
    Hunger = FMath::Max(0.0f, Hunger - HungerDrainRate * DeltaTime);
    Thirst = FMath::Max(0.0f, Thirst - ThirstDrainRate * DeltaTime);

    // Stamina drain when running, regen when walking/idle
    if (bIsRunning && GetVelocity().SizeSquared() > 100.0f)
    {
        Stamina = FMath::Max(0.0f, Stamina - StaminaDrainRate * DeltaTime);
        if (Stamina <= 0.0f)
        {
            StopSprint();
        }
    }
    else
    {
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenRate * DeltaTime);
    }

    // Health drain if starving or dehydrated
    if (Hunger <= 0.0f || Thirst <= 0.0f)
    {
        TakeDamage(2.0f * DeltaTime, FDamageEvent(), nullptr, nullptr);
    }

    // Fear decays over time when not threatened
    Fear = FMath::Max(0.0f, Fear - 5.0f * DeltaTime);
}

float ATranspersonalCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    Health = FMath::Max(0.0f, Health - ActualDamage);

    if (Health <= 0.0f && bIsAlive)
    {
        bIsAlive = false;
        OnCharacterDied();
    }

    return ActualDamage;
}

void ATranspersonalCharacter::OnCharacterDied()
{
    // Disable movement
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }

    // Disable input
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        DisableInput(PC);
    }

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATranspersonalCharacter::AddFear(float Amount)
{
    Fear = FMath::Clamp(Fear + Amount, 0.0f, MaxFear);
}

void ATranspersonalCharacter::Eat(float NutritionValue)
{
    Hunger = FMath::Min(MaxHunger, Hunger + NutritionValue);
}

void ATranspersonalCharacter::Drink(float HydrationValue)
{
    Thirst = FMath::Min(MaxThirst, Thirst + HydrationValue);
}

bool ATranspersonalCharacter::IsAlive() const
{
    return bIsAlive;
}

float ATranspersonalCharacter::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
}

float ATranspersonalCharacter::GetHungerPercent() const
{
    return (MaxHunger > 0.0f) ? (Hunger / MaxHunger) : 0.0f;
}

float ATranspersonalCharacter::GetThirstPercent() const
{
    return (MaxThirst > 0.0f) ? (Thirst / MaxThirst) : 0.0f;
}

float ATranspersonalCharacter::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 0.0f;
}

float ATranspersonalCharacter::GetFearPercent() const
{
    return (MaxFear > 0.0f) ? (Fear / MaxFear) : 0.0f;
}
