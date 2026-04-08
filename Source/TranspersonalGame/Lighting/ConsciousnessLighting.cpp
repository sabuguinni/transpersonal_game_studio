#include "ConsciousnessLighting.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"

UConsciousnessLighting::UConsciousnessLighting()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default lighting profiles
    FLightingProfile NormalProfile;
    NormalProfile.AmbientColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);
    NormalProfile.AmbientIntensity = 1.0f;
    NormalProfile.DirectionalColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    NormalProfile.DirectionalIntensity = 3.0f;
    NormalProfile.SunAngle = FRotator(-45.0f, 45.0f, 0.0f);
    NormalProfile.FogDensity = 0.02f;
    NormalProfile.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    NormalProfile.VolumetricFogIntensity = 1.0f;
    LightingProfiles.Add(EConsciousnessState::Normal, NormalProfile);

    FLightingProfile MeditativeProfile;
    MeditativeProfile.AmbientColor = FLinearColor(0.3f, 0.5f, 0.7f, 1.0f);
    MeditativeProfile.AmbientIntensity = 0.7f;
    MeditativeProfile.DirectionalColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MeditativeProfile.DirectionalIntensity = 2.0f;
    MeditativeProfile.SunAngle = FRotator(-30.0f, 0.0f, 0.0f);
    MeditativeProfile.FogDensity = 0.04f;
    MeditativeProfile.FogColor = FLinearColor(0.5f, 0.7f, 0.9f, 1.0f);
    MeditativeProfile.VolumetricFogIntensity = 1.5f;
    LightingProfiles.Add(EConsciousnessState::Meditative, MeditativeProfile);

    FLightingProfile TranscendentProfile;
    TranscendentProfile.AmbientColor = FLinearColor(0.8f, 0.6f, 0.9f, 1.0f);
    TranscendentProfile.AmbientIntensity = 1.2f;
    TranscendentProfile.DirectionalColor = FLinearColor(1.0f, 0.8f, 1.0f, 1.0f);
    TranscendentProfile.DirectionalIntensity = 4.0f;
    TranscendentProfile.SunAngle = FRotator(-60.0f, 90.0f, 0.0f);
    TranscendentProfile.FogDensity = 0.01f;
    TranscendentProfile.FogColor = FLinearColor(0.9f, 0.7f, 1.0f, 1.0f);
    TranscendentProfile.VolumetricFogIntensity = 2.0f;
    LightingProfiles.Add(EConsciousnessState::Transcendent, TranscendentProfile);

    FLightingProfile UnityProfile;
    UnityProfile.AmbientColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    UnityProfile.AmbientIntensity = 1.5f;
    UnityProfile.DirectionalColor = FLinearColor(1.0f, 1.0f, 0.9f, 1.0f);
    UnityProfile.DirectionalIntensity = 5.0f;
    UnityProfile.SunAngle = FRotator(0.0f, 0.0f, 0.0f);
    UnityProfile.FogDensity = 0.005f;
    UnityProfile.FogColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    UnityProfile.VolumetricFogIntensity = 3.0f;
    LightingProfiles.Add(EConsciousnessState::Unity, UnityProfile);

    FLightingProfile VoidProfile;
    VoidProfile.AmbientColor = FLinearColor(0.1f, 0.1f, 0.2f, 1.0f);
    VoidProfile.AmbientIntensity = 0.3f;
    VoidProfile.DirectionalColor = FLinearColor(0.2f, 0.2f, 0.4f, 1.0f);
    VoidProfile.DirectionalIntensity = 0.5f;
    VoidProfile.SunAngle = FRotator(-90.0f, 0.0f, 0.0f);
    VoidProfile.FogDensity = 0.08f;
    VoidProfile.FogColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
    VoidProfile.VolumetricFogIntensity = 0.5f;
    LightingProfiles.Add(EConsciousnessState::Void, VoidProfile);

    FLightingProfile CosmicProfile;
    CosmicProfile.AmbientColor = FLinearColor(0.2f, 0.3f, 0.8f, 1.0f);
    CosmicProfile.AmbientIntensity = 2.0f;
    CosmicProfile.DirectionalColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    CosmicProfile.DirectionalIntensity = 6.0f;
    CosmicProfile.SunAngle = FRotator(30.0f, 180.0f, 0.0f);
    CosmicProfile.FogDensity = 0.001f;
    CosmicProfile.FogColor = FLinearColor(0.3f, 0.5f, 1.0f, 1.0f);
    CosmicProfile.VolumetricFogIntensity = 4.0f;
    LightingProfiles.Add(EConsciousnessState::Cosmic, CosmicProfile);
}

void UConsciousnessLighting::BeginPlay()
{
    Super::BeginPlay();
    InitializeLightReferences();
    UpdateLighting();
}

void UConsciousnessLighting::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsTransitioning)
    {
        UpdateTransition(DeltaTime);
    }
}

