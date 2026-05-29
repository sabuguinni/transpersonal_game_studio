#include "Anim_MotionMatchingCore.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingCore::UAnim_MotionMatchingCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Default settings
    bMotionMatchingEnabled = true;
    SearchFrequency = 30.0f;
    MinQualityThreshold = 0.5f;
    BlendTime = 0.2f;
    
    // Initialize search parameters
    CurrentSearchParams.VelocityWeight = 0.7f;
    CurrentSearchParams.AccelerationWeight = 0.3f;
    
    PreviousVelocity = FVector::ZeroVector;
    PreviousFrameTime = 0.0f;
    LastSearchTime = 0.0f;
}

void UAnim_MotionMatchingCore::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMotionMatching();
    
    // Start motion search timer
    if (bMotionMatchingEnabled && SearchFrequency > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            SearchTimerHandle,
            this,
            &UAnim_MotionMatchingCore::OnSearchTimer,
            1.0f / SearchFrequency,
            true
        );
    }
}

void UAnim_MotionMatchingCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bMotionMatchingEnabled || !OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update search parameters based on current character state
    UpdateSearchParameters();
    
    // Track time for acceleration calculation
    PreviousFrameTime = GetWorld()->GetTimeSeconds();
}

void UAnim_MotionMatchingCore::InitializeMotionMatching()
{
    // Get owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingCore: Owner is not a Character"));
        return;
    }
    
    // Get movement component
    MovementComponent = OwnerCharacter->GetCharacterMovement();
    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingCore: Character has no movement component"));
        return;
    }
    
    // Build default pose database if empty
    if (PoseDatabase.Num() == 0)
    {
        BuildDefaultPoseDatabase();
    }
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingCore: Initialized with %d poses"), PoseDatabase.Num());
}

void UAnim_MotionMatchingCore::AddPoseToDatabase(const FAnim_MotionPoseData& PoseData)
{
    // Validate pose data
    if (PoseData.QualityScore < 0.0f || PoseData.QualityScore > 1.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingCore: Invalid quality score for pose %d"), PoseData.PoseID);
        return;
    }
    
    // Check for duplicate pose IDs
    for (const FAnim_MotionPoseData& ExistingPose : PoseDatabase)
    {
        if (ExistingPose.PoseID == PoseData.PoseID)
        {
            UE_LOG(LogTemp, Warning, TEXT("MotionMatchingCore: Pose ID %d already exists"), PoseData.PoseID);
            return;
        }
    }
    
    PoseDatabase.Add(PoseData);
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingCore: Added pose %d to database"), PoseData.PoseID);
}

void UAnim_MotionMatchingCore::ClearPoseDatabase()
{
    PoseDatabase.Empty();
    CurrentPose = FAnim_MotionPoseData();
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingCore: Cleared pose database"));
}

FAnim_MotionPoseData UAnim_MotionMatchingCore::FindBestMatchingPose(const FAnim_MotionSearchParams& SearchParams)
{
    if (PoseDatabase.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingCore: No poses in database"));
        return FAnim_MotionPoseData();
    }
    
    FAnim_MotionPoseData BestPose;
    float BestScore = -1.0f;
    
    for (const FAnim_MotionPoseData& Pose : PoseDatabase)
    {
        // Check quality threshold
        if (Pose.QualityScore < MinQualityThreshold)
        {
            continue;
        }
        
        // Check required tags
        if (!PoseHasRequiredTags(Pose, SearchParams.RequiredTags))
        {
            continue;
        }
        
        // Check excluded tags
        if (PoseHasExcludedTags(Pose, SearchParams.ExcludedTags))
        {
            continue;
        }
        
        // Calculate match score
        float Score = CalculatePoseMatchScore(Pose, SearchParams);
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestPose = Pose;
        }
    }
    
    if (BestScore < 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingCore: No suitable pose found"));
        return FAnim_MotionPoseData();
    }
    
    return BestPose;
}

