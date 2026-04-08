#include "LightingSystemManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ULightingSystemManager::ULightingSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void ULightingSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingSystem();
}

void ULightingSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bAutoProgressTime)
    {
        UpdateTimeProgression(DeltaTime);
    }

    if (bIsTransitioning)
    {
        UpdateLightingTransition(DeltaTime);
    }
}

void ULightingSystemManager::InitializeLightingSystem()
{
    FindLightingActors();
    SetupDefaultPresets();
    
    // Apply initial lighting state
    SetTimeOfDay(ETimeOfDay::Morning, 0.0f);
    SetWeatherState(EWeatherState::Clear, 0.0f);
    SetEmotionalTone(EEmotionalTone::Peaceful, 0.0f);
}

void ULightingSystemManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find Sun Light (Primary Directional Light)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    for (AActor* Actor : DirectionalLights)
    {
        ADirectionalLight* DirLight = Cast<ADirectionalLight>(Actor);
        if (DirLight && DirLight->GetLightComponent()->bAtmosphereSunLight)
        {
            SunLight = DirLight;
            break;
        }
    }

    // Find Sky Light
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    if (SkyLights.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(SkyLights[0]);
    }

    // Find Fog Actor
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FogActors[0]);
    }

    // Find Cloud Actor
    TArray<AActor*> CloudActors;
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), CloudActors);
    if (CloudActors.Num() > 0)
    {
        CloudActor = Cast<AVolumetricCloud>(CloudActors[0]);
    }

    // Find Atmosphere Actor
    TArray<AActor*> AtmosphereActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), AtmosphereActors);
    if (AtmosphereActors.Num() > 0)
    {
        AtmosphereActor = Cast<ASkyAtmosphere>(AtmosphereActors[0]);
    }
}

void ULightingSystemManager::SetupDefaultPresets()
{
    // === TIME OF DAY PRESETS ===
    
    // Dawn - Soft, warm light breaking through
    FLightingState DawnState;
    DawnState.SunAngle = 10.0f;
    DawnState.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnState.SunIntensity = 3.0f;
    DawnState.SkyColor = FLinearColor(0.8f, 0.6f, 0.9f, 1.0f);
    DawnState.CloudDensity = 0.4f;
    DawnState.FogDensity = 0.05f;
    DawnState.FogColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
    DawnState.AtmosphericPerspective = 1.2f;
    TimeOfDayPresets.Add(ETimeOfDay::Dawn, DawnState);

    // Morning - Clear, hopeful lighting
    FLightingState MorningState;
    MorningState.SunAngle = 30.0f;
    MorningState.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    MorningState.SunIntensity = 8.0f;
    MorningState.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    MorningState.CloudDensity = 0.2f;
    MorningState.FogDensity = 0.02f;
    MorningState.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MorningState.AtmosphericPerspective = 1.0f;
    TimeOfDayPresets.Add(ETimeOfDay::Morning, MorningState);

    // Midday - Harsh, revealing light
    FLightingState MiddayState;
    MiddayState.SunAngle = 80.0f;
    MiddayState.SunColor = FLinearColor::White;
    MiddayState.SunIntensity = 12.0f;
    MiddayState.SkyColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    MiddayState.CloudDensity = 0.1f;
    MiddayState.FogDensity = 0.01f;
    MiddayState.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    MiddayState.AtmosphericPerspective = 0.8f;
    TimeOfDayPresets.Add(ETimeOfDay::Midday, MiddayState);

    // Afternoon - Warm, but tension building
    FLightingState AfternoonState;
    AfternoonState.SunAngle = 45.0f;
    AfternoonState.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AfternoonState.SunIntensity = 10.0f;
    AfternoonState.SkyColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
    AfternoonState.CloudDensity = 0.3f;
    AfternoonState.FogDensity = 0.025f;
    AfternoonState.FogColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
    AfternoonState.AtmosphericPerspective = 1.1f;
    TimeOfDayPresets.Add(ETimeOfDay::Afternoon, AfternoonState);

    // Dusk - Golden hour with underlying menace
    FLightingState DuskState;
    DuskState.SunAngle = 15.0f;
    DuskState.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskState.SunIntensity = 5.0f;
    DuskState.SkyColor = FLinearColor(0.9f, 0.5f, 0.7f, 1.0f);
    DuskState.CloudDensity = 0.5f;
    DuskState.FogDensity = 0.04f;
    DuskState.FogColor = FLinearColor(0.7f, 0.6f, 0.8f, 1.0f);
    DuskState.AtmosphericPerspective = 1.5f;
    TimeOfDayPresets.Add(ETimeOfDay::Dusk, DuskState);

    // Night - Dangerous, limited visibility
    FLightingState NightState;
    NightState.SunAngle = -10.0f;
    NightState.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightState.SunIntensity = 0.5f;
    NightState.SkyColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    NightState.CloudDensity = 0.6f;
    NightState.FogDensity = 0.06f;
    NightState.FogColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    NightState.AtmosphericPerspective = 2.0f;
    TimeOfDayPresets.Add(ETimeOfDay::Night, NightState);

    // Deep Night - Maximum danger
    FLightingState DeepNightState;
    DeepNightState.SunAngle = -30.0f;
    DeepNightState.SunColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    DeepNightState.SunIntensity = 0.2f;
    DeepNightState.SkyColor = FLinearColor(0.05f, 0.1f, 0.2f, 1.0f);
    DeepNightState.CloudDensity = 0.8f;
    DeepNightState.FogDensity = 0.08f;
    DeepNightState.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    DeepNightState.AtmosphericPerspective = 2.5f;
    TimeOfDayPresets.Add(ETimeOfDay::DeepNight, DeepNightState);

    // === EMOTIONAL TONE PRESETS ===
    
    // Peaceful - Natural, calming
    FLightingState PeacefulState;
    PeacefulState.SunIntensity = 1.0f; // Multiplier
    PeacefulState.SunColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    PeacefulState.FogDensity = 1.0f; // Multiplier
    EmotionalPresets.Add(EEmotionalTone::Peaceful, PeacefulState);

    // Tense - Slightly desaturated, cooler
    FLightingState TenseState;
    TenseState.SunIntensity = 0.8f;
    TenseState.SunColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    TenseState.FogDensity = 1.3f;
    EmotionalPresets.Add(EEmotionalTone::Tense, TenseState);

    // Dangerous - Harsh contrasts, warmer shadows
    FLightingState DangerousState;
    DangerousState.SunIntensity = 1.2f;
    DangerousState.SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    DangerousState.FogDensity = 1.5f;
    EmotionalPresets.Add(EEmotionalTone::Dangerous, DangerousState);

    // Terrifying - High contrast, deep shadows
    FLightingState TerrifyingState;
    TerrifyingState.SunIntensity = 1.5f;
    TerrifyingState.SunColor = FLinearColor(1.0f, 0.8f, 0.7f, 1.0f);
    TerrifyingState.FogDensity = 2.0f;
    EmotionalPresets.Add(EEmotionalTone::Terrifying, TerrifyingState);

    // Mysterious - Muted, atmospheric
    FLightingState MysteriousState;
    MysteriousState.SunIntensity = 0.6f;
    MysteriousState.SunColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MysteriousState.FogDensity = 1.8f;
    EmotionalPresets.Add(EEmotionalTone::Mysterious, MysteriousState);

    // Hopeful - Bright, warm
    FLightingState HopefulState;
    HopefulState.SunIntensity = 1.3f;
    HopefulState.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    HopefulState.FogDensity = 0.7f;
    EmotionalPresets.Add(EEmotionalTone::Hopeful, HopefulState);
}

