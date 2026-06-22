// TranspersonalCharacter.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260622_007
// Full ACharacter implementation: WASD movement, camera boom, survival stats

#include "TranspersonalCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    // Camera boom (third-person)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 8.f;
    CameraBoom->CameraLagMaxDistance = 200.f;
    CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 60.f));

    // Follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Character movement config
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
    MoveComp->JumpZVelocity = 600.f;
    MoveComp->AirControl = 0.2f;
    MoveComp->MaxWalkSpeed = 400.f;
    MoveComp->MaxWalkSpeedCrouched = 200.f;
    MoveComp->NavAgentProps.bCanCrouch = true;

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

    bIsSprinting = false;
    bIsCrouching = false;
    WalkSpeed = 400.f;
    SprintSpeed = 700.f;
    CrouchSpeed = 200.f;

    // Drain rates per second
    HungerDrainRate = 0.5f;
    ThirstDrainRate = 0.8f;
    StaminaRecoveryRate = 10.f;
    StaminaSprintCost = 15.f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Start survival tick timer (every 1 second)
    GetWorldTimerManager().SetTimer(
        SurvivalTimerHandle,
        this,
        &ATranspersonalCharacter::TickSurvivalStats,
        1.0f,
        true
    );

    // Bind Enhanced Input if available
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

    // Sprint stamina drain
    if (bIsSprinting && GetVelocity().SizeSquared() > 100.f)
    {
        Stamina = FMath::Max(0.f, Stamina - StaminaSprintCost * DeltaTime);
        if (Stamina <= 0.f)
        {
            StopSprinting();
        }
    }
    else
    {
        // Recover stamina when not sprinting
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRecoveryRate * DeltaTime);
    }
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Try Enhanced Input first
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
            EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::StartSprinting);
            EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopSprinting);
        }
        if (CrouchAction)
        {
            EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::ToggleCrouch);
        }
    }
    else
    {
        // Legacy input fallback (always works without Enhanced Input setup)
        PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
        PlayerInputComponent->BindAxis("MoveRight", this, &ATranspersonalCharacter::MoveRight);
        PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
        PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
        PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
        PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
        PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ATranspersonalCharacter::StartSprinting);
        PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprinting);
        PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ATranspersonalCharacter::ToggleCrouch);
    }
}

// ── Enhanced Input handlers ──────────────────────────────────────────────────

void ATranspersonalCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
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
    const FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

// ── Legacy input handlers ────────────────────────────────────────────────────

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

// ── Sprint / Crouch ──────────────────────────────────────────────────────────

void ATranspersonalCharacter::StartSprinting()
{
    if (Stamina > 10.f)
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
    if (bIsCrouching)
    {
        UnCrouch();
        bIsCrouching = false;
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
    else
    {
        Crouch();
        bIsCrouching = true;
        GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
    }
}

// ── Survival stats ───────────────────────────────────────────────────────────

void ATranspersonalCharacter::TickSurvivalStats()
{
    // Drain hunger and thirst over time
    Hunger = FMath::Max(0.f, Hunger - HungerDrainRate);
    Thirst = FMath::Max(0.f, Thirst - ThirstDrainRate);

    // Health damage when starving or dehydrated
    if (Hunger <= 0.f || Thirst <= 0.f)
    {
        TakeDamage(2.f, FDamageEvent(), GetController(), this);
    }

    // Fear decay
    Fear = FMath::Max(0.f, Fear - 1.f);
}

float ATranspersonalCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    Health = FMath::Max(0.f, Health - ActualDamage);

    if (Health <= 0.f)
    {
        OnDeath();
    }

    return ActualDamage;
}

void ATranspersonalCharacter::OnDeath()
{
    // Disable input on death
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        DisableInput(PC);
    }
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Ragdoll on death
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ATranspersonalCharacter::AddFear(float Amount)
{
    Fear = FMath::Clamp(Fear + Amount, 0.f, MaxFear);
}

void ATranspersonalCharacter::Eat(float NutritionValue)
{
    Hunger = FMath::Min(MaxHunger, Hunger + NutritionValue);
}

void ATranspersonalCharacter::Drink(float HydrationValue)
{
    Thirst = FMath::Min(MaxThirst, Thirst + HydrationValue);
}
