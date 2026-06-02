#include "Char_PlayerMovementSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UChar_PlayerMovementSystem::UChar_PlayerMovementSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    MovementConfig = FChar_MovementConfig();
    CameraConfig = FChar_CameraConfig();
    
    CurrentStamina = MaxStamina;
    bIsRunning = false;
    bIsCrouching = false;
    bWantsToRun = false;
    LastStaminaUpdate = 0.0f;
}

void UChar_PlayerMovementSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    ValidateConfiguration();
    
    if (OwnerCharacter)
    {
        SetupPlayerMovement(OwnerCharacter);
        SetupPlayerCamera(OwnerCharacter);
        ApplyMovementSpeed();
    }
}

void UChar_PlayerMovementSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateMovementState(DeltaTime);
    UpdateStamina(DeltaTime);
}

void UChar_PlayerMovementSystem::InitializeComponents()
{
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Char_PlayerMovementSystem: Owner is not a Character"));
        return;
    }
    
    MovementComponent = OwnerCharacter->GetCharacterMovement();
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Char_PlayerMovementSystem: No CharacterMovementComponent found"));
        return;
    }
    
    // Find or create spring arm component
    SpringArmComponent = OwnerCharacter->FindComponentByClass<USpringArmComponent>();
    if (!SpringArmComponent)
    {
        SpringArmComponent = OwnerCharacter->CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
        if (SpringArmComponent)
        {
            SpringArmComponent->AttachToComponent(OwnerCharacter->GetRootComponent(), 
                FAttachmentTransformRules::KeepRelativeTransform);
        }
    }
    
    // Find or create camera component
    CameraComponent = OwnerCharacter->FindComponentByClass<UCameraComponent>();
    if (!CameraComponent && SpringArmComponent)
    {
        CameraComponent = OwnerCharacter->CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
        if (CameraComponent)
        {
            CameraComponent->AttachToComponent(SpringArmComponent, 
                FAttachmentTransformRules::KeepRelativeTransform);
        }
    }
}

void UChar_PlayerMovementSystem::ValidateConfiguration()
{
    // Ensure movement speeds are valid
    if (MovementConfig.WalkSpeed <= 0.0f)
    {
        MovementConfig.WalkSpeed = 300.0f;
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerMovementSystem: Invalid WalkSpeed, reset to 300"));
    }
    
    if (MovementConfig.RunSpeed <= MovementConfig.WalkSpeed)
    {
        MovementConfig.RunSpeed = MovementConfig.WalkSpeed * 2.0f;
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerMovementSystem: RunSpeed too low, adjusted"));
    }
    
    if (MovementConfig.CrouchSpeed <= 0.0f)
    {
        MovementConfig.CrouchSpeed = MovementConfig.WalkSpeed * 0.5f;
        UE_LOG(LogTemp, Warning, TEXT("Char_PlayerMovementSystem: Invalid CrouchSpeed, adjusted"));
    }
    
    // Ensure stamina values are valid
    if (MaxStamina <= 0.0f)
    {
        MaxStamina = 100.0f;
    }
    
    if (CurrentStamina > MaxStamina)
    {
        CurrentStamina = MaxStamina;
    }
}

void UChar_PlayerMovementSystem::SetupPlayerMovement(ACharacter* Character)
{
    if (!Character || !MovementComponent)
    {
        return;
    }
    
    // Configure character movement component
    MovementComponent->MaxWalkSpeed = MovementConfig.WalkSpeed;
    MovementComponent->JumpZVelocity = MovementConfig.JumpVelocity;
    MovementComponent->AirControl = 0.2f;
    MovementComponent->GroundFriction = 8.0f;
    MovementComponent->MaxAcceleration = 2048.0f;
    MovementComponent->BrakingDecelerationWalking = 2048.0f;
    MovementComponent->BrakingDecelerationFalling = 1500.0f;
    MovementComponent->BrakingDecelerationFlying = 3000.0f;
    MovementComponent->BrakingDecelerationSwimming = 10.0f;
    
    // Configure crouching
    MovementComponent->MaxWalkSpeedCrouched = MovementConfig.CrouchSpeed;
    MovementComponent->bCanWalkOffLedgesWhenCrouching = true;
    
    UE_LOG(LogTemp, Log, TEXT("Char_PlayerMovementSystem: Movement setup complete"));
}

