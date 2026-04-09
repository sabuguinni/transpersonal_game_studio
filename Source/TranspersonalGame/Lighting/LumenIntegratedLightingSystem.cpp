#include "LumenIntegratedLightingSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/VolumetricCloud.h"
#include "Components/VolumetricCloudComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "RenderingThread.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"

ULumenIntegratedLightingSystem::ULumenIntegratedLightingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    // Initialize default state
    CurrentState.SunRotation = FRotator(-45.0f, 0.0f, 0.0f);
    CurrentState.SunColor = FLinearColor::White;
    CurrentState.SunIntensity = 10.0f;
    CurrentState.FogDensity = 0.02f;
    CurrentState.FogHeightFalloff = 0.2f;
    CurrentState.FogInscatteringColor = FLinearColor(0.447f, 0.639f, 1.0f, 1.0f);
    
    TargetState = CurrentState;
}

void ULumenIntegratedLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingComponents();
    InitializeMoodPresets();
    ConfigureLumenSettings();
    ConfigureVirtualShadowMaps();
    
    UE_LOG(LogTemp, Warning, TEXT("Lumen Integrated Lighting System initialized"));
}

void ULumenIntegratedLightingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsTransitioning)
    {
        UpdateLightingTransition(DeltaTime);
    }
    
    UpdateTimeOfDayLighting();
}

void ULumenIntegratedLightingSystem::InitializeLightingComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find or create directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }
    else
    {
        // Create sun light
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("SunLight");
        SunLight = World->SpawnActor<ADirectionalLight>(SpawnParams);
        
        if (SunLight && SunLight->GetLightComponent())
        {
            SunLight->GetLightComponent()->SetAtmosphereSunLight(true);
            SunLight->GetLightComponent()->SetCastShadows(true);
            SunLight->GetLightComponent()->SetCastVolumetricShadow(true);
        }
    }

    // Find or create sky atmosphere
    TArray<AActor*> SkyAtmospheres;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyAtmospheres);
    
    if (SkyAtmospheres.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(SkyAtmospheres[0]);
    }
    else
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("SkyAtmosphere");
        SkyAtmosphere = World->SpawnActor<ASkyAtmosphere>(SpawnParams);
    }

    // Find or create sky light
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    
    if (SkyLights.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(SkyLights[0]);
    }
    else
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("SkyLight");
        SkyLight = World->SpawnActor<ASkyLight>(SpawnParams);
        
        if (SkyLight && SkyLight->GetLightComponent())
        {
            SkyLight->GetLightComponent()->SetRealTimeCapture(true);
            SkyLight->GetLightComponent()->SetSourceType(SLS_CapturedScene);
        }
    }

    // Find or create exponential height fog
    TArray<AActor*> HeightFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
    
    if (HeightFogs.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(HeightFogs[0]);
    }
    else
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("HeightFog");
        HeightFog = World->SpawnActor<AExponentialHeightFog>(SpawnParams);
    }

    // Find or create post process volume
    TArray<AActor*> PostProcessVolumes;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), PostProcessVolumes);
    
    if (PostProcessVolumes.Num() > 0)
    {
        MasterPostProcess = Cast<APostProcessVolume>(PostProcessVolumes[0]);
    }
    else
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("MasterPostProcess");
        MasterPostProcess = World->SpawnActor<APostProcessVolume>(SpawnParams);
        
        if (MasterPostProcess)
        {
            MasterPostProcess->bUnbound = true;
        }
    }

    // Find or create volumetric clouds
    TArray<AActor*> VolClouds;
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), VolClouds);
    
    if (VolClouds.Num() > 0)
    {
        VolumetricClouds = Cast<AVolumetricCloud>(VolClouds[0]);
    }
    else
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("VolumetricClouds");
        VolumetricClouds = World->SpawnActor<AVolumetricCloud>(SpawnParams);
    }
}

