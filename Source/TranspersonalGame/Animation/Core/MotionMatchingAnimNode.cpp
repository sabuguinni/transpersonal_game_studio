#include "MotionMatchingAnimNode.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimTrace.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

DECLARE_CYCLE_STAT(TEXT("TranspersonalMotionMatching_Evaluate"), STAT_TranspersonalMotionMatching_Evaluate, STATGROUP_Anim);
DECLARE_CYCLE_STAT(TEXT("TranspersonalMotionMatching_Update"), STAT_TranspersonalMotionMatching_Update, STATGROUP_Anim);

FAnimNode_TranspersonalMotionMatching::FAnimNode_TranspersonalMotionMatching()
{
    BlendTime = 0.2f;
    MotionMatchingAlpha = 1.0f;
    CharacterType = ECharacterArchetype::Paleontologist;
    EmotionalState = EEmotionalState::Confident;
    bSurvivalMode = false;
    AlertnessLevel = 0.0f;
    bThreatDetected = false;
    ThreatDirection = FVector::ZeroVector;
    BodyWeight = 1.0f;
    BodyHeight = 1.0f;
    bUseVelocityBasedBlending = true;
    VelocityThreshold = 50.0f;
    bUseMovementPrediction = true;
    PredictionTime = 0.5f;
    bShowDebugInfo = false;
    DebugColor = FColor::Green;
    
    // Initialize cached values
    CurrentBlendWeight = 0.0f;
    BlendTimeRemaining = 0.0f;
    PersonalityUpdateTimer = 0.0f;
    LastAlertness = 0.0f;
    bWasThreatDetected = false;
    LastThreatDirection = FVector::ZeroVector;
}

void FAnimNode_TranspersonalMotionMatching::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
    DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread);
    
    FAnimNode_Base::Initialize_AnyThread(Context);
    Source.Initialize(Context);
    
    // Initialize personality cache
    CachedPersonality = PersonalityModifiers;
    PersonalityUpdateTimer = 0.0f;
    
    UE_LOG(LogAnimation, Log, TEXT("TranspersonalMotionMatching: Initialized for character type %d"), (int32)CharacterType);
}

void FAnimNode_TranspersonalMotionMatching::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
    DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread);
    
    FAnimNode_Base::CacheBones_AnyThread(Context);
    Source.CacheBones(Context);
}

void FAnimNode_TranspersonalMotionMatching::PreUpdate(const UAnimInstance* InAnimInstance)
{
    DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(PreUpdate);
    
    if (!InAnimInstance)
    {
        return;
    }
    
    // Update personality cache periodically
    PersonalityUpdateTimer += InAnimInstance->GetDeltaSeconds();
    if (PersonalityUpdateTimer >= PERSONALITY_UPDATE_FREQUENCY)
    {
        UpdatePersonalityCache(InAnimInstance);
        UpdateBehaviorCache(InAnimInstance);
        PersonalityUpdateTimer = 0.0f;
    }
    
    // Build trajectory for pose search
    if (Database && Schema)
    {
        BuildQueryTrajector(InAnimInstance);
    }
}

void FAnimNode_TranspersonalMotionMatching::Update_AnyThread(const FAnimationUpdateContext& Context)
{
    DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread);
    SCOPE_CYCLE_COUNTER(STAT_TranspersonalMotionMatching_Update);
    
    FAnimNode_Base::Update_AnyThread(Context);
    Source.Update(Context);
    
    // Update blend timing
    if (BlendTimeRemaining > 0.0f)
    {
        BlendTimeRemaining -= Context.GetDeltaTime();
        BlendTimeRemaining = FMath::Max(0.0f, BlendTimeRemaining);
        
        // Calculate blend weight based on remaining time
        float TotalBlendTime = CalculatePersonalityBlendTime();
        CurrentBlendWeight = BlendTimeRemaining > 0.0f ? (1.0f - (BlendTimeRemaining / TotalBlendTime)) : 1.0f;
    }
    
    TRACE_ANIM_NODE_VALUE(Context, TEXT("MotionMatchingAlpha"), MotionMatchingAlpha);
    TRACE_ANIM_NODE_VALUE(Context, TEXT("BlendWeight"), CurrentBlendWeight);
    TRACE_ANIM_NODE_VALUE(Context, TEXT("AlertnessLevel"), AlertnessLevel);
}

