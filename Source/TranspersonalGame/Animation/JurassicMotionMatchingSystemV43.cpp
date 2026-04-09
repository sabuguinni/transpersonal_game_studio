#include "JurassicMotionMatchingSystemV43.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"

UJurassicMotionMatchingSystemV43::UJurassicMotionMatchingSystemV43()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default values
    CurrentArchetype = EJurassicMovementArchetype::Paleontologist;
    CurrentEmotionalState = EJurassicEmotionalState::Neutral;
    EmotionalIntensity = 0.5f;
    CurrentTerrainType = EJurassicTerrainType::FlatGround;
    
    IKTraceDistance = 50.0f;
    IKInterpSpeed = 10.0f;
    EmotionalBlendSpeed = 2.0f;
    bEnableDebugDrawing = false;
    
    UE_LOG(LogTemp, Log, TEXT("JurassicMotionMatchingSystemV43: Component initialized"));
}

void UJurassicMotionMatchingSystemV43::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner character and components
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        
        UE_LOG(LogTemp, Log, TEXT("JurassicMotionMatchingSystemV43: Character components acquired"));
    }
    
    // Initialize archetype configurations
    InitializeArchetypeConfigurations();
    
    // Apply initial archetype settings
    ApplyArchetypeModifiers();
    
    UE_LOG(LogTemp, Log, TEXT("JurassicMotionMatchingSystemV43: BeginPlay completed"));
}

void UJurassicMotionMatchingSystemV43::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update core systems
    UpdateCharacterTrajectory(DeltaTime);
    UpdateMotionMatchingQuery(DeltaTime);
    UpdateEmotionalBlending(DeltaTime);
    UpdateFootIK(DeltaTime);
    
    // Debug visualization
    if (bEnableDebugDrawing)
    {
        DrawDebugTrajectory(true, true);
        DrawDebugFootIK();
    }
}

void UJurassicMotionMatchingSystemV43::InitializeMotionMatchingSystem(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicMotionMatchingSystemV43: Null character provided"));
        return;
    }
    
    OwnerCharacter = Character;
    MovementComponent = Character->GetCharacterMovement();
    SkeletalMeshComponent = Character->GetMesh();
    
    // Initialize trajectory system
    CharacterTrajectory.HistoryPositions.Empty();
    CharacterTrajectory.PredictedPositions.Empty();
    CharacterTrajectory.HistoryVelocities.Empty();
    CharacterTrajectory.PredictedVelocities.Empty();
    
    // Set initial database based on archetype
    SwitchToDatabaseByContext(TEXT("Locomotion"));
    
    UE_LOG(LogTemp, Log, TEXT("JurassicMotionMatchingSystemV43: Motion Matching system initialized for character: %s"), 
           *Character->GetName());
}

void UJurassicMotionMatchingSystemV43::SetCharacterArchetype(EJurassicMovementArchetype NewArchetype)
{
    if (CurrentArchetype == NewArchetype)
    {
        return;
    }
    
    EJurassicMovementArchetype PreviousArchetype = CurrentArchetype;
    CurrentArchetype = NewArchetype;
    
    // Apply archetype-specific modifications
    ApplyArchetypeModifiers();
    
    UE_LOG(LogTemp, Log, TEXT("JurassicMotionMatchingSystemV43: Archetype changed from %d to %d"), 
           (int32)PreviousArchetype, (int32)NewArchetype);
}

void UJurassicMotionMatchingSystemV43::SetEmotionalState(EJurassicEmotionalState NewState, float Intensity)
{
    CurrentEmotionalState = NewState;
    EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    // Emotional states can trigger database switches
    switch (NewState)
    {
        case EJurassicEmotionalState::Fearful:
        case EJurassicEmotionalState::Desperate:
            SwitchToDatabaseByContext(TEXT("Combat"));
            break;
            
        case EJurassicEmotionalState::Cautious:
            SwitchToDatabaseByContext(TEXT("Stealth"));
            break;
            
        case EJurassicEmotionalState::Curious:
            SwitchToDatabaseByContext(TEXT("Interaction"));
            break;
            
        default:
            SwitchToDatabaseByContext(TEXT("Locomotion"));
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("JurassicMotionMatchingSystemV43: Emotional state set to %d with intensity %.2f"), 
           (int32)NewState, Intensity);
}

