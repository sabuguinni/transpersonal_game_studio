#include "Audio_PrehistoricSoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AAudio_PrehistoricSoundManager::AAudio_PrehistoricSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    RootComponent = AmbientAudioComponent;
    AmbientAudioComponent->bAutoActivate = false;

    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudio"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = false;

    ThreatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ThreatAudio"));
    ThreatAudioComponent->SetupAttachment(RootComponent);
    ThreatAudioComponent->bAutoActivate = false;

    // Initialize state
    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    CurrentThreatIntensity = 0.0f;
    
    // Initialize settings
    MasterVolume = 1.0f;
    AmbientVolume = 0.8f;
    MusicVolume = 0.6f;
    ThreatVolume = 0.9f;
    
    BiomeCheckInterval = 2.0f;
    ThreatCheckInterval = 0.5f;
    
    LastBiomeCheck = 0.0f;
    LastThreatCheck = 0.0f;
    bIsTransitioning = false;
}

void AAudio_PrehistoricSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioMaps();
    
    // Start with forest biome
    SetBiome(EAudio_BiomeType::Forest);
    SetThreatLevel(EAudio_ThreatLevel::Safe);
    
    UE_LOG(LogTemp, Warning, TEXT("Prehistoric Sound Manager initialized"));
}

void AAudio_PrehistoricSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check for biome changes
    if (CurrentTime - LastBiomeCheck >= BiomeCheckInterval)
    {
        UpdateBiomeAudio();
        LastBiomeCheck = CurrentTime;
    }
    
    // Check for threat level changes
    if (CurrentTime - LastThreatCheck >= ThreatCheckInterval)
    {
        UpdateThreatAudio();
        LastThreatCheck = CurrentTime;
    }
}

void AAudio_PrehistoricSoundManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome != CurrentBiome && !bIsTransitioning)
    {
        TransitionToNewBiome(NewBiome);
    }
}

void AAudio_PrehistoricSoundManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel, float Intensity)
{
    if (NewThreatLevel != CurrentThreatLevel || FMath::Abs(Intensity - CurrentThreatIntensity) > 0.1f)
    {
        TransitionToNewThreat(NewThreatLevel, Intensity);
    }
}

void AAudio_PrehistoricSoundManager::PlayDinosaurRoar(const FVector& Location, float Volume)
{
    // Play positional dinosaur roar sound
    if (GetWorld())
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            nullptr, // Would be loaded dinosaur roar sound
            Location,
            Volume * MasterVolume
        );
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur roar played at location: %s"), *Location.ToString());
    }
}

void AAudio_PrehistoricSoundManager::PlayFootstepSound(const FVector& Location, bool bIsLarge)
{
    // Play positional footstep sound
    if (GetWorld())
    {
        float VolumeMultiplier = bIsLarge ? 1.5f : 1.0f;
        
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            nullptr, // Would be loaded footstep sound
            Location,
            VolumeMultiplier * MasterVolume * 0.8f
        );
    }
}

void AAudio_PrehistoricSoundManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    // Update all audio component volumes
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * MasterVolume);
    }
    
    if (MusicAudioComponent)
    {
        MusicAudioComponent->SetVolumeMultiplier(MusicVolume * MasterVolume);
    }
    
    if (ThreatAudioComponent)
    {
        ThreatAudioComponent->SetVolumeMultiplier(ThreatVolume * MasterVolume);
    }
}

void AAudio_PrehistoricSoundManager::UpdateBiomeAudio()
{
    EAudio_BiomeType DetectedBiome = DetectCurrentBiome();
    SetBiome(DetectedBiome);
}

void AAudio_PrehistoricSoundManager::UpdateThreatAudio()
{
    float ThreatIntensity = 0.0f;
    EAudio_ThreatLevel DetectedThreat = DetectCurrentThreatLevel(ThreatIntensity);
    SetThreatLevel(DetectedThreat, ThreatIntensity);
}

