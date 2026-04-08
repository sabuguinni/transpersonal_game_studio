#include "ConsciousnessLightingSystem.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/Engine.h"

AConsciousnessLightingSystem::AConsciousnessLightingSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create directional light (sun/moon)
    DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));
    DirectionalLight->SetupAttachment(RootComponent);
    DirectionalLight->SetIntensity(3.0f);
    DirectionalLight->SetLightColor(FLinearColor::White);
    DirectionalLight->SetCastShadows(true);
    DirectionalLight->SetCastVolumetricShadow(true);

    // Create sky light for ambient lighting
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetIntensity(1.0f);
    SkyLight->SetLightColor(FLinearColor::White);
    SkyLight->SetSourceType(SLS_CapturedScene);

    // Create atmospheric fog
    AtmosphericFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("AtmosphericFog"));
    AtmosphericFog->SetupAttachment(RootComponent);
    AtmosphericFog->SetFogDensity(0.02f);
    AtmosphericFog->SetFogHeightFalloff(0.2f);
    AtmosphericFog->SetFogInscatteringColor(FLinearColor(0.447f, 0.638f, 1.0f));

    // Create post process volume for advanced effects
    PostProcessVolume = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessVolume"));
    PostProcessVolume->SetupAttachment(RootComponent);
    PostProcessVolume->bUnbound = true;
}

void AConsciousnessLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingProfiles();
    SetInstantState(CurrentState);
}

void AConsciousnessLightingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeAccumulator += DeltaTime;
    
    if (bIsTransitioning)
    {
        UpdateTransition(DeltaTime);
    }
    
    if (bEnablePulsing || bEnableColorShifting)
    {
        UpdateDynamicEffects(DeltaTime);
    }
}

