// TranspersonalCharacter.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260622_006
// Prehistoric survival game — player character implementation

#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    // Camera boom — third person follow camera
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = FVector(0.f, 0.f, 80.f);

    // Follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Movement defaults — prehistoric human
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
    GetCharacterMovement()->JumpZVelocity = 420.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 400.f;       // Walk
    GetCharacterMovement()->MaxWalkSpeedCrouched = 200.f;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Survival stats — initial values
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

    bIsRunning = false;
    bIsCrouching = false;
    RunSpeedMultiplier = 1.8f;
    StaminaDrainRate = 10.f;
    StaminaRegenRate = 5.f;
    HungerDrainRate = 0.5f;
    ThirstDrainRate = 0.8f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Bind Enhanced Input
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
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateSurvivalStats(DeltaTime);
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Move);
        }
        if (LookAction)
        {
            EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Look);
        }
        if (JumpAction)
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
        if (SprintAction)
        {
            EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::StartSprint);
            EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopSprint);
        }
        if (CrouchAction)
        {
            EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::ToggleCrouch);
        }
    }
}

void ATranspersonalCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(ForwardDir, MovementVector.Y);
        AddMovementInput(RightDir, MovementVector.X);
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
    if (Stamina > 10.f)
    {
        bIsRunning = true;
        GetCharacterMovement()->MaxWalkSpeed = 400.f * RunSpeedMultiplier;
    }
}

void ATranspersonalCharacter::StopSprint()
{
    bIsRunning = false;
    GetCharacterMovement()->MaxWalkSpeed = 400.f;
}

void ATranspersonalCharacter::ToggleCrouch()
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

void ATranspersonalCharacter::UpdateSurvivalStats(float DeltaTime)
{
    // Hunger drains over time
    Hunger = FMath::Max(0.f, Hunger - HungerDrainRate * DeltaTime);

    // Thirst drains faster
    Thirst = FMath::Max(0.f, Thirst - ThirstDrainRate * DeltaTime);

    // Stamina: drain when sprinting, regen when idle
    if (bIsRunning && GetVelocity().SizeSquared() > 100.f)
    {
        Stamina = FMath::Max(0.f, Stamina - StaminaDrainRate * DeltaTime);
        if (Stamina <= 0.f)
        {
            StopSprint();
        }
    }
    else
    {
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenRate * DeltaTime);
    }

    // Health drains if starving or dehydrated
    if (Hunger <= 0.f || Thirst <= 0.f)
    {
        Health = FMath::Max(0.f, Health - 2.f * DeltaTime);
    }
}

float ATranspersonalCharacter::GetHealthPercent() const
{
    return MaxHealth > 0.f ? Health / MaxHealth : 0.f;
}

float ATranspersonalCharacter::GetHungerPercent() const
{
    return MaxHunger > 0.f ? Hunger / MaxHunger : 0.f;
}

float ATranspersonalCharacter::GetThirstPercent() const
{
    return MaxThirst > 0.f ? Thirst / MaxThirst : 0.f;
}

float ATranspersonalCharacter::GetStaminaPercent() const
{
    return MaxStamina > 0.f ? Stamina / MaxStamina : 0.f;
}

void ATranspersonalCharacter::ApplyDamage(float DamageAmount)
{
    Health = FMath::Max(0.f, Health - DamageAmount);
    // Increase fear when taking damage
    Fear = FMath::Min(MaxFear, Fear + DamageAmount * 0.5f);
}

void ATranspersonalCharacter::Eat(float NutritionValue)
{
    Hunger = FMath::Min(MaxHunger, Hunger + NutritionValue);
}

void ATranspersonalCharacter::Drink(float HydrationValue)
{
    Thirst = FMath::Min(MaxThirst, Thirst + HydrationValue);
}