void UAnim_MotionMatchingCore::UpdateSearchParameters()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Get current velocity
    FVector CurrentVelocity = GetCharacterVelocity();
    CurrentSearchParams.DesiredVelocity = CurrentVelocity;
    
    // Calculate acceleration
    FVector CurrentAcceleration = GetCharacterAcceleration();
    CurrentSearchParams.DesiredAcceleration = CurrentAcceleration;
    
    // Update tags based on character state
    CurrentSearchParams.RequiredTags.Empty();
    CurrentSearchParams.ExcludedTags.Empty();
    
    // Add movement state tags
    if (MovementComponent->IsFalling())
    {
        CurrentSearchParams.RequiredTags.Add("Falling");
        CurrentSearchParams.ExcludedTags.Add("Grounded");
    }
    else
    {
        CurrentSearchParams.RequiredTags.Add("Grounded");
        CurrentSearchParams.ExcludedTags.Add("Falling");
    }
    
    // Add speed-based tags
    float Speed = CurrentVelocity.Size();
    if (Speed < 50.0f)
    {
        CurrentSearchParams.RequiredTags.Add("Idle");
    }
    else if (Speed < 200.0f)
    {
        CurrentSearchParams.RequiredTags.Add("Walk");
    }
    else
    {
        CurrentSearchParams.RequiredTags.Add("Run");
    }
    
    // Store previous velocity for next frame
    PreviousVelocity = CurrentVelocity;
}

void UAnim_MotionMatchingCore::PerformMotionSearch()
{
    if (!bMotionMatchingEnabled)
    {
        return;
    }
    
    // Find best matching pose
    FAnim_MotionPoseData NewPose = FindBestMatchingPose(CurrentSearchParams);
    
    // Update current pose if we found a better match
    if (NewPose.PoseID != 0 && NewPose.PoseID != CurrentPose.PoseID)
    {
        CurrentPose = NewPose;
        LastSearchTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("MotionMatchingCore: Selected pose %d"), CurrentPose.PoseID);
    }
}

