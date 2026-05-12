#include "Audio_PolishEffectsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Components/AudioComponent.h"

AAudio_PolishEffectsManager::AAudio_PolishEffectsManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create audio components
    FootstepAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FootstepAudio"));
    FootstepAudioComponent->bAutoActivate = false;
    FootstepAudioComponent->SetVolumeMultiplier(0.7f);

    ImpactAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ImpactAudio"));
    ImpactAudioComponent->bAutoActivate = false;
    ImpactAudioComponent->SetVolumeMultiplier(0.8f);

    EnvironmentalAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EnvironmentalAudio"));
    EnvironmentalAudioComponent->bAutoActivate = false;
    EnvironmentalAudioComponent->SetVolumeMultiplier(0.6f);

    RootComponent = FootstepAudioComponent;
}

void AAudio_PolishEffectsManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeScreenShakeSettings();
    InitializeDamageFlashSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio Polish Effects Manager initialized"));
}

void AAudio_PolishEffectsManager::InitializeScreenShakeSettings()
{
    // Low intensity shake
    FAudio_ScreenShakeData LowShake;
    LowShake.Intensity = 0.3f;
    LowShake.Duration = 0.2f;
    LowShake.MaxDistance = 1000.0f;
    ScreenShakeSettings.Add(EAudio_EffectIntensity::Low, LowShake);

    // Medium intensity shake
    FAudio_ScreenShakeData MediumShake;
    MediumShake.Intensity = 0.6f;
    MediumShake.Duration = 0.4f;
    MediumShake.MaxDistance = 1500.0f;
    ScreenShakeSettings.Add(EAudio_EffectIntensity::Medium, MediumShake);

    // High intensity shake
    FAudio_ScreenShakeData HighShake;
    HighShake.Intensity = 0.9f;
    HighShake.Duration = 0.6f;
    HighShake.MaxDistance = 2000.0f;
    ScreenShakeSettings.Add(EAudio_EffectIntensity::High, HighShake);

    // Extreme intensity shake
    FAudio_ScreenShakeData ExtremeShake;
    ExtremeShake.Intensity = 1.2f;
    ExtremeShake.Duration = 0.8f;
    ExtremeShake.MaxDistance = 2500.0f;
    ScreenShakeSettings.Add(EAudio_EffectIntensity::Extreme, ExtremeShake);
}

void AAudio_PolishEffectsManager::InitializeDamageFlashSettings()
{
    // Low damage flash
    FAudio_DamageFlashData LowFlash;
    LowFlash.FlashColor = FLinearColor(1.0f, 0.8f, 0.0f, 0.3f); // Yellow tint
    LowFlash.FlashIntensity = 0.4f;
    LowFlash.FlashDuration = 0.2f;
    LowFlash.FadeOutTime = 0.15f;
    DamageFlashSettings.Add(EAudio_EffectIntensity::Low, LowFlash);

    // Medium damage flash
    FAudio_DamageFlashData MediumFlash;
    MediumFlash.FlashColor = FLinearColor(1.0f, 0.5f, 0.0f, 0.5f); // Orange tint
    MediumFlash.FlashIntensity = 0.6f;
    MediumFlash.FlashDuration = 0.3f;
    MediumFlash.FadeOutTime = 0.2f;
    DamageFlashSettings.Add(EAudio_EffectIntensity::Medium, MediumFlash);

    // High damage flash
    FAudio_DamageFlashData HighFlash;
    HighFlash.FlashColor = FLinearColor(1.0f, 0.2f, 0.0f, 0.7f); // Red-orange tint
    HighFlash.FlashIntensity = 0.8f;
    HighFlash.FlashDuration = 0.4f;
    HighFlash.FadeOutTime = 0.25f;
    DamageFlashSettings.Add(EAudio_EffectIntensity::High, HighFlash);

    // Extreme damage flash
    FAudio_DamageFlashData ExtremeFlash;
    ExtremeFlash.FlashColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.9f); // Pure red
    ExtremeFlash.FlashIntensity = 1.0f;
    ExtremeFlash.FlashDuration = 0.5f;
    ExtremeFlash.FadeOutTime = 0.3f;
    DamageFlashSettings.Add(EAudio_EffectIntensity::Extreme, ExtremeFlash);
}

void AAudio_PolishEffectsManager::TriggerScreenShake(EAudio_EffectIntensity Intensity, FVector SourceLocation)
{
    if (!ScreenShakeSettings.Contains(Intensity))
    {
        UE_LOG(LogTemp, Warning, TEXT("Screen shake intensity not found: %d"), (int32)Intensity);
        return;
    }

    const FAudio_ScreenShakeData& ShakeData = ScreenShakeSettings[Intensity];
    
    // Calculate distance-based intensity
    float DistanceIntensity = CalculateDistanceIntensity(SourceLocation, ShakeData.MaxDistance);
    if (DistanceIntensity <= 0.0f)
    {
        return; // Too far away
    }

    // Apply intensity multiplier
    float FinalIntensity = ShakeData.Intensity * DistanceIntensity * EffectIntensityMultiplier;

    // Get player controller
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController)
    {
        // Use a basic camera shake if no specific class is set
        if (ShakeData.ShakeClass)
        {
            PlayerController->ClientStartCameraShake(ShakeData.ShakeClass, FinalIntensity);
        }
        else
        {
            // Create a simple camera shake effect
            UE_LOG(LogTemp, Log, TEXT("Triggering screen shake - Intensity: %f, Duration: %f"), 
                   FinalIntensity, ShakeData.Duration);
        }
    }
}

