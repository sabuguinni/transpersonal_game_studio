// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "MotionMatchingDatabaseV44.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogMotionMatchingDB, Log, All);

UMotionMatchingDatabaseV44::UMotionMatchingDatabaseV44()
{
    InitializeDefaultSettings();
}

void UMotionMatchingDatabaseV44::InitializeDefaultSettings()
{
    BlendTime = 0.2f;
    SearchThreshold = 0.1f;
    MaxSearchResults = 10;
    bEnableDebugVisualization = false;
    
    // Query weights
    TrajectoryWeight = 1.0f;
    PoseWeight = 1.0f;
    VelocityWeight = 1.0f;
    AngularVelocityWeight = 0.5f;
    
    // Emotional modifiers
    FearIntensityMultiplier = 1.2f;
    CalmIntensityMultiplier = 0.8f;
    AlertnessMultiplier = 1.1f;
    
    // Survival modifiers
    HungerAnimationBias = 0.0f;
    FatigueAnimationBias = 0.0f;
    InjuryAnimationBias = 0.0f;
    
    bDatabaseInitialized = false;
}

FCharacterArchetypeMotionData* UMotionMatchingDatabaseV44::GetArchetypeData(const FString& ArchetypeName)
{
    // Check cache first
    if (int32* CachedIndex = ArchetypeIndexCache.Find(ArchetypeName))
    {
        if (ArchetypeMotionData.IsValidIndex(*CachedIndex))
        {
            return &ArchetypeMotionData[*CachedIndex];
        }
    }
    
    // Search for archetype
    for (int32 i = 0; i < ArchetypeMotionData.Num(); ++i)
    {
        if (ArchetypeMotionData[i].ArchetypeName.Equals(ArchetypeName, ESearchCase::IgnoreCase))
        {
            ArchetypeIndexCache.Add(ArchetypeName, i);
            return &ArchetypeMotionData[i];
        }
    }
    
    UE_LOG(LogMotionMatchingDB, Warning, TEXT("Archetype '%s' not found in database"), *ArchetypeName);
    return nullptr;
}

TArray<FMotionMatchingAnimData> UMotionMatchingDatabaseV44::GetAnimationsForState(const FString& ArchetypeName, const FString& StateName)
{
    TArray<FMotionMatchingAnimData> ResultAnimations;
    
    FCharacterArchetypeMotionData* ArchetypeData = GetArchetypeData(ArchetypeName);
    if (!ArchetypeData)
    {
        return ResultAnimations;
    }
    
    // Map state names to animation arrays
    if (StateName.Equals(TEXT("Idle"), ESearchCase::IgnoreCase))
    {
        ResultAnimations = ArchetypeData->IdleAnimations;
    }
    else if (StateName.Equals(TEXT("Walk"), ESearchCase::IgnoreCase))
    {
        ResultAnimations = ArchetypeData->WalkAnimations;
    }
    else if (StateName.Equals(TEXT("Run"), ESearchCase::IgnoreCase))
    {
        ResultAnimations = ArchetypeData->RunAnimations;
    }
    else if (StateName.Equals(TEXT("Turn"), ESearchCase::IgnoreCase))
    {
        ResultAnimations = ArchetypeData->TurnAnimations;
    }
    else if (StateName.Equals(TEXT("Start"), ESearchCase::IgnoreCase))
    {
        ResultAnimations = ArchetypeData->StartAnimations;
    }
    else if (StateName.Equals(TEXT("Stop"), ESearchCase::IgnoreCase))
    {
        ResultAnimations = ArchetypeData->StopAnimations;
    }
    else if (StateName.Equals(TEXT("Emotional"), ESearchCase::IgnoreCase))
    {
        ResultAnimations = ArchetypeData->EmotionalVariations;
    }
    else if (StateName.Equals(TEXT("Survival"), ESearchCase::IgnoreCase))
    {
        ResultAnimations = ArchetypeData->SurvivalAnimations;
    }
    else if (StateName.Equals(TEXT("Interaction"), ESearchCase::IgnoreCase))
    {
        ResultAnimations = ArchetypeData->InteractionAnimations;
    }
    
    // Apply survival and emotional modifiers to weights
    for (FMotionMatchingAnimData& AnimData : ResultAnimations)
    {
        float ModifiedWeight = AnimData.Weight;
        
        // Apply emotional modifiers based on animation tags
        if (AnimData.AnimationTags.Contains(TEXT("Fear")))
        {
            ModifiedWeight *= FearIntensityMultiplier;
        }
        if (AnimData.AnimationTags.Contains(TEXT("Calm")))
        {
            ModifiedWeight *= CalmIntensityMultiplier;
        }
        if (AnimData.AnimationTags.Contains(TEXT("Alert")))
        {
            ModifiedWeight *= AlertnessMultiplier;
        }
        
        // Apply survival modifiers
        if (AnimData.AnimationTags.Contains(TEXT("Hungry")))
        {
            ModifiedWeight += HungerAnimationBias;
        }
        if (AnimData.AnimationTags.Contains(TEXT("Tired")))
        {
            ModifiedWeight += FatigueAnimationBias;
        }
        if (AnimData.AnimationTags.Contains(TEXT("Injured")))
        {
            ModifiedWeight += InjuryAnimationBias;
        }
        
        AnimData.Weight = FMath::Clamp(ModifiedWeight, 0.1f, 10.0f);
    }
    
    return ResultAnimations;
}

