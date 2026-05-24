#include "Audio_VolcanicHazardSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAudio_VolcanicHazardSystem::UAudio_VolcanicHazardSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize default values
    CurrentThreatLevel = EAudio_VolcanicThreatLevel::Safe;
    bSeismicActive = false;
    CurrentSeismicIntensity = 0.0f;
    PlayerDistanceToVolcano = 10000.0f;
    SeismicUpdateInterval = 0.5f;
    MaxVolcanicDistance = 10000.0f;
}

void UAudio_VolcanicHazardSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_VolcanicHazardSystem: BeginPlay"));
    
    // Initialize audio components
    InitializeAudioComponents();
    
    // Start seismic monitoring
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            SeismicUpdateTimer,
            this,
            &UAudio_VolcanicHazardSystem::UpdateSeismicAudio,
            SeismicUpdateInterval,
            true
        );
    }
}

void UAudio_VolcanicHazardSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SeismicUpdateTimer);
        GetWorld()->GetTimerManager().ClearTimer(AlertTimer);
    }
    
    // Stop all audio
    StopSeismicRumble();
    StopEmergencyAlert();
    
    Super::EndPlay(EndPlayReason);
}

void UAudio_VolcanicHazardSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update volcanic audio intensity based on player proximity
    CalculateVolcanicAudioIntensity();
}

void UAudio_VolcanicHazardSystem::SetVolcanicThreatLevel(EAudio_VolcanicThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel != NewThreatLevel)
    {
        EAudio_VolcanicThreatLevel PreviousLevel = CurrentThreatLevel;
        CurrentThreatLevel = NewThreatLevel;
        
        UE_LOG(LogTemp, Warning, TEXT("Volcanic Threat Level Changed: %d -> %d"), 
               (int32)PreviousLevel, (int32)CurrentThreatLevel);
        
        ProcessThreatLevelChange();
    }
}

void UAudio_VolcanicHazardSystem::TriggerEmergencyAlert(const FString& AlertMessage, float Duration)
{
    UE_LOG(LogTemp, Warning, TEXT("VOLCANIC EMERGENCY ALERT: %s"), *AlertMessage);
    
    // Play emergency alert audio
    PlayEmergencyAlert();
    
    // Set timer to stop alert
    if (GetWorld() && Duration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            AlertTimer,
            this,
            &UAudio_VolcanicHazardSystem::StopEmergencyAlert,
            Duration,
            false
        );
    }
    
    // Trigger camera shake for urgency
    ApplyVolcanicCameraShake(0.8f);
}

void UAudio_VolcanicHazardSystem::StartSeismicRumble(float Intensity)
{
    CurrentSeismicIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
    bSeismicActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Starting seismic rumble with intensity: %f"), CurrentSeismicIntensity);
    
    if (RumbleAudioComponent && VolcanicAudioData.RumbleSoundCue)
    {
        RumbleAudioComponent->SetSound(VolcanicAudioData.RumbleSoundCue);
        RumbleAudioComponent->Play();
        FadeInRumble(2.0f);
    }
}

void UAudio_VolcanicHazardSystem::StopSeismicRumble()
{
    if (bSeismicActive)
    {
        bSeismicActive = false;
        CurrentSeismicIntensity = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Stopping seismic rumble"));
        
        if (RumbleAudioComponent && RumbleAudioComponent->IsPlaying())
        {
            FadeOutRumble(3.0f);
        }
    }
}

void UAudio_VolcanicHazardSystem::TriggerVolcanicEruption()
{
    UE_LOG(LogTemp, Error, TEXT("VOLCANIC ERUPTION TRIGGERED!"));
    
    // Set maximum threat level
    SetVolcanicThreatLevel(EAudio_VolcanicThreatLevel::Eruption);
    
    // Play eruption audio
    if (EruptionAudioComponent && VolcanicAudioData.EruptionSoundCue)
    {
        EruptionAudioComponent->SetSound(VolcanicAudioData.EruptionSoundCue);
        EruptionAudioComponent->SetVolumeMultiplier(VolcanicAudioData.BaseVolume * 1.5f);
        EruptionAudioComponent->Play();
    }
    
    // Trigger emergency alert
    TriggerEmergencyAlert(TEXT("VOLCANIC ERUPTION IN PROGRESS! EVACUATE IMMEDIATELY!"), 60.0f);
    
    // Maximum camera shake
    ApplyVolcanicCameraShake(2.0f);
    
    // Start intense seismic activity
    StartSeismicRumble(2.0f);
}

