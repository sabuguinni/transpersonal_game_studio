// TranspersonalCharacter.cpp
// Transpersonal Game Studio — Prehistoric Survival Game
// Agent #03 / #04 implementation — ATranspersonalCharacter

#include "TranspersonalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Camera boom — 3rd person offset
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 8.f;

    // Follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->JumpZVelocity = 420.f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Survival stats defaults
    SurvivalStats.Health = 100.f;
    SurvivalStats.MaxHealth = 100.f;
    SurvivalStats.Hunger = 100.f;
    SurvivalStats.MaxHunger = 100.f;
    SurvivalStats.Thirst = 100.f;
    SurvivalStats.MaxThirst = 100.f;
    SurvivalStats.Stamina = 100.f;
    SurvivalStats.MaxStamina = 100.f;
    SurvivalStats.Fear = 0.f;
    SurvivalStats.Temperature = 37.f;
    SurvivalStats.bIsDead = false;

    bIsSprinting = false;
    WalkSpeed = 300.f;
    SprintSpeed = 600.f;
    SurvivalTickInterval = 5.f;
    SurvivalTickAccumulator = 0.f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (SurvivalStats.bIsDead)
    {
        return;
    }

    // Stamina drain / recovery
    if (bIsSprinting && GetVelocity().SizeSquared() > 100.f)
    {
        SurvivalStats.Stamina = FMath::Max(0.f, SurvivalStats.Stamina - StaminaDrainRate * DeltaTime);
        if (SurvivalStats.Stamina <= 0.f)
        {
            StopSprinting();
        }
    }
    else
    {
        SurvivalStats.Stamina = FMath::Min(SurvivalStats.MaxStamina,
            SurvivalStats.Stamina + StaminaRecoveryRate * DeltaTime);
    }

    // Survival tick (hunger/thirst drain at interval)
    SurvivalTickAccumulator += DeltaTime;
    if (SurvivalTickAccumulator >= SurvivalTickInterval)
    {
        SurvivalTickAccumulator = 0.f;
        TickSurvivalStats(SurvivalTickInterval);
    }
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

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

void ATranspersonalCharacter::StartSprinting()
{
    if (SurvivalStats.Stamina > 10.f && !SurvivalStats.bIsDead)
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
    }
    else
    {
        Crouch();
    }
}

void ATranspersonalCharacter::ApplyDamage(float DamageAmount)
{
    if (SurvivalStats.bIsDead)
    {
        return;
    }

    SurvivalStats.Health = FMath::Max(0.f, SurvivalStats.Health - DamageAmount);

    if (SurvivalStats.Health <= 0.f)
    {
        Die();
    }
}

void ATranspersonalCharacter::Heal(float HealAmount)
{
    if (!SurvivalStats.bIsDead)
    {
        SurvivalStats.Health = FMath::Min(SurvivalStats.MaxHealth, SurvivalStats.Health + HealAmount);
    }
}

void ATranspersonalCharacter::Eat(float NutritionValue)
{
    SurvivalStats.Hunger = FMath::Min(SurvivalStats.MaxHunger, SurvivalStats.Hunger + NutritionValue);
}

void ATranspersonalCharacter::Drink(float HydrationValue)
{
    SurvivalStats.Thirst = FMath::Min(SurvivalStats.MaxThirst, SurvivalStats.Thirst + HydrationValue);
}

void ATranspersonalCharacter::SetFearLevel(float NewFear)
{
    SurvivalStats.Fear = FMath::Clamp(NewFear, 0.f, 100.f);

    // High fear reduces movement speed slightly (adrenaline trade-off)
    if (SurvivalStats.Fear > 75.f && !bIsSprinting)
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 0.85f;
    }
    else if (!bIsSprinting)
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

bool ATranspersonalCharacter::IsAlive() const
{
    return !SurvivalStats.bIsDead;
}

void ATranspersonalCharacter::Die()
{
    if (SurvivalStats.bIsDead)
    {
        return;
    }

    SurvivalStats.bIsDead = true;
    bIsSprinting = false;

    // Disable movement
    GetCharacterMovement()->DisableMovement();
    GetCharacterMovement()->StopMovementImmediately();

    // Ragdoll on death
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetSimulatePhysics(true);

    // Disable input
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->DisableInput(PC);
    }
}

void ATranspersonalCharacter::TickSurvivalStats(float Elapsed)
{
    // Hunger drains over time
    SurvivalStats.Hunger = FMath::Max(0.f, SurvivalStats.Hunger - HungerDrainRate * Elapsed);

    // Thirst drains faster than hunger
    SurvivalStats.Thirst = FMath::Max(0.f, SurvivalStats.Thirst - ThirstDrainRate * Elapsed);

    // Starvation damage
    if (SurvivalStats.Hunger <= 0.f)
    {
        ApplyDamage(StarvationDamageRate * Elapsed);
    }

    // Dehydration damage (faster than starvation)
    if (SurvivalStats.Thirst <= 0.f)
    {
        ApplyDamage(DehydrationDamageRate * Elapsed);
    }
}