void UJurassicMotionMatchingSystemV43::UpdateTerrainAdaptation(EJurassicTerrainType TerrainType)
{
    CurrentTerrainType = TerrainType;
    FootIKData.CurrentTerrainType = TerrainType;
    
    // Adjust movement parameters based on terrain
    if (MovementComponent)
    {
        float SpeedModifier = 1.0f;
        
        switch (TerrainType)
        {
            case EJurassicTerrainType::RockyUneven:
                SpeedModifier = 0.8f;
                break;
            case EJurassicTerrainType::SteepIncline:
                SpeedModifier = 0.6f;
                break;
            case EJurassicTerrainType::MuddySoft:
                SpeedModifier = 0.7f;
                break;
            case EJurassicTerrainType::WaterShallow:
                SpeedModifier = 0.5f;
                break;
            case EJurassicTerrainType::VegetationDense:
                SpeedModifier = 0.75f;
                break;
            default:
                SpeedModifier = 1.0f;
                break;
        }
        
        // Apply speed modification while preserving archetype characteristics
        FJurassicArchetypeAnimationData ArchetypeData = GetArchetypeData();
        MovementComponent->MaxWalkSpeed = ArchetypeData.BaseWalkSpeed * SpeedModifier;
    }
    
    UE_LOG(LogTemp, Log, TEXT("JurassicMotionMatchingSystemV43: Terrain adaptation updated to %d"), 
           (int32)TerrainType);
}

UPoseSearchDatabase* UJurassicMotionMatchingSystemV43::GetActiveDatabase() const
{
    return ActiveDatabase.Get();
}

void UJurassicMotionMatchingSystemV43::SetActiveDatabase(UPoseSearchDatabase* NewDatabase)
{
    ActiveDatabase = NewDatabase;
    
    if (NewDatabase)
    {
        UE_LOG(LogTemp, Log, TEXT("JurassicMotionMatchingSystemV43: Active database set to %s"), 
               *NewDatabase->GetName());
    }
}

void UJurassicMotionMatchingSystemV43::SwitchToDatabaseByContext(const FString& Context)
{
    UPoseSearchDatabase* TargetDatabase = nullptr;
    
    if (Context == TEXT("Locomotion"))
    {
        TargetDatabase = MotionMatchingConfig.LocomotionDatabase.LoadSynchronous();
    }
    else if (Context == TEXT("Stealth"))
    {
        TargetDatabase = MotionMatchingConfig.StealthDatabase.LoadSynchronous();
    }
    else if (Context == TEXT("Interaction"))
    {
        TargetDatabase = MotionMatchingConfig.InteractionDatabase.LoadSynchronous();
    }
    else if (Context == TEXT("Emotional"))
    {
        TargetDatabase = MotionMatchingConfig.EmotionalDatabase.LoadSynchronous();
    }
    else if (Context == TEXT("Combat"))
    {
        TargetDatabase = MotionMatchingConfig.CombatDatabase.LoadSynchronous();
    }
    
    if (TargetDatabase)
    {
        SetActiveDatabase(TargetDatabase);
        UE_LOG(LogTemp, Log, TEXT("JurassicMotionMatchingSystemV43: Switched to %s database"), *Context);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicMotionMatchingSystemV43: Failed to load %s database"), *Context);
    }
}

void UJurassicMotionMatchingSystemV43::UpdateCharacterTrajectory(float DeltaTime)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Sample current position and velocity
    FVector CurrentPosition = OwnerCharacter->GetActorLocation();
    FVector CurrentVelocity = MovementComponent ? MovementComponent->Velocity : FVector::ZeroVector;
    
    // Update history
    SampleTrajectoryHistory(DeltaTime);
    
    // Add current sample to history
    CharacterTrajectory.HistoryPositions.Add(CurrentPosition);
    CharacterTrajectory.HistoryVelocities.Add(CurrentVelocity);
    
    // Maintain history size
    int32 MaxHistorySamples = FMath::CeilToInt(CharacterTrajectory.HistoryTime * CharacterTrajectory.SampleRate);
    while (CharacterTrajectory.HistoryPositions.Num() > MaxHistorySamples)
    {
        CharacterTrajectory.HistoryPositions.RemoveAt(0);
        CharacterTrajectory.HistoryVelocities.RemoveAt(0);
    }
    
    // Update predictions
    PredictMovementTrajectory(CharacterTrajectory.PredictionTime);
}

