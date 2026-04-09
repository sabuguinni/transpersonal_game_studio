#include "TranspersonalAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "TranspersonalAnimationComponent.h"
#include "TranspersonalAnimationTypes.h"

UTranspersonalAnimInstance::UTranspersonalAnimInstance()
{
    // Initialize default values
    BlendTime = 0.2f;
    ConfidenceModifier = 1.0f;
    NervousnessModifier = 1.0f;
    WeightModifier = 1.0f;
    SpeedModifier = 1.0f;
    
    // Physical variations - slight randomization for uniqueness
    LeftLegLengthRatio = FMath::RandRange(0.98f, 1.02f);
    RightLegLengthRatio = FMath::RandRange(0.98f, 1.02f);
    BackCurvature = FMath::RandRange(-2.0f, 2.0f);
    HeadTiltBias = FMath::RandRange(-1.0f, 1.0f);
    
    // Initialize personality with default values
    Personality.Confidence = 0.7f;
    Personality.Nervousness = 0.3f;
    Personality.Aggression = 0.2f;
    Personality.Curiosity = 0.8f;
    Personality.Caution = 0.6f;
    Personality.Energy = 0.8f;
    Personality.Weight = 1.0f;
    Personality.Stride = 1.0f;
    Personality.HeadMovement = 0.5f;
    Personality.ShoulderTension = 0.3f;
}

void UTranspersonalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    OwningPawn = TryGetPawnOwner();
    if (OwningPawn)
    {
        MovementComponent = OwningPawn->FindComponentByClass<UCharacterMovementComponent>();
        AnimationComponent = OwningPawn->FindComponentByClass<UTranspersonalAnimationComponent>();
        
        UE_LOG(LogTemp, Log, TEXT("TranspersonalAnimInstance: Initialized for %s"), *OwningPawn->GetName());
        
        // Initialize personality based on character type
        if (AnimationComponent)
        {
            CharacterType = AnimationComponent->GetCharacterArchetype();
            ApplyPersonalityToAnimation();
        }
    }
}

void UTranspersonalAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);
    
    if (!OwningPawn || !MovementComponent)
    {
        return;
    }
    
    // Update basic movement data
    FVector Velocity = MovementComponent->Velocity;
    Speed = Velocity.Size();
    Direction = UKismetMathLibrary::CalculateDirection(Velocity, OwningPawn->GetActorRotation()).Yaw;
    
    bIsMoving = Speed > MIN_MOVEMENT_THRESHOLD;
    bIsFalling = MovementComponent->IsFalling();
    bIsJumping = MovementComponent->IsMovingOnGround() && Velocity.Z > 0;
    bIsCrouching = MovementComponent->IsCrouching();
    
    // Update timers
    FootIKUpdateTimer += DeltaTimeX;
    PersonalityUpdateTimer += DeltaTimeX;
    EmotionalInfluenceTimer += DeltaTimeX;
    
    if (!bIsMoving)
    {
        TimeSinceLastMovement += DeltaTimeX;
    }
    else
    {
        TimeSinceLastMovement = 0.0f;
    }
    
    // Update systems at different frequencies for performance
    UpdateMotionMatching();
    
    if (FootIKUpdateTimer >= FOOT_IK_UPDATE_FREQUENCY)
    {
        UpdateFootIK();
        FootIKUpdateTimer = 0.0f;
    }
    
    if (PersonalityUpdateTimer >= PERSONALITY_UPDATE_FREQUENCY)
    {
        UpdatePersonalityModifiers();
        ApplyPersonalityToAnimation();
        PersonalityUpdateTimer = 0.0f;
    }
    
    // Update dinosaur-specific behavior if applicable
    if (CharacterType == ECharacterArchetype::Dinosaur_Herbivore || 
        CharacterType == ECharacterArchetype::Dinosaur_Carnivore)
    {
        UpdateDinosaurBehavior();
    }
    
    // Cache for next frame
    LastVelocity = Velocity;
    LastSpeed = Speed;
}

void UTranspersonalAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaTimeX);
    
    // Thread-safe updates only
    UpdateEmotionalInfluence();
    ApplyPhysicalVariations();
}

void UTranspersonalAnimInstance::UpdateMotionMatching()
{
    if (!AnimationComponent)
    {
        return;
    }
    
    // Select appropriate database based on current state
    ActiveDatabase = SelectMotionMatchingDatabase();
    
    // Calculate blend time based on movement change
    BlendTime = CalculateBlendTime();
    
    // Update movement state for database selection
    if (bIsFalling)
    {
        MovementState = EMovementState::Falling;
    }
    else if (bIsJumping)
    {
        MovementState = EMovementState::Jumping;
    }
    else if (bIsCrouching)
    {
        MovementState = EMovementState::Crouching;
    }
    else if (bIsSneaking)
    {
        MovementState = EMovementState::Sneaking;
    }
    else if (bIsMoving)
    {
        if (Speed > 300.0f)
        {
            MovementState = EMovementState::Running;
        }
        else
        {
            MovementState = EMovementState::Walking;
        }
    }
    else
    {
        MovementState = EMovementState::Idle;
    }
}

UPoseSearchDatabase* UTranspersonalAnimInstance::SelectMotionMatchingDatabase()
{
    if (!AnimationComponent)
    {
        return ActiveDatabase;
    }
    
    // Get database from animation component based on current state
    return AnimationComponent->GetDatabaseForState(MovementState, EmotionalState, CharacterType);
}

float UTranspersonalAnimInstance::GetBlendTimeForTransition() const
{
    return BlendTime;
}

float UTranspersonalAnimInstance::CalculateBlendTime()
{
    float SpeedDelta = FMath::Abs(Speed - LastSpeed);
    float BaseBlendTime = 0.2f;
    
    // Faster blend for quick changes, slower for gradual
    if (SpeedDelta > 100.0f)
    {
        BaseBlendTime = 0.1f; // Quick response to sudden changes
    }
    else if (SpeedDelta < 10.0f)
    {
        BaseBlendTime = 0.3f; // Smooth for subtle changes
    }
    
    // Apply personality modifiers
    BaseBlendTime *= (2.0f - ConfidenceModifier); // Confident characters transition faster
    BaseBlendTime *= (1.0f + NervousnessModifier * 0.5f); // Nervous characters are more hesitant
    
    return FMath::Clamp(BaseBlendTime, 0.05f, 0.5f);
}

void UTranspersonalAnimInstance::UpdateFootIK()
{
    if (!ShouldUseFootIK())
    {
        LeftFootIKAlpha = 0.0f;
        RightFootIKAlpha = 0.0f;
        return;
    }
    
    CalculateFootPlacement();
}

bool UTranspersonalAnimInstance::ShouldUseFootIK() const
{
    // Enable Foot IK when on ground and moving slowly or stationary
    return MovementComponent && 
           MovementComponent->IsMovingOnGround() && 
           Speed < 400.0f; // Disable at high speeds for performance
}

void UTranspersonalAnimInstance::CalculateFootPlacement()
{
    if (!OwningPawn)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = OwningPawn->FindComponentByClass<USkeletalMeshComponent>();
    if (!MeshComp)
    {
        return;
    }
    
    // Get foot bone locations
    FVector LeftFootLocation = MeshComp->GetBoneLocation(TEXT("foot_l"));
    FVector RightFootLocation = MeshComp->GetBoneLocation(TEXT("foot_r"));
    
    // Trace for ground
    LeftFootEffectorLocation = TraceForGround(LeftFootLocation, LeftFootIKAlpha);
    RightFootEffectorLocation = TraceForGround(RightFootLocation, RightFootIKAlpha);
    
    // Calculate hip offset to keep character grounded
    float LeftOffset = (LeftFootEffectorLocation - LeftFootLocation).Z;
    float RightOffset = (RightFootEffectorLocation - RightFootLocation).Z;
    
    // Use the lower foot as reference to prevent floating
    float TargetHipOffset = FMath::Min(LeftOffset, RightOffset);
    HipOffset = FMath::FInterpTo(HipOffset, TargetHipOffset, GetWorld()->GetDeltaSeconds(), 5.0f);
    
    // Apply physical variations for unique movement
    LeftFootEffectorLocation.Z += BackCurvature * LeftLegLengthRatio;
    RightFootEffectorLocation.Z += BackCurvature * RightLegLengthRatio;
    
    // Cache for debug drawing
    LastLeftFootTrace = LeftFootEffectorLocation;
    LastRightFootTrace = RightFootEffectorLocation;
}

