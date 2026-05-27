#include "Audio_AdaptiveMusicSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UAudio_AdaptiveMusicSystem::UAudio_AdaptiveMusicSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentMusicState = EAudio_MusicState::Exploration;
    CurrentBiome = EAudio_BiomeType::Savana;
    bIsTransitioning = false;
    TransitionTimer = 0.0f;
    DangerTimer = 0.0f;
    PreviousState = EAudio_MusicState::Exploration;
    CurrentConfig = nullptr;
    TargetConfig = nullptr;
}

void UAudio_AdaptiveMusicSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Create audio components
    MusicAudioComponent = NewObject<UAudioComponent>(this);
    if (MusicAudioComponent)
    {
        MusicAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        MusicAudioComponent->RegisterComponent();
    }
    
    AmbientAudioComponent = NewObject<UAudioComponent>(this);
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        AmbientAudioComponent->RegisterComponent();
    }
    
    NarrativeAudioComponent = NewObject<UAudioComponent>(this);
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        NarrativeAudioComponent->RegisterComponent();
    }
    
    InitializeMusicConfigs();
    InitializeBiomeConfigs();
    
    // Start with exploration music
    PlayMusicForState(CurrentMusicState);
    PlayAmbientForBiome(CurrentBiome);
}

void UAudio_AdaptiveMusicSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsTransitioning)
    {
        UpdateTransition(DeltaTime);
    }
    
    // Update danger timer
    if (DangerTimer > 0.0f)
    {
        DangerTimer -= DeltaTime;
        if (DangerTimer <= 0.0f && CurrentMusicState == EAudio_MusicState::Danger)
        {
            SetMusicState(EAudio_MusicState::Exploration);
        }
    }
    
    CheckForDangers();
}

void UAudio_AdaptiveMusicSystem::SetMusicState(EAudio_MusicState NewState)
{
    if (NewState == CurrentMusicState && !bIsTransitioning)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio: Transitioning from %d to %d"), 
        (int32)CurrentMusicState, (int32)NewState);
    
    PreviousState = CurrentMusicState;
    CurrentMusicState = NewState;
    TransitionToState(NewState);
}

void UAudio_AdaptiveMusicSystem::SetBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome)
    {
        return;
    }
    
    CurrentBiome = NewBiome;
    PlayAmbientForBiome(NewBiome);
    
    UE_LOG(LogTemp, Log, TEXT("Audio: Biome changed to %d"), (int32)NewBiome);
}

void UAudio_AdaptiveMusicSystem::PlayNarrativeAudio(const FString& AudioURL, float Volume)
{
    if (!NarrativeAudioComponent)
    {
        return;
    }
    
    // Stop current narrative audio
    StopNarrativeAudio();
    
    // For now, log the URL - in full implementation this would stream from URL
    UE_LOG(LogTemp, Log, TEXT("Audio: Playing narrative audio from URL: %s"), *AudioURL);
    
    // Set to narrative state temporarily
    SetMusicState(EAudio_MusicState::Narrative);
}

void UAudio_AdaptiveMusicSystem::StopNarrativeAudio()
{
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->FadeOut(1.0f, 0.0f);
    }
}

void UAudio_AdaptiveMusicSystem::OnDinosaurNearby(float Distance, const FString& DinosaurType)
{
    if (Distance < DangerDetectionRadius)
    {
        SetMusicState(EAudio_MusicState::Danger);
        DangerTimer = DangerStateTimeout;
        
        UE_LOG(LogTemp, Warning, TEXT("Audio: Dinosaur %s detected at distance %f"), 
            *DinosaurType, Distance);
    }
}

void UAudio_AdaptiveMusicSystem::OnPlayerHealthLow(float HealthPercentage)
{
    if (HealthPercentage < 0.3f)
    {
        SetMusicState(EAudio_MusicState::Survival);
        UE_LOG(LogTemp, Warning, TEXT("Audio: Player health low: %f%%"), HealthPercentage * 100.0f);
    }
}

void UAudio_AdaptiveMusicSystem::OnCombatStart()
{
    SetMusicState(EAudio_MusicState::Combat);
    UE_LOG(LogTemp, Log, TEXT("Audio: Combat started"));
}

void UAudio_AdaptiveMusicSystem::OnCombatEnd()
{
    SetMusicState(EAudio_MusicState::Exploration);
    UE_LOG(LogTemp, Log, TEXT("Audio: Combat ended"));
}

