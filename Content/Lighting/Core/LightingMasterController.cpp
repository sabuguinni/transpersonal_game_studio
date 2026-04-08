#include "LightingMasterController.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

ALightingMasterController::ALightingMasterController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize lighting components
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetIntensity(3.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.8f, 1.0f));
    SunLight->SetCastShadows(true);
    SunLight->SetMobility(EComponentMobility::Movable);

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(RootComponent);

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetRealTimeCapture(true);

    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricClouds->SetupAttachment(RootComponent);

    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);

    PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
    PostProcessComponent->SetupAttachment(RootComponent);
    PostProcessComponent->bUnbound = true;
}

void ALightingMasterController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAtmosphericStates();
    UpdateSunPosition();
    UpdateSkyAtmosphere();
    UpdateVolumetricClouds();
    UpdateHeightFog();
    UpdatePostProcessEffects();
}

void ALightingMasterController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoAdvanceTime)
    {
        UpdateTimeOfDay(DeltaTime);
    }

    UpdateAtmosphericMood(DeltaTime);
    UpdateSunPosition();
    UpdateSkyAtmosphere();
    UpdateVolumetricClouds();
    UpdateHeightFog();
    UpdatePostProcessEffects();
}

void ALightingMasterController::InitializeAtmosphericStates()
{
    // Safe Exploration - Warm, inviting light with soft shadows
    FAtmosphericState SafeState;
    SafeState.SunIntensity = 3.5f;
    SafeState.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    SafeState.SkyLightIntensity = 1.2f;
    SafeState.FogDensity = 0.01f;
    SafeState.FogHeightFalloff = 0.15f;
    SafeState.FogInscatteringColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
    SafeState.CloudCoverage = 0.2f;
    SafeState.VolumetricScatteringIntensity = 0.8f;
    SafeState.ShadowSharpness = 0.6f;
    MoodStates.Add(EAtmosphericMood::SafeExploration, SafeState);

    // Tension Building - Cooler tones, deeper shadows, increased contrast
    FAtmosphericState TensionState;
    TensionState.SunIntensity = 2.8f;
    TensionState.SunColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
    TensionState.SkyLightIntensity = 0.8f;
    TensionState.FogDensity = 0.025f;
    TensionState.FogHeightFalloff = 0.25f;
    TensionState.FogInscatteringColor = FLinearColor(0.4f, 0.5f, 0.7f, 1.0f);
    TensionState.CloudCoverage = 0.4f;
    TensionState.VolumetricScatteringIntensity = 1.2f;
    TensionState.ShadowSharpness = 0.8f;
    MoodStates.Add(EAtmosphericMood::TensionBuilding, TensionState);

    // Danger Imminent - Harsh contrasts, cold light, thick fog
    FAtmosphericState DangerState;
    DangerState.SunIntensity = 2.2f;
    DangerState.SunColor = FLinearColor(0.8f, 0.75f, 0.6f, 1.0f);
    DangerState.SkyLightIntensity = 0.6f;
    DangerState.FogDensity = 0.04f;
    DangerState.FogHeightFalloff = 0.35f;
    DangerState.FogInscatteringColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    DangerState.CloudCoverage = 0.6f;
    DangerState.VolumetricScatteringIntensity = 1.5f;
    DangerState.ShadowSharpness = 0.9f;
    MoodStates.Add(EAtmosphericMood::DangerImminent, DangerState);

    // Predator Hunting - Dramatic shadows, muted colors, oppressive atmosphere
    FAtmosphericState PredatorState;
    PredatorState.SunIntensity = 1.8f;
    PredatorState.SunColor = FLinearColor(0.7f, 0.65f, 0.5f, 1.0f);
    PredatorState.SkyLightIntensity = 0.4f;
    PredatorState.FogDensity = 0.06f;
    PredatorState.FogHeightFalloff = 0.45f;
    PredatorState.FogInscatteringColor = FLinearColor(0.25f, 0.3f, 0.5f, 1.0f);
    PredatorState.CloudCoverage = 0.8f;
    PredatorState.VolumetricScatteringIntensity = 2.0f;
    PredatorState.ShadowSharpness = 1.0f;
    MoodStates.Add(EAtmosphericMood::PredatorHunting, PredatorState);

    // Night Terror - Minimal light, deep blues, maximum fear
    FAtmosphericState NightState;
    NightState.SunIntensity = 0.5f;
    NightState.SunColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    NightState.SkyLightIntensity = 0.2f;
    NightState.FogDensity = 0.08f;
    NightState.FogHeightFalloff = 0.6f;
    NightState.FogInscatteringColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    NightState.CloudCoverage = 0.9f;
    NightState.VolumetricScatteringIntensity = 2.5f;
    NightState.ShadowSharpness = 1.0f;
    MoodStates.Add(EAtmosphericMood::NightTerror, NightState);

    // False Calm - Deceptively beautiful but with subtle wrongness
    FAtmosphericState FalseCalmState;
    FalseCalmState.SunIntensity = 4.0f;
    FalseCalmState.SunColor = FLinearColor(1.1f, 1.0f, 0.85f, 1.0f);
    FalseCalmState.SkyLightIntensity = 1.4f;
    FalseCalmState.FogDensity = 0.005f;
    FalseCalmState.FogHeightFalloff = 0.1f;
    FalseCalmState.FogInscatteringColor = FLinearColor(0.8f, 0.8f, 1.0f, 1.0f);
    FalseCalmState.CloudCoverage = 0.1f;
    FalseCalmState.VolumetricScatteringIntensity = 0.6f;
    FalseCalmState.ShadowSharpness = 0.4f;
    MoodStates.Add(EAtmosphericMood::FalseCalm, FalseCalmState);

    // Initialize time of day states
    InitializeTimeOfDayStates();
}