void AAudio_PrehistoricSoundManager::TransitionToNewBiome(EAudio_BiomeType NewBiome)
{
    bIsTransitioning = true;
    CurrentBiome = NewBiome;
    
    // Get biome audio data
    if (BiomeAudioMap.Contains(NewBiome))
    {
        const FAudio_BiomeAudioData& BiomeData = BiomeAudioMap[NewBiome];
        
        // Fade out current ambient
        if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->FadeOut(BiomeData.FadeTime, 0.0f);
        }
        
        // Fade out current music
        if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
        {
            MusicAudioComponent->FadeOut(BiomeData.FadeTime, 0.0f);
        }
        
        // Start new audio after fade time
        FTimerHandle FadeTimer;
        GetWorld()->GetTimerManager().SetTimer(FadeTimer, [this, BiomeData]()
        {
            // Set new ambient sound
            if (BiomeData.AmbientSound.IsValid())
            {
                AmbientAudioComponent->SetSound(BiomeData.AmbientSound.LoadSynchronous());
                AmbientAudioComponent->SetVolumeMultiplier(BiomeData.BaseVolume * AmbientVolume * MasterVolume);
                AmbientAudioComponent->FadeIn(BiomeData.FadeTime);
            }
            
            // Set new music
            if (BiomeData.MusicTrack.IsValid())
            {
                MusicAudioComponent->SetSound(BiomeData.MusicTrack.LoadSynchronous());
                MusicAudioComponent->SetVolumeMultiplier(BiomeData.BaseVolume * MusicVolume * MasterVolume);
                MusicAudioComponent->FadeIn(BiomeData.FadeTime);
            }
            
            bIsTransitioning = false;
            
        }, BiomeData.FadeTime, false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Transitioning to biome: %d"), (int32)NewBiome);
}

void AAudio_PrehistoricSoundManager::TransitionToNewThreat(EAudio_ThreatLevel NewThreat, float Intensity)
{
    CurrentThreatLevel = NewThreat;
    CurrentThreatIntensity = Intensity;
    
    // Get threat audio data
    if (ThreatAudioMap.Contains(NewThreat))
    {
        const FAudio_ThreatAudioData& ThreatData = ThreatAudioMap[NewThreat];
        
        if (NewThreat == EAudio_ThreatLevel::Safe)
        {
            // Fade out threat audio
            if (ThreatAudioComponent && ThreatAudioComponent->IsPlaying())
            {
                ThreatAudioComponent->FadeOut(2.0f, 0.0f);
            }
        }
        else
        {
            // Play threat audio
            if (ThreatData.ThreatMusic.IsValid())
            {
                ThreatAudioComponent->SetSound(ThreatData.ThreatMusic.LoadSynchronous());
                float ThreatVolumeMult = ThreatData.IntensityMultiplier * Intensity * ThreatVolume * MasterVolume;
                ThreatAudioComponent->SetVolumeMultiplier(ThreatVolumeMult);
                ThreatAudioComponent->FadeIn(1.0f);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Threat level changed to: %d, Intensity: %f"), (int32)NewThreat, Intensity);
}

EAudio_BiomeType AAudio_PrehistoricSoundManager::DetectCurrentBiome()
{
    // Simple biome detection based on player location
    // In a real implementation, this would check terrain data, foliage density, etc.
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
        
        // Simple height-based biome detection
        if (PlayerLocation.Z > 500.0f)
        {
            return EAudio_BiomeType::Canyon;
        }
        else if (PlayerLocation.Z < 0.0f)
        {
            return EAudio_BiomeType::Cave;
        }
        else if (FMath::Abs(PlayerLocation.X) > 2000.0f)
        {
            return EAudio_BiomeType::Plains;
        }
        else
        {
            return EAudio_BiomeType::Forest;
        }
    }
    
    return CurrentBiome;
}

EAudio_ThreatLevel AAudio_PrehistoricSoundManager::DetectCurrentThreatLevel(float& OutIntensity)
{
    OutIntensity = 0.0f;
    
    // Simple threat detection based on nearby dinosaur actors
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return EAudio_ThreatLevel::Safe;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    // Find nearby threats (simplified - would check for actual dinosaur actors)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    float ClosestThreatDistance = 10000.0f;
    bool bThreatFound = false;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetActorLabel().Contains(TEXT("TRex")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance < ClosestThreatDistance)
            {
                ClosestThreatDistance = Distance;
                bThreatFound = true;
            }
        }
    }
    
    if (bThreatFound)
    {
        if (ClosestThreatDistance < 500.0f)
        {
            OutIntensity = 1.0f - (ClosestThreatDistance / 500.0f);
            return EAudio_ThreatLevel::Extreme;
        }
        else if (ClosestThreatDistance < 1000.0f)
        {
            OutIntensity = 1.0f - (ClosestThreatDistance / 1000.0f);
            return EAudio_ThreatLevel::Danger;
        }
        else if (ClosestThreatDistance < 2000.0f)
        {
            OutIntensity = 1.0f - (ClosestThreatDistance / 2000.0f);
            return EAudio_ThreatLevel::Caution;
        }
    }
    
    return EAudio_ThreatLevel::Safe;
}

void AAudio_PrehistoricSoundManager::InitializeAudioMaps()
{
    // Initialize biome audio data (would be loaded from data assets in production)
    BiomeAudioMap.Empty();
    
    // Forest biome
    FAudio_BiomeAudioData ForestData;
    ForestData.BaseVolume = 0.7f;
    ForestData.FadeTime = 3.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Forest, ForestData);
    
    // Plains biome
    FAudio_BiomeAudioData PlainsData;
    PlainsData.BaseVolume = 0.6f;
    PlainsData.FadeTime = 4.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Plains, PlainsData);
    
    // Canyon biome
    FAudio_BiomeAudioData CanyonData;
    CanyonData.BaseVolume = 0.8f;
    CanyonData.FadeTime = 2.5f;
    BiomeAudioMap.Add(EAudio_BiomeType::Canyon, CanyonData);
    
    // Initialize threat audio data
    ThreatAudioMap.Empty();
    
    // Safe level
    FAudio_ThreatAudioData SafeData;
    SafeData.IntensityMultiplier = 0.0f;
    ThreatAudioMap.Add(EAudio_ThreatLevel::Safe, SafeData);
    
    // Danger level
    FAudio_ThreatAudioData DangerData;
    DangerData.IntensityMultiplier = 1.2f;
    DangerData.ThreatRadius = 1000.0f;
    ThreatAudioMap.Add(EAudio_ThreatLevel::Danger, DangerData);
    
    // Extreme level
    FAudio_ThreatAudioData ExtremeData;
    ExtremeData.IntensityMultiplier = 1.8f;
    ExtremeData.ThreatRadius = 500.0f;
    ThreatAudioMap.Add(EAudio_ThreatLevel::Extreme, ExtremeData);
}