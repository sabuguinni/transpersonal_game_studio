#include "TranspersonalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"

// SurvivalComponent — forward-declared in .h, included here for full definition
// NOTE: If SurvivalComponent lives in a separate module, add that module to Build.cs.
// For now we use a minimal inline stub so this file compiles independently.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

// ---------------------------------------------------------------------------
// Minimal SurvivalComponent stub (replace with real include once module exists)
// ---------------------------------------------------------------------------
#ifndef SURVIVAL_COMPONENT_DEFINED
#define SURVIVAL_COMPONENT_DEFINED

#include "UObject/ObjectMacros.h"

// We cannot redefine USurvivalComponent here if it already exists in another TU.
// Use a compile guard — if the real header is available it takes precedence.
#if !defined(SURVIVAL_COMPONENT_FULL_HEADER)

class USurvivalComponent : public UActorComponent
{
public:
    float Health   = 100.f;
    float Hunger   = 100.f;
    float Thirst   = 100.f;
    float Stamina  = 100.f;
    float Fear     = 0.f;

    float GetHealth()  const { return Health; }
    float GetHunger()  const { return Hunger; }
    float GetThirst()  const { return Thirst; }
    float GetStamina() const { return Stamina; }
    float GetFear()    const { return Fear; }
    bool  IsAlive()    const { return Health > 0.f; }

    void DrainStamina(float Amount) { Stamina = FMath::Max(0.f, Stamina - Amount); }
    void RegenStamina(float Amount) { Stamina = FMath::Min(100.f, Stamina + Amount); }
};

#endif // SURVIVAL_COMPONENT_FULL_HEADER
#endif // SURVIVAL_COMPONENT_DEFINED

// ---------------------------------------------------------------------------
// ATranspersonalCharacter
// ---------------------------------------------------------------------------

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Movement defaults ────────────────────────────────────────────────────
    WalkSpeed   = 400.f;
    SprintSpeed = 800.f;
    CrouchSpeed = 200.f;
    bIsSprinting  = false;
    bIsExhausted  = false;

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed         = WalkSpeed;
        MoveComp->JumpZVelocity        = 420.f;
        MoveComp->AirControl           = 0.35f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate         = FRotator(0.f, 540.f, 0.f);
        MoveComp->GravityScale         = 1.2f;
    }

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    // ── Camera boom ──────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength         = 400.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset            = FVector(0.f, 0.f, 60.f);

    // ── Follow camera ────────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ── Survival component ───────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Ensure movement speed is synced with properties (may be overridden in BP)
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
    }
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateSprintState(DeltaTime);
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Axis bindings
    PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn",        this, &ATranspersonalCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp",      this, &ATranspersonalCharacter::LookUp);

    // Action bindings
    PlayerInputComponent->BindAction("Jump",   IE_Pressed,  this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump",   IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &ATranspersonalCharacter::StartSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprint);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed,  this, &ATranspersonalCharacter::StartCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ATranspersonalCharacter::StopCrouch);
}

// ── Movement handlers ────────────────────────────────────────────────────────

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

void ATranspersonalCharacter::Turn(float Value)
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

    bIsSprinting = true;
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = SprintSpeed;
    }
}

void ATranspersonalCharacter::StopSprint()
{
    bIsSprinting = false;
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
    }
}

void ATranspersonalCharacter::StartCrouch()
{
    Crouch();
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = CrouchSpeed;
    }
}

void ATranspersonalCharacter::StopCrouch()
{
    UnCrouch();
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
    }
}

// ── Sprint stamina management ────────────────────────────────────────────────

void ATranspersonalCharacter::UpdateSprintState(float DeltaTime)
{
    if (!SurvivalComp) return;

    if (bIsSprinting)
    {
        const bool bMoving = GetVelocity().SizeSquared() > 100.f;
        if (bMoving)
        {
            SurvivalComp->DrainStamina(StaminaDrainRate * DeltaTime);

            if (SurvivalComp->GetStamina() <= ExhaustionThreshold)
            {
                bIsExhausted = true;
                StopSprint();
            }
        }
    }
    else
    {
        // Regen stamina when not sprinting
        if (SurvivalComp->GetStamina() < 100.f)
        {
            SurvivalComp->RegenStamina(StaminaRegenRate * DeltaTime);
        }

        // Clear exhaustion once stamina recovers above 30%
        if (bIsExhausted && SurvivalComp->GetStamina() >= 30.f)
        {
            bIsExhausted = false;
        }
    }
}

// ── Survival queries ─────────────────────────────────────────────────────────

float ATranspersonalCharacter::GetHealth()  const { return SurvivalComp ? SurvivalComp->GetHealth()  : 0.f; }
float ATranspersonalCharacter::GetHunger()  const { return SurvivalComp ? SurvivalComp->GetHunger()  : 0.f; }
float ATranspersonalCharacter::GetThirst()  const { return SurvivalComp ? SurvivalComp->GetThirst()  : 0.f; }
float ATranspersonalCharacter::GetStamina() const { return SurvivalComp ? SurvivalComp->GetStamina() : 0.f; }
float ATranspersonalCharacter::GetFear()    const { return SurvivalComp ? SurvivalComp->GetFear()    : 0.f; }

bool ATranspersonalCharacter::IsAlive() const
{
    return SurvivalComp ? SurvivalComp->IsAlive() : false;
}