void ULightingSystemManager::SetTimeOfDay(ETimeOfDay NewTimeOfDay, float TransitionDuration)
{
    if (NewTimeOfDay == CurrentTimeOfDay && !bIsTransitioning) return;

    CurrentTimeOfDay = NewTimeOfDay;
    
    if (TimeOfDayPresets.Contains(NewTimeOfDay))
    {
        if (TransitionDuration > 0.0f)
        {
            // Start transition
            bIsTransitioning = true;
            this->TransitionDuration = TransitionDuration;
            TransitionProgress = 0.0f;
            
            // Store current state as start state
            // TransitionStartState = GetCurrentLightingState();
            TransitionTargetState = TimeOfDayPresets[NewTimeOfDay];
        }
        else
        {
            // Immediate change
            ApplyLightingState(TimeOfDayPresets[NewTimeOfDay]);
        }
    }
}

void ULightingSystemManager::SetWeatherState(EWeatherState NewWeather, float TransitionDuration)
{
    CurrentWeather = NewWeather;
    // Weather effects will modify the base lighting state
}

void ULightingSystemManager::SetEmotionalTone(EEmotionalTone NewTone, float TransitionDuration)
{
    CurrentEmotionalTone = NewTone;
    // Emotional modifiers will be applied on top of time/weather
}

