#include "AnimationSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Engine/CollisionProfile.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrehistoricLocomotionDatabase = nullptr;
    FootIKTraceDistance = 50.0f;
    FootIKInterpSpeed = 10.0f;
    MaxFootIKOffset = 30.0f;
}

void UAnimationSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Initializing..."));
    
    // Load animation assets
    LoadAnimationAssets();
    
    // Setup terrain-specific blend spaces
    SetupTerrainBlendSpaces();
    
    // Setup survival action montages
    SetupSurvivalMontages();
    
    // Initialize Motion Matching database
    InitializeMotionMatchingDatabase();
    
    UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Initialization complete"));
}

void UAnimationSystemManager::Deinitialize()
{
    RegisteredCharacters.Empty();
    TerrainBlendSpaces.Empty();
    SurvivalMontages.Empty();
    PrehistoricLocomotionDatabase = nullptr;
    
    Super::Deinitialize();
}

void UAnimationSystemManager::InitializeMotionMatchingDatabase()
{
    // Try to load the Motion Matching database
    PrehistoricLocomotionDatabase = LoadObject<UMotionMatchingDatabase>(
        nullptr, 
        TEXT("/Game/Characters/Animation/MotionMatching/PrehistoricLocomotion_MM")
    );
    
    if (PrehistoricLocomotionDatabase)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Motion Matching database loaded successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Motion Matching database not found - using fallback system"));
    }
}

UAnimSequence* UAnimationSystemManager::FindBestMatchingAnimation(const FAnim_LocomotionData& LocomotionData)
{
    // If we have Motion Matching database, use it
    if (PrehistoricLocomotionDatabase)
    {
        // TODO: Implement Motion Matching query
        // For now, return nullptr to use blend spaces
        return nullptr;
    }
    
    // Fallback: Use traditional blend space system
    return nullptr;
}

void UAnimationSystemManager::RegisterCharacterForMotionMatching(APawn* Character)
{
    if (!Character)
    {
        return;
    }
    
    if (!ValidateCharacterForAnimation(Character))
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Character validation failed for %s"), *Character->GetName());
        return;
    }
    
    // Add to registered characters if not already present
    TWeakObjectPtr<APawn> CharacterPtr = Character;
    if (!RegisteredCharacters.Contains(CharacterPtr))
    {
        RegisteredCharacters.Add(CharacterPtr);
        UE_LOG(LogTemp, Log, TEXT("AnimationSystemManager: Registered character %s for motion matching"), *Character->GetName());
    }
}

FAnim_IKFootData UAnimationSystemManager::CalculateFootIK(APawn* Character, const FName& FootBoneName, float TraceDistance)
{
    FAnim_IKFootData FootData;
    
    if (!ValidateCharacterForAnimation(Character))
    {
        return FootData;
    }
    
    // Perform foot trace
    FVector FootWorldLocation = PerformFootTrace(Character, FootBoneName, TraceDistance);
    
    if (FootWorldLocation != FVector::ZeroVector)
    {
        FootData.bValidHit = true;
        FootData.FootLocation = FootWorldLocation;
        
        // Calculate foot rotation based on surface normal
        // For now, use default rotation - can be enhanced with surface normal calculation
        FootData.FootRotation = FRotator::ZeroRotator;
        FootData.IKAlpha = 1.0f;
    }
    
    return FootData;
}

void UAnimationSystemManager::UpdateTerrainAdaptation(APawn* Character)
{
    if (!ValidateCharacterForAnimation(Character))
    {
        return;
    }
    
    // Calculate both feet IK
    FAnim_IKFootData LeftFootData = CalculateFootIK(Character, TEXT("foot_l"), FootIKTraceDistance);
    FAnim_IKFootData RightFootData = CalculateFootIK(Character, TEXT("foot_r"), FootIKTraceDistance);
    
    // Apply IK data to character's animation instance
    USkeletalMeshComponent* MeshComp = GetCharacterMesh(Character);
    if (MeshComp && MeshComp->GetAnimInstance())
    {
        // Store IK data in animation instance for Blueprint access
        // This would typically be done through custom animation instance variables
        UE_LOG(LogTemp, VeryVerbose, TEXT("AnimationSystemManager: Updated terrain adaptation for %s"), *Character->GetName());
    }
}

