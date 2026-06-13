#include "Light_DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Math/UnrealMathUtility.h"

ALight_DayNightCycleManager::ALight_DayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    SunLight = nullptr;
    SkyAtmosphere = nullptr;
    VolumeFog = nullptr;
    
    InitializeCretaceousSettings();
}

void ALight_DayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    
    if (bUseCretaceousLighting)
    {
        ApplyCretaceousAtmosphere();
    }
    
    UpdateLighting();
}

void ALight_DayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoAdvanceTime)
    {
        float TimeIncrement = (24.0f / (CycleDurationMinutes * 60.0f)) * DeltaTime;
        CurrentTimeOfDay += TimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
        
        UpdateLighting();
    }
}

void ALight_DayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateLighting();
}

ELight_TimeOfDay ALight_DayNightCycleManager::GetCurrentTimeOfDay() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ELight_TimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 21.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void ALight_DayNightCycleManager::ApplyCretaceousAtmosphere()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(5.0f);
        LightComp->SetLightColor(FLinearColor(1.0f, 0.94f, 0.78f, 1.0f)); // Warm amber
        LightComp->SetCastShadows(true);
        LightComp->SetCastVolumetricShadow(true);
    }
    
    if (VolumeFog && VolumeFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = VolumeFog->GetComponent();
        FogComp->SetFogDensity(0.02f);
        FogComp->SetFogHeightFalloff(0.2f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.7f, 0.55f, 0.39f, 1.0f));
        FogComp->SetVolumetricFog(true);
    }
}

void ALight_DayNightCycleManager::UpdateLighting()
{
    if (!SunLight || !SunLight->GetLightComponent())
        return;
    
    ELight_TimeOfDay CurrentPeriod = GetCurrentTimeOfDay();
    
    if (TimeSettings.Contains(CurrentPeriod))
    {
        const FLight_TimeSettings& Settings = TimeSettings[CurrentPeriod];
        
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
        
        // Calculate sun angle based on time
        float SunAngle = ((CurrentTimeOfDay - 6.0f) / 12.0f) * 180.0f - 90.0f;
        SunAngle = FMath::Clamp(SunAngle, -90.0f, 90.0f);
        
        FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
        
        // Update fog if available
        if (VolumeFog && VolumeFog->GetComponent())
        {
            UExponentialHeightFogComponent* FogComp = VolumeFog->GetComponent();
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }
}

void ALight_DayNightCycleManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Find DirectionalLight
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    // Find SkyAtmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }
    
    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        VolumeFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}

void ALight_DayNightCycleManager::InterpolateLightingSettings(const FLight_TimeSettings& From, const FLight_TimeSettings& To, float Alpha)
{
    if (!SunLight || !SunLight->GetLightComponent())
        return;
    
    UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
    
    float InterpIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    FLinearColor InterpColor = FMath::Lerp(From.SunColor, To.SunColor, Alpha);
    
    LightComp->SetIntensity(InterpIntensity);
    LightComp->SetLightColor(InterpColor);
}

void ALight_DayNightCycleManager::InitializeCretaceousSettings()
{
    // Dawn - Soft orange light
    FLight_TimeSettings DawnSettings;
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    TimeSettings.Add(ELight_TimeOfDay::Dawn, DawnSettings);
    
    // Morning - Warm golden
    FLight_TimeSettings MorningSettings;
    MorningSettings.SunIntensity = 4.0f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningSettings.FogDensity = 0.03f;
    MorningSettings.FogColor = FLinearColor(0.7f, 0.6f, 0.4f, 1.0f);
    TimeSettings.Add(ELight_TimeOfDay::Morning, MorningSettings);
    
    // Midday - Bright amber
    FLight_TimeSettings MiddaySettings;
    MiddaySettings.SunIntensity = 6.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    MiddaySettings.FogDensity = 0.02f;
    MiddaySettings.FogColor = FLinearColor(0.7f, 0.55f, 0.39f, 1.0f);
    TimeSettings.Add(ELight_TimeOfDay::Midday, MiddaySettings);
    
    // Afternoon - Warm amber
    FLight_TimeSettings AfternoonSettings;
    AfternoonSettings.SunIntensity = 4.5f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    AfternoonSettings.FogDensity = 0.025f;
    AfternoonSettings.FogColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    TimeSettings.Add(ELight_TimeOfDay::Afternoon, AfternoonSettings);
    
    // Dusk - Deep orange
    FLight_TimeSettings DuskSettings;
    DuskSettings.SunIntensity = 1.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DuskSettings.FogDensity = 0.05f;
    DuskSettings.FogColor = FLinearColor(0.9f, 0.4f, 0.2f, 1.0f);
    TimeSettings.Add(ELight_TimeOfDay::Dusk, DuskSettings);
    
    // Night - Cool moonlight
    FLight_TimeSettings NightSettings;
    NightSettings.SunIntensity = 0.3f;
    NightSettings.SunColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    NightSettings.FogDensity = 0.06f;
    NightSettings.FogColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    TimeSettings.Add(ELight_TimeOfDay::Night, NightSettings);
}