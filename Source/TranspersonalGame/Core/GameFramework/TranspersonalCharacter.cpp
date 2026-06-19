// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "TranspersonalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Core/Survival/SurvivalComponent.h"
#include "Core/Physics/Core_RagdollSystem.h"
#include "Core/Physics/Core_PhysicsSystemManager.h"

DEFINE_LOG_CATEGORY(LogTranspersonalCharacter);

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Camera Boom ──────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // ── Follow Camera ────────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ── Survival Component ───────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // ── Physics Components ───────────────────────────────────────────────────
    RagdollComponent = CreateDefaultSubobject<UCore_RagdollSystem>(TEXT("RagdollComponent"));
    PhysicsManager   = CreateDefaultSubobject<UCore_PhysicsSystemManager>(TEXT("PhysicsManager"));

    // ── Movement Defaults ────────────────────────────────────────────────────
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate              = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity             = 420.0f;
    GetCharacterMovement()->AirControl                = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed              = 300.0f;
    GetCharacterMovement()->MinAnalogWalkSpeed        = 20.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

    // ── State Defaults ───────────────────────────────────────────────────────
    bIsHiding       = false;
    bIsSneaking     = false;
    bIsRunning      = false;
    bIsRagdolled    = false;
    bPhysicsEnabled = true;

    // ── Crafting Defaults ────────────────────────────────────────────────────
    CraftingLevel      = 1;
    CraftingExperience = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────
void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Add Enhanced Input Mapping Context
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

    UE_LOG(LogTranspersonalCharacter, Log, TEXT("ATranspersonalCharacter::BeginPlay — SurvivalComp=%s"),
        SurvivalComp ? TEXT("OK") : TEXT("NULL"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────
void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateMovementSpeed();
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
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
        const FVector  ForwardDir  = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector  RightDir    = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
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
    // Running costs stamina — check SurvivalComp
    if (SurvivalComp && SurvivalComp->GetStamina() > 10.0f)
    {
        bIsRunning = true;
        UpdateMovementSpeed();
    }
}

void ATranspersonalCharacter::StopRunning()
{
    bIsRunning = false;
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::Interact()
{
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Interact triggered"));
    // Interaction system — to be wired to InteractionComponent by Agent #11
}

void ATranspersonalCharacter::OpenCraftingMenu()
{
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Crafting menu opened — Level:%d"), CraftingLevel);
    // UI wiring to be done by UI Agent
}

// ─────────────────────────────────────────────────────────────────────────────
// Movement Speed Update
// ─────────────────────────────────────────────────────────────────────────────
void ATranspersonalCharacter::UpdateMovementSpeed()
{
    float TargetSpeed = 300.0f; // Walk

    if (bIsSneaking)
    {
        TargetSpeed = 150.0f; // Sneak
    }
    else if (bIsRunning)
    {
        // Drain stamina while running
        if (SurvivalComp)
        {
            SurvivalComp->ConsumeStamina(0.5f); // per tick — scaled by DeltaTime in SurvivalComp
            if (SurvivalComp->GetStamina() <= 0.0f)
            {
                bIsRunning = false;
                TargetSpeed = 300.0f;
            }
            else
            {
                TargetSpeed = 600.0f; // Sprint
            }
        }
        else
        {
            TargetSpeed = 600.0f;
        }
    }

    GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

// ─────────────────────────────────────────────────────────────────────────────
// Physics State Update
// ─────────────────────────────────────────────────────────────────────────────
void ATranspersonalCharacter::UpdatePhysicsState(float DeltaTime)
{
    if (!bPhysicsEnabled) return;
    // RagdollComponent drives its own tick — nothing to poll here
}

void ATranspersonalCharacter::HandleCollisionEvents()
{
    // Wired to OnComponentHit in future — placeholder for Agent #12 (Combat)
}

void ATranspersonalCharacter::ProcessRagdollTransition()
{
    if (bIsRagdolled)
    {
        EnableRagdoll();
    }
    else
    {
        DisableRagdoll();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival Delegates
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
// Survival Getters
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
// Physics Functions
// ─────────────────────────────────────────────────────────────────────────────
void ATranspersonalCharacter::EnableRagdoll()
{
    bIsRagdolled = true;
    GetMesh()->SetSimulatePhysics(true);
    GetCharacterMovement()->DisableMovement();
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Ragdoll ENABLED"));
}

void ATranspersonalCharacter::DisableRagdoll()
{
    bIsRagdolled = false;
    GetMesh()->SetSimulatePhysics(false);
    GetMesh()->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Ragdoll DISABLED"));
}

void ATranspersonalCharacter::ApplyImpact(const FVector& ImpactForce, const FVector& ImpactLocation)
{
    if (GetMesh())
    {
        GetMesh()->AddImpulseAtLocation(ImpactForce, ImpactLocation);
    }
    // High-force impacts trigger ragdoll
    if (ImpactForce.Size() > 5000.0f && !bIsRagdolled)
    {
        EnableRagdoll();
    }
}

void ATranspersonalCharacter::SetPhysicsEnabled(bool bEnabled)
{
    bPhysicsEnabled = bEnabled;
    if (RagdollComponent)
    {
        RagdollComponent->SetComponentTickEnabled(bEnabled);
    }
}
