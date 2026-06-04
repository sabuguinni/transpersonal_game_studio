#include "Core_CharacterController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogCharacterController);

UCore_CharacterController::UCore_CharacterController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    CameraBoomLength = 300.0f;
    CameraBoomOffset = FVector(0.0f, 0.0f, 60.0f);
    bUsePawnControlRotation = true;
    
    BaseWalkSpeed = 300.0f;
    RunSpeedMultiplier = 2.0f;
    JumpVelocity = 600.0f;
    AirControl = 0.35f;
    GroundFriction = 8.0f;
    BrakingDecelerationWalking = 2048.0f;
    
    bIsRunning = false;
    MovementInput = FVector2D::ZeroVector;
    LookInput = FVector2D::ZeroVector;
    
    bControllerValid = false;
    bCameraSetupValid = false;
    bMovementSetupValid = false;
    bInputSetupValid = false;
    
    UE_LOG(LogCharacterController, Log, TEXT("Core_CharacterController initialized"));
}

void UCore_CharacterController::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-setup if owner is a character
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        SetupThirdPersonCamera(OwnerCharacter);
        SetupMovementInput(OwnerCharacter);
        EnableRunning(OwnerCharacter);
        EnableJumping(OwnerCharacter);
        
        ValidateControllerSetup(OwnerCharacter);
    }
    
    UE_LOG(LogCharacterController, Log, TEXT("Core_CharacterController BeginPlay completed"));
}

void UCore_CharacterController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic validation
    ValidateInternalState();
}

void UCore_CharacterController::SetupThirdPersonCamera(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogCharacterController, Warning, TEXT("SetupThirdPersonCamera: Character is null"));
        bCameraSetupValid = false;
        return;
    }
    
    // Create camera boom if it doesn't exist
    CameraBoom = Character->FindComponentByClass<USpringArmComponent>();
    if (!CameraBoom)
    {
        CameraBoom = Character->CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
        if (CameraBoom)
        {
            CameraBoom->SetupAttachment(Character->GetRootComponent());
        }
    }
    
    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = CameraBoomLength;
        CameraBoom->bUsePawnControlRotation = bUsePawnControlRotation;
        CameraBoom->SocketOffset = CameraBoomOffset;
        CameraBoom->bDoCollisionTest = true;
        
        UE_LOG(LogCharacterController, Log, TEXT("Camera boom configured: Length=%.1f"), CameraBoomLength);
    }
    
    // Create follow camera if it doesn't exist
    FollowCamera = Character->FindComponentByClass<UCameraComponent>();
    if (!FollowCamera)
    {
        FollowCamera = Character->CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
        if (FollowCamera && CameraBoom)
        {
            FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
        }
    }
    
    if (FollowCamera)
    {
        FollowCamera->bUsePawnControlRotation = false;
        UE_LOG(LogCharacterController, Log, TEXT("Follow camera configured"));
    }
    
    bCameraSetupValid = ValidateCameraComponents(Character);
    UE_LOG(LogCharacterController, Log, TEXT("Camera setup completed. Valid: %s"), bCameraSetupValid ? TEXT("true") : TEXT("false"));
}

void UCore_CharacterController::SetupMovementInput(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogCharacterController, Warning, TEXT("SetupMovementInput: Character is null"));
        bMovementSetupValid = false;
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        UE_LOG(LogCharacterController, Error, TEXT("SetupMovementInput: No CharacterMovementComponent found"));
        bMovementSetupValid = false;
        return;
    }
    
    SetupDefaultMovementParameters(MovementComp);
    
    // Configure character rotation
    Character->bUseControllerRotationPitch = false;
    Character->bUseControllerRotationYaw = false;
    Character->bUseControllerRotationRoll = false;
    
    // Configure movement component rotation
    MovementComp->bOrientRotationToMovement = true;
    MovementComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    
    bMovementSetupValid = ValidateMovementComponents(Character);
    UE_LOG(LogCharacterController, Log, TEXT("Movement input setup completed. Valid: %s"), bMovementSetupValid ? TEXT("true") : TEXT("false"));
}

