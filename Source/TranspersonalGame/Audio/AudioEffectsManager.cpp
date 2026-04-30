#include "AudioEffectsManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "TimerManager.h"

UAudioEffectsManager::UAudioEffectsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    GlobalEffectIntensity = 1.0f;
    bEffectsEnabled = true;
    ScreenShakeIntensity = 1.0f;
    DamageFlashDuration = 0.5f;
    DamageFlashColor = FLinearColor::Red;

    // Initialize default effect settings
    FAudio_EffectSettings DefaultScreenShake;
    DefaultScreenShake.Duration = 1.0f;
    DefaultScreenShake.Intensity = 1.0f;
    DefaultScreenShake.FadeInTime = 0.1f;
    DefaultScreenShake.FadeOutTime = 0.3f;
    DefaultScreenShake.bShouldLoop = false;
    EffectSettings.Add(EAudio_EffectType::ScreenShake, DefaultScreenShake);

    FAudio_EffectSettings DefaultDamageFlash;
    DefaultDamageFlash.Duration = 0.5f;
    DefaultDamageFlash.Intensity = 0.8f;
    DefaultDamageFlash.FadeInTime = 0.05f;
    DefaultDamageFlash.FadeOutTime = 0.2f;
    DefaultDamageFlash.bShouldLoop = false;
    EffectSettings.Add(EAudio_EffectType::DamageFlash, DefaultDamageFlash);

    FAudio_EffectSettings DefaultFootstepDust;
    DefaultFootstepDust.Duration = 0.3f;
    DefaultFootstepDust.Intensity = 0.6f;
    DefaultFootstepDust.FadeInTime = 0.0f;
    DefaultFootstepDust.FadeOutTime = 0.1f;
    DefaultFootstepDust.bShouldLoop = false;
    EffectSettings.Add(EAudio_EffectType::FootstepDust, DefaultFootstepDust);

    FAudio_EffectSettings DefaultProximityTension;
    DefaultProximityTension.Duration = 2.0f;
    DefaultProximityTension.Intensity = 1.0f;
    DefaultProximityTension.FadeInTime = 0.5f;
    DefaultProximityTension.FadeOutTime = 1.0f;
    DefaultProximityTension.bShouldLoop = true;
    EffectSettings.Add(EAudio_EffectType::ProximityTension, DefaultProximityTension);
}

void UAudioEffectsManager::BeginPlay()
{
    Super::BeginPlay();
    
    LoadNarrativeAudioAssets();
    
    UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Initialized with %d effect settings"), EffectSettings.Num());
}

void UAudioEffectsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update active effect timers
    TArray<EAudio_EffectType> ExpiredEffects;
    for (auto& EffectTimer : ActiveEffectTimers)
    {
        EffectTimer.Value -= DeltaTime;
        if (EffectTimer.Value <= 0.0f)
        {
            ExpiredEffects.Add(EffectTimer.Key);
        }
    }
    
    // Remove expired effects
    for (EAudio_EffectType ExpiredEffect : ExpiredEffects)
    {
        ActiveEffectTimers.Remove(ExpiredEffect);
    }
    
    // Clean up finished audio components
    CleanupFinishedAudioComponents();
    
    // Update proximity effects
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerCharacter)
    {
        for (const auto& ProximityPair : ProximitySourceActors)
        {
            if (ProximityPair.Value && IsValid(ProximityPair.Value))
            {
                float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), ProximityPair.Value->GetActorLocation());
                UpdateProximityTension(ProximityPair.Value, Distance);
            }
        }
    }
}

void UAudioEffectsManager::TriggerScreenShake(EAudio_IntensityLevel Intensity, float Duration)
{
    if (!bEffectsEnabled) return;
    
    float ShakeIntensity = ScreenShakeIntensity * GlobalEffectIntensity;
    
    switch (Intensity)
    {
        case EAudio_IntensityLevel::Low:
            ShakeIntensity *= 0.3f;
            break;
        case EAudio_IntensityLevel::Medium:
            ShakeIntensity *= 0.6f;
            break;
        case EAudio_IntensityLevel::High:
            ShakeIntensity *= 1.0f;
            break;
        case EAudio_IntensityLevel::Extreme:
            ShakeIntensity *= 1.5f;
            break;
    }
    
    ApplyScreenShakeEffect(ShakeIntensity, Duration);
    ActiveEffectTimers.Add(EAudio_EffectType::ScreenShake, Duration);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Screen shake triggered - Intensity: %f, Duration: %f"), ShakeIntensity, Duration);
}

void UAudioEffectsManager::TriggerDamageFlash(float Duration, FLinearColor FlashColor)
{
    if (!bEffectsEnabled) return;
    
    ApplyDamageFlashEffect(Duration, FlashColor);
    ActiveEffectTimers.Add(EAudio_EffectType::DamageFlash, Duration);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Damage flash triggered - Duration: %f"), Duration);
}

