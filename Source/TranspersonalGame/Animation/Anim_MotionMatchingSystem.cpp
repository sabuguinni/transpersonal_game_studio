#include "Anim_MotionMatchingSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"

UAnim_MotionMatchingSystem::UAnim_MotionMatchingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    SearchRadius = 100.0f;
    BlendTime = 0.2f;
    UpdateFrequency = 30.0f;
    LastUpdateTime = 0.0f;
    bIsInitialized = false;
    CurrentClip = nullptr;
    CurrentPlayTime = 0.0f;
    SkeletalMeshComponent = nullptr;
}

void UAnim_MotionMatchingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void UAnim_MotionMatchingSystem::InitializeComponent()
{
    if (AActor* Owner = GetOwner())
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!SkeletalMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("MotionMatchingSystem: No SkeletalMeshComponent found on owner"));
            return;
        }

        // Build motion database if empty
        if (MotionDatabase.Num() == 0)
        {
            BuildMotionDatabase();
        }

        bIsInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("MotionMatchingSystem initialized with %d motion clips"), MotionDatabase.Num());
    }
}

void UAnim_MotionMatchingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsInitialized || !SkeletalMeshComponent)
    {
        return;
    }

    // Update at specified frequency
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= (1.0f / UpdateFrequency))
    {
        UpdateCurrentMotion(DeltaTime);
        LastUpdateTime = CurrentTime;
    }

    // Update play time
    if (CurrentClip)
    {
        CurrentPlayTime += DeltaTime;
    }
}

void UAnim_MotionMatchingSystem::UpdateCurrentMotion(float DeltaTime)
{
    // Update motion data
    UpdateMotionData();

    // Find best matching clip
    FAnim_MotionClip* BestClip = FindBestMatchingClip(CurrentMotionData);
    
    if (BestClip && BestClip != CurrentClip)
    {
        PlayMotionClip(BestClip);
    }
}

void UAnim_MotionMatchingSystem::UpdateMotionData()
{
    if (!SkeletalMeshComponent)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Get velocity and speed
    CurrentMotionData.Velocity = GetCharacterVelocity();
    CurrentMotionData.Speed = CurrentMotionData.Velocity.Size();

    // Calculate movement direction relative to actor forward
    if (CurrentMotionData.Speed > 1.0f)
    {
        FVector Forward = Owner->GetActorForwardVector();
        FVector VelocityNorm = CurrentMotionData.Velocity.GetSafeNormal();
        CurrentMotionData.Direction = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, VelocityNorm)));
        
        // Check if moving backwards
        FVector Right = Owner->GetActorRightVector();
        float RightDot = FVector::DotProduct(Right, VelocityNorm);
        if (RightDot < 0.0f)
        {
            CurrentMotionData.Direction = -CurrentMotionData.Direction;
        }
    }
    else
    {
        CurrentMotionData.Direction = 0.0f;
    }

    // Update state flags
    CurrentMotionData.bIsInAir = IsCharacterInAir();
    CurrentMotionData.bIsCrouching = IsCharacterCrouching();

    // Update survival state based on character state
    if (ACharacter* Character = Cast<ACharacter>(Owner))
    {
        // This would be set by the character's survival system
        CurrentMotionData.SurvivalState = ESurvivalState::Normal;
    }
}

FAnim_MotionClip* UAnim_MotionMatchingSystem::FindBestMatchingClip(const FAnim_MotionData& TargetMotion)
{
    if (MotionDatabase.Num() == 0)
    {
        return nullptr;
    }

    FAnim_MotionClip* BestClip = nullptr;
    float BestDistance = FLT_MAX;

    for (FAnim_MotionClip& Clip : MotionDatabase)
    {
        float Distance = CalculateMotionDistance(TargetMotion, Clip.MotionData);
        
        if (Distance < BestDistance && Distance <= SearchRadius)
        {
            BestDistance = Distance;
            BestClip = &Clip;
        }
    }

    return BestClip;
}

