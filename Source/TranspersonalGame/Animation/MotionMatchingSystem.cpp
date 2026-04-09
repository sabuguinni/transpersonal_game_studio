// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "MotionMatchingSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogMotionMatching);

UMotionMatchingSystem::UMotionMatchingSystem()
{
    // Initialize default values
    ActiveDatabase = nullptr;
    CurrentBlendWeight = 0.0f;
    SelectedPoseIndex = -1;
    
    QueryRadius = 100.0f;
    TrajectoryWeight = 1.0f;
    PoseWeight = 1.0f;
    VelocityWeight = 1.0f;
    
    UpdateFrequency = 30.0f; // 30 Hz for performance
    MaxCandidates = 50;
    bEnableDebugDraw = false;
    
    TimeSinceLastUpdate = 0.0f;
    LastVelocity = FVector::ZeroVector;
    LastAcceleration = FVector::ZeroVector;
    LastSpeed = 0.0f;
    
    FramesSinceLastQuery = 0;
    bNeedsUpdate = true;
    
    MaxTrajectoryHistorySize = 10;
    TrajectoryHistory.Reserve(MaxTrajectoryHistorySize);
}

void UMotionMatchingSystem::UpdateMotionMatching(ACharacter* Character, float DeltaTime)
{
    if (!Character || !Character->GetCharacterMovement())
    {
        UE_LOG(LogMotionMatching, Warning, TEXT("Invalid character or movement component"));
        return;
    }

    // Update timing
    TimeSinceLastUpdate += DeltaTime;
    FramesSinceLastQuery++;
    
    // Check if we need to update (performance optimization)
    float UpdateInterval = 1.0f / UpdateFrequency;
    if (TimeSinceLastUpdate < UpdateInterval)
    {
        return;
    }
    
    TimeSinceLastUpdate = 0.0f;
    
    // Get current movement state
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    FVector CurrentVelocity = MovementComp->Velocity;
    float CurrentSpeed = CurrentVelocity.Size2D();
    bool bIsGrounded = !MovementComp->IsFalling();
    bool bIsInAir = MovementComp->IsFalling();
    
    // Calculate acceleration
    FVector CurrentAcceleration = (CurrentVelocity - LastVelocity) / DeltaTime;
    
    // Update trajectory history
    UpdateTrajectoryHistory(CurrentVelocity);
    
    // Apply movement profile
    FVector ModifiedVelocity = CurrentVelocity;
    FRotator CurrentRotation = Character->GetActorRotation();
    ApplyMovementProfile(ModifiedVelocity, CurrentRotation, DeltaTime);
    
    // Select best database
    UPoseSearchDatabase* BestDatabase = SelectBestDatabase(CurrentSpeed, bIsGrounded, bIsInAir);
    
    if (BestDatabase != ActiveDatabase)
    {
        // Database changed, need to blend
        ActiveDatabase = BestDatabase;
        CurrentBlendWeight = 0.0f;
        UE_LOG(LogMotionMatching, Log, TEXT("Switched to new database: %s"), 
               BestDatabase ? *BestDatabase->GetName() : TEXT("None"));
    }
    
    // Query best pose from active database
    if (ActiveDatabase)
    {
        SelectedPoseIndex = QueryBestPose(ActiveDatabase, ModifiedVelocity, CurrentAcceleration);
    }
    
    // Update blend weight
    if (ActiveDatabase)
    {
        float BlendSpeed = 5.0f; // Adjust based on database config
        CurrentBlendWeight = FMath::FInterpTo(CurrentBlendWeight, 1.0f, DeltaTime, BlendSpeed);
    }
    
    // Store values for next frame
    LastVelocity = CurrentVelocity;
    LastAcceleration = CurrentAcceleration;
    LastSpeed = CurrentSpeed;
    
    // Update fatigue based on movement intensity
    float MovementIntensity = CurrentSpeed / 600.0f; // Normalize to 0-1
    UpdateFatigue(MovementIntensity, DeltaTime);
    
    // Debug drawing
    if (bEnableDebugDraw && Character->GetWorld())
    {
        DrawDebugInfo(Character->GetWorld(), Character->GetActorLocation());
    }
    
    UE_LOG(LogMotionMatching, VeryVerbose, TEXT("Motion Matching Update - Speed: %f, Pose: %d, Blend: %f"), 
           CurrentSpeed, SelectedPoseIndex, CurrentBlendWeight);
}

