#include "Audio_AdaptiveAudioManager.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

AAudio_AdaptiveAudioManager::AAudio_AdaptiveAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    
    MusicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicAudioComponent"));
    MusicAudioComponent->SetupAttachment(RootComponent);
    MusicAudioComponent->bAutoActivate = false;
    
    ProximityAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ProximityAudioComponent"));
    ProximityAudioComponent->SetupAttachment(RootComponent);
    ProximityAudioComponent->bAutoActivate = false;
    
    // Initialize default values
    AudioUpdateInterval = 0.5f;
    BiomeTransitionRadius = 500.0f;
    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    DistanceToNearestThreat = 10000.0f;
    AudioUpdateTimer = 0.0f;
    PlayerPawn = nullptr;
    
    // Initialize biome audio data
    FAudio_BiomeAudioData ForestData;
    ForestData.BaseVolume = 0.6f;
    ForestData.FadeTime = 3.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Forest, ForestData);
    
    FAudio_BiomeAudioData CanyonData;
    CanyonData.BaseVolume = 0.5f;
    CanyonData.FadeTime = 2.5f;
    BiomeAudioMap.Add(EAudio_BiomeType::Canyon, CanyonData);
    
    FAudio_BiomeAudioData PlainsData;
    PlainsData.BaseVolume = 0.7f;
    PlainsData.FadeTime = 2.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Plains, PlainsData);
    
    // Initialize dinosaur audio data
    FAudio_ProximityAudioData TRexData;
    TRexData.TriggerDistance = 2000.0f;
    TRexData.MaxVolume = 1.2f;
    DinosaurAudioMap.Add(TEXT("TRex"), TRexData);
    
    FAudio_ProximityAudioData RaptorData;
    RaptorData.TriggerDistance = 800.0f;
    RaptorData.MaxVolume = 0.8f;
    DinosaurAudioMap.Add(TEXT("Raptor"), RaptorData);
    
    FAudio_ProximityAudioData BrachiosaurusData;
    BrachiosaurusData.TriggerDistance = 1500.0f;
    BrachiosaurusData.MaxVolume = 1.0f;
    DinosaurAudioMap.Add(TEXT("Brachiosaurus"), BrachiosaurusData);
}

void AAudio_AdaptiveAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player pawn reference
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
        }
    }
    
    // Start with forest biome
    SetBiome(EAudio_BiomeType::Forest);
    SetThreatLevel(EAudio_ThreatLevel::Safe);
    
    // Set up timer for audio updates
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(
            TimerHandle,
            this,
            &AAudio_AdaptiveAudioManager::UpdateAudioSystem,
            AudioUpdateInterval,
            true
        );
    }
}

void AAudio_AdaptiveAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    AudioUpdateTimer += DeltaTime;
    if (AudioUpdateTimer >= AudioUpdateInterval)
    {
        AudioUpdateTimer = 0.0f;
        UpdateProximityAudio();
    }
}

void AAudio_AdaptiveAudioManager::SetBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        EAudio_BiomeType OldBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        
        TransitionBiomeAudio(OldBiome, NewBiome);
        OnBiomeChanged(NewBiome);
    }
}

void AAudio_AdaptiveAudioManager::SetThreatLevel(EAudio_ThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel != NewThreatLevel)
    {
        CurrentThreatLevel = NewThreatLevel;
        OnThreatLevelChanged(NewThreatLevel);
        
        // Adjust music intensity based on threat level
        float VolumeMultiplier = 1.0f;
        switch (NewThreatLevel)
        {
            case EAudio_ThreatLevel::Safe:
                VolumeMultiplier = 0.6f;
                break;
            case EAudio_ThreatLevel::Caution:
                VolumeMultiplier = 0.8f;
                break;
            case EAudio_ThreatLevel::Danger:
                VolumeMultiplier = 1.0f;
                break;
            case EAudio_ThreatLevel::Combat:
                VolumeMultiplier = 1.2f;
                break;
        }
        
        if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
        {
            MusicAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
        }
    }
}

void AAudio_AdaptiveAudioManager::TriggerDinosaurAudio(const FString& DinosaurType, float Distance)
{
    if (FAudio_ProximityAudioData* AudioData = DinosaurAudioMap.Find(DinosaurType))
    {
        if (Distance <= AudioData->TriggerDistance)
        {
            float VolumeScale = 1.0f - (Distance / AudioData->TriggerDistance);
            VolumeScale = FMath::Clamp(VolumeScale, 0.1f, 1.0f);
            
            if (ProximityAudioComponent && AudioData->ApproachSound.LoadSynchronous())
            {
                ProximityAudioComponent->SetSound(AudioData->ApproachSound.LoadSynchronous());
                ProximityAudioComponent->SetVolumeMultiplier(AudioData->MaxVolume * VolumeScale);
                
                if (!ProximityAudioComponent->IsPlaying())
                {
                    ProximityAudioComponent->Play();
                }
            }
            
            // Update threat level based on proximity
            EAudio_ThreatLevel NewThreatLevel = EAudio_ThreatLevel::Safe;
            if (Distance < AudioData->TriggerDistance * 0.3f)
            {
                NewThreatLevel = EAudio_ThreatLevel::Combat;
            }
            else if (Distance < AudioData->TriggerDistance * 0.6f)
            {
                NewThreatLevel = EAudio_ThreatLevel::Danger;
            }
            else
            {
                NewThreatLevel = EAudio_ThreatLevel::Caution;
            }
            
            SetThreatLevel(NewThreatLevel);
        }
    }
}