void UAnimationSystemManager::ApplyEnvironmentalAnimations(APawn* Character, EAnim_TerrainType TerrainType)
{
    if (!ValidateCharacterForAnimation(Character))
    {
        return;
    }
    
    // Get terrain-specific blend space
    UBlendSpace* TerrainBlendSpace = GetTerrainSpecificBlendSpace(TerrainType);
    
    if (TerrainBlendSpace)
    {
        // Apply terrain-specific locomotion
        UE_LOG(LogTemp, VeryVerbose, TEXT("AnimationSystemManager: Applied %s terrain animations to %s"), 
            *UEnum::GetValueAsString(TerrainType), *Character->GetName());
    }
}

bool UAnimationSystemManager::PlaySurvivalMontage(APawn* Character, EAnim_SurvivalAction Action)
{
    if (!ValidateCharacterForAnimation(Character))
    {
        return false;
    }
    
    UAnimMontage** FoundMontage = SurvivalMontages.Find(Action);
    if (!FoundMontage || !*FoundMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: No montage found for survival action %s"), 
            *UEnum::GetValueAsString(Action));
        return false;
    }
    
    USkeletalMeshComponent* MeshComp = GetCharacterMesh(Character);
    if (MeshComp && MeshComp->GetAnimInstance())
    {
        float MontageLength = MeshComp->GetAnimInstance()->Montage_Play(*FoundMontage);
        if (MontageLength > 0.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("AnimationSystemManager: Playing survival montage %s for %s"), 
                *UEnum::GetValueAsString(Action), *Character->GetName());
            return true;
        }
    }
    
    return false;
}

void UAnimationSystemManager::StopAllMontages(APawn* Character)
{
    if (!ValidateCharacterForAnimation(Character))
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = GetCharacterMesh(Character);
    if (MeshComp && MeshComp->GetAnimInstance())
    {
        MeshComp->GetAnimInstance()->Montage_Stop(0.2f);
        UE_LOG(LogTemp, Log, TEXT("AnimationSystemManager: Stopped all montages for %s"), *Character->GetName());
    }
}

bool UAnimationSystemManager::IsPlayingMontage(APawn* Character, EAnim_SurvivalAction Action)
{
    if (!ValidateCharacterForAnimation(Character))
    {
        return false;
    }
    
    UAnimMontage** FoundMontage = SurvivalMontages.Find(Action);
    if (!FoundMontage || !*FoundMontage)
    {
        return false;
    }
    
    USkeletalMeshComponent* MeshComp = GetCharacterMesh(Character);
    if (MeshComp && MeshComp->GetAnimInstance())
    {
        return MeshComp->GetAnimInstance()->Montage_IsPlaying(*FoundMontage);
    }
    
    return false;
}

void UAnimationSystemManager::UpdateLocomotionBlendSpace(APawn* Character, const FAnim_LocomotionData& LocomotionData)
{
    if (!ValidateCharacterForAnimation(Character))
    {
        return;
    }
    
    // Get appropriate blend space for terrain type
    UBlendSpace* BlendSpace = GetTerrainSpecificBlendSpace(LocomotionData.TerrainType);
    
    if (BlendSpace)
    {
        // Update blend space parameters through animation instance
        USkeletalMeshComponent* MeshComp = GetCharacterMesh(Character);
        if (MeshComp && MeshComp->GetAnimInstance())
        {
            // This would typically update variables in a custom animation instance
            UE_LOG(LogTemp, VeryVerbose, TEXT("AnimationSystemManager: Updated locomotion blend space for %s (Speed: %.2f, Direction: %.2f)"), 
                *Character->GetName(), LocomotionData.Speed, LocomotionData.Direction);
        }
    }
}

