#include "Audio_SystemManager.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize audio components
    InitializeAudioComponents();
    
    // Set default values
    MasterVolume = 1.0f;
    bEnableProximityWarnings = true;
    bEnableEnvironmentalAudio = true;
    ProximityCheckTimer = 0.0f;
    bTRexNearby = false;
    bRaptorsNearby = false;
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    SetupAudioLayerDefaults();
    
    // Start proximity checking timer
    if (bEnableProximityWarnings)
    {
        GetWorldTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &AAudio_SystemManager::UpdateProximityWarnings,
            ProximitySettings.ProximityUpdateRate,
            true
        );
    }
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    ProximityCheckTimer += DeltaTime;
    
    // Update proximity warnings periodically
    if (ProximityCheckTimer >= ProximitySettings.ProximityUpdateRate)
    {
        ProximityCheckTimer = 0.0f;
        if (bEnableProximityWarnings)
        {
            CheckDinosaurProximity();
        }
    }
}

void AAudio_SystemManager::InitializeAudioComponents()
{
    // Create audio components for each layer
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.7f);

    ProximityAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ProximityAudioComponent"));
    ProximityAudioComponent->SetupAttachment(RootComponent);
    ProximityAudioComponent->bAutoActivate = false;
    ProximityAudioComponent->SetVolumeMultiplier(0.8f);

    NarrationAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrationAudioComponent"));
    NarrationAudioComponent->SetupAttachment(RootComponent);
    NarrationAudioComponent->bAutoActivate = false;
    NarrationAudioComponent->SetVolumeMultiplier(1.0f);

    SeismicAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SeismicAudioComponent"));
    SeismicAudioComponent->SetupAttachment(RootComponent);
    SeismicAudioComponent->bAutoActivate = false;
    SeismicAudioComponent->SetVolumeMultiplier(1.2f);

    DinosaurSFXComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DinosaurSFXComponent"));
    DinosaurSFXComponent->SetupAttachment(RootComponent);
    DinosaurSFXComponent->bAutoActivate = false;
    DinosaurSFXComponent->SetVolumeMultiplier(0.9f);

    PlayerSFXComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PlayerSFXComponent"));
    PlayerSFXComponent->SetupAttachment(RootComponent);
    PlayerSFXComponent->bAutoActivate = false;
    PlayerSFXComponent->SetVolumeMultiplier(0.8f);
}

void AAudio_SystemManager::SetupAudioLayerDefaults()
{
    // Initialize default audio layer configurations
    AudioLayers.Empty();
    
    // Ambient layer
    FAudio_LayerConfig AmbientConfig;
    AmbientConfig.LayerType = EAudio_LayerType::Ambient;
    AmbientConfig.Volume = 0.7f;
    AmbientConfig.bIs3D = true;
    AmbientConfig.AttenuationDistance = 8000.0f;
    AudioLayers.Add(AmbientConfig);
    
    // Proximity warning layer
    FAudio_LayerConfig ProximityConfig;
    ProximityConfig.LayerType = EAudio_LayerType::Proximity;
    ProximityConfig.Volume = 0.8f;
    ProximityConfig.bIs3D = true;
    ProximityConfig.AttenuationDistance = 5000.0f;
    AudioLayers.Add(ProximityConfig);
    
    // Narration layer
    FAudio_LayerConfig NarrationConfig;
    NarrationConfig.LayerType = EAudio_LayerType::Narration;
    NarrationConfig.Volume = 1.0f;
    NarrationConfig.bIs3D = false;
    AudioLayers.Add(NarrationConfig);
    
    // Seismic warning layer
    FAudio_LayerConfig SeismicConfig;
    SeismicConfig.LayerType = EAudio_LayerType::SeismicWarning;
    SeismicConfig.Volume = 1.2f;
    SeismicConfig.bIs3D = true;
    SeismicConfig.AttenuationDistance = 6000.0f;
    AudioLayers.Add(SeismicConfig);
    
    // Dinosaur SFX layer
    FAudio_LayerConfig DinosaurConfig;
    DinosaurConfig.LayerType = EAudio_LayerType::DinosaurSFX;
    DinosaurConfig.Volume = 0.9f;
    DinosaurConfig.bIs3D = true;
    DinosaurConfig.AttenuationDistance = 4000.0f;
    AudioLayers.Add(DinosaurConfig);
    
    // Player SFX layer
    FAudio_LayerConfig PlayerConfig;
    PlayerConfig.LayerType = EAudio_LayerType::PlayerSFX;
    PlayerConfig.Volume = 0.8f;
    PlayerConfig.bIs3D = false;
    AudioLayers.Add(PlayerConfig);
}

