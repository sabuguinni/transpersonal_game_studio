#include "World_WeatherSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"

UWorld_WeatherSystemManager::UWorld_WeatherSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default weather state
    CurrentWeatherState = EWorld_WeatherState::Clear;
    TargetWeatherState = EWorld_WeatherState::Clear;
    CurrentWeatherIntensity = 1.0f;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;

    // Initialize weather configuration
    WeatherTransitionTime = 30.0f;
    WeatherChangeFrequency = 300.0f; // 5 minutes
    MinWeatherDuration = 120.0f; // 2 minutes
    MaxWeatherDuration = 600.0f; // 10 minutes
    bEnableAutomaticWeatherChanges = true;

    // Initialize timers
    WeatherStateTimer = 0.0f;
    NextWeatherChangeTime = 0.0f;
    TransitionStartTime = 0.0f;

    // Initialize audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    IntensityAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("IntensityAudioComponent"));

    // Initialize VFX components
    WeatherParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("WeatherParticleComponent"));

    // Set component properties
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->bAutoActivate = false;
        AmbientAudioComponent->SetVolumeMultiplier(0.5f);
    }

    if (IntensityAudioComponent)
    {
        IntensityAudioComponent->bAutoActivate = false;
        IntensityAudioComponent->SetVolumeMultiplier(0.3f);
    }

    if (WeatherParticleComponent)
    {
        WeatherParticleComponent->bAutoActivate = false;
        WeatherParticleComponent->SetAutoDestroy(false);
    }
}

void UWorld_WeatherSystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Load weather configurations
    LoadWeatherConfigurations();
    
    // Setup default weather transitions
    SetupDefaultWeatherTransitions();
    
    // Initialize components
    InitializeAudioComponents();
    InitializeVFXComponents();
    
    // Find lighting actors in the world
    FindLightingActors();
    
    // Validate weather system
    ValidateWeatherSystem();
    
    // Start weather system
    StartWeatherSystem();

    UE_LOG(LogTemp, Log, TEXT("Weather System Manager initialized successfully"));
}

void UWorld_WeatherSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableAutomaticWeatherChanges)
    {
        return;
    }

    // Update weather state timer
    WeatherStateTimer += DeltaTime;

    // Process weather transitions
    if (bIsTransitioning)
    {
        UpdateWeatherTransition(DeltaTime);
    }
    else
    {
        ProcessAutomaticWeatherChanges(DeltaTime);
    }

    // Update weather audio and VFX
    UpdateWeatherAudio(DeltaTime);
    UpdateWeatherVFX(DeltaTime);
}

void UWorld_WeatherSystemManager::SetWeatherState(EWorld_WeatherState NewState, bool bForceImmediate)
{
    if (NewState == CurrentWeatherState && !bIsTransitioning)
    {
        return;
    }

    EWorld_WeatherState OldState = CurrentWeatherState;
    TargetWeatherState = NewState;

    if (bForceImmediate)
    {
        CurrentWeatherState = NewState;
        CurrentWeatherIntensity = 1.0f;
        bIsTransitioning = false;
        TransitionProgress = 1.0f;
        
        ApplyWeatherEffects(CurrentWeatherState, CurrentWeatherIntensity);
        OnWeatherStateChanged(OldState, CurrentWeatherState);
        OnWeatherTransitionCompleted(CurrentWeatherState);
    }
    else
    {
        bIsTransitioning = true;
        TransitionProgress = 0.0f;
        TransitionStartTime = GetWorld()->GetTimeSeconds();
        
        OnWeatherTransitionStarted(CurrentWeatherState, TargetWeatherState);
    }

    // Reset weather timer
    WeatherStateTimer = 0.0f;
    NextWeatherChangeTime = FMath::RandRange(MinWeatherDuration, MaxWeatherDuration);

    UE_LOG(LogTemp, Log, TEXT("Weather state transition: %s -> %s (Immediate: %s)"), 
        *UEnum::GetValueAsString(OldState), 
        *UEnum::GetValueAsString(NewState), 
        bForceImmediate ? TEXT("true") : TEXT("false"));
}

void UWorld_WeatherSystemManager::StartWeatherSystem()
{
    bEnableAutomaticWeatherChanges = true;
    NextWeatherChangeTime = FMath::RandRange(MinWeatherDuration, MaxWeatherDuration);
    
    // Apply current weather effects
    ApplyWeatherEffects(CurrentWeatherState, CurrentWeatherIntensity);
    
    UE_LOG(LogTemp, Log, TEXT("Weather system started"));
}

