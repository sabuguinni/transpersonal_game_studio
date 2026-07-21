#include "Core_WalkAroundPhysics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogWalkAroundPhysics);

UCore_WalkAroundPhysics::UCore_WalkAroundPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize movement configuration with realistic values for WALK AROUND milestone
    MovementConfig.WalkSpeed = 300.0f;
    MovementConfig.RunSpeedMultiplier = 2.0f;
    MovementConfig.JumpVelocity = 600.0f;
    MovementConfig.AirControl = 0.2f;
    MovementConfig.GroundFriction = 8.0f;
    MovementConfig.BrakingDecelerationWalking = 2000.0f;
    MovementConfig.BrakingDecelerationFalling = 0.0f;
    
    CurrentState = ECore_WalkAroundState::Idle;
    StateTime = 0.0f;
}

void UCore_WalkAroundPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Get component references
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        MovementComponent = Character->GetCharacterMovement();
        CapsuleComponent = Character->GetCapsuleComponent();
        
        if (MovementComponent.IsValid())
        {
            InitializeWalkAroundMovement(MovementComponent.Get());
            UE_LOG(LogWalkAroundPhysics, Log, TEXT("WalkAroundPhysics initialized for character: %s"), *Character->GetName());
        }
        else
        {
            UE_LOG(LogWalkAroundPhysics, Warning, TEXT("Failed to get CharacterMovementComponent for: %s"), *Character->GetName());
        }
    }
    else
    {
        UE_LOG(LogWalkAroundPhysics, Error, TEXT("WalkAroundPhysics component attached to non-Character actor: %s"), *GetOwner()->GetName());
    }
}

void UCore_WalkAroundPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!ValidateComponents())
    {
        return;
    }
    
    // Update movement state based on current velocity
    const FVector CurrentVelocity = MovementComponent->Velocity;
    const bool bIsOnGround = MovementComponent->IsMovingOnGround();
    const bool bIsJumping = MovementComponent->IsFalling() && CurrentVelocity.Z > 0.0f;
    
    UpdateMovementState(CurrentVelocity, bIsOnGround, bIsJumping);
    UpdateStateTimers(DeltaTime);
    
    // Apply terrain interaction if on ground
    if (bIsOnGround)
    {
        FHitResult GroundHit;
        const FVector Start = GetOwner()->GetActorLocation();
        const FVector End = Start - FVector(0, 0, 200.0f);
        
        if (GetWorld()->LineTraceSingleByChannel(GroundHit, Start, End, ECC_WorldStatic))
        {
            ApplyTerrainInteraction(GroundHit);
        }
    }
    
    PreviousVelocity = CurrentVelocity;
}

void UCore_WalkAroundPhysics::InitializeWalkAroundMovement(UCharacterMovementComponent* MovementComp)
{
    if (!MovementComp)
    {
        UE_LOG(LogWalkAroundPhysics, Error, TEXT("InitializeWalkAroundMovement called with null MovementComponent"));
        return;
    }
    
    // Configure basic movement parameters for WALK AROUND milestone
    MovementComp->MaxWalkSpeed = MovementConfig.WalkSpeed;
    MovementComp->JumpZVelocity = MovementConfig.JumpVelocity;
    MovementComp->AirControl = MovementConfig.AirControl;
    MovementComp->GroundFriction = MovementConfig.GroundFriction;
    MovementComp->BrakingDecelerationWalking = MovementConfig.BrakingDecelerationWalking;
    MovementComp->BrakingDecelerationFalling = MovementConfig.BrakingDecelerationFalling;
    
    // Enable essential movement modes
    MovementComp->SetMovementMode(MOVE_Walking);
    MovementComp->bCanWalkOffLedges = true;
    MovementComp->bCanWalkOffLedgesWhenCrouching = true;
    
    // Configure rotation settings for responsive control
    MovementComp->bOrientRotationToMovement = true;
    MovementComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    MovementComp->bUseControllerDesiredRotation = false;
    
    UE_LOG(LogWalkAroundPhysics, Log, TEXT("Movement component configured for WALK AROUND milestone"));
}

void UCore_WalkAroundPhysics::ConfigureCameraBoom(USpringArmComponent* SpringArm, UCameraComponent* Camera)
{
    if (!SpringArm || !Camera)
    {
        UE_LOG(LogWalkAroundPhysics, Warning, TEXT("ConfigureCameraBoom called with null components"));
        return;
    }
    
    // Configure spring arm for third person view
    SpringArm->TargetArmLength = 400.0f;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bInheritPitch = true;
    SpringArm->bInheritYaw = true;
    SpringArm->bInheritRoll = false;
    SpringArm->bDoCollisionTest = true;
    
    // Set camera offset for better view
    SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    SpringArm->SocketOffset = FVector(0.0f, 0.0f, 0.0f);
    
    UE_LOG(LogWalkAroundPhysics, Log, TEXT("Camera boom configured for WALK AROUND milestone"));
}

void UCore_WalkAroundPhysics::UpdateMovementState(const FVector& Velocity, bool bIsOnGround, bool bIsJumping)
{
    const ECore_WalkAroundState NewState = DetermineStateFromVelocity(Velocity, bIsOnGround, bIsJumping);
    
    if (NewState != CurrentState)
    {
        const ECore_WalkAroundState PreviousState = CurrentState;
        CurrentState = NewState;
        StateTime = 0.0f;
        
        UE_LOG(LogWalkAroundPhysics, VeryVerbose, TEXT("Movement state changed: %d -> %d"), 
               static_cast<int32>(PreviousState), static_cast<int32>(CurrentState));
    }
}

