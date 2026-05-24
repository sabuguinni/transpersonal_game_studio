#include "Narr_VoicelineController.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

UNarr_VoicelineController::UNarr_VoicelineController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    MasterVolume = 1.0f;
    bAllowInterruptions = true;
    DefaultCooldownTime = 30.0f;
}

void UNarr_VoicelineController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Narr_VoicelineController: Component initialized"));
    
    // Initialize with some default voicelines for testing
    FNarr_VoicelineData TestVoiceline;
    TestVoiceline.VoicelineID = TEXT("test_field_observation");
    TestVoiceline.VoicelineText = FText::FromString(TEXT("Field observation test voiceline"));
    TestVoiceline.VoicelineType = ENarr_VoicelineType::FieldObservation;
    TestVoiceline.Priority = ENarr_VoicelinePriority::Normal;
    TestVoiceline.CharacterName = TEXT("FieldResearcher");
    TestVoiceline.Duration = 10.0f;
    TestVoiceline.bCanInterrupt = true;
    TestVoiceline.CooldownTime = 30.0f;
    
    RegisterVoiceline(TestVoiceline);
}

void UNarr_VoicelineController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateVoicelinePlayback(DeltaTime);
    UpdateCooldowns(DeltaTime);
}

bool UNarr_VoicelineController::PlayVoiceline(const FString& VoicelineID)
{
    if (VoicelineID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_VoicelineController: Cannot play voiceline with empty ID"));
        return false;
    }
    
    // Check if voiceline exists
    FNarr_VoicelineData* VoicelineData = VoicelineDatabase.Find(VoicelineID);
    if (!VoicelineData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_VoicelineController: Voiceline '%s' not found in database"), *VoicelineID);
        return false;
    }
    
    // Check cooldown
    if (IsVoicelineOnCooldown(VoicelineID))
    {
        UE_LOG(LogTemp, Log, TEXT("Narr_VoicelineController: Voiceline '%s' is on cooldown"), *VoicelineID);
        return false;
    }
    
    // Check if we can interrupt current voiceline
    if (CurrentVoiceline.bIsPlaying)
    {
        if (!CanInterruptCurrentVoiceline(VoicelineData->Priority))
        {
            UE_LOG(LogTemp, Log, TEXT("Narr_VoicelineController: Cannot interrupt current voiceline with '%s'"), *VoicelineID);
            return false;
        }
        
        // Interrupt current voiceline
        OnVoicelineInterrupted.Broadcast(CurrentVoiceline.VoicelineData);
        StopCurrentVoiceline();
    }
    
    // Start new voiceline
    CurrentVoiceline.VoicelineData = *VoicelineData;
    CurrentVoiceline.StartTime = GetWorld()->GetTimeSeconds();
    CurrentVoiceline.bIsPlaying = true;
    
    // Create audio component if we have an audio asset
    if (VoicelineData->AudioAsset.IsValid())
    {
        USoundBase* SoundAsset = VoicelineData->AudioAsset.LoadSynchronous();
        if (SoundAsset)
        {
            CurrentVoiceline.AudioComponent = UGameplayStatics::SpawnSound2D(
                GetWorld(),
                SoundAsset,
                MasterVolume
            );
        }
    }
    
    // Start cooldown
    StartVoicelineCooldown(VoicelineID, VoicelineData->CooldownTime);
    
    // Broadcast event
    OnVoicelineStarted.Broadcast(*VoicelineData);
    
    UE_LOG(LogTemp, Log, TEXT("Narr_VoicelineController: Started voiceline '%s' by %s"), 
           *VoicelineID, *VoicelineData->CharacterName);
    
    return true;
}

