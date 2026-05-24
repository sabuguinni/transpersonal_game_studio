#include "Anim_MontageController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnim_MontageController::UAnim_MontageController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for efficiency

    // Initialize state
    CurrentMontageType = EAnim_MontageType::None;
    CurrentMontage = nullptr;
    bIsMontageActive = false;
    MontagePosition = 0.0f;
    MontageLength = 0.0f;

    // Settings
    bAutoStopOnMovement = true;
    MovementThreshold = 50.0f; // cm/s
    bQueueMontages = false;

    // Cache
    LastLocation = FVector::ZeroVector;
    LastMovementTime = 0.0f;
}

void UAnim_MontageController::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner references
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter.IsValid())
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        if (SkeletalMeshComponent.IsValid())
        {
            AnimInstance = SkeletalMeshComponent->GetAnimInstance();
        }
        LastLocation = OwnerCharacter->GetActorLocation();
    }

    // Setup default montage library (these would be set in Blueprint or data assets)
    if (MontageLibrary.Num() == 0)
    {
        // Initialize with default values - actual montage assets would be assigned in Blueprint
        FAnim_MontageData JumpData;
        JumpData.PlayRate = 1.0f;
        JumpData.BlendInTime = 0.1f;
        JumpData.BlendOutTime = 0.2f;
        MontageLibrary.Add(EAnim_MontageType::Jump, JumpData);

        FAnim_MontageData InteractData;
        InteractData.PlayRate = 1.0f;
        InteractData.BlendInTime = 0.25f;
        InteractData.BlendOutTime = 0.25f;
        MontageLibrary.Add(EAnim_MontageType::Interact, InteractData);

        FAnim_MontageData CraftData;
        CraftData.PlayRate = 1.0f;
        CraftData.BlendInTime = 0.3f;
        CraftData.BlendOutTime = 0.3f;
        CraftData.bLooping = true;
        MontageLibrary.Add(EAnim_MontageType::Craft, CraftData);

        FAnim_MontageData GatherData;
        GatherData.PlayRate = 1.2f;
        GatherData.BlendInTime = 0.2f;
        GatherData.BlendOutTime = 0.2f;
        MontageLibrary.Add(EAnim_MontageType::Gather, GatherData);

        FAnim_MontageData EatData;
        EatData.PlayRate = 0.8f;
        EatData.BlendInTime = 0.4f;
        EatData.BlendOutTime = 0.4f;
        MontageLibrary.Add(EAnim_MontageType::Eat, EatData);

        FAnim_MontageData DrinkData;
        DrinkData.PlayRate = 1.0f;
        DrinkData.BlendInTime = 0.3f;
        DrinkData.BlendOutTime = 0.3f;
        MontageLibrary.Add(EAnim_MontageType::Drink, DrinkData);

        FAnim_MontageData FearData;
        FearData.PlayRate = 1.5f;
        FearData.BlendInTime = 0.1f;
        FearData.BlendOutTime = 0.5f;
        MontageLibrary.Add(EAnim_MontageType::Fear, FearData);

        FAnim_MontageData PainData;
        PainData.PlayRate = 1.2f;
        PainData.BlendInTime = 0.05f;
        PainData.BlendOutTime = 0.3f;
        MontageLibrary.Add(EAnim_MontageType::Pain, PainData);
    }
}

void UAnim_MontageController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateMontageState();
    
    if (bAutoStopOnMovement && bIsMontageActive)
    {
        CheckMovementInterrupt();
    }

    if (bQueueMontages && MontageQueue.Num() > 0)
    {
        ProcessMontageQueue();
    }
}

