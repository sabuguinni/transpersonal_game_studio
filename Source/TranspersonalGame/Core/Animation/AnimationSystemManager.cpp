#include "AnimationSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    bFootIKEnabled = true;
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 15.0f;
    CurrentQualityLevel = 3; // High quality by default
    bAnimationLODEnabled = true;
}

void UAnimationSystemManager::InitializeAnimationSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Initializing Animation System"));
    
    InitializeDefaultProfiles();
    SetupMotionMatchingDefaults();
    
    // Clear existing state
    CharacterAnimationStates.Empty();
    StateCallbacks.Empty();
    CharacterDistances.Empty();
    CharacterLODLevels.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Animation System initialized successfully"));
}

void UAnimationSystemManager::RegisterCharacterAnimationProfile(const FCharacterAnimationProfile& Profile)
{
    if (!ValidateAnimationProfile(Profile))
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystemManager: Invalid animation profile for character %s"), *Profile.CharacterName);
        return;
    }
    
    CharacterProfiles.Add(Profile.CharacterName, Profile);
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Registered animation profile for %s"), *Profile.CharacterName);
}

FCharacterAnimationProfile UAnimationSystemManager::GetAnimationProfileForCharacter(const FString& CharacterName)
{
    if (CharacterProfiles.Contains(CharacterName))
    {
        return CharacterProfiles[CharacterName];
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Profile not found for %s, returning default"), *CharacterName);
    return FCharacterAnimationProfile();
}

void UAnimationSystemManager::ApplyAnimationProfileToCharacter(ACharacter* Character, const FString& ProfileName)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystemManager: Character is null"));
        return;
    }
    
    if (!CharacterProfiles.Contains(ProfileName))
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystemManager: Profile %s not found"), *ProfileName);
        return;
    }
    
    const FCharacterAnimationProfile& Profile = CharacterProfiles[ProfileName];
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    
    if (MeshComp && Profile.AnimBlueprintClass)
    {
        MeshComp->SetAnimInstanceClass(Profile.AnimBlueprintClass);
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Applied profile %s to character"), *ProfileName);
    }
}

void UAnimationSystemManager::ConfigureMotionMatching(const FMotionMatchingConfig& Config)
{
    MotionMatchingSettings = Config;
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Motion Matching configured - BlendTime: %f, SearchRadius: %f"), 
           Config.BlendTime, Config.SearchRadius);
}

void UAnimationSystemManager::UpdateMotionMatchingDatabase(const TArray<UAnimSequence*>& Animations)
{
    MotionMatchingDatabase = Animations;
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Motion Matching database updated with %d animations"), 
           Animations.Num());
}

UAnimSequence* UAnimationSystemManager::FindBestMatchingAnimation(ACharacter* Character, const FVector& Velocity, const FVector& Acceleration)
{
    if (!Character || MotionMatchingDatabase.Num() == 0)
    {
        return nullptr;
    }
    
    // Simplified motion matching algorithm
    float BestScore = FLT_MAX;
    UAnimSequence* BestMatch = nullptr;
    
    for (UAnimSequence* Anim : MotionMatchingDatabase)
    {
        if (!Anim) continue;
        
        // Calculate matching score based on velocity and acceleration
        float VelocityMagnitude = Velocity.Size();
        float Score = FMath::Abs(VelocityMagnitude - 100.0f); // Simplified scoring
        
        if (Score < BestScore)
        {
            BestScore = Score;
            BestMatch = Anim;
        }
    }
    
    return BestMatch;
}

void UAnimationSystemManager::EnableFootIK(ACharacter* Character, bool bEnable)
{
    if (!Character) return;
    
    bFootIKEnabled = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Foot IK %s for character"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UAnimationSystemManager::SetIKTargets(ACharacter* Character, const FVector& LeftFootTarget, const FVector& RightFootTarget)
{
    if (!Character || !bFootIKEnabled) return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (!MeshComp) return;
    
    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!AnimInstance) return;
    
    // Set IK targets through animation blueprint variables
    // This would typically be done through exposed variables in the AnimBP
    UE_LOG(LogTemp, Log, TEXT("AnimationSystemManager: Setting IK targets - Left: %s, Right: %s"), 
           *LeftFootTarget.ToString(), *RightFootTarget.ToString());
}

void UAnimationSystemManager::UpdateTerrainAdaptation(ACharacter* Character, float DeltaTime)
{
    if (!Character || !bFootIKEnabled) return;
    
    UWorld* World = Character->GetWorld();
    if (!World) return;
    
    FVector CharacterLocation = Character->GetActorLocation();
    FVector LeftFootLocation = CharacterLocation + FVector(-20.0f, 0.0f, 0.0f);
    FVector RightFootLocation = CharacterLocation + FVector(20.0f, 0.0f, 0.0f);
    
    // Trace for ground
    FHitResult LeftHit, RightHit;
    FVector TraceStart = LeftFootLocation + FVector(0.0f, 0.0f, 50.0f);
    FVector TraceEnd = LeftFootLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);
    
    bool bLeftHit = World->LineTraceSingleByChannel(LeftHit, TraceStart, TraceEnd, ECC_WorldStatic);
    
    TraceStart = RightFootLocation + FVector(0.0f, 0.0f, 50.0f);
    TraceEnd = RightFootLocation - FVector(0.0f, 0.0f, FootIKTraceDistance);
    
    bool bRightHit = World->LineTraceSingleByChannel(RightHit, TraceStart, TraceEnd, ECC_WorldStatic);
    
    if (bLeftHit && bRightHit)
    {
        SetIKTargets(Character, LeftHit.Location, RightHit.Location);
    }
}