UPoseSearchDatabase* UMotionMatchingSystem::SelectBestDatabase(float Speed, bool bIsGrounded, bool bIsInAir)
{
    UPoseSearchDatabase* BestDatabase = nullptr;
    float BestScore = -1.0f;
    
    for (const FMotionMatchingDatabaseConfig& Config : DatabaseConfigs)
    {
        if (!Config.Database)
        {
            continue;
        }
        
        float Score = CalculateDatabaseScore(Config, Speed, bIsGrounded, bIsInAir);
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestDatabase = Config.Database;
        }
    }
    
    return BestDatabase;
}

float UMotionMatchingSystem::CalculateDatabaseScore(const FMotionMatchingDatabaseConfig& Config, float Speed, bool bIsGrounded, bool bIsInAir)
{
    // Check basic requirements
    if (Config.bRequireGrounded && !bIsGrounded)
    {
        return -1.0f;
    }
    
    if (Config.bRequireInAir && !bIsInAir)
    {
        return -1.0f;
    }
    
    // Check speed range
    if (Speed < Config.MinSpeed || Speed > Config.MaxSpeed)
    {
        return -1.0f;
    }
    
    // Calculate score based on how well speed fits the range
    float SpeedRange = Config.MaxSpeed - Config.MinSpeed;
    float SpeedFit = 1.0f;
    
    if (SpeedRange > 0.0f)
    {
        float SpeedNormalized = (Speed - Config.MinSpeed) / SpeedRange;
        SpeedFit = 1.0f - FMath::Abs(SpeedNormalized - 0.5f) * 2.0f; // Best fit at middle of range
    }
    
    // Apply priority
    float FinalScore = SpeedFit * Config.Priority;
    
    return FinalScore;
}

int32 UMotionMatchingSystem::QueryBestPose(UPoseSearchDatabase* Database, const FVector& Velocity, const FVector& Acceleration)
{
    if (!Database)
    {
        return -1;
    }
    
    // This is a simplified version - real implementation would use UE5's Pose Search system
    // For now, we'll simulate pose selection based on velocity and movement profile
    
    int32 NumPoses = 100; // Simulated number of poses in database
    int32 BestPoseIndex = 0;
    float BestScore = -1.0f;
    
    for (int32 i = 0; i < FMath::Min(NumPoses, MaxCandidates); i++)
    {
        // Simulate pose matching score
        float VelocityScore = FMath::RandRange(0.0f, 1.0f);
        float TrajectoryScore = FMath::RandRange(0.0f, 1.0f);
        float PoseScore = FMath::RandRange(0.0f, 1.0f);
        
        // Weight the scores
        float TotalScore = (VelocityScore * VelocityWeight + 
                           TrajectoryScore * TrajectoryWeight + 
                           PoseScore * PoseWeight) / 3.0f;
        
        // Apply movement profile influence
        TotalScore *= (1.0f + MovementProfile.Confidence * 0.1f);
        TotalScore *= (1.0f - MovementProfile.Nervousness * 0.1f);
        TotalScore *= (1.0f - MovementProfile.Fatigue * 0.2f);
        
        if (TotalScore > BestScore)
        {
            BestScore = TotalScore;
            BestPoseIndex = i;
        }
    }
    
    return BestPoseIndex;
}

