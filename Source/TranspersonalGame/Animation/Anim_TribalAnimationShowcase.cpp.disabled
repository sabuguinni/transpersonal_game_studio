#include "Anim_TribalAnimationShowcase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnim_TribalAnimationShowcase::UAnim_TribalAnimationShowcase()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize state
    bShowcaseActive = false;
    CurrentAnimationType = EAnim_TribalAnimationType::Idle;
    CurrentAnimationIndex = 0;
    CurrentAnimationTimer = 0.0f;
    TransitionTimer = 0.0f;
    bIsTransitioning = false;
    TotalShowcaseTime = 0.0f;
    ShowcaseLoopCount = 0;

    // Initialize component references
    TargetMeshComponent = nullptr;
    TargetAnimInstance = nullptr;

    // Set default showcase settings
    ShowcaseSettings.AnimationDuration = 3.0f;
    ShowcaseSettings.TransitionTime = 0.5f;
    ShowcaseSettings.bAutoLoop = true;
    ShowcaseSettings.bRandomizeOrder = false;
}

void UAnim_TribalAnimationShowcase::BeginPlay()
{
    Super::BeginPlay();

    // Find the skeletal mesh component on the owner
    TargetMeshComponent = FindOwnerMeshComponent();
    if (TargetMeshComponent)
    {
        TargetAnimInstance = TargetMeshComponent->GetAnimInstance();
        UE_LOG(LogTemp, Log, TEXT("TribalAnimationShowcase: Found mesh component and anim instance"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalAnimationShowcase: No skeletal mesh component found on owner"));
    }

    // Initialize tribal animations
    InitializeTribalAnimations();
    ValidateAnimationData();
}

void UAnim_TribalAnimationShowcase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bShowcaseActive)
    {
        UpdateShowcase(DeltaTime);
    }
}

void UAnim_TribalAnimationShowcase::StartShowcase()
{
    if (!TargetMeshComponent || !TargetAnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalAnimationShowcase: Cannot start showcase - missing mesh or anim instance"));
        return;
    }

    if (TribalAnimations.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalAnimationShowcase: Cannot start showcase - no animations configured"));
        return;
    }

    bShowcaseActive = true;
    CurrentAnimationIndex = 0;
    CurrentAnimationTimer = 0.0f;
    TransitionTimer = 0.0f;
    bIsTransitioning = false;
    TotalShowcaseTime = 0.0f;

    // Randomize sequence if enabled
    if (ShowcaseSettings.bRandomizeOrder && ShowcaseSettings.ShowcaseSequence.Num() > 1)
    {
        for (int32 i = ShowcaseSettings.ShowcaseSequence.Num() - 1; i > 0; i--)
        {
            int32 j = FMath::RandRange(0, i);
            ShowcaseSettings.ShowcaseSequence.Swap(i, j);
        }
    }

    // Start with first animation
    if (ShowcaseSettings.ShowcaseSequence.Num() > 0)
    {
        PlayTribalAnimation(ShowcaseSettings.ShowcaseSequence[0]);
    }
    else if (TribalAnimations.Num() > 0)
    {
        PlayTribalAnimation(TribalAnimations[0].AnimationType);
    }

    UE_LOG(LogTemp, Log, TEXT("TribalAnimationShowcase: Started showcase with %d animations"), TribalAnimations.Num());
}

void UAnim_TribalAnimationShowcase::StopShowcase()
{
    bShowcaseActive = false;
    CurrentAnimationTimer = 0.0f;
    TransitionTimer = 0.0f;
    bIsTransitioning = false;

    // Stop current montage if playing
    if (TargetAnimInstance)
    {
        TargetAnimInstance->StopAllMontages(0.5f);
    }

    UE_LOG(LogTemp, Log, TEXT("TribalAnimationShowcase: Stopped showcase"));
}

void UAnim_TribalAnimationShowcase::PlayTribalAnimation(EAnim_TribalAnimationType AnimationType)
{
    // Find animation data for the requested type
    FAnim_TribalAnimationData* AnimData = nullptr;
    for (FAnim_TribalAnimationData& Data : TribalAnimations)
    {
        if (Data.AnimationType == AnimationType)
        {
            AnimData = &Data;
            break;
        }
    }

    if (!AnimData)
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalAnimationShowcase: Animation type not found: %d"), (int32)AnimationType);
        return;
    }

    CurrentAnimationType = AnimationType;
    
    // Play the animation montage
    if (PlayAnimationMontage(*AnimData))
    {
        UE_LOG(LogTemp, Log, TEXT("TribalAnimationShowcase: Playing animation: %d"), (int32)AnimationType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalAnimationShowcase: Failed to play animation: %d"), (int32)AnimationType);
    }
}