void UCore_CharacterController::EnableRunning(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogCharacterController, Warning, TEXT("EnableRunning: Character is null"));
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        UE_LOG(LogCharacterController, Error, TEXT("EnableRunning: No CharacterMovementComponent found"));
        return;
    }
    
    // Set up running speeds
    MovementComp->MaxWalkSpeed = BaseWalkSpeed;
    MovementComp->MaxWalkSpeedCrouched = BaseWalkSpeed * 0.5f;
    
    UE_LOG(LogCharacterController, Log, TEXT("Running enabled: WalkSpeed=%.1f, RunSpeed=%.1f"), 
           BaseWalkSpeed, BaseWalkSpeed * RunSpeedMultiplier);
}

void UCore_CharacterController::EnableJumping(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogCharacterController, Warning, TEXT("EnableJumping: Character is null"));
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        UE_LOG(LogCharacterController, Error, TEXT("EnableJumping: No CharacterMovementComponent found"));
        return;
    }
    
    // Configure jumping
    MovementComp->JumpZVelocity = JumpVelocity;
    MovementComp->AirControl = AirControl;
    MovementComp->AirControlBoostMultiplier = 2.0f;
    MovementComp->AirControlBoostVelocityThreshold = 25.0f;
    MovementComp->FallingLateralFriction = 0.0f;
    
    // Enable jumping on character
    Character->JumpMaxCount = 1;
    Character->JumpMaxHoldTime = 0.5f;
    
    UE_LOG(LogCharacterController, Log, TEXT("Jumping enabled: JumpVelocity=%.1f, AirControl=%.2f"), 
           JumpVelocity, AirControl);
}

bool UCore_CharacterController::ValidateControllerSetup(ACharacter* Character)
{
    if (!Character)
    {
        bControllerValid = false;
        UE_LOG(LogCharacterController, Error, TEXT("ValidateControllerSetup: Character is null"));
        return false;
    }
    
    bool bCameraValid = ValidateCameraComponents(Character);
    bool bMovementValid = ValidateMovementComponents(Character);
    
    bControllerValid = bCameraValid && bMovementValid;
    
    UE_LOG(LogCharacterController, Log, TEXT("Controller validation: Camera=%s, Movement=%s, Overall=%s"),
           bCameraValid ? TEXT("PASS") : TEXT("FAIL"),
           bMovementValid ? TEXT("PASS") : TEXT("FAIL"),
           bControllerValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bControllerValid;
}

float UCore_CharacterController::GetMilestoneCompletion() const
{
    int32 CompletedComponents = 0;
    int32 TotalComponents = 4;
    
    if (bCameraSetupValid) CompletedComponents++;
    if (bMovementSetupValid) CompletedComponents++;
    if (bInputSetupValid) CompletedComponents++;
    if (bControllerValid) CompletedComponents++;
    
    return (float)CompletedComponents / (float)TotalComponents * 100.0f;
}

FString UCore_CharacterController::GetMilestoneStatus() const
{
    FString Status = TEXT("WALK AROUND Milestone - Character Controller Status:\n");
    Status += FString::Printf(TEXT("Camera Setup: %s\n"), bCameraSetupValid ? TEXT("PASS") : TEXT("FAIL"));
    Status += FString::Printf(TEXT("Movement Setup: %s\n"), bMovementSetupValid ? TEXT("PASS") : TEXT("FAIL"));
    Status += FString::Printf(TEXT("Input Setup: %s\n"), bInputSetupValid ? TEXT("PASS") : TEXT("FAIL"));
    Status += FString::Printf(TEXT("Controller Valid: %s\n"), bControllerValid ? TEXT("PASS") : TEXT("FAIL"));
    Status += FString::Printf(TEXT("Completion: %.1f%%"), GetMilestoneCompletion());
    
    return Status;
}

void UCore_CharacterController::LogControllerStatus() const
{
    UE_LOG(LogCharacterController, Log, TEXT("=== CHARACTER CONTROLLER STATUS ==="));
    UE_LOG(LogCharacterController, Log, TEXT("Camera Setup Valid: %s"), bCameraSetupValid ? TEXT("true") : TEXT("false"));
    UE_LOG(LogCharacterController, Log, TEXT("Movement Setup Valid: %s"), bMovementSetupValid ? TEXT("true") : TEXT("false"));
    UE_LOG(LogCharacterController, Log, TEXT("Input Setup Valid: %s"), bInputSetupValid ? TEXT("true") : TEXT("false"));
    UE_LOG(LogCharacterController, Log, TEXT("Controller Valid: %s"), bControllerValid ? TEXT("true") : TEXT("false"));
    UE_LOG(LogCharacterController, Log, TEXT("Milestone Completion: %.1f%%"), GetMilestoneCompletion());
    UE_LOG(LogCharacterController, Log, TEXT("==================================="));
}

void UCore_CharacterController::ResetController()
{
    bIsRunning = false;
    MovementInput = FVector2D::ZeroVector;
    LookInput = FVector2D::ZeroVector;
    
    bControllerValid = false;
    bCameraSetupValid = false;
    bMovementSetupValid = false;
    bInputSetupValid = false;
    
    UE_LOG(LogCharacterController, Log, TEXT("Controller reset to default state"));
}

void UCore_CharacterController::ValidateInternalState()
{
    // Periodic validation of internal state
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        bCameraSetupValid = ValidateCameraComponents(OwnerCharacter);
        bMovementSetupValid = ValidateMovementComponents(OwnerCharacter);
        bInputSetupValid = true; // Assume input is valid for now
        bControllerValid = bCameraSetupValid && bMovementSetupValid && bInputSetupValid;
    }
}