void UJurassicMotionMatchingSystemV43::PredictMovementTrajectory(float PredictionTime)
{
    if (!MovementComponent)
    {
        return;
    }
    
    CharacterTrajectory.PredictedPositions.Empty();
    CharacterTrajectory.PredictedVelocities.Empty();
    
    FVector CurrentPosition = OwnerCharacter->GetActorLocation();
    FVector CurrentVelocity = MovementComponent->Velocity;
    
    int32 PredictionSamples = FMath::CeilToInt(PredictionTime * CharacterTrajectory.SampleRate);
    float TimeStep = PredictionTime / PredictionSamples;
    
    for (int32 i = 0; i < PredictionSamples; ++i)
    {
        float TimeAhead = (i + 1) * TimeStep;
        FVector PredictedPos = PredictFuturePosition(TimeAhead);
        FVector PredictedVel = CurrentVelocity; // Simplified prediction
        
        CharacterTrajectory.PredictedPositions.Add(PredictedPos);
        CharacterTrajectory.PredictedVelocities.Add(PredictedVel);
    }
}

void UJurassicMotionMatchingSystemV43::UpdateFootIK(float DeltaTime)
{
    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        return;
    }
    
    // Perform foot traces for both feet
    FVector LeftFootPos, RightFootPos;
    FRotator LeftFootRot, RightFootRot;
    float LeftAlpha, RightAlpha;
    
    PerformFootTrace(true, LeftFootPos, LeftFootRot, LeftAlpha);
    PerformFootTrace(false, RightFootPos, RightFootRot, RightAlpha);
    
    // Interpolate to smooth IK values
    FootIKData.LeftFootPosition = FMath::VInterpTo(FootIKData.LeftFootPosition, LeftFootPos, DeltaTime, IKInterpSpeed);
    FootIKData.RightFootPosition = FMath::VInterpTo(FootIKData.RightFootPosition, RightFootPos, DeltaTime, IKInterpSpeed);
    FootIKData.LeftFootRotation = FMath::RInterpTo(FootIKData.LeftFootRotation, LeftFootRot, DeltaTime, IKInterpSpeed);
    FootIKData.RightFootRotation = FMath::RInterpTo(FootIKData.RightFootRotation, RightFootRot, DeltaTime, IKInterpSpeed);
    
    FootIKData.LeftFootAlpha = FMath::FInterpTo(FootIKData.LeftFootAlpha, LeftAlpha, DeltaTime, IKInterpSpeed);
    FootIKData.RightFootAlpha = FMath::FInterpTo(FootIKData.RightFootAlpha, RightAlpha, DeltaTime, IKInterpSpeed);
    
    // Calculate hip offset to maintain balance
    float TargetHipOffset = FMath::Min(FootIKData.LeftFootPosition.Z, FootIKData.RightFootPosition.Z);
    FootIKData.HipOffset = FMath::FInterpTo(FootIKData.HipOffset, TargetHipOffset, DeltaTime, IKInterpSpeed);
}

void UJurassicMotionMatchingSystemV43::PerformFootTrace(bool bLeftFoot, FVector& OutFootPosition, FRotator& OutFootRotation, float& OutAlpha)
{
    if (!OwnerCharacter || !SkeletalMeshComponent)
    {
        OutFootPosition = FVector::ZeroVector;
        OutFootRotation = FRotator::ZeroRotator;
        OutAlpha = 0.0f;
        return;
    }
    
    // Get foot bone location
    FName FootBoneName = bLeftFoot ? TEXT("foot_l") : TEXT("foot_r");
    FVector FootLocation = SkeletalMeshComponent->GetBoneLocation(FootBoneName);
    
    // Trace downward from foot
    FVector TraceStart = FootLocation + FVector(0, 0, 20);
    FVector TraceEnd = FootLocation - FVector(0, 0, IKTraceDistance);
    
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic
    );
    
    if (bHit)
    {
        OutFootPosition = HitResult.Location;
        OutFootRotation = FRotationMatrix::MakeFromZ(HitResult.Normal).Rotator();
        OutAlpha = 1.0f;
        
        // Adjust based on terrain type
        switch (CurrentTerrainType)
        {
            case EJurassicTerrainType::MuddySoft:
                OutFootPosition.Z -= 2.0f; // Sink slightly into mud
                break;
            case EJurassicTerrainType::WaterShallow:
                OutFootPosition.Z -= 5.0f; // Account for water depth
                break;
            default:
                break;
        }
    }
    else
    {
        OutFootPosition = FootLocation;
        OutFootRotation = FRotator::ZeroRotator;
        OutAlpha = 0.0f;
    }
}

