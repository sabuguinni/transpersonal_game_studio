#include "ConsciousnessLighting.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UConsciousnessLighting::UConsciousnessLighting()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentState = EConsciousnessLightingState::Mundane;
    TargetState = EConsciousnessLightingState::Mundane;
    TransitionProgress = 1.0f;
    TransitionDuration = 2.0f;
    bIsTransitioning = false;
    TransitionSpeed = 1.0f;
}

void UConsciousnessLighting::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingStates();
    
    // Find lighting components in the world
    if (UWorld* World = GetWorld())
    {
        // Find directional light (sun)
        TArray<AActor*> DirectionalLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
        if (DirectionalLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        }
        
        // Find sky light
        TArray<AActor*> SkyLights;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
        if (SkyLights.Num() > 0)
        {
            SkyLight = Cast<ASkyLight>(SkyLights[0]);
        }
        
        // Find height fog
        TArray<AActor*> HeightFogs;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
        if (HeightFogs.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(HeightFogs[0]);
        }
        
        // Find post process volume
        TArray<AActor*> PostProcessVolumes;
        UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), PostProcessVolumes);
        if (PostProcessVolumes.Num() > 0)
        {
            PostProcessVolume = Cast<APostProcessVolume>(PostProcessVolumes[0]);
        }
    }
    
    // Apply initial lighting state
    if (LightingStates.Contains(CurrentState))
    {
        CurrentLightingData = LightingStates[CurrentState];
        ApplyLightingData(CurrentLightingData);
    }
}

void UConsciousnessLighting::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsTransitioning)
    {
        UpdateTransition(DeltaTime);
    }
}

void UConsciousnessLighting::InitializeLightingStates()
{
    // Mundane State - Normal daylight
    FLightingStateData MundaneData;
    MundaneData.AmbientColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);
    MundaneData.DirectionalColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    MundaneData.DirectionalIntensity = 3.0f;
    MundaneData.AmbientIntensity = 0.3f;
    MundaneData.SkyLightIntensity = 1.0f;
    MundaneData.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    MundaneData.FogDensity = 0.02f;
    MundaneData.EtherealParticleIntensity = 0.0f;
    MundaneData.ChromaticAberration = 0.0f;
    MundaneData.Bloom = 0.675f;
    MundaneData.Saturation = 1.0f;
    MundaneData.Contrast = 1.0f;
    LightingStates.Add(EConsciousnessLightingState::Mundane, MundaneData);
    
    // Awakening State - Soft golden light
    FLightingStateData AwakeningData;
    AwakeningData.AmbientColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
    AwakeningData.DirectionalColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    AwakeningData.DirectionalIntensity = 2.5f;
    AwakeningData.AmbientIntensity = 0.4f;
    AwakeningData.SkyLightIntensity = 1.2f;
    AwakeningData.FogColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    AwakeningData.FogDensity = 0.015f;
    AwakeningData.EtherealParticleIntensity = 0.3f;
    AwakeningData.ChromaticAberration = 0.2f;
    AwakeningData.Bloom = 0.8f;
    AwakeningData.Saturation = 1.1f;
    AwakeningData.Contrast = 1.05f;
    LightingStates.Add(EConsciousnessLightingState::Awakening, AwakeningData);
    
    // Meditative State - Cool, serene blues
    FLightingStateData MeditativeData;
    MeditativeData.AmbientColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    MeditativeData.DirectionalColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    MeditativeData.DirectionalIntensity = 1.8f;
    MeditativeData.AmbientIntensity = 0.5f;
    MeditativeData.SkyLightIntensity = 1.5f;
    MeditativeData.FogColor = FLinearColor(0.4f, 0.6f, 0.9f, 1.0f);
    MeditativeData.FogDensity = 0.025f;
    MeditativeData.EtherealParticleIntensity = 0.6f;
    MeditativeData.ChromaticAberration = 0.1f;
    MeditativeData.Bloom = 0.9f;
    MeditativeData.Saturation = 0.9f;
    MeditativeData.Contrast = 0.95f;
    LightingStates.Add(EConsciousnessLightingState::Meditative, MeditativeData);
    
    // Transcendent State - Ethereal purples and golds
    FLightingStateData TranscendentData;
    TranscendentData.AmbientColor = FLinearColor(0.6f, 0.4f, 0.8f, 1.0f);
    TranscendentData.DirectionalColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    TranscendentData.DirectionalIntensity = 2.0f;
    TranscendentData.AmbientIntensity = 0.6f;
    TranscendentData.SkyLightIntensity = 2.0f;
    TranscendentData.FogColor = FLinearColor(0.8f, 0.6f, 1.0f, 1.0f);
    TranscendentData.FogDensity = 0.03f;
    TranscendentData.EtherealParticleIntensity = 0.8f;
    TranscendentData.ChromaticAberration = 0.3f;
    TranscendentData.Bloom = 1.2f;
    TranscendentData.Saturation = 1.3f;
    TranscendentData.Contrast = 1.1f;
    LightingStates.Add(EConsciousnessLightingState::Transcendent, TranscendentData);
    
    // Unity State - Brilliant white light
    FLightingStateData UnityData;
    UnityData.AmbientColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
    UnityData.DirectionalColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    UnityData.DirectionalIntensity = 4.0f;
    UnityData.AmbientIntensity = 0.8f;
    UnityData.SkyLightIntensity = 3.0f;
    UnityData.FogColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    UnityData.FogDensity = 0.01f;
    UnityData.EtherealParticleIntensity = 1.0f;
    UnityData.ChromaticAberration = 0.0f;
    UnityData.Bloom = 1.5f;
    UnityData.Saturation = 0.8f;
    UnityData.Contrast = 1.2f;
    LightingStates.Add(EConsciousnessLightingState::Unity, UnityData);
    
    // Void State - Deep darkness with subtle hints
    FLightingStateData VoidData;
    VoidData.AmbientColor = FLinearColor(0.05f, 0.05f, 0.1f, 1.0f);
    VoidData.DirectionalColor = FLinearColor(0.2f, 0.2f, 0.3f, 1.0f);
    VoidData.DirectionalIntensity = 0.5f;
    VoidData.AmbientIntensity = 0.1f;
    VoidData.SkyLightIntensity = 0.3f;
    VoidData.FogColor = FLinearColor(0.1f, 0.1f, 0.2f, 1.0f);
    VoidData.FogDensity = 0.05f;
    VoidData.EtherealParticleIntensity = 0.2f;
    VoidData.ChromaticAberration = 0.5f;
    VoidData.Bloom = 0.3f;
    VoidData.Saturation = 0.5f;
    VoidData.Contrast = 1.3f;
    LightingStates.Add(EConsciousnessLightingState::Void, VoidData);
}