void AAudio_PolishEffectsManager::TriggerDinosaurFootstepShake(FVector DinosaurLocation, float DinosaurMass)
{
    // Determine intensity based on dinosaur mass
    EAudio_EffectIntensity Intensity = EAudio_EffectIntensity::Low;
    
    if (DinosaurMass > 8000.0f) // T-Rex size
    {
        Intensity = EAudio_EffectIntensity::Extreme;
    }
    else if (DinosaurMass > 4000.0f) // Large herbivore
    {
        Intensity = EAudio_EffectIntensity::High;
    }
    else if (DinosaurMass > 1000.0f) // Medium dinosaur
    {
        Intensity = EAudio_EffectIntensity::Medium;
    }

    TriggerScreenShake(Intensity, DinosaurLocation);
    PlayFootstepAudio(DinosaurLocation, DinosaurMass / 10000.0f);
}

void AAudio_PolishEffectsManager::TriggerImpactShake(FVector ImpactLocation, float ImpactForce)
{
    // Determine intensity based on impact force
    EAudio_EffectIntensity Intensity = EAudio_EffectIntensity::Low;
    
    if (ImpactForce > 5000.0f)
    {
        Intensity = EAudio_EffectIntensity::Extreme;
    }
    else if (ImpactForce > 2000.0f)
    {
        Intensity = EAudio_EffectIntensity::High;
    }
    else if (ImpactForce > 500.0f)
    {
        Intensity = EAudio_EffectIntensity::Medium;
    }

    TriggerScreenShake(Intensity, ImpactLocation);
    PlayImpactAudio(ImpactLocation, ImpactForce / 1000.0f);
}

void AAudio_PolishEffectsManager::TriggerDamageFlash(EAudio_EffectIntensity Intensity, float DamageAmount)
{
    if (!DamageFlashSettings.Contains(Intensity))
    {
        UE_LOG(LogTemp, Warning, TEXT("Damage flash intensity not found: %d"), (int32)Intensity);
        return;
    }

    if (bDamageFlashActive)
    {
        // Stop current flash
        GetWorld()->GetTimerManager().ClearTimer(DamageFlashTimer);
    }

    const FAudio_DamageFlashData& FlashData = DamageFlashSettings[Intensity];
    ApplyDamageFlashToViewport(FlashData);
    
    bDamageFlashActive = true;
    
    // Set timer to stop flash
    GetWorld()->GetTimerManager().SetTimer(
        DamageFlashTimer,
        this,
        &AAudio_PolishEffectsManager::StopDamageFlash,
        FlashData.FlashDuration,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("Damage flash triggered - Intensity: %d, Damage: %f"), 
           (int32)Intensity, DamageAmount);
}

void AAudio_PolishEffectsManager::TriggerHealthWarningFlash()
{
    TriggerDamageFlash(EAudio_EffectIntensity::Medium, 0.0f);
}

void AAudio_PolishEffectsManager::TriggerCriticalDamageFlash()
{
    TriggerDamageFlash(EAudio_EffectIntensity::Extreme, 100.0f);
}

void AAudio_PolishEffectsManager::PlayFootstepAudio(FVector Location, float Volume)
{
    if (FootstepAudioComponent)
    {
        FootstepAudioComponent->SetWorldLocation(Location);
        FootstepAudioComponent->SetVolumeMultiplier(FMath::Clamp(Volume, 0.1f, 1.0f));
        FootstepAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Playing footstep audio at location: %s, Volume: %f"), 
               *Location.ToString(), Volume);
    }
}

void AAudio_PolishEffectsManager::PlayImpactAudio(FVector Location, float Intensity)
{
    if (ImpactAudioComponent)
    {
        ImpactAudioComponent->SetWorldLocation(Location);
        ImpactAudioComponent->SetVolumeMultiplier(FMath::Clamp(Intensity, 0.2f, 1.0f));
        ImpactAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Playing impact audio at location: %s, Intensity: %f"), 
               *Location.ToString(), Intensity);
    }
}

void AAudio_PolishEffectsManager::PlayEnvironmentalAudio(FVector Location, float Range)
{
    if (EnvironmentalAudioComponent)
    {
        EnvironmentalAudioComponent->SetWorldLocation(Location);
        EnvironmentalAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Playing environmental audio at location: %s, Range: %f"), 
               *Location.ToString(), Range);
    }
}

float AAudio_PolishEffectsManager::CalculateDistanceIntensity(FVector SourceLocation, float MaxDistance)
{
    // Get player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), SourceLocation);
    
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }

    // Linear falloff
    return 1.0f - (Distance / MaxDistance);
}

void AAudio_PolishEffectsManager::SetEffectIntensityMultiplier(float Multiplier)
{
    EffectIntensityMultiplier = FMath::Clamp(Multiplier, 0.0f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("Effect intensity multiplier set to: %f"), EffectIntensityMultiplier);
}

void AAudio_PolishEffectsManager::StopDamageFlash()
{
    bDamageFlashActive = false;
    
    // Remove damage flash from viewport
    if (GEngine && GEngine->GameViewport)
    {
        // Reset viewport overlay color
        UE_LOG(LogTemp, Log, TEXT("Stopping damage flash effect"));
    }
}

void AAudio_PolishEffectsManager::ApplyDamageFlashToViewport(const FAudio_DamageFlashData& FlashData)
{
    // Apply damage flash effect to viewport
    if (GEngine && GEngine->GameViewport)
    {
        // This would typically involve setting a viewport overlay or post-process effect
        UE_LOG(LogTemp, Log, TEXT("Applying damage flash - Color: R:%f G:%f B:%f A:%f"), 
               FlashData.FlashColor.R, FlashData.FlashColor.G, 
               FlashData.FlashColor.B, FlashData.FlashColor.A);
    }
}