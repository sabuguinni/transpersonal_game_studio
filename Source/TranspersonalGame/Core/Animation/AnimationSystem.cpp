// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "AnimationSystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimBlueprint.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

UAnimationSystem::UAnimationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    CurrentMovementState = EMovementAnimState::Idle;
    CurrentEmotionalState = EEmotionalAnimState::Neutral;
    CurrentSurvivalContext = ESurvivalAnimContext::Normal;
    
    // Motion Matching defaults
    MotionMatchingConfig.BlendTime = 0.2f;
    MotionMatchingConfig.SearchThreshold = 0.1f;
    
    // Foot IK defaults
    FootIKConfig.TraceDistance = 50.0f;
    FootIKConfig.InterpSpeed = 10.0f;
    FootIKConfig.MaxFootOffset = 30.0f;
    FootIKConfig.EnableFootRotation = true;
    
    // Initialize foot IK state
    LeftFootOffset = 0.0f;
    RightFootOffset = 0.0f;
    HipOffset = 0.0f;
    LeftFootRotation = FRotator::ZeroRotator;
    RightFootRotation = FRotator::ZeroRotator;
    
    // Character reference
    OwnerCharacter = nullptr;
    CharacterMesh = nullptr;
    
    bIsInitialized = false;
    bEnableFootIK = true;
    bEnableMotionMatching = true;
    bDebugMode = false;
}

void UAnimationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CharacterMesh = OwnerCharacter->GetMesh();
        
        // Initialize with default traits based on character type
        FCharacterAnimationTraits DefaultTraits = GetAnimationTraitsForArchetype(ECharacterArchetype::Protagonist);
        SetAnimationTraits(DefaultTraits);
        
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystem: Initialized for character %s"), 
               *OwnerCharacter->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem: No valid character owner found!"));
    }
}

void UAnimationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !CharacterMesh)
        return;
    
    // Update motion matching
    if (bEnableMotionMatching)
    {
        FVector Velocity = OwnerCharacter->GetVelocity();
        FVector Acceleration = OwnerCharacter->GetCharacterMovement()->GetCurrentAcceleration();
        UpdateMotionMatching(Velocity, Acceleration, DeltaTime);
    }
    
    // Update foot IK
    if (bEnableFootIK)
    {
        UpdateFootIK(DeltaTime);
    }
    
    // Update emotional state based on survival context
    UpdateEmotionalState(DeltaTime);
    
    // Apply character-specific animation modifications
    ApplyCharacterTraits(DeltaTime);
}

void UAnimationSystem::InitializeCharacterAnimation(ACharacter* Character, const FCharacterAnimationTraits& Traits)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystem: Cannot initialize with null character"));
        return;
    }
    
    OwnerCharacter = Character;
    CharacterMesh = Character->GetMesh();
    
    SetAnimationTraits(Traits);
    
    // Setup motion matching databases based on character type
    SetupMotionMatchingForCharacter(Traits.CharacterType);
    
    // Configure foot IK for character skeleton
    ConfigureFootIKForSkeleton();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystem: Character animation initialized for %s (Type: %s)"), 
           *Traits.CharacterName, 
           *UEnum::GetValueAsString(Traits.CharacterType));
}

void UAnimationSystem::SetAnimationTraits(const FCharacterAnimationTraits& NewTraits)
{
    AnimationTraits = NewTraits;
    
    // Apply traits to character movement
    if (OwnerCharacter && OwnerCharacter->GetCharacterMovement())
    {
        UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
        
        // Modify movement speeds based on traits
        float BaseWalkSpeed = 150.0f;
        float BaseRunSpeed = 375.0f;
        
        Movement->MaxWalkSpeed = BaseWalkSpeed * AnimationTraits.WalkSpeed;
        Movement->MaxWalkSpeedCrouched = BaseWalkSpeed * 0.5f * AnimationTraits.WalkSpeed;
        
        // Adjust based on fitness and age
        float AgeFactor = FMath::Clamp(1.0f - (AnimationTraits.Age - 25) * 0.01f, 0.7f, 1.0f);
        float FitnessFactor = FMath::Lerp(0.8f, 1.2f, AnimationTraits.Fitness);
        
        Movement->MaxWalkSpeed *= AgeFactor * FitnessFactor;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Animation traits updated for %s"), 
           *AnimationTraits.CharacterName);
}

