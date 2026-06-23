// TranspersonalCharacter.cpp
// Transpersonal Game Studio — Core Systems Programmer (#03)
// Prehistoric survival game — player character implementation.
// Integrates SurvivalComponent, Enhanced Input, SpringArm/Camera.

#include "TranspersonalCharacter.h"
#include "Core/Survival/SurvivalComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Camera Boom ────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 10.0f;

    // ── Follow Camera ──────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ── Survival Component ─────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // ── Movement defaults ──────────────────────────────────────────────────
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->JumpZVelocity = 420.0f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->GravityScale = 1.75f;
    GetCharacterMovement()->MaxAcceleration = 2048.0f;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Register Enhanced Input mapping context
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

    // Drain stamina while sprinting; auto-stop sprint when stamina is empty
    if (bIsSprinting && SurvivalComp)
    {
        const float StaminaDrain = 15.0f * DeltaTime;
        SurvivalComp->ModifyStamina(-StaminaDrain);

        if (SurvivalComp->GetStamina() <= 0.0f)
        {
            StopSprint();
        }
    }
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
            EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &ACharacter::Jump);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
        if (SprintAction)
        {
            EIC->BindAction(SprintAction, ETriggerEvent::Started,   this, &ATranspersonalCharacter::HandleSprintStart);
            EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::HandleSprintStop);
        }
        if (CrouchAction)
        {
            EIC->BindAction(CrouchAction, ETriggerEvent::Started,   this, &ACharacter::Crouch);
            EIC->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ACharacter::UnCrouch);
        }
    }
}

// ── Movement helpers ───────────────────────────────────────────────────────

void ATranspersonalCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    if (!Controller) return;

    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

    const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDir, MovementVector.Y);
    AddMovementInput(RightDir,   MovementVector.X);
}

void ATranspersonalCharacter::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (!Controller) return;

    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
}

void ATranspersonalCharacter::HandleSprintStart()
{
    if (SurvivalComp && SurvivalComp->GetStamina() > 5.0f)
    {
        StartSprint();
    }
}

void ATranspersonalCharacter::HandleSprintStop()
{
    StopSprint();
}

// ── Public API ─────────────────────────────────────────────────────────────

void ATranspersonalCharacter::StartSprint()
{
    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void ATranspersonalCharacter::StopSprint()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

float ATranspersonalCharacter::GetHealth() const
{
    return SurvivalComp ? SurvivalComp->GetHealth() : 0.0f;
}

float ATranspersonalCharacter::GetHunger() const
{
    return SurvivalComp ? SurvivalComp->GetHunger() : 0.0f;
}

float ATranspersonalCharacter::GetThirst() const
{
    return SurvivalComp ? SurvivalComp->GetThirst() : 0.0f;
}

float ATranspersonalCharacter::GetStamina() const
{
    return SurvivalComp ? SurvivalComp->GetStamina() : 0.0f;
}

bool ATranspersonalCharacter::IsAlive() const
{
    return SurvivalComp ? SurvivalComp->IsAlive() : false;
}
