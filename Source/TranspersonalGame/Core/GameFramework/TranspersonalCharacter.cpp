// TranspersonalCharacter.cpp
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260625_004
// Prehistoric survival game — full implementation with SurvivalComponent

#include "Core/GameFramework/TranspersonalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Survival Component ────────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // ── Capsule ───────────────────────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    // ── Movement defaults ─────────────────────────────────────────────────────
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    // Don't rotate character with controller — camera handles that
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    // ── Camera Boom ───────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;

    // ── Follow Camera ─────────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Register Enhanced Input mapping context
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

    // Set initial walk speed
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead) return;

    // Drain stamina while sprinting; restore walk speed if stamina depleted
    if (bIsSprinting && SurvivalComp)
    {
        const float StaminaDrain = 20.f * DeltaTime; // 20 units/sec drain
        SurvivalComp->ConsumeStamina(StaminaDrain);

        if (SurvivalComp->GetStamina() <= 0.f)
        {
            StopSprint();
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Input Setup
// ─────────────────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this,
                            &ATranspersonalCharacter::Move);
        }
        if (LookAction)
        {
            EIC->BindAction(LookAction, ETriggerEvent::Triggered, this,
                            &ATranspersonalCharacter::Look);
        }
        if (JumpAction)
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &ACharacter::Jump);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
        if (SprintAction)
        {
            EIC->BindAction(SprintAction, ETriggerEvent::Started,   this,
                            &ATranspersonalCharacter::StartSprint);
            EIC->BindAction(SprintAction, ETriggerEvent::Completed, this,
                            &ATranspersonalCharacter::StopSprint);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Input Handlers
// ─────────────────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::Move(const FInputActionValue& Value)
{
    if (bIsDead) return;

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

void ATranspersonalCharacter::StartSprint()
{
    if (bIsDead) return;
    if (SurvivalComp && SurvivalComp->GetStamina() > 10.f)
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

// ─────────────────────────────────────────────────────────────────────────────
// Survival Accessors
// ─────────────────────────────────────────────────────────────────────────────

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

float ATranspersonalCharacter::GetFear() const
{
    return SurvivalComp ? SurvivalComp->GetFear() : 0.f;
}

// ─────────────────────────────────────────────────────────────────────────────
// Damage / Death
// ─────────────────────────────────────────────────────────────────────────────

float ATranspersonalCharacter::TakeDamage(float DamageAmount,
                                           struct FDamageEvent const& DamageEvent,
                                           AController* EventInstigator,
                                           AActor* DamageCauser)
{
    if (bIsDead) return 0.f;

    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent,
                                                  EventInstigator, DamageCauser);

    if (SurvivalComp)
    {
        SurvivalComp->ApplyDamage(ActualDamage);

        // Spike fear when attacked by a dinosaur
        SurvivalComp->AddFear(ActualDamage * 0.5f);

        if (SurvivalComp->GetHealth() <= 0.f)
        {
            Die();
        }
    }

    return ActualDamage;
}

void ATranspersonalCharacter::Die()
{
    if (bIsDead) return;
    bIsDead = true;

    // Disable input
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        DisableInput(PC);
    }

    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Respawn after 5 seconds (handled by GameMode in a full implementation)
    // For now, destroy after 10s so the level stays clean
    SetLifeSpan(10.f);
}
