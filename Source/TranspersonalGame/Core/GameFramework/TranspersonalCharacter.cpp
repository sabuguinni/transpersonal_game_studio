#include "TranspersonalCharacter.h"
#include "Core/Survival/SurvivalComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// ============================================================
// Constructor
// ============================================================
ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Survival Component ─────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // ── Camera Boom ────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // ── Follow Camera ──────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ── Movement defaults ──────────────────────────────────────────────────
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
        MoveComp->JumpZVelocity = 420.0f;
        MoveComp->AirControl = 0.35f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    }

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;
}

// ============================================================
// BeginPlay
// ============================================================
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

// ============================================================
// Tick
// ============================================================
void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Drain stamina while sprinting; restore when idle
    if (SurvivalComp)
    {
        if (bIsSprinting)
        {
            // Sprinting costs 10 stamina/sec
            const float StaminaDrain = 10.0f * DeltaTime;
            SurvivalComp->ModifyStamina(-StaminaDrain);

            // Auto-stop sprint if stamina depleted
            if (SurvivalComp->GetStamina() <= 0.0f)
            {
                StopSprint();
            }
        }
        else
        {
            // Recover stamina at 5/sec when not sprinting
            const float StaminaRegen = 5.0f * DeltaTime;
            SurvivalComp->ModifyStamina(StaminaRegen);
        }
    }
}

// ============================================================
// Input Setup
// ============================================================
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

// ============================================================
// Movement handlers
// ============================================================
void ATranspersonalCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
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

// ============================================================
// Sprint
// ============================================================
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

void ATranspersonalCharacter::StartSprint()
{
    bIsSprinting = true;
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = RunSpeed;
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

// ============================================================
// Survival stat accessors (delegate to SurvivalComp)
// ============================================================
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
