#include "Audio_ScreenShakeManager.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "Sound/SoundBase.h"

AAudio_ScreenShakeManager::AAudio_ScreenShakeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create audio component for shake sounds
    ShakeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ShakeAudioComponent"));
    ShakeAudioComponent->SetupAttachment(RootComponent);
    ShakeAudioComponent->bAutoActivate = false;

    // Initialize default values
    CachedPlayerController = nullptr;
}

void AAudio_ScreenShakeManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeShakePresets();
    
    // Cache player controller
    CachedPlayerController = UGameplayStatics::GetPlayerController(this, 0);
}

void AAudio_ScreenShakeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAudio_ScreenShakeManager::InitializeShakePresets()
{
    // Light shake for small dinosaurs or distant impacts
    FAudio_ShakeSettings LightShake;
    LightShake.Duration = 0.3f;
    LightShake.Amplitude = 0.2f;
    LightShake.Frequency = 8.0f;
    LightShake.FalloffRadius = 500.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Light, LightShake);

    // Medium shake for medium dinosaurs
    FAudio_ShakeSettings MediumShake;
    MediumShake.Duration = 0.6f;
    MediumShake.Amplitude = 0.5f;
    MediumShake.Frequency = 12.0f;
    MediumShake.FalloffRadius = 800.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Medium, MediumShake);

    // Heavy shake for T-Rex footsteps
    FAudio_ShakeSettings HeavyShake;
    HeavyShake.Duration = 1.0f;
    HeavyShake.Amplitude = 1.0f;
    HeavyShake.Frequency = 15.0f;
    HeavyShake.FalloffRadius = 1200.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Heavy, HeavyShake);

    // Extreme shake for T-Rex attacks or massive impacts
    FAudio_ShakeSettings ExtremeShake;
    ExtremeShake.Duration = 1.5f;
    ExtremeShake.Amplitude = 2.0f;
    ExtremeShake.Frequency = 20.0f;
    ExtremeShake.FalloffRadius = 1500.0f;
    ShakePresets.Add(EAudio_ShakeIntensity::Extreme, ExtremeShake);
}

void AAudio_ScreenShakeManager::TriggerScreenShake(EAudio_ShakeIntensity Intensity, FVector ShakeLocation)
{
    if (!CachedPlayerController)
    {
        CachedPlayerController = UGameplayStatics::GetPlayerController(this, 0);
        if (!CachedPlayerController)
        {
            return;
        }
    }

    if (!ShakePresets.Contains(Intensity))
    {
        return;
    }

    FAudio_ShakeSettings ShakeSettings = ShakePresets[Intensity];
    
    // Calculate distance-based intensity
    APawn* PlayerPawn = CachedPlayerController->GetPawn();
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), ShakeLocation);
        float IntensityMultiplier = CalculateShakeIntensityByDistance(ShakeLocation, ShakeSettings.FalloffRadius);
        
        if (IntensityMultiplier > 0.1f)
        {
            // Create simple camera shake effect
            FVector ShakeVector = FVector(
                ShakeSettings.Amplitude * IntensityMultiplier,
                ShakeSettings.Amplitude * IntensityMultiplier * 0.7f,
                ShakeSettings.Amplitude * IntensityMultiplier * 0.5f
            );

            // Play audio feedback
            if (ShakeAudioComponent && FootstepRumbleSound)
            {
                ShakeAudioComponent->SetSound(FootstepRumbleSound);
                ShakeAudioComponent->SetVolumeMultiplier(IntensityMultiplier);
                ShakeAudioComponent->Play();
            }

            UE_LOG(LogTemp, Warning, TEXT("Screen shake triggered: Intensity=%d, Distance=%.1f, Multiplier=%.2f"), 
                   (int32)Intensity, Distance, IntensityMultiplier);
        }
    }
}

void AAudio_ScreenShakeManager::TriggerTRexFootstepShake(FVector TRexLocation)
{
    TriggerScreenShake(EAudio_ShakeIntensity::Heavy, TRexLocation);
}

void AAudio_ScreenShakeManager::TriggerDamageImpactShake(float DamageAmount)
{
    EAudio_ShakeIntensity Intensity = EAudio_ShakeIntensity::Light;
    
    if (DamageAmount > 75.0f)
    {
        Intensity = EAudio_ShakeIntensity::Extreme;
    }
    else if (DamageAmount > 50.0f)
    {
        Intensity = EAudio_ShakeIntensity::Heavy;
    }
    else if (DamageAmount > 25.0f)
    {
        Intensity = EAudio_ShakeIntensity::Medium;
    }

    // Use player location for damage shake
    if (CachedPlayerController && CachedPlayerController->GetPawn())
    {
        FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
        TriggerScreenShake(Intensity, PlayerLocation);

        // Play damage impact sound
        if (ShakeAudioComponent && ImpactShakeSound)
        {
            ShakeAudioComponent->SetSound(ImpactShakeSound);
            ShakeAudioComponent->SetVolumeMultiplier(FMath::Clamp(DamageAmount / 100.0f, 0.3f, 1.0f));
            ShakeAudioComponent->Play();
        }
    }
}

void AAudio_ScreenShakeManager::StopAllShakes()
{
    if (ShakeAudioComponent)
    {
        ShakeAudioComponent->Stop();
    }
}

float AAudio_ScreenShakeManager::CalculateShakeIntensityByDistance(FVector ShakeLocation, float MaxDistance)
{
    if (!CachedPlayerController || !CachedPlayerController->GetPawn())
    {
        return 0.0f;
    }

    FVector PlayerLocation = CachedPlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, ShakeLocation);
    
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }

    // Linear falloff with minimum intensity
    float IntensityMultiplier = 1.0f - (Distance / MaxDistance);
    return FMath::Clamp(IntensityMultiplier, 0.0f, 1.0f);
}