void UAnim_TribalAnimationShowcase::PlayNextAnimation()
{
    if (!bShowcaseActive || TribalAnimations.Num() == 0)
    {
        return;
    }

    // Use showcase sequence if defined, otherwise use all animations
    if (ShowcaseSettings.ShowcaseSequence.Num() > 0)
    {
        CurrentAnimationIndex = (CurrentAnimationIndex + 1) % ShowcaseSettings.ShowcaseSequence.Num();
        PlayTribalAnimation(ShowcaseSettings.ShowcaseSequence[CurrentAnimationIndex]);
    }
    else
    {
        CurrentAnimationIndex = (CurrentAnimationIndex + 1) % TribalAnimations.Num();
        PlayTribalAnimation(TribalAnimations[CurrentAnimationIndex].AnimationType);
    }

    // Check if we completed a full loop
    if (CurrentAnimationIndex == 0)
    {
        ShowcaseLoopCount++;
        if (!ShowcaseSettings.bAutoLoop)
        {
            StopShowcase();
        }
    }
}

void UAnim_TribalAnimationShowcase::SetShowcaseSettings(const FAnim_TribalShowcaseSettings& NewSettings)
{
    ShowcaseSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("TribalAnimationShowcase: Updated showcase settings"));
}

float UAnim_TribalAnimationShowcase::GetShowcaseProgress() const
{
    if (!bShowcaseActive || TribalAnimations.Num() == 0)
    {
        return 0.0f;
    }

    int32 TotalAnimations = ShowcaseSettings.ShowcaseSequence.Num() > 0 ? 
        ShowcaseSettings.ShowcaseSequence.Num() : TribalAnimations.Num();
    
    float AnimProgress = CurrentAnimationTimer / ShowcaseSettings.AnimationDuration;
    float TotalProgress = (CurrentAnimationIndex + AnimProgress) / TotalAnimations;
    
    return FMath::Clamp(TotalProgress, 0.0f, 1.0f);
}

void UAnim_TribalAnimationShowcase::AddAnimationToShowcase(const FAnim_TribalAnimationData& AnimationData)
{
    TribalAnimations.Add(AnimationData);
    UE_LOG(LogTemp, Log, TEXT("TribalAnimationShowcase: Added animation type: %d"), (int32)AnimationData.AnimationType);
}

void UAnim_TribalAnimationShowcase::RemoveAnimationFromShowcase(EAnim_TribalAnimationType AnimationType)
{
    for (int32 i = TribalAnimations.Num() - 1; i >= 0; i--)
    {
        if (TribalAnimations[i].AnimationType == AnimationType)
        {
            TribalAnimations.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("TribalAnimationShowcase: Removed animation type: %d"), (int32)AnimationType);
        }
    }
}

void UAnim_TribalAnimationShowcase::ClearShowcaseAnimations()
{
    TribalAnimations.Empty();
    UE_LOG(LogTemp, Log, TEXT("TribalAnimationShowcase: Cleared all animations"));
}