void ALightingMasterController::InitializeTimeOfDayStates()
{
    // Dawn - 6:00
    FAtmosphericState DawnState;
    DawnState.SunIntensity = 1.5f;
    DawnState.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnState.SkyLightIntensity = 0.6f;
    DawnState.FogDensity = 0.03f;
    DawnState.FogInscatteringColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    TimeOfDayStates.Add(ETimeOfDay::Dawn, DawnState);

    // Morning - 9:00
    FAtmosphericState MorningState;
    MorningState.SunIntensity = 3.0f;
    MorningState.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    MorningState.SkyLightIntensity = 1.0f;
    MorningState.FogDensity = 0.02f;
    MorningState.FogInscatteringColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
    TimeOfDayStates.Add(ETimeOfDay::Morning, MorningState);

    // Noon - 12:00
    FAtmosphericState NoonState;
    NoonState.SunIntensity = 4.0f;
    NoonState.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    NoonState.SkyLightIntensity = 1.2f;
    NoonState.FogDensity = 0.015f;
    NoonState.FogInscatteringColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    TimeOfDayStates.Add(ETimeOfDay::Noon, NoonState);

    // Afternoon - 15:00
    FAtmosphericState AfternoonState;
    AfternoonState.SunIntensity = 3.5f;
    AfternoonState.SunColor = FLinearColor(1.0f, 0.9f, 0.75f, 1.0f);
    AfternoonState.SkyLightIntensity = 1.1f;
    AfternoonState.FogDensity = 0.018f;
    AfternoonState.FogInscatteringColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    TimeOfDayStates.Add(ETimeOfDay::Afternoon, AfternoonState);

    // Dusk - 18:00
    FAtmosphericState DuskState;
    DuskState.SunIntensity = 2.0f;
    DuskState.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskState.SkyLightIntensity = 0.7f;
    DuskState.FogDensity = 0.035f;
    DuskState.FogInscatteringColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    TimeOfDayStates.Add(ETimeOfDay::Dusk, DuskState);

    // Night - 21:00
    FAtmosphericState NightState;
    NightState.SunIntensity = 0.3f;
    NightState.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightState.SkyLightIntensity = 0.3f;
    NightState.FogDensity = 0.05f;
    NightState.FogInscatteringColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    TimeOfDayStates.Add(ETimeOfDay::Night, NightState);

    // Deep Night - 24:00/0:00
    FAtmosphericState DeepNightState;
    DeepNightState.SunIntensity = 0.1f;
    DeepNightState.SunColor = FLinearColor(0.2f, 0.3f, 0.7f, 1.0f);
    DeepNightState.SkyLightIntensity = 0.2f;
    DeepNightState.FogDensity = 0.06f;
    DeepNightState.FogInscatteringColor = FLinearColor(0.1f, 0.2f, 0.5f, 1.0f);
    TimeOfDayStates.Add(ETimeOfDay::DeepNight, DeepNightState);
}