bool UAnim_MontageController::PlayMontage(EAnim_MontageType MontageType, float CustomPlayRate, FName StartSection)
{
    if (!AnimInstance.IsValid() || MontageType == EAnim_MontageType::None)
    {
        return false;
    }

    const FAnim_MontageData* MontageData = MontageLibrary.Find(MontageType);
    if (!MontageData || !MontageData->Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Montage not found for type: %d"), (int32)MontageType);
        return false;
    }

    // Stop current montage if playing
    if (bIsMontageActive && CurrentMontage)
    {
        AnimInstance->Montage_Stop(MontageData->BlendOutTime, CurrentMontage);
    }

    // Play the new montage
    float PlayRate = (CustomPlayRate > 0.0f) ? CustomPlayRate : MontageData->PlayRate;
    FName Section = (StartSection != NAME_None) ? StartSection : MontageData->StartSection;

    float MontageLength = AnimInstance->Montage_Play(MontageData->Montage, PlayRate, EMontagePlayReturnType::MontageLength, 0.0f, true);
    
    if (MontageLength > 0.0f)
    {
        CurrentMontageType = MontageType;
        CurrentMontage = MontageData->Montage;
        bIsMontageActive = true;
        this->MontageLength = MontageLength;

        // Jump to section if specified
        if (Section != NAME_None)
        {
            AnimInstance->Montage_JumpToSection(Section, CurrentMontage);
        }

        // Bind montage events
        FOnMontageBlendingOutStarted BlendingOutDelegate;
        BlendingOutDelegate.BindUObject(this, &UAnim_MontageController::OnMontageBlendingOut);
        AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, CurrentMontage);

        FOnMontageEnded EndedDelegate;
        EndedDelegate.BindUObject(this, &UAnim_MontageController::OnMontageEnded_Internal);
        AnimInstance->Montage_SetEndDelegate(EndedDelegate, CurrentMontage);

        // Broadcast event
        OnMontageStarted.Broadcast(MontageType, CurrentMontage);

        UE_LOG(LogTemp, Log, TEXT("Started montage: %s"), *CurrentMontage->GetName());
        return true;
    }

    return false;
}

bool UAnim_MontageController::PlayMontageByAsset(UAnimMontage* Montage, float PlayRate, float BlendInTime, FName StartSection)
{
    if (!AnimInstance.IsValid() || !Montage)
    {
        return false;
    }

    // Stop current montage
    if (bIsMontageActive && CurrentMontage)
    {
        AnimInstance->Montage_Stop(BlendInTime, CurrentMontage);
    }

    float MontageLength = AnimInstance->Montage_Play(Montage, PlayRate, EMontagePlayReturnType::MontageLength, 0.0f, true);
    
    if (MontageLength > 0.0f)
    {
        CurrentMontageType = EAnim_MontageType::None; // Custom montage
        CurrentMontage = Montage;
        bIsMontageActive = true;
        this->MontageLength = MontageLength;

        if (StartSection != NAME_None)
        {
            AnimInstance->Montage_JumpToSection(StartSection, CurrentMontage);
        }

        OnMontageStarted.Broadcast(EAnim_MontageType::None, CurrentMontage);
        return true;
    }

    return false;
}

void UAnim_MontageController::StopMontage(float BlendOutTime)
{
    if (!AnimInstance.IsValid() || !bIsMontageActive || !CurrentMontage)
    {
        return;
    }

    float ActualBlendOutTime = (BlendOutTime >= 0.0f) ? BlendOutTime : 0.25f;
    AnimInstance->Montage_Stop(ActualBlendOutTime, CurrentMontage);
}

void UAnim_MontageController::PauseMontage()
{
    if (AnimInstance.IsValid() && bIsMontageActive && CurrentMontage)
    {
        AnimInstance->Montage_Pause(CurrentMontage);
    }
}

void UAnim_MontageController::ResumeMontage()
{
    if (AnimInstance.IsValid() && bIsMontageActive && CurrentMontage)
    {
        AnimInstance->Montage_Resume(CurrentMontage);
    }
}

void UAnim_MontageController::SetMontagePosition(float Position)
{
    if (AnimInstance.IsValid() && bIsMontageActive && CurrentMontage)
    {
        AnimInstance->Montage_SetPosition(CurrentMontage, Position);
    }
}

void UAnim_MontageController::JumpToSection(FName SectionName)
{
    if (AnimInstance.IsValid() && bIsMontageActive && CurrentMontage)
    {
        AnimInstance->Montage_JumpToSection(SectionName, CurrentMontage);
    }
}

float UAnim_MontageController::GetMontagePlayRate() const
{
    if (AnimInstance.IsValid() && bIsMontageActive && CurrentMontage)
    {
        return AnimInstance->Montage_GetPlayRate(CurrentMontage);
    }
    return 0.0f;
}