void ULumenIntegratedLightingSystem::InitializeMoodPresets()
{
    // Dawn - Hopeful
    FAtmosphericLightingState DawnState;
    DawnState.SunRotation = FRotator(-15.0f, 90.0f, 0.0f);
    DawnState.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnState.SunIntensity = 5.0f;
    DawnState.FogDensity = 0.05f;
    DawnState.FogInscatteringColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnState.ExposureBias = 0.5f;
    DawnState.ColorGradingTint = FLinearColor(1.1f, 0.9f, 0.8f, 1.0f);
    MoodPresets.Add(EAtmosphericMood::Dawn_Hopeful, DawnState);

    // Morning - Energetic
    FAtmosphericLightingState MorningState;
    MorningState.SunRotation = FRotator(-30.0f, 120.0f, 0.0f);
    MorningState.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    MorningState.SunIntensity = 8.0f;
    MorningState.FogDensity = 0.02f;
    MorningState.FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MorningState.ExposureBias = 0.0f;
    MorningState.ColorGradingTint = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    MoodPresets.Add(EAtmosphericMood::Morning_Energetic, MorningState);

    // Midday - Intense
    FAtmosphericLightingState MiddayState;
    MiddayState.SunRotation = FRotator(-80.0f, 180.0f, 0.0f);
    MiddayState.SunColor = FLinearColor::White;
    MiddayState.SunIntensity = 12.0f;
    MiddayState.FogDensity = 0.01f;
    MiddayState.FogInscatteringColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    MiddayState.ExposureBias = -0.2f;
    MiddayState.ColorGradingTint = FLinearColor(0.95f, 0.98f, 1.0f, 1.0f);
    MoodPresets.Add(EAtmosphericMood::Midday_Intense, MiddayState);

    // Afternoon - Warm
    FAtmosphericLightingState AfternoonState;
    AfternoonState.SunRotation = FRotator(-45.0f, 240.0f, 0.0f);
    AfternoonState.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AfternoonState.SunIntensity = 9.0f;
    AfternoonState.FogDensity = 0.025f;
    AfternoonState.FogInscatteringColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    AfternoonState.ExposureBias = 0.2f;
    AfternoonState.ColorGradingTint = FLinearColor(1.1f, 1.0f, 0.9f, 1.0f);
    MoodPresets.Add(EAtmosphericMood::Afternoon_Warm, AfternoonState);

    // Dusk - Mysterious
    FAtmosphericLightingState DuskState;
    DuskState.SunRotation = FRotator(-5.0f, 270.0f, 0.0f);
    DuskState.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskState.SunIntensity = 3.0f;
    DuskState.FogDensity = 0.04f;
    DuskState.FogInscatteringColor = FLinearColor(0.8f, 0.4f, 0.6f, 1.0f);
    DuskState.ExposureBias = 0.8f;
    DuskState.ColorGradingTint = FLinearColor(1.2f, 0.8f, 0.7f, 1.0f);
    MoodPresets.Add(EAtmosphericMood::Dusk_Mysterious, DuskState);

    // Night - Dangerous
    FAtmosphericLightingState NightState;
    NightState.SunRotation = FRotator(15.0f, 0.0f, 0.0f);
    NightState.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightState.SunIntensity = 0.5f;
    NightState.FogDensity = 0.03f;
    NightState.FogInscatteringColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    NightState.ExposureBias = 1.5f;
    NightState.ColorGradingTint = FLinearColor(0.8f, 0.9f, 1.2f, 1.0f);
    MoodPresets.Add(EAtmosphericMood::Night_Dangerous, NightState);

    // Storm - Threatening
    FAtmosphericLightingState StormState;
    StormState.SunRotation = FRotator(-30.0f, 180.0f, 0.0f);
    StormState.SunColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    StormState.SunIntensity = 4.0f;
    StormState.FogDensity = 0.08f;
    StormState.FogInscatteringColor = FLinearColor(0.4f, 0.5f, 0.6f, 1.0f);
    StormState.ExposureBias = 0.5f;
    StormState.ColorGradingTint = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
    MoodPresets.Add(EAtmosphericMood::Storm_Threatening, StormState);

    // Fog - Eerie
    FAtmosphericLightingState FogState;
    FogState.SunRotation = FRotator(-20.0f, 180.0f, 0.0f);
    FogState.SunColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
    FogState.SunIntensity = 2.0f;
    FogState.FogDensity = 0.15f;
    FogState.FogInscatteringColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    FogState.ExposureBias = 1.0f;
    FogState.ColorGradingTint = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    MoodPresets.Add(EAtmosphericMood::Fog_Eerie, FogState);
}