void UConsciousnessLighting::TransitionToState(EConsciousnessState NewState)
{
    if (NewState == CurrentState && !bIsTransitioning)
    {
        return;
    }

    if (LightingProfiles.Contains(CurrentState))
    {
        PreviousProfile = LightingProfiles[CurrentState];
    }

    if (LightingProfiles.Contains(NewState))
    {
        TargetProfile = LightingProfiles[NewState];
        CurrentState = NewState;
        bIsTransitioning = true;
        TransitionProgress = 0.0f;
    }
}

void UConsciousnessLighting::SetLightingProfile(EConsciousnessState State, const FLightingProfile& Profile)
{
    LightingProfiles.Add(State, Profile);
    
    if (State == CurrentState)
    {
        UpdateLighting();
    }
}

FLightingProfile UConsciousnessLighting::GetCurrentLightingProfile() const
{
    if (LightingProfiles.Contains(CurrentState))
    {
        return LightingProfiles[CurrentState];
    }
    return FLightingProfile();
}

void UConsciousnessLighting::UpdateLighting()
{
    if (LightingProfiles.Contains(CurrentState))
    {
        ApplyLightingProfile(LightingProfiles[CurrentState]);
    }
}

void UConsciousnessLighting::InitializeLightReferences()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }

    // Find point lights
    TArray<AActor*> PointLights;
    UGameplayStatics::GetAllActorsOfClass(World, APointLight::StaticClass(), PointLights);
    for (AActor* Actor : PointLights)
    {
        if (APointLight* PointLight = Cast<APointLight>(Actor))
        {
            AmbientLights.Add(PointLight);
        }
    }

    // Find spot lights
    TArray<AActor*> SpotLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASpotLight::StaticClass(), SpotLights);
    for (AActor* Actor : SpotLights)
    {
        if (ASpotLight* SpotLight = Cast<ASpotLight>(Actor))
        {
            AccentLights.Add(SpotLight);
        }
    }

    // Find height fog
    TArray<AActor*> HeightFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
    if (HeightFogs.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(HeightFogs[0]);
    }
}

void UConsciousnessLighting::UpdateTransition(float DeltaTime)
{
    if (!bIsTransitioning) return;

    float TransitionSpeed = 1.0f / TargetProfile.TransitionDuration;
    TransitionProgress += DeltaTime * TransitionSpeed;

    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        ApplyLightingProfile(TargetProfile);
    }
    else
    {
        InterpolateLighting(PreviousProfile, TargetProfile, TransitionProgress);
    }
}

void UConsciousnessLighting::ApplyLightingProfile(const FLightingProfile& Profile)
{
    // Apply directional light settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* DirLightComp = SunLight->GetLightComponent();
        DirLightComp->SetLightColor(Profile.DirectionalColor);
        DirLightComp->SetIntensity(Profile.DirectionalIntensity);
        SunLight->SetActorRotation(Profile.SunAngle);
    }

    // Apply ambient light settings to point lights
    for (APointLight* PointLight : AmbientLights)
    {
        if (PointLight && PointLight->GetLightComponent())
        {
            UPointLightComponent* PointLightComp = PointLight->GetLightComponent();
            PointLightComp->SetLightColor(Profile.AmbientColor);
            PointLightComp->SetIntensity(Profile.AmbientIntensity);
        }
    }

    // Apply fog settings
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        FogComp->SetFogDensity(Profile.FogDensity);
        FogComp->SetFogInscatteringColor(Profile.FogColor);
        FogComp->SetVolumetricFogScatteringDistribution(Profile.VolumetricFogIntensity);
    }
}

void UConsciousnessLighting::InterpolateLighting(const FLightingProfile& From, const FLightingProfile& To, float Alpha)
{
    FLightingProfile InterpolatedProfile;
    
    InterpolatedProfile.AmbientColor = FLinearColor::LerpUsingHSV(From.AmbientColor, To.AmbientColor, Alpha);
    InterpolatedProfile.AmbientIntensity = FMath::Lerp(From.AmbientIntensity, To.AmbientIntensity, Alpha);
    InterpolatedProfile.DirectionalColor = FLinearColor::LerpUsingHSV(From.DirectionalColor, To.DirectionalColor, Alpha);
    InterpolatedProfile.DirectionalIntensity = FMath::Lerp(From.DirectionalIntensity, To.DirectionalIntensity, Alpha);
    InterpolatedProfile.SunAngle = FMath::Lerp(From.SunAngle, To.SunAngle, Alpha);
    InterpolatedProfile.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    InterpolatedProfile.FogColor = FLinearColor::LerpUsingHSV(From.FogColor, To.FogColor, Alpha);
    InterpolatedProfile.VolumetricFogIntensity = FMath::Lerp(From.VolumetricFogIntensity, To.VolumetricFogIntensity, Alpha);

    ApplyLightingProfile(InterpolatedProfile);
}