FVector UTranspersonalAnimInstance::TraceForGround(FVector FootLocation, float& OutIKAlpha)
{
    if (!OwningPawn)
    {
        OutIKAlpha = 0.0f;
        return FootLocation;
    }
    
    UWorld* World = OwningPawn->GetWorld();
    if (!World)
    {
        OutIKAlpha = 0.0f;
        return FootLocation;
    }
    
    // Trace downward from foot location
    FVector StartTrace = FootLocation + FVector(0, 0, 20.0f);
    FVector EndTrace = FootLocation - FVector(0, 0, FOOT_TRACE_DISTANCE);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwningPawn);
    
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartTrace,
        EndTrace,
        ECC_Visibility,
        QueryParams
    );
    
    if (bHit)
    {
        OutIKAlpha = 1.0f;
        
        // Calculate foot rotation based on surface normal
        FVector SurfaceNormal = HitResult.Normal;
        FRotator FootRotation = UKismetMathLibrary::MakeRotFromZX(SurfaceNormal, OwningPawn->GetActorForwardVector());
        
        // Apply to appropriate foot
        if (FootLocation.Y < 0) // Left foot (assuming standard skeleton)
        {
            LeftFootEffectorRotation = FootRotation;
        }
        else // Right foot
        {
            RightFootEffectorRotation = FootRotation;
        }
        
        return HitResult.Location;
    }
    
    OutIKAlpha = 0.0f;
    return FootLocation;
}

void UTranspersonalAnimInstance::UpdatePersonalityModifiers()
{
    if (!AnimationComponent)
    {
        return;
    }
    
    // Get current emotional state influence
    float EmotionalInfluence = AnimationComponent->GetEmotionalInfluence();
    
    // Apply emotional state to personality modifiers
    switch (EmotionalState)
    {
    case EEmotionalState::Confident:
        ConfidenceModifier = FMath::Lerp(1.0f, 1.3f, EmotionalInfluence);
        NervousnessModifier = FMath::Lerp(1.0f, 0.7f, EmotionalInfluence);
        SpeedModifier = FMath::Lerp(1.0f, 1.1f, EmotionalInfluence);
        break;
        
    case EEmotionalState::Nervous:
        ConfidenceModifier = FMath::Lerp(1.0f, 0.6f, EmotionalInfluence);
        NervousnessModifier = FMath::Lerp(1.0f, 1.5f, EmotionalInfluence);
        SpeedModifier = FMath::Lerp(1.0f, 0.9f, EmotionalInfluence);
        break;
        
    case EEmotionalState::Afraid:
        ConfidenceModifier = FMath::Lerp(1.0f, 0.4f, EmotionalInfluence);
        NervousnessModifier = FMath::Lerp(1.0f, 2.0f, EmotionalInfluence);
        SpeedModifier = FMath::Lerp(1.0f, 1.3f, EmotionalInfluence); // Fear can make you faster
        WeightModifier = FMath::Lerp(1.0f, 0.8f, EmotionalInfluence); // Lighter steps when afraid
        break;
        
    case EEmotionalState::Aggressive:
        ConfidenceModifier = FMath::Lerp(1.0f, 1.4f, EmotionalInfluence);
        NervousnessModifier = FMath::Lerp(1.0f, 0.5f, EmotionalInfluence);
        WeightModifier = FMath::Lerp(1.0f, 1.3f, EmotionalInfluence); // Heavier, more deliberate steps
        break;
        
    case EEmotionalState::Curious:
        ConfidenceModifier = FMath::Lerp(1.0f, 1.1f, EmotionalInfluence);
        SpeedModifier = FMath::Lerp(1.0f, 0.8f, EmotionalInfluence); // Slower, more observant
        break;
        
    case EEmotionalState::Tired:
        SpeedModifier = FMath::Lerp(1.0f, 0.6f, EmotionalInfluence);
        WeightModifier = FMath::Lerp(1.0f, 1.2f, EmotionalInfluence); // Heavier steps when tired
        break;
    }
    
    // Clamp all modifiers to reasonable ranges
    ConfidenceModifier = FMath::Clamp(ConfidenceModifier, 0.3f, 2.0f);
    NervousnessModifier = FMath::Clamp(NervousnessModifier, 0.3f, 2.5f);
    SpeedModifier = FMath::Clamp(SpeedModifier, 0.5f, 1.5f);
    WeightModifier = FMath::Clamp(WeightModifier, 0.6f, 1.5f);
}

