#include "AudioManager.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UAudioManager::UAudioManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for audio updates

    // Initialize default biome configurations
    BiomeConfigs.SetNum(5);
    
    // Forest biome (default)
    BiomeConfigs[0].BiomeType = EAudio_BiomeType::Forest;
    BiomeConfigs[0].MaxAudibleDistance = 5000.0f;
    
    // Swamp biome
    BiomeConfigs[1].BiomeType = EAudio_BiomeType::Swamp;
    BiomeConfigs[1].MaxAudibleDistance = 3000.0f;
    
    // Savanna biome
    BiomeConfigs[2].BiomeType = EAudio_BiomeType::Savanna;
    BiomeConfigs[2].MaxAudibleDistance = 8000.0f;
    
    // Desert biome
    BiomeConfigs[3].BiomeType = EAudio_BiomeType::Desert;
    BiomeConfigs[3].MaxAudibleDistance = 10000.0f;
    
    // Snow Mountain biome
    BiomeConfigs[4].BiomeType = EAudio_BiomeType::SnowMountain;
    BiomeConfigs[4].MaxAudibleDistance = 6000.0f;

    // Initialize default dinosaur audio data
    DinosaurAudioData.SetNum(3);
    
    // T-Rex
    DinosaurAudioData[0].DinosaurType = TEXT("TRex");
    DinosaurAudioData[0].ThreatRadius = 2000.0f;
    DinosaurAudioData[0].VolumeMultiplier = 1.5f;
    
    // Raptor
    DinosaurAudioData[1].DinosaurType = TEXT("Raptor");
    DinosaurAudioData[1].ThreatRadius = 800.0f;
    DinosaurAudioData[1].VolumeMultiplier = 1.0f;
    
    // Brachiosaurus
    DinosaurAudioData[2].DinosaurType = TEXT("Brachiosaurus");
    DinosaurAudioData[2].ThreatRadius = 1500.0f;
    DinosaurAudioData[2].VolumeMultiplier = 1.2f;

    // Set default values
    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentTension = EAudio_TensionLevel::Calm;
    TargetTension = EAudio_TensionLevel::Calm;
    ProximityCheckInterval = 0.5f;
    TensionTransitionSpeed = 2.0f;
    MaxHeartbeatVolume = 0.8f;
    CurrentHeartbeatIntensity = 0.0f;
    TimeOfDayNormalized = 0.5f;
    ProximityCheckTimer = 0.0f;
}

void UAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    CreateAudioComponents();
    UpdateAmbientLayers();
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Initialized with %d biome configs and %d dinosaur types"), 
           BiomeConfigs.Num(), DinosaurAudioData.Num());
}

void UAudioManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update tension transition
    UpdateTensionTransition(DeltaTime);
    
    // Check dinosaur proximity at intervals
    ProximityCheckTimer += DeltaTime;
    if (ProximityCheckTimer >= ProximityCheckInterval)
    {
        CheckDinosaurProximity();
        ProximityCheckTimer = 0.0f;
    }
}

void UAudioManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        CurrentBiome = NewBiome;
        UpdateAmbientLayers();
        
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Switched to biome %d"), (int32)NewBiome);
    }
}

void UAudioManager::SetTensionLevel(EAudio_TensionLevel NewTension)
{
    if (TargetTension != NewTension)
    {
        TargetTension = NewTension;
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Target tension set to %d"), (int32)NewTension);
    }
}

void UAudioManager::UpdatePlayerPosition(const FVector& NewPlayerLocation)
{
    PlayerLocation = NewPlayerLocation;
}

void UAudioManager::RegisterDinosaur(AActor* DinosaurActor, const FString& DinosaurType)
{
    if (DinosaurActor && !DinosaurType.IsEmpty())
    {
        RegisteredDinosaurs.Add(DinosaurActor, DinosaurType);
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Registered dinosaur %s of type %s"), 
               *DinosaurActor->GetName(), *DinosaurType);
    }
}

void UAudioManager::UnregisterDinosaur(AActor* DinosaurActor)
{
    if (DinosaurActor && RegisteredDinosaurs.Contains(DinosaurActor))
    {
        FString DinosaurType = RegisteredDinosaurs[DinosaurActor];
        RegisteredDinosaurs.Remove(DinosaurActor);
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Unregistered dinosaur %s of type %s"), 
               *DinosaurActor->GetName(), *DinosaurType);
    }
}

