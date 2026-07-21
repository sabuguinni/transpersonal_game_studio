#include "Audio_ImmersivePolishSystem.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

AAudio_ImmersivePolishSystem::AAudio_ImmersivePolishSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    EffectsAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EffectsAudioComponent"));
    EffectsAudioComponent->SetupAttachment(RootComponent);
    EffectsAudioComponent->bAutoActivate = false;

    VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));
    VoiceAudioComponent->SetupAttachment(RootComponent);
    VoiceAudioComponent->bAutoActivate = false;

    // Initialize default values
    CurrentImmersionLevel = EAudio_ImmersionLevel::Standard;
    TargetImmersionLevel = EAudio_ImmersionLevel::Standard;
    
    bEnableScreenShakeAudio = true;
    bEnableDamageFlashAudio = true;
    bEnableFootstepParticleAudio = true;
    bEnableDayNightCycleAudio = true;

    PolishEffectUpdateInterval = 0.1f;
    ImmersionTransitionDuration = 2.0f;
    
    bIsSystemActive = false;
    CurrentTransitionTime = 0.0f;

    // Initialize immersive settings
    ImmersiveSettings = FAudio_ImmersiveSettings();
}

void AAudio_ImmersivePolishSystem::BeginPlay()
{
    Super::BeginPlay();

    // Apply initial immersive settings
    ApplyImmersiveSettings();
    
    // Start the polish effects system
    StartPolishEffectsSystem();

    UE_LOG(LogTemp, Log, TEXT("Audio_ImmersivePolishSystem: System initialized with immersion level %d"), 
           static_cast<int32>(CurrentImmersionLevel));
}

void AAudio_ImmersivePolishSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsSystemActive)
    {
        // Handle immersion level transitions
        if (CurrentImmersionLevel != TargetImmersionLevel)
        {
            CurrentTransitionTime += DeltaTime;
            if (CurrentTransitionTime >= ImmersionTransitionDuration)
            {
                TransitionImmersionLevel();
            }
        }

        // Update ambient audio based on current state
        UpdateAmbientAudio();
    }
}

void AAudio_ImmersivePolishSystem::SetImmersionLevel(EAudio_ImmersionLevel NewLevel)
{
    if (NewLevel != CurrentImmersionLevel)
    {
        TargetImmersionLevel = NewLevel;
        CurrentTransitionTime = 0.0f;
        PreviousSettings = ImmersiveSettings;

        UE_LOG(LogTemp, Log, TEXT("Audio_ImmersivePolishSystem: Transitioning from level %d to %d"), 
               static_cast<int32>(CurrentImmersionLevel), static_cast<int32>(NewLevel));
    }
}

void AAudio_ImmersivePolishSystem::PlayTRexProximityAlert(FVector Location, float Intensity)
{
    if (!bIsSystemActive || !TRexProximityAlert)
    {
        return;
    }

    // Calculate distance-based volume
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
        float VolumeMultiplier = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.1f, 1.0f);
        VolumeMultiplier *= Intensity * ImmersiveSettings.VoiceVolumeMultiplier;

        VoiceAudioComponent->SetSound(TRexProximityAlert);
        VoiceAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
        VoiceAudioComponent->SetWorldLocation(Location);
        VoiceAudioComponent->Play();

        UE_LOG(LogTemp, Warning, TEXT("Audio_ImmersivePolishSystem: T-Rex proximity alert triggered at distance %.1f with volume %.2f"), 
               Distance, VolumeMultiplier);
    }
}

void AAudio_ImmersivePolishSystem::PlayMedicalEmergencyAlert(float UrgencyLevel)
{
    if (!bIsSystemActive || !MedicalEmergencyAlert)
    {
        return;
    }

    float VolumeMultiplier = FMath::Clamp(UrgencyLevel * ImmersiveSettings.VoiceVolumeMultiplier, 0.5f, 2.0f);
    
    VoiceAudioComponent->SetSound(MedicalEmergencyAlert);
    VoiceAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
    VoiceAudioComponent->Play();

    UE_LOG(LogTemp, Warning, TEXT("Audio_ImmersivePolishSystem: Medical emergency alert triggered with urgency %.2f"), 
           UrgencyLevel);
}

void AAudio_ImmersivePolishSystem::TriggerScreenShakeAudio(float ShakeIntensity, float Duration)
{
    if (!bIsSystemActive || !bEnableScreenShakeAudio || !ScreenShakeRumbleSFX)
    {
        return;
    }

    float VolumeMultiplier = FMath::Clamp(ShakeIntensity * ImmersiveSettings.EffectsVolumeMultiplier, 0.1f, 1.5f);
    
    EffectsAudioComponent->SetSound(ScreenShakeRumbleSFX);
    EffectsAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
    EffectsAudioComponent->Play();

    // Schedule stop after duration
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this]() { EffectsAudioComponent->FadeOut(0.5f, 0.0f); },
        Duration,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("Audio_ImmersivePolishSystem: Screen shake audio triggered with intensity %.2f for %.1fs"), 
           ShakeIntensity, Duration);
}