FCharacterAnimationTraits UAnimationSystem::GetAnimationTraitsForArchetype(ECharacterArchetype Archetype)
{
    FCharacterAnimationTraits Traits;
    Traits.CharacterType = Archetype;
    
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist:
            Traits.CharacterName = TEXT("Dr. Paleontologist");
            Traits.WalkSpeed = 1.0f;
            Traits.RunSpeed = 1.0f;
            Traits.PostureHeight = 0.85f; // Academic posture
            Traits.ConfidenceLevel = 0.7f;
            Traits.CautiousnessFactor = 0.6f;
            Traits.EnergyLevel = 0.8f;
            Traits.AcademicPosture = 0.9f;
            Traits.Age = 35;
            Traits.Fitness = 0.7f;
            Traits.HasAcademicBackground = true;
            Traits.HasFieldworkExperience = true;
            Traits.SurvivalExperience = 0.1f; // New to this environment
            break;
            
        case ECharacterArchetype::Survivor:
            Traits.CharacterName = TEXT("Experienced Survivor");
            Traits.WalkSpeed = 1.1f;
            Traits.RunSpeed = 1.2f;
            Traits.PostureHeight = 0.75f; // Lower, more alert
            Traits.ConfidenceLevel = 0.9f;
            Traits.CautiousnessFactor = 0.8f;
            Traits.EnergyLevel = 0.9f;
            Traits.AcademicPosture = 0.2f;
            Traits.Age = 40;
            Traits.Fitness = 0.9f;
            Traits.SurvivalExperience = 0.9f;
            break;
            
        case ECharacterArchetype::Scholar:
            Traits.CharacterName = TEXT("Academic Scholar");
            Traits.WalkSpeed = 0.9f;
            Traits.RunSpeed = 0.8f;
            Traits.PostureHeight = 0.9f; // Very upright
            Traits.ConfidenceLevel = 0.8f;
            Traits.CautiousnessFactor = 0.4f;
            Traits.EnergyLevel = 0.6f;
            Traits.AcademicPosture = 1.0f;
            Traits.Age = 50;
            Traits.Fitness = 0.5f;
            Traits.HasAcademicBackground = true;
            Traits.SurvivalExperience = 0.0f;
            break;
            
        case ECharacterArchetype::Explorer:
            Traits.CharacterName = TEXT("Field Explorer");
            Traits.WalkSpeed = 1.2f;
            Traits.RunSpeed = 1.3f;
            Traits.PostureHeight = 0.8f;
            Traits.ConfidenceLevel = 0.9f;
            Traits.CautiousnessFactor = 0.7f;
            Traits.EnergyLevel = 1.0f;
            Traits.AcademicPosture = 0.5f;
            Traits.Age = 30;
            Traits.Fitness = 0.95f;
            Traits.HasFieldworkExperience = true;
            Traits.SurvivalExperience = 0.6f;
            break;
            
        default:
            // Use default values from struct constructor
            break;
    }
    
    return Traits;
}

void UAnimationSystem::UpdateMotionMatching(FVector Velocity, FVector Acceleration, float DeltaTime)
{
    if (!OwnerCharacter || !bEnableMotionMatching)
        return;
    
    // Determine movement state based on velocity
    float Speed = Velocity.Size2D();
    EMovementAnimState NewState = CurrentMovementState;
    
    if (Speed < 5.0f)
    {
        NewState = EMovementAnimState::Idle;
    }
    else if (Speed < 200.0f)
    {
        NewState = EMovementAnimState::Walking;
    }
    else
    {
        NewState = EMovementAnimState::Running;
    }
    
    // Check for special movement states
    if (OwnerCharacter->GetCharacterMovement()->IsCrouching())
    {
        NewState = EMovementAnimState::Crouching;
    }
    else if (OwnerCharacter->GetCharacterMovement()->IsFalling())
    {
        NewState = EMovementAnimState::Falling;
    }
    
    // Transition if state changed
    if (NewState != CurrentMovementState)
    {
        TransitionToMovementState(NewState);
    }
    
    // Update motion matching parameters
    UpdateMotionMatchingParameters(Velocity, Acceleration, DeltaTime);
}

