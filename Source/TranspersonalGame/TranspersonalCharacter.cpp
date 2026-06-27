// TranspersonalCharacter.cpp
// Engine Architect #02 — Cycle 020
// Full ACharacter implementation: WASD movement, sprint, jump, camera boom, survival stats

#include "TranspersonalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    // Camera boom (spring arm)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 8.f;
    CameraBoom->CameraLagMaxDistance = 50.f;
    CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 60.f));

    // Follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Character movement settings
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

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

    WalkSpeed = 300.f;
    SprintSpeed = 600.f;
    bIsSprinting = false;
    bIsAlive = true;

    // Drain rates per second
    HungerDrainRate = 0.5f;
    ThirstDrainRate = 0.8f;
    StaminaRegenRate = 10.f;
    StaminaDrainRate = 20.f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Start survival stat drain timer (every 1 second)
    GetWorld()->GetTimerManager().SetTimer(
        SurvivalTimerHandle,
        this,
        &ATranspersonalCharacter::UpdateSurvivalStats,
        1.0f,
        true
    );

    // Add Enhanced Input mapping context if available
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

    // Stamina management during sprint
    if (bIsSprinting && GetCharacterMovement()->Velocity.SizeSquared() > 100.f)
    {
        Stamina = FMath::Clamp(Stamina - StaminaDrainRate * DeltaTime, 0.f, MaxStamina);
        if (Stamina <= 0.f)
        {
            StopSprint();
        }
    }
    else if (!bIsSprinting && Stamina < MaxStamina)
    {
        Stamina = FMath::Clamp(Stamina + StaminaRegenRate * DeltaTime, 0.f, MaxStamina);
    }
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Move
        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Move);
        }

        // Look
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
    }
    else
    {
        // Legacy input fallback
        PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
        PlayerInputComponent->BindAxis("MoveRight", this, &ATranspersonalCharacter::MoveRight);
        PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
        PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
        PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
        PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
        PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ATranspersonalCharacter::StartSprint);
        PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprint);
    }
}

// Enhanced Input handlers
void ATranspersonalCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

        const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDir, MovementVector.Y);
        AddMovementInput(RightDir,   MovementVector.X);
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

// Legacy input handlers
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

void ATranspersonalCharacter::StartSprint()
{
    if (Stamina > 10.f && bIsAlive)
    {
        bIsSprinting = true;
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    }
}

void ATranspersonalCharacter::StopSprint()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ATranspersonalCharacter::UpdateSurvivalStats()
{
    if (!bIsAlive) return;

    // Drain hunger and thirst over time
    Hunger = FMath::Clamp(Hunger - HungerDrainRate, 0.f, MaxHunger);
    Thirst = FMath::Clamp(Thirst - ThirstDrainRate, 0.f, MaxThirst);

    // Starvation / dehydration damage
    if (Hunger <= 0.f || Thirst <= 0.f)
    {
        TakeSurvivalDamage(2.f);
    }

    // Death check
    if (Health <= 0.f)
    {
        Die();
    }
}

void ATranspersonalCharacter::TakeSurvivalDamage(float DamageAmount)
{
    if (!bIsAlive) return;
    Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);
}

void ATranspersonalCharacter::Die()
{
    if (!bIsAlive) return;
    bIsAlive = false;
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // Ragdoll on death
    GetMesh()->SetSimulatePhysics(true);
    GetWorld()->GetTimerManager().ClearTimer(SurvivalTimerHandle);
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