bool UCore_WalkAroundPhysics::CanJump() const
{
    if (!MovementComponent.IsValid())
    {
        return false;
    }
    
    return MovementComponent->IsMovingOnGround() && 
           (CurrentState == ECore_WalkAroundState::Idle || 
            CurrentState == ECore_WalkAroundState::Walking || 
            CurrentState == ECore_WalkAroundState::Running);
}

void UCore_WalkAroundPhysics::ApplyTerrainInteraction(const FHitResult& GroundHit)
{
    if (!GroundHit.bBlockingHit || !MovementComponent.IsValid())
    {
        return;
    }
    
    // Update ground normal for slope calculations
    GroundNormal = GroundHit.Normal;
    
    // Calculate slope angle
    const float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GroundNormal, FVector::UpVector)));
    
    // Apply slope-based friction adjustment
    if (SlopeAngle > 25.0f) // Steep slope
    {
        const float SlopeFriction = FMath::Lerp(MovementConfig.GroundFriction, MovementConfig.GroundFriction * 0.5f, 
                                               (SlopeAngle - 25.0f) / 40.0f);
        MovementComponent->GroundFriction = SlopeFriction;
    }
    else
    {
        MovementComponent->GroundFriction = MovementConfig.GroundFriction;
    }
    
    // Debug visualization in development builds
    #if UE_BUILD_DEVELOPMENT
    if (CVarShowDebugTraversal.GetValueOnGameThread())
    {
        DrawDebugLine(GetWorld(), GroundHit.Location, GroundHit.Location + GroundNormal * 100.0f, 
                     FColor::Green, false, 0.1f, 0, 2.0f);
    }
    #endif
}

bool UCore_WalkAroundPhysics::ValidateWalkAroundMilestone() const
{
    if (!ValidateComponents())
    {
        UE_LOG(LogWalkAroundPhysics, Warning, TEXT("WALK AROUND validation failed: Missing components"));
        return false;
    }
    
    // Check if movement component is properly configured
    const bool bMovementConfigured = (MovementComponent->MaxWalkSpeed > 0.0f) && 
                                   (MovementComponent->JumpZVelocity > 0.0f);
    
    // Check if character can move
    const bool bCanMove = MovementComponent->GetMaxSpeed() > 0.0f;
    
    // Check if on valid ground
    const bool bOnGround = MovementComponent->IsMovingOnGround();
    
    const bool bMilestoneValid = bMovementConfigured && bCanMove;
    
    UE_LOG(LogWalkAroundPhysics, Log, TEXT("WALK AROUND milestone validation: %s (Movement: %s, CanMove: %s, OnGround: %s)"),
           bMilestoneValid ? TEXT("PASS") : TEXT("FAIL"),
           bMovementConfigured ? TEXT("OK") : TEXT("FAIL"),
           bCanMove ? TEXT("OK") : TEXT("FAIL"),
           bOnGround ? TEXT("OK") : TEXT("FAIL"));
    
    return bMilestoneValid;
}

void UCore_WalkAroundPhysics::UpdateStateTimers(float DeltaTime)
{
    StateTime += DeltaTime;
}

ECore_WalkAroundState UCore_WalkAroundPhysics::DetermineStateFromVelocity(const FVector& Velocity, bool bIsOnGround, bool bIsJumping) const
{
    const float Speed2D = Velocity.Size2D();
    const float VerticalVelocity = Velocity.Z;
    
    // Jumping state
    if (bIsJumping && VerticalVelocity > 100.0f)
    {
        return ECore_WalkAroundState::Jumping;
    }
    
    // Falling state
    if (!bIsOnGround && VerticalVelocity < -100.0f)
    {
        return ECore_WalkAroundState::Falling;
    }
    
    // Landing state (brief transition)
    if (bIsOnGround && CurrentState == ECore_WalkAroundState::Falling && StateTime < 0.2f)
    {
        return ECore_WalkAroundState::Landing;
    }
    
    // Ground movement states
    if (bIsOnGround)
    {
        const float RunThreshold = MovementConfig.WalkSpeed * MovementConfig.RunSpeedMultiplier * 0.8f;
        const float WalkThreshold = MovementConfig.WalkSpeed * 0.1f;
        
        if (Speed2D > RunThreshold)
        {
            return ECore_WalkAroundState::Running;
        }
        else if (Speed2D > WalkThreshold)
        {
            return ECore_WalkAroundState::Walking;
        }
        else
        {
            return ECore_WalkAroundState::Idle;
        }
    }
    
    // Default to current state if no clear transition
    return CurrentState;
}

void UCore_WalkAroundPhysics::ApplyMovementConfiguration()
{
    if (!MovementComponent.IsValid())
    {
        return;
    }
    
    MovementComponent->MaxWalkSpeed = MovementConfig.WalkSpeed;
    MovementComponent->JumpZVelocity = MovementConfig.JumpVelocity;
    MovementComponent->AirControl = MovementConfig.AirControl;
    MovementComponent->GroundFriction = MovementConfig.GroundFriction;
    MovementComponent->BrakingDecelerationWalking = MovementConfig.BrakingDecelerationWalking;
    MovementComponent->BrakingDecelerationFalling = MovementConfig.BrakingDecelerationFalling;
}

bool UCore_WalkAroundPhysics::ValidateComponents() const
{
    return MovementComponent.IsValid() && CapsuleComponent.IsValid() && GetOwner() != nullptr;
}