void ALightingMasterController::UpdateTimeOfDay(float DeltaTime)
{
    if (DayDurationMinutes > 0.0f)
    {
        float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        CurrentTimeOfDay += TimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
    }
}

void ALightingMasterController::UpdateAtmosphericMood(float DeltaTime)
{
    if (bMoodTransitioning)
    {
        MoodBlendAlpha += (MoodTransitionSpeed * DeltaTime);
        
        if (MoodBlendAlpha >= 1.0f)
        {
            MoodBlendAlpha = 1.0f;
            bMoodTransitioning = false;
            CurrentState = TargetState;
        }
        else
        {
            // Blend between current and target state
            FAtmosphericState BlendedState;
            BlendedState.SunIntensity = FMath::Lerp(CurrentState.SunIntensity, TargetState.SunIntensity, MoodBlendAlpha);
            BlendedState.SunColor = FMath::Lerp(CurrentState.SunColor, TargetState.SunColor, MoodBlendAlpha);
            BlendedState.SkyLightIntensity = FMath::Lerp(CurrentState.SkyLightIntensity, TargetState.SkyLightIntensity, MoodBlendAlpha);
            BlendedState.FogDensity = FMath::Lerp(CurrentState.FogDensity, TargetState.FogDensity, MoodBlendAlpha);
            BlendedState.FogHeightFalloff = FMath::Lerp(CurrentState.FogHeightFalloff, TargetState.FogHeightFalloff, MoodBlendAlpha);
            BlendedState.FogInscatteringColor = FMath::Lerp(CurrentState.FogInscatteringColor, TargetState.FogInscatteringColor, MoodBlendAlpha);
            BlendedState.CloudCoverage = FMath::Lerp(CurrentState.CloudCoverage, TargetState.CloudCoverage, MoodBlendAlpha);
            BlendedState.VolumetricScatteringIntensity = FMath::Lerp(CurrentState.VolumetricScatteringIntensity, TargetState.VolumetricScatteringIntensity, MoodBlendAlpha);
            BlendedState.ShadowSharpness = FMath::Lerp(CurrentState.ShadowSharpness, TargetState.ShadowSharpness, MoodBlendAlpha);
            
            CurrentState = BlendedState;
        }
    }
}

void ALightingMasterController::UpdateSunPosition()
{
    // Convert time of day to sun angle (0-360 degrees)
    float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at horizon
    
    // Calculate sun rotation
    FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
    SunLight->SetWorldRotation(SunRotation);
    
    // Update sun intensity based on angle (stronger during day, weaker at night)
    float SunHeightFactor = FMath::Clamp(FMath::Sin(FMath::DegreesToRadians(SunAngle)), 0.0f, 1.0f);
    float AdjustedIntensity = CurrentState.SunIntensity * SunHeightFactor;
    SunLight->SetIntensity(AdjustedIntensity);
}

void ALightingMasterController::UpdateSkyAtmosphere()
{
    if (SkyAtmosphere)
    {
        // Update sky atmosphere properties based on current state
        SkyAtmosphere->SetSkyLuminanceFactor(CurrentState.SkyLightIntensity);
        SkyAtmosphere->SetAerialPerspectiveDistanceScale(CurrentState.VolumetricScatteringIntensity);
    }
}

void ALightingMasterController::UpdateVolumetricClouds()
{
    if (VolumetricClouds)
    {
        // Update cloud properties based on current state
        // Note: This would require access to cloud material parameters
        // Implementation depends on the specific cloud material setup
    }
}

void ALightingMasterController::UpdateHeightFog()
{
    if (HeightFog)
    {
        HeightFog->SetFogDensity(CurrentState.FogDensity);
        HeightFog->SetFogHeightFalloff(CurrentState.FogHeightFalloff);
        HeightFog->SetFogInscatteringColor(CurrentState.FogInscatteringColor);
    }
}