void AAudio_ImmersivePolishSystem::TriggerDamageFlashAudio(float DamageAmount)
{
    if (!bIsSystemActive || !bEnableDamageFlashAudio)
    {
        return;
    }

    // Play a brief audio cue for damage feedback
    float VolumeMultiplier = FMath::Clamp(DamageAmount / 100.0f * ImmersiveSettings.EffectsVolumeMultiplier, 0.2f, 1.0f);
    
    if (ScreenShakeRumbleSFX) // Reuse rumble sound for damage flash
    {
        EffectsAudioComponent->SetSound(ScreenShakeRumbleSFX);
        EffectsAudioComponent->SetVolumeMultiplier(VolumeMultiplier * 0.5f); // Quieter for damage
        EffectsAudioComponent->SetPitchMultiplier(1.5f); // Higher pitch for damage
        EffectsAudioComponent->Play();

        // Reset pitch after playing
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]() { EffectsAudioComponent->SetPitchMultiplier(1.0f); },
            0.5f,
            false
        );
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_ImmersivePolishSystem: Damage flash audio triggered for %.1f damage"), 
           DamageAmount);
}

void AAudio_ImmersivePolishSystem::PlayFootstepParticleAudio(FVector FootstepLocation, float ParticleIntensity)
{
    if (!bIsSystemActive || !bEnableFootstepParticleAudio || !FootstepDustSFX)
    {
        return;
    }

    float VolumeMultiplier = FMath::Clamp(ParticleIntensity * ImmersiveSettings.EffectsVolumeMultiplier, 0.1f, 0.8f);
    
    EffectsAudioComponent->SetSound(FootstepDustSFX);
    EffectsAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
    EffectsAudioComponent->SetWorldLocation(FootstepLocation);
    EffectsAudioComponent->Play();

    UE_LOG(LogTemp, VeryVerbose, TEXT("Audio_ImmersivePolishSystem: Footstep particle audio at location %s with intensity %.2f"), 
           *FootstepLocation.ToString(), ParticleIntensity);
}

void AAudio_ImmersivePolishSystem::UpdateDayNightCycleAudio(float TimeOfDay)
{
    if (!bIsSystemActive || !bEnableDayNightCycleAudio)
    {
        return;
    }

    // Adjust ambient audio based on time of day
    float NightIntensity = FMath::Sin(TimeOfDay * PI); // 0 = midnight, 1 = noon
    float AmbientVolume = FMath::Lerp(0.3f, 1.0f, NightIntensity) * ImmersiveSettings.AmbientVolumeMultiplier;
    
    if (AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume);
    }

    UE_LOG(LogTemp, VeryVerbose, TEXT("Audio_ImmersivePolishSystem: Day/night cycle audio updated - TimeOfDay: %.2f, Volume: %.2f"), 
           TimeOfDay, AmbientVolume);
}

