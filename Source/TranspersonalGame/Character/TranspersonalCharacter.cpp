// TranspersonalCharacter.cpp
// Engine Architect #02 — Cycle AUTO_20260702_001
// Prehistoric survival game — ACharacter subclass with WASD movement, survival stats, camera boom

#include "TranspersonalCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Camera boom — 3rd person follow camera
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 60.0f);

    // Follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Character movement config
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 500.0f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
    GetCharacterMovement()->GravityScale = 1.5f;

    // Don't rotate character with camera
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

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
    Temperature = 37.0f;

    // Sprint
    bIsSprinting = false;
    WalkSpeed = 400.0f;
    SprintSpeed = 700.0f;
    CrouchSpeed = 180.0f;

    // Drain rates per second
    HungerDrainRate = 0.5f;
    ThirstDrainRate = 0.8f;
    StaminaDrainRate = 5.0f;
    StaminaRegenRate = 8.0f;
    FearDecayRate = 2.0f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Add input mapping context
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
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

    // Start survival stat drain timer — tick every 1 second
    GetWorldTimerManager().SetTimer(
        SurvivalTimerHandle,
        this,
        &ATranspersonalCharacter::UpdateSurvivalStats,
        1.0f,
        true
    );
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stamina regeneration when not sprinting
    if (!bIsSprinting && Stamina < MaxStamina)
    {
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenRate * DeltaTime);
    }

    // Sprint stamina drain
    if (bIsSprinting && GetVelocity().SizeSquared() > 100.0f)
    {
        Stamina = FMath::Max(0.0f, Stamina - StaminaDrainRate * DeltaTime);
        if (Stamina <= 0.0f)
        {
            StopSprinting();
        }
    }

    // Fear natural decay
    if (Fear > 0.0f)
    {
        Fear = FMath::Max(0.0f, Fear - FearDecayRate * DeltaTime);
    }
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jump
        if (JumpAction)
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }

        // Move
        if (MoveAction)
        {
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Move);
        }

        // Look
        if (LookAction)
        {
            EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Look);
        }

        // Sprint
        if (SprintAction)
        {
            EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::StartSprinting);
            EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopSprinting);
        }

        // Crouch
        if (CrouchAction)
        {
            EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::ToggleCrouch);
        }

        // Interact
        if (InteractAction)
        {
            EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::Interact);
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

void ATranspersonalCharacter::StartSprinting()
{
    if (Stamina > 10.0f)
    {
        bIsSprinting = true;
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    }
}

void ATranspersonalCharacter::StopSprinting()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ATranspersonalCharacter::ToggleCrouch()
{
    if (bIsCrouched)
    {
        UnCrouch();
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
    else
    {
        Crouch();
        GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
    }
}

void ATranspersonalCharacter::Interact()
{
    // Raycast forward for interactable objects
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * 200.0f;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
    {
        if (AActor* HitActor = HitResult.GetActor())
        {
            // Broadcast interaction event — Blueprint handles specific logic
            OnInteract.Broadcast(HitActor);
        }
    }
}

void ATranspersonalCharacter::UpdateSurvivalStats()
{
    // Drain hunger and thirst over time
    Hunger = FMath::Max(0.0f, Hunger - HungerDrainRate);
    Thirst = FMath::Max(0.0f, Thirst - ThirstDrainRate);

    // If starving or dehydrated, apply health damage
    if (Hunger <= 0.0f)
    {
        ApplyDamage(1.0f, nullptr);
    }
    if (Thirst <= 0.0f)
    {
        ApplyDamage(2.0f, nullptr);
    }
}

void ATranspersonalCharacter::ApplyDamage(float DamageAmount, AActor* DamageCauser)
{
    Health = FMath::Max(0.0f, Health - DamageAmount);
    OnHealthChanged.Broadcast(Health, MaxHealth);

    if (Health <= 0.0f)
    {
        OnDeath.Broadcast();
    }
}

void ATranspersonalCharacter::HealCharacter(float HealAmount)
{
    Health = FMath::Min(MaxHealth, Health + HealAmount);
    OnHealthChanged.Broadcast(Health, MaxHealth);
}

void ATranspersonalCharacter::ConsumeFood(float FoodValue)
{
    Hunger = FMath::Min(MaxHunger, Hunger + FoodValue);
}

void ATranspersonalCharacter::DrinkWater(float WaterValue)
{
    Thirst = FMath::Min(MaxThirst, Thirst + WaterValue);
}

void ATranspersonalCharacter::AddFear(float FearAmount)
{
    Fear = FMath::Min(MaxFear, Fear + FearAmount);

    // High fear causes movement speed penalty
    if (Fear > 70.0f)
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 0.85f;
    }
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