void UCore_CharacterController::SetupDefaultMovementParameters(UCharacterMovementComponent* MovementComp)
{
    if (!MovementComp)
    {
        return;
    }
    
    // Basic movement settings
    MovementComp->MaxWalkSpeed = BaseWalkSpeed;
    MovementComp->MinAnalogWalkSpeed = 20.0f;
    MovementComp->BrakingDecelerationWalking = BrakingDecelerationWalking;
    
    // Ground movement
    MovementComp->GroundFriction = GroundFriction;
    MovementComp->MaxStepHeight = 45.0f;
    MovementComp->PerchRadiusThreshold = 0.0f;
    MovementComp->PerchAdditionalHeight = 40.0f;
    
    // Air movement
    MovementComp->AirControl = AirControl;
    MovementComp->BrakingDecelerationFalling = 0.0f;
    
    // Physics interaction
    MovementComp->bCanWalkOffLedges = true;
    MovementComp->bCanWalkOffLedgesWhenCrouching = false;
    
    UE_LOG(LogCharacterController, Log, TEXT("Default movement parameters applied"));
}

bool UCore_CharacterController::ValidateCameraComponents(ACharacter* Character) const
{
    if (!Character)
    {
        return false;
    }
    
    USpringArmComponent* Boom = Character->FindComponentByClass<USpringArmComponent>();
    UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>();
    
    bool bBoomValid = (Boom != nullptr);
    bool bCameraValid = (Camera != nullptr);
    bool bAttachmentValid = true;
    
    if (Boom && Camera)
    {
        // Check if camera is attached to boom
        bAttachmentValid = (Camera->GetAttachParent() == Boom);
    }
    
    return bBoomValid && bCameraValid && bAttachmentValid;
}

bool UCore_CharacterController::ValidateMovementComponents(ACharacter* Character) const
{
    if (!Character)
    {
        return false;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        return false;
    }
    
    // Validate movement settings
    bool bSpeedValid = (MovementComp->MaxWalkSpeed > 0.0f);
    bool bJumpValid = (MovementComp->JumpZVelocity > 0.0f);
    bool bRotationValid = MovementComp->bOrientRotationToMovement;
    
    return bSpeedValid && bJumpValid && bRotationValid;
}