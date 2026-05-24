#include "Anim_AnimationMontageManager.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"

UAnim_AnimationMontageManager::UAnim_AnimationMontageManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize settings
    DefaultBlendTime = 0.25f;
    bAutoProcessQueue = true;
    QueueProcessInterval = 0.1f;
    
    // Initialize state
    bIsMontageActive = false;
    LastQueueProcessTime = 0.0f;
    CachedAnimInstance = nullptr;
    CachedMeshComponent = nullptr;
}

void UAnim_AnimationMontageManager::BeginPlay()
{
    Super::BeginPlay();
    CacheAnimationComponents();
}

void UAnim_AnimationMontageManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoProcessQueue)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastQueueProcessTime >= QueueProcessInterval)
        {
            ProcessMontageQueue();
            LastQueueProcessTime = CurrentTime;
        }
    }
}

bool UAnim_AnimationMontageManager::PlayMontage(const FAnim_MontageData& MontageData, bool bForcePlay)
{
    return InternalPlayMontage(MontageData, bForcePlay);
}

bool UAnim_AnimationMontageManager::PlayMontageByID(const FString& MontageID, bool bForcePlay)
{
    if (FAnim_MontageData* FoundMontage = RegisteredMontages.Find(MontageID))
    {
        return InternalPlayMontage(*FoundMontage, bForcePlay);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Montage with ID '%s' not found in registered montages"), *MontageID);
    return false;
}

void UAnim_AnimationMontageManager::StopMontage(float BlendOutTime)
{
    if (!CachedAnimInstance || !bIsMontageActive)
    {
        return;
    }
    
    if (CurrentMontageData.Montage)
    {
        CachedAnimInstance->Montage_Stop(BlendOutTime, CurrentMontageData.Montage);
        BroadcastMontageEvent(CurrentMontageData.MontageID, CurrentMontageData.MontageType, OnMontageInterrupted);
    }
    
    bIsMontageActive = false;
    CurrentMontageData = FAnim_MontageData();
}

void UAnim_AnimationMontageManager::StopMontageByType(EAnim_MontageType MontageType, float BlendOutTime)
{
    if (bIsMontageActive && CurrentMontageData.MontageType == MontageType)
    {
        StopMontage(BlendOutTime);
    }
}

void UAnim_AnimationMontageManager::PauseMontage()
{
    if (CachedAnimInstance && bIsMontageActive && CurrentMontageData.Montage)
    {
        CachedAnimInstance->Montage_Pause(CurrentMontageData.Montage);
    }
}

void UAnim_AnimationMontageManager::ResumeMontage()
{
    if (CachedAnimInstance && bIsMontageActive && CurrentMontageData.Montage)
    {
        CachedAnimInstance->Montage_Resume(CurrentMontageData.Montage);
    }
}

void UAnim_AnimationMontageManager::QueueMontage(const FAnim_MontageData& MontageData)
{
    FAnim_MontageQueueEntry QueueEntry;
    QueueEntry.MontageData = MontageData;
    QueueEntry.QueueTime = GetWorld()->GetTimeSeconds();
    QueueEntry.bAutoPlay = true;
    
    MontageQueue.Add(QueueEntry);
}

void UAnim_AnimationMontageManager::QueueMontageByID(const FString& MontageID)
{
    if (FAnim_MontageData* FoundMontage = RegisteredMontages.Find(MontageID))
    {
        QueueMontage(*FoundMontage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot queue montage with ID '%s' - not found in registered montages"), *MontageID);
    }
}

void UAnim_AnimationMontageManager::ClearMontageQueue()
{
    MontageQueue.Empty();
}

void UAnim_AnimationMontageManager::RemoveFromQueue(const FString& MontageID)
{
    MontageQueue.RemoveAll([&MontageID](const FAnim_MontageQueueEntry& Entry)
    {
        return Entry.MontageData.MontageID == MontageID;
    });
}

void UAnim_AnimationMontageManager::RegisterMontage(const FAnim_MontageData& MontageData)
{
    if (!MontageData.MontageID.IsEmpty() && MontageData.Montage)
    {
        RegisteredMontages.Add(MontageData.MontageID, MontageData);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register montage - invalid ID or null montage asset"));
    }
}

void UAnim_AnimationMontageManager::UnregisterMontage(const FString& MontageID)
{
    RegisteredMontages.Remove(MontageID);
}

void UAnim_AnimationMontageManager::ClearAllMontages()
{
    RegisteredMontages.Empty();
}

bool UAnim_AnimationMontageManager::IsPlayingMontage() const
{
    return bIsMontageActive && CachedAnimInstance && CachedAnimInstance->IsAnyMontagePlaying();
}

bool UAnim_AnimationMontageManager::IsPlayingMontageOfType(EAnim_MontageType MontageType) const
{
    return bIsMontageActive && CurrentMontageData.MontageType == MontageType;
}

FString UAnim_AnimationMontageManager::GetCurrentMontageID() const
{
    return bIsMontageActive ? CurrentMontageData.MontageID : TEXT("");
}

EAnim_MontageType UAnim_AnimationMontageManager::GetCurrentMontageType() const
{
    return bIsMontageActive ? CurrentMontageData.MontageType : EAnim_MontageType::None;
}

float UAnim_AnimationMontageManager::GetMontagePosition() const
{
    if (!CachedAnimInstance || !bIsMontageActive || !CurrentMontageData.Montage)
    {
        return 0.0f;
    }
    
    return CachedAnimInstance->Montage_GetPosition(CurrentMontageData.Montage);
}

float UAnim_AnimationMontageManager::GetMontageLength() const
{
    if (!CurrentMontageData.Montage)
    {
        return 0.0f;
    }
    
    return CurrentMontageData.Montage->GetPlayLength();
}

bool UAnim_AnimationMontageManager::CanPlayMontage(const FAnim_MontageData& MontageData) const
{
    if (!MontageData.Montage)
    {
        return false;
    }
    
    if (!bIsMontageActive)
    {
        return true;
    }
    
    return CanInterruptCurrentMontage(MontageData.Priority);
}

bool UAnim_AnimationMontageManager::CanInterruptCurrentMontage(EAnim_MontagePriority NewPriority) const
{
    if (!bIsMontageActive)
    {
        return true;
    }
    
    if (!CurrentMontageData.bCanBeInterrupted)
    {
        return NewPriority == EAnim_MontagePriority::Override;
    }
    
    return static_cast<uint8>(NewPriority) >= static_cast<uint8>(CurrentMontageData.Priority);
}

void UAnim_AnimationMontageManager::SetMontageInterruptible(bool bCanInterrupt)
{
    if (bIsMontageActive)
    {
        CurrentMontageData.bCanBeInterrupted = bCanInterrupt;
    }
}

void UAnim_AnimationMontageManager::CacheAnimationComponents()
{
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        CachedMeshComponent = OwnerCharacter->GetMesh();
        if (CachedMeshComponent)
        {
            CachedAnimInstance = CachedMeshComponent->GetAnimInstance();
        }
    }
}

void UAnim_AnimationMontageManager::ProcessMontageQueue()
{
    if (MontageQueue.Num() == 0 || bIsMontageActive)
    {
        return;
    }
    
    PlayNextInQueue();
}

void UAnim_AnimationMontageManager::PlayNextInQueue()
{
    if (MontageQueue.Num() == 0)
    {
        return;
    }
    
    FAnim_MontageQueueEntry NextEntry = MontageQueue[0];
    MontageQueue.RemoveAt(0);
    
    if (NextEntry.bAutoPlay)
    {
        InternalPlayMontage(NextEntry.MontageData, false);
    }
}

bool UAnim_AnimationMontageManager::InternalPlayMontage(const FAnim_MontageData& MontageData, bool bForcePlay)
{
    if (!CachedAnimInstance || !MontageData.Montage)
    {
        return false;
    }
    
    // Check if we can play this montage
    if (!bForcePlay && !CanPlayMontage(MontageData))
    {
        return false;
    }
    
    // Stop current montage if playing
    if (bIsMontageActive)
    {
        StopMontage(MontageData.BlendInTime);
    }
    
    // Play the new montage
    float MontageLength = CachedAnimInstance->Montage_Play(
        MontageData.Montage,
        MontageData.PlayRate,
        EMontagePlayReturnType::MontageLength,
        0.0f,
        true
    );
    
    if (MontageLength > 0.0f)
    {
        CurrentMontageData = MontageData;
        bIsMontageActive = true;
        
        // Bind montage events
        FOnMontageBlendingOutStarted BlendingOutDelegate;
        BlendingOutDelegate.BindUObject(this, &UAnim_AnimationMontageManager::OnMontageBlendingOut);
        CachedAnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageData.Montage);
        
        FOnMontageEnded EndedDelegate;
        EndedDelegate.BindUObject(this, &UAnim_AnimationMontageManager::OnMontageEnded_Internal);
        CachedAnimInstance->Montage_SetEndDelegate(EndedDelegate, MontageData.Montage);
        
        BroadcastMontageEvent(MontageData.MontageID, MontageData.MontageType, OnMontageStarted);
        return true;
    }
    
    return false;
}

void UAnim_AnimationMontageManager::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (bIsMontageActive && CurrentMontageData.Montage == Montage)
    {
        if (bInterrupted)
        {
            BroadcastMontageEvent(CurrentMontageData.MontageID, CurrentMontageData.MontageType, OnMontageInterrupted);
        }
    }
}

void UAnim_AnimationMontageManager::OnMontageEnded_Internal(UAnimMontage* Montage, bool bInterrupted)
{
    if (bIsMontageActive && CurrentMontageData.Montage == Montage)
    {
        BroadcastMontageEvent(CurrentMontageData.MontageID, CurrentMontageData.MontageType, OnMontageEnded);
        
        bIsMontageActive = false;
        CurrentMontageData = FAnim_MontageData();
        
        // Process next in queue if available
        if (bAutoProcessQueue)
        {
            PlayNextInQueue();
        }
    }
}

void UAnim_AnimationMontageManager::BroadcastMontageEvent(const FString& MontageID, EAnim_MontageType MontageType, FAnim_OnMontageEvent& Event)
{
    if (Event.IsBound())
    {
        Event.Broadcast(MontageID, MontageType);
    }
}