void UAnimationSystem::SetMotionMatchingDatabase(UPoseSearchDatabase* Database)
{
    if (Database)
    {
        MotionMatchingConfig.LocomotionDatabase = Database;
        UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Motion matching database set"));
    }
}

void UAnimationSystem::TransitionToMovementState(EMovementAnimState NewState)
{
    if (NewState == CurrentMovementState)
        return;
    
    EMovementAnimState PreviousState = CurrentMovementState;
    CurrentMovementState = NewState;
    
    // Apply character-specific state modifications
    ApplyStateTransitionModifications(PreviousState, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: State transition %s -> %s"), 
           *UEnum::GetValueAsString(PreviousState),
           *UEnum::GetValueAsString(NewState));
}

void UAnimationSystem::SetEmotionalState(EEmotionalAnimState NewState)
{
    if (NewState == CurrentEmotionalState)
        return;
    
    CurrentEmotionalState = NewState;
    
    // Modify animation traits based on emotional state
    ApplyEmotionalStateModifications();
    
    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Emotional state changed to %s"), 
           *UEnum::GetValueAsString(NewState));
}

void UAnimationSystem::SetSurvivalContext(ESurvivalAnimContext NewContext)
{
    if (NewContext == CurrentSurvivalContext)
        return;
    
    CurrentSurvivalContext = NewContext;
    
    // Apply survival context modifications
    ApplySurvivalContextModifications();
    
    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Survival context changed to %s"), 
           *UEnum::GetValueAsString(NewContext));
}

void UAnimationSystem::UpdateFootIK(float DeltaTime)
{
    if (!CharacterMesh || !bEnableFootIK)
        return;
    
    // Perform foot traces
    FVector LeftFootLocation, RightFootLocation;
    FRotator LeftFootRot, RightFootRot;
    
    bool bLeftHit = PerformFootTrace(FootIKConfig.LeftFootBone, LeftFootLocation, LeftFootRot);
    bool bRightHit = PerformFootTrace(FootIKConfig.RightFootBone, RightFootLocation, RightFootRot);
    
    // Calculate foot offsets
    float NewLeftOffset = 0.0f;
    float NewRightOffset = 0.0f;
    
    if (bLeftHit)
    {
        FVector FootWorldPos = CharacterMesh->GetBoneLocation(FootIKConfig.LeftFootBone);
        NewLeftOffset = LeftFootLocation.Z - FootWorldPos.Z;
        NewLeftOffset = FMath::Clamp(NewLeftOffset, -FootIKConfig.MaxFootOffset, FootIKConfig.MaxFootOffset);
    }
    
    if (bRightHit)
    {
        FVector FootWorldPos = CharacterMesh->GetBoneLocation(FootIKConfig.RightFootBone);
        NewRightOffset = RightFootLocation.Z - FootWorldPos.Z;
        NewRightOffset = FMath::Clamp(NewRightOffset, -FootIKConfig.MaxFootOffset, FootIKConfig.MaxFootOffset);
    }
    
    // Interpolate offsets
    LeftFootOffset = FMath::FInterpTo(LeftFootOffset, NewLeftOffset, DeltaTime, FootIKConfig.InterpSpeed);
    RightFootOffset = FMath::FInterpTo(RightFootOffset, NewRightOffset, DeltaTime, FootIKConfig.InterpSpeed);
    
    // Calculate hip offset (average of foot offsets)
    float TargetHipOffset = (LeftFootOffset + RightFootOffset) * 0.5f;
    HipOffset = FMath::FInterpTo(HipOffset, TargetHipOffset, DeltaTime, FootIKConfig.InterpSpeed);
    
    // Update foot rotations
    if (FootIKConfig.EnableFootRotation)
    {
        LeftFootRotation = FMath::RInterpTo(LeftFootRotation, LeftFootRot, DeltaTime, FootIKConfig.InterpSpeed);
        RightFootRotation = FMath::RInterpTo(RightFootRotation, RightFootRot, DeltaTime, FootIKConfig.InterpSpeed);
    }
    
    // Debug visualization
    if (bDebugMode)
    {
        DrawFootIKDebug();
    }
}

