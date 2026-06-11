#include "Light_VolumetricFogSystem.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ULight_VolumetricFogSystem::ULight_VolumetricFogSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    InitializeFogPresets();
}

void ULight_VolumetricFogSystem::BeginPlay()
{
    Super::BeginPlay();
    
    FindHeightFogActor();
    SetCretaceousFogSettings();
}

void ULight_VolumetricFogSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsTransitioning)
    {
        UpdateFogTransition(DeltaTime);
    }
}

void ULight_VolumetricFogSystem::FindHeightFogActor()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> HeightFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
    
    if (HeightFogs.Num() > 0)
    {
        HeightFogActor = Cast<AExponentialHeightFog>(HeightFogs[0]);
    }
}

void ULight_VolumetricFogSystem::InitializeFogPresets()
{
    // Clear weather - minimal fog
    ClearWeatherFog.FogDensity = 0.005f;
    ClearWeatherFog.FogHeightFalloff = 0.1f;
    ClearWeatherFog.FogMaxOpacity = 0.6f;
    ClearWeatherFog.FogInscatteringColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    ClearWeatherFog.DirectionalInscatteringColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    ClearWeatherFog.DirectionalInscatteringExponent = 8.0f;
    
    // Light fog - Cretaceous atmospheric haze
    LightFogWeather.FogDensity = 0.02f;
    LightFogWeather.FogHeightFalloff = 0.2f;
    LightFogWeather.FogMaxOpacity = 0.8f;
    LightFogWeather.FogInscatteringColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    LightFogWeather.DirectionalInscatteringColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    LightFogWeather.DirectionalInscatteringExponent = 6.0f;
    
    // Heavy fog - dense prehistoric atmosphere
    HeavyFogWeather.FogDensity = 0.08f;
    HeavyFogWeather.FogHeightFalloff = 0.4f;
    HeavyFogWeather.FogMaxOpacity = 1.0f;
    HeavyFogWeather.FogInscatteringColor = FLinearColor(0.9f, 0.9f, 0.85f, 1.0f);
    HeavyFogWeather.DirectionalInscatteringColor = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f);
    HeavyFogWeather.DirectionalInscatteringExponent = 4.0f;
    
    // Mist - early morning/evening
    MistWeather.FogDensity = 0.04f;
    MistWeather.FogHeightFalloff = 0.15f;
    MistWeather.FogMaxOpacity = 0.9f;
    MistWeather.FogInscatteringColor = FLinearColor(0.95f, 0.95f, 1.0f, 1.0f);
    MistWeather.DirectionalInscatteringColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    MistWeather.DirectionalInscatteringExponent = 5.0f;
    
    // Haze - hot day atmospheric distortion
    HazeWeather.FogDensity = 0.015f;
    HazeWeather.FogHeightFalloff = 0.05f;
    HazeWeather.FogMaxOpacity = 0.7f;
    HazeWeather.FogInscatteringColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    HazeWeather.DirectionalInscatteringColor = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f);
    HazeWeather.DirectionalInscatteringExponent = 10.0f;
    
    // Storm - dramatic weather
    StormWeather.FogDensity = 0.12f;
    StormWeather.FogHeightFalloff = 0.6f;
    StormWeather.FogMaxOpacity = 1.0f;
    StormWeather.FogInscatteringColor = FLinearColor(0.7f, 0.7f, 0.8f, 1.0f);
    StormWeather.DirectionalInscatteringColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
    StormWeather.DirectionalInscatteringExponent = 2.0f;
}

void ULight_VolumetricFogSystem::SetWeatherType(ELight_WeatherType NewWeather)
{
    CurrentWeather = NewWeather;
    FLight_FogPreset Preset = GetPresetForWeather(NewWeather);
    ApplyFogPreset(Preset);
}

void ULight_VolumetricFogSystem::TransitionToWeather(ELight_WeatherType TargetWeather, float TransitionDuration)
{
    if (CurrentWeather == TargetWeather) return;
    
    CurrentFogSettings = GetPresetForWeather(CurrentWeather);
    TargetFogSettings = GetPresetForWeather(TargetWeather);
    
    this->TransitionDuration = TransitionDuration;
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
    
    CurrentWeather = TargetWeather;
}

void ULight_VolumetricFogSystem::ApplyFogPreset(const FLight_FogPreset& Preset)
{
    if (!HeightFogActor || !HeightFogActor->GetComponent()) return;
    
    UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
    
    FogComp->SetFogDensity(Preset.FogDensity);
    FogComp->SetFogHeightFalloff(Preset.FogHeightFalloff);
    FogComp->SetFogMaxOpacity(Preset.FogMaxOpacity);
    FogComp->SetFogInscatteringColor(Preset.FogInscatteringColor);
    FogComp->SetDirectionalInscatteringColor(Preset.DirectionalInscatteringColor);
    FogComp->SetDirectionalInscatteringExponent(Preset.DirectionalInscatteringExponent);
}

void ULight_VolumetricFogSystem::SetCretaceousFogSettings()
{
    // Apply default Cretaceous period atmospheric settings
    SetWeatherType(ELight_WeatherType::LightFog);
}

void ULight_VolumetricFogSystem::UpdateFogTransition(float DeltaTime)
{
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
    }
    
    FLight_FogPreset InterpolatedSettings = LerpFogPresets(CurrentFogSettings, TargetFogSettings, TransitionProgress);
    ApplyFogPreset(InterpolatedSettings);
}

FLight_FogPreset ULight_VolumetricFogSystem::GetPresetForWeather(ELight_WeatherType Weather) const
{
    switch (Weather)
    {
        case ELight_WeatherType::Clear:
            return ClearWeatherFog;
        case ELight_WeatherType::LightFog:
            return LightFogWeather;
        case ELight_WeatherType::HeavyFog:
            return HeavyFogWeather;
        case ELight_WeatherType::Mist:
            return MistWeather;
        case ELight_WeatherType::Haze:
            return HazeWeather;
        case ELight_WeatherType::Storm:
            return StormWeather;
        default:
            return LightFogWeather;
    }
}

FLight_FogPreset ULight_VolumetricFogSystem::LerpFogPresets(const FLight_FogPreset& A, const FLight_FogPreset& B, float Alpha) const
{
    FLight_FogPreset Result;
    
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogMaxOpacity = FMath::Lerp(A.FogMaxOpacity, B.FogMaxOpacity, Alpha);
    Result.FogInscatteringColor = FMath::Lerp(A.FogInscatteringColor, B.FogInscatteringColor, Alpha);
    Result.DirectionalInscatteringColor = FMath::Lerp(A.DirectionalInscatteringColor, B.DirectionalInscatteringColor, Alpha);
    Result.DirectionalInscatteringExponent = FMath::Lerp(A.DirectionalInscatteringExponent, B.DirectionalInscatteringExponent, Alpha);
    
    return Result;
}