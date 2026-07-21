#include "AudioFeedbackManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/LegacyCameraShake.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void UAudioFeedbackManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize screen shake presets
    TRexShakeData.Intensity = 3.0f;
    TRexShakeData.Duration = 1.5f;
    TRexShakeData.Range = 5000.0f;
    TRexShakeData.bFadeOut = true;

    FootstepShakeData.Intensity = 1.0f;
    FootstepShakeData.Duration = 0.3f;
    FootstepShakeData.Range = 2000.0f;
    FootstepShakeData.bFadeOut = true;

    DamageShakeData.Intensity = 2.0f;
    DamageShakeData.Duration = 0.8f;
    DamageShakeData.Range = 500.0f;
    DamageShakeData.bFadeOut = true;

    // Initialize damage flash settings
    DefaultDamageFlash.FlashColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.8f);
    DefaultDamageFlash.FlashDuration = 0.3f;
    DefaultDamageFlash.FlashIntensity = 0.8f;

    // Initialize audio components
    InitializeAudioComponents();
    SetupBiomeAudioAssets();

    UE_LOG(LogTemp, Warning, TEXT("AudioFeedbackManager initialized successfully"));
}

void UAudioFeedbackManager::Deinitialize()
{
    if (AmbientAudioComponent && IsValid(AmbientAudioComponent))
    {
        AmbientAudioComponent->Stop();
        AmbientAudioComponent = nullptr;
    }

    if (ProximityWarningComponent && IsValid(ProximityWarningComponent))
    {
        ProximityWarningComponent->Stop();
        ProximityWarningComponent = nullptr;
    }

    if (DamageAudioComponent && IsValid(DamageAudioComponent))
    {
        DamageAudioComponent->Stop();
        DamageAudioComponent = nullptr;
    }

    Super::Deinitialize();
}

void UAudioFeedbackManager::TriggerScreenShake(const FVector& EpicenterLocation, const FAudio_ScreenShakeData& ShakeData)
{
    APlayerController* PC = GetPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioFeedbackManager: No PlayerController found for screen shake"));
        return;
    }

    // Calculate distance to player
    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), EpicenterLocation);
    if (Distance > ShakeData.Range)
    {
        return; // Too far away
    }

    // Calculate intensity based on distance
    float DistanceRatio = 1.0f - (Distance / ShakeData.Range);
    float AdjustedIntensity = ShakeData.Intensity * DistanceRatio;

    // Create and play camera shake
    if (AdjustedIntensity > 0.1f)
    {
        // Use legacy camera shake for compatibility
        PC->ClientStartCameraShake(ULegacyCameraShake::StaticClass(), AdjustedIntensity);
        
        UE_LOG(LogTemp, Log, TEXT("Screen shake triggered: Intensity=%.2f, Distance=%.2f"), 
               AdjustedIntensity, Distance);
    }
}

void UAudioFeedbackManager::TriggerTRexProximityShake(const FVector& TRexLocation)
{
    TriggerScreenShake(TRexLocation, TRexShakeData);
    PlayProximityWarningSound(TRexLocation);
}

void UAudioFeedbackManager::TriggerFootstepShake(const FVector& FootstepLocation, float CreatureSize)
{
    FAudio_ScreenShakeData ScaledShake = FootstepShakeData;
    ScaledShake.Intensity *= CreatureSize;
    ScaledShake.Range *= CreatureSize;
    
    TriggerScreenShake(FootstepLocation, ScaledShake);
}

void UAudioFeedbackManager::TriggerDamageFlash(float DamageAmount)
{
    if (bDamageFlashActive)
    {
        return; // Already flashing
    }

    bDamageFlashActive = true;

    // Scale flash intensity based on damage
    FAudio_DamageFlashData ScaledFlash = DefaultDamageFlash;
    ScaledFlash.FlashIntensity = FMath::Clamp(DamageAmount / 50.0f, 0.2f, 1.0f);

    // Trigger screen shake for damage
    APlayerController* PC = GetPlayerController();
    if (PC && PC->GetPawn())
    {
        TriggerScreenShake(PC->GetPawn()->GetActorLocation(), DamageShakeData);
    }

    // Reset flash state after duration
    FTimerHandle FlashTimer;
    GetWorld()->GetTimerManager().SetTimer(FlashTimer, [this]()
    {
        bDamageFlashActive = false;
    }, ScaledFlash.FlashDuration, false);

    UE_LOG(LogTemp, Log, TEXT("Damage flash triggered: Amount=%.2f, Intensity=%.2f"), 
           DamageAmount, ScaledFlash.FlashIntensity);
}