void FAnimNode_TranspersonalMotionMatching::Evaluate_AnyThread(FPoseContext& Output)
{
    DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread);
    SCOPE_CYCLE_COUNTER(STAT_TranspersonalMotionMatching_Evaluate);
    
    // Start with source pose
    Source.Evaluate(Output);
    
    // Early exit if no database or schema
    if (!Database || !Schema || MotionMatchingAlpha <= 0.0f)
    {
        return;
    }
    
    // Perform pose search
    FPoseSearchResult SearchResult;
    if (PerformPoseSearch(Output.AnimInstanceProxy->GetAnimInstanceObject(), SearchResult))
    {
        // Apply motion matching result
        if (SearchResult.Database && SearchResult.PoseIdx != INDEX_NONE)
        {
            // Get the animation sequence and time from search result
            if (const UAnimSequence* FoundSequence = SearchResult.Database->GetAnimationAssetStruct(SearchResult.AssetIdx).GetAnimationAsset<UAnimSequence>())
            {
                // Create a pose context for the found animation
                FPoseContext MotionMatchingPose(Output);
                
                // Sample the animation at the found time
                FAnimExtractContext ExtractionContext(SearchResult.TimeOffsetSeconds, false);
                FoundSequence->GetBonePose(MotionMatchingPose.Pose, MotionMatchingPose.Curve, ExtractionContext);
                
                // Apply personality modifications to the motion matching pose
                ApplyPersonalityModifications(MotionMatchingPose);
                ApplyPhysicalVariations(MotionMatchingPose);
                
                // Blend between source and motion matching pose
                float FinalAlpha = MotionMatchingAlpha * CurrentBlendWeight;\n                if (FinalAlpha > 0.0f && FinalAlpha < 1.0f)\n                {\n                    Output.Pose.BlendWith(MotionMatchingPose.Pose, FinalAlpha);\n                    Output.Curve.Blend(MotionMatchingPose.Curve, FinalAlpha);\n                }\n                else if (FinalAlpha >= 1.0f)\n                {\n                    Output.Pose = MotionMatchingPose.Pose;\n                    Output.Curve = MotionMatchingPose.Curve;\n                }\n                \n                // Store this result for next frame\n                LastSearchResult = SearchResult;\n            }\n        }\n    }\n    \n    // Draw debug information if enabled\n    if (bShowDebugInfo)\n    {\n        DrawDebugInformation(Output.AnimInstanceProxy->GetAnimInstanceObject());\n    }\n}\n\nvoid FAnimNode_TranspersonalMotionMatching::GatherDebugData(FNodeDebugData& DebugData)\n{\n    FString DebugLine = DebugData.GetNodeName(this);\n    DebugLine += FString::Printf(TEXT(\"(CharType: %d, Emotion: %d, Alert: %.2f)\"), \n        (int32)CharacterType, (int32)EmotionalState, AlertnessLevel);\n    \n    if (Database)\n    {\n        DebugLine += FString::Printf(TEXT(\", DB: %s\"), *Database->GetName());\n    }\n    \n    DebugData.AddDebugItem(DebugLine);\n    Source.GatherDebugData(DebugData);\n}\n\nvoid FAnimNode_TranspersonalMotionMatching::UpdatePersonalityCache(const UAnimInstance* AnimInstance)\n{\n    if (!AnimInstance)\n    {\n        return;\n    }\n    \n    // Update cached personality based on current state\n    CachedPersonality = PersonalityModifiers;\n    \n    // Apply emotional state modifications\n    switch (EmotionalState)\n    {\n        case EEmotionalState::Confident:\n            CachedPersonality.ConfidenceLevel = FMath::Clamp(CachedPersonality.ConfidenceLevel + 0.2f, 0.0f, 1.0f);\n            CachedPersonality.MovementSpeed = FMath::Clamp(CachedPersonality.MovementSpeed + 0.1f, 0.5f, 2.0f);\n            break;\n            \n        case EEmotionalState::Nervous:\n            CachedPersonality.NervousnessLevel = FMath::Clamp(CachedPersonality.NervousnessLevel + 0.3f, 0.0f, 1.0f);\n            CachedPersonality.MovementSpeed = FMath::Clamp(CachedPersonality.MovementSpeed - 0.1f, 0.5f, 2.0f);\n            break;\n            \n        case EEmotionalState::Fearful:\n            CachedPersonality.NervousnessLevel = FMath::Clamp(CachedPersonality.NervousnessLevel + 0.5f, 0.0f, 1.0f);\n            CachedPersonality.ConfidenceLevel = FMath::Clamp(CachedPersonality.ConfidenceLevel - 0.3f, 0.0f, 1.0f);\n            break;\n            \n        case EEmotionalState::Aggressive:\n            CachedPersonality.ConfidenceLevel = FMath::Clamp(CachedPersonality.ConfidenceLevel + 0.4f, 0.0f, 1.0f);\n            CachedPersonality.MovementSpeed = FMath::Clamp(CachedPersonality.MovementSpeed + 0.2f, 0.5f, 2.0f);\n            break;\n    }\n    \n    // Apply survival mode modifications\n    if (bSurvivalMode)\n    {\n        CachedPersonality.NervousnessLevel = FMath::Clamp(CachedPersonality.NervousnessLevel + 0.2f, 0.0f, 1.0f);\n        CachedPersonality.Alertness = FMath::Clamp(CachedPersonality.Alertness + 0.3f, 0.0f, 1.0f);\n    }\n}\n\nvoid FAnimNode_TranspersonalMotionMatching::UpdateBehaviorCache(const UAnimInstance* AnimInstance)\n{\n    if (!AnimInstance)\n    {\n        return;\n    }\n    \n    // Detect changes in alertness\n    if (FMath::Abs(AlertnessLevel - LastAlertness) > 0.1f)\n    {\n        // Alertness changed significantly, trigger blend\n        BlendTimeRemaining = CalculatePersonalityBlendTime() * 0.5f; // Faster response to alertness\n        LastAlertness = AlertnessLevel;\n    }\n    \n    // Detect threat state changes\n    if (bThreatDetected != bWasThreatDetected)\n    {\n        // Threat state changed, immediate response\n        BlendTimeRemaining = CalculatePersonalityBlendTime() * 0.3f; // Very fast response to threats\n        bWasThreatDetected = bThreatDetected;\n        \n        if (bThreatDetected)\n        {\n            UE_LOG(LogAnimation, Log, TEXT(\"TranspersonalMotionMatching: Threat detected, adjusting animation\"));\n        }\n    }\n    \n    // Update threat direction\n    if (!ThreatDirection.Equals(LastThreatDirection, 10.0f))\n    {\n        LastThreatDirection = ThreatDirection;\n    }\n}\n\nUPoseSearchDatabase* FAnimNode_TranspersonalMotionMatching::SelectOptimalDatabase() const\n{\n    // For now, return the assigned database\n    // In the future, this could select different databases based on:\n    // - Character type\n    // - Emotional state\n    // - Physical variations\n    // - Environmental context\n    \n    return Database;\n}\n\nfloat FAnimNode_TranspersonalMotionMatching::CalculatePersonalityBlendTime() const\n{\n    float BaseTime = BlendTime;\n    \n    // Modify blend time based on personality\n    float PersonalityModifier = 1.0f;\n    \n    // Nervous characters blend faster (more reactive)\n    PersonalityModifier *= FMath::Lerp(1.0f, 0.7f, CachedPersonality.NervousnessLevel);\n    \n    // Confident characters can take their time\n    PersonalityModifier *= FMath::Lerp(1.0f, 1.3f, CachedPersonality.ConfidenceLevel);\n    \n    // High alertness = faster reactions\n    PersonalityModifier *= FMath::Lerp(1.0f, 0.5f, AlertnessLevel);\n    \n    // Threat detected = immediate response\n    if (bThreatDetected)\n    {\n        PersonalityModifier *= 0.3f;\n    }\n    \n    float FinalTime = BaseTime * PersonalityModifier;\n    return FMath::Clamp(FinalTime, MIN_BLEND_TIME, MAX_BLEND_TIME);\n}\n\nvoid FAnimNode_TranspersonalMotionMatching::ApplyPersonalityModifications(FPoseContext& Output) const\n{\n    // Apply subtle modifications to the pose based on personality\n    // This would involve bone transformations that reflect character traits\n    \n    // Example: Nervous characters might have slightly hunched shoulders\n    if (CachedPersonality.NervousnessLevel > 0.1f)\n    {\n        // Find spine bones and apply slight forward lean\n        // This is a simplified example - real implementation would be more sophisticated\n    }\n    \n    // Example: Confident characters might have straighter posture\n    if (CachedPersonality.ConfidenceLevel > 0.7f)\n    {\n        // Apply slight chest-out, shoulders-back adjustment\n    }\n}\n\nvoid FAnimNode_TranspersonalMotionMatching::ApplyPhysicalVariations(FPoseContext& Output) const\n{\n    // Apply physical trait variations\n    // This could involve:\n    // - Leg length adjustments\n    // - Spine curvature\n    // - Arm length differences\n    // - Weight distribution effects\n    \n    // Example implementation would modify bone transforms based on PhysicalTraits\n}\n\nvoid FAnimNode_TranspersonalMotionMatching::DrawDebugInformation(const UAnimInstance* AnimInstance) const\n{\n    if (!AnimInstance || !bShowDebugInfo)\n    {\n        return;\n    }\n    \n    UWorld* World = AnimInstance->GetWorld();\n    if (!World)\n    {\n        return;\n    }\n    \n    AActor* Owner = AnimInstance->GetOwningActor();\n    if (!Owner)\n    {\n        return;\n    }\n    \n    FVector ActorLocation = Owner->GetActorLocation();\n    FVector DebugLocation = ActorLocation + FVector(0, 0, 200);\n    \n    // Draw personality info\n    FString DebugText = FString::Printf(TEXT(\"Personality: C:%.1f N:%.1f A:%.1f\"), \n        CachedPersonality.ConfidenceLevel, CachedPersonality.NervousnessLevel, AlertnessLevel);\n    \n    DrawDebugString(World, DebugLocation, DebugText, nullptr, DebugColor, 0.0f);\n    \n    // Draw threat information if detected\n    if (bThreatDetected)\n    {\n        FVector ThreatDebugLocation = DebugLocation + FVector(0, 0, 20);\n        DrawDebugString(World, ThreatDebugLocation, TEXT(\"THREAT DETECTED\"), nullptr, FColor::Red, 0.0f);\n        \n        if (!ThreatDirection.IsZero())\n        {\n            FVector ThreatEnd = ActorLocation + (ThreatDirection.GetSafeNormal() * 100.0f);\n            DrawDebugDirectionalArrow(World, ActorLocation, ThreatEnd, 50.0f, FColor::Red, false, 0.0f, 0, 3.0f);\n        }\n    }\n}\n\nbool FAnimNode_TranspersonalMotionMatching::PerformPoseSearch(const UAnimInstance* AnimInstance, FPoseSearchResult& OutResult)\n{\n    if (!AnimInstance || !Database || !Schema)\n    {\n        return false;\n    }\n    \n    // This is a simplified pose search implementation\n    // Real implementation would use the full PoseSearch API\n    \n    // For now, just return a basic result\n    OutResult.Database = Database;\n    OutResult.PoseIdx = 0;\n    OutResult.AssetIdx = 0;\n    OutResult.TimeOffsetSeconds = 0.0f;\n    \n    return true;\n}\n\nvoid FAnimNode_TranspersonalMotionMatching::BuildQueryTrajector(const UAnimInstance* AnimInstance)\n{\n    if (!AnimInstance)\n    {\n        return;\n    }\n    \n    // Build trajectory for motion matching query\n    // This would sample the character's movement trajectory\n    // and create a query that the pose search can use\n    \n    // Implementation would depend on the specific PoseSearch API\n}\n\nfloat FAnimNode_TranspersonalMotionMatching::CalculateSearchCost(const FPoseSearchResult& Result) const\n{\n    // Calculate cost for pose search result\n    // This could factor in:\n    // - Distance from current pose\n    // - Personality compatibility\n    // - Physical trait compatibility\n    // - Emotional state appropriateness\n    \n    return 0.0f; // Simplified for now\n}\n\nfloat FAnimNode_TranspersonalMotionMatching::GetPersonalitySpeedModifier() const\n{\n    float Modifier = 1.0f;\n    \n    // Nervous characters move faster when alert\n    Modifier *= FMath::Lerp(1.0f, 1.2f, CachedPersonality.NervousnessLevel * AlertnessLevel);\n    \n    // Confident characters maintain steady pace\n    Modifier *= FMath::Lerp(0.9f, 1.1f, CachedPersonality.ConfidenceLevel);\n    \n    return FMath::Clamp(Modifier, 0.5f, 2.0f);\n}\n\nfloat FAnimNode_TranspersonalMotionMatching::GetPersonalityWeightModifier() const\n{\n    float Modifier = BodyWeight;\n    \n    // Emotional state affects perceived weight\n    switch (EmotionalState)\n    {\n        case EEmotionalState::Fearful:\n            Modifier *= 0.8f; // Fear makes movement lighter, more reactive\n            break;\n        case EEmotionalState::Confident:\n            Modifier *= 1.1f; // Confidence adds weight to movement\n            break;\n        case EEmotionalState::Aggressive:\n            Modifier *= 1.2f; // Aggression adds deliberate weight\n            break;\n    }\n    \n    return FMath::Clamp(Modifier, 0.5f, 2.0f);\n}\n\nfloat FAnimNode_TranspersonalMotionMatching::GetEmotionalTensionModifier() const\n{\n    float Tension = 0.0f;\n    \n    // Base tension from nervousness\n    Tension += CachedPersonality.NervousnessLevel * 0.5f;\n    \n    // Alertness adds tension\n    Tension += AlertnessLevel * 0.3f;\n    \n    // Threat detection maxes out tension\n    if (bThreatDetected)\n    {\n        Tension = 1.0f;\n    }\n    \n    return FMath::Clamp(Tension, 0.0f, 1.0f);\n}\n\nFVector FAnimNode_TranspersonalMotionMatching::GetPosturalAdjustment() const\n{\n    FVector Adjustment = FVector::ZeroVector;\n    \n    // Nervous posture: slight forward lean, shoulders up\n    if (CachedPersonality.NervousnessLevel > 0.1f)\n    {\n        Adjustment.X -= CachedPersonality.NervousnessLevel * 2.0f; // Forward lean\n        Adjustment.Z += CachedPersonality.NervousnessLevel * 1.0f; // Shoulders up\n    }\n    \n    // Confident posture: chest out, shoulders back\n    if (CachedPersonality.ConfidenceLevel > 0.7f)\n    {\n        Adjustment.X += CachedPersonality.ConfidenceLevel * 1.5f; // Chest out\n    }\n    \n    // Threat response: defensive crouch\n    if (bThreatDetected)\n    {\n        Adjustment.Z -= 5.0f; // Lower center of gravity\n    }\n    \n    return Adjustment;\n}