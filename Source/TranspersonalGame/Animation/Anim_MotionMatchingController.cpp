#include "Anim_MotionMatchingController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UAnim_MotionMatchingController::UAnim_MotionMatchingController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Initialize motion matching settings
    VelocityWeight = 1.0f;
    DirectionWeight = 0.8f;
    TurnRateWeight = 0.6f;
    MinimumMatchingThreshold = 0.3f;
    BlendTime = 0.2f;

    // Initialize internal state
    bMotionMatchingEnabled = true;
    LastUpdateTime = 0.0f;
    MotionUpdateInterval = 0.016f; // ~60 FPS
    bIsPlayingMontage = false;
    CurrentMontageTime = 0.0f;
    DatabaseSearchIndex = 0;
    LastDatabaseUpdateTime = 0.0f;
    DatabaseUpdateInterval = 0.1f; // Update database search 10 times per second

    // Initialize references
    OwnerCharacter = nullptr;
    CharacterMesh = nullptr;
    MovementBlendSpace = nullptr;
    JumpMontage = nullptr;
    LandingMontage = nullptr;
}

void UAnim_MotionMatchingController::BeginPlay()
{
    Super::BeginPlay();

    // Get character reference
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CharacterMesh = OwnerCharacter->GetMesh();
        UE_LOG(LogTemp, Log, TEXT("Motion Matching Controller initialized for character: %s"), *OwnerCharacter->GetName());
    }

    // Initialize motion database with default clips
    InitializeMotionDatabase();
}

void UAnim_MotionMatchingController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bMotionMatchingEnabled || !OwnerCharacter || !CharacterMesh)
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Update motion data at specified interval
    if (CurrentTime - LastUpdateTime >= MotionUpdateInterval)
    {
        UpdateMotionData();
        LastUpdateTime = CurrentTime;

        // Find and play best matching animation if not playing a montage
        if (!bIsPlayingMontage && MotionDatabase.Num() > 0)
        {
            if (CurrentTime - LastDatabaseUpdateTime >= DatabaseUpdateInterval)
            {
                BestMatchingClip = FindBestMatchingClip(CurrentMotionData);
                if (BestMatchingClip.MatchingScore >= MinimumMatchingThreshold)
                {
                    PlayMatchingAnimation(BestMatchingClip);
                }
                LastDatabaseUpdateTime = CurrentTime;
            }
        }
    }

    // Update montage tracking
    if (bIsPlayingMontage && CharacterMesh->GetAnimInstance())
    {
        UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
        if (!AnimInstance->IsAnyMontagePlaying())
        {
            bIsPlayingMontage = false;
            CurrentMontageTime = 0.0f;
        }
    }
}

void UAnim_MotionMatchingController::UpdateMotionData()
{
    if (!OwnerCharacter)
    {
        return;
    }

    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }

    // Update velocity and speed
    CurrentMotionData.Velocity = MovementComp->Velocity;
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();

    // Calculate movement direction relative to character forward
    if (CurrentMotionData.Speed > 0.1f)
    {
        FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        FVector VelocityDirection = CurrentMotionData.Velocity.GetSafeNormal();
        CurrentMotionData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ForwardVector, VelocityDirection)));
        
        // Determine if turning left or right
        FVector RightVector = OwnerCharacter->GetActorRightVector();
        float RightDot = FVector::DotProduct(RightVector, VelocityDirection);
        if (RightDot < 0.0f)
        {
            CurrentMotionData.Direction = -CurrentMotionData.Direction;
        }
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
    }

    // Update air state
    CurrentMotionData.bIsInAir = MovementComp->IsFalling();

    // Update crouching state
    CurrentMotionData.bIsCrouching = MovementComp->IsCrouching();

    // Calculate turn rate
    static FRotator LastRotation = OwnerCharacter->GetActorRotation();
    FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
    float DeltaYaw = FMath::FindDeltaAngleDegrees(LastRotation.Yaw, CurrentRotation.Yaw);
    CurrentMotionData.TurnRate = DeltaYaw / GetWorld()->GetDeltaSeconds();
    LastRotation = CurrentRotation;
}

FAnim_MotionClip UAnim_MotionMatchingController::FindBestMatchingClip(const FAnim_MotionMatchingData& TargetMotion)
{
    FAnim_MotionClip BestClip;
    float BestScore = 0.0f;

    for (const FAnim_MotionClip& Clip : MotionDatabase)
    {
        float Score = CalculateMotionScore(Clip.MotionData, TargetMotion);
        if (Score > BestScore)
        {
            BestScore = Score;
            BestClip = Clip;
            BestClip.MatchingScore = Score;
        }
    }

    return BestClip;
}

