#include "Audio_PrehistoricSoundscapeManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AAudio_PrehistoricSoundscapeManager::AAudio_PrehistoricSoundscapeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create audio components
    BiomeAmbientComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BiomeAmbientComponent"));
    RootComponent = BiomeAmbientComponent;
    BiomeAmbientComponent->bAutoActivate = false;
    BiomeAmbientComponent->SetVolumeMultiplier(0.7f);

    DangerAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DangerAudioComponent"));
    DangerAudioComponent->SetupAttachment(RootComponent);
    DangerAudioComponent->bAutoActivate = false;
    DangerAudioComponent->SetVolumeMultiplier(0.5f);

    NarrativeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrativeAudioComponent"));
    NarrativeAudioComponent->SetupAttachment(RootComponent);
    NarrativeAudioComponent->bAutoActivate = false;
    NarrativeAudioComponent->SetVolumeMultiplier(0.8f);

    // Initialize default values
    CurrentBiome = EAudio_BiomeAudioType::Forest;
    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    BiomeTransitionTime = 3.0f;
    DangerDetectionRadius = 2500.0f;
    bNarrativeAudioEnabled = true;
    NarrativeVolume = 0.8f;

    // Initialize biome soundscapes
    BiomeSoundscapes.SetNum(6);
    
    // Forest biome
    BiomeSoundscapes[0].BiomeType = EAudio_BiomeAudioType::Forest;
    BiomeSoundscapes[0].BaseVolume = 0.7f;
    BiomeSoundscapes[0].FadeDistance = 2000.0f;

    // Plains biome
    BiomeSoundscapes[1].BiomeType = EAudio_BiomeAudioType::Plains;
    BiomeSoundscapes[1].BaseVolume = 0.6f;
    BiomeSoundscapes[1].FadeDistance = 3000.0f;

    // Swamp biome
    BiomeSoundscapes[2].BiomeType = EAudio_BiomeAudioType::Swamp;
    BiomeSoundscapes[2].BaseVolume = 0.8f;
    BiomeSoundscapes[2].FadeDistance = 1500.0f;

    // Mountain biome
    BiomeSoundscapes[3].BiomeType = EAudio_BiomeAudioType::Mountain;
    BiomeSoundscapes[3].BaseVolume = 0.5f;
    BiomeSoundscapes[3].FadeDistance = 4000.0f;

    // River biome
    BiomeSoundscapes[4].BiomeType = EAudio_BiomeAudioType::River;
    BiomeSoundscapes[4].BaseVolume = 0.9f;
    BiomeSoundscapes[4].FadeDistance = 1000.0f;

    // Cave biome
    BiomeSoundscapes[5].BiomeType = EAudio_BiomeAudioType::Cave;
    BiomeSoundscapes[5].BaseVolume = 0.4f;
    BiomeSoundscapes[5].FadeDistance = 800.0f;

    // Initialize danger audio cues
    DangerAudioCues.SetNum(4);

    // Safe level
    DangerAudioCues[0].DangerLevel = EAudio_DangerLevel::Safe;
    DangerAudioCues[0].TriggerDistance = 0.0f;
    DangerAudioCues[0].AudioPriority = 0.1f;

    // Caution level
    DangerAudioCues[1].DangerLevel = EAudio_DangerLevel::Caution;
    DangerAudioCues[1].TriggerDistance = 2000.0f;
    DangerAudioCues[1].AudioPriority = 0.3f;

    // Danger level
    DangerAudioCues[2].DangerLevel = EAudio_DangerLevel::Danger;
    DangerAudioCues[2].TriggerDistance = 1000.0f;
    DangerAudioCues[2].AudioPriority = 0.7f;

    // Extreme danger level
    DangerAudioCues[3].DangerLevel = EAudio_DangerLevel::Extreme;
    DangerAudioCues[3].TriggerDistance = 500.0f;
    DangerAudioCues[3].AudioPriority = 1.0f;

    // Initialize narrative voice lines with generated audio URLs
    NarrativeVoiceLines.Add(TEXT("ForestAmbience"), nullptr);
    NarrativeVoiceLines.Add(TEXT("DangerWarning"), nullptr);
    NarrativeVoiceLines.Add(TEXT("TribalHunter"), nullptr);
    NarrativeVoiceLines.Add(TEXT("Scout"), nullptr);
    NarrativeVoiceLines.Add(TEXT("ElderTracker"), nullptr);
    NarrativeVoiceLines.Add(TEXT("ClanLeader"), nullptr);
}