bool UAnimationSystem::PerformFootTrace(FName FootBone, FVector& OutLocation, FRotator& OutRotation)
{
    if (!CharacterMesh || !OwnerCharacter)
        return false;
    
    FVector FootWorldPos = CharacterMesh->GetBoneLocation(FootBone);
    FVector StartTrace = FootWorldPos + FVector(0, 0, 20.0f);
    FVector EndTrace = FootWorldPos - FVector(0, 0, FootIKConfig.TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartTrace,
        EndTrace,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        OutLocation = HitResult.Location;
        
        // Calculate rotation based on surface normal
        FVector UpVector = HitResult.Normal;
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector RightVector = FVector::CrossProduct(ForwardVector, UpVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(UpVector, RightVector).GetSafeNormal();
        
        OutRotation = FRotationMatrix::MakeFromXZ(ForwardVector, UpVector).Rotator();
        
        return true;
    }
    
    return false;
}

void UAnimationSystem::SetupMotionMatchingForCharacter(ECharacterArchetype CharacterType)
{
    // This would typically load different motion matching databases
    // based on character archetype
    
    // For now, we'll set up basic configuration
    switch (CharacterType)
    {
        case ECharacterArchetype::Protagonist:
            MotionMatchingConfig.BlendTime = 0.25f; // Slightly slower, more deliberate
            MotionMatchingConfig.SearchThreshold = 0.15f;
            break;
            
        case ECharacterArchetype::Survivor:
            MotionMatchingConfig.BlendTime = 0.15f; // Quick, reactive
            MotionMatchingConfig.SearchThreshold = 0.1f;
            break;
            
        case ECharacterArchetype::Scholar:
            MotionMatchingConfig.BlendTime = 0.3f; // Slower, more careful
            MotionMatchingConfig.SearchThreshold = 0.2f;
            break;
            
        case ECharacterArchetype::Explorer:
            MotionMatchingConfig.BlendTime = 0.2f; // Balanced
            MotionMatchingConfig.SearchThreshold = 0.12f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Motion matching configured for %s"), 
           *UEnum::GetValueAsString(CharacterType));
}

void UAnimationSystem::ConfigureFootIKForSkeleton()
{
    if (!CharacterMesh)
        return;
    
    // Try to find appropriate bone names for the skeleton
    USkeletalMesh* SkeletalMesh = CharacterMesh->GetSkeletalMeshAsset();
    if (!SkeletalMesh)
        return;
    
    // Common bone name variations
    TArray<FString> LeftFootNames = {TEXT("foot_l"), TEXT("LeftFoot"), TEXT("L_Foot"), TEXT("foot_L")};
    TArray<FString> RightFootNames = {TEXT("foot_r"), TEXT("RightFoot"), TEXT("R_Foot"), TEXT("foot_R")};
    TArray<FString> HipNames = {TEXT("pelvis"), TEXT("Hips"), TEXT("Hip"), TEXT("Root")};
    
    // Find matching bones
    for (const FString& Name : LeftFootNames)
    {
        if (CharacterMesh->GetBoneIndex(*Name) != INDEX_NONE)
        {
            FootIKConfig.LeftFootBone = *Name;
            break;
        }
    }
    
    for (const FString& Name : RightFootNames)
    {
        if (CharacterMesh->GetBoneIndex(*Name) != INDEX_NONE)
        {
            FootIKConfig.RightFootBone = *Name;
            break;
        }
    }
    
    for (const FString& Name : HipNames)
    {
        if (CharacterMesh->GetBoneIndex(*Name) != INDEX_NONE)
        {
            FootIKConfig.HipBone = *Name;
            break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Foot IK configured - Left: %s, Right: %s, Hip: %s"),
           *FootIKConfig.LeftFootBone.ToString(),
           *FootIKConfig.RightFootBone.ToString(),
           *FootIKConfig.HipBone.ToString());
}

void UAnimationSystem::UpdateMotionMatchingParameters(FVector Velocity, FVector Acceleration, float DeltaTime)
{
    // Update trajectory prediction
    FVector PredictedVelocity = Velocity + (Acceleration * DeltaTime);
    
    // Store for motion matching query
    CurrentVelocity = Velocity;
    CurrentAcceleration = Acceleration;
    PredictedPosition = OwnerCharacter->GetActorLocation() + (PredictedVelocity * 0.5f);
    
    // Apply character trait modifications
    float ConfidenceModifier = FMath::Lerp(0.8f, 1.2f, AnimationTraits.ConfidenceLevel);
    float CautiousnessModifier = FMath::Lerp(1.2f, 0.9f, AnimationTraits.CautiousnessFactor);
    
    // Modify motion matching parameters based on traits
    float ModifiedBlendTime = MotionMatchingConfig.BlendTime * ConfidenceModifier * CautiousnessModifier;
    
    // Apply emotional state modifications
    switch (CurrentEmotionalState)
    {
        case EEmotionalAnimState::Fearful:
            ModifiedBlendTime *= 0.7f; // Quicker, more reactive
            break;
        case EEmotionalAnimState::Confident:
            ModifiedBlendTime *= 1.3f; // More deliberate
            break;
        case EEmotionalAnimState::Panicked:
            ModifiedBlendTime *= 0.5f; // Very quick, jerky
            break;
    }
    
    // Store the modified blend time for use in animation blueprint
    CurrentBlendTime = ModifiedBlendTime;
}

void UAnimationSystem::ApplyCharacterTraits(float DeltaTime)
{
    if (!OwnerCharacter)
        return;
    
    // Apply posture modifications based on traits
    // This would typically be done through animation blueprint variables
    
    // Example: Modify character movement based on traits
    UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
    if (Movement)
    {
        // Apply fatigue effects
        float FatigueModifier = 1.0f;
        if (CurrentSurvivalContext == ESurvivalAnimContext::Exhausted)
        {
            FatigueModifier = 0.7f;
        }
        
        // Apply injury effects
        if (CurrentSurvivalContext == ESurvivalAnimContext::Wounded)
        {
            FatigueModifier *= 0.8f;
        }
        
        // Apply the modifiers (this is a simplified example)
        // In a real implementation, this would be more sophisticated
    }
}

void UAnimationSystem::UpdateEmotionalState(float DeltaTime)
{
    // Simple emotional state logic based on survival context
    // In a real game, this would be much more complex
    
    switch (CurrentSurvivalContext)
    {
        case ESurvivalAnimContext::Hungry:
        case ESurvivalAnimContext::Thirsty:
            if (CurrentEmotionalState == EEmotionalAnimState::Neutral)
            {
                SetEmotionalState(EEmotionalAnimState::Cautious);
            }
            break;
            
        case ESurvivalAnimContext::Wounded:
            SetEmotionalState(EEmotionalAnimState::Fearful);
            break;
            
        case ESurvivalAnimContext::Cold:
        case ESurvivalAnimContext::Hot:
            if (CurrentEmotionalState == EEmotionalAnimState::Neutral)
            {
                SetEmotionalState(EEmotionalAnimState::Exhausted);
            }
            break;
    }
}

void UAnimationSystem::ApplyStateTransitionModifications(EMovementAnimState FromState, EMovementAnimState ToState)
{
    // Apply character-specific transition modifications
    float TransitionSpeed = 1.0f;
    
    // Academic characters have more deliberate transitions
    if (AnimationTraits.AcademicPosture > 0.7f)
    {
        TransitionSpeed *= 0.8f;
    }
    
    // Experienced survivors have quicker transitions
    if (AnimationTraits.SurvivalExperience > 0.5f)
    {
        TransitionSpeed *= 1.2f;
    }
    
    // Apply the transition speed modification
    // This would typically be passed to the animation blueprint
}

void UAnimationSystem::ApplyEmotionalStateModifications()
{
    // Modify animation parameters based on emotional state
    switch (CurrentEmotionalState)
    {
        case EEmotionalAnimState::Fearful:
            // Increase alertness, reduce confidence
            break;
            
        case EEmotionalAnimState::Confident:
            // Increase posture, reduce caution
            break;
            
        case EEmotionalAnimState::Exhausted:
            // Reduce energy, increase slouch
            break;
            
        case EEmotionalAnimState::Panicked:
            // Increase speed, reduce precision
            break;
    }
}

void UAnimationSystem::ApplySurvivalContextModifications()
{
    // Modify animation based on survival needs
    switch (CurrentSurvivalContext)
    {
        case ESurvivalAnimContext::Hungry:
            // Slightly hunched posture, slower movement
            break;
            
        case ESurvivalAnimContext::CarryingLoad:
            // Adjust posture for weight, slower movement
            break;
            
        case ESurvivalAnimContext::Wounded:
            // Favoring one side, protective posture
            break;
    }
}

void UAnimationSystem::DrawFootIKDebug()
{
    if (!GetWorld() || !OwnerCharacter)
        return;
    
    FVector ActorLocation = OwnerCharacter->GetActorLocation();
    
    // Draw foot offset indicators
    DrawDebugSphere(GetWorld(), ActorLocation + FVector(-50, 0, LeftFootOffset), 5.0f, 12, FColor::Red, false, 0.1f);
    DrawDebugSphere(GetWorld(), ActorLocation + FVector(50, 0, RightFootOffset), 5.0f, 12, FColor::Blue, false, 0.1f);
    DrawDebugSphere(GetWorld(), ActorLocation + FVector(0, 0, HipOffset), 5.0f, 12, FColor::Green, false, 0.1f);
    
    // Draw trace lines
    if (CharacterMesh)
    {
        FVector LeftFootPos = CharacterMesh->GetBoneLocation(FootIKConfig.LeftFootBone);
        FVector RightFootPos = CharacterMesh->GetBoneLocation(FootIKConfig.RightFootBone);
        
        DrawDebugLine(GetWorld(), LeftFootPos + FVector(0,0,20), LeftFootPos - FVector(0,0,FootIKConfig.TraceDistance), FColor::Red, false, 0.1f);
        DrawDebugLine(GetWorld(), RightFootPos + FVector(0,0,20), RightFootPos - FVector(0,0,FootIKConfig.TraceDistance), FColor::Blue, false, 0.1f);
    }
}

// Blueprint-accessible getters
float UAnimationSystem::GetLeftFootOffset() const
{
    return LeftFootOffset;
}

float UAnimationSystem::GetRightFootOffset() const
{
    return RightFootOffset;
}

float UAnimationSystem::GetHipOffset() const
{
    return HipOffset;
}

FRotator UAnimationSystem::GetLeftFootRotation() const
{
    return LeftFootRotation;
}

FRotator UAnimationSystem::GetRightFootRotation() const
{
    return RightFootRotation;
}

EMovementAnimState UAnimationSystem::GetCurrentMovementState() const
{
    return CurrentMovementState;
}

EEmotionalAnimState UAnimationSystem::GetCurrentEmotionalState() const
{
    return CurrentEmotionalState;
}

ESurvivalAnimContext UAnimationSystem::GetCurrentSurvivalContext() const
{
    return CurrentSurvivalContext;
}

FCharacterAnimationTraits UAnimationSystem::GetCurrentAnimationTraits() const
{
    return AnimationTraits;
}

void UAnimationSystem::SetDebugMode(bool bEnabled)
{
    bDebugMode = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Debug mode %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UAnimationSystem::SetEnableFootIK(bool bEnabled)
{
    bEnableFootIK = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Foot IK %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UAnimationSystem::SetEnableMotionMatching(bool bEnabled)
{
    bEnableMotionMatching = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("AnimationSystem: Motion Matching %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}