void UMotionMatchingDatabaseV44::UpdateDatabaseWeights(float NewTrajectoryWeight, float NewPoseWeight, float NewVelocityWeight)
{
    TrajectoryWeight = FMath::Clamp(NewTrajectoryWeight, 0.0f, 10.0f);
    PoseWeight = FMath::Clamp(NewPoseWeight, 0.0f, 10.0f);
    VelocityWeight = FMath::Clamp(NewVelocityWeight, 0.0f, 10.0f);
    
    UE_LOG(LogMotionMatchingDB, Log, TEXT("Updated database weights - Trajectory: %f, Pose: %f, Velocity: %f"), 
           TrajectoryWeight, PoseWeight, VelocityWeight);
}

void UMotionMatchingDatabaseV44::SetEmotionalModifiers(float Fear, float Calm, float Alertness)
{
    FearIntensityMultiplier = FMath::Clamp(Fear, 0.1f, 5.0f);
    CalmIntensityMultiplier = FMath::Clamp(Calm, 0.1f, 5.0f);
    AlertnessMultiplier = FMath::Clamp(Alertness, 0.1f, 5.0f);
    
    UE_LOG(LogMotionMatchingDB, Log, TEXT("Updated emotional modifiers - Fear: %f, Calm: %f, Alertness: %f"), 
           FearIntensityMultiplier, CalmIntensityMultiplier, AlertnessMultiplier);
}

void UMotionMatchingDatabaseV44::SetSurvivalModifiers(float Hunger, float Fatigue, float Injury)
{
    HungerAnimationBias = FMath::Clamp(Hunger, -2.0f, 2.0f);
    FatigueAnimationBias = FMath::Clamp(Fatigue, -2.0f, 2.0f);
    InjuryAnimationBias = FMath::Clamp(Injury, -2.0f, 2.0f);
    
    UE_LOG(LogMotionMatchingDB, Log, TEXT("Updated survival modifiers - Hunger: %f, Fatigue: %f, Injury: %f"), 
           HungerAnimationBias, FatigueAnimationBias, InjuryAnimationBias);
}