void UMotionMatchingSystem::ApplyMovementProfile(FVector& Velocity, FRotator& Rotation, float DeltaTime)
{
    // Apply personality traits to movement
    ApplyPersonalityToMovement(Velocity, DeltaTime);
    
    // Apply physical traits
    ApplyPhysicalTraitsToMovement(Velocity, DeltaTime);
    
    // Apply stride and step frequency modifications
    float StrideModifier = MovementProfile.StrideLength;
    float FrequencyModifier = MovementProfile.StepFrequency;
    
    // Modify velocity based on stride characteristics
    if (Velocity.Size2D() > 0.1f)
    {
        FVector Direction = Velocity.GetSafeNormal2D();
        float Speed = Velocity.Size2D();
        
        // Apply stride length (affects speed for same step frequency)
        Speed *= StrideModifier;
        
        // Apply step frequency (affects how often steps occur)
        // This would be more complex in a real implementation
        Speed *= FrequencyModifier;
        
        Velocity = Direction * Speed;
        Velocity.Z = Velocity.Z; // Preserve vertical component
    }
}

void UMotionMatchingSystem::UpdateFatigue(float MovementIntensity, float DeltaTime)
{
    // Increase fatigue based on movement intensity and fitness
    float FatigueRate = MovementIntensity * (2.0f - MovementProfile.Fitness) * 0.01f;
    MovementProfile.Fatigue = FMath::Clamp(MovementProfile.Fatigue + FatigueRate * DeltaTime, 0.0f, 1.0f);
    
    // Natural recovery when not moving intensely
    if (MovementIntensity < 0.3f)
    {
        float RecoveryRate = MovementProfile.Fitness * 0.05f;
        MovementProfile.Fatigue = FMath::Clamp(MovementProfile.Fatigue - RecoveryRate * DeltaTime, 0.0f, 1.0f);
    }
    
    UE_LOG(LogMotionMatching, VeryVerbose, TEXT("Fatigue level: %f"), MovementProfile.Fatigue);
}

void UMotionMatchingSystem::ApplyPersonalityToMovement(FVector& MovementVector, float DeltaTime)
{
    // Confidence affects stride and posture
    float ConfidenceModifier = FMath::Lerp(0.8f, 1.2f, MovementProfile.Confidence);
    MovementVector *= ConfidenceModifier;
    
    // Nervousness adds jitter
    if (MovementProfile.Nervousness > 0.1f)
    {
        FVector Jitter = ApplyPersonalityNoise(MovementVector, DeltaTime);
        MovementVector += Jitter * MovementProfile.Nervousness;
    }
    
    // Determination affects forward momentum
    if (MovementVector.Size2D() > 0.1f)
    {
        FVector Forward = MovementVector.GetSafeNormal2D();
        float DeterminationBoost = MovementProfile.Determination * 0.1f;
        MovementVector += Forward * DeterminationBoost * MovementVector.Size2D();
    }
    
    // Caution reduces speed and adds hesitation
    float CautionModifier = FMath::Lerp(1.0f, 0.7f, MovementProfile.Caution);
    MovementVector *= CautionModifier;
}

void UMotionMatchingSystem::ApplyPhysicalTraitsToMovement(FVector& MovementVector, float DeltaTime)
{
    // Age affects movement speed and stability
    float AgeModifier = FMath::Lerp(1.0f, 0.6f, MovementProfile.Age);
    MovementVector *= AgeModifier;
    
    // Fitness affects endurance and efficiency
    float FitnessModifier = FMath::Lerp(0.7f, 1.1f, MovementProfile.Fitness);
    MovementVector *= FitnessModifier;
    
    // Fatigue reduces everything
    float FatigueModifier = 1.0f - (MovementProfile.Fatigue * 0.4f);
    MovementVector *= FatigueModifier;
    
    // Injury creates asymmetric movement
    if (MovementProfile.Injury > 0.1f)
    {
        float LimpFactor = FMath::Sin(GetWorld()->GetTimeSeconds() * 3.0f) * MovementProfile.Injury;
        MovementVector.Y += LimpFactor * 20.0f; // Side-to-side compensation
        
        // Reduce overall efficiency
        MovementVector *= (1.0f - MovementProfile.Injury * 0.3f);
    }
}