void UConsciousnessLighting::SetConsciousnessState(EConsciousnessLightingState NewState, float TransitionDuration)
{
    if (NewState == CurrentState && !bIsTransitioning)
    {
        return;
    }
    
    TargetState = NewState;
    this->TransitionDuration = TransitionDuration;
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
    
    if (LightingStates.Contains(TargetState))
    {
        TargetLightingData = LightingStates[TargetState];
    }
}

void UConsciousnessLighting::UpdateLightingForConsciousnessLevel(float ConsciousnessLevel)
{
    // Map consciousness level (0-1) to lighting states
    EConsciousnessLightingState NewState = EConsciousnessLightingState::Mundane;
    
    if (ConsciousnessLevel < 0.2f)
    {
        NewState = EConsciousnessLightingState::Mundane;
    }
    else if (ConsciousnessLevel < 0.4f)
    {
        NewState = EConsciousnessLightingState::Awakening;
    }
    else if (ConsciousnessLevel < 0.6f)
    {
        NewState = EConsciousnessLightingState::Meditative;
    }
    else if (ConsciousnessLevel < 0.8f)
    {
        NewState = EConsciousnessLightingState::Transcendent;
    }
    else
    {
        NewState = EConsciousnessLightingState::Unity;
    }
    
    SetConsciousnessState(NewState, 3.0f);
}

void UConsciousnessLighting::TriggerEnlightenmentFlash()
{
    // Temporarily flash to Unity state and back
    EConsciousnessLightingState PreviousTarget = TargetState;
    SetConsciousnessState(EConsciousnessLightingState::Unity, 0.5f);
    
    // Schedule return to previous state
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, PreviousTarget]()
    {
        SetConsciousnessState(PreviousTarget, 1.0f);
    }, 1.0f, false);
}

void UConsciousnessLighting::SetMeditationAmbience(bool bEnabled)
{
    if (bEnabled)
    {
        SetConsciousnessState(EConsciousnessLightingState::Meditative, 2.0f);
    }
    else
    {
        SetConsciousnessState(EConsciousnessLightingState::Mundane, 2.0f);
    }
}

