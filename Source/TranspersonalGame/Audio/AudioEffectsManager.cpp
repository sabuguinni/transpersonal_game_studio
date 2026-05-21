#include "AudioEffectsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Math/UnrealMathUtility.h"

UAudioEffectsManager::UAudioEffectsManager()
{
    // Initialize default configurations
    DefaultShakeConfig.Intensity = 1.0f;
    DefaultShakeConfig.Duration = 0.5f;
    DefaultShakeConfig.Frequency = 10.0f;
    DefaultShakeConfig.MaxDistance = 2000.0f;

    DefaultFlashConfig.FlashColor = FLinearColor::Red;
    DefaultFlashConfig.FlashIntensity = 0.8f;
    DefaultFlashConfig.FlashDuration = 0.3f;
    DefaultFlashConfig.FadeOutTime = 0.2f;

    CurrentTimeOfDay = 12.0f;
    DayNightSpeed = 1.0f;
    CurrentBiome = TEXT("Savana");

    // Initialize audio components
    InitializeAudioComponents();
}

void UAudioEffectsManager::InitializeAudioComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Create ambient audio components
    DayAmbientComponent = NewObject<UAudioComponent>(this);
    if (DayAmbientComponent)
    {
        DayAmbientComponent->bAutoActivate = false;
        DayAmbientComponent->SetVolumeMultiplier(0.5f);
    }

    NightAmbientComponent = NewObject<UAudioComponent>(this);
    if (NightAmbientComponent)
    {
        NightAmbientComponent->bAutoActivate = false;
        NightAmbientComponent->SetVolumeMultiplier(0.3f);
    }

    WeatherAmbientComponent = NewObject<UAudioComponent>(this);
    if (WeatherAmbientComponent)
    {
        WeatherAmbientComponent->bAutoActivate = false;
        WeatherAmbientComponent->SetVolumeMultiplier(0.4f);
    }
}

void UAudioEffectsManager::TriggerProximityShake(FVector SourceLocation, const FAudio_ScreenShakeConfig& Config)
{
    APlayerController* PC = GetLocalPlayerController();
    if (!PC || !PC->PlayerCameraManager)
    {
        return;
    }

    // Calculate distance to player
    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float Distance = FVector::Dist(SourceLocation, PlayerLocation);

    if (Distance > Config.MaxDistance)
    {
        return;
    }

    // Calculate intensity based on distance
    float DistanceRatio = 1.0f - (Distance / Config.MaxDistance);
    float AdjustedIntensity = Config.Intensity * DistanceRatio;

    // Apply screen shake
    if (AdjustedIntensity > 0.1f)
    {
        PC->PlayerCameraManager->StartCameraShake(nullptr, AdjustedIntensity);
        
        UE_LOG(LogTemp, Log, TEXT("AudioEffectsManager: Proximity shake triggered - Distance: %f, Intensity: %f"), Distance, AdjustedIntensity);
    }
}

void UAudioEffectsManager::TriggerFootstepShake(FVector FootstepLocation, float DinosaurMass)
{
    // Calculate shake intensity based on dinosaur mass
    float BaseIntensity = FMath::Clamp(DinosaurMass / 10000.0f, 0.1f, 2.0f);
    
    FAudio_ScreenShakeConfig ShakeConfig;
    ShakeConfig.Intensity = BaseIntensity;
    ShakeConfig.Duration = 0.3f;
    ShakeConfig.Frequency = 8.0f;
    ShakeConfig.MaxDistance = 1500.0f;

    TriggerProximityShake(FootstepLocation, ShakeConfig);

    // Play footstep sound
    if (FootstepHeavySound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), FootstepHeavySound, FootstepLocation, 1.0f);
    }
}

void UAudioEffectsManager::TriggerDamageFlash(const FAudio_DamageFlashConfig& Config)
{
    APlayerController* PC = GetLocalPlayerController();
    if (!PC)
    {
        return;
    }

    // Create damage flash effect (simplified implementation)
    // In a full implementation, this would manipulate post-process materials
    UE_LOG(LogTemp, Warning, TEXT("AudioEffectsManager: Damage flash triggered - Color: R=%f G=%f B=%f, Intensity: %f"), 
           Config.FlashColor.R, Config.FlashColor.G, Config.FlashColor.B, Config.FlashIntensity);

    // Play damage audio
    if (DamageImpactSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), DamageImpactSound, 0.8f);
    }

    if (HeartbeatIntenseSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), HeartbeatIntenseSound, 0.6f);
    }
}