bool UNarr_VoicelineController::PlayVoicelineByType(ENarr_VoicelineType VoicelineType, ENarr_VoicelinePriority MinPriority)
{
    FNarr_VoicelineData* SelectedVoiceline = FindVoicelineByType(VoicelineType, MinPriority);
    
    if (!SelectedVoiceline)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_VoicelineController: No available voiceline found for type %d with min priority %d"), 
               (int32)VoicelineType, (int32)MinPriority);
        return false;
    }
    
    return PlayVoiceline(SelectedVoiceline->VoicelineID);
}

void UNarr_VoicelineController::StopCurrentVoiceline()
{
    if (CurrentVoiceline.bIsPlaying)
    {
        CurrentVoiceline.bIsPlaying = false;
        
        if (CurrentVoiceline.AudioComponent && IsValid(CurrentVoiceline.AudioComponent))
        {
            CurrentVoiceline.AudioComponent->Stop();
            CurrentVoiceline.AudioComponent = nullptr;
        }
        
        OnVoicelineCompleted.Broadcast(CurrentVoiceline.VoicelineData);
        
        UE_LOG(LogTemp, Log, TEXT("Narr_VoicelineController: Stopped current voiceline"));
    }
}

void UNarr_VoicelineController::StopAllVoicelines()
{
    StopCurrentVoiceline();
    UE_LOG(LogTemp, Log, TEXT("Narr_VoicelineController: Stopped all voicelines"));
}

void UNarr_VoicelineController::RegisterVoiceline(const FNarr_VoicelineData& VoicelineData)
{
    if (VoicelineData.VoicelineID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_VoicelineController: Cannot register voiceline with empty ID"));
        return;
    }
    
    VoicelineDatabase.Add(VoicelineData.VoicelineID, VoicelineData);
    
    UE_LOG(LogTemp, Log, TEXT("Narr_VoicelineController: Registered voiceline '%s' by %s"), 
           *VoicelineData.VoicelineID, *VoicelineData.CharacterName);
}

bool UNarr_VoicelineController::IsVoicelineAvailable(const FString& VoicelineID) const
{
    return VoicelineDatabase.Contains(VoicelineID) && !IsVoicelineOnCooldown(VoicelineID);
}

bool UNarr_VoicelineController::IsAnyVoicelinePlaying() const
{
    return CurrentVoiceline.bIsPlaying;
}

FNarr_VoicelineData UNarr_VoicelineController::GetCurrentVoiceline() const
{
    return CurrentVoiceline.VoicelineData;
}

bool UNarr_VoicelineController::CanInterruptCurrentVoiceline(ENarr_VoicelinePriority NewPriority) const
{
    if (!CurrentVoiceline.bIsPlaying)
    {
        return true;
    }
    
    if (!bAllowInterruptions)
    {
        return false;
    }
    
    if (!CurrentVoiceline.VoicelineData.bCanInterrupt)
    {
        return false;
    }
    
    // Higher priority values can interrupt lower priority values
    return (int32)NewPriority > (int32)CurrentVoiceline.VoicelineData.Priority;
}

void UNarr_VoicelineController::SetVoicelineVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (CurrentVoiceline.AudioComponent && IsValid(CurrentVoiceline.AudioComponent))
    {
        CurrentVoiceline.AudioComponent->SetVolumeMultiplier(MasterVolume);
    }
}

void UNarr_VoicelineController::SetVoicelinePaused(bool bPaused)
{
    if (CurrentVoiceline.AudioComponent && IsValid(CurrentVoiceline.AudioComponent))
    {
        if (bPaused)
        {
            CurrentVoiceline.AudioComponent->SetPaused(true);
        }
        else
        {
            CurrentVoiceline.AudioComponent->SetPaused(false);
        }
    }
}