void AAudio_SystemManager::PlayAudioLayer(EAudio_LayerType LayerType, USoundBase* Sound, float VolumeMultiplier)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SystemManager: Attempted to play null sound on layer"));
        return;
    }

    UAudioComponent* AudioComp = GetAudioComponentForLayer(LayerType);
    if (!AudioComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SystemManager: No audio component found for layer type"));
        return;
    }

    // Find layer configuration
    FAudio_LayerConfig* LayerConfig = AudioLayers.FindByPredicate([LayerType](const FAudio_LayerConfig& Config)
    {
        return Config.LayerType == LayerType;
    });

    float FinalVolume = MasterVolume * VolumeMultiplier;
    if (LayerConfig)
    {
        FinalVolume *= LayerConfig->Volume;
    }

    AudioComp->SetSound(Sound);
    AudioComp->SetVolumeMultiplier(FinalVolume);
    AudioComp->Play();

    UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Playing audio on layer with volume %f"), FinalVolume);
}

void AAudio_SystemManager::StopAudioLayer(EAudio_LayerType LayerType, float FadeOutTime)
{
    UAudioComponent* AudioComp = GetAudioComponentForLayer(LayerType);
    if (!AudioComp)
    {
        return;
    }

    if (FadeOutTime > 0.0f)
    {
        AudioComp->FadeOut(FadeOutTime, 0.0f);
    }
    else
    {
        AudioComp->Stop();
    }
}

void AAudio_SystemManager::SetLayerVolume(EAudio_LayerType LayerType, float Volume, float FadeTime)
{
    UAudioComponent* AudioComp = GetAudioComponentForLayer(LayerType);
    if (!AudioComp)
    {
        return;
    }

    float FinalVolume = MasterVolume * Volume;
    
    if (FadeTime > 0.0f)
    {
        AudioComp->FadeIn(FadeTime, FinalVolume);
    }
    else
    {
        AudioComp->SetVolumeMultiplier(FinalVolume);
    }
}

void AAudio_SystemManager::TriggerProximityWarning(EAudio_LayerType WarningType, float Intensity)
{
    if (!bEnableProximityWarnings)
    {
        return;
    }

    UAudioComponent* AudioComp = GetAudioComponentForLayer(WarningType);
    if (!AudioComp || !AudioComp->GetSound())
    {
        return;
    }

    float WarningVolume = MasterVolume * Intensity;
    AudioComp->SetVolumeMultiplier(WarningVolume);
    AudioComp->Play();

    UE_LOG(LogTemp, Log, TEXT("Audio_SystemManager: Triggered proximity warning with intensity %f"), Intensity);
}

void AAudio_SystemManager::UpdateProximityWarnings()
{
    if (!bEnableProximityWarnings)
    {
        return;
    }

    CheckDinosaurProximity();
}

void AAudio_SystemManager::PlayNarration(USoundBase* NarrationSound, float Volume)
{
    if (!NarrationSound)
    {
        return;
    }

    // Stop any currently playing narration
    StopNarration(0.5f);

    // Play new narration
    PlayAudioLayer(EAudio_LayerType::Narration, NarrationSound, Volume);
}

void AAudio_SystemManager::StopNarration(float FadeOutTime)
{
    StopAudioLayer(EAudio_LayerType::Narration, FadeOutTime);
}

void AAudio_SystemManager::SetEnvironmentalAudio(USoundBase* AmbientSound, float Volume)
{
    if (!bEnableEnvironmentalAudio || !AmbientSound)
    {
        return;
    }

    PlayAudioLayer(EAudio_LayerType::Ambient, AmbientSound, Volume);
}

void AAudio_SystemManager::TransitionEnvironmentalAudio(USoundBase* NewAmbientSound, float TransitionTime)
{
    if (!bEnableEnvironmentalAudio || !NewAmbientSound)
    {
        return;
    }

    // Fade out current ambient audio
    StopAudioLayer(EAudio_LayerType::Ambient, TransitionTime * 0.5f);

    // Schedule new ambient audio to fade in
    FTimerHandle TransitionTimer;
    GetWorldTimerManager().SetTimer(TransitionTimer, [this, NewAmbientSound, TransitionTime]()
    {
        SetEnvironmentalAudio(NewAmbientSound, 0.7f);
    }, TransitionTime * 0.5f, false);
}

