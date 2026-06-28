// TranspersonalCharacter.cpp — Full implementation of prehistoric survival player character
// Agent #03 — Core Systems Programmer
// Implements: WASD movement, sprint/stamina drain, camera boom, SurvivalComponent integration

#include "TranspersonalCharacter.h"
#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ─── Capsule ──────────────────────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    // ─── Movement defaults ────────────────────────────────────────────────────
    WalkSpeed  = 300.f;
    RunSpeed   = 600.f;
    CrouchSpeed = 150.f;
    bIsSprinting = false;
    bIsExhausted = false;

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
        MoveComp->JumpZVelocity = 420.f;
        MoveComp->AirControl = 0.2f;
        MoveComp->MaxWalkSpeed = WalkSpeed;
        MoveComp->MaxWalkSpeedCrouched = CrouchSpeed;
        MoveComp->NavAgentProps.bCanCrouch = true;
    }

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    // ─── Camera Boom ──────────────────────────────────────────────────────────
    CameraArmLength = 400.f;
    CameraLagSpeed  = 10.f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = CameraArmLength;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = CameraLagSpeed;
    CameraBoom->CameraLagMaxDistance = 150.f;
    CameraBoom->SocketOffset = FVector(0.f, 0.f, 60.f);

    // ─── Follow Camera ────────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    FollowCamera->FieldOfView = 90.f;

    // ─── Survival Component ───────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Initialise movement speed from defaults
    UpdateMovementSpeed();

    if (SurvivalComp)
    {
        UE_LOG(LogTemp, Log, TEXT("TranspersonalCharacter: SurvivalComponent active — Health=%.1f Hunger=%.1f Thirst=%.1f"),
            SurvivalComp->GetHealth(),
            SurvivalComp->GetHunger(),
            SurvivalComp->GetThirst());
    }
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateStaminaDrain(DeltaTime);
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Axis bindings — works with both legacy and Enhanced Input
    PlayerInputComponent->BindAxis("MoveForward",  this, &ATranspersonalCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight",    this, &ATranspersonalCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn",         this, &ATranspersonalCharacter::TurnCamera);
    PlayerInputComponent->BindAxis("LookUp",       this, &ATranspersonalCharacter::LookUp);

    // Action bindings
    PlayerInputComponent->BindAction("Jump",   IE_Pressed,  this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump",   IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &ATranspersonalCharacter::StartSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprint);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed,  this, &ACharacter::Crouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ACharacter::UnCrouch);
}

// ─── Movement ─────────────────────────────────────────────────────────────────

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

void ATranspersonalCharacter::TurnCamera(float Value)
{
    AddControllerYawInput(Value);
}

void ATranspersonalCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ATranspersonalCharacter::StartSprint()
{
    if (bIsExhausted) return;

    // Check stamina before allowing sprint
    if (SurvivalComp && SurvivalComp->GetStamina() < 10.f)
    {
        bIsExhausted = true;
        return;
    }

    bIsSprinting = true;
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::StopSprint()
{
    bIsSprinting = false;
    UpdateMovementSpeed();
}

// ─── Private Helpers ──────────────────────────────────────────────────────────

void ATranspersonalCharacter::UpdateMovementSpeed()
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp) return;

    if (bIsSprinting && !bIsExhausted)
    {
        MoveComp->MaxWalkSpeed = RunSpeed;
    }
    else
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
    }
}

void ATranspersonalCharacter::UpdateStaminaDrain(float DeltaTime)
{
    if (!SurvivalComp) return;

    const bool bIsMoving = GetVelocity().SizeSquared() > 100.f;

    if (bIsSprinting && bIsMoving)
    {
        // Drain stamina while sprinting
        SurvivalComp->DrainStamina(15.f * DeltaTime);

        if (SurvivalComp->GetStamina() <= 0.f)
        {
            bIsExhausted = true;
            bIsSprinting = false;
            UpdateMovementSpeed();
        }
    }
    else
    {
        // Recover stamina when not sprinting
        SurvivalComp->RecoverStamina(8.f * DeltaTime);

        // Clear exhaustion when stamina recovers past 25%
        if (bIsExhausted && SurvivalComp->GetStamina() >= 25.f)
        {
            bIsExhausted = false;
        }
    }
}

// ─── Survival Interface ───────────────────────────────────────────────────────

float ATranspersonalCharacter::GetHealth() const
{
    return SurvivalComp ? SurvivalComp->GetHealth() : 0.f;
}

float ATranspersonalCharacter::GetHunger() const
{
    return SurvivalComp ? SurvivalComp->GetHunger() : 0.f;
}

float ATranspersonalCharacter::GetThirst() const
{
    return SurvivalComp ? SurvivalComp->GetThirst() : 0.f;
}

float ATranspersonalCharacter::GetStamina() const
{
    return SurvivalComp ? SurvivalComp->GetStamina() : 0.f;
}

bool ATranspersonalCharacter::IsAlive() const
{
    return SurvivalComp ? SurvivalComp->GetHealth() > 0.f : false;
}

void ATranspersonalCharacter::ApplyDamage_Survival(float DamageAmount, AActor* DamageSource)
{
    if (!SurvivalComp) return;

    SurvivalComp->ApplyDamage(DamageAmount);

    UE_LOG(LogTemp, Warning, TEXT("TranspersonalCharacter took %.1f damage from %s — Health remaining: %.1f"),
        DamageAmount,
        DamageSource ? *DamageSource->GetName() : TEXT("Unknown"),
        SurvivalComp->GetHealth());

    if (!IsAlive())
    {
        UE_LOG(LogTemp, Error, TEXT("TranspersonalCharacter DIED — triggering death sequence"));
        // Death handled by GameMode — broadcast death event
        OnDeath.Broadcast();
    }
}