UBlendSpace* UAnimationSystemManager::GetTerrainSpecificBlendSpace(EAnim_TerrainType TerrainType)
{
    UBlendSpace** FoundBlendSpace = TerrainBlendSpaces.Find(TerrainType);
    if (FoundBlendSpace && *FoundBlendSpace)
    {
        return *FoundBlendSpace;
    }
    
    // Return default grass blend space if specific terrain not found
    UBlendSpace** DefaultBlendSpace = TerrainBlendSpaces.Find(EAnim_TerrainType::Grass);
    return (DefaultBlendSpace && *DefaultBlendSpace) ? *DefaultBlendSpace : nullptr;
}

void UAnimationSystemManager::TriggerFootstepEvent(APawn* Character, const FName& FootBoneName)
{
    if (!ValidateCharacterForAnimation(Character))
    {
        return;
    }
    
    // Trigger footstep sound/VFX based on terrain type
    // This would typically interface with the Audio system
    UE_LOG(LogTemp, VeryVerbose, TEXT("AnimationSystemManager: Triggered footstep event for %s (%s)"), 
        *Character->GetName(), *FootBoneName.ToString());
}

void UAnimationSystemManager::TriggerSurvivalActionEvent(APawn* Character, EAnim_SurvivalAction Action)
{
    if (!ValidateCharacterForAnimation(Character))
    {
        return;
    }
    
    // Trigger survival action events (sounds, particles, gameplay effects)
    UE_LOG(LogTemp, Log, TEXT("AnimationSystemManager: Triggered survival action event %s for %s"), 
        *UEnum::GetValueAsString(Action), *Character->GetName());
}

void UAnimationSystemManager::LoadAnimationAssets()
{
    // Load animation assets from content browser
    // This is typically done through asset references or soft object pointers
    UE_LOG(LogTemp, Log, TEXT("AnimationSystemManager: Loading animation assets..."));
}

void UAnimationSystemManager::SetupTerrainBlendSpaces()
{
    // Load terrain-specific blend spaces
    struct FTerrainBlendSpaceData
    {
        EAnim_TerrainType TerrainType;
        FString AssetPath;
    };
    
    TArray<FTerrainBlendSpaceData> BlendSpaceData = {
        { EAnim_TerrainType::Grass, TEXT("/Game/Characters/Animation/BlendSpaces/BS_GrassLocomotion") },
        { EAnim_TerrainType::Rock, TEXT("/Game/Characters/Animation/BlendSpaces/BS_RockLocomotion") },
        { EAnim_TerrainType::Sand, TEXT("/Game/Characters/Animation/BlendSpaces/BS_SandLocomotion") },
        { EAnim_TerrainType::Mud, TEXT("/Game/Characters/Animation/BlendSpaces/BS_MudLocomotion") },
        { EAnim_TerrainType::Snow, TEXT("/Game/Characters/Animation/BlendSpaces/BS_SnowLocomotion") },
        { EAnim_TerrainType::Water, TEXT("/Game/Characters/Animation/BlendSpaces/BS_WaterLocomotion") }
    };
    
    for (const auto& Data : BlendSpaceData)
    {
        UBlendSpace* BlendSpace = LoadObject<UBlendSpace>(nullptr, *Data.AssetPath);
        if (BlendSpace)
        {
            TerrainBlendSpaces.Add(Data.TerrainType, BlendSpace);
            UE_LOG(LogTemp, Log, TEXT("AnimationSystemManager: Loaded blend space for %s terrain"), 
                *UEnum::GetValueAsString(Data.TerrainType));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Failed to load blend space for %s terrain"), 
                *UEnum::GetValueAsString(Data.TerrainType));
        }
    }
}