void UAudioManager::CheckDinosaurProximity()
{
    if (RegisteredDinosaurs.Num() == 0)
    {
        SetTensionLevel(EAudio_TensionLevel::Calm);
        return;
    }

    float ClosestDistance = CalculateDistanceToNearestDinosaur();
    EAudio_TensionLevel NewTension = CalculateTensionFromProximity(ClosestDistance);
    
    SetTensionLevel(NewTension);
    
    // Update heartbeat based on tension
    float HeartbeatIntensity = 0.0f;
    switch (NewTension)
    {
        case EAudio_TensionLevel::Calm:
            HeartbeatIntensity = 0.0f;
            break;
        case EAudio_TensionLevel::Alert:
            HeartbeatIntensity = 0.3f;
            break;
        case EAudio_TensionLevel::Danger:
            HeartbeatIntensity = 0.6f;
            break;
        case EAudio_TensionLevel::Terror:
            HeartbeatIntensity = 1.0f;
            break;
    }
    
    PlayHeartbeat(HeartbeatIntensity);
}

void UAudioManager::PlayHeartbeat(float Intensity)
{
    CurrentHeartbeatIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (HeartbeatAudioComponent)
    {
        UActorComponent* AudioComp = HeartbeatAudioComponent;
        if (UActorComponent* AudioComponent = Cast<UActorComponent>(AudioComp))
        {
            // Set volume based on intensity
            float Volume = CurrentHeartbeatIntensity * MaxHeartbeatVolume;
            
            // In a real implementation, you would set the audio component's volume here
            // For now, we'll log the action
            UE_LOG(LogTemp, Log, TEXT("AudioManager: Heartbeat intensity set to %.2f (volume %.2f)"), 
                   CurrentHeartbeatIntensity, Volume);
        }
    }
}

void UAudioManager::StopHeartbeat()
{
    CurrentHeartbeatIntensity = 0.0f;
    
    if (HeartbeatAudioComponent)
    {
        // Stop the heartbeat audio component
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Heartbeat stopped"));
    }
}

void UAudioManager::PlayEnvironmentalSound(TSoftObjectPtr<USoundBase> Sound, const FVector& Location, float Volume)
{
    if (Sound.IsValid())
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound.Get(), Location, Volume);
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Played environmental sound at location (%.1f, %.1f, %.1f)"), 
               Location.X, Location.Y, Location.Z);
    }
}

void UAudioManager::SetTimeOfDay(float TimeNormalized)
{
    TimeOfDayNormalized = FMath::Clamp(TimeNormalized, 0.0f, 1.0f);
    
    // Adjust ambient audio based on time of day
    // Night time (0.0-0.2 and 0.8-1.0) should have different ambient sounds
    bool bIsNight = (TimeOfDayNormalized < 0.2f || TimeOfDayNormalized > 0.8f);
    
    if (bIsNight)
    {
        // Increase tension slightly at night
        if (TargetTension == EAudio_TensionLevel::Calm)
        {
            SetTensionLevel(EAudio_TensionLevel::Alert);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Time of day set to %.2f (Night: %s)"), 
           TimeOfDayNormalized, bIsNight ? TEXT("Yes") : TEXT("No"));
}

void UAudioManager::PlayVoiceLine(TSoftObjectPtr<USoundBase> VoiceSound, float Volume)
{
    if (VoiceSound.IsValid() && VoiceAudioComponent)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), VoiceSound.Get(), Volume);
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Played voice line with volume %.2f"), Volume);
    }
}

void UAudioManager::StopAllVoiceLines()
{
    if (VoiceAudioComponent)
    {
        // Stop all voice audio
        UE_LOG(LogTemp, Log, TEXT("AudioManager: Stopped all voice lines"));
    }
}

void UAudioManager::UpdateTensionTransition(float DeltaTime)
{
    if (CurrentTension != TargetTension)
    {
        // Smooth transition between tension levels
        float TransitionSpeed = TensionTransitionSpeed * DeltaTime;
        int32 CurrentLevel = (int32)CurrentTension;
        int32 TargetLevel = (int32)TargetTension;
        
        if (CurrentLevel < TargetLevel)
        {
            CurrentLevel = FMath::Min(CurrentLevel + 1, TargetLevel);
        }
        else if (CurrentLevel > TargetLevel)
        {
            CurrentLevel = FMath::Max(CurrentLevel - 1, TargetLevel);
        }
        
        CurrentTension = (EAudio_TensionLevel)CurrentLevel;
        
        if (CurrentTension == TargetTension)
        {
            UpdateTensionLayers();
            UE_LOG(LogTemp, Log, TEXT("AudioManager: Tension transition completed to level %d"), CurrentLevel);
        }
    }
}