void UChar_PlayerMovementSystem::SetupPlayerCamera(ACharacter* Character)
{
    if (!Character || !SpringArmComponent || !CameraComponent)
    {
        return;
    }
    
    // Configure spring arm
    SpringArmComponent->TargetArmLength = CameraConfig.SpringArmLength;
    SpringArmComponent->SetRelativeRotation(CameraConfig.SpringArmRotation);
    SpringArmComponent->bUsePawnControlRotation = CameraConfig.bUsePawnControlRotation;
    SpringArmComponent->bInheritPitch = CameraConfig.bInheritPitch;
    SpringArmComponent->bInheritYaw = CameraConfig.bInheritYaw;
    SpringArmComponent->bInheritRoll = CameraConfig.bInheritRoll;
    SpringArmComponent->bDoCollisionTest = true;
    SpringArmComponent->bEnableCameraLag = true;
    SpringArmComponent->CameraLagSpeed = 3.0f;
    
    // Configure camera
    CameraComponent->bUsePawnControlRotation = false;
    CameraComponent->SetFieldOfView(90.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Char_PlayerMovementSystem: Camera setup complete"));
}

void UChar_PlayerMovementSystem::StartRunning()
{
    if (CanRun())
    {
        bWantsToRun = true;
        bIsRunning = true;
        ApplyMovementSpeed();
        UE_LOG(LogTemp, Log, TEXT("Char_PlayerMovementSystem: Started running"));
    }
}

void UChar_PlayerMovementSystem::StopRunning()
{
    bWantsToRun = false;
    bIsRunning = false;
    ApplyMovementSpeed();
    UE_LOG(LogTemp, Log, TEXT("Char_PlayerMovementSystem: Stopped running"));
}

void UChar_PlayerMovementSystem::StartCrouching()
{
    if (OwnerCharacter && !bIsCrouching)
    {
        OwnerCharacter->Crouch();
        bIsCrouching = true;
        StopRunning(); // Can't run while crouching
        UE_LOG(LogTemp, Log, TEXT("Char_PlayerMovementSystem: Started crouching"));
    }
}

void UChar_PlayerMovementSystem::StopCrouching()
{
    if (OwnerCharacter && bIsCrouching)
    {
        OwnerCharacter->UnCrouch();
        bIsCrouching = false;
        UE_LOG(LogTemp, Log, TEXT("Char_PlayerMovementSystem: Stopped crouching"));
    }
}

void UChar_PlayerMovementSystem::HandleJump()
{
    if (OwnerCharacter && OwnerCharacter->CanJump())
    {
        OwnerCharacter->Jump();
        UE_LOG(LogTemp, Log, TEXT("Char_PlayerMovementSystem: Jump executed"));
    }
}

bool UChar_PlayerMovementSystem::CanRun() const
{
    return CurrentStamina >= MovementConfig.MinStaminaToRun && !bIsCrouching;
}

void UChar_PlayerMovementSystem::UpdateStamina(float DeltaTime)
{
    if (bIsRunning && MovementComponent && MovementComponent->Velocity.Size() > 0.1f)
    {
        // Drain stamina while running and moving
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - (MovementConfig.StaminaDrainRate * DeltaTime));
        
        // Stop running if stamina is depleted
        if (CurrentStamina <= 0.0f && bIsRunning)
        {
            StopRunning();
        }
    }
    else if (!bIsRunning)
    {
        // Regenerate stamina when not running
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (MovementConfig.StaminaRegenRate * DeltaTime));
    }
}

void UChar_PlayerMovementSystem::ApplyMovementSpeed()
{
    if (!MovementComponent)
    {
        return;
    }
    
    float TargetSpeed = MovementConfig.WalkSpeed;
    
    if (bIsCrouching)
    {
        TargetSpeed = MovementConfig.CrouchSpeed;
    }
    else if (bIsRunning && CanRun())
    {
        TargetSpeed = MovementConfig.RunSpeed;
    }
    
    MovementComponent->MaxWalkSpeed = TargetSpeed;
}

void UChar_PlayerMovementSystem::UpdateMovementState(float DeltaTime)
{
    // Check if we should stop running due to stamina
    if (bWantsToRun && !CanRun())
    {
        StopRunning();
    }
    else if (bWantsToRun && CanRun() && !bIsRunning)
    {
        bIsRunning = true;
        ApplyMovementSpeed();
    }
    
    // Update movement speed based on current state
    ApplyMovementSpeed();
}

void UChar_PlayerMovementSystem::SetupInputBindings(UInputComponent* PlayerInputComponent)
{
    if (!PlayerInputComponent)
    {
        return;
    }
    
    // Movement bindings would be set up in the character class
    // This function is here for reference and future expansion
    UE_LOG(LogTemp, Log, TEXT("Char_PlayerMovementSystem: Input bindings setup called"));
}

void UChar_PlayerMovementSystem::LookUp(float Value)
{
    if (OwnerCharacter && Value != 0.0f)
    {
        OwnerCharacter->AddControllerPitchInput(Value * CameraConfig.MouseSensitivity);
    }
}

void UChar_PlayerMovementSystem::Turn(float Value)
{
    if (OwnerCharacter && Value != 0.0f)
    {
        OwnerCharacter->AddControllerYawInput(Value * CameraConfig.MouseSensitivity);
    }
}

void UChar_PlayerMovementSystem::AdjustCameraDistance(float Delta)
{
    if (SpringArmComponent)
    {
        float NewLength = SpringArmComponent->TargetArmLength + (Delta * 50.0f);
        NewLength = FMath::Clamp(NewLength, 100.0f, 800.0f);
        SpringArmComponent->TargetArmLength = NewLength;
        CameraConfig.SpringArmLength = NewLength;
    }
}