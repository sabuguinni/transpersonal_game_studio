#include "Audio_AdvancedFeedbackController.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

AAudio_AdvancedFeedbackController::AAudio_AdvancedFeedbackController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    RootComponent = AudioComponent;

    // Initialize default settings
    MaxFeedbackDistance = 5000.0f;
    FeedbackCooldownTime = 1.0f;
    bIsFeedbackActive = false;
    CurrentFeedbackType = EAudio_FeedbackType::None;

    // Initialize feedback configurations
    InitializeFeedbackConfigurations();
}

void AAudio_AdvancedFeedbackController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_AdvancedFeedbackController: System initialized"));
    
    // Set up audio component
    if (AudioComponent)
    {
        AudioComponent->bAutoActivate = false;
        AudioComponent->SetVolumeMultiplier(0.8f);
    }
}

void AAudio_AdvancedFeedbackController::InitializeFeedbackConfigurations()
{
    // T-Rex Proximity Configuration
    FAudio_FeedbackConfig TRexConfig;
    TRexConfig.FeedbackType = EAudio_FeedbackType::TRexProximity;
    TRexConfig.IntensityLevel = 1.0f;
    TRexConfig.Duration = 3.0f;
    TRexConfig.bEnableScreenShake = true;
    TRexConfig.bEnableAudioCue = true;
    TRexConfig.bEnableVisualEffect = false;
    FeedbackConfigurations.Add(EAudio_FeedbackType::TRexProximity, TRexConfig);

    // Damage Received Configuration
    FAudio_FeedbackConfig DamageConfig;
    DamageConfig.FeedbackType = EAudio_FeedbackType::DamageReceived;
    DamageConfig.IntensityLevel = 0.8f;
    DamageConfig.Duration = 1.5f;
    DamageConfig.bEnableScreenShake = true;
    DamageConfig.bEnableAudioCue = true;
    DamageConfig.bEnableVisualEffect = true;
    FeedbackConfigurations.Add(EAudio_FeedbackType::DamageReceived, DamageConfig);

    // Environmental Hazard Configuration
    FAudio_FeedbackConfig HazardConfig;
    HazardConfig.FeedbackType = EAudio_FeedbackType::EnvironmentalHazard;
    HazardConfig.IntensityLevel = 0.6f;
    HazardConfig.Duration = 2.0f;
    HazardConfig.bEnableScreenShake = false;
    HazardConfig.bEnableAudioCue = true;
    HazardConfig.bEnableVisualEffect = false;
    FeedbackConfigurations.Add(EAudio_FeedbackType::EnvironmentalHazard, HazardConfig);

    // Critical Alert Configuration
    FAudio_FeedbackConfig AlertConfig;
    AlertConfig.FeedbackType = EAudio_FeedbackType::CriticalAlert;
    AlertConfig.IntensityLevel = 1.0f;
    AlertConfig.Duration = 4.0f;
    AlertConfig.bEnableScreenShake = false;
    AlertConfig.bEnableAudioCue = true;
    AlertConfig.bEnableVisualEffect = false;
    FeedbackConfigurations.Add(EAudio_FeedbackType::CriticalAlert, AlertConfig);

    // Seismic Activity Configuration
    FAudio_FeedbackConfig SeismicConfig;
    SeismicConfig.FeedbackType = EAudio_FeedbackType::SeismicActivity;
    SeismicConfig.IntensityLevel = 0.9f;
    SeismicConfig.Duration = 5.0f;
    SeismicConfig.bEnableScreenShake = true;
    SeismicConfig.bEnableAudioCue = true;
    SeismicConfig.bEnableVisualEffect = false;
    FeedbackConfigurations.Add(EAudio_FeedbackType::SeismicActivity, SeismicConfig);
}

void AAudio_AdvancedFeedbackController::TriggerFeedback(EAudio_FeedbackType FeedbackType, float Intensity, FVector SourceLocation)
{
    if (bIsFeedbackActive && CurrentFeedbackType == FeedbackType)
    {
        return; // Prevent feedback spam
    }

    FAudio_FeedbackConfig* Config = FeedbackConfigurations.Find(FeedbackType);
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_AdvancedFeedbackController: No configuration found for feedback type"));
        return;
    }

    // Calculate distance-based intensity if source location is provided
    float FinalIntensity = Intensity;
    if (SourceLocation != FVector::ZeroVector)
    {
        FinalIntensity = CalculateIntensityByDistance(SourceLocation, MaxFeedbackDistance);
        FinalIntensity *= Intensity; // Apply additional intensity modifier
    }

    // Set feedback state
    bIsFeedbackActive = true;
    CurrentFeedbackType = FeedbackType;

    // Trigger audio feedback
    if (Config->bEnableAudioCue)
    {
        PlayFeedbackAudio(FeedbackType, FinalIntensity);
    }

    // Trigger camera shake
    if (Config->bEnableScreenShake)
    {
        TriggerCameraShake(FeedbackType, FinalIntensity);
    }

    // Start cooldown timer
    if (FeedbackCooldownTime > 0.0f)
    {
        GetWorldTimerManager().SetTimer(
            FeedbackCooldownTimer,
            this,
            &AAudio_AdvancedFeedbackController::OnFeedbackCooldownComplete,
            Config->Duration,
            false
        );
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_AdvancedFeedbackController: Triggered feedback type %d with intensity %.2f"), 
           (int32)FeedbackType, FinalIntensity);
}

void AAudio_AdvancedFeedbackController::StopAllFeedback()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }

    bIsFeedbackActive = false;
    CurrentFeedbackType = EAudio_FeedbackType::None;

    // Clear any active timers
    GetWorldTimerManager().ClearTimer(FeedbackCooldownTimer);

    UE_LOG(LogTemp, Log, TEXT("Audio_AdvancedFeedbackController: All feedback stopped"));
}