void AConsciousnessLightingSystem::InitializeLightingProfiles()
{
    // Ordinary Consciousness - Natural daylight
    FLightingProfile OrdinaryProfile;
    OrdinaryProfile.DirectionalLightColor = FLinearColor(1.0f, 0.95f, 0.8f);
    OrdinaryProfile.DirectionalLightIntensity = 3.0f;
    OrdinaryProfile.SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f);
    OrdinaryProfile.SkyLightIntensity = 1.0f;
    OrdinaryProfile.FogColor = FLinearColor(0.447f, 0.638f, 1.0f);
    OrdinaryProfile.FogDensity = 0.02f;
    OrdinaryProfile.SunDirection = FVector(0.3f, 0.5f, -0.8f);
    LightingProfiles.Add(EConsciousnessState::Ordinary, OrdinaryProfile);

    // Meditative State - Soft, warm, golden light
    FLightingProfile MeditativeProfile;
    MeditativeProfile.DirectionalLightColor = FLinearColor(1.0f, 0.8f, 0.4f);
    MeditativeProfile.DirectionalLightIntensity = 2.0f;
    MeditativeProfile.SkyLightColor = FLinearColor(0.9f, 0.7f, 0.5f);
    MeditativeProfile.SkyLightIntensity = 0.8f;
    MeditativeProfile.FogColor = FLinearColor(1.0f, 0.8f, 0.6f);
    MeditativeProfile.FogDensity = 0.03f;
    MeditativeProfile.SunDirection = FVector(0.1f, 0.2f, -0.9f);
    LightingProfiles.Add(EConsciousnessState::Meditative, MeditativeProfile);

    // Transcendent Experience - Bright, ethereal, multi-colored
    FLightingProfile TranscendentProfile;
    TranscendentProfile.DirectionalLightColor = FLinearColor(0.9f, 0.9f, 1.0f);
    TranscendentProfile.DirectionalLightIntensity = 5.0f;
    TranscendentProfile.SkyLightColor = FLinearColor(0.8f, 0.9f, 1.0f);
    TranscendentProfile.SkyLightIntensity = 1.5f;
    TranscendentProfile.FogColor = FLinearColor(0.9f, 0.8f, 1.0f);
    TranscendentProfile.FogDensity = 0.015f;
    TranscendentProfile.SunDirection = FVector(0.0f, 0.0f, -1.0f);
    LightingProfiles.Add(EConsciousnessState::Transcendent, TranscendentProfile);

    // Shadow Integration - Dark, moody, with hints of color
    FLightingProfile ShadowProfile;
    ShadowProfile.DirectionalLightColor = FLinearColor(0.3f, 0.2f, 0.4f);
    ShadowProfile.DirectionalLightIntensity = 1.0f;
    ShadowProfile.SkyLightColor = FLinearColor(0.2f, 0.1f, 0.3f);
    ShadowProfile.SkyLightIntensity = 0.3f;
    ShadowProfile.FogColor = FLinearColor(0.2f, 0.1f, 0.2f);
    ShadowProfile.FogDensity = 0.05f;
    ShadowProfile.SunDirection = FVector(0.8f, 0.3f, -0.5f);
    LightingProfiles.Add(EConsciousnessState::Shadow, ShadowProfile);

    // Unity Consciousness - Pure white light, minimal fog
    FLightingProfile UnityProfile;
    UnityProfile.DirectionalLightColor = FLinearColor::White;
    UnityProfile.DirectionalLightIntensity = 8.0f;
    UnityProfile.SkyLightColor = FLinearColor::White;
    UnityProfile.SkyLightIntensity = 2.0f;
    UnityProfile.FogColor = FLinearColor::White;
    UnityProfile.FogDensity = 0.005f;
    UnityProfile.SunDirection = FVector(0.0f, 0.0f, -1.0f);
    LightingProfiles.Add(EConsciousnessState::Unity, UnityProfile);

    // Void State - Minimal lighting, deep space feel
    FLightingProfile VoidProfile;
    VoidProfile.DirectionalLightColor = FLinearColor(0.05f, 0.05f, 0.1f);
    VoidProfile.DirectionalLightIntensity = 0.1f;
    VoidProfile.SkyLightColor = FLinearColor(0.02f, 0.02f, 0.05f);
    VoidProfile.SkyLightIntensity = 0.1f;
    VoidProfile.FogColor = FLinearColor(0.01f, 0.01f, 0.02f);
    VoidProfile.FogDensity = 0.001f;
    VoidProfile.SunDirection = FVector(0.0f, 0.0f, -1.0f);
    LightingProfiles.Add(EConsciousnessState::Void, VoidProfile);
}

void AConsciousnessLightingSystem::TransitionToState(EConsciousnessState NewState)
{
    if (NewState == CurrentState && !bIsTransitioning)
        return;

    OnStateTransitionStart(CurrentState, NewState);

    TargetState = NewState;
    
    if (LightingProfiles.Contains(CurrentState))
    {
        StartProfile = LightingProfiles[CurrentState];
    }
    
    if (LightingProfiles.Contains(TargetState))
    {
        EndProfile = LightingProfiles[TargetState];
        TransitionDuration = EndProfile.TransitionDuration;
    }

    bIsTransitioning = true;
    TransitionTimer = 0.0f;
    TransitionProgress = 0.0f;
}

void AConsciousnessLightingSystem::SetInstantState(EConsciousnessState NewState)
{
    CurrentState = NewState;
    TargetState = NewState;
    bIsTransitioning = false;
    TransitionProgress = 1.0f;

    if (LightingProfiles.Contains(NewState))
    {
        ApplyLightingProfile(LightingProfiles[NewState]);
    }
}

void AConsciousnessLightingSystem::UpdateTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime;
    TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);

    // Use smooth step for easing
    float EasedProgress = FMath::SmoothStep(0.0f, 1.0f, TransitionProgress);
    
    FLightingProfile InterpolatedProfile = InterpolateLightingProfiles(StartProfile, EndProfile, EasedProgress);
    ApplyLightingProfile(InterpolatedProfile);

    if (TransitionProgress >= 1.0f)
    {
        bIsTransitioning = false;
        CurrentState = TargetState;
        OnStateTransitionComplete(CurrentState);
    }
}

