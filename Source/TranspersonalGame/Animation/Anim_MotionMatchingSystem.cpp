#include "Anim_MotionMatchingSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.033f; // 30 FPS for motion matching
    
    bMotionMatchingEnabled = true;
    MotionMatchingUpdateRate = 30.0f;
    PoseMatchingThreshold = 0.8f;
    LastMotionUpdateTime = 0.0f;
    
    OwnerMeshComponent = nullptr;
    OwnerAnimInstance = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner character's mesh component
    if (AActor* Owner = GetOwner())
    {
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            OwnerMeshComponent = Character->GetMesh();
            if (OwnerMeshComponent)
            {
                OwnerAnimInstance = OwnerMeshComponent->GetAnimInstance();
                UE_LOG(LogTemp, Log, TEXT("Motion Matching System initialized for character: %s"), *Owner->GetName());
            }
        }
    }
    
    InitializeMotionDatabase();
    InitializeSurvivalMontages();
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bMotionMatchingEnabled || !OwnerMeshComponent || !OwnerAnimInstance)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastMotionUpdateTime >= (1.0f / MotionMatchingUpdateRate))
    {
        // Update motion data from character movement
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                FVector Velocity = MovementComp->Velocity;
                bool bInAir = MovementComp->IsFalling();
                bool bCrouching = MovementComp->IsCrouching();
                
                // Determine current survival action based on character state
                ESurvivalAction CurrentAction = ESurvivalAction::None;
                if (Velocity.Size() > 300.0f)
                {
                    CurrentAction = ESurvivalAction::Running;
                }
                else if (Velocity.Size() > 50.0f)
                {
                    CurrentAction = ESurvivalAction::Walking;
                }
                else if (bCrouching)
                {
                    CurrentAction = ESurvivalAction::Crouching;
                }
                
                UpdateMotionData(Velocity, bInAir, bCrouching, CurrentAction);
                
                // Find best matching pose
                FAnim_MotionMatchingPose BestPose = FindBestMatchingPose(CurrentMotionData);
                if (BestPose.MatchingScore > PoseMatchingThreshold)
                {
                    BlendToNewPose(BestPose);
                }
            }
        }
        
        LastMotionUpdateTime = CurrentTime;
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData(const FVector& NewVelocity, bool bInAir, bool bCrouching, ESurvivalAction Action)
{
    CurrentMotionData.Velocity = NewVelocity;
    CurrentMotionData.Speed = NewVelocity.Size();
    CurrentMotionData.bIsInAir = bInAir;
    CurrentMotionData.bIsCrouching = bCrouching;
    CurrentMotionData.CurrentAction = Action;
    
    // Calculate movement direction relative to character forward
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        FVector ForwardVector = Character->GetActorForwardVector();
        FVector RightVector = Character->GetActorRightVector();
        
        FVector NormalizedVelocity = NewVelocity.GetSafeNormal();
        float ForwardDot = FVector::DotProduct(NormalizedVelocity, ForwardVector);
        float RightDot = FVector::DotProduct(NormalizedVelocity, RightVector);
        
        CurrentMotionData.Direction = UKismetMathLibrary::Atan2(RightDot, ForwardDot) * (180.0f / PI);
    }
}

FAnim_MotionMatchingPose UAnim_MotionMatchingSystem::FindBestMatchingPose(const FAnim_MotionData& CurrentMotion)
{
    FAnim_MotionMatchingPose BestPose;
    float BestScore = 0.0f;
    
    for (const FAnim_MotionMatchingPose& Pose : PoseDatabase)
    {
        float Score = CalculatePoseMatchingScore(CurrentMotion, Pose.MotionData);
        if (Score > BestScore)
        {
            BestScore = Score;
            BestPose = Pose;
            BestPose.MatchingScore = Score;
        }
    }
    
    return BestPose;
}

float UAnim_MotionMatchingSystem::CalculatePoseMatchingScore(const FAnim_MotionData& Current, const FAnim_MotionData& Candidate)
{
    float Score = 1.0f;
    
    // Speed matching (weight: 0.4)
    float SpeedDiff = FMath::Abs(Current.Speed - Candidate.Speed);
    float SpeedScore = FMath::Max(0.0f, 1.0f - (SpeedDiff / 500.0f)); // Normalize by max expected speed
    Score *= (0.4f * SpeedScore + 0.6f);
    
    // Direction matching (weight: 0.3)
    float DirectionDiff = FMath::Abs(Current.Direction - Candidate.Direction);
    if (DirectionDiff > 180.0f) DirectionDiff = 360.0f - DirectionDiff; // Handle angle wrap
    float DirectionScore = FMath::Max(0.0f, 1.0f - (DirectionDiff / 180.0f));
    Score *= (0.3f * DirectionScore + 0.7f);
    
    // State matching (weight: 0.3)
    float StateScore = 1.0f;
    if (Current.bIsInAir != Candidate.bIsInAir) StateScore *= 0.1f;
    if (Current.bIsCrouching != Candidate.bIsCrouching) StateScore *= 0.5f;
    if (Current.CurrentAction != Candidate.CurrentAction) StateScore *= 0.3f;
    Score *= (0.3f * StateScore + 0.7f);
    
    return FMath::Clamp(Score, 0.0f, 1.0f);
}