FVector UMotionMatchingSystem::ApplyPersonalityNoise(const FVector& BaseMovement, float DeltaTime)
{
    // Generate subtle noise based on personality traits
    float NoiseScale = BaseMovement.Size2D() * 0.05f;
    
    FVector Noise;
    Noise.X = FMath::Sin(GetWorld()->GetTimeSeconds() * 5.0f) * NoiseScale;
    Noise.Y = FMath::Cos(GetWorld()->GetTimeSeconds() * 7.0f) * NoiseScale;
    Noise.Z = 0.0f;
    
    return Noise;
}

TArray<FVector> UMotionMatchingSystem::PredictTrajectory(const FVector& CurrentVelocity, const FVector& InputVector, float PredictionTime)
{
    TArray<FVector> Trajectory;
    
    int32 NumSteps = 10;
    float StepTime = PredictionTime / NumSteps;
    
    FVector Position = FVector::ZeroVector;
    FVector Velocity = CurrentVelocity;
    
    for (int32 i = 0; i < NumSteps; i++)
    {
        // Simple trajectory prediction - would be more complex in practice
        FVector DesiredVelocity = CalculateDesiredVelocity(InputVector, 600.0f);
        Velocity = FMath::VInterpTo(Velocity, DesiredVelocity, StepTime, 5.0f);
        
        Position += Velocity * StepTime;
        Trajectory.Add(Position);
    }
    
    return Trajectory;
}

FVector UMotionMatchingSystem::CalculateDesiredVelocity(const FVector& InputVector, float MaxSpeed)
{
    FVector DesiredVelocity = InputVector.GetSafeNormal2D() * MaxSpeed;
    
    // Apply movement profile modifications
    DesiredVelocity *= MovementProfile.StrideLength;
    DesiredVelocity *= (1.0f - MovementProfile.Fatigue * 0.3f);
    
    return DesiredVelocity;
}

void UMotionMatchingSystem::UpdateTrajectoryHistory(const FVector& CurrentVelocity)
{
    TrajectoryHistory.Add(CurrentVelocity);
    
    if (TrajectoryHistory.Num() > MaxTrajectoryHistorySize)
    {
        TrajectoryHistory.RemoveAt(0);
    }
}

void UMotionMatchingSystem::AddDatabaseConfig(UPoseSearchDatabase* Database, float MinSpeed, float MaxSpeed, float BlendTime)
{
    if (!Database)
    {
        return;
    }
    
    FMotionMatchingDatabaseConfig NewConfig;
    NewConfig.Database = Database;
    NewConfig.MinSpeed = MinSpeed;
    NewConfig.MaxSpeed = MaxSpeed;
    NewConfig.BlendTime = BlendTime;
    
    DatabaseConfigs.Add(NewConfig);
    SortDatabasesByPriority();
    
    UE_LOG(LogMotionMatching, Log, TEXT("Added database config: %s (Speed: %f-%f)"), 
           *Database->GetName(), MinSpeed, MaxSpeed);
}

void UMotionMatchingSystem::RemoveDatabaseConfig(UPoseSearchDatabase* Database)
{
    DatabaseConfigs.RemoveAll([Database](const FMotionMatchingDatabaseConfig& Config)
    {
        return Config.Database == Database;
    });
}

void UMotionMatchingSystem::SortDatabasesByPriority()
{
    DatabaseConfigs.Sort([](const FMotionMatchingDatabaseConfig& A, const FMotionMatchingDatabaseConfig& B)
    {
        return A.Priority > B.Priority;
    });
}