void AConsciousnessLightingSystem::ApplyLightingProfile(const FLightingProfile& Profile)
{
    if (DirectionalLight)
    {
        DirectionalLight->SetLightColor(Profile.DirectionalLightColor);
        DirectionalLight->SetIntensity(Profile.DirectionalLightIntensity);
        
        FRotator SunRotation = Profile.SunDirection.Rotation();
        DirectionalLight->SetWorldRotation(SunRotation);
    }

    if (SkyLight)
    {
        SkyLight->SetLightColor(Profile.SkyLightColor);
        SkyLight->SetIntensity(Profile.SkyLightIntensity);
        SkyLight->RecaptureSky();
    }

    if (AtmosphericFog)
    {
        AtmosphericFog->SetFogInscatteringColor(Profile.FogColor);
        AtmosphericFog->SetFogDensity(Profile.FogDensity);
        AtmosphericFog->SetFogHeightFalloff(Profile.FogHeightFalloff);
    }
}

FLightingProfile AConsciousnessLightingSystem::InterpolateLightingProfiles(const FLightingProfile& From, const FLightingProfile& To, float Alpha)
{
    FLightingProfile Result;
    
    Result.DirectionalLightColor = FMath::Lerp(From.DirectionalLightColor, To.DirectionalLightColor, Alpha);
    Result.DirectionalLightIntensity = FMath::Lerp(From.DirectionalLightIntensity, To.DirectionalLightIntensity, Alpha);
    Result.SkyLightColor = FMath::Lerp(From.SkyLightColor, To.SkyLightColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(From.SkyLightIntensity, To.SkyLightIntensity, Alpha);
    Result.FogColor = FMath::Lerp(From.FogColor, To.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(From.FogHeightFalloff, To.FogHeightFalloff, Alpha);
    Result.SunDirection = FMath::Lerp(From.SunDirection, To.SunDirection, Alpha);
    
    return Result;
}

void AConsciousnessLightingSystem::UpdateDynamicEffects(float DeltaTime)
{
    if (bEnablePulsing && DirectionalLight)
    {
        float PulseValue = FMath::Sin(TimeAccumulator * PulseFrequency * 2.0f * PI) * PulseAmplitude;
        float BaseIntensity = LightingProfiles.Contains(CurrentState) ? 
            LightingProfiles[CurrentState].DirectionalLightIntensity : 3.0f;
        
        DirectionalLight->SetIntensity(BaseIntensity + PulseValue);
    }

    if (bEnableColorShifting && DirectionalLight)
    {
        float HueShift = FMath::Sin(TimeAccumulator * ColorShiftSpeed) * 0.1f;
        FLinearColor BaseColor = LightingProfiles.Contains(CurrentState) ? 
            LightingProfiles[CurrentState].DirectionalLightColor : FLinearColor::White;
        
        FLinearColor ShiftedColor = BaseColor;
        // Simple hue shifting approximation
        ShiftedColor.R = FMath::Clamp(BaseColor.R + HueShift, 0.0f, 1.0f);
        ShiftedColor.G = FMath::Clamp(BaseColor.G + HueShift * 0.5f, 0.0f, 1.0f);
        ShiftedColor.B = FMath::Clamp(BaseColor.B - HueShift * 0.3f, 0.0f, 1.0f);
        
        DirectionalLight->SetLightColor(ShiftedColor);
    }
}

void AConsciousnessLightingSystem::EnableDynamicEffects(bool bEnable)
{
    bEnablePulsing = bEnable;
    bEnableColorShifting = bEnable;
}

void AConsciousnessLightingSystem::SetPulseParameters(float Frequency, float Amplitude)
{
    PulseFrequency = Frequency;
    PulseAmplitude = Amplitude;
}