void UAnimationSystemManager::SetupSurvivalMontages()
{
    // Load survival action montages
    struct FSurvivalMontageData
    {
        EAnim_SurvivalAction Action;
        FString AssetPath;
    };
    
    TArray<FSurvivalMontageData> MontageData = {
        { EAnim_SurvivalAction::Gathering, TEXT("/Game/Characters/Animation/Montages/AM_Gather_Berries") },
        { EAnim_SurvivalAction::Crafting, TEXT("/Game/Characters/Animation/Montages/AM_Craft_Tool") },
        { EAnim_SurvivalAction::Hunting, TEXT("/Game/Characters/Animation/Montages/AM_Hunt_Throw") },
        { EAnim_SurvivalAction::Climbing, TEXT("/Game/Characters/Animation/Montages/AM_Climb_Rock") },
        { EAnim_SurvivalAction::Swimming, TEXT("/Game/Characters/Animation/Montages/AM_Swim_River") },
        { EAnim_SurvivalAction::Resting, TEXT("/Game/Characters/Animation/Montages/AM_Rest_Fire") }
    };
    
    for (const auto& Data : MontageData)
    {
        UAnimMontage* Montage = LoadObject<UAnimMontage>(nullptr, *Data.AssetPath);
        if (Montage)
        {
            SurvivalMontages.Add(Data.Action, Montage);
            UE_LOG(LogTemp, Log, TEXT("AnimationSystemManager: Loaded montage for %s action"), 
                *UEnum::GetValueAsString(Data.Action));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AnimationSystemManager: Failed to load montage for %s action"), 
                *UEnum::GetValueAsString(Data.Action));
        }
    }
}

FVector UAnimationSystemManager::PerformFootTrace(APawn* Character, const FName& FootBoneName, float TraceDistance)
{
    if (!Character)
    {
        return FVector::ZeroVector;
    }
    
    USkeletalMeshComponent* MeshComp = GetCharacterMesh(Character);
    if (!MeshComp)
    {
        return FVector::ZeroVector;
    }
    
    // Get foot bone location
    FVector FootBoneLocation = MeshComp->GetBoneLocation(FootBoneName);
    if (FootBoneLocation == FVector::ZeroVector)
    {
        return FVector::ZeroVector;
    }
    
    // Perform line trace downward
    FVector TraceStart = FootBoneLocation + FVector(0, 0, 20.0f);
    FVector TraceEnd = FootBoneLocation - FVector(0, 0, TraceDistance);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    
    UWorld* World = Character->GetWorld();
    if (World && World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Location;
    }
    
    return FVector::ZeroVector;
}

FRotator UAnimationSystemManager::CalculateFootRotationFromNormal(const FVector& SurfaceNormal)
{
    // Calculate foot rotation to align with surface normal
    FVector ForwardVector = FVector::ForwardVector;
    FVector RightVector = FVector::CrossProduct(SurfaceNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, SurfaceNormal).GetSafeNormal();
    
    return FRotationMatrix::MakeFromXZ(ForwardVector, SurfaceNormal).Rotator();
}

bool UAnimationSystemManager::ValidateCharacterForAnimation(APawn* Character)
{
    if (!Character)
    {
        return false;
    }
    
    USkeletalMeshComponent* MeshComp = GetCharacterMesh(Character);
    if (!MeshComp || !MeshComp->GetAnimInstance())
    {
        return false;
    }
    
    return true;
}

USkeletalMeshComponent* UAnimationSystemManager::GetCharacterMesh(APawn* Character)
{
    if (!Character)
    {
        return nullptr;
    }
    
    // Try to get mesh from Character class first
    if (ACharacter* CharacterCast = Cast<ACharacter>(Character))
    {
        return CharacterCast->GetMesh();
    }
    
    // Fallback: look for skeletal mesh component
    return Character->FindComponentByClass<USkeletalMeshComponent>();
}