void UAudioEffectsManager::TriggerFootstepDust(FVector Location, EAudio_IntensityLevel Intensity)
{
    if (!bEffectsEnabled) return;
    
    // For now, just log the footstep dust effect
    // In a full implementation, this would spawn particle effects
    float EffectIntensity = GlobalEffectIntensity;
    
    switch (Intensity)
    {
        case EAudio_IntensityLevel::Low:
            EffectIntensity *= 0.4f;
            break;
        case EAudio_IntensityLevel::Medium:
            EffectIntensity *= 0.7f;
            break;
        case EAudio_IntensityLevel::High:
            EffectIntensity *= 1.0f;
            break;
        case EAudio_IntensityLevel::Extreme:
            EffectIntensity *= 1.3f;
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Footstep dust at location (%f, %f, %f) - Intensity: %f"), 
           Location.X, Location.Y, Location.Z, EffectIntensity);
}

void UAudioEffectsManager::UpdateProximityTension(AActor* SourceActor, float Distance)
{
    if (!SourceActor || !bEffectsEnabled) return;
    
    // Find proximity effect settings for this actor
    FString ActorName = SourceActor->GetName();
    if (ProximityEffects.Contains(ActorName))
    {
        const FAudio_ProximityEffect& Settings = ProximityEffects[ActorName];
        
        if (Distance <= Settings.MaxDistance)
        {
            float Intensity = CalculateProximityIntensity(Distance, Settings);
            
            if (Intensity > 0.1f && Settings.ProximitySound)
            {
                // Play or update proximity sound
                UAudioComponent* AudioComp = CreateAudioComponent();
                if (AudioComp)
                {
                    AudioComp->SetSound(Settings.ProximitySound);
                    AudioComp->SetVolumeMultiplier(Intensity * GlobalEffectIntensity);
                    AudioComp->SetWorldLocation(SourceActor->GetActorLocation());
                    AudioComp->Play();
                }
            }
        }
    }
}

void UAudioEffectsManager::PlayNarrativeClip(const FString& ClipName, FVector Location)
{
    if (!bEffectsEnabled) return;
    
    USoundBase* SoundToPlay = nullptr;
    
    if (ClipName == "ForestNarrator" && ForestNarratorSound)
    {
        SoundToPlay = ForestNarratorSound;
    }
    else if (ClipName == "WaterGuide" && WaterGuideSound)
    {
        SoundToPlay = WaterGuideSound;
    }
    
    if (SoundToPlay)
    {
        UAudioComponent* AudioComp = CreateAudioComponent();
        if (AudioComp)
        {
            AudioComp->SetSound(SoundToPlay);
            AudioComp->SetVolumeMultiplier(GlobalEffectIntensity);
            
            if (Location != FVector::ZeroVector)
            {
                AudioComp->SetWorldLocation(Location);
            }
            
            AudioComp->Play();
            UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Playing narrative clip '%s'"), *ClipName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Narrative clip '%s' not found"), *ClipName);
    }
}

void UAudioEffectsManager::PlayForestNarration(FVector Location)
{
    PlayNarrativeClip("ForestNarrator", Location);
}

void UAudioEffectsManager::PlayWaterGuideNarration(FVector Location)
{
    PlayNarrativeClip("WaterGuide", Location);
}

void UAudioEffectsManager::StartForestAmbience(float FadeInTime)
{
    if (!ForestAmbienceSound || !bEffectsEnabled) return;
    
    UAudioComponent* AudioComp = CreateAudioComponent();
    if (AudioComp)
    {
        AudioComp->SetSound(ForestAmbienceSound);
        AudioComp->SetVolumeMultiplier(0.0f);
        AudioComp->bAutoDestroy = false;
        AudioComp->Play();
        
        // Fade in the ambience
        AudioComp->FadeIn(FadeInTime, GlobalEffectIntensity * 0.7f);
        
        UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Started forest ambience with fade-in time: %f"), FadeInTime);
    }
}

void UAudioEffectsManager::StartWaterAmbience(FVector Location, float FadeInTime)
{
    if (!WaterFlowSound || !bEffectsEnabled) return;
    
    UAudioComponent* AudioComp = CreateAudioComponent();
    if (AudioComp)
    {
        AudioComp->SetSound(WaterFlowSound);
        AudioComp->SetWorldLocation(Location);
        AudioComp->SetVolumeMultiplier(0.0f);
        AudioComp->bAutoDestroy = false;
        AudioComp->Play();
        
        // Fade in the water ambience
        AudioComp->FadeIn(FadeInTime, GlobalEffectIntensity * 0.8f);
        
        UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Started water ambience at location (%f, %f, %f)"), 
               Location.X, Location.Y, Location.Z);
    }
}