void UAudioFeedbackManager::TriggerPainVoiceover(float PainIntensity)
{
    if (DamageAudioComponent && DamageSoundCue)
    {
        DamageAudioComponent->SetVolumeMultiplier(FMath::Clamp(PainIntensity / 100.0f, 0.3f, 1.0f));
        DamageAudioComponent->SetSound(DamageSoundCue);
        DamageAudioComponent->Play();
    }
}

void UAudioFeedbackManager::PlayProximityWarningSound(const FVector& ThreatLocation)
{
    if (ProximityWarningComponent && TRexProximitySoundCue)
    {
        ProximityWarningComponent->SetWorldLocation(ThreatLocation);
        ProximityWarningComponent->SetSound(TRexProximitySoundCue);
        ProximityWarningComponent->Play();
    }
}

void UAudioFeedbackManager::PlayDamageAudioCue(EDamageType DamageType)
{
    if (DamageAudioComponent && DamageSoundCue)
    {
        // Adjust pitch based on damage type
        float Pitch = 1.0f;
        switch (DamageType)
        {
            case EDamageType::Claw:
                Pitch = 1.2f;
                break;
            case EDamageType::Bite:
                Pitch = 0.8f;
                break;
            case EDamageType::Crush:
                Pitch = 0.6f;
                break;
            default:
                Pitch = 1.0f;
                break;
        }

        DamageAudioComponent->SetPitchMultiplier(Pitch);
        DamageAudioComponent->SetSound(DamageSoundCue);
        DamageAudioComponent->Play();
    }
}

void UAudioFeedbackManager::UpdateBiomeAmbientAudio(EBiomeType NewBiome)
{
    if (CurrentBiome == NewBiome)
    {
        return; // No change
    }

    CurrentBiome = NewBiome;

    if (AmbientAudioComponent)
    {
        // Stop current ambient audio
        AmbientAudioComponent->FadeOut(1.0f, 0.0f);

        // Find new biome sound
        USoundCue** BiomeSound = BiomeAmbientSounds.Find(NewBiome);
        if (BiomeSound && *BiomeSound)
        {
            AmbientAudioComponent->SetSound(*BiomeSound);
            AmbientAudioComponent->FadeIn(1.0f, CurrentAmbientVolume);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Biome ambient audio updated to: %d"), (int32)NewBiome);
}

void UAudioFeedbackManager::SetAmbientAudioVolume(float Volume)
{
    CurrentAmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(CurrentAmbientVolume);
    }
}

void UAudioFeedbackManager::InitializeAudioComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Create ambient audio component
    AmbientAudioComponent = NewObject<UAudioComponent>(this);
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->SetVolumeMultiplier(CurrentAmbientVolume);
    }

    // Create proximity warning component
    ProximityWarningComponent = NewObject<UAudioComponent>(this);
    if (ProximityWarningComponent)
    {
        ProximityWarningComponent->bAutoActivate = false;
        ProximityWarningComponent->SetVolumeMultiplier(0.8f);
    }

    // Create damage audio component
    DamageAudioComponent = NewObject<UAudioComponent>(this);
    if (DamageAudioComponent)
    {
        DamageAudioComponent->bAutoActivate = false;
        DamageAudioComponent->SetVolumeMultiplier(0.9f);
    }
}

void UAudioFeedbackManager::SetupBiomeAudioAssets()
{
    // Initialize biome ambient sounds map
    // These would be loaded from actual sound assets in a full implementation
    BiomeAmbientSounds.Empty();
    
    // For now, we'll set up the structure for future asset loading
    BiomeAmbientSounds.Add(EBiomeType::Savana, nullptr);
    BiomeAmbientSounds.Add(EBiomeType::Forest, nullptr);
    BiomeAmbientSounds.Add(EBiomeType::Desert, nullptr);
    BiomeAmbientSounds.Add(EBiomeType::Swamp, nullptr);
    BiomeAmbientSounds.Add(EBiomeType::Mountain, nullptr);
}

APlayerController* UAudioFeedbackManager::GetPlayerController() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    return World->GetFirstPlayerController();
}

UCameraShakeBase* UAudioFeedbackManager::CreateScreenShake(const FAudio_ScreenShakeData& ShakeData)
{
    // This would create a custom camera shake class in a full implementation
    // For now, we use the legacy system in TriggerScreenShake
    return nullptr;
}