void UMotionMatchingSystem::DrawDebugInfo(UWorld* World, const FVector& Location)
{
    if (!World)
    {
        return;
    }
    
    FVector DebugLocation = Location + FVector(0, 0, 200);
    
    // Draw current state
    FString StateText = FString::Printf(TEXT("MM State\nSpeed: %.1f\nPose: %d\nBlend: %.2f\nFatigue: %.2f"), 
                                       LastSpeed, SelectedPoseIndex, CurrentBlendWeight, MovementProfile.Fatigue);
    
    DrawDebugString(World, DebugLocation, StateText, nullptr, FColor::Yellow, 0.0f);
    
    // Draw trajectory history
    for (int32 i = 1; i < TrajectoryHistory.Num(); i++)
    {
        FVector Start = Location + TrajectoryHistory[i-1];
        FVector End = Location + TrajectoryHistory[i];
        DrawDebugLine(World, Start, End, FColor::Blue, false, 0.0f, 0, 2.0f);
    }
    
    // Draw active database indicator
    if (ActiveDatabase)
    {
        DrawDebugSphere(World, DebugLocation + FVector(100, 0, 0), 10.0f, 8, FColor::Green);
    }
}

void UMotionMatchingSystem::LogMotionMatchingState()
{
    UE_LOG(LogMotionMatching, Log, TEXT("=== Motion Matching State ==="));
    UE_LOG(LogMotionMatching, Log, TEXT("Active Database: %s"), ActiveDatabase ? *ActiveDatabase->GetName() : TEXT("None"));
    UE_LOG(LogMotionMatching, Log, TEXT("Selected Pose: %d"), SelectedPoseIndex);
    UE_LOG(LogMotionMatching, Log, TEXT("Blend Weight: %f"), CurrentBlendWeight);
    UE_LOG(LogMotionMatching, Log, TEXT("Last Speed: %f"), LastSpeed);
    UE_LOG(LogMotionMatching, Log, TEXT("Fatigue: %f"), MovementProfile.Fatigue);
    UE_LOG(LogMotionMatching, Log, TEXT("Confidence: %f"), MovementProfile.Confidence);
}

FString UMotionMatchingSystem::GetCurrentStateDescription()
{
    return FString::Printf(TEXT("Database: %s, Pose: %d, Speed: %.1f, Fatigue: %.2f"), 
                          ActiveDatabase ? *ActiveDatabase->GetName() : TEXT("None"),
                          SelectedPoseIndex, LastSpeed, MovementProfile.Fatigue);
}

// UProtagonistMotionMatching Implementation

UProtagonistMotionMatching::UProtagonistMotionMatching()
{
    bIsObservingDinosaur = false;
    StressLevel = 0.2f; // Baseline stress in dangerous world
    CuriosityLevel = 0.8f; // High curiosity for scientist
    bIsHoldingTool = false;
    bIsCrafting = false;
    
    ObservationDatabase = nullptr;
    CraftingDatabase = nullptr;
    ToolUseDatabase = nullptr;
    
    StressMovementJitter = 0.1f;
    StressBreathingRate = 1.0f;
    StressFidgeting = 0.3f;
    
    StressAccumulation = 0.0f;
    LastThreatLevel = 0.0f;
    TimeSinceLastThreat = 0.0f;
    
    ObservationTimer = 0.0f;
    CraftingTimer = 0.0f;
    bInScientificMode = false;
}

void UProtagonistMotionMatching::UpdateMotionMatching(ACharacter* Character, float DeltaTime)
{
    // Update stress level first
    UpdateStressLevel(LastThreatLevel, DeltaTime);
    
    // Update scientific behavior
    UpdateScientificBehavior(DeltaTime);
    
    // Call parent implementation
    Super::UpdateMotionMatching(Character, DeltaTime);
    
    // Apply protagonist-specific movement modifications
    if (Character && Character->GetCharacterMovement())
    {
        FVector CurrentVelocity = Character->GetCharacterMovement()->Velocity;
        ApplyStressToMovement(CurrentVelocity, DeltaTime);
    }
}