void ALightingMasterController::UpdatePostProcessEffects()
{
    if (PostProcessComponent)
    {
        // Update post-process settings based on current atmospheric state
        FPostProcessSettings& Settings = PostProcessComponent->Settings;
        
        // Adjust exposure based on mood
        Settings.bOverride_AutoExposureBias = true;
        
        switch (CurrentMood)
        {
            case EAtmosphericMood::SafeExploration:
                Settings.AutoExposureBias = 0.2f;
                break;
            case EAtmosphericMood::TensionBuilding:
                Settings.AutoExposureBias = -0.1f;
                break;
            case EAtmosphericMood::DangerImminent:
                Settings.AutoExposureBias = -0.3f;
                break;
            case EAtmosphericMood::PredatorHunting:
                Settings.AutoExposureBias = -0.5f;
                break;
            case EAtmosphericMood::NightTerror:
                Settings.AutoExposureBias = -0.8f;
                break;
            case EAtmosphericMood::FalseCalm:
                Settings.AutoExposureBias = 0.4f;
                break;
        }
        
        // Adjust contrast and saturation for mood
        Settings.bOverride_ColorContrast = true;
        Settings.bOverride_ColorSaturation = true;
        
        float ContrastMultiplier = (CurrentMood == EAtmosphericMood::PredatorHunting || CurrentMood == EAtmosphericMood::NightTerror) ? 1.2f : 1.0f;
        float SaturationMultiplier = (CurrentMood == EAtmosphericMood::DangerImminent || CurrentMood == EAtmosphericMood::PredatorHunting) ? 0.8f : 1.0f;
        
        Settings.ColorContrast = FVector4(ContrastMultiplier, ContrastMultiplier, ContrastMultiplier, 1.0f);
        Settings.ColorSaturation = FVector4(SaturationMultiplier, SaturationMultiplier, SaturationMultiplier, 1.0f);
    }
}

void ALightingMasterController::SetAtmosphericMood(EAtmosphericMood NewMood, float TransitionTime)
{
    if (NewMood != CurrentMood)
    {
        CurrentMood = NewMood;
        
        if (MoodStates.Contains(NewMood))
        {
            TargetState = MoodStates[NewMood];
            
            // Blend with time of day state
            ETimeOfDay CurrentTimeEnum = GetCurrentTimeOfDayEnum();
            if (TimeOfDayStates.Contains(CurrentTimeEnum))
            {
                FAtmosphericState TimeState = TimeOfDayStates[CurrentTimeEnum];
                
                // Blend mood and time of day (mood takes priority but time affects color temperature)
                TargetState.SunColor = FMath::Lerp(TargetState.SunColor, TimeState.SunColor, 0.3f);
                TargetState.FogInscatteringColor = FMath::Lerp(TargetState.FogInscatteringColor, TimeState.FogInscatteringColor, 0.2f);
            }
            
            MoodBlendAlpha = 0.0f;
            bMoodTransitioning = true;
            MoodTransitionSpeed = (TransitionTime > 0.0f) ? (1.0f / TransitionTime) : 10.0f;
        }
    }
}

void ALightingMasterController::SetTimeOfDay(float NewTime, bool bInstant)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    
    if (bInstant)
    {
        UpdateSunPosition();
    }
}

void ALightingMasterController::TriggerStormSequence(float Duration)
{
    bStormApproaching = true;
    
    // Create dramatic storm lighting
    SetAtmosphericMood(EAtmosphericMood::DangerImminent, 5.0f);
    
    // Schedule return to previous state after duration
    FTimerHandle StormTimer;
    GetWorld()->GetTimerManager().SetTimer(StormTimer, [this]()
    {
        bStormApproaching = false;
        SetAtmosphericMood(EAtmosphericMood::TensionBuilding, 10.0f);
    }, Duration, false);
}

void ALightingMasterController::CreateTensionLighting(FVector PlayerLocation, float Intensity)
{
    // Gradually shift to tension building mood when player approaches danger
    float TensionFactor = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (TensionFactor > 0.5f)
    {
        SetAtmosphericMood(EAtmosphericMood::TensionBuilding, 2.0f);
    }
    else if (TensionFactor > 0.8f)
    {
        SetAtmosphericMood(EAtmosphericMood::DangerImminent, 1.0f);
    }
}

ETimeOfDay ALightingMasterController::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ETimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ETimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 14.0f)
        return ETimeOfDay::Noon;
    else if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 17.0f)
        return ETimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 20.0f)
        return ETimeOfDay::Dusk;
    else if (CurrentTimeOfDay >= 20.0f && CurrentTimeOfDay < 23.0f)
        return ETimeOfDay::Night;
    else
        return ETimeOfDay::DeepNight;
}

float ALightingMasterController::GetSunAngle() const
{
    return (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f;
}

bool ALightingMasterController::IsNightTime() const
{
    return CurrentTimeOfDay >= 20.0f || CurrentTimeOfDay < 6.0f;
}