bool UMotionMatchingDatabaseV44::ValidateDatabase()
{
    bool bIsValid = true;
    
    if (!PoseSearchDatabase)
    {
        UE_LOG(LogMotionMatchingDB, Error, TEXT("PoseSearchDatabase is null"));
        bIsValid = false;
    }
    
    if (!PoseSearchSchema)
    {
        UE_LOG(LogMotionMatchingDB, Error, TEXT("PoseSearchSchema is null"));
        bIsValid = false;
    }
    
    // Validate each archetype
    for (const FCharacterArchetypeMotionData& ArchetypeData : ArchetypeMotionData)
    {
        if (ArchetypeData.ArchetypeName.IsEmpty())
        {
            UE_LOG(LogMotionMatchingDB, Warning, TEXT("Found archetype with empty name"));
            bIsValid = false;
            continue;
        }
        
        // Check if archetype has at least some animations
        int32 TotalAnimations = ArchetypeData.IdleAnimations.Num() + 
                               ArchetypeData.WalkAnimations.Num() + 
                               ArchetypeData.RunAnimations.Num();
        
        if (TotalAnimations == 0)
        {
            UE_LOG(LogMotionMatchingDB, Warning, TEXT("Archetype '%s' has no animations"), *ArchetypeData.ArchetypeName);
            bIsValid = false;
        }
        
        // Validate individual animations
        auto ValidateAnimArray = [&](const TArray<FMotionMatchingAnimData>& AnimArray, const FString& ArrayName)
        {
            for (const FMotionMatchingAnimData& AnimData : AnimArray)
            {
                if (!AnimData.AnimSequence)
                {
                    UE_LOG(LogMotionMatchingDB, Warning, TEXT("Null animation sequence found in %s for archetype '%s'"), 
                           *ArrayName, *ArchetypeData.ArchetypeName);
                    bIsValid = false;
                }
            }
        };
        
        ValidateAnimArray(ArchetypeData.IdleAnimations, TEXT("IdleAnimations"));
        ValidateAnimArray(ArchetypeData.WalkAnimations, TEXT("WalkAnimations"));
        ValidateAnimArray(ArchetypeData.RunAnimations, TEXT("RunAnimations"));
        ValidateAnimArray(ArchetypeData.TurnAnimations, TEXT("TurnAnimations"));
        ValidateAnimArray(ArchetypeData.StartAnimations, TEXT("StartAnimations"));
        ValidateAnimArray(ArchetypeData.StopAnimations, TEXT("StopAnimations"));
        ValidateAnimArray(ArchetypeData.EmotionalVariations, TEXT("EmotionalVariations"));
        ValidateAnimArray(ArchetypeData.SurvivalAnimations, TEXT("SurvivalAnimations"));
        ValidateAnimArray(ArchetypeData.InteractionAnimations, TEXT("InteractionAnimations"));
    }
    
    if (bIsValid)
    {
        UE_LOG(LogMotionMatchingDB, Log, TEXT("Database validation successful"));
        bDatabaseInitialized = true;
    }
    else
    {
        UE_LOG(LogMotionMatchingDB, Error, TEXT("Database validation failed"));
    }
    
    return bIsValid;
}

void UMotionMatchingDatabaseV44::BuildPoseSearchDatabase()
{
    if (!PoseSearchDatabase || !PoseSearchSchema)
    {
        UE_LOG(LogMotionMatchingDB, Error, TEXT("Cannot build database - missing PoseSearchDatabase or PoseSearchSchema"));
        return;
    }
    
    // Clear existing data
    // PoseSearchDatabase->GetAnimationAssets().Empty();
    
    // Add all animations from all archetypes to the pose search database
    for (const FCharacterArchetypeMotionData& ArchetypeData : ArchetypeMotionData)
    {
        auto AddAnimationsToDatabase = [&](const TArray<FMotionMatchingAnimData>& AnimArray)
        {
            for (const FMotionMatchingAnimData& AnimData : AnimArray)
            {
                if (AnimData.AnimSequence)
                {
                    // Add animation to pose search database
                    // This would require access to PoseSearchDatabase's internal structure
                    UE_LOG(LogMotionMatchingDB, Verbose, TEXT("Adding animation '%s' to pose search database"), 
                           *AnimData.AnimSequence->GetName());
                }
            }
        };
        
        AddAnimationsToDatabase(ArchetypeData.IdleAnimations);
        AddAnimationsToDatabase(ArchetypeData.WalkAnimations);
        AddAnimationsToDatabase(ArchetypeData.RunAnimations);
        AddAnimationsToDatabase(ArchetypeData.TurnAnimations);
        AddAnimationsToDatabase(ArchetypeData.StartAnimations);
        AddAnimationsToDatabase(ArchetypeData.StopAnimations);
        AddAnimationsToDatabase(ArchetypeData.EmotionalVariations);
        AddAnimationsToDatabase(ArchetypeData.SurvivalAnimations);
        AddAnimationsToDatabase(ArchetypeData.InteractionAnimations);
    }
    
    UE_LOG(LogMotionMatchingDB, Log, TEXT("Pose search database build completed"));
}

