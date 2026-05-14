#include "Anim_MovementStateMachine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UAnim_MovementStateMachine::UAnim_MovementStateMachine()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize movement data
    MovementData = FAnim_MovementData();
    
    // Set reasonable defaults
    WalkThreshold = 50.0f;
    RunThreshold = 300.0f;
    JumpVelocityThreshold = 100.0f;
    GroundTraceDistance = 200.0f;
    MinStateTime = 0.1f;
    
    StateTimer = 0.0f;
    LastStateChangeTime = 0.0f;
    PreviousState = EAnim_MovementState::Idle;
}

void UAnim_MovementStateMachine::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache component references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CharacterMovement = OwnerCharacter->GetCharacterMovement();
        SkeletalMesh = OwnerCharacter->GetMesh();
        
        UE_LOG(LogTemp, Log, TEXT("Animation State Machine initialized for character: %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Animation State Machine: Owner is not a Character!"));
    }
    
    // Initialize state
    MovementData.CurrentState = EAnim_MovementState::Idle;
    PreviousState = EAnim_MovementState::Idle;
}

void UAnim_MovementStateMachine::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !CharacterMovement)
    {
        return;
    }
    
    // Update state timer
    StateTimer += DeltaTime;
    
    // Analyze current movement
    AnalyzeMovement(CharacterMovement);
    
    // Update movement state
    UpdateMovementState(DeltaTime);
    
    // Calculate ground distance for IK
    CalculateGroundDistance();
}

void UAnim_MovementStateMachine::UpdateMovementState(float DeltaTime)
{
    if (!CharacterMovement)
    {
        return;
    }
    
    // Determine what state we should be in
    EAnim_MovementState TargetState = DetermineMovementState();
    
    // Check if we can transition to the target state
    if (TargetState != MovementData.CurrentState && CanTransitionTo(TargetState))
    {
        TransitionToState(TargetState);
    }
}

EAnim_MovementState UAnim_MovementStateMachine::DetermineMovementState() const
{
    if (!CharacterMovement || !OwnerCharacter)
    {
        return EAnim_MovementState::Idle;
    }
    
    // Check if character is in air
    if (CharacterMovement->IsFalling())
    {
        // Distinguish between jumping and falling
        if (CharacterMovement->Velocity.Z > JumpVelocityThreshold)
        {
            return EAnim_MovementState::Jumping;
        }
        else
        {
            return EAnim_MovementState::Falling;
        }
    }
    
    // Check if character is swimming
    if (CharacterMovement->IsSwimming())
    {
        return EAnim_MovementState::Swimming;
    }
    
    // Check if character is crouching
    if (CharacterMovement->IsCrouching())
    {
        return EAnim_MovementState::Crouching;
    }
    
    // Check movement speed for ground movement
    float CurrentSpeed = MovementData.Speed;
    
    if (CurrentSpeed < WalkThreshold)
    {
        return EAnim_MovementState::Idle;
    }
    else if (CurrentSpeed < RunThreshold)
    {
        return EAnim_MovementState::Walking;
    }
    else
    {
        return EAnim_MovementState::Running;
    }
}

void UAnim_MovementStateMachine::TransitionToState(EAnim_MovementState NewState)
{
    if (NewState == MovementData.CurrentState)
    {
        return;
    }
    
    // Check minimum state time (prevent rapid state changes)
    float TimeSinceLastChange = GetWorld()->GetTimeSeconds() - LastStateChangeTime;
    if (TimeSinceLastChange < MinStateTime)
    {
        return;
    }
    
    // Validate transition
    if (!IsValidStateTransition(MovementData.CurrentState, NewState))
    {
        return;
    }
    
    // Store previous state
    EAnim_MovementState OldState = MovementData.CurrentState;
    PreviousState = OldState;
    
    // Update state
    MovementData.CurrentState = NewState;
    StateTimer = 0.0f;
    LastStateChangeTime = GetWorld()->GetTimeSeconds();
    
    // Handle state change
    OnStateChanged(OldState, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("Animation State: %s -> %s"), 
           *UEnum::GetValueAsString(OldState), 
           *UEnum::GetValueAsString(NewState));
}

bool UAnim_MovementStateMachine::CanTransitionTo(EAnim_MovementState TargetState) const
{
    // Check if enough time has passed since last state change
    float TimeSinceLastChange = GetWorld()->GetTimeSeconds() - LastStateChangeTime;
    if (TimeSinceLastChange < MinStateTime)
    {
        return false;
    }
    
    // Check if transition is valid
    return IsValidStateTransition(MovementData.CurrentState, TargetState);
}

bool UAnim_MovementStateMachine::IsValidStateTransition(EAnim_MovementState From, EAnim_MovementState To) const
{
    // All transitions are valid for now - can be restricted later
    // Example restrictions:
    // - Can't go directly from Jumping to Running without going through Falling/Landing
    // - Can't go from Swimming to Jumping directly
    
    return true;
}

void UAnim_MovementStateMachine::OnStateChanged(EAnim_MovementState OldState, EAnim_MovementState NewState)
{
    // Handle any special logic when states change
    // This is where you could trigger animation montages, sound effects, etc.
    
    switch (NewState)
    {
    case EAnim_MovementState::Jumping:
        // Could trigger jump sound or particle effect
        break;
    case EAnim_MovementState::Running:
        // Could trigger running sound loop
        break;
    case EAnim_MovementState::Swimming:
        // Could trigger splash effects
        break;
    default:
        break;
    }
}

void UAnim_MovementStateMachine::AnalyzeMovement(UCharacterMovementComponent* MovementComponent)
{
    if (!MovementComponent || !OwnerCharacter)
    {
        return;
    }
    
    // Get velocity and calculate speed
    FVector Velocity = MovementComponent->Velocity;
    MovementData.Speed = Velocity.Size2D(); // Horizontal speed only
    
    // Calculate movement direction relative to character facing
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector RightVector = OwnerCharacter->GetActorRightVector();
    
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.0f);
    if (HorizontalVelocity.SizeSquared() > 0.01f)
    {
        HorizontalVelocity.Normalize();
        
        // Calculate angle (-180 to 180)
        float ForwardDot = FVector::DotProduct(HorizontalVelocity, ForwardVector);
        float RightDot = FVector::DotProduct(HorizontalVelocity, RightVector);
        MovementData.Direction = FMath::Atan2(RightDot, ForwardDot) * 180.0f / PI;
    }
    else
    {
        MovementData.Direction = 0.0f;
    }
    
    // Update movement state flags
    MovementData.bIsInAir = MovementComponent->IsFalling();
    MovementData.bIsCrouching = MovementComponent->IsCrouching();
}

void UAnim_MovementStateMachine::CalculateGroundDistance()
{
    if (!OwnerCharacter || !GetWorld())
    {
        MovementData.GroundDistance = 0.0f;
        return;
    }
    
    // Perform line trace downward to find ground distance
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, GroundTraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        MovementData.GroundDistance = HitResult.Distance;
    }
    else
    {
        MovementData.GroundDistance = GroundTraceDistance; // Max distance if no ground found
    }
    
    // Optional: Draw debug line in development builds
    #if WITH_EDITOR
    if (GetWorld()->IsPlayInEditor())
    {
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, 
                     bHit ? FColor::Green : FColor::Red, false, 0.1f);
    }
    #endif
}