void UTranspersonalAnimInstance::SetEmotionalInfluence(EEmotionalState NewState, float Intensity)
{
    EmotionalState = NewState;
    
    if (AnimationComponent)
    {
        AnimationComponent->SetEmotionalInfluence(Intensity);
    }
    
    // Immediate update of modifiers for responsive feel
    UpdatePersonalityModifiers();
    
    UE_LOG(LogTemp, Log, TEXT("TranspersonalAnimInstance: Emotional state changed to %d with intensity %.2f"), 
           (int32)NewState, Intensity);
}

void UTranspersonalAnimInstance::ApplyPersonalityToAnimation()
{
    if (!AnimationComponent)
    {
        return;
    }
    
    // Update personality struct with current modifiers
    Personality.Confidence = ConfidenceModifier;
    Personality.Nervousness = NervousnessModifier;
    Personality.Weight = WeightModifier;
    Personality.Stride = SpeedModifier;
    
    // Apply archetype-specific personality traits
    switch (CharacterType)
    {
    case ECharacterArchetype::Paleontologist:
        Personality.Curiosity = FMath::Lerp(0.8f, 1.0f, ConfidenceModifier - 1.0f);
        Personality.Caution = FMath::Lerp(0.6f, 0.8f, NervousnessModifier - 1.0f);
        break;
        
    case ECharacterArchetype::Dinosaur_Herbivore:
        Personality.Caution = FMath::Lerp(0.7f, 1.0f, AlertnessLevel);
        Personality.Energy = EnergyLevel;
        break;
        
    case ECharacterArchetype::Dinosaur_Carnivore:
        Personality.Aggression = FMath::Lerp(0.3f, 1.0f, bIsHunting ? 1.0f : 0.0f);
        Personality.Confidence = FMath::Lerp(0.8f, 1.0f, EnergyLevel);
        break;
    }
}

void UTranspersonalAnimInstance::UpdateEmotionalInfluence()
{
    // Gradual decay of emotional influence over time
    if (AnimationComponent)
    {
        float CurrentInfluence = AnimationComponent->GetEmotionalInfluence();
        float DecayRate = 0.5f; // Emotions fade over time
        float NewInfluence = FMath::Max(0.0f, CurrentInfluence - DecayRate * GetWorld()->GetDeltaSeconds());
        
        AnimationComponent->SetEmotionalInfluence(NewInfluence);
    }
}

void UTranspersonalAnimInstance::ApplyPhysicalVariations()
{
    // Apply subtle physical variations that make each character unique
    // These affect the final pose slightly to create individual "signatures"
    
    // Leg length variations affect stride and foot placement
    if (LeftLegLengthRatio != 1.0f || RightLegLengthRatio != 1.0f)
    {
        // Adjust foot IK targets based on leg length ratios
        LeftFootEffectorLocation.Z *= LeftLegLengthRatio;
        RightFootEffectorLocation.Z *= RightLegLengthRatio;
    }
    
    // Back curvature affects spine posture
    if (FMath::Abs(BackCurvature) > 0.1f)
    {
        SpineTwist.Pitch = BackCurvature * 0.5f;
        SpineFlexAlpha = FMath::Abs(BackCurvature) / 5.0f; // Normalize to 0-1 range
    }
    
    // Head tilt bias affects look direction
    if (FMath::Abs(HeadTiltBias) > 0.1f)
    {
        // This would be applied to head bone rotation in the animation blueprint
        // For now, we store it for blueprint access
    }
}