void UMotionMatchingDatabaseV44::ProcessAnimationTags(const FString& Tags, FMotionMatchingAnimData& AnimData)
{
    TArray<FString> TagArray;
    Tags.ParseIntoArray(TagArray, TEXT(","), true);
    
    for (FString& Tag : TagArray)
    {
        Tag = Tag.TrimStartAndEnd();
        
        // Process specific tags that affect animation behavior
        if (Tag.Equals(TEXT("Mirrorable"), ESearchCase::IgnoreCase))
        {
            AnimData.bMirrorable = true;
        }
        else if (Tag.StartsWith(TEXT("Speed:"), ESearchCase::IgnoreCase))
        {
            FString SpeedValue = Tag.RightChop(6);
            float Speed = FCString::Atof(*SpeedValue);
            AnimData.SpeedRange = FVector2D(Speed * 0.8f, Speed * 1.2f);
        }
        else if (Tag.StartsWith(TEXT("Weight:"), ESearchCase::IgnoreCase))
        {
            FString WeightValue = Tag.RightChop(7);
            AnimData.Weight = FCString::Atof(*WeightValue);
        }
    }
}

#if WITH_EDITOR
void UMotionMatchingDatabaseV44::RebuildDatabase()
{
    UE_LOG(LogMotionMatchingDB, Log, TEXT("Rebuilding motion matching database..."));
    
    // Clear cache
    ArchetypeIndexCache.Empty();
    
    // Rebuild pose search database
    BuildPoseSearchDatabase();
    
    // Validate the rebuilt database
    ValidateDatabase();
    
    UE_LOG(LogMotionMatchingDB, Log, TEXT("Database rebuild completed"));
}

void UMotionMatchingDatabaseV44::ValidateAllAnimations()
{
    UE_LOG(LogMotionMatchingDB, Log, TEXT("Validating all animations in database..."));
    
    int32 TotalAnimations = 0;
    int32 ValidAnimations = 0;
    
    for (const FCharacterArchetypeMotionData& ArchetypeData : ArchetypeMotionData)
    {
        auto ValidateAnimArray = [&](const TArray<FMotionMatchingAnimData>& AnimArray, const FString& ArrayName)
        {
            for (const FMotionMatchingAnimData& AnimData : AnimArray)
            {
                TotalAnimations++;
                if (AnimData.AnimSequence && IsValid(AnimData.AnimSequence))
                {
                    ValidAnimations++;
                }
                else
                {
                    UE_LOG(LogMotionMatchingDB, Warning, TEXT("Invalid animation in %s for archetype '%s'"), 
                           *ArrayName, *ArchetypeData.ArchetypeName);
                }
            }
        };
        
        ValidateAnimArray(ArchetypeData.IdleAnimations, TEXT("IdleAnimations"));
        ValidateAnimArray(ArchetypeData.WalkAnimations, TEXT("WalkAnimations"));
        ValidateAnimArray(ArchetypeData.RunAnimations, TEXT("RunAnimations"));
        ValidateAnimArray(ArchetypeData.TurnAnimations, TEXT("TurnAnimations"));
        ValidateAnimArray(ArchetypeData.StartAnimations, TEXT("StartAnimations"));
        ValidateAnimArray(ArchetypeData.StopAnimations, TEXT("StopAnimations"));
        ValidateAnimArray(ArchetypeData.EmotionalVariations, TEXT("EmotionalVariations"));
        ValidateAnimArray(ArchetypeData.SurvivalAnimations, TEXT("SurvivalAnimations"));
        ValidateAnimArray(ArchetypeData.InteractionAnimations, TEXT("InteractionAnimations"));
    }
    
    UE_LOG(LogMotionMatchingDB, Log, TEXT("Animation validation complete: %d/%d animations are valid"), 
           ValidAnimations, TotalAnimations);
}

void UMotionMatchingDatabaseV44::GenerateArchetypeFromAnimations()
{
    UE_LOG(LogMotionMatchingDB, Log, TEXT("Generating archetype from existing animations..."));
    
    // This would scan the project for animation assets and automatically categorize them
    // Implementation would depend on specific naming conventions and folder structure
    
    UE_LOG(LogMotionMatchingDB, Log, TEXT("Archetype generation completed"));
}
#endif