float UAnim_MotionMatchingController::CalculateMotionScore(const FAnim_MotionMatchingData& ClipMotion, const FAnim_MotionMatchingData& TargetMotion)
{
    float Score = 0.0f;

    // Velocity matching
    float VelocityDiff = FVector::Dist(ClipMotion.Velocity, TargetMotion.Velocity);
    float VelocityScore = FMath::Exp(-VelocityDiff * 0.01f); // Exponential decay
    Score += VelocityScore * VelocityWeight;

    // Direction matching
    float DirectionDiff = FMath::Abs(ClipMotion.Direction - TargetMotion.Direction);
    float DirectionScore = FMath::Exp(-DirectionDiff * 0.02f);
    Score += DirectionScore * DirectionWeight;

    // Turn rate matching
    float TurnRateDiff = FMath::Abs(ClipMotion.TurnRate - TargetMotion.TurnRate);
    float TurnRateScore = FMath::Exp(-TurnRateDiff * 0.01f);
    Score += TurnRateScore * TurnRateWeight;

    // Boolean state matching
    if (ClipMotion.bIsInAir == TargetMotion.bIsInAir)
    {
        Score += 0.5f;
    }

    if (ClipMotion.bIsCrouching == TargetMotion.bIsCrouching)
    {
        Score += 0.3f;
    }

    // Normalize score
    float MaxPossibleScore = VelocityWeight + DirectionWeight + TurnRateWeight + 0.8f;
    return Score / MaxPossibleScore;
}

void UAnim_MotionMatchingController::PlayMatchingAnimation(const FAnim_MotionClip& MotionClip)
{
    if (!CharacterMesh || !MotionClip.AnimationSequence)
    {
        return;
    }

    UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }

    // Create a temporary montage for the motion clip
    // In a full implementation, you would cache these montages
    UAnimMontage* TempMontage = NewObject<UAnimMontage>();
    if (TempMontage)
    {
        // Configure the montage with the animation sequence
        // This is a simplified implementation
        AnimInstance->Montage_Play(TempMontage, 1.0f, EMontagePlayReturnType::MontageLength, MotionClip.StartTime);
    }
}

void UAnim_MotionMatchingController::AddMotionClipToDatabase(UAnimSequence* Animation, float StartTime, float EndTime, const FAnim_MotionMatchingData& MotionData)
{
    if (!Animation)
    {
        return;
    }

    FAnim_MotionClip NewClip;
    NewClip.AnimationSequence = Animation;
    NewClip.StartTime = StartTime;
    NewClip.EndTime = EndTime;
    NewClip.MotionData = MotionData;
    NewClip.MatchingScore = 0.0f;

    MotionDatabase.Add(NewClip);

    UE_LOG(LogTemp, Log, TEXT("Added motion clip to database: %s"), *Animation->GetName());
}

void UAnim_MotionMatchingController::PlayJumpAnimation()
{
    if (JumpMontage && CharacterMesh)
    {
        UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(JumpMontage);
            bIsPlayingMontage = true;
            UE_LOG(LogTemp, Log, TEXT("Playing jump animation"));
        }
    }
}

void UAnim_MotionMatchingController::PlayLandingAnimation()
{
    if (LandingMontage && CharacterMesh)
    {
        UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(LandingMontage);
            bIsPlayingMontage = true;
            UE_LOG(LogTemp, Log, TEXT("Playing landing animation"));
        }
    }
}

void UAnim_MotionMatchingController::SetMovementBlendSpaceValues(float Speed, float Direction)
{
    if (!CharacterMesh)
    {
        return;
    }

    UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
    if (AnimInstance)
    {
        // Set blend space parameters
        // In a real implementation, you would set these through the animation blueprint
        UE_LOG(LogTemp, Log, TEXT("Setting blend space values - Speed: %f, Direction: %f"), Speed, Direction);
    }
}

void UAnim_MotionMatchingController::InitializeMotionDatabase()
{
    // Clear existing database
    MotionDatabase.Empty();

    // Add default motion clips
    // In a real implementation, you would load these from assets
    
    // Idle motion
    FAnim_MotionMatchingData IdleMotion;
    IdleMotion.Speed = 0.0f;
    IdleMotion.Direction = 0.0f;
    IdleMotion.bIsInAir = false;
    IdleMotion.bIsCrouching = false;
    IdleMotion.TurnRate = 0.0f;

    // Walk forward motion
    FAnim_MotionMatchingData WalkMotion;
    WalkMotion.Speed = 150.0f;
    WalkMotion.Direction = 0.0f;
    WalkMotion.bIsInAir = false;
    WalkMotion.bIsCrouching = false;
    WalkMotion.TurnRate = 0.0f;

    // Run forward motion
    FAnim_MotionMatchingData RunMotion;
    RunMotion.Speed = 400.0f;
    RunMotion.Direction = 0.0f;
    RunMotion.bIsInAir = false;
    RunMotion.bIsCrouching = false;
    RunMotion.TurnRate = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Motion database initialized with %d default clips"), MotionDatabase.Num());
}

void UAnim_MotionMatchingController::ClearMotionDatabase()
{
    MotionDatabase.Empty();
    UE_LOG(LogTemp, Log, TEXT("Motion database cleared"));
}

bool UAnim_MotionMatchingController::IsMotionMatchingActive() const
{
    return bMotionMatchingEnabled && OwnerCharacter != nullptr && CharacterMesh != nullptr;
}