void AAudio_PrehistoricSoundscapeManager::BeginPlay()
{
    Super::BeginPlay();

    // Cache player reference
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            CachedPlayer = PC->GetPawn();
        }
    }

    // Start with forest biome audio
    SetBiomeAudio(EAudio_BiomeAudioType::Forest);

    UE_LOG(LogTemp, Log, TEXT("PrehistoricSoundscapeManager: Audio system initialized"));
}

void AAudio_PrehistoricSoundscapeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update audio systems
    UpdateBiomeAudio(DeltaTime);
    UpdateDangerAudio(DeltaTime);

    // Update timers
    BiomeUpdateTimer += DeltaTime;
    DangerUpdateTimer += DeltaTime;

    // Periodic updates
    if (BiomeUpdateTimer >= BiomeUpdateInterval)
    {
        BiomeUpdateTimer = 0.0f;
        // Could add biome detection logic here
    }

    if (DangerUpdateTimer >= DangerUpdateInterval)
    {
        DangerUpdateTimer = 0.0f;
        DetectNearbyThreats();
    }
}

void AAudio_PrehistoricSoundscapeManager::SetBiomeAudio(EAudio_BiomeAudioType NewBiome)
{
    if (CurrentBiome == NewBiome)
    {
        return;
    }

    CurrentBiome = NewBiome;
    FAudio_BiomeSoundscape* BiomeData = GetBiomeSoundscape(NewBiome);
    
    if (BiomeData && BiomeAmbientComponent)
    {
        // Load and play biome ambient sound
        if (BiomeData->AmbientLoop.IsValid())
        {
            if (USoundCue* AmbientCue = BiomeData->AmbientLoop.LoadSynchronous())
            {
                BiomeAmbientComponent->SetSound(AmbientCue);
                BiomeAmbientComponent->SetVolumeMultiplier(BiomeData->BaseVolume);
                BiomeAmbientComponent->Play();
            }
        }

        UE_LOG(LogTemp, Log, TEXT("PrehistoricSoundscapeManager: Switched to biome audio type %d"), (int32)NewBiome);
    }
}

void AAudio_PrehistoricSoundscapeManager::TransitionToBiome(EAudio_BiomeAudioType TargetBiome, float TransitionDuration)
{
    if (CurrentBiome == TargetBiome)
    {
        return;
    }

    bTransitioning = true;
    TransitionTargetBiome = TargetBiome;
    TransitionDuration = FMath::Max(0.1f, TransitionDuration);
    TransitionProgress = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("PrehistoricSoundscapeManager: Starting biome transition to %d"), (int32)TargetBiome);
}