void AAudio_ImmersivePolishSystem::StartPolishEffectsSystem()
{
    if (bIsSystemActive)
    {
        return;
    }

    bIsSystemActive = true;

    // Start the update timer
    GetWorld()->GetTimerManager().SetTimer(
        PolishEffectTimer,
        this,
        &AAudio_ImmersivePolishSystem::UpdatePolishEffects,
        PolishEffectUpdateInterval,
        true
    );

    // Start ambient audio if available
    if (PrehistoricAmbientLoop)
    {
        AmbientAudioComponent->SetSound(PrehistoricAmbientLoop);
        AmbientAudioComponent->SetVolumeMultiplier(ImmersiveSettings.AmbientVolumeMultiplier);
        AmbientAudioComponent->Play();
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_ImmersivePolishSystem: Polish effects system started"));
}

void AAudio_ImmersivePolishSystem::StopPolishEffectsSystem()
{
    if (!bIsSystemActive)
    {
        return;
    }

    bIsSystemActive = false;

    // Clear timers
    GetWorld()->GetTimerManager().ClearTimer(PolishEffectTimer);
    GetWorld()->GetTimerManager().ClearTimer(ImmersionTransitionTimer);

    // Stop all audio components
    AmbientAudioComponent->Stop();
    EffectsAudioComponent->Stop();
    VoiceAudioComponent->Stop();

    UE_LOG(LogTemp, Log, TEXT("Audio_ImmersivePolishSystem: Polish effects system stopped"));
}

void AAudio_ImmersivePolishSystem::ResetAllPolishEffects()
{
    // Reset to default immersion level
    CurrentImmersionLevel = EAudio_ImmersionLevel::Standard;
    TargetImmersionLevel = EAudio_ImmersionLevel::Standard;
    CurrentTransitionTime = 0.0f;

    // Reset settings
    ImmersiveSettings = FAudio_ImmersiveSettings();
    ApplyImmersiveSettings();

    UE_LOG(LogTemp, Log, TEXT("Audio_ImmersivePolishSystem: All polish effects reset to defaults"));
}

bool AAudio_ImmersivePolishSystem::IsPolishEffectsSystemActive() const
{
    return bIsSystemActive;
}

void AAudio_ImmersivePolishSystem::UpdatePolishEffects()
{
    if (!bIsSystemActive)
    {
        return;
    }

    // Update various polish effects based on game state
    UpdateAmbientAudio();
    UpdateEffectsAudio();
    UpdateVoiceAudio();
}

void AAudio_ImmersivePolishSystem::TransitionImmersionLevel()
{
    CurrentImmersionLevel = TargetImmersionLevel;
    CurrentTransitionTime = 0.0f;

    // Update settings based on new immersion level
    switch (CurrentImmersionLevel)
    {
        case EAudio_ImmersionLevel::Minimal:
            ImmersiveSettings.AmbientVolumeMultiplier = 0.3f;
            ImmersiveSettings.EffectsVolumeMultiplier = 0.5f;
            ImmersiveSettings.VoiceVolumeMultiplier = 0.7f;
            ImmersiveSettings.ReverbIntensity = 0.2f;
            break;
        case EAudio_ImmersionLevel::Standard:
            ImmersiveSettings.AmbientVolumeMultiplier = 0.7f;
            ImmersiveSettings.EffectsVolumeMultiplier = 0.8f;
            ImmersiveSettings.VoiceVolumeMultiplier = 1.0f;
            ImmersiveSettings.ReverbIntensity = 0.5f;
            break;
        case EAudio_ImmersionLevel::Enhanced:
            ImmersiveSettings.AmbientVolumeMultiplier = 1.0f;
            ImmersiveSettings.EffectsVolumeMultiplier = 1.2f;
            ImmersiveSettings.VoiceVolumeMultiplier = 1.2f;
            ImmersiveSettings.ReverbIntensity = 0.7f;
            break;
        case EAudio_ImmersionLevel::Maximum:
            ImmersiveSettings.AmbientVolumeMultiplier = 1.3f;
            ImmersiveSettings.EffectsVolumeMultiplier = 1.5f;
            ImmersiveSettings.VoiceVolumeMultiplier = 1.5f;
            ImmersiveSettings.ReverbIntensity = 1.0f;
            break;
    }

    ApplyImmersiveSettings();

    UE_LOG(LogTemp, Log, TEXT("Audio_ImmersivePolishSystem: Immersion level transition completed to level %d"), 
           static_cast<int32>(CurrentImmersionLevel));
}

void AAudio_ImmersivePolishSystem::ApplyImmersiveSettings()
{
    // Apply settings to audio components
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(ImmersiveSettings.AmbientVolumeMultiplier);
    }
    
    if (EffectsAudioComponent)
    {
        EffectsAudioComponent->SetVolumeMultiplier(ImmersiveSettings.EffectsVolumeMultiplier);
    }
    
    if (VoiceAudioComponent)
    {
        VoiceAudioComponent->SetVolumeMultiplier(ImmersiveSettings.VoiceVolumeMultiplier);
    }
}

void AAudio_ImmersivePolishSystem::UpdateAmbientAudio()
{
    // Update ambient audio based on current game state
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        // Adjust volume based on current immersion settings
        AmbientAudioComponent->SetVolumeMultiplier(ImmersiveSettings.AmbientVolumeMultiplier);
    }
}

void AAudio_ImmersivePolishSystem::UpdateEffectsAudio()
{
    // Update effects audio processing
    if (EffectsAudioComponent)
    {
        // Apply current effects volume multiplier
        EffectsAudioComponent->SetVolumeMultiplier(ImmersiveSettings.EffectsVolumeMultiplier);
    }
}

void AAudio_ImmersivePolishSystem::UpdateVoiceAudio()
{
    // Update voice audio processing
    if (VoiceAudioComponent)
    {
        // Apply current voice volume multiplier
        VoiceAudioComponent->SetVolumeMultiplier(ImmersiveSettings.VoiceVolumeMultiplier);
    }
}