void UAnim_MotionMatchingSystem::BlendToNewPose(const FAnim_MotionMatchingPose& NewPose, float BlendTime)
{
    if (!OwnerAnimInstance || !NewPose.AnimSequence)
    {
        return;
    }
    
    CurrentPose = NewPose;
    
    // Play the animation sequence at the specified time
    // Note: In a full implementation, this would use more sophisticated blending
    UE_LOG(LogTemp, Log, TEXT("Blending to new pose: %s at time %f"), 
           *NewPose.AnimSequence->GetName(), NewPose.TimeInAnimation);
}

void UAnim_MotionMatchingSystem::PlaySurvivalMontage(ESurvivalAction Action)
{
    if (!OwnerAnimInstance)
    {
        return;
    }
    
    UAnimMontage** FoundMontage = SurvivalMontages.Find(Action);
    if (FoundMontage && *FoundMontage)
    {
        OwnerAnimInstance->Montage_Play(*FoundMontage);
        UE_LOG(LogTemp, Log, TEXT("Playing survival montage for action: %d"), (int32)Action);
    }
}

void UAnim_MotionMatchingSystem::AddPoseToDatabase(UAnimSequence* Animation, float TimeStamp, const FAnim_MotionData& MotionData)
{
    if (!Animation)
    {
        return;
    }
    
    FAnim_MotionMatchingPose NewPose;
    NewPose.AnimSequence = Animation;
    NewPose.TimeInAnimation = TimeStamp;
    NewPose.MotionData = MotionData;
    NewPose.MatchingScore = 0.0f;
    
    PoseDatabase.Add(NewPose);
}

void UAnim_MotionMatchingSystem::InitializeMotionDatabase()
{
    // Initialize with basic locomotion data
    // In a full implementation, this would load from animation assets
    
    // Create sample idle poses
    FAnim_MotionData IdleMotion;
    IdleMotion.Speed = 0.0f;
    IdleMotion.CurrentAction = ESurvivalAction::None;
    
    // Create sample walk poses
    for (float Speed = 50.0f; Speed <= 200.0f; Speed += 25.0f)
    {
        for (float Direction = -180.0f; Direction <= 180.0f; Direction += 45.0f)
        {
            FAnim_MotionData WalkMotion;
            WalkMotion.Speed = Speed;
            WalkMotion.Direction = Direction;
            WalkMotion.CurrentAction = ESurvivalAction::Walking;
            
            // Add to database (would reference actual animation sequences)
            AddPoseToDatabase(nullptr, 0.0f, WalkMotion);
        }
    }
    
    // Create sample run poses
    for (float Speed = 300.0f; Speed <= 600.0f; Speed += 50.0f)
    {
        for (float Direction = -90.0f; Direction <= 90.0f; Direction += 30.0f)
        {
            FAnim_MotionData RunMotion;
            RunMotion.Speed = Speed;
            RunMotion.Direction = Direction;
            RunMotion.CurrentAction = ESurvivalAction::Running;
            
            AddPoseToDatabase(nullptr, 0.0f, RunMotion);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Motion database initialized with %d poses"), PoseDatabase.Num());
}

void UAnim_MotionMatchingSystem::InitializeSurvivalMontages()
{
    // Initialize survival action montages
    // In a full implementation, these would be loaded from content browser
    
    SurvivalMontages.Empty();
    
    // Add placeholder entries for all survival actions
    SurvivalMontages.Add(ESurvivalAction::Gathering, nullptr);
    SurvivalMontages.Add(ESurvivalAction::Crafting, nullptr);
    SurvivalMontages.Add(ESurvivalAction::Hunting, nullptr);
    SurvivalMontages.Add(ESurvivalAction::Building, nullptr);
    SurvivalMontages.Add(ESurvivalAction::Climbing, nullptr);
    SurvivalMontages.Add(ESurvivalAction::Swimming, nullptr);
    SurvivalMontages.Add(ESurvivalAction::Hiding, nullptr);
    SurvivalMontages.Add(ESurvivalAction::Fighting, nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("Survival montages initialized"));
}

UAnimSequence* UAnim_MotionMatchingSystem::GetRandomAnimationFromArray(const TArray<UAnimSequence*>& AnimArray)
{
    if (AnimArray.Num() == 0)
    {
        return nullptr;
    }
    
    int32 RandomIndex = FMath::RandRange(0, AnimArray.Num() - 1);
    return AnimArray[RandomIndex];
}