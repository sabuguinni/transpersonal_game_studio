// TranspersonalCharacter.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260627_009
// Full implementation of the playable prehistoric survival character.

#include "TranspersonalCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule size for a primitive human
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Camera boom (spring arm)
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

    // Character movement tuning for prehistoric human
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        MoveComp->JumpZVelocity = 600.0f;
        MoveComp->AirControl = 0.2f;
        MoveComp->MaxWalkSpeed = 400.0f;       // Normal walk
        MoveComp->MaxWalkSpeedCrouched = 200.0f;
        MoveComp->NavAgentProps.bCanCrouch = true;
    }

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Survival stats defaults
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

    bIsRunning = false;
    bIsCrouching = false;
    bIsAlive = true;

    // Stat drain rates (per second)
    HungerDrainRate = 0.5f;
    ThirstDrainRate = 0.8f;
    StaminaRecoveryRate = 10.0f;
    StaminaDrainRate = 15.0f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Start survival stat tick timer (every 1 second)
    GetWorld()->GetTimerManager().SetTimer(
        SurvivalTimerHandle,
        this,
        &ATranspersonalCharacter::TickSurvivalStats,
        1.0f,
        true
    );
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stamina drain while running
    if (bIsRunning && GetVelocity().SizeSquared() > 100.0f)
    {
        Stamina = FMath::Clamp(Stamina - StaminaDrainRate * DeltaTime, 0.0f, MaxStamina);
        if (Stamina <= 0.0f)
        {
            StopRunning();
        }
    }
    else if (!bIsRunning)
    {
        // Recover stamina when not running
        Stamina = FMath::Clamp(Stamina + StaminaRecoveryRate * DeltaTime, 0.0f, MaxStamina);
    }
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Axis bindings
    PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ATranspersonalCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

    // Action bindings
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ATranspersonalCharacter::StartRunning);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopRunning);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ATranspersonalCharacter::ToggleCrouch);
}

void ATranspersonalCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ATranspersonalCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ATranspersonalCharacter::StartRunning()
{
    if (Stamina > 10.0f)
    {
        bIsRunning = true;
        UCharacterMovementComponent* MoveComp = GetCharacterMovement();
        if (MoveComp)
        {
            MoveComp->MaxWalkSpeed = 700.0f;
        }
    }
}

void ATranspersonalCharacter::StopRunning()
{
    bIsRunning = false;
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 400.0f;
    }
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

void ATranspersonalCharacter::TickSurvivalStats()
{
    if (!bIsAlive) return;

    // Drain hunger and thirst over time
    Hunger = FMath::Clamp(Hunger - HungerDrainRate, 0.0f, MaxHunger);
    Thirst = FMath::Clamp(Thirst - ThirstDrainRate, 0.0f, MaxThirst);

    // Starvation/dehydration damage
    if (Hunger <= 0.0f)
    {
        ApplyDamage(2.0f);
    }
    if (Thirst <= 0.0f)
    {
        ApplyDamage(3.0f);
    }

    // Fear decay over time
    Fear = FMath::Clamp(Fear - 1.0f, 0.0f, MaxFear);
}

void ATranspersonalCharacter::ApplyDamage(float DamageAmount)
{
    if (!bIsAlive) return;

    Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);

    if (Health <= 0.0f)
    {
        Die();
    }
}

void ATranspersonalCharacter::Heal(float HealAmount)
{
    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);
}

void ATranspersonalCharacter::Eat(float NutritionValue)
{
    Hunger = FMath::Clamp(Hunger + NutritionValue, 0.0f, MaxHunger);
}

void ATranspersonalCharacter::Drink(float HydrationValue)
{
    Thirst = FMath::Clamp(Thirst + HydrationValue, 0.0f, MaxThirst);
}

void ATranspersonalCharacter::AddFear(float FearAmount)
{
    Fear = FMath::Clamp(Fear + FearAmount, 0.0f, MaxFear);
}

void ATranspersonalCharacter::Die()
{
    bIsAlive = false;
    GetWorld()->GetTimerManager().ClearTimer(SurvivalTimerHandle);

    // Ragdoll on death
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