FJurassicArchetypeAnimationData UJurassicMotionMatchingSystemV43::GetArchetypeData() const
{
    const FJurassicArchetypeAnimationData* FoundData = ArchetypeConfigurations.Find(CurrentArchetype);
    if (FoundData)
    {
        return *FoundData;
    }
    
    // Return default configuration if not found
    return FJurassicArchetypeAnimationData();
}

void UJurassicMotionMatchingSystemV43::ApplyArchetypeModifiers()
{
    if (!MovementComponent)
    {
        return;
    }
    
    FJurassicArchetypeAnimationData ArchetypeData = GetArchetypeData();
    
    // Apply movement speed modifications
    MovementComponent->MaxWalkSpeed = ArchetypeData.BaseWalkSpeed;
    MovementComponent->MaxWalkSpeedCrouched = ArchetypeData.BaseWalkSpeed * ArchetypeData.StealthSpeedMultiplier;
    
    // Apply acceleration based on movement efficiency
    float BaseAcceleration = 2048.0f;
    MovementComponent->MaxAcceleration = BaseAcceleration * ArchetypeData.MovementEfficiency;
    
    UE_LOG(LogTemp, Log, TEXT("JurassicMotionMatchingSystemV43: Applied archetype modifiers - Walk: %.1f, Efficiency: %.2f"), 
           ArchetypeData.BaseWalkSpeed, ArchetypeData.MovementEfficiency);
}

void UJurassicMotionMatchingSystemV43::BlendEmotionalStates(EJurassicEmotionalState TargetState, float BlendRate)
{
    // Implement emotional state blending logic
    // This would typically involve interpolating between animation parameters
    
    UE_LOG(LogTemp, Log, TEXT("JurassicMotionMatchingSystemV43: Blending to emotional state %d at rate %.2f"), 
           (int32)TargetState, BlendRate);
}

void UJurassicMotionMatchingSystemV43::DrawDebugTrajectory(bool bShowHistory, bool bShowPrediction)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Draw history trajectory
    if (bShowHistory && CharacterTrajectory.HistoryPositions.Num() > 1)
    {
        for (int32 i = 1; i < CharacterTrajectory.HistoryPositions.Num(); ++i)
        {
            DrawDebugLine(
                GetWorld(),
                CharacterTrajectory.HistoryPositions[i-1],
                CharacterTrajectory.HistoryPositions[i],
                FColor::Blue,
                false,
                0.1f,
                0,
                2.0f
            );
        }
    }
    
    // Draw prediction trajectory
    if (bShowPrediction && CharacterTrajectory.PredictedPositions.Num() > 1)
    {
        FVector CurrentPos = OwnerCharacter ? OwnerCharacter->GetActorLocation() : FVector::ZeroVector;
        
        // Line from current position to first prediction
        if (CharacterTrajectory.PredictedPositions.Num() > 0)
        {
            DrawDebugLine(
                GetWorld(),
                CurrentPos,
                CharacterTrajectory.PredictedPositions[0],
                FColor::Green,
                false,
                0.1f,
                0,
                2.0f
            );
        }
        
        // Lines between prediction points
        for (int32 i = 1; i < CharacterTrajectory.PredictedPositions.Num(); ++i)
        {
            DrawDebugLine(
                GetWorld(),
                CharacterTrajectory.PredictedPositions[i-1],
                CharacterTrajectory.PredictedPositions[i],
                FColor::Green,
                false,
                0.1f,
                0,
                2.0f
            );
        }
    }
}