void UWorld_WeatherSystemManager::StopWeatherSystem()
{
    bEnableAutomaticWeatherChanges = false;
    bIsTransitioning = false;
    
    // Stop all audio components
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    
    if (IntensityAudioComponent && IntensityAudioComponent->IsPlaying())
    {
        IntensityAudioComponent->Stop();
    }
    
    // Stop particle effects
    if (WeatherParticleComponent && WeatherParticleComponent->IsActive())
    {
        WeatherParticleComponent->Deactivate();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Weather system stopped"));
}

void UWorld_WeatherSystemManager::PauseWeatherSystem()
{
    bEnableAutomaticWeatherChanges = false;
    UE_LOG(LogTemp, Log, TEXT("Weather system paused"));
}

void UWorld_WeatherSystemManager::ResumeWeatherSystem()
{
    bEnableAutomaticWeatherChanges = true;
    UE_LOG(LogTemp, Log, TEXT("Weather system resumed"));
}

void UWorld_WeatherSystemManager::SetWeatherTransitionTime(float NewTransitionTime)
{
    WeatherTransitionTime = FMath::Max(1.0f, NewTransitionTime);
}

void UWorld_WeatherSystemManager::SetWeatherChangeFrequency(float NewFrequency)
{
    WeatherChangeFrequency = FMath::Max(30.0f, NewFrequency);
}

void UWorld_WeatherSystemManager::SetBiomeInfluence(EBiomeType BiomeType, float Influence)
{
    BiomeWeatherInfluence.Add(BiomeType, FMath::Clamp(Influence, 0.0f, 2.0f));
}

void UWorld_WeatherSystemManager::UpdateWeatherAudio(float DeltaTime)
{
    if (!AmbientAudioComponent || !IntensityAudioComponent)
    {
        return;
    }

    // Get current weather audio config
    FWorld_WeatherAudioConfig* AudioConfig = WeatherAudioConfigs.Find(CurrentWeatherState);
    if (!AudioConfig)
    {
        return;
    }

    // Update ambient audio
    if (AudioConfig->AmbientSound.IsValid())
    {
        if (!AmbientAudioComponent->IsPlaying())
        {
            AmbientAudioComponent->SetSound(AudioConfig->AmbientSound.Get());
            AmbientAudioComponent->Play();
        }
        
        float TargetVolume = FMath::Lerp(AudioConfig->BaseVolume, AudioConfig->MaxVolume, CurrentWeatherIntensity);
        float CurrentVolume = AmbientAudioComponent->GetVolumeMultiplier();
        float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, 2.0f);
        AmbientAudioComponent->SetVolumeMultiplier(NewVolume);
    }

    // Update intensity audio based on weather intensity
    if (AudioConfig->IntensitySound.IsValid() && CurrentWeatherIntensity > 0.5f)
    {
        if (!IntensityAudioComponent->IsPlaying())
        {
            IntensityAudioComponent->SetSound(AudioConfig->IntensitySound.Get());
            IntensityAudioComponent->Play();
        }
        
        float IntensityVolume = (CurrentWeatherIntensity - 0.5f) * 2.0f * AudioConfig->MaxVolume;
        IntensityAudioComponent->SetVolumeMultiplier(IntensityVolume);
    }
    else if (IntensityAudioComponent->IsPlaying())
    {
        IntensityAudioComponent->Stop();
    }
}

void UWorld_WeatherSystemManager::SetWeatherAudioVolume(float Volume)
{
    float ClampedVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(ClampedVolume);
    }
    
    if (IntensityAudioComponent)
    {
        IntensityAudioComponent->SetVolumeMultiplier(ClampedVolume * 0.7f);
    }
}

void UWorld_WeatherSystemManager::UpdateWeatherVFX(float DeltaTime)
{
    if (!WeatherParticleComponent)
    {
        return;
    }

    // Get current weather VFX config
    FWorld_WeatherVFXConfig* VFXConfig = WeatherVFXConfigs.Find(CurrentWeatherState);
    if (!VFXConfig)
    {
        return;
    }

    // Update particle effects
    if (VFXConfig->ParticleEffect.IsValid())
    {
        if (!WeatherParticleComponent->IsActive())
        {
            WeatherParticleComponent->SetTemplate(VFXConfig->ParticleEffect.Get());
            WeatherParticleComponent->Activate();
        }
    }
    else if (WeatherParticleComponent->IsActive())
    {
        WeatherParticleComponent->Deactivate();
    }

    // Update lighting and fog
    UpdateLightingEffects(*VFXConfig, DeltaTime);
}