void ULumenIntegratedLightingSystem::SetAtmosphericMood(EAtmosphericMood NewMood, float TransitionDuration)
{
    if (MoodPresets.Contains(NewMood))
    {
        CurrentMood = NewMood;
        TargetState = MoodPresets[NewMood];
        
        bIsTransitioning = true;
        TransitionProgress = 0.0f;
        this->TransitionDuration = TransitionDuration;
        
        OnMoodChanged(NewMood);
        
        UE_LOG(LogTemp, Warning, TEXT("Transitioning to atmospheric mood: %d"), (int32)NewMood);
    }
}

void ULumenIntegratedLightingSystem::SetTimeOfDay(float TimeOfDay, bool bInstant)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 24.0f);
    
    if (bInstant)
    {
        UpdateTimeOfDayLighting();
    }
    
    OnTimeOfDayChanged(CurrentTimeOfDay);
}

void ULumenIntegratedLightingSystem::SetWeatherCondition(float CloudCoverage, float FogIntensity, float RainIntensity)
{
    if (VolumetricClouds && VolumetricClouds->GetVolumetricCloudComponent())
    {
        // Update cloud coverage
        // Note: This would require access to the cloud material parameters
        UE_LOG(LogTemp, Warning, TEXT("Setting cloud coverage: %f"), CloudCoverage);
    }
    
    if (HeightFog && HeightFog->GetComponent())
    {
        float NewFogDensity = CurrentState.FogDensity * (1.0f + FogIntensity * 2.0f);
        HeightFog->GetComponent()->SetFogDensity(NewFogDensity);
    }
}

void ULumenIntegratedLightingSystem::SetLightingQuality(ELightingQuality Quality)
{
    LightingQuality = Quality;
    ConfigureLumenSettings();
    ConfigureVirtualShadowMaps();
}

void ULumenIntegratedLightingSystem::SetLumenGlobalIllumination(bool bEnabled, float Quality)
{
    bUseLumenGI = bEnabled;
    
    if (MasterPostProcess)
    {
        // Configure Lumen GI settings through console variables
        FString Command = FString::Printf(TEXT("r.Lumen.GlobalIllumination.FinalGatherQuality %f"), Quality);
        GetWorld()->Exec(GetWorld(), *Command);
    }
}

void ULumenIntegratedLightingSystem::SetLumenReflections(bool bEnabled, float Quality)
{
    bUseLumenReflections = bEnabled;
    
    if (MasterPostProcess)
    {
        FString Command = FString::Printf(TEXT("r.Lumen.Reflections.Quality %f"), Quality);
        GetWorld()->Exec(GetWorld(), *Command);
    }
}

void ULumenIntegratedLightingSystem::SetLumenSceneLightingUpdateSpeed(float Speed)
{
    FString Command = FString::Printf(TEXT("r.Lumen.SceneLightingUpdateSpeed %f"), Speed);
    GetWorld()->Exec(GetWorld(), *Command);
}

void ULumenIntegratedLightingSystem::UpdateSkyAtmosphere()
{
    if (!SkyAtmosphere || !SkyAtmosphere->GetSkyAtmosphereComponent()) return;
    
    USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetSkyAtmosphereComponent();
    
    // Update atmosphere properties based on current state
    SkyComp->SetAtmosphereHeight(CurrentState.AtmosphereHeight);
    SkyComp->SetRayleighScattering(CurrentState.RayleighScattering);
    SkyComp->SetMieScatteringScale(CurrentState.MieScatteringScale);
}

void ULumenIntegratedLightingSystem::SetVolumetricClouds(bool bEnabled, float Coverage, float Density)
{
    if (VolumetricClouds)
    {
        VolumetricClouds->SetActorHiddenInGame(!bEnabled);
        
        if (bEnabled && VolumetricClouds->GetVolumetricCloudComponent())
        {
            // Configure cloud parameters
            UE_LOG(LogTemp, Warning, TEXT("Configuring volumetric clouds: Coverage=%f, Density=%f"), Coverage, Density);
        }
    }
}