void AAudio_AdaptiveAudioManager::UpdateProximityAudio()
{
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    DistanceToNearestThreat = 10000.0f;
    
    // Find all dinosaur actors and calculate distances
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("TRex")))
            {
                float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                if (Distance < DistanceToNearestThreat)
                {
                    DistanceToNearestThreat = Distance;
                    TriggerDinosaurAudio(TEXT("TRex"), Distance);
                }
            }
            else if (Actor && Actor->GetName().Contains(TEXT("Raptor")))
            {
                float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                if (Distance < DistanceToNearestThreat)
                {
                    DistanceToNearestThreat = Distance;
                    TriggerDinosaurAudio(TEXT("Raptor"), Distance);
                }
            }
            else if (Actor && Actor->GetName().Contains(TEXT("Brachiosaurus")))
            {
                float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                TriggerDinosaurAudio(TEXT("Brachiosaurus"), Distance);
            }
        }
    }
}

void AAudio_AdaptiveAudioManager::UpdateAudioSystem()
{
    if (!PlayerPawn)
    {
        return;
    }
    
    // Detect current biome based on player location
    EAudio_BiomeType DetectedBiome = DetectCurrentBiome();
    SetBiome(DetectedBiome);
    
    // Calculate threat level based on nearby dangers
    EAudio_ThreatLevel CalculatedThreat = CalculateThreatLevel();
    SetThreatLevel(CalculatedThreat);
}

EAudio_BiomeType AAudio_AdaptiveAudioManager::DetectCurrentBiome()
{
    if (!PlayerPawn)
    {
        return EAudio_BiomeType::Forest;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Simple biome detection based on location
    // This would be expanded with proper biome data
    if (PlayerLocation.X > 1500.0f)
    {
        return EAudio_BiomeType::Canyon;
    }
    else if (PlayerLocation.Y > 1500.0f || PlayerLocation.Y < -1500.0f)
    {
        return EAudio_BiomeType::Plains;
    }
    else if (PlayerLocation.Z > 800.0f)
    {
        return EAudio_BiomeType::Mountain;
    }
    else if (PlayerLocation.Z < -200.0f)
    {
        return EAudio_BiomeType::Swamp;
    }
    
    return EAudio_BiomeType::Forest;
}

EAudio_ThreatLevel AAudio_AdaptiveAudioManager::CalculateThreatLevel()
{
    if (DistanceToNearestThreat < 500.0f)
    {
        return EAudio_ThreatLevel::Combat;
    }
    else if (DistanceToNearestThreat < 1000.0f)
    {
        return EAudio_ThreatLevel::Danger;
    }
    else if (DistanceToNearestThreat < 2000.0f)
    {
        return EAudio_ThreatLevel::Caution;
    }
    
    return EAudio_ThreatLevel::Safe;
}

void AAudio_AdaptiveAudioManager::TransitionBiomeAudio(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome)
{
    if (FAudio_BiomeAudioData* NewBiomeData = BiomeAudioMap.Find(ToBiome))
    {
        // Fade out current ambient audio
        if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->FadeOut(NewBiomeData->FadeTime, 0.0f);
        }
        
        // Fade out current music
        if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
        {
            MusicAudioComponent->FadeOut(NewBiomeData->FadeTime, 0.0f);
        }
        
        // Start new ambient audio after fade time
        if (UWorld* World = GetWorld())
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(
                TimerHandle,
                [this, NewBiomeData]()
                {
                    if (AmbientAudioComponent && NewBiomeData->AmbientSound.LoadSynchronous())
                    {
                        AmbientAudioComponent->SetSound(NewBiomeData->AmbientSound.LoadSynchronous());
                        AmbientAudioComponent->FadeIn(NewBiomeData->FadeTime, NewBiomeData->BaseVolume);
                    }
                    
                    if (MusicAudioComponent && NewBiomeData->MusicTrack.LoadSynchronous())
                    {
                        MusicAudioComponent->SetSound(NewBiomeData->MusicTrack.LoadSynchronous());
                        MusicAudioComponent->FadeIn(NewBiomeData->FadeTime, NewBiomeData->BaseVolume * 0.8f);
                    }
                },
                NewBiomeData->FadeTime,
                false
            );
        }
    }
}