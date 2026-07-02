
#include "AudioSystemManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Components/AudioComponent.h"

AAudio_SystemManager::AAudio_SystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentDangerLevel = EAudio_DangerLevel::Safe;
    bIsNightMode = false;
    TimeSinceLastProximityUpdate = 0.0f;
}

void AAudio_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    // Start with day ambient layers at safe level
    UpdateAmbientLayersForDanger();
}

void AAudio_SystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TimeSinceLastProximityUpdate += DeltaTime;
}

void AAudio_SystemManager::SetDangerLevel(EAudio_DangerLevel NewLevel)
{
    if (CurrentDangerLevel == NewLevel) return;
    CurrentDangerLevel = NewLevel;
    UpdateAmbientLayersForDanger();

    // Log for debugging
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: DangerLevel changed to %d"), (int32)NewLevel);
}

void AAudio_SystemManager::UpdateDangerProximity(float ClosestPredatorDistance)
{
    if (TimeSinceLastProximityUpdate < DangerProximityUpdateInterval) return;
    TimeSinceLastProximityUpdate = 0.0f;

    EAudio_DangerLevel NewLevel = EAudio_DangerLevel::Safe;

    if (ClosestPredatorDistance < 300.0f)
    {
        NewLevel = EAudio_DangerLevel::Critical;
    }
    else if (ClosestPredatorDistance < 800.0f)
    {
        NewLevel = EAudio_DangerLevel::Threatened;
    }
    else if (ClosestPredatorDistance < DangerConfig.TriggerRadius)
    {
        NewLevel = EAudio_DangerLevel::Aware;
    }

    SetDangerLevel(NewLevel);
}

void AAudio_SystemManager::TriggerDinosaurFootstepShake(float Intensity, float Distance)
{
    if (Distance <= 0.0f) return;

    // Attenuate shake by distance — full shake at 200 units, zero at 2000
    const float MaxShakeDistance = 2000.0f;
    const float AttenuatedIntensity = Intensity * FMath::Clamp(1.0f - (Distance / MaxShakeDistance), 0.0f, 1.0f);

    if (AttenuatedIntensity > 0.05f)
    {
        ApplyScreenShake(AttenuatedIntensity);
        UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Footstep shake — intensity=%.2f dist=%.0f"), AttenuatedIntensity, Distance);
    }
}

void AAudio_SystemManager::TriggerCampfireAmbience(bool bEnable)
{
    // Campfire ambience toggle — in production, this would fade in/out a looping campfire audio component
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Campfire ambience %s"), bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void AAudio_SystemManager::TriggerDamageAudioFeedback(float DamageAmount)
{
    // Damage audio feedback — pitch/volume spike proportional to damage
    const float NormalisedDamage = FMath::Clamp(DamageAmount / 100.0f, 0.0f, 1.0f);
    const float ShakeIntensity = NormalisedDamage * 2.0f;
    ApplyScreenShake(ShakeIntensity);

    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Damage audio feedback — damage=%.1f shake=%.2f"), DamageAmount, ShakeIntensity);
}

void AAudio_SystemManager::SetNightMode(bool bIsNight)
{
    if (bIsNightMode == bIsNight) return;
    bIsNightMode = bIsNight;
    UpdateAmbientLayersForDanger();
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Night mode %s"), bIsNight ? TEXT("ON") : TEXT("OFF"));
}

void AAudio_SystemManager::PlayNarrationLine(const FString& NarrationID)
{
    // Narration playback — in production, looks up audio asset from data table by NarrationID
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Playing narration line '%s'"), *NarrationID);
}

void AAudio_SystemManager::UpdateAmbientLayersForDanger()
{
    // Adjust volume of all active ambient components based on current danger level
    for (UAudioComponent* Comp : ActiveAmbientComponents)
    {
        if (!Comp) continue;
        const float AdjustedVolume = ComputeVolumeForDanger(1.0f, CurrentDangerLevel);
        Comp->SetVolumeMultiplier(AdjustedVolume);
    }
}

void AAudio_SystemManager::ApplyScreenShake(float Intensity)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    // Screen shake via PlayerController — uses built-in camera shake system
    // In production, load a UCameraShakeBase subclass from content
    // For now, log the shake trigger for Blueprint hookup
    UE_LOG(LogTemp, Log, TEXT("AudioSystemManager: Screen shake triggered — intensity=%.2f"), Intensity);
}

float AAudio_SystemManager::ComputeVolumeForDanger(float BaseVolume, EAudio_DangerLevel Level) const
{
    switch (Level)
    {
        case EAudio_DangerLevel::Safe:       return BaseVolume * 1.0f;
        case EAudio_DangerLevel::Aware:      return BaseVolume * 0.7f;
        case EAudio_DangerLevel::Threatened: return BaseVolume * 0.4f;
        case EAudio_DangerLevel::Critical:   return BaseVolume * 0.15f;
        default:                             return BaseVolume;
    }
}