FName UAnim_MontageController::GetCurrentSection() const
{
    if (AnimInstance.IsValid() && bIsMontageActive && CurrentMontage)
    {
        return AnimInstance->Montage_GetCurrentSection(CurrentMontage);
    }
    return NAME_None;
}

TArray<FName> UAnim_MontageController::GetMontageSections() const
{
    TArray<FName> Sections;
    if (CurrentMontage)
    {
        for (const FCompositeSection& Section : CurrentMontage->CompositeSections)
        {
            Sections.Add(Section.SectionName);
        }
    }
    return Sections;
}

// Survival Specific Montages
void UAnim_MontageController::PlayJumpMontage()
{
    PlayMontage(EAnim_MontageType::Jump);
}

void UAnim_MontageController::PlayInteractMontage()
{
    PlayMontage(EAnim_MontageType::Interact);
}

void UAnim_MontageController::PlayCraftMontage()
{
    PlayMontage(EAnim_MontageType::Craft);
}

void UAnim_MontageController::PlayGatherMontage()
{
    PlayMontage(EAnim_MontageType::Gather);
}

void UAnim_MontageController::PlayEatMontage()
{
    PlayMontage(EAnim_MontageType::Eat);
}

void UAnim_MontageController::PlayDrinkMontage()
{
    PlayMontage(EAnim_MontageType::Drink);
}

void UAnim_MontageController::PlayFearMontage()
{
    PlayMontage(EAnim_MontageType::Fear);
}

void UAnim_MontageController::PlayPainMontage()
{
    PlayMontage(EAnim_MontageType::Pain);
}

// Private Methods
void UAnim_MontageController::UpdateMontageState()
{
    if (!AnimInstance.IsValid())
    {
        return;
    }

    if (bIsMontageActive && CurrentMontage)
    {
        if (AnimInstance->Montage_IsActive(CurrentMontage))
        {
            MontagePosition = AnimInstance->Montage_GetPosition(CurrentMontage);
        }
        else
        {
            // Montage finished
            bIsMontageActive = false;
            CurrentMontage = nullptr;
            CurrentMontageType = EAnim_MontageType::None;
            MontagePosition = 0.0f;
            MontageLength = 0.0f;
        }
    }
}

void UAnim_MontageController::ProcessMontageQueue()
{
    if (!bIsMontageActive && MontageQueue.Num() > 0)
    {
        EAnim_MontageType NextMontage = MontageQueue[0];
        MontageQueue.RemoveAt(0);
        PlayMontage(NextMontage);
    }
}

void UAnim_MontageController::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == CurrentMontage)
    {
        UE_LOG(LogTemp, Log, TEXT("Montage blending out: %s (Interrupted: %s)"), 
               *Montage->GetName(), bInterrupted ? TEXT("Yes") : TEXT("No"));
    }
}

void UAnim_MontageController::OnMontageEnded_Internal(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == CurrentMontage)
    {
        EAnim_MontageType EndedType = CurrentMontageType;
        
        bIsMontageActive = false;
        CurrentMontage = nullptr;
        CurrentMontageType = EAnim_MontageType::None;
        MontagePosition = 0.0f;
        MontageLength = 0.0f;

        OnMontageEnded.Broadcast(EndedType, !bInterrupted);
        
        UE_LOG(LogTemp, Log, TEXT("Montage ended: %s (Completed: %s)"), 
               *Montage->GetName(), bInterrupted ? TEXT("No") : TEXT("Yes"));
    }
}

void UAnim_MontageController::CheckMovementInterrupt()
{
    if (!OwnerCharacter.IsValid())
    {
        return;
    }

    FVector CurrentLocation = OwnerCharacter->GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, LastLocation);
    float DeltaTime = GetWorld()->GetTimeSeconds() - LastMovementTime;
    
    if (DeltaTime > 0.0f)
    {
        float Speed = Distance / DeltaTime;
        if (Speed > MovementThreshold)
        {
            // Character is moving fast enough to interrupt montage
            StopMontage(0.2f);
            UE_LOG(LogTemp, Log, TEXT("Montage interrupted by movement (Speed: %.1f cm/s)"), Speed);
        }
    }

    LastLocation = CurrentLocation;
    LastMovementTime = GetWorld()->GetTimeSeconds();
}