float UAnim_MotionMatchingSystem::CalculateMotionDistance(const FAnim_MotionData& A, const FAnim_MotionData& B)
{
    float Distance = 0.0f;

    // Speed difference (weighted heavily)
    Distance += FMath::Abs(A.Speed - B.Speed) * 2.0f;

    // Direction difference
    float DirectionDiff = FMath::Abs(A.Direction - B.Direction);
    DirectionDiff = FMath::Min(DirectionDiff, 360.0f - DirectionDiff); // Handle wrap-around
    Distance += DirectionDiff * 0.5f;

    // State mismatches (high penalty)
    if (A.bIsInAir != B.bIsInAir)
    {
        Distance += 100.0f;
    }

    if (A.bIsCrouching != B.bIsCrouching)
    {
        Distance += 50.0f;
    }

    if (A.SurvivalState != B.SurvivalState)
    {
        Distance += 30.0f;
    }

    return Distance;
}

void UAnim_MotionMatchingSystem::PlayMotionClip(FAnim_MotionClip* Clip)
{
    if (!Clip || !Clip->AnimSequence || !SkeletalMeshComponent)
    {
        return;
    }

    CurrentClip = Clip;
    CurrentPlayTime = Clip->StartTime;

    // Play the animation sequence
    if (UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance())
    {
        // This would typically use a custom anim node or montage system
        // For now, we'll use a simple approach
        SkeletalMeshComponent->PlayAnimation(Clip->AnimSequence, false);
        
        UE_LOG(LogTemp, Log, TEXT("Playing motion clip: %s"), 
               Clip->AnimSequence ? *Clip->AnimSequence->GetName() : TEXT("None"));
    }
}

void UAnim_MotionMatchingSystem::AddMotionClip(UAnimSequence* Animation, float StartTime, float EndTime, const FAnim_MotionData& MotionData)
{
    if (!Animation)
    {
        return;
    }

    FAnim_MotionClip NewClip;
    NewClip.AnimSequence = Animation;
    NewClip.StartTime = StartTime;
    NewClip.EndTime = EndTime;
    NewClip.MotionData = MotionData;
    NewClip.Quality = 1.0f;

    MotionDatabase.Add(NewClip);
    
    UE_LOG(LogTemp, Log, TEXT("Added motion clip: %s (%.2f-%.2f)"), 
           *Animation->GetName(), StartTime, EndTime);
}

void UAnim_MotionMatchingSystem::BuildMotionDatabase()
{
    // This would typically load animations from a data table or asset registry
    // For now, we'll create some basic motion data for common states
    
    // Create basic motion data for different movement states
    FAnim_MotionData IdleMotion;
    IdleMotion.Speed = 0.0f;
    IdleMotion.Direction = 0.0f;
    IdleMotion.bIsInAir = false;
    IdleMotion.bIsCrouching = false;
    IdleMotion.SurvivalState = ESurvivalState::Normal;

    FAnim_MotionData WalkMotion;
    WalkMotion.Speed = 150.0f;
    WalkMotion.Direction = 0.0f;
    WalkMotion.bIsInAir = false;
    WalkMotion.bIsCrouching = false;
    WalkMotion.SurvivalState = ESurvivalState::Normal;

    FAnim_MotionData RunMotion;
    RunMotion.Speed = 400.0f;
    RunMotion.Direction = 0.0f;
    RunMotion.bIsInAir = false;
    RunMotion.bIsCrouching = false;
    RunMotion.SurvivalState = ESurvivalState::Normal;

    FAnim_MotionData JumpMotion;
    JumpMotion.Speed = 200.0f;
    JumpMotion.Direction = 0.0f;
    JumpMotion.bIsInAir = true;
    JumpMotion.bIsCrouching = false;
    JumpMotion.SurvivalState = ESurvivalState::Normal;

    // Note: Actual animation sequences would be loaded from assets
    // This is just setting up the motion data structure
    
    UE_LOG(LogTemp, Log, TEXT("Motion database built with basic motion data"));
}

FVector UAnim_MotionMatchingSystem::GetCharacterVelocity() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            return MovementComp->Velocity;
        }
    }
    
    return FVector::ZeroVector;
}

bool UAnim_MotionMatchingSystem::IsCharacterInAir() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            return MovementComp->IsFalling();
        }
    }
    
    return false;
}

bool UAnim_MotionMatchingSystem::IsCharacterCrouching() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        return Character->bIsCrouched;
    }
    
    return false;
}