void ULumenIntegratedLightingSystem::SetInteriorLightingMode(bool bIsInterior, float AmbientIntensity)
{
    if (SkyLight && SkyLight->GetLightComponent())
    {
        if (bIsInterior)
        {
            SkyLight->GetLightComponent()->SetIntensity(AmbientIntensity);
            SkyLight->GetLightComponent()->SetRealTimeCapture(false);
        }
        else
        {
            SkyLight->GetLightComponent()->SetIntensity(1.0f);
            SkyLight->GetLightComponent()->SetRealTimeCapture(true);
        }
    }
}

void ULumenIntegratedLightingSystem::OptimizeForDistance(float ViewDistance)
{
    // Adjust Lumen quality based on view distance
    float QualityScale = FMath::Clamp(1000.0f / ViewDistance, 0.3f, 1.0f);
    
    FString Command = FString::Printf(TEXT("r.Lumen.TraceMeshSDFs.StepFactor %f"), 2.0f / QualityScale);
    GetWorld()->Exec(GetWorld(), *Command);
}

void ULumenIntegratedLightingSystem::SetShadowQuality(float Quality)
{
    if (bUseVirtualShadowMaps)
    {
        FString Command = FString::Printf(TEXT("r.Shadow.Virtual.ResolutionLodBiasDirectional %f"), -Quality);
        GetWorld()->Exec(GetWorld(), *Command);
    }
}

void ULumenIntegratedLightingSystem::UpdateLightingTransition(float DeltaTime)
{
    if (!bIsTransitioning) return;
    
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        CurrentState = TargetState;
    }
    else
    {
        CurrentState = InterpolateLightingStates(CurrentState, TargetState, TransitionProgress);
    }
    
    ApplyLightingState(CurrentState);
}

void ULumenIntegratedLightingSystem::ApplyLightingState(const FAtmosphericLightingState& State)
{
    // Apply sun properties
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->SetActorRotation(State.SunRotation);
        SunLight->GetLightComponent()->SetLightColor(State.SunColor);
        SunLight->GetLightComponent()->SetIntensity(State.SunIntensity);
    }
    
    // Apply fog properties
    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogDensity(State.FogDensity);
        HeightFog->GetComponent()->SetFogHeightFalloff(State.FogHeightFalloff);
        HeightFog->GetComponent()->SetFogInscatteringColor(State.FogInscatteringColor);
    }
    
    // Apply post process properties
    if (MasterPostProcess)
    {
        // Update exposure and color grading
        // Note: This would require direct manipulation of post process settings
        UE_LOG(LogTemp, Verbose, TEXT("Applying post process state: ExposureBias=%f"), State.ExposureBias);
    }
    
    UpdateSkyAtmosphere();
}

void ULumenIntegratedLightingSystem::UpdateTimeOfDayLighting()
{
    float SunElevation = CalculateSunElevation(CurrentTimeOfDay);
    FLinearColor SunColor = CalculateSunColor(CurrentTimeOfDay, SunElevation);
    
    if (SunLight && SunLight->GetLightComponent())
    {
        FRotator SunRotation = FRotator(SunElevation, CurrentTimeOfDay * 15.0f - 90.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
        SunLight->GetLightComponent()->SetLightColor(SunColor);
        
        // Adjust intensity based on sun elevation
        float IntensityMultiplier = FMath::Clamp(SunElevation / 90.0f + 0.1f, 0.1f, 1.0f);
        SunLight->GetLightComponent()->SetIntensity(CurrentState.SunIntensity * IntensityMultiplier);
    }
}

void ULumenIntegratedLightingSystem::ConfigureLumenSettings()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Configure Lumen based on quality setting
    switch (LightingQuality)
    {
        case ELightingQuality::Performance:
            World->Exec(World, TEXT("r.Lumen.GlobalIllumination.FinalGatherQuality 0.5"));
            World->Exec(World, TEXT("r.Lumen.Reflections.Quality 0.5"));
            World->Exec(World, TEXT("r.Lumen.TraceMeshSDFs.StepFactor 4.0"));
            break;
            
        case ELightingQuality::Balanced:
            World->Exec(World, TEXT("r.Lumen.GlobalIllumination.FinalGatherQuality 1.0"));
            World->Exec(World, TEXT("r.Lumen.Reflections.Quality 1.0"));
            World->Exec(World, TEXT("r.Lumen.TraceMeshSDFs.StepFactor 2.0"));
            break;
            
        case ELightingQuality::Quality:
            World->Exec(World, TEXT("r.Lumen.GlobalIllumination.FinalGatherQuality 1.5"));
            World->Exec(World, TEXT("r.Lumen.Reflections.Quality 1.5"));
            World->Exec(World, TEXT("r.Lumen.TraceMeshSDFs.StepFactor 1.0"));
            break;
            
        case ELightingQuality::Cinematic:
            World->Exec(World, TEXT("r.Lumen.GlobalIllumination.FinalGatherQuality 2.0"));
            World->Exec(World, TEXT("r.Lumen.Reflections.Quality 2.0"));
            World->Exec(World, TEXT("r.Lumen.TraceMeshSDFs.StepFactor 0.5"));
            break;
    }
}

