#include "MotionMatchingController.h"
#include "../Core/AnimationSystemManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UMotionMatchingController::UMotionMatchingController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    ActiveDatabase = EMotionMatchingDatabase::Locomotion;
    LastQueryUpdateTime = 0.0f;
    PreviousVelocity = FVector::ZeroVector;
    AnimationManager = nullptr;
}

void UMotionMatchingController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find animation manager component
    AnimationManager = GetOwner()->FindComponentByClass<UAnimationSystemManager>();
    if (!AnimationManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingController: No AnimationSystemManager found"));
    }
    
    // Initialize databases if not set
    if (AnimationDatabases.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingController: No animation databases configured"));
    }
}

void UMotionMatchingController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update query at specified rate
    if (CurrentTime - LastQueryUpdateTime >= (1.0f / QueryUpdateRate))
    {
        UpdateQueryFromMovement();
        SelectOptimalDatabase();
        LastQueryUpdateTime = CurrentTime;
    }
}

void UMotionMatchingController::SetActiveDatabase(EMotionMatchingDatabase Database)
{
    if (AnimationDatabases.Contains(Database))
    {
        ActiveDatabase = Database;
        UE_LOG(LogTemp, Log, TEXT("Motion Matching: Switched to database %d"), (int32)Database);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Motion Matching: Database %d not found"), (int32)Database);
    }
}

void UMotionMatchingController::UpdateQuery(const FMotionMatchingQuery& NewQuery)
{
    CurrentQuery = NewQuery;
    
    // Apply emotional modulation if animation manager is available
    if (AnimationManager)
    {
        float FearLevel = AnimationManager->CurrentAnimationData.FearLevel;
        float ExhaustionLevel = AnimationManager->CurrentAnimationData.Exhaustion;
        ModulateQueryWithEmotion(FearLevel, ExhaustionLevel);
    }
}

float UMotionMatchingController::CalculateDynamicBlendTime(float FearLevel, float MovementIntensity)
{
    // Base blend time
    float BlendTime = FMath::Lerp(BlendTimeMin, BlendTimeMax, 0.5f);
    
    // Fear makes blending faster (more reactive)
    BlendTime *= (1.0f - (FearLevel * FearInfluenceOnBlending));
    
    // High movement intensity requires faster blending
    BlendTime *= (1.0f - (MovementIntensity * 0.2f));
    
    return FMath::Clamp(BlendTime, BlendTimeMin, BlendTimeMax);
}

UPoseSearchDatabase* UMotionMatchingController::GetCurrentDatabase() const
{
    if (AnimationDatabases.Contains(ActiveDatabase))
    {
        return AnimationDatabases[ActiveDatabase];
    }
    return nullptr;
}

void UMotionMatchingController::ModulateQueryWithEmotion(float FearLevel, float ExhaustionLevel)
{
    // Fear affects movement characteristics
    if (FearLevel > 0.5f)
    {
        // High fear: more erratic, faster movements
        CurrentQuery.DesiredSpeed *= (1.0f + FearLevel * 0.3f);
        CurrentQuery.EmotionalState = FearLevel;
    }
    
    // Exhaustion affects movement efficiency
    if (ExhaustionLevel > 0.3f)
    {
        // Exhaustion: slower, less precise movements
        CurrentQuery.DesiredSpeed *= (1.0f - ExhaustionLevel * ExhaustionInfluenceOnSpeed);
        CurrentQuery.DesiredAcceleration *= (1.0f - ExhaustionLevel * 0.4f);
    }
}

void UMotionMatchingController::UpdateQueryFromMovement()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || !Character->GetCharacterMovement())
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    FVector CurrentVelocity = MovementComp->Velocity;
    
    // Update query with current movement data
    CurrentQuery.DesiredVelocity = CurrentVelocity;
    CurrentQuery.DesiredSpeed = CurrentVelocity.Size();
    
    // Calculate acceleration
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        CurrentQuery.DesiredAcceleration = (CurrentVelocity - PreviousVelocity) / DeltaTime;
    }
    
    // Calculate direction relative to character forward
    FVector Forward = Character->GetActorForwardVector();
    FVector VelocityNorm = CurrentVelocity.GetSafeNormal();
    CurrentQuery.DesiredDirection = FVector::DotProduct(Forward, VelocityNorm);
    
    // Update terrain complexity
    CurrentQuery.TerrainComplexity = CalculateTerrainComplexity();
    
    PreviousVelocity = CurrentVelocity;
}

void UMotionMatchingController::SelectOptimalDatabase()
{
    if (!AnimationManager)
    {
        return;
    }
    
    ECharacterMovementState MovementState = AnimationManager->CurrentAnimationData.MovementState;
    float FearLevel = AnimationManager->CurrentAnimationData.FearLevel;
    float Speed = CurrentQuery.DesiredSpeed;
    
    // Database selection logic based on context
    EMotionMatchingDatabase NewDatabase = ActiveDatabase;
    
    // High fear or frightened state
    if (FearLevel > 0.7f || MovementState == ECharacterMovementState::Frightened)
    {
        NewDatabase = EMotionMatchingDatabase::Emotional;
    }
    // Stealth movement
    else if (MovementState == ECharacterMovementState::Sneaking || MovementState == ECharacterMovementState::Crouching)
    {
        NewDatabase = EMotionMatchingDatabase::Stealth;
    }
    // Climbing or complex terrain
    else if (MovementState == ECharacterMovementState::Climbing || CurrentQuery.TerrainComplexity > 0.6f)
    {
        NewDatabase = EMotionMatchingDatabase::Climbing;
    }
    // Interaction states
    else if (MovementState == ECharacterMovementState::Interacting)
    {
        NewDatabase = EMotionMatchingDatabase::Interaction;
    }
    // Default locomotion
    else
    {
        NewDatabase = EMotionMatchingDatabase::Locomotion;
    }
    
    // Switch database if needed
    if (NewDatabase != ActiveDatabase)
    {
        SetActiveDatabase(NewDatabase);
    }
}

float UMotionMatchingController::CalculateTerrainComplexity()
{
    // This would integrate with terrain analysis system
    // For now, return a base value that can be overridden
    if (AnimationManager)
    {
        switch (AnimationManager->CurrentAnimationData.CurrentTerrain)
        {
            case ETerrainType::Rocky:
            case ETerrainType::Uneven:
                return 0.8f;
            case ETerrainType::Uphill:
            case ETerrainType::Downhill:
                return 0.6f;
            case ETerrainType::Muddy:
            case ETerrainType::Vegetation:
                return 0.4f;
            case ETerrainType::Water:
                return 0.7f;
            default:
                return 0.2f;
        }
    }
    
    return 0.2f;
}