UAudioComponent* AAudio_SystemManager::GetAudioComponentForLayer(EAudio_LayerType LayerType)
{
    switch (LayerType)
    {
        case EAudio_LayerType::Ambient:
            return AmbientAudioComponent;
        case EAudio_LayerType::Proximity:
            return ProximityAudioComponent;
        case EAudio_LayerType::Narration:
            return NarrationAudioComponent;
        case EAudio_LayerType::SeismicWarning:
            return SeismicAudioComponent;
        case EAudio_LayerType::DinosaurSFX:
            return DinosaurSFXComponent;
        case EAudio_LayerType::PlayerSFX:
            return PlayerSFXComponent;
        default:
            return nullptr;
    }
}

void AAudio_SystemManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
    
    // Update all active audio components
    for (const FAudio_LayerConfig& LayerConfig : AudioLayers)
    {
        UAudioComponent* AudioComp = GetAudioComponentForLayer(LayerConfig.LayerType);
        if (AudioComp && AudioComp->IsPlaying())
        {
            float LayerVolume = LayerConfig.Volume * MasterVolume;
            AudioComp->SetVolumeMultiplier(LayerVolume);
        }
    }
}

bool AAudio_SystemManager::IsLayerPlaying(EAudio_LayerType LayerType)
{
    UAudioComponent* AudioComp = GetAudioComponentForLayer(LayerType);
    return AudioComp && AudioComp->IsPlaying();
}

float AAudio_SystemManager::CalculateDistanceToPlayer()
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return 99999.0f;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    FVector AudioManagerLocation = GetActorLocation();
    
    return FVector::Dist(PlayerLocation, AudioManagerLocation);
}

void AAudio_SystemManager::CheckDinosaurProximity()
{
    // Get all actors in the world to check for dinosaurs
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    bool bTRexFound = false;
    bool bRaptorFound = false;
    float ClosestTRexDistance = 99999.0f;
    float ClosestRaptorDistance = 99999.0f;

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();

    // Check for dinosaurs by actor name/label
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }

        FString ActorName = Actor->GetName();
        FVector ActorLocation = Actor->GetActorLocation();
        float Distance = FVector::Dist(PlayerLocation, ActorLocation);

        // Check for T-Rex
        if (ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Tyrannosaurus")))
        {
            if (Distance <= ProximitySettings.TRexWarningDistance)
            {
                bTRexFound = true;
                ClosestTRexDistance = FMath::Min(ClosestTRexDistance, Distance);
            }
        }
        // Check for Raptors
        else if (ActorName.Contains(TEXT("Raptor")) || ActorName.Contains(TEXT("Velociraptor")))
        {
            if (Distance <= ProximitySettings.RaptorWarningDistance)
            {
                bRaptorFound = true;
                ClosestRaptorDistance = FMath::Min(ClosestRaptorDistance, Distance);
            }
        }
    }

    // Trigger warnings if dinosaurs are detected
    if (bTRexFound && !bTRexNearby)
    {
        float Intensity = 1.0f - (ClosestTRexDistance / ProximitySettings.TRexWarningDistance);
        Intensity *= ProximitySettings.SeismicIntensityMultiplier;
        TriggerProximityWarning(EAudio_LayerType::SeismicWarning, Intensity);
        bTRexNearby = true;
    }
    else if (!bTRexFound && bTRexNearby)
    {
        StopAudioLayer(EAudio_LayerType::SeismicWarning, 2.0f);
        bTRexNearby = false;
    }

    if (bRaptorFound && !bRaptorsNearby)
    {
        float Intensity = 1.0f - (ClosestRaptorDistance / ProximitySettings.RaptorWarningDistance);
        TriggerProximityWarning(EAudio_LayerType::Proximity, Intensity);
        bRaptorsNearby = true;
    }
    else if (!bRaptorFound && bRaptorsNearby)
    {
        StopAudioLayer(EAudio_LayerType::Proximity, 1.5f);
        bRaptorsNearby = false;
    }
}