void UJurassicMotionMatchingSystemV43::DrawDebugFootIK()
{
    if (!GetWorld() || !OwnerCharacter)
    {
        return;
    }
    
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    
    // Draw foot IK positions
    DrawDebugSphere(
        GetWorld(),
        FootIKData.LeftFootPosition,
        5.0f,
        8,
        FColor::Red,
        false,
        0.1f
    );
    
    DrawDebugSphere(
        GetWorld(),
        FootIKData.RightFootPosition,
        5.0f,
        8,
        FColor::Orange,
        false,
        0.1f
    );
    
    // Draw hip offset
    FVector HipPosition = CharacterLocation + FVector(0, 0, FootIKData.HipOffset);
    DrawDebugSphere(
        GetWorld(),
        HipPosition,
        8.0f,
        8,
        FColor::Yellow,
        false,
        0.1f
    );
}

void UJurassicMotionMatchingSystemV43::LogAnimationState()
{
    UE_LOG(LogTemp, Log, TEXT("=== JurassicMotionMatchingSystemV43 State ==="));
    UE_LOG(LogTemp, Log, TEXT("Archetype: %d"), (int32)CurrentArchetype);
    UE_LOG(LogTemp, Log, TEXT("Emotional State: %d (Intensity: %.2f)"), (int32)CurrentEmotionalState, EmotionalIntensity);
    UE_LOG(LogTemp, Log, TEXT("Terrain Type: %d"), (int32)CurrentTerrainType);
    UE_LOG(LogTemp, Log, TEXT("Active Database: %s"), ActiveDatabase.IsValid() ? *ActiveDatabase->GetName() : TEXT("None"));
    UE_LOG(LogTemp, Log, TEXT("Trajectory History Samples: %d"), CharacterTrajectory.HistoryPositions.Num());
    UE_LOG(LogTemp, Log, TEXT("Trajectory Prediction Samples: %d"), CharacterTrajectory.PredictedPositions.Num());
    UE_LOG(LogTemp, Log, TEXT("Foot IK - Left Alpha: %.2f, Right Alpha: %.2f"), FootIKData.LeftFootAlpha, FootIKData.RightFootAlpha);
}

void UJurassicMotionMatchingSystemV43::InitializeArchetypeConfigurations()
{
    // Paleontologist - Scientific, careful, analytical
    FJurassicArchetypeAnimationData PaleontologistData;
    PaleontologistData.BaseWalkSpeed = 120.0f;
    PaleontologistData.BaseRunSpeed = 350.0f;
    PaleontologistData.StealthSpeedMultiplier = 0.6f;
    PaleontologistData.PanicSpeedMultiplier = 1.1f;
    PaleontologistData.PostureConfidence = 0.7f;
    PaleontologistData.GestureFrequency = 0.3f;
    PaleontologistData.AlertnessLevel = 0.8f;
    PaleontologistData.MovementEfficiency = 0.6f;
    PaleontologistData.FearSensitivity = 0.7f;
    PaleontologistData.CuriositySensitivity = 0.9f;
    PaleontologistData.AdaptabilityRate = 0.5f;
    ArchetypeConfigurations.Add(EJurassicMovementArchetype::Paleontologist, PaleontologistData);
    
    // Tribal Survivor - Natural, efficient, instinctive
    FJurassicArchetypeAnimationData TribalData;
    TribalData.BaseWalkSpeed = 160.0f;
    TribalData.BaseRunSpeed = 450.0f;
    TribalData.StealthSpeedMultiplier = 0.8f;
    TribalData.PanicSpeedMultiplier = 1.4f;
    TribalData.PostureConfidence = 0.8f;
    TribalData.GestureFrequency = 0.5f;
    TribalData.AlertnessLevel = 0.9f;
    TribalData.MovementEfficiency = 0.9f;
    TribalData.FearSensitivity = 0.4f;
    TribalData.CuriositySensitivity = 0.6f;
    TribalData.AdaptabilityRate = 0.8f;
    ArchetypeConfigurations.Add(EJurassicMovementArchetype::TribalSurvivor, TribalData);
    
    // Modern Lost - Uncertain, clumsy, overwhelmed
    FJurassicArchetypeAnimationData ModernData;
    ModernData.BaseWalkSpeed = 140.0f;
    ModernData.BaseRunSpeed = 380.0f;
    ModernData.StealthSpeedMultiplier = 0.4f;
    ModernData.PanicSpeedMultiplier = 1.2f;
    ModernData.PostureConfidence = 0.3f;
    ModernData.GestureFrequency = 0.7f;
    ModernData.AlertnessLevel = 0.6f;
    ModernData.MovementEfficiency = 0.4f;
    ModernData.FearSensitivity = 0.9f;
    ModernData.CuriositySensitivity = 0.5f;
    ModernData.AdaptabilityRate = 0.3f;
    ArchetypeConfigurations.Add(EJurassicMovementArchetype::ModernLost, ModernData);
    
    // Elder Wise - Slow, deliberate, experienced
    FJurassicArchetypeAnimationData ElderData;
    ElderData.BaseWalkSpeed = 100.0f;
    ElderData.BaseRunSpeed = 280.0f;
    ElderData.StealthSpeedMultiplier = 0.7f;
    ElderData.PanicSpeedMultiplier = 0.9f;
    ElderData.PostureConfidence = 0.9f;
    ElderData.GestureFrequency = 0.2f;
    ElderData.AlertnessLevel = 0.7f;
    ElderData.MovementEfficiency = 0.8f;
    ElderData.FearSensitivity = 0.3f;
    ElderData.CuriositySensitivity = 0.8f;
    ElderData.AdaptabilityRate = 0.4f;
    ArchetypeConfigurations.Add(EJurassicMovementArchetype::ElderWise, ElderData);
    
    // Young Adaptive - Quick, energetic, learning
    FJurassicArchetypeAnimationData YoungData;
    YoungData.BaseWalkSpeed = 180.0f;
    YoungData.BaseRunSpeed = 500.0f;
    YoungData.StealthSpeedMultiplier = 0.6f;
    YoungData.PanicSpeedMultiplier = 1.5f;
    YoungData.PostureConfidence = 0.6f;
    YoungData.GestureFrequency = 0.8f;
    YoungData.AlertnessLevel = 0.8f;
    YoungData.MovementEfficiency = 0.7f;
    YoungData.FearSensitivity = 0.6f;
    YoungData.CuriositySensitivity = 0.9f;
    YoungData.AdaptabilityRate = 0.9f;
    ArchetypeConfigurations.Add(EJurassicMovementArchetype::YoungAdaptive, YoungData);
    
    UE_LOG(LogTemp, Log, TEXT("JurassicMotionMatchingSystemV43: Archetype configurations initialized"));
}