void UConsciousnessLighting::UpdateTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }
    
    TransitionProgress += (DeltaTime * TransitionSpeed) / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        CurrentState = TargetState;
        CurrentLightingData = TargetLightingData;
    }
    
    // Interpolate between current and target lighting data
    FLightingStateData InterpolatedData = InterpolateLightingData(CurrentLightingData, TargetLightingData, TransitionProgress);
    ApplyLightingData(InterpolatedData);
    
    if (TransitionProgress >= 1.0f)
    {
        CurrentLightingData = TargetLightingData;
    }
}

void UConsciousnessLighting::ApplyLightingData(const FLightingStateData& Data)
{
    // Apply directional light settings
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetLightColor(Data.DirectionalColor);
        SunLight->GetLightComponent()->SetIntensity(Data.DirectionalIntensity);
    }
    
    // Apply sky light settings
    if (SkyLight && SkyLight->GetLightComponent())
    {
        SkyLight->GetLightComponent()->SetIntensity(Data.SkyLightIntensity);
    }
    
    // Apply height fog settings
    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogInscatteringColor(Data.FogColor);
        HeightFog->GetComponent()->SetFogDensity(Data.FogDensity);
    }
    
    // Update post process effects
    UpdatePostProcessEffects(Data);
    
    // Update ethereal particles
    UpdateEtherealParticles(Data.EtherealParticleIntensity);
    
    // Update global material parameters
    if (GlobalMaterialParameters)
    {
        UMaterialParameterCollectionInstance* Instance = GetWorld()->GetParameterCollectionInstance(GlobalMaterialParameters);
        if (Instance)
        {
            Instance->SetVectorParameterValue(FName("ConsciousnessAmbientColor"), Data.AmbientColor);
            Instance->SetScalarParameterValue(FName("ConsciousnessLevel"), static_cast<float>(CurrentState));
            Instance->SetScalarParameterValue(FName("EtherealIntensity"), Data.EtherealParticleIntensity);
        }
    }
}

void UConsciousnessLighting::UpdatePostProcessEffects(const FLightingStateData& Data)
{
    if (!PostProcessVolume)
    {
        return;
    }
    
    FPostProcessSettings& Settings = PostProcessVolume->Settings;
    
    // Update bloom
    Settings.bOverride_BloomIntensity = true;
    Settings.BloomIntensity = Data.Bloom;
    
    // Update chromatic aberration
    Settings.bOverride_ChromaticAberrationIntensity = true;
    Settings.ChromaticAberrationIntensity = Data.ChromaticAberration;
    
    // Update color grading
    Settings.bOverride_ColorSaturation = true;
    Settings.ColorSaturation = FVector4(Data.Saturation, Data.Saturation, Data.Saturation, 1.0f);
    
    Settings.bOverride_ColorContrast = true;
    Settings.ColorContrast = FVector4(Data.Contrast, Data.Contrast, Data.Contrast, 1.0f);
}

void UConsciousnessLighting::UpdateEtherealParticles(float Intensity)
{
    for (UNiagaraComponent* ParticleSystem : EtherealParticleSystems)
    {
        if (ParticleSystem)
        {
            ParticleSystem->SetFloatParameter(FName("Intensity"), Intensity);
            ParticleSystem->SetFloatParameter(FName("SpawnRate"), Intensity * 100.0f);
        }
    }
}

FLightingStateData UConsciousnessLighting::InterpolateLightingData(const FLightingStateData& From, const FLightingStateData& To, float Alpha)
{
    FLightingStateData Result;
    
    Result.AmbientColor = FMath::Lerp(From.AmbientColor, To.AmbientColor, Alpha);
    Result.DirectionalColor = FMath::Lerp(From.DirectionalColor, To.DirectionalColor, Alpha);
    Result.DirectionalIntensity = FMath::Lerp(From.DirectionalIntensity, To.DirectionalIntensity, Alpha);
    Result.AmbientIntensity = FMath::Lerp(From.AmbientIntensity, To.AmbientIntensity, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(From.SkyLightIntensity, To.SkyLightIntensity, Alpha);
    Result.FogColor = FMath::Lerp(From.FogColor, To.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    Result.EtherealParticleIntensity = FMath::Lerp(From.EtherealParticleIntensity, To.EtherealParticleIntensity, Alpha);
    Result.ChromaticAberration = FMath::Lerp(From.ChromaticAberration, To.ChromaticAberration, Alpha);
    Result.Bloom = FMath::Lerp(From.Bloom, To.Bloom, Alpha);
    Result.Saturation = FMath::Lerp(From.Saturation, To.Saturation, Alpha);
    Result.Contrast = FMath::Lerp(From.Contrast, To.Contrast, Alpha);
    
    return Result;
}