void UAudio_VolcanicHazardSystem::UpdatePlayerProximityToVolcano(float DistanceToVolcano)
{
    PlayerDistanceToVolcano = DistanceToVolcano;
    
    // Auto-adjust threat level based on proximity
    if (DistanceToVolcano < 1000.0f && CurrentThreatLevel < EAudio_VolcanicThreatLevel::Critical)
    {
        SetVolcanicThreatLevel(EAudio_VolcanicThreatLevel::Critical);
    }
    else if (DistanceToVolcano < 2500.0f && CurrentThreatLevel < EAudio_VolcanicThreatLevel::Danger)
    {
        SetVolcanicThreatLevel(EAudio_VolcanicThreatLevel::Danger);
    }
    else if (DistanceToVolcano < 5000.0f && CurrentThreatLevel < EAudio_VolcanicThreatLevel::Warning)
    {
        SetVolcanicThreatLevel(EAudio_VolcanicThreatLevel::Warning);
    }
}

void UAudio_VolcanicHazardSystem::InitializeAudioComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    // Create rumble audio component
    RumbleAudioComponent = NewObject<UAudioComponent>(Owner);
    if (RumbleAudioComponent)
    {
        RumbleAudioComponent->SetupAttachment(Owner->GetRootComponent());
        RumbleAudioComponent->SetVolumeMultiplier(0.0f); // Start silent
        RumbleAudioComponent->bAutoActivate = false;
        Owner->AddInstanceComponent(RumbleAudioComponent);
    }
    
    // Create alert audio component
    AlertAudioComponent = NewObject<UAudioComponent>(Owner);
    if (AlertAudioComponent)
    {
        AlertAudioComponent->SetupAttachment(Owner->GetRootComponent());
        AlertAudioComponent->SetVolumeMultiplier(VolcanicAudioData.BaseVolume);
        AlertAudioComponent->bAutoActivate = false;
        Owner->AddInstanceComponent(AlertAudioComponent);
    }
    
    // Create eruption audio component
    EruptionAudioComponent = NewObject<UAudioComponent>(Owner);
    if (EruptionAudioComponent)
    {
        EruptionAudioComponent->SetupAttachment(Owner->GetRootComponent());
        EruptionAudioComponent->SetVolumeMultiplier(VolcanicAudioData.BaseVolume);
        EruptionAudioComponent->bAutoActivate = false;
        Owner->AddInstanceComponent(EruptionAudioComponent);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Volcanic audio components initialized"));
}

void UAudio_VolcanicHazardSystem::UpdateSeismicAudio()
{
    if (!bSeismicActive || !RumbleAudioComponent) return;
    
    // Calculate intensity based on threat level and proximity
    float IntensityMultiplier = 1.0f;
    
    switch (CurrentThreatLevel)
    {
        case EAudio_VolcanicThreatLevel::Safe:
            IntensityMultiplier = 0.0f;
            break;
        case EAudio_VolcanicThreatLevel::Warning:
            IntensityMultiplier = 0.3f;
            break;
        case EAudio_VolcanicThreatLevel::Danger:
            IntensityMultiplier = 0.6f;
            break;
        case EAudio_VolcanicThreatLevel::Critical:
            IntensityMultiplier = 0.9f;
            break;
        case EAudio_VolcanicThreatLevel::Eruption:
            IntensityMultiplier = 1.5f;
            break;
    }
    
    // Apply proximity falloff
    float ProximityFactor = 1.0f - FMath::Clamp(PlayerDistanceToVolcano / MaxVolcanicDistance, 0.0f, 1.0f);
    float FinalIntensity = CurrentSeismicIntensity * IntensityMultiplier * ProximityFactor;
    
    // Update audio properties
    UpdateRumbleVolume(FinalIntensity);
    UpdateRumblePitch(0.8f + (FinalIntensity * 0.4f)); // Lower pitch for more ominous feel
}