float UAnim_MotionMatchingCore::CalculatePoseMatchScore(const FAnim_MotionPoseData& PoseData, const FAnim_MotionSearchParams& SearchParams)
{
    // Calculate velocity difference (0-1, lower is better)
    float VelocityDiff = GetVelocityDifference(PoseData.Velocity, SearchParams.DesiredVelocity);
    
    // Calculate acceleration difference (0-1, lower is better)
    float AccelerationDiff = GetAccelerationDifference(PoseData.Acceleration, SearchParams.DesiredAcceleration);
    
    // Weighted score (higher is better)
    float Score = PoseData.QualityScore * (
        (1.0f - VelocityDiff) * SearchParams.VelocityWeight +
        (1.0f - AccelerationDiff) * SearchParams.AccelerationWeight
    );
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

FVector UAnim_MotionMatchingCore::GetCharacterVelocity() const
{
    if (!MovementComponent)
    {
        return FVector::ZeroVector;
    }
    
    return MovementComponent->Velocity;
}

FVector UAnim_MotionMatchingCore::GetCharacterAcceleration() const
{
    if (!MovementComponent)
    {
        return FVector::ZeroVector;
    }
    
    // Calculate acceleration from velocity change
    FVector CurrentVelocity = MovementComponent->Velocity;
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    
    if (DeltaTime > 0.0f)
    {
        return (CurrentVelocity - PreviousVelocity) / DeltaTime;
    }
    
    return FVector::ZeroVector;
}

bool UAnim_MotionMatchingCore::PoseHasRequiredTags(const FAnim_MotionPoseData& PoseData, const TArray<FString>& RequiredTags)
{
    for (const FString& RequiredTag : RequiredTags)
    {
        if (!PoseData.PoseTags.Contains(RequiredTag))
        {
            return false;
        }
    }
    return true;
}

bool UAnim_MotionMatchingCore::PoseHasExcludedTags(const FAnim_MotionPoseData& PoseData, const TArray<FString>& ExcludedTags)
{
    for (const FString& ExcludedTag : ExcludedTags)
    {
        if (PoseData.PoseTags.Contains(ExcludedTag))
        {
            return true;
        }
    }
    return false;
}

void UAnim_MotionMatchingCore::BuildDefaultPoseDatabase()
{
    // Clear existing database
    ClearPoseDatabase();
    
    // Create basic idle poses
    FAnim_MotionPoseData IdlePose;
    IdlePose.PoseID = 1;
    IdlePose.Velocity = FVector::ZeroVector;
    IdlePose.Acceleration = FVector::ZeroVector;
    IdlePose.RootMotionDirection = FVector::ForwardVector;
    IdlePose.QualityScore = 1.0f;
    IdlePose.PoseTags.Add("Idle");
    IdlePose.PoseTags.Add("Grounded");
    AddPoseToDatabase(IdlePose);
    
    // Create basic walk poses
    for (int32 i = 0; i < 8; ++i)
    {
        FAnim_MotionPoseData WalkPose;
        WalkPose.PoseID = 10 + i;
        
        // Create circular walk pattern
        float Angle = (float)i / 8.0f * 2.0f * PI;
        FVector WalkDirection = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
        
        WalkPose.Velocity = WalkDirection * 150.0f; // Walk speed
        WalkPose.Acceleration = FVector::ZeroVector;
        WalkPose.RootMotionDirection = WalkDirection;
        WalkPose.QualityScore = 0.9f;
        WalkPose.PoseTags.Add("Walk");
        WalkPose.PoseTags.Add("Grounded");
        AddPoseToDatabase(WalkPose);
    }
    
    // Create basic run poses
    for (int32 i = 0; i < 8; ++i)
    {
        FAnim_MotionPoseData RunPose;
        RunPose.PoseID = 20 + i;
        
        // Create circular run pattern
        float Angle = (float)i / 8.0f * 2.0f * PI;
        FVector RunDirection = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
        
        RunPose.Velocity = RunDirection * 400.0f; // Run speed
        RunPose.Acceleration = FVector::ZeroVector;
        RunPose.RootMotionDirection = RunDirection;
        RunPose.QualityScore = 0.8f;
        RunPose.PoseTags.Add("Run");
        RunPose.PoseTags.Add("Grounded");
        AddPoseToDatabase(RunPose);
    }
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingCore: Built default pose database with %d poses"), PoseDatabase.Num());
}

void UAnim_MotionMatchingCore::SetMotionMatchingEnabled(bool bEnabled)
{
    bMotionMatchingEnabled = bEnabled;
    
    if (bEnabled && SearchFrequency > 0.0f)
    {
        // Start timer
        GetWorld()->GetTimerManager().SetTimer(
            SearchTimerHandle,
            this,
            &UAnim_MotionMatchingCore::OnSearchTimer,
            1.0f / SearchFrequency,
            true
        );
    }
    else
    {
        // Stop timer
        GetWorld()->GetTimerManager().ClearTimer(SearchTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingCore: Motion matching %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UAnim_MotionMatchingCore::SetSearchFrequency(float NewFrequency)
{
    SearchFrequency = FMath::Clamp(NewFrequency, 1.0f, 60.0f);
    
    // Restart timer with new frequency
    if (bMotionMatchingEnabled)
    {
        SetMotionMatchingEnabled(false);
        SetMotionMatchingEnabled(true);
    }
}

void UAnim_MotionMatchingCore::OnSearchTimer()
{
    PerformMotionSearch();
}

bool UAnim_MotionMatchingCore::ValidatePoseDatabase() const
{
    if (PoseDatabase.Num() == 0)
    {
        return false;
    }
    
    // Check for duplicate IDs
    TSet<int32> UsedIDs;
    for (const FAnim_MotionPoseData& Pose : PoseDatabase)
    {
        if (UsedIDs.Contains(Pose.PoseID))
        {
            return false;
        }
        UsedIDs.Add(Pose.PoseID);
    }
    
    return true;
}

float UAnim_MotionMatchingCore::GetVelocityDifference(const FVector& A, const FVector& B) const
{
    float MaxSpeed = 1000.0f; // Maximum expected speed
    float Difference = FVector::Dist(A, B);
    return FMath::Clamp(Difference / MaxSpeed, 0.0f, 1.0f);
}

float UAnim_MotionMatchingCore::GetAccelerationDifference(const FVector& A, const FVector& B) const
{
    float MaxAcceleration = 2000.0f; // Maximum expected acceleration
    float Difference = FVector::Dist(A, B);
    return FMath::Clamp(Difference / MaxAcceleration, 0.0f, 1.0f);
}