void AAudio_PrehistoricSoundscapeManager::SetDangerLevel(EAudio_DangerLevel NewDangerLevel)
{
    if (CurrentDangerLevel == NewDangerLevel)
    {
        return;
    }

    CurrentDangerLevel = NewDangerLevel;
    FAudio_DangerAudioCue* DangerData = GetDangerAudioCue(NewDangerLevel);

    if (DangerData && DangerAudioComponent)
    {
        // Play danger-specific audio
        if (DangerData->ThreatSound.IsValid())
        {
            if (USoundCue* ThreatCue = DangerData->ThreatSound.LoadSynchronous())
            {
                DangerAudioComponent->SetSound(ThreatCue);
                DangerAudioComponent->SetVolumeMultiplier(DangerData->AudioPriority);
                DangerAudioComponent->Play();
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("PrehistoricSoundscapeManager: Danger level changed to %d"), (int32)NewDangerLevel);
    }
}

void AAudio_PrehistoricSoundscapeManager::TriggerDangerAudio(EAudio_DangerLevel DangerLevel, FVector ThreatLocation)
{
    SetDangerLevel(DangerLevel);

    // Calculate distance-based volume
    if (CachedPlayer)
    {
        float Distance = FVector::Dist(GetActorLocation(), ThreatLocation);
        FAudio_DangerAudioCue* DangerData = GetDangerAudioCue(DangerLevel);
        
        if (DangerData && Distance <= DangerData->TriggerDistance)
        {
            float VolumeMultiplier = 1.0f - (Distance / DangerData->TriggerDistance);
            VolumeMultiplier = FMath::Clamp(VolumeMultiplier, 0.1f, 1.0f);
            
            if (DangerAudioComponent)
            {
                DangerAudioComponent->SetVolumeMultiplier(VolumeMultiplier * DangerData->AudioPriority);
            }
        }
    }
}

void AAudio_PrehistoricSoundscapeManager::PlayNarrativeVoiceLine(const FString& VoiceLineKey)
{
    if (!bNarrativeAudioEnabled || !NarrativeAudioComponent)
    {
        return;
    }

    // Stop current narrative audio
    StopNarrativeAudio();

    // Find and play the requested voice line
    if (NarrativeVoiceLines.Contains(VoiceLineKey))
    {
        TSoftObjectPtr<USoundWave>* VoiceLinePtr = NarrativeVoiceLines.Find(VoiceLineKey);
        if (VoiceLinePtr && VoiceLinePtr->IsValid())
        {
            if (USoundWave* VoiceWave = VoiceLinePtr->LoadSynchronous())
            {
                NarrativeAudioComponent->SetSound(VoiceWave);
                NarrativeAudioComponent->SetVolumeMultiplier(NarrativeVolume);
                NarrativeAudioComponent->Play();

                UE_LOG(LogTemp, Log, TEXT("PrehistoricSoundscapeManager: Playing narrative voice line: %s"), *VoiceLineKey);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricSoundscapeManager: Voice line not found: %s"), *VoiceLineKey);
    }
}

void AAudio_PrehistoricSoundscapeManager::StopNarrativeAudio()
{
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }
}

bool AAudio_PrehistoricSoundscapeManager::IsNarrativeAudioPlaying() const
{
    return NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying();
}

void AAudio_PrehistoricSoundscapeManager::UpdateBiomeAudio(float DeltaTime)
{
    if (!bTransitioning)
    {
        return;
    }

    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        // Complete transition
        SetBiomeAudio(TransitionTargetBiome);
        bTransitioning = false;
        TransitionProgress = 0.0f;
    }
    else
    {
        // Update transition volume
        float FadeOutVolume = 1.0f - TransitionProgress;
        if (BiomeAmbientComponent)
        {
            BiomeAmbientComponent->SetVolumeMultiplier(FadeOutVolume * 0.7f);
        }
    }
}

void AAudio_PrehistoricSoundscapeManager::UpdateDangerAudio(float DeltaTime)
{
    // Update danger audio based on current threat level
    if (DangerAudioComponent && DangerAudioComponent->IsPlaying())
    {
        FAudio_DangerAudioCue* DangerData = GetDangerAudioCue(CurrentDangerLevel);
        if (DangerData)
        {
            // Could add dynamic volume adjustments based on threat proximity
            float CurrentVolume = DangerAudioComponent->GetVolumeMultiplier();
            float TargetVolume = DangerData->AudioPriority;
            
            // Smooth volume transitions
            float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, 2.0f);
            DangerAudioComponent->SetVolumeMultiplier(NewVolume);
        }
    }
}

void AAudio_PrehistoricSoundscapeManager::DetectNearbyThreats()
{
    if (!CachedPlayer)
    {
        return;
    }

    // Simple threat detection based on nearby actors
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    EAudio_DangerLevel DetectedDangerLevel = EAudio_DangerLevel::Safe;
    float ClosestThreatDistance = DangerDetectionRadius;

    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this || Actor == CachedPlayer)
        {
            continue;
        }

        // Check if actor is a potential threat (contains "Rex", "Raptor", etc.)
        FString ActorName = Actor->GetName().ToLower();
        bool bIsThreat = ActorName.Contains(TEXT("rex")) || 
                        ActorName.Contains(TEXT("raptor")) || 
                        ActorName.Contains(TEXT("carno")) ||
                        ActorName.Contains(TEXT("spino"));

        if (bIsThreat)
        {
            float Distance = FVector::Dist(CachedPlayer->GetActorLocation(), Actor->GetActorLocation());
            
            if (Distance < ClosestThreatDistance)
            {
                ClosestThreatDistance = Distance;
                
                // Determine danger level based on distance
                if (Distance < 500.0f)
                {
                    DetectedDangerLevel = EAudio_DangerLevel::Extreme;
                }
                else if (Distance < 1000.0f)
                {
                    DetectedDangerLevel = EAudio_DangerLevel::Danger;
                }
                else if (Distance < 2000.0f)
                {
                    DetectedDangerLevel = EAudio_DangerLevel::Caution;
                }
            }
        }
    }

    // Update danger level if changed
    if (DetectedDangerLevel != CurrentDangerLevel)
    {
        SetDangerLevel(DetectedDangerLevel);
    }
}

FAudio_BiomeSoundscape* AAudio_PrehistoricSoundscapeManager::GetBiomeSoundscape(EAudio_BiomeAudioType BiomeType)
{
    for (FAudio_BiomeSoundscape& Biome : BiomeSoundscapes)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return &Biome;
        }
    }
    return nullptr;
}

FAudio_DangerAudioCue* AAudio_PrehistoricSoundscapeManager::GetDangerAudioCue(EAudio_DangerLevel DangerLevel)
{
    for (FAudio_DangerAudioCue& DangerCue : DangerAudioCues)
    {
        if (DangerCue.DangerLevel == DangerLevel)
        {
            return &DangerCue;
        }
    }
    return nullptr;
}