void ULumenIntegratedLightingSystem::ConfigureVirtualShadowMaps()
{
    if (!bUseVirtualShadowMaps) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Enable Virtual Shadow Maps
    World->Exec(World, TEXT("r.Shadow.Virtual.Enable 1"));
    
    // Configure quality based on lighting quality setting
    switch (LightingQuality)
    {
        case ELightingQuality::Performance:
            World->Exec(World, TEXT("r.Shadow.Virtual.ResolutionLodBiasDirectional 1"));
            World->Exec(World, TEXT("r.Shadow.Virtual.SMRT.RayCountDirectional 4"));
            break;
            
        case ELightingQuality::Balanced:
            World->Exec(World, TEXT("r.Shadow.Virtual.ResolutionLodBiasDirectional 0"));
            World->Exec(World, TEXT("r.Shadow.Virtual.SMRT.RayCountDirectional 7"));
            break;
            
        case ELightingQuality::Quality:
            World->Exec(World, TEXT("r.Shadow.Virtual.ResolutionLodBiasDirectional -0.5"));
            World->Exec(World, TEXT("r.Shadow.Virtual.SMRT.RayCountDirectional 12"));
            break;
            
        case ELightingQuality::Cinematic:
            World->Exec(World, TEXT("r.Shadow.Virtual.ResolutionLodBiasDirectional -1"));
            World->Exec(World, TEXT("r.Shadow.Virtual.SMRT.RayCountDirectional 16"));
            break;
    }
}

FAtmosphericLightingState ULumenIntegratedLightingSystem::InterpolateLightingStates(const FAtmosphericLightingState& A, const FAtmosphericLightingState& B, float Alpha)
{
    FAtmosphericLightingState Result;
    
    Result.SunRotation = FMath::Lerp(A.SunRotation, B.SunRotation, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogInscatteringColor = FMath::Lerp(A.FogInscatteringColor, B.FogInscatteringColor, Alpha);
    Result.ExposureBias = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    Result.ColorGradingTint = FMath::Lerp(A.ColorGradingTint, B.ColorGradingTint, Alpha);
    
    return Result;
}

float ULumenIntegratedLightingSystem::CalculateSunElevation(float TimeOfDay)
{
    // Convert time of day to sun elevation angle
    // Peak at noon (12:00), lowest at midnight (0:00/24:00)
    float NormalizedTime = FMath::Abs(TimeOfDay - 12.0f) / 12.0f;
    return FMath::Lerp(90.0f, -15.0f, NormalizedTime);
}

FLinearColor ULumenIntegratedLightingSystem::CalculateSunColor(float TimeOfDay, float SunElevation)
{
    // Warmer colors during sunrise/sunset, cooler during day/night
    if (SunElevation > 45.0f)
    {
        // Midday - neutral white
        return FLinearColor::White;
    }
    else if (SunElevation > 0.0f)
    {
        // Sunrise/sunset - warm colors
        float WarmFactor = 1.0f - (SunElevation / 45.0f);
        return FMath::Lerp(FLinearColor::White, FLinearColor(1.0f, 0.7f, 0.4f, 1.0f), WarmFactor);
    }
    else
    {
        // Night - cool blue
        return FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    }
}