void UTranspersonalAnimInstance::UpdateDinosaurBehavior()
{
    if (!AnimationComponent)
    {
        return;
    }
    
    // Update dinosaur-specific behavioral states
    UpdateAlertness();
    UpdateLookAt();
    
    // Update behavioral flags based on AI state
    // These would typically be set by the AI/Behavior Tree system
    
    // Hunger affects movement speed and alertness
    if (HungerLevel > 0.8f)
    {
        bIsFeeding = false;
        bIsHunting = (CharacterType == ECharacterArchetype::Dinosaur_Carnivore);
        SpeedModifier = FMath::Lerp(SpeedModifier, 1.2f, 0.1f); // Hungry animals move faster
    }
    else if (HungerLevel < 0.3f)
    {
        bIsFeeding = true;
        bIsHunting = false;
        SpeedModifier = FMath::Lerp(SpeedModifier, 0.8f, 0.1f); // Well-fed animals are slower
    }
    
    // Energy affects overall activity
    if (EnergyLevel < 0.3f)
    {
        bIsResting = true;
        bIsHunting = false;
        bIsFeeding = false;
        SpeedModifier = FMath::Lerp(SpeedModifier, 0.6f, 0.1f);
    }
    else
    {
        bIsResting = false;
    }
    
    // Alertness affects head movement and posture
    if (AlertnessLevel > 0.7f)
    {
        bIsAlert = true;
        Personality.HeadMovement = FMath::Lerp(Personality.HeadMovement, 1.0f, 0.1f);
        Personality.ShoulderTension = FMath::Lerp(Personality.ShoulderTension, 0.8f, 0.1f);
    }
    else
    {
        bIsAlert = false;
        Personality.HeadMovement = FMath::Lerp(Personality.HeadMovement, 0.3f, 0.05f);
        Personality.ShoulderTension = FMath::Lerp(Personality.ShoulderTension, 0.2f, 0.05f);
    }
}

void UTranspersonalAnimInstance::UpdateAlertness()
{
    // Alertness naturally decays over time unless there's a threat
    float DecayRate = 0.2f;
    AlertnessLevel = FMath::Max(0.0f, AlertnessLevel - DecayRate * GetWorld()->GetDeltaSeconds());
    
    // Check for nearby threats (this would typically be handled by AI perception)
    // For now, we simulate based on player proximity
    if (OwningPawn)
    {
        UWorld* World = OwningPawn->GetWorld();
        if (World)
        {
            APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
            if (PlayerPawn)
            {
                float DistanceToPlayer = FVector::Dist(OwningPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
                float ThreatDistance = 1000.0f; // 10 meters
                
                if (DistanceToPlayer < ThreatDistance)
                {
                    float ThreatLevel = 1.0f - (DistanceToPlayer / ThreatDistance);
                    AlertnessLevel = FMath::Max(AlertnessLevel, ThreatLevel);
                    bHasThreatTarget = true;
                    LookAtTarget = PlayerPawn->GetActorLocation();
                }
                else
                {
                    bHasThreatTarget = false;
                }
            }
        }
    }
}

void UTranspersonalAnimInstance::UpdateLookAt()
{
    if (bHasThreatTarget)
    {
        LookAtAlpha = FMath::FInterpTo(LookAtAlpha, 1.0f, GetWorld()->GetDeltaSeconds(), 3.0f);
    }
    else
    {
        LookAtAlpha = FMath::FInterpTo(LookAtAlpha, 0.0f, GetWorld()->GetDeltaSeconds(), 2.0f);
    }
    
    // Apply alertness to look at behavior
    LookAtAlpha *= AlertnessLevel;
}