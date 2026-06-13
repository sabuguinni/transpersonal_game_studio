#include "Anim_MotionMatchingComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingComponent::UAnim_MotionMatchingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    OwnerCharacter = nullptr;
    SkeletalMesh = nullptr;
    CurrentBlendWeight = 1.0f;
    DesiredVelocity = FVector::ZeroVector;
    CurrentVelocity = FVector::ZeroVector;
}

void UAnim_MotionMatchingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMesh = OwnerCharacter->GetMesh();
    }
    
    InitializeMotionDatabase();
}

void UAnim_MotionMatchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Update current velocity
    CurrentVelocity = OwnerCharacter->GetVelocity();
    
    // Update motion matching if we have a desired velocity
    if (!DesiredVelocity.IsNearlyZero())
    {
        UpdateMotionMatching(DesiredVelocity, CurrentVelocity);
    }
}

void UAnim_MotionMatchingComponent::UpdateMotionMatching(const FVector& InDesiredVelocity, const FVector& InCurrentVelocity)
{
    DesiredVelocity = InDesiredVelocity;
    CurrentVelocity = InCurrentVelocity;
    
    // Find best matching motion
    FAnim_MotionData BestMatch = FindBestMatch(DesiredVelocity, CurrentVelocity);
    
    // Blend to new motion if it's different from current
    if (BestMatch.AnimSequence != CurrentMotion.AnimSequence)
    {
        BlendToMotion(BestMatch, MatchingSettings.BlendTime);
    }
}

FAnim_MotionData UAnim_MotionMatchingComponent::FindBestMatch(const FVector& TargetVelocity, const FVector& CurrentVel)
{
    if (MotionDatabase.Num() == 0)
    {
        return FAnim_MotionData();
    }
    
    float BestScore = FLT_MAX;
    int32 BestIndex = 0;
    
    // Evaluate all motions in database
    for (int32 i = 0; i < MotionDatabase.Num(); ++i)
    {
        float Score = CalculateMotionScore(MotionDatabase[i], TargetVelocity, CurrentVel);
        if (Score < BestScore)
        {
            BestScore = Score;
            BestIndex = i;
        }
    }
    
    return MotionDatabase[BestIndex];
}

float UAnim_MotionMatchingComponent::CalculateMotionScore(const FAnim_MotionData& Motion, const FVector& TargetVel, const FVector& CurrentVel)
{
    float Score = 0.0f;
    
    // Velocity matching
    FVector VelocityDiff = Motion.RootMotionVelocity - TargetVel;
    Score += VelocityDiff.SizeSquared() * MatchingSettings.VelocityWeight;
    
    // Direction matching
    if (!TargetVel.IsNearlyZero() && !Motion.RootMotionVelocity.IsNearlyZero())
    {
        FVector TargetDir = TargetVel.GetSafeNormal();
        FVector MotionDir = Motion.RootMotionVelocity.GetSafeNormal();
        float DirectionDiff = 1.0f - FVector::DotProduct(TargetDir, MotionDir);
        Score += DirectionDiff * MatchingSettings.DirectionWeight;
    }
    
    // Acceleration matching (simplified)
    FVector AccelDiff = (TargetVel - CurrentVel);
    FVector MotionAccel = Motion.RootMotionVelocity - CurrentVel;
    Score += (AccelDiff - MotionAccel).SizeSquared() * MatchingSettings.AccelerationWeight;
    
    return Score;
}

void UAnim_MotionMatchingComponent::AddMotionToDatabase(const FAnim_MotionData& NewMotion)
{
    MotionDatabase.Add(NewMotion);
}

void UAnim_MotionMatchingComponent::LoadMotionDatabase()
{
    if (!MotionDataTable)
    {
        return;
    }
    
    // Load from data table
    TArray<FName> RowNames = MotionDataTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        FAnim_MotionData* RowData = MotionDataTable->FindRow<FAnim_MotionData>(RowName, TEXT("LoadMotionDatabase"));
        if (RowData)
        {
            MotionDatabase.Add(*RowData);
        }
    }
}

void UAnim_MotionMatchingComponent::InitializeMotionDatabase()
{
    // Clear existing database
    MotionDatabase.Empty();
    
    // Load from data table if available
    if (MotionDataTable)
    {
        LoadMotionDatabase();
    }
    
    // If no data table, create basic motion set
    if (MotionDatabase.Num() == 0)
    {
        // Create basic idle motion
        FAnim_MotionData IdleMotion;
        IdleMotion.MotionTag = TEXT("Idle");
        IdleMotion.RootMotionVelocity = FVector::ZeroVector;
        IdleMotion.Duration = 2.0f;
        MotionDatabase.Add(IdleMotion);
        
        // Create basic walk motion
        FAnim_MotionData WalkMotion;
        WalkMotion.MotionTag = TEXT("Walk");
        WalkMotion.RootMotionVelocity = FVector(200.0f, 0.0f, 0.0f);
        WalkMotion.Duration = 1.0f;
        MotionDatabase.Add(WalkMotion);
        
        // Create basic run motion
        FAnim_MotionData RunMotion;
        RunMotion.MotionTag = TEXT("Run");
        RunMotion.RootMotionVelocity = FVector(500.0f, 0.0f, 0.0f);
        RunMotion.Duration = 0.8f;
        MotionDatabase.Add(RunMotion);
    }
    
    // Set initial motion
    if (MotionDatabase.Num() > 0)
    {
        CurrentMotion = MotionDatabase[0];
    }
}

void UAnim_MotionMatchingComponent::BlendToMotion(const FAnim_MotionData& NewMotion, float BlendTime)
{
    CurrentMotion = NewMotion;
    CurrentBlendWeight = 1.0f;
    
    // Apply to skeletal mesh animation
    if (SkeletalMesh && SkeletalMesh->GetAnimInstance())
    {
        if (NewMotion.AnimSequence)
        {
            SkeletalMesh->GetAnimInstance()->Montage_Play(nullptr, 1.0f);
        }
    }
}