void UAudio_AdaptiveMusicSystem::InitializeMusicConfigs()
{
    // Exploration music config
    FAudio_MusicConfig ExplorationConfig;
    ExplorationConfig.Volume = 0.6f;
    ExplorationConfig.FadeInTime = 3.0f;
    ExplorationConfig.FadeOutTime = 2.0f;
    MusicConfigs.Add(EAudio_MusicState::Exploration, ExplorationConfig);
    
    // Danger music config
    FAudio_MusicConfig DangerConfig;
    DangerConfig.Volume = 0.8f;
    DangerConfig.FadeInTime = 1.0f;
    DangerConfig.FadeOutTime = 2.0f;
    MusicConfigs.Add(EAudio_MusicState::Danger, DangerConfig);
    
    // Combat music config
    FAudio_MusicConfig CombatConfig;
    CombatConfig.Volume = 0.9f;
    CombatConfig.FadeInTime = 0.5f;
    CombatConfig.FadeOutTime = 1.0f;
    MusicConfigs.Add(EAudio_MusicState::Combat, CombatConfig);
    
    // Narrative music config
    FAudio_MusicConfig NarrativeConfig;
    NarrativeConfig.Volume = 0.4f;
    NarrativeConfig.FadeInTime = 2.0f;
    NarrativeConfig.FadeOutTime = 3.0f;
    MusicConfigs.Add(EAudio_MusicState::Narrative, NarrativeConfig);
    
    // Survival music config
    FAudio_MusicConfig SurvivalConfig;
    SurvivalConfig.Volume = 0.7f;
    SurvivalConfig.FadeInTime = 1.5f;
    SurvivalConfig.FadeOutTime = 2.0f;
    MusicConfigs.Add(EAudio_MusicState::Survival, SurvivalConfig);
}

void UAudio_AdaptiveMusicSystem::InitializeBiomeConfigs()
{
    // Savana ambient config
    FAudio_MusicConfig SavanaConfig;
    SavanaConfig.Volume = 0.5f;
    SavanaConfig.bLooping = true;
    BiomeAmbientConfigs.Add(EAudio_BiomeType::Savana, SavanaConfig);
    
    // Forest ambient config
    FAudio_MusicConfig ForestConfig;
    ForestConfig.Volume = 0.6f;
    ForestConfig.bLooping = true;
    BiomeAmbientConfigs.Add(EAudio_BiomeType::Forest, ForestConfig);
    
    // Desert ambient config
    FAudio_MusicConfig DesertConfig;
    DesertConfig.Volume = 0.4f;
    DesertConfig.bLooping = true;
    BiomeAmbientConfigs.Add(EAudio_BiomeType::Desert, DesertConfig);
    
    // Swamp ambient config
    FAudio_MusicConfig SwampConfig;
    SwampConfig.Volume = 0.7f;
    SwampConfig.bLooping = true;
    BiomeAmbientConfigs.Add(EAudio_BiomeType::Swamp, SwampConfig);
    
    // Mountain ambient config
    FAudio_MusicConfig MountainConfig;
    MountainConfig.Volume = 0.5f;
    MountainConfig.bLooping = true;
    BiomeAmbientConfigs.Add(EAudio_BiomeType::Mountain, MountainConfig);
}

void UAudio_AdaptiveMusicSystem::TransitionToState(EAudio_MusicState NewState)
{
    CurrentConfig = MusicConfigs.Find(CurrentMusicState);
    TargetConfig = MusicConfigs.Find(NewState);
    
    if (CurrentConfig && TargetConfig)
    {
        bIsTransitioning = true;
        TransitionTimer = 0.0f;
        
        // Start fade out of current music
        if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
        {
            MusicAudioComponent->FadeOut(CurrentConfig->FadeOutTime, 0.0f);
        }
    }
}

void UAudio_AdaptiveMusicSystem::UpdateTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime;
    
    if (CurrentConfig && TransitionTimer >= CurrentConfig->FadeOutTime)
    {
        // Fade out complete, start new music
        PlayMusicForState(CurrentMusicState);
        bIsTransitioning = false;
        TransitionTimer = 0.0f;
    }
}

void UAudio_AdaptiveMusicSystem::PlayMusicForState(EAudio_MusicState State)
{
    if (!MusicAudioComponent)
    {
        return;
    }
    
    FAudio_MusicConfig* Config = MusicConfigs.Find(State);
    if (!Config)
    {
        return;
    }
    
    // In full implementation, load appropriate sound cue based on state
    UE_LOG(LogTemp, Log, TEXT("Audio: Playing music for state %d with volume %f"), 
        (int32)State, Config->Volume);
    
    // For now, just set volume and log
    MusicAudioComponent->SetVolumeMultiplier(Config->Volume);
}

void UAudio_AdaptiveMusicSystem::PlayAmbientForBiome(EAudio_BiomeType Biome)
{
    if (!AmbientAudioComponent)
    {
        return;
    }
    
    FAudio_MusicConfig* Config = BiomeAmbientConfigs.Find(Biome);
    if (!Config)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio: Playing ambient for biome %d with volume %f"), 
        (int32)Biome, Config->Volume);
    
    AmbientAudioComponent->SetVolumeMultiplier(Config->Volume);
}

void UAudio_AdaptiveMusicSystem::CheckForDangers()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get player location
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    // Check for nearby dinosaurs (simplified check)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dino")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance < DangerDetectionRadius)
            {
                OnDinosaurNearby(Distance, Actor->GetName());
                break;
            }
        }
    }
}