// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "Core/GameFramework/TranspersonalCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTranspersonalCharacter);

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Capsule ──────────────────────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // ── Movement ─────────────────────────────────────────────────────────────
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate              = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity             = 700.f;
    GetCharacterMovement()->AirControl                = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed              = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed        = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    // ── Camera Boom ───────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength         = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // ── Follow Camera ─────────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ── Survival Component ────────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // ── Physics Components ────────────────────────────────────────────────────
    RagdollComponent = CreateDefaultSubobject<UCore_RagdollSystem>(TEXT("RagdollComponent"));
    PhysicsManager   = CreateDefaultSubobject<UCore_PhysicsSystemManager>(TEXT("PhysicsManager"));

    // ── Default State ─────────────────────────────────────────────────────────
    bIsHiding      = false;
    bIsSneaking    = false;
    bIsRunning     = false;
    bIsRagdolled   = false;
    bPhysicsEnabled = true;
    CraftingLevel   = 1;
    CraftingExperience = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Bind Enhanced Input mapping context
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

    UE_LOG(LogTranspersonalCharacter, Log, TEXT("ATranspersonalCharacter::BeginPlay — SurvivalComp valid: %s"),
           SurvivalComp ? TEXT("YES") : TEXT("NO"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdatePhysicsState(DeltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────
// Input Setup
// ─────────────────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (JumpAction)
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &ACharacter::Jump);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
        if (MoveAction)
        {
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Move);
        }
        if (LookAction)
        {
            EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Look);
        }
        if (SneakAction)
        {
            EIC->BindAction(SneakAction, ETriggerEvent::Started,   this, &ATranspersonalCharacter::StartSneaking);
            EIC->BindAction(SneakAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopSneaking);
        }
        if (RunAction)
        {
            EIC->BindAction(RunAction, ETriggerEvent::Started,   this, &ATranspersonalCharacter::StartRunning);
            EIC->BindAction(RunAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopRunning);
        }
        if (InteractAction)
        {
            EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::Interact);
        }
        if (CraftAction)
        {
            EIC->BindAction(CraftAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::OpenCraftingMenu);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Input Handlers
// ─────────────────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller)
    {
        const FRotator Rotation    = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector  ForwardDir  = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector  RightDir    = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDir, MovementVector.Y);
        AddMovementInput(RightDir,   MovementVector.X);

        // Drain stamina while running
        if (bIsRunning && SurvivalComp)
        {
            SurvivalComp->ConsumeStamina(5.0f * GetWorld()->GetDeltaSeconds());
        }
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

void ATranspersonalCharacter::StartSneaking()
{
    bIsSneaking = true;
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::StopSneaking()
{
    bIsSneaking = false;
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::StartRunning()
{
    bIsRunning = true;
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::StopRunning()
{
    bIsRunning = false;
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::Interact()
{
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Interact triggered"));
}

void ATranspersonalCharacter::OpenCraftingMenu()
{
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Crafting menu opened — level %d"), CraftingLevel);
}

// ─────────────────────────────────────────────────────────────────────────────
// Movement Speed
// ─────────────────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::UpdateMovementSpeed()
{
    float Speed = 500.f; // walk
    if (bIsSneaking) Speed = 200.f;
    if (bIsRunning)  Speed = 900.f;
    GetCharacterMovement()->MaxWalkSpeed = Speed;
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival Delegates — thin wrappers to SurvivalComp
// ─────────────────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::ConsumeStamina(float Amount)
{
    if (SurvivalComp) SurvivalComp->ConsumeStamina(Amount);
}

void ATranspersonalCharacter::IncreaseFear(float Amount)
{
    if (SurvivalComp) SurvivalComp->IncreaseFear(Amount);
}

void ATranspersonalCharacter::RestoreHealth(float Amount)
{
    if (SurvivalComp) SurvivalComp->RestoreHealth(Amount);
}

void ATranspersonalCharacter::RestoreStamina(float Amount)
{
    if (SurvivalComp) SurvivalComp->RestoreStamina(Amount);
}

void ATranspersonalCharacter::RestoreHunger(float Amount)
{
    if (SurvivalComp) SurvivalComp->RestoreHunger(Amount);
}

void ATranspersonalCharacter::RestoreThirst(float Amount)
{
    if (SurvivalComp) SurvivalComp->RestoreThirst(Amount);
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival Getters — delegate to SurvivalComp
// ─────────────────────────────────────────────────────────────────────────────

float ATranspersonalCharacter::GetHealthPercentage() const
{
    return SurvivalComp ? SurvivalComp->GetHealthPercentage() : 1.0f;
}

float ATranspersonalCharacter::GetStaminaPercentage() const
{
    return SurvivalComp ? SurvivalComp->GetStaminaPercentage() : 1.0f;
}

float ATranspersonalCharacter::GetHungerPercentage() const
{
    return SurvivalComp ? SurvivalComp->GetHungerPercentage() : 1.0f;
}

float ATranspersonalCharacter::GetThirstPercentage() const
{
    return SurvivalComp ? SurvivalComp->GetThirstPercentage() : 1.0f;
}

float ATranspersonalCharacter::GetFearPercentage() const
{
    return SurvivalComp ? SurvivalComp->GetFearPercentage() : 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// Physics
// ─────────────────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::EnableRagdoll()
{
    if (bIsRagdolled) return;
    bIsRagdolled = true;
    GetMesh()->SetSimulatePhysics(true);
    GetCharacterMovement()->DisableMovement();
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Ragdoll ENABLED"));
}

void ATranspersonalCharacter::DisableRagdoll()
{
    if (!bIsRagdolled) return;
    bIsRagdolled = false;
    GetMesh()->SetSimulatePhysics(false);
    GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Ragdoll DISABLED"));
}

void ATranspersonalCharacter::ApplyImpact(const FVector& ImpactForce, const FVector& ImpactLocation)
{
    if (!bIsRagdolled)
    {
        EnableRagdoll();
    }
    GetMesh()->AddImpulseAtLocation(ImpactForce, ImpactLocation);
}

void ATranspersonalCharacter::SetPhysicsEnabled(bool bEnabled)
{
    bPhysicsEnabled = bEnabled;
    if (!bEnabled && bIsRagdolled)
    {
        DisableRagdoll();
    }
}

void ATranspersonalCharacter::UpdatePhysicsState(float DeltaTime)
{
    // Auto-recover from ragdoll after 3s if still alive
    if (bIsRagdolled && SurvivalComp && SurvivalComp->GetHealthPercentage() > 0.0f)
    {
        static float RagdollTimer = 0.0f;
        RagdollTimer += DeltaTime;
        if (RagdollTimer > 3.0f)
        {
            RagdollTimer = 0.0f;
            DisableRagdoll();
        }
    }
}

void ATranspersonalCharacter::HandleCollisionEvents()
{
    // Handled via OnComponentHit delegates set up in Blueprint
}

void ATranspersonalCharacter::ProcessRagdollTransition()
{
    // Smooth blend between ragdoll and animation handled in AnimBP
}