void UAudio_VolcanicHazardSystem::ProcessThreatLevelChange()
{
    switch (CurrentThreatLevel)
    {
        case EAudio_VolcanicThreatLevel::Safe:
            StopSeismicRumble();
            break;
            
        case EAudio_VolcanicThreatLevel::Warning:
            StartSeismicRumble(0.3f);
            break;
            
        case EAudio_VolcanicThreatLevel::Danger:
            StartSeismicRumble(0.6f);
            ApplyVolcanicCameraShake(0.2f);
            break;
            
        case EAudio_VolcanicThreatLevel::Critical:
            StartSeismicRumble(0.9f);
            ApplyVolcanicCameraShake(0.4f);
            TriggerEmergencyAlert(TEXT("Critical volcanic activity detected! Prepare for evacuation!"), 20.0f);
            break;
            
        case EAudio_VolcanicThreatLevel::Eruption:
            // Handled by TriggerVolcanicEruption()
            break;
    }
}

void UAudio_VolcanicHazardSystem::PlayEmergencyAlert()
{
    if (AlertAudioComponent && VolcanicAudioData.WarningAlertCue)
    {
        AlertAudioComponent->SetSound(VolcanicAudioData.WarningAlertCue);
        AlertAudioComponent->SetVolumeMultiplier(VolcanicAudioData.BaseVolume * 1.2f);
        AlertAudioComponent->Play();
    }
}

void UAudio_VolcanicHazardSystem::StopEmergencyAlert()
{
    if (AlertAudioComponent && AlertAudioComponent->IsPlaying())
    {
        AlertAudioComponent->Stop();
    }
}

void UAudio_VolcanicHazardSystem::CalculateVolcanicAudioIntensity()
{
    // This method is called every tick to update audio based on real-time conditions
    if (bSeismicActive)
    {
        // Add subtle variations to seismic intensity
        float TimeVariation = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.5f) * 0.1f;
        float ModifiedIntensity = CurrentSeismicIntensity + TimeVariation;
        
        UpdateRumbleVolume(FMath::Clamp(ModifiedIntensity, 0.0f, 2.0f));
    }
}

void UAudio_VolcanicHazardSystem::ApplyVolcanicCameraShake(float Intensity)
{
    if (!VolcanicShakeClass) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (PC)
    {
        PC->ClientStartCameraShake(VolcanicShakeClass, Intensity);
        UE_LOG(LogTemp, Log, TEXT("Applied volcanic camera shake with intensity: %f"), Intensity);
    }
}

void UAudio_VolcanicHazardSystem::UpdateRumbleVolume(float VolumeMultiplier)
{
    if (RumbleAudioComponent)
    {
        float ClampedVolume = FMath::Clamp(VolumeMultiplier * VolcanicAudioData.BaseVolume, 0.0f, 2.0f);
        RumbleAudioComponent->SetVolumeMultiplier(ClampedVolume);
    }
}

void UAudio_VolcanicHazardSystem::UpdateRumblePitch(float PitchMultiplier)
{
    if (RumbleAudioComponent)
    {
        float ClampedPitch = FMath::Clamp(PitchMultiplier, 0.5f, 2.0f);
        RumbleAudioComponent->SetPitchMultiplier(ClampedPitch);
    }
}

void UAudio_VolcanicHazardSystem::FadeInRumble(float Duration)
{
    if (RumbleAudioComponent)
    {
        RumbleAudioComponent->FadeIn(Duration, VolcanicAudioData.BaseVolume);
    }
}

void UAudio_VolcanicHazardSystem::FadeOutRumble(float Duration)
{
    if (RumbleAudioComponent)
    {
        RumbleAudioComponent->FadeOut(Duration, 0.0f);
    }
}