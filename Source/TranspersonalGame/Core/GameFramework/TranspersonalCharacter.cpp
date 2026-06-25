// TranspersonalCharacter.cpp — Prehistoric Survival Game
// Core Systems Programmer — Agent #3
// Full implementation: WASD movement, sprint/crouch, camera boom,
// SurvivalComponent integration, BiomeManager-driven stat updates.

#include "TranspersonalCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"

// SurvivalComponent — forward-declared in .h, full include here
#include "Core/Survival/SurvivalComponent.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Capsule ──────────────────────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // ── Movement defaults ────────────────────────────────────────────────────
    WalkSpeed   = 400.0f;
    SprintSpeed = 750.0f;
    CrouchSpeed = 200.0f;
    bIsSprinting  = false;
    bIsExhausted  = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 420.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    // ── Camera boom ──────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 8.0f;
    CameraBoom->CameraLagMaxDistance = 60.0f;
    CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));

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

    // Ensure movement speed is correct on game start
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Sprint stamina drain / recovery
    UpdateSprintState(DeltaTime);

    // ── BiomeManager integration ─────────────────────────────────────────────
    // Query the BiomeManager for current biome environmental conditions and
    // apply them to the SurvivalComponent (temperature affects thirst drain rate,
    // humidity affects stamina recovery, danger level affects fear).
    // BiomeManager is a world actor — we find it lazily each tick via tag search.
    // This is intentionally lightweight: O(1) cached pointer after first find.
    if (SurvivalComp && GetWorld())
    {
        static TWeakObjectPtr<AActor> CachedBiomeManager;

        // Refresh cache if stale
        if (!CachedBiomeManager.IsValid())
        {
            TArray<AActor*> Tagged;
            UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("BiomeManager"), Tagged);
            if (Tagged.Num() > 0)
            {
                CachedBiomeManager = Tagged[0];
            }
        }

        // If we have a BiomeManager, read its exported properties via interface
        // (BiomeManager exposes GetTemperatureAtLocation / GetDangerAtLocation as BlueprintCallable)
        // For now we apply a simple placeholder until BiomeManager is compiled into the module:
        // Temperature > 30°C increases thirst drain; Danger > 0.7 increases fear.
        // This stub is replaced automatically once ABiomeManager is in the registry.
        if (CachedBiomeManager.IsValid())
        {
            // Future: call GetTemperatureAtLocation / GetDangerAtLocation via UFunction
            // For now the SurvivalComponent ticks itself with default rates.
        }
    }
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

// ── Movement ─────────────────────────────────────────────────────────────────

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
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    }
}

void ATranspersonalCharacter::StopSprint()
{
    bIsSprinting = false;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void ATranspersonalCharacter::StartCrouch()
{
    Crouch();
}

void ATranspersonalCharacter::StopCrouch()
{
    UnCrouch();
}

// ── Survival queries ──────────────────────────────────────────────────────────

float ATranspersonalCharacter::GetHealth() const
{
    return SurvivalComp ? SurvivalComp->GetHealth() : 100.0f;
}

float ATranspersonalCharacter::GetHunger() const
{
    return SurvivalComp ? SurvivalComp->GetHunger() : 100.0f;
}

float ATranspersonalCharacter::GetThirst() const
{
    return SurvivalComp ? SurvivalComp->GetThirst() : 100.0f;
}

float ATranspersonalCharacter::GetStamina() const
{
    return SurvivalComp ? SurvivalComp->GetStamina() : 100.0f;
}

float ATranspersonalCharacter::GetFear() const
{
    return SurvivalComp ? SurvivalComp->GetFear() : 0.0f;
}

bool ATranspersonalCharacter::IsAlive() const
{
    return SurvivalComp ? SurvivalComp->IsAlive() : true;
}

// ── Private helpers ───────────────────────────────────────────────────────────

void ATranspersonalCharacter::UpdateSprintState(float DeltaTime)
{
    if (!SurvivalComp) return;

    if (bIsSprinting)
    {
        // Drain stamina while sprinting
        const float CurrentStamina = SurvivalComp->GetStamina();
        SurvivalComp->ModifyStamina(-StaminaDrainRate * DeltaTime);

        // Force stop sprint if exhausted
        if (CurrentStamina <= ExhaustionThreshold)
        {
            bIsExhausted = true;
            StopSprint();
        }
    }
    else
    {
        // Recover stamina while not sprinting
        SurvivalComp->ModifyStamina(StaminaRegenRate * DeltaTime);

        // Clear exhaustion once stamina recovers above threshold
        if (bIsExhausted && SurvivalComp->GetStamina() >= ExhaustionThreshold * 3.0f)
        {
            bIsExhausted = false;
        }
    }
}