void AAudio_AdvancedFeedbackController::SetFeedbackConfiguration(EAudio_FeedbackType FeedbackType, const FAudio_FeedbackConfig& Config)
{
    FeedbackConfigurations.Add(FeedbackType, Config);
    UE_LOG(LogTemp, Log, TEXT("Audio_AdvancedFeedbackController: Updated configuration for feedback type %d"), (int32)FeedbackType);
}

bool AAudio_AdvancedFeedbackController::IsFeedbackTypeActive(EAudio_FeedbackType FeedbackType) const
{
    return bIsFeedbackActive && CurrentFeedbackType == FeedbackType;
}

void AAudio_AdvancedFeedbackController::CheckTRexProximity()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find T-Rex actors in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);

    FVector PlayerLocation = FVector::ZeroVector;
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC && PC->GetPawn())
    {
        PlayerLocation = PC->GetPawn()->GetActorLocation();
    }

    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("TRex")))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            if (Distance < MaxFeedbackDistance)
            {
                float Intensity = 1.0f - (Distance / MaxFeedbackDistance);
                TriggerFeedback(EAudio_FeedbackType::TRexProximity, Intensity, Actor->GetActorLocation());
                break;
            }
        }
    }
}

void AAudio_AdvancedFeedbackController::OnPlayerDamaged(float DamageAmount, FVector DamageLocation)
{
    float Intensity = FMath::Clamp(DamageAmount / 100.0f, 0.1f, 1.0f); // Normalize damage to 0.1-1.0
    TriggerFeedback(EAudio_FeedbackType::DamageReceived, Intensity, DamageLocation);
}

void AAudio_AdvancedFeedbackController::OnEnvironmentalHazard(FVector HazardLocation, float HazardIntensity)
{
    TriggerFeedback(EAudio_FeedbackType::EnvironmentalHazard, HazardIntensity, HazardLocation);
}

void AAudio_AdvancedFeedbackController::OnFeedbackCooldownComplete()
{
    bIsFeedbackActive = false;
    CurrentFeedbackType = EAudio_FeedbackType::None;
    UE_LOG(LogTemp, Log, TEXT("Audio_AdvancedFeedbackController: Feedback cooldown complete"));
}

void AAudio_AdvancedFeedbackController::PlayFeedbackAudio(EAudio_FeedbackType FeedbackType, float Intensity)
{
    USoundBase* SoundToPlay = GetSoundForFeedbackType(FeedbackType);
    
    if (SoundToPlay && AudioComponent)
    {
        AudioComponent->SetSound(SoundToPlay);
        AudioComponent->SetVolumeMultiplier(Intensity);
        AudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Audio_AdvancedFeedbackController: Playing audio for feedback type %d"), (int32)FeedbackType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_AdvancedFeedbackController: No sound asset found for feedback type %d"), (int32)FeedbackType);
    }
}

void AAudio_AdvancedFeedbackController::TriggerCameraShake(EAudio_FeedbackType FeedbackType, float Intensity)
{
    TSubclassOf<UCameraShakeBase> ShakeClass = GetCameraShakeForFeedbackType(FeedbackType);
    
    if (ShakeClass)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
            if (PC)
            {
                PC->ClientStartCameraShake(ShakeClass, Intensity);
                UE_LOG(LogTemp, Log, TEXT("Audio_AdvancedFeedbackController: Triggered camera shake for feedback type %d"), (int32)FeedbackType);
            }
        }
    }
}

USoundBase* AAudio_AdvancedFeedbackController::GetSoundForFeedbackType(EAudio_FeedbackType FeedbackType) const
{
    switch (FeedbackType)
    {
        case EAudio_FeedbackType::TRexProximity:
            return TRexProximitySound;
        case EAudio_FeedbackType::DamageReceived:
            return DamageReceivedSound;
        case EAudio_FeedbackType::EnvironmentalHazard:
            return EnvironmentalHazardSound;
        case EAudio_FeedbackType::CriticalAlert:
            return CriticalAlertSound;
        case EAudio_FeedbackType::SeismicActivity:
            return SeismicActivitySound;
        default:
            return nullptr;
    }
}

TSubclassOf<UCameraShakeBase> AAudio_AdvancedFeedbackController::GetCameraShakeForFeedbackType(EAudio_FeedbackType FeedbackType) const
{
    switch (FeedbackType)
    {
        case EAudio_FeedbackType::TRexProximity:
            return TRexProximityCameraShake;
        case EAudio_FeedbackType::DamageReceived:
            return DamageCameraShake;
        case EAudio_FeedbackType::SeismicActivity:
            return SeismicCameraShake;
        default:
            return nullptr;
    }
}

float AAudio_AdvancedFeedbackController::CalculateIntensityByDistance(FVector SourceLocation, float MaxDistance) const
{
    UWorld* World = GetWorld();
    if (!World) return 1.0f;

    FVector PlayerLocation = FVector::ZeroVector;
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC && PC->GetPawn())
    {
        PlayerLocation = PC->GetPawn()->GetActorLocation();
    }

    float Distance = FVector::Dist(SourceLocation, PlayerLocation);
    float Intensity = 1.0f - FMath::Clamp(Distance / MaxDistance, 0.0f, 1.0f);
    
    return FMath::Max(0.1f, Intensity); // Ensure minimum intensity
}

void AAudio_AdvancedFeedbackController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Periodic T-Rex proximity check
    static float ProximityCheckTimer = 0.0f;
    ProximityCheckTimer += DeltaTime;
    
    if (ProximityCheckTimer >= 2.0f) // Check every 2 seconds
    {
        CheckTRexProximity();
        ProximityCheckTimer = 0.0f;
    }
}