void UAudioEffectsManager::PlayDamageAudio(USoundBase* DamageSound, float VolumeMultiplier)
{
    if (DamageSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), DamageSound, VolumeMultiplier);
    }
}

void UAudioEffectsManager::SpawnFootstepDust(FVector Location, FVector Normal, float ParticleScale)
{
    UWorld* World = GetWorld();
    if (!World || !FootstepDustParticles)
    {
        return;
    }

    // Spawn dust particle system
    UGameplayStatics::SpawnEmitterAtLocation(
        World,
        FootstepDustParticles,
        Location,
        Normal.Rotation(),
        FVector(ParticleScale),
        true
    );

    UE_LOG(LogTemp, Log, TEXT("AudioEffectsManager: Footstep dust spawned at location: %s"), *Location.ToString());
}

void UAudioEffectsManager::UpdateAmbientAudio(float TimeOfDay, FString BiomeName)
{
    CurrentTimeOfDay = TimeOfDay;
    CurrentBiome = BiomeName;

    // Calculate day/night audio mixing
    float DayVolume = 0.0f;
    float NightVolume = 0.0f;

    if (TimeOfDay >= 6.0f && TimeOfDay <= 18.0f)
    {
        // Daytime (6 AM to 6 PM)
        DayVolume = 1.0f;
        NightVolume = 0.0f;
    }
    else if (TimeOfDay >= 18.0f && TimeOfDay <= 20.0f)
    {
        // Evening transition (6 PM to 8 PM)
        float TransitionFactor = (TimeOfDay - 18.0f) / 2.0f;
        DayVolume = 1.0f - TransitionFactor;
        NightVolume = TransitionFactor;
    }
    else if (TimeOfDay >= 4.0f && TimeOfDay <= 6.0f)
    {
        // Morning transition (4 AM to 6 AM)
        float TransitionFactor = (TimeOfDay - 4.0f) / 2.0f;
        DayVolume = TransitionFactor;
        NightVolume = 1.0f - TransitionFactor;
    }
    else
    {
        // Nighttime
        DayVolume = 0.0f;
        NightVolume = 1.0f;
    }

    // Apply volume changes
    if (DayAmbientComponent)
    {
        DayAmbientComponent->SetVolumeMultiplier(DayVolume * 0.5f);
    }

    if (NightAmbientComponent)
    {
        NightAmbientComponent->SetVolumeMultiplier(NightVolume * 0.3f);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioEffectsManager: Ambient audio updated - Time: %f, Biome: %s, Day Volume: %f, Night Volume: %f"), 
           TimeOfDay, *BiomeName, DayVolume, NightVolume);
}

void UAudioEffectsManager::SetDayNightCycleSpeed(float CycleSpeedMultiplier)
{
    DayNightSpeed = FMath::Clamp(CycleSpeedMultiplier, 0.1f, 10.0f);
    UE_LOG(LogTemp, Log, TEXT("AudioEffectsManager: Day/Night cycle speed set to: %f"), DayNightSpeed);
}

APlayerController* UAudioEffectsManager::GetLocalPlayerController() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return World->GetFirstPlayerController();
}

UWorld* UAudioEffectsManager::GetWorld() const
{
    if (HasAnyFlags(RF_ClassDefaultObject))
    {
        return nullptr;
    }

    UObject* Outer = GetOuter();
    while (Outer)
    {
        UWorld* World = Outer->GetWorld();
        if (World)
        {
            return World;
        }
        Outer = Outer->GetOuter();
    }

    return GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
}

float UAudioEffectsManager::CalculateDistanceAttenuation(FVector SourceLocation, FVector ListenerLocation, float MaxDistance) const
{
    float Distance = FVector::Dist(SourceLocation, ListenerLocation);
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    return 1.0f - (Distance / MaxDistance);
}