void UAudioEffectsManager::StopAllAmbience(float FadeOutTime)
{
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            AudioComp->FadeOut(FadeOutTime, 0.0f);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Stopping all ambience with fade-out time: %f"), FadeOutTime);
}

void UAudioEffectsManager::RegisterProximitySource(const FString& SourceName, AActor* SourceActor, FAudio_ProximityEffect Settings)
{
    if (!SourceActor) return;
    
    ProximitySourceActors.Add(SourceName, SourceActor);
    ProximityEffects.Add(SourceName, Settings);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Registered proximity source '%s'"), *SourceName);
}

void UAudioEffectsManager::UnregisterProximitySource(const FString& SourceName)
{
    ProximitySourceActors.Remove(SourceName);
    ProximityEffects.Remove(SourceName);
    
    UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Unregistered proximity source '%s'"), *SourceName);
}

void UAudioEffectsManager::SetGlobalEffectIntensity(float NewIntensity)
{
    GlobalEffectIntensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Global effect intensity set to %f"), GlobalEffectIntensity);
}

void UAudioEffectsManager::StopAllEffects()
{
    ActiveEffectTimers.Empty();
    
    for (UAudioComponent* AudioComp : ActiveAudioComponents)
    {
        if (AudioComp && AudioComp->IsPlaying())
        {
            AudioComp->Stop();
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: All effects stopped"));
}

bool UAudioEffectsManager::IsEffectActive(EAudio_EffectType EffectType) const
{
    return ActiveEffectTimers.Contains(EffectType);
}

void UAudioEffectsManager::LoadNarrativeAudioAssets()
{
    // In a full implementation, these would load the actual audio assets
    // For now, we'll just log that we're attempting to load them
    UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Loading narrative audio assets..."));
    
    // Placeholder - in real implementation, use LoadObject or similar
    // ForestNarratorSound = LoadObject<USoundWave>(nullptr, TEXT("/Game/Audio/Narrative/ForestNarrator"));
    // WaterGuideSound = LoadObject<USoundWave>(nullptr, TEXT("/Game/Audio/Narrative/WaterGuide"));
    // ForestAmbienceSound = LoadObject<USoundWave>(nullptr, TEXT("/Game/Audio/Ambience/ForestAmbience"));
    // WaterFlowSound = LoadObject<USoundWave>(nullptr, TEXT("/Game/Audio/Ambience/WaterFlow"));
}

UAudioComponent* UAudioEffectsManager::CreateAudioComponent()
{
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(GetOwner());
    if (AudioComp)
    {
        AudioComp->bAutoDestroy = true;
        AudioComp->bStopWhenOwnerDestroyed = true;
        ActiveAudioComponents.Add(AudioComp);
        return AudioComp;
    }
    return nullptr;
}

void UAudioEffectsManager::CleanupFinishedAudioComponents()
{
    ActiveAudioComponents.RemoveAll([](UAudioComponent* AudioComp)
    {
        return !AudioComp || !IsValid(AudioComp) || (!AudioComp->IsPlaying() && AudioComp->bAutoDestroy);
    });
}

float UAudioEffectsManager::CalculateProximityIntensity(float Distance, const FAudio_ProximityEffect& Settings)
{
    if (Distance >= Settings.MaxDistance) return 0.0f;
    if (Distance <= Settings.MinDistance) return 1.0f;
    
    float NormalizedDistance = (Distance - Settings.MinDistance) / (Settings.MaxDistance - Settings.MinDistance);
    
    if (Settings.IntensityCurve)
    {
        return Settings.IntensityCurve->GetFloatValue(1.0f - NormalizedDistance);
    }
    
    // Default linear falloff
    return 1.0f - NormalizedDistance;
}

void UAudioEffectsManager::ApplyScreenShakeEffect(float Intensity, float Duration)
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController)
    {
        // In a full implementation, this would use a proper camera shake class
        // For now, just log the effect
        UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Applying screen shake - Intensity: %f, Duration: %f"), Intensity, Duration);
        
        // Placeholder for actual screen shake implementation
        // PlayerController->ClientStartCameraShake(SomeScreenShakeClass, Intensity);
    }
}

void UAudioEffectsManager::ApplyDamageFlashEffect(float Duration, FLinearColor Color)
{
    // In a full implementation, this would create a screen overlay or post-process effect
    // For now, just log the effect
    UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Applying damage flash - Duration: %f, Color: (%f,%f,%f,%f)"), 
           Duration, Color.R, Color.G, Color.B, Color.A);
    
    // Placeholder for actual damage flash implementation
    // This could involve creating a widget overlay or modifying post-process settings
}