void UAudioManager::UpdateAmbientLayers()
{
    FAudio_BiomeAudioConfig* BiomeConfig = GetCurrentBiomeConfig();
    if (BiomeConfig)
    {
        // Update ambient audio layers for current biome
        for (const FAudio_SoundLayer& Layer : BiomeConfig->AmbientLayers)
        {
            if (Layer.Sound.IsValid())
            {
                // In a real implementation, you would configure audio components here
                UE_LOG(LogTemp, Log, TEXT("AudioManager: Updated ambient layer with volume %.2f"), Layer.Volume);
            }
        }
    }
}

void UAudioManager::UpdateTensionLayers()
{
    FAudio_BiomeAudioConfig* BiomeConfig = GetCurrentBiomeConfig();
    if (BiomeConfig)
    {
        // Update tension audio layers based on current tension level
        float TensionMultiplier = (float)CurrentTension / 3.0f; // Normalize to 0-1
        
        for (const FAudio_SoundLayer& Layer : BiomeConfig->TensionLayers)
        {
            if (Layer.Sound.IsValid())
            {
                float AdjustedVolume = Layer.Volume * TensionMultiplier;
                // Configure tension audio component volume
                UE_LOG(LogTemp, Log, TEXT("AudioManager: Updated tension layer with volume %.2f"), AdjustedVolume);
            }
        }
    }
}

FAudio_BiomeAudioConfig* UAudioManager::GetCurrentBiomeConfig()
{
    for (FAudio_BiomeAudioConfig& Config : BiomeConfigs)
    {
        if (Config.BiomeType == CurrentBiome)
        {
            return &Config;
        }
    }
    return nullptr;
}

FAudio_DinosaurAudioData* UAudioManager::GetDinosaurAudioData(const FString& DinosaurType)
{
    for (FAudio_DinosaurAudioData& Data : DinosaurAudioData)
    {
        if (Data.DinosaurType == DinosaurType)
        {
            return &Data;
        }
    }
    return nullptr;
}

float UAudioManager::CalculateDistanceToNearestDinosaur()
{
    float MinDistance = FLT_MAX;
    
    for (const auto& DinosaurPair : RegisteredDinosaurs)
    {
        if (DinosaurPair.Key)
        {
            FVector DinosaurLocation = DinosaurPair.Key->GetActorLocation();
            float Distance = FVector::Dist(PlayerLocation, DinosaurLocation);
            MinDistance = FMath::Min(MinDistance, Distance);
        }
    }
    
    return (MinDistance == FLT_MAX) ? 10000.0f : MinDistance; // Return large distance if no dinosaurs
}

EAudio_TensionLevel UAudioManager::CalculateTensionFromProximity(float Distance)
{
    // Define distance thresholds for tension levels
    const float TerrorDistance = 500.0f;
    const float DangerDistance = 1000.0f;
    const float AlertDistance = 2000.0f;
    
    if (Distance <= TerrorDistance)
    {
        return EAudio_TensionLevel::Terror;
    }
    else if (Distance <= DangerDistance)
    {
        return EAudio_TensionLevel::Danger;
    }
    else if (Distance <= AlertDistance)
    {
        return EAudio_TensionLevel::Alert;
    }
    else
    {
        return EAudio_TensionLevel::Calm;
    }
}

void UAudioManager::CreateAudioComponents()
{
    // Create audio components for different audio layers
    AmbientAudioComponents.Empty();
    
    // Create multiple ambient audio components for layered soundscape
    for (int32 i = 0; i < 4; ++i)
    {
        UActorComponent* NewAudioComp = NewObject<UActorComponent>(GetOwner());
        if (NewAudioComp)
        {
            AmbientAudioComponents.Add(NewAudioComp);
        }
    }
    
    // Create heartbeat audio component
    HeartbeatAudioComponent = NewObject<UActorComponent>(GetOwner());
    
    // Create voice audio component
    VoiceAudioComponent = NewObject<UActorComponent>(GetOwner());
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Created %d ambient audio components"), AmbientAudioComponents.Num());
}

void UAudioManager::CleanupAudioComponents()
{
    AmbientAudioComponents.Empty();
    HeartbeatAudioComponent = nullptr;
    VoiceAudioComponent = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("AudioManager: Cleaned up audio components"));
}