void UJurassicMotionMatchingSystemV43::UpdateMotionMatchingQuery(float DeltaTime)
{
    // This would interface with the actual Motion Matching node in the Animation Blueprint
    // For now, we log the query state for debugging
    
    if (bEnableDebugDrawing)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Motion Matching Query - Archetype: %d, Emotion: %d, Terrain: %d"), 
               (int32)CurrentArchetype, (int32)CurrentEmotionalState, (int32)CurrentTerrainType);
    }
}

void UJurassicMotionMatchingSystemV43::UpdateEmotionalBlending(float DeltaTime)
{
    // Implement emotional state blending over time
    // This would typically modify animation parameters based on current emotional state
    
    if (EmotionalIntensity > 0.1f)
    {
        // Emotional intensity naturally decays over time unless reinforced
        EmotionalIntensity = FMath::Max(0.0f, EmotionalIntensity - (DeltaTime * 0.1f));
    }
}

void UJurassicMotionMatchingSystemV43::CalculateTerrainAdaptation()
{
    // Analyze current terrain and adjust movement parameters
    // This could involve raycasting or using terrain analysis systems
}

FVector UJurassicMotionMatchingSystemV43::PredictFuturePosition(float TimeAhead) const
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return FVector::ZeroVector;
    }
    
    FVector CurrentPosition = OwnerCharacter->GetActorLocation();
    FVector CurrentVelocity = MovementComponent->Velocity;
    
    // Simple linear prediction (could be enhanced with acceleration, steering, etc.)
    return CurrentPosition + (CurrentVelocity * TimeAhead);
}

void UJurassicMotionMatchingSystemV43::SampleTrajectoryHistory(float DeltaTime)
{
    // Sample trajectory at the specified rate
    static float TimeSinceLastSample = 0.0f;
    TimeSinceLastSample += DeltaTime;
    
    float SampleInterval = 1.0f / CharacterTrajectory.SampleRate;
    
    if (TimeSinceLastSample >= SampleInterval)
    {
        TimeSinceLastSample = 0.0f;
        // Sampling is handled in UpdateCharacterTrajectory
    }
}