void UProtagonistMotionMatching::UpdateStressLevel(float ThreatLevel, float DeltaTime)
{
    LastThreatLevel = ThreatLevel;
    
    if (ThreatLevel > 0.1f)
    {
        // Increase stress when threat is present
        float StressIncrease = ThreatLevel * DeltaTime * 2.0f;
        StressLevel = FMath::Clamp(StressLevel + StressIncrease, 0.0f, 1.0f);
        TimeSinceLastThreat = 0.0f;
    }
    else
    {
        // Gradual stress reduction when safe
        TimeSinceLastThreat += DeltaTime;
        if (TimeSinceLastThreat > 5.0f) // 5 seconds of safety before stress reduction
        {
            float StressDecrease = DeltaTime * 0.5f;
            StressLevel = FMath::Clamp(StressLevel - StressDecrease, 0.1f, 1.0f); // Minimum baseline stress
        }
    }
    
    // Stress affects movement profile
    MovementProfile.Nervousness = FMath::Lerp(0.1f, 0.8f, StressLevel);
    MovementProfile.Caution = FMath::Lerp(0.3f, 0.9f, StressLevel);
    
    UE_LOG(LogMotionMatching, VeryVerbose, TEXT("Protagonist stress level: %f"), StressLevel);
}

void UProtagonistMotionMatching::ApplyStressToMovement(FVector& MovementVector, float DeltaTime)
{
    if (StressLevel <= 0.2f)
    {
        return;
    }
    
    // Add stress-induced movement jitter
    float JitterIntensity = (StressLevel - 0.2f) * StressMovementJitter;
    
    FVector Jitter;
    Jitter.X = FMath::Sin(GetWorld()->GetTimeSeconds() * 8.0f) * JitterIntensity * 10.0f;
    Jitter.Y = FMath::Cos(GetWorld()->GetTimeSeconds() * 12.0f) * JitterIntensity * 10.0f;
    Jitter.Z = 0.0f;
    
    MovementVector += Jitter;
    
    // Stress affects movement speed (fight or flight)
    if (StressLevel > 0.6f)
    {
        // High stress increases movement speed (urgency)
        MovementVector *= (1.0f + (StressLevel - 0.6f) * 0.5f);
    }
    else if (StressLevel > 0.4f)
    {
        // Medium stress reduces movement speed (caution)
        MovementVector *= (1.0f - (StressLevel - 0.4f) * 0.3f);
    }
}

void UProtagonistMotionMatching::UpdateScientificBehavior(float DeltaTime)
{
    // Update timers
    ObservationTimer += DeltaTime;
    CraftingTimer += DeltaTime;
    
    // Determine if in scientific mode
    bInScientificMode = bIsObservingDinosaur || bIsCrafting || bIsHoldingTool;
    
    // Scientific behavior affects movement profile
    if (bInScientificMode)
    {
        // More deliberate, careful movement when in scientific mode
        MovementProfile.Caution = FMath::Max(MovementProfile.Caution, 0.7f);
        MovementProfile.Determination = FMath::Max(MovementProfile.Determination, 0.8f);
        
        // Curiosity affects head movement frequency (would be handled in animation)
        if (bIsObservingDinosaur)
        {
            CuriosityLevel = FMath::FInterpTo(CuriosityLevel, 1.0f, DeltaTime, 2.0f);
        }
    }
    else
    {
        // Return to baseline curiosity
        CuriosityLevel = FMath::FInterpTo(CuriosityLevel, 0.8f, DeltaTime, 1.0f);
    }
    
    UE_LOG(LogMotionMatching, VeryVerbose, TEXT("Scientific mode: %s, Curiosity: %f"), 
           bInScientificMode ? TEXT("True") : TEXT("False"), CuriosityLevel);
}

UPoseSearchDatabase* UProtagonistMotionMatching::SelectScientificDatabase()
{
    // Select appropriate database based on scientific activity
    if (bIsCrafting && CraftingDatabase)
    {
        return CraftingDatabase;
    }
    else if (bIsHoldingTool && ToolUseDatabase)
    {
        return ToolUseDatabase;
    }
    else if (bIsObservingDinosaur && ObservationDatabase)
    {
        return ObservationDatabase;
    }
    
    // Fall back to regular database selection
    return SelectBestDatabase(LastSpeed, true, false);
}