void ULightingSystemManager::UpdateTimeProgression(float DeltaTime)
{
    float TimeStep = DeltaTime * TimeProgressionSpeed / (DayLengthInMinutes * 60.0f);
    CurrentTimeFloat += TimeStep;
    
    if (CurrentTimeFloat >= 1.0f)
    {
        CurrentTimeFloat -= 1.0f; // Wrap around
    }

    // Convert float time to time of day enum
    ETimeOfDay NewTimeOfDay;
    if (CurrentTimeFloat < 0.1f) NewTimeOfDay = ETimeOfDay::DeepNight;
    else if (CurrentTimeFloat < 0.2f) NewTimeOfDay = ETimeOfDay::Dawn;
    else if (CurrentTimeFloat < 0.4f) NewTimeOfDay = ETimeOfDay::Morning;
    else if (CurrentTimeFloat < 0.6f) NewTimeOfDay = ETimeOfDay::Midday;
    else if (CurrentTimeFloat < 0.75f) NewTimeOfDay = ETimeOfDay::Afternoon;
    else if (CurrentTimeFloat < 0.9f) NewTimeOfDay = ETimeOfDay::Dusk;
    else NewTimeOfDay = ETimeOfDay::Night;

    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        SetTimeOfDay(NewTimeOfDay, 2.0f); // 2 second transition
    }
}

void ULightingSystemManager::UpdateLightingTransition(float DeltaTime)
{
    if (!bIsTransitioning) return;

    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
    }

    // Blend between start and target states
    FLightingState BlendedState = BlendLightingStates(TransitionStartState, TransitionTargetState, TransitionProgress);
    ApplyLightingState(BlendedState);
}

void ULightingSystemManager::ApplyLightingState(const FLightingState& State)
{
    // Apply sun light settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent();
        
        // Set sun rotation based on angle
        FRotator SunRotation = CalculateSunRotation(CurrentTimeFloat);
        SunLight->SetActorRotation(SunRotation);
        
        // Set sun properties
        SunComponent->SetLightColor(State.SunColor);
        SunComponent->SetIntensity(State.SunIntensity);
    }

    // Apply fog settings
    if (FogActor && FogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComponent = FogActor->GetComponent();
        FogComponent->SetFogDensity(State.FogDensity);
        FogComponent->SetFogInscatteringColor(State.FogColor);
    }

    // Apply sky light settings
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        SkyLightActor->GetLightComponent()->RecaptureSky();
    }
}

FRotator ULightingSystemManager::CalculateSunRotation(float TimeOfDay)
{
    // Convert time of day (0-1) to sun angle
    // 0.0 = midnight (sun below horizon)
    // 0.5 = noon (sun at zenith)
    // 1.0 = midnight again
    
    float SunAngle = (TimeOfDay - 0.5f) * 180.0f; // -90 to +90 degrees
    
    return FRotator(-SunAngle, 0.0f, 0.0f);
}

FLightingState ULightingSystemManager::BlendLightingStates(const FLightingState& StateA, const FLightingState& StateB, float Alpha)
{
    FLightingState Result;
    
    Result.SunAngle = FMath::Lerp(StateA.SunAngle, StateB.SunAngle, Alpha);
    Result.SunColor = FMath::Lerp(StateA.SunColor, StateB.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(StateA.SunIntensity, StateB.SunIntensity, Alpha);
    Result.SkyColor = FMath::Lerp(StateA.SkyColor, StateB.SkyColor, Alpha);
    Result.CloudDensity = FMath::Lerp(StateA.CloudDensity, StateB.CloudDensity, Alpha);
    Result.FogDensity = FMath::Lerp(StateA.FogDensity, StateB.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(StateA.FogColor, StateB.FogColor, Alpha);
    Result.AtmosphericPerspective = FMath::Lerp(StateA.AtmosphericPerspective, StateB.AtmosphericPerspective, Alpha);
    
    return Result;
}

void ULightingSystemManager::TriggerTensionLighting(float Intensity, float Duration)
{
    SetEmotionalTone(EEmotionalTone::Tense, 1.0f);
    
    // Schedule return to normal after duration
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        RestoreNormalLighting(2.0f);
    }, Duration, false);
}

void ULightingSystemManager::TriggerDangerLighting(float Intensity, float Duration)
{
    SetEmotionalTone(EEmotionalTone::Dangerous, 0.5f);
    
    // Schedule return to normal after duration
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        RestoreNormalLighting(3.0f);
    }, Duration, false);
}

void ULightingSystemManager::RestoreNormalLighting(float TransitionDuration)
{
    SetEmotionalTone(EEmotionalTone::Peaceful, TransitionDuration);
}

void ULightingSystemManager::StartRainEffect(float Intensity)
{
    SetWeatherState(EWeatherState::LightRain, 3.0f);
}

void ULightingSystemManager::StopRainEffect(float TransitionDuration)
{
    SetWeatherState(EWeatherState::Clear, TransitionDuration);
}

void ULightingSystemManager::StartStormEffect()
{
    SetWeatherState(EWeatherState::Storm, 2.0f);
}

void ULightingSystemManager::StopStormEffect()
{
    SetWeatherState(EWeatherState::Clear, 5.0f);
}