void UAnimationSystemManager::TransitionToAnimationState(ACharacter* Character, const FString& StateName, float BlendTime)
{
    if (!Character) return;
    
    CharacterAnimationStates.Add(Character, StateName);
    
    // Execute callback if registered
    if (StateCallbacks.Contains(StateName))
    {
        UE_LOG(LogTemp, Log, TEXT("AnimationSystemManager: Executing callback for state %s"), *StateName);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Transitioning to state %s with blend time %f"), 
           *StateName, BlendTime);
}

FString UAnimationSystemManager::GetCurrentAnimationState(ACharacter* Character)
{
    if (CharacterAnimationStates.Contains(Character))
    {
        return CharacterAnimationStates[Character];
    }
    return TEXT("Unknown");
}

void UAnimationSystemManager::RegisterAnimationStateCallback(const FString& StateName, const FString& CallbackFunction)
{
    StateCallbacks.Add(StateName, CallbackFunction);
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Registered callback %s for state %s"), 
           *CallbackFunction, *StateName);
}

void UAnimationSystemManager::PlayEmotionalAnimation(ACharacter* Character, const FString& EmotionType, float Intensity)
{
    if (!Character) return;
    
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Playing emotional animation %s with intensity %f"), 
           *EmotionType, Intensity);
    
    // This would typically trigger specific emotional animation layers
    TransitionToAnimationState(Character, FString::Printf(TEXT("Emotion_%s"), *EmotionType), 0.5f);
}

void UAnimationSystemManager::BlendEmotionalLayers(ACharacter* Character, const TMap<FString, float>& EmotionWeights)
{
    if (!Character) return;
    
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Blending emotional layers with %d emotions"), 
           EmotionWeights.Num());
    
    // Implementation would blend multiple emotional animation layers
    for (const auto& Emotion : EmotionWeights)
    {
        UE_LOG(LogTemp, Log, TEXT("Emotion: %s, Weight: %f"), *Emotion.Key, Emotion.Value);
    }
}

void UAnimationSystemManager::SetCharacterPersonality(ACharacter* Character, const FString& PersonalityType)
{
    if (!Character) return;
    
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Setting personality %s for character"), *PersonalityType);
    
    // Personality affects base animation choices and emotional responses
    // This would modify the character's animation profile dynamically
}

void UAnimationSystemManager::SetAnimationQualityLevel(int32 QualityLevel)
{
    CurrentQualityLevel = FMath::Clamp(QualityLevel, 1, 4);
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Animation quality set to level %d"), CurrentQualityLevel);
}

void UAnimationSystemManager::OptimizeAnimationsForDistance(ACharacter* Character, float DistanceToPlayer)
{
    if (!Character) return;
    
    CharacterDistances.Add(Character, DistanceToPlayer);
    
    if (bAnimationLODEnabled)
    {
        UpdateAnimationLOD(Character, DistanceToPlayer);
    }
}

void UAnimationSystemManager::EnableAnimationLOD(bool bEnable)
{
    bAnimationLODEnabled = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Animation LOD %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UAnimationSystemManager::InitializeDefaultProfiles()
{
    // Create default human profile
    FCharacterAnimationProfile HumanProfile;
    HumanProfile.CharacterName = TEXT("Human");
    CharacterProfiles.Add(TEXT("Human"), HumanProfile);
    
    // Create default NPC profile
    FCharacterAnimationProfile NPCProfile;
    NPCProfile.CharacterName = TEXT("NPC");
    CharacterProfiles.Add(TEXT("NPC"), NPCProfile);
    
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Default profiles initialized"));
}

void UAnimationSystemManager::SetupMotionMatchingDefaults()
{
    MotionMatchingSettings.bUseMotionMatching = true;
    MotionMatchingSettings.BlendTime = 0.2f;
    MotionMatchingSettings.SearchRadius = 100.0f;
    MotionMatchingSettings.MaxCandidates = 10;
    MotionMatchingSettings.FeatureChannels.Add(TEXT("Velocity"));
    MotionMatchingSettings.FeatureChannels.Add(TEXT("Position"));
    MotionMatchingSettings.FeatureChannels.Add(TEXT("Trajectory"));
}

bool UAnimationSystemManager::ValidateAnimationProfile(const FCharacterAnimationProfile& Profile)
{
    if (Profile.CharacterName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("AnimationSystemManager: Character name cannot be empty"));
        return false;
    }
    
    return true;
}

void UAnimationSystemManager::ApplyPerformanceOptimizations(ACharacter* Character)
{
    if (!Character) return;
    
    float Distance = CharacterDistances.FindRef(Character);
    
    // Apply different optimizations based on distance and quality level
    if (Distance > 1000.0f && CurrentQualityLevel < 3)
    {
        // Reduce animation update rate for distant characters
        USkeletalMeshComponent* MeshComp = Character->GetMesh();
        if (MeshComp)
        {
            MeshComp->SetUpdateAnimationInEditor(false);
        }
    }
}

void UAnimationSystemManager::UpdateAnimationLOD(ACharacter* Character, float Distance)
{
    if (!Character) return;
    
    int32 LODLevel = 0;
    
    if (Distance > 500.0f)
        LODLevel = 1;
    if (Distance > 1000.0f)
        LODLevel = 2;
    if (Distance > 2000.0f)
        LODLevel = 3;
    
    CharacterLODLevels.Add(Character, LODLevel);
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if (MeshComp)
    {
        MeshComp->SetForcedLOD(LODLevel + 1);
    }
}