void UNarr_VoicelineController::LoadVoicelinesFromDataTable(UDataTable* VoicelineDataTable)
{
    if (!VoicelineDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Narr_VoicelineController: Cannot load from null data table"));
        return;
    }
    
    TArray<FNarr_VoicelineData*> AllRows;
    VoicelineDataTable->GetAllRows<FNarr_VoicelineData>(TEXT("LoadVoicelinesFromDataTable"), AllRows);
    
    int32 LoadedCount = 0;
    for (FNarr_VoicelineData* Row : AllRows)
    {
        if (Row && !Row->VoicelineID.IsEmpty())
        {
            RegisterVoiceline(*Row);
            LoadedCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Narr_VoicelineController: Loaded %d voicelines from data table"), LoadedCount);
}

void UNarr_VoicelineController::UpdateVoicelinePlayback(float DeltaTime)
{
    if (!CurrentVoiceline.bIsPlaying)
    {
        return;
    }
    
    // Check if audio component is still valid and playing
    if (CurrentVoiceline.AudioComponent && IsValid(CurrentVoiceline.AudioComponent))
    {
        if (!CurrentVoiceline.AudioComponent->IsPlaying())
        {
            OnAudioComponentFinished();
        }
    }
    else
    {
        // No audio component, use duration-based timing
        float ElapsedTime = GetWorld()->GetTimeSeconds() - CurrentVoiceline.StartTime;
        if (ElapsedTime >= CurrentVoiceline.VoicelineData.Duration)
        {
            OnAudioComponentFinished();
        }
    }
}

void UNarr_VoicelineController::UpdateCooldowns(float DeltaTime)
{
    TArray<FString> ExpiredCooldowns;
    
    for (auto& CooldownPair : VoicelineCooldowns)
    {
        CooldownPair.Value -= DeltaTime;
        if (CooldownPair.Value <= 0.0f)
        {
            ExpiredCooldowns.Add(CooldownPair.Key);
        }
    }
    
    // Remove expired cooldowns
    for (const FString& ExpiredID : ExpiredCooldowns)
    {
        VoicelineCooldowns.Remove(ExpiredID);
    }
}

bool UNarr_VoicelineController::IsVoicelineOnCooldown(const FString& VoicelineID) const
{
    const float* CooldownTime = VoicelineCooldowns.Find(VoicelineID);
    return CooldownTime && *CooldownTime > 0.0f;
}

void UNarr_VoicelineController::StartVoicelineCooldown(const FString& VoicelineID, float CooldownDuration)
{
    VoicelineCooldowns.Add(VoicelineID, CooldownDuration);
}

void UNarr_VoicelineController::OnAudioComponentFinished()
{
    if (CurrentVoiceline.bIsPlaying)
    {
        FNarr_VoicelineData CompletedVoiceline = CurrentVoiceline.VoicelineData;
        CurrentVoiceline.bIsPlaying = false;
        CurrentVoiceline.AudioComponent = nullptr;
        
        OnVoicelineCompleted.Broadcast(CompletedVoiceline);
        
        UE_LOG(LogTemp, Log, TEXT("Narr_VoicelineController: Completed voiceline '%s'"), 
               *CompletedVoiceline.VoicelineID);
    }
}

FNarr_VoicelineData* UNarr_VoicelineController::FindVoicelineByType(ENarr_VoicelineType VoicelineType, ENarr_VoicelinePriority MinPriority)
{
    FNarr_VoicelineData* BestMatch = nullptr;
    int32 HighestPriority = (int32)MinPriority - 1;
    
    for (auto& VoicelinePair : VoicelineDatabase)
    {
        FNarr_VoicelineData& VoicelineData = VoicelinePair.Value;
        
        // Check type match
        if (VoicelineData.VoicelineType != VoicelineType)
        {
            continue;
        }
        
        // Check minimum priority
        if ((int32)VoicelineData.Priority < (int32)MinPriority)
        {
            continue;
        }
        
        // Check cooldown
        if (IsVoicelineOnCooldown(VoicelineData.VoicelineID))
        {
            continue;
        }
        
        // Find highest priority available
        if ((int32)VoicelineData.Priority > HighestPriority)
        {
            BestMatch = &VoicelineData;
            HighestPriority = (int32)VoicelineData.Priority;
        }
    }
    
    return BestMatch;
}