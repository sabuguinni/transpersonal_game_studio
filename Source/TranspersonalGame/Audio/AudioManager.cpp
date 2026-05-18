#include "AudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"

void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    GlobalVolumeMultiplier = 1.0f;
    MaxDinosaurAudioDistance = 10000.0f;
    CurrentThreatLevel = EAudio_ThreatLevel::None;
    
    InitializeDinosaurSounds();
    LoadAudioAssets();
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager initialized successfully"));
}

void UAudioManager::Deinitialize()
{
    if (CurrentNarrationComponent && IsValid(CurrentNarrationComponent))
    {
        CurrentNarrationComponent->Stop();
        CurrentNarrationComponent = nullptr;
    }
    
    if (AmbienceComponent && IsValid(AmbienceComponent))
    {
        AmbienceComponent->Stop();
        AmbienceComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UAudioManager::InitializeDinosaurSounds()
{
    DinosaurSoundDatabase.Empty();
    
    // T-Rex sound configuration
    FAudio_DinosaurSoundData TRexData;
    TRexData.DinosaurType = EAudio_DinosaurType::TRex;
    TRexData.MaxAudibleDistance = 8000.0f;
    TRexData.VolumeMultiplier = 1.2f;
    DinosaurSoundDatabase.Add(TRexData);
    
    // Raptor sound configuration
    FAudio_DinosaurSoundData RaptorData;
    RaptorData.DinosaurType = EAudio_DinosaurType::Raptor;
    RaptorData.MaxAudibleDistance = 3000.0f;
    RaptorData.VolumeMultiplier = 0.8f;
    DinosaurSoundDatabase.Add(RaptorData);
    
    // Triceratops sound configuration
    FAudio_DinosaurSoundData TriceratopsData;
    TriceratopsData.DinosaurType = EAudio_DinosaurType::Triceratops;
    TriceratopsData.MaxAudibleDistance = 5000.0f;
    TriceratopsData.VolumeMultiplier = 1.0f;
    DinosaurSoundDatabase.Add(TriceratopsData);
    
    // Brachiosaurus sound configuration
    FAudio_DinosaurSoundData BrachiosaurusData;
    BrachiosaurusData.DinosaurType = EAudio_DinosaurType::Brachiosaurus;
    BrachiosaurusData.MaxAudibleDistance = 6000.0f;
    BrachiosaurusData.VolumeMultiplier = 1.1f;
    DinosaurSoundDatabase.Add(BrachiosaurusData);
}

void UAudioManager::LoadAudioAssets()
{
    // Load default sounds - these would be replaced with actual asset references
    // DefaultFootstepSound = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/Dinosaurs/DefaultFootstep"));
    // ScreenShakeRumbleSound = LoadObject<USoundCue>(nullptr, TEXT("/Game/Audio/Effects/ScreenShakeRumble"));
    
    UE_LOG(LogTemp, Log, TEXT("Audio assets loading initiated"));
}

void UAudioManager::PlayDinosaurFootstep(EAudio_DinosaurType DinosaurType, const FVector& Location, float VolumeScale)
{
    FAudio_DinosaurSoundData* SoundData = GetDinosaurSoundData(DinosaurType);
    if (!SoundData)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get player location for distance calculation
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(Location, PlayerLocation);
    
    if (Distance > SoundData->MaxAudibleDistance)
    {
        return; // Too far to hear
    }
    
    float VolumeMultiplier = CalculateVolumeByDistance(Location, PlayerLocation, SoundData->MaxAudibleDistance);
    VolumeMultiplier *= SoundData->VolumeMultiplier * VolumeScale * GlobalVolumeMultiplier;
    
    // Play footstep sound
    if (SoundData->FootstepSound.LoadSynchronous())
    {
        UGameplayStatics::PlaySoundAtLocation(World, SoundData->FootstepSound.Get(), Location, VolumeMultiplier);
    }
    else if (DefaultFootstepSound.LoadSynchronous())
    {
        UGameplayStatics::PlaySoundAtLocation(World, DefaultFootstepSound.Get(), Location, VolumeMultiplier);
    }
    
    // Trigger screen shake for heavy dinosaurs
    if (DinosaurType == EAudio_DinosaurType::TRex || DinosaurType == EAudio_DinosaurType::Brachiosaurus)
    {
        TriggerScreenShakeAudio(VolumeMultiplier * 0.5f, Location);
    }
}

void UAudioManager::PlayDinosaurRoar(EAudio_DinosaurType DinosaurType, const FVector& Location, float VolumeScale)
{
    FAudio_DinosaurSoundData* SoundData = GetDinosaurSoundData(DinosaurType);
    if (!SoundData)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    float VolumeMultiplier = CalculateVolumeByDistance(Location, PlayerLocation, SoundData->MaxAudibleDistance);
    VolumeMultiplier *= SoundData->VolumeMultiplier * VolumeScale * GlobalVolumeMultiplier;
    
    if (SoundData->RoarSound.LoadSynchronous())
    {
        UGameplayStatics::PlaySoundAtLocation(World, SoundData->RoarSound.Get(), Location, VolumeMultiplier);
    }
    
    // Update threat level based on dinosaur type and proximity
    float Distance = FVector::Dist(Location, PlayerLocation);
    if (Distance < 2000.0f)
    {
        UpdateThreatLevel(EAudio_ThreatLevel::Critical);
    }
    else if (Distance < 4000.0f)
    {
        UpdateThreatLevel(EAudio_ThreatLevel::High);
    }
}

void UAudioManager::PlayProximityWarning(EAudio_DinosaurType DinosaurType, const FVector& PlayerLocation, const FVector& DinosaurLocation)
{
    float Distance = FVector::Dist(PlayerLocation, DinosaurLocation);
    
    // Different warning distances for different dinosaurs
    float WarningDistance = 3000.0f;
    switch (DinosaurType)
    {
        case EAudio_DinosaurType::TRex:
            WarningDistance = 5000.0f;
            break;
        case EAudio_DinosaurType::Raptor:
            WarningDistance = 2000.0f;
            break;
        case EAudio_DinosaurType::Triceratops:
            WarningDistance = 3000.0f;
            break;
        case EAudio_DinosaurType::Brachiosaurus:
            WarningDistance = 4000.0f;
            break;
    }
    
    if (Distance <= WarningDistance)
    {
        // Play appropriate warning narration
        FString WarningClip;
        switch (DinosaurType)
        {
            case EAudio_DinosaurType::TRex:
                WarningClip = TEXT("TRexProximityWarning");
                break;
            case EAudio_DinosaurType::Raptor:
                WarningClip = TEXT("RaptorHuntWarning");
                break;
            default:
                WarningClip = TEXT("GenericDinosaurWarning");
                break;
        }
        
        PlayNarrationClip(WarningClip, false);
    }
}

void UAudioManager::SetBiomeAmbience(const FString& BiomeName, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Stop current ambience
    if (AmbienceComponent && IsValid(AmbienceComponent))
    {
        AmbienceComponent->FadeOut(2.0f, 0.0f);
    }
    
    // Load and play new ambience based on biome
    FString AmbiencePath = FString::Printf(TEXT("/Game/Audio/Ambience/%s_Ambience"), *BiomeName);
    
    // Create new ambience component
    AmbienceComponent = UGameplayStatics::SpawnSoundAtLocation(World, nullptr, Location);
    if (AmbienceComponent)
    {
        AmbienceComponent->SetVolumeMultiplier(0.6f * GlobalVolumeMultiplier);
        AmbienceComponent->FadeIn(3.0f, 1.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Biome ambience set to: %s"), *BiomeName);
}

void UAudioManager::UpdateThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel == NewThreatLevel)
    {
        return;
    }
    
    CurrentThreatLevel = NewThreatLevel;
    
    // Adjust ambience volume based on threat level
    if (AmbienceComponent && IsValid(AmbienceComponent))
    {
        float ThreatVolumeMultiplier = 1.0f;
        switch (CurrentThreatLevel)
        {
            case EAudio_ThreatLevel::None:
                ThreatVolumeMultiplier = 1.0f;
                break;
            case EAudio_ThreatLevel::Low:
                ThreatVolumeMultiplier = 0.8f;
                break;
            case EAudio_ThreatLevel::Medium:
                ThreatVolumeMultiplier = 0.6f;
                break;
            case EAudio_ThreatLevel::High:
                ThreatVolumeMultiplier = 0.4f;
                break;
            case EAudio_ThreatLevel::Critical:
                ThreatVolumeMultiplier = 0.2f;
                break;
        }
        
        AmbienceComponent->SetVolumeMultiplier(ThreatVolumeMultiplier * GlobalVolumeMultiplier);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Threat level updated to: %d"), (int32)CurrentThreatLevel);
}

void UAudioManager::TriggerScreenShakeAudio(float Intensity, const FVector& SourceLocation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Play rumble sound effect
    if (ScreenShakeRumbleSound.LoadSynchronous())
    {
        UGameplayStatics::PlaySoundAtLocation(World, ScreenShakeRumbleSound.Get(), SourceLocation, Intensity);
    }
    
    // Trigger camera shake for all players
    APlayerController* PC = World->GetFirstPlayerController();
    if (PC)
    {
        float ShakeIntensity = FMath::Clamp(Intensity, 0.1f, 2.0f);
        // PC->ClientStartCameraShake(ScreenShakeClass, ShakeIntensity);
    }
}

void UAudioManager::PlayNarrationClip(const FString& ClipName, bool bInterruptCurrent)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Stop current narration if interrupting
    if (bInterruptCurrent && CurrentNarrationComponent && IsValid(CurrentNarrationComponent))
    {
        CurrentNarrationComponent->Stop();
    }
    
    // Don't play if narration is already playing and not interrupting
    if (!bInterruptCurrent && CurrentNarrationComponent && IsValid(CurrentNarrationComponent) && CurrentNarrationComponent->IsPlaying())
    {
        return;
    }
    
    // Load narration sound
    FString NarrationPath = FString::Printf(TEXT("/Game/Audio/Narration/%s"), *ClipName);
    
    // Create narration component
    APlayerController* PC = World->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
        CurrentNarrationComponent = UGameplayStatics::SpawnSoundAtLocation(World, nullptr, PlayerLocation);
        
        if (CurrentNarrationComponent)
        {
            CurrentNarrationComponent->SetVolumeMultiplier(1.0f * GlobalVolumeMultiplier);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Playing narration clip: %s"), *ClipName);
}

void UAudioManager::StopNarration()
{
    if (CurrentNarrationComponent && IsValid(CurrentNarrationComponent))
    {
        CurrentNarrationComponent->FadeOut(1.0f, 0.0f);
        CurrentNarrationComponent = nullptr;
    }
}

FAudio_DinosaurSoundData* UAudioManager::GetDinosaurSoundData(EAudio_DinosaurType DinosaurType)
{
    for (FAudio_DinosaurSoundData& SoundData : DinosaurSoundDatabase)
    {
        if (SoundData.DinosaurType == DinosaurType)
        {
            return &SoundData;
        }
    }
    return nullptr;
}

float UAudioManager::CalculateVolumeByDistance(const FVector& SourceLocation, const FVector& ListenerLocation, float MaxDistance)
{
    float Distance = FVector::Dist(SourceLocation, ListenerLocation);
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Logarithmic falloff for more realistic audio distance
    float NormalizedDistance = Distance / MaxDistance;
    return FMath::Clamp(1.0f - (NormalizedDistance * NormalizedDistance), 0.0f, 1.0f);
}