void UAnim_TribalAnimationShowcase::InitializeTribalAnimations()
{
    // Initialize default tribal animations (placeholders for now)
    // In a real implementation, these would reference actual animation assets
    
    FAnim_TribalAnimationData IdleAnim;
    IdleAnim.AnimationType = EAnim_TribalAnimationType::Idle;
    IdleAnim.PlayRate = 1.0f;
    IdleAnim.bLooping = true;
    TribalAnimations.Add(IdleAnim);

    FAnim_TribalAnimationData WalkAnim;
    WalkAnim.AnimationType = EAnim_TribalAnimationType::Walk;
    WalkAnim.PlayRate = 1.0f;
    WalkAnim.bLooping = true;
    TribalAnimations.Add(WalkAnim);

    FAnim_TribalAnimationData RunAnim;
    RunAnim.AnimationType = EAnim_TribalAnimationType::Run;
    RunAnim.PlayRate = 1.2f;
    RunAnim.bLooping = true;
    TribalAnimations.Add(RunAnim);

    FAnim_TribalAnimationData CraftingAnim;
    CraftingAnim.AnimationType = EAnim_TribalAnimationType::CraftingAction;
    CraftingAnim.PlayRate = 1.0f;
    CraftingAnim.bLooping = false;
    CraftingAnim.BlendInTime = 0.5f;
    CraftingAnim.BlendOutTime = 0.5f;
    TribalAnimations.Add(CraftingAnim);

    FAnim_TribalAnimationData HuntingAnim;
    HuntingAnim.AnimationType = EAnim_TribalAnimationType::HuntingPose;
    HuntingAnim.PlayRate = 0.8f;
    HuntingAnim.bLooping = true;
    TribalAnimations.Add(HuntingAnim);

    // Set up default showcase sequence
    ShowcaseSettings.ShowcaseSequence.Add(EAnim_TribalAnimationType::Idle);
    ShowcaseSettings.ShowcaseSequence.Add(EAnim_TribalAnimationType::Walk);
    ShowcaseSettings.ShowcaseSequence.Add(EAnim_TribalAnimationType::Run);
    ShowcaseSettings.ShowcaseSequence.Add(EAnim_TribalAnimationType::CraftingAction);
    ShowcaseSettings.ShowcaseSequence.Add(EAnim_TribalAnimationType::HuntingPose);

    UE_LOG(LogTemp, Log, TEXT("TribalAnimationShowcase: Initialized %d tribal animations"), TribalAnimations.Num());
}

void UAnim_TribalAnimationShowcase::UpdateShowcase(float DeltaTime)
{
    TotalShowcaseTime += DeltaTime;

    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        if (TransitionTimer >= ShowcaseSettings.TransitionTime)
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
            TransitionToNextAnimation();
        }
    }
    else
    {
        CurrentAnimationTimer += DeltaTime;
        if (CurrentAnimationTimer >= ShowcaseSettings.AnimationDuration)
        {
            bIsTransitioning = true;
            CurrentAnimationTimer = 0.0f;
        }
    }
}

void UAnim_TribalAnimationShowcase::TransitionToNextAnimation()
{
    PlayNextAnimation();
}

bool UAnim_TribalAnimationShowcase::PlayAnimationMontage(const FAnim_TribalAnimationData& AnimationData)
{
    if (!TargetAnimInstance || !AnimationData.AnimationMontage)
    {
        // For now, just log the animation type since we don't have actual montage assets
        UE_LOG(LogTemp, Log, TEXT("TribalAnimationShowcase: Would play animation montage for type: %d"), (int32)AnimationData.AnimationType);
        return true;
    }

    // Play the actual montage when assets are available
    float MontageLength = TargetAnimInstance->Montage_Play(
        AnimationData.AnimationMontage,
        AnimationData.PlayRate,
        EMontagePlayReturnType::MontageLength,
        0.0f,
        true
    );

    return MontageLength > 0.0f;
}

void UAnim_TribalAnimationShowcase::OnAnimationComplete()
{
    if (bShowcaseActive)
    {
        bIsTransitioning = true;
        TransitionTimer = 0.0f;
    }
}

USkeletalMeshComponent* UAnim_TribalAnimationShowcase::FindOwnerMeshComponent()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    // First try to get mesh from Character
    if (ACharacter* Character = Cast<ACharacter>(Owner))
    {
        return Character->GetMesh();
    }

    // Then try to get mesh from Pawn
    if (APawn* Pawn = Cast<APawn>(Owner))
    {
        return Pawn->FindComponentByClass<USkeletalMeshComponent>();
    }

    // Finally, try to find any skeletal mesh component
    return Owner->FindComponentByClass<USkeletalMeshComponent>();
}

void UAnim_TribalAnimationShowcase::ValidateAnimationData()
{
    int32 ValidAnimations = 0;
    for (const FAnim_TribalAnimationData& AnimData : TribalAnimations)
    {
        if (AnimData.AnimationMontage != nullptr)
        {
            ValidAnimations++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("TribalAnimationShowcase: %d/%d animations have valid montage assets"), 
        ValidAnimations, TribalAnimations.Num());
}