void UWorld_WeatherSystemManager::SetWeatherParticleIntensity(float Intensity)
{
    if (WeatherParticleComponent && WeatherParticleComponent->IsActive())
    {
        float ClampedIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
        // Apply intensity to particle parameters
        WeatherParticleComponent->SetFloatParameter(FName("Intensity"), ClampedIntensity);
    }
}

void UWorld_WeatherSystemManager::UpdateWeatherTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ElapsedTime = CurrentTime - TransitionStartTime;
    TransitionProgress = FMath::Clamp(ElapsedTime / WeatherTransitionTime, 0.0f, 1.0f);

    // Interpolate weather effects
    InterpolateWeatherEffects(CurrentWeatherState, TargetWeatherState, TransitionProgress);

    // Check if transition is complete
    if (TransitionProgress >= 1.0f)
    {
        EWorld_WeatherState OldState = CurrentWeatherState;
        CurrentWeatherState = TargetWeatherState;
        bIsTransitioning = false;
        TransitionProgress = 1.0f;
        CurrentWeatherIntensity = 1.0f;

        ApplyWeatherEffects(CurrentWeatherState, CurrentWeatherIntensity);
        OnWeatherStateChanged(OldState, CurrentWeatherState);
        OnWeatherTransitionCompleted(CurrentWeatherState);

        UE_LOG(LogTemp, Log, TEXT("Weather transition completed: %s"), 
            *UEnum::GetValueAsString(CurrentWeatherState));
    }
}

void UWorld_WeatherSystemManager::ProcessAutomaticWeatherChanges(float DeltaTime)
{
    if (WeatherStateTimer >= NextWeatherChangeTime)
    {
        EWorld_WeatherState NextState = SelectNextWeatherState();
        if (NextState != CurrentWeatherState)
        {
            SetWeatherState(NextState, false);
        }
        else
        {
            // Reset timer for next check
            WeatherStateTimer = 0.0f;
            NextWeatherChangeTime = FMath::RandRange(MinWeatherDuration, MaxWeatherDuration);
        }
    }
}

EWorld_WeatherState UWorld_WeatherSystemManager::SelectNextWeatherState()
{
    TArray<EWorld_WeatherState> PossibleStates;
    TArray<float> StateProbabilities;

    // Calculate probabilities for each possible weather state
    for (const FWorld_WeatherTransition& Transition : WeatherTransitions)
    {
        if (Transition.FromState == CurrentWeatherState)
        {
            float Probability = CalculateWeatherTransitionProbability(Transition.FromState, Transition.ToState);
            
            PossibleStates.Add(Transition.ToState);
            StateProbabilities.Add(Probability);
        }
    }

    if (PossibleStates.Num() == 0)
    {
        return CurrentWeatherState;
    }

    // Select state based on weighted probability
    float TotalProbability = 0.0f;
    for (float Prob : StateProbabilities)
    {
        TotalProbability += Prob;
    }

    float RandomValue = FMath::RandRange(0.0f, TotalProbability);
    float AccumulatedProbability = 0.0f;

    for (int32 i = 0; i < PossibleStates.Num(); ++i)
    {
        AccumulatedProbability += StateProbabilities[i];
        if (RandomValue <= AccumulatedProbability)
        {
            return PossibleStates[i];
        }
    }

    return PossibleStates.Last();
}

float UWorld_WeatherSystemManager::CalculateWeatherTransitionProbability(EWorld_WeatherState FromState, EWorld_WeatherState ToState)
{
    // Find the transition configuration
    for (const FWorld_WeatherTransition& Transition : WeatherTransitions)
    {
        if (Transition.FromState == FromState && Transition.ToState == ToState)
        {
            float BaseProbability = Transition.Probability;
            
            // Apply biome influences
            float BiomeModifier = 1.0f;
            // Note: Biome influence would be calculated based on current player location
            // For now, using a default modifier
            
            return BaseProbability * BiomeModifier;
        }
    }

    return 0.0f;
}

void UWorld_WeatherSystemManager::InitializeAudioComponents()
{
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetupAttachment(GetOwner()->GetRootComponent());
        AmbientAudioComponent->bAutoActivate = false;
    }

    if (IntensityAudioComponent)
    {
        IntensityAudioComponent->SetupAttachment(GetOwner()->GetRootComponent());
        IntensityAudioComponent->bAutoActivate = false;
    }
}

void UWorld_WeatherSystemManager::InitializeVFXComponents()
{
    if (WeatherParticleComponent)
    {
        WeatherParticleComponent->SetupAttachment(GetOwner()->GetRootComponent());
        WeatherParticleComponent->bAutoActivate = false;
    }
}

void UWorld_WeatherSystemManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find directional light (sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find sky light
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(FoundActors[0]);
    }

    // Find height fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    UE_LOG(LogTemp, Log, TEXT("Lighting actors found - Sun: %s, Sky: %s, Fog: %s"),
        SunLight ? TEXT("Yes") : TEXT("No"),
        SkyLight ? TEXT("Yes") : TEXT("No"),
        HeightFog ? TEXT("Yes") : TEXT("No"));
}

void UWorld_WeatherSystemManager::ApplyWeatherEffects(EWorld_WeatherState WeatherState, float Intensity)
{
    FWorld_WeatherVFXConfig* VFXConfig = WeatherVFXConfigs.Find(WeatherState);
    if (!VFXConfig)
    {
        return;
    }

    UpdateLightingEffects(*VFXConfig, 0.0f);
}

void UWorld_WeatherSystemManager::InterpolateWeatherEffects(EWorld_WeatherState FromState, EWorld_WeatherState ToState, float Alpha)
{
    FWorld_WeatherVFXConfig* FromConfig = WeatherVFXConfigs.Find(FromState);
    FWorld_WeatherVFXConfig* ToConfig = WeatherVFXConfigs.Find(ToState);

    if (!FromConfig || !ToConfig)
    {
        return;
    }

    // Interpolate lighting values
    float InterpolatedSunIntensity = FMath::Lerp(FromConfig->SunIntensity, ToConfig->SunIntensity, Alpha);
    FLinearColor InterpolatedSunColor = FMath::Lerp(FromConfig->SunColor, ToConfig->SunColor, Alpha);
    float InterpolatedSkyIntensity = FMath::Lerp(FromConfig->SkyLightIntensity, ToConfig->SkyLightIntensity, Alpha);
    FLinearColor InterpolatedFogColor = FMath::Lerp(FromConfig->FogColor, ToConfig->FogColor, Alpha);
    float InterpolatedFogDensity = FMath::Lerp(FromConfig->FogDensity, ToConfig->FogDensity, Alpha);

    // Apply interpolated values
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetIntensity(InterpolatedSunIntensity);
        SunLight->GetLightComponent()->SetLightColor(InterpolatedSunColor);
    }

    if (SkyLight && SkyLight->GetLightComponent())
    {
        SkyLight->GetLightComponent()->SetIntensity(InterpolatedSkyIntensity);
    }

    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogInscatteringColor(InterpolatedFogColor);
        HeightFog->GetComponent()->SetFogDensity(InterpolatedFogDensity);
    }

    // Update current weather intensity
    CurrentWeatherIntensity = Alpha;
}

void UWorld_WeatherSystemManager::UpdateLightingEffects(const FWorld_WeatherVFXConfig& VFXConfig, float DeltaTime)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetIntensity(VFXConfig.SunIntensity * CurrentWeatherIntensity);
        SunLight->GetLightComponent()->SetLightColor(VFXConfig.SunColor);
    }

    if (SkyLight && SkyLight->GetLightComponent())
    {
        SkyLight->GetLightComponent()->SetIntensity(VFXConfig.SkyLightIntensity * CurrentWeatherIntensity);
    }

    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogInscatteringColor(VFXConfig.FogColor);
        HeightFog->GetComponent()->SetFogDensity(VFXConfig.FogDensity * CurrentWeatherIntensity);
    }
}

void UWorld_WeatherSystemManager::LoadWeatherConfigurations()
{
    // Initialize default weather audio configurations
    FWorld_WeatherAudioConfig ClearAudio;
    ClearAudio.BaseVolume = 0.2f;
    ClearAudio.MaxVolume = 0.4f;
    WeatherAudioConfigs.Add(EWorld_WeatherState::Clear, ClearAudio);

    FWorld_WeatherAudioConfig RainAudio;
    RainAudio.BaseVolume = 0.4f;
    RainAudio.MaxVolume = 0.8f;
    WeatherAudioConfigs.Add(EWorld_WeatherState::LightRain, RainAudio);
    WeatherAudioConfigs.Add(EWorld_WeatherState::HeavyRain, RainAudio);

    FWorld_WeatherAudioConfig StormAudio;
    StormAudio.BaseVolume = 0.6f;
    StormAudio.MaxVolume = 1.0f;
    WeatherAudioConfigs.Add(EWorld_WeatherState::Thunderstorm, StormAudio);

    // Initialize default weather VFX configurations
    FWorld_WeatherVFXConfig ClearVFX;
    ClearVFX.SunIntensity = 3.0f;
    ClearVFX.SkyLightIntensity = 1.0f;
    ClearVFX.FogDensity = 0.01f;
    WeatherVFXConfigs.Add(EWorld_WeatherState::Clear, ClearVFX);

    FWorld_WeatherVFXConfig OvercastVFX;
    OvercastVFX.SunIntensity = 1.5f;
    OvercastVFX.SkyLightIntensity = 0.7f;
    OvercastVFX.FogDensity = 0.03f;
    OvercastVFX.FogColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
    WeatherVFXConfigs.Add(EWorld_WeatherState::Overcast, OvercastVFX);

    FWorld_WeatherVFXConfig FogVFX;
    FogVFX.SunIntensity = 0.8f;
    FogVFX.SkyLightIntensity = 0.5f;
    FogVFX.FogDensity = 0.08f;
    FogVFX.FogColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
    WeatherVFXConfigs.Add(EWorld_WeatherState::Fog, FogVFX);
}

void UWorld_WeatherSystemManager::SetupDefaultWeatherTransitions()
{
    WeatherTransitions.Empty();

    // Clear weather transitions
    WeatherTransitions.Add({EWorld_WeatherState::Clear, EWorld_WeatherState::PartlyCloudy, 45.0f, 0.3f});
    WeatherTransitions.Add({EWorld_WeatherState::Clear, EWorld_WeatherState::Fog, 60.0f, 0.1f});

    // Partly cloudy transitions
    WeatherTransitions.Add({EWorld_WeatherState::PartlyCloudy, EWorld_WeatherState::Clear, 30.0f, 0.4f});
    WeatherTransitions.Add({EWorld_WeatherState::PartlyCloudy, EWorld_WeatherState::Overcast, 40.0f, 0.3f});
    WeatherTransitions.Add({EWorld_WeatherState::PartlyCloudy, EWorld_WeatherState::LightRain, 50.0f, 0.2f});

    // Overcast transitions
    WeatherTransitions.Add({EWorld_WeatherState::Overcast, EWorld_WeatherState::PartlyCloudy, 35.0f, 0.3f});
    WeatherTransitions.Add({EWorld_WeatherState::Overcast, EWorld_WeatherState::LightRain, 40.0f, 0.4f});
    WeatherTransitions.Add({EWorld_WeatherState::Overcast, EWorld_WeatherState::HeavyRain, 45.0f, 0.2f});

    // Rain transitions
    WeatherTransitions.Add({EWorld_WeatherState::LightRain, EWorld_WeatherState::Overcast, 30.0f, 0.4f});
    WeatherTransitions.Add({EWorld_WeatherState::LightRain, EWorld_WeatherState::HeavyRain, 25.0f, 0.3f});
    WeatherTransitions.Add({EWorld_WeatherState::HeavyRain, EWorld_WeatherState::LightRain, 20.0f, 0.4f});
    WeatherTransitions.Add({EWorld_WeatherState::HeavyRain, EWorld_WeatherState::Thunderstorm, 30.0f, 0.2f});

    // Thunderstorm transitions
    WeatherTransitions.Add({EWorld_WeatherState::Thunderstorm, EWorld_WeatherState::HeavyRain, 25.0f, 0.5f});
    WeatherTransitions.Add({EWorld_WeatherState::Thunderstorm, EWorld_WeatherState::LightRain, 35.0f, 0.3f});

    // Fog transitions
    WeatherTransitions.Add({EWorld_WeatherState::Fog, EWorld_WeatherState::Clear, 45.0f, 0.6f});
    WeatherTransitions.Add({EWorld_WeatherState::Fog, EWorld_WeatherState::PartlyCloudy, 40.0f, 0.3f});
}

void UWorld_WeatherSystemManager::ValidateWeatherSystem()
{
    if (!AmbientAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weather System: Ambient audio component is null"));
    }

    if (!IntensityAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weather System: Intensity audio component is null"));
    }

    if (!WeatherParticleComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weather System: Weather particle component is null"));
    }

    if (WeatherTransitions.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weather System: No weather transitions configured"));
    }

    UE_LOG(LogTemp, Log, TEXT("Weather System validation completed"));
}