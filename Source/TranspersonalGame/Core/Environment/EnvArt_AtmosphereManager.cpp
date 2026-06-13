#include "EnvArt_AtmosphereManager.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    InitializeLightingPresets();
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindSceneLightingActors();
    
    // Apply initial lighting settings
    if (LightingPresets.Contains(CurrentTimeOfDay))
    {
        ApplyLightingSettings(LightingPresets[CurrentTimeOfDay], true);
    }
}

void AEnvArt_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoAdvanceTime)
    {
        // Advance time automatically
        float TimeIncrement = DeltaTime / (DayDurationMinutes * 60.0f);
        CurrentTimeNormalized += TimeIncrement;
        
        if (CurrentTimeNormalized >= 1.0f)
        {
            CurrentTimeNormalized = 0.0f;
        }
        
        // Update time of day based on normalized time
        EEnvArt_TimeOfDay NewTimeOfDay = CurrentTimeOfDay;
        if (CurrentTimeNormalized < 0.1f)
            NewTimeOfDay = EEnvArt_TimeOfDay::Night;
        else if (CurrentTimeNormalized < 0.2f)
            NewTimeOfDay = EEnvArt_TimeOfDay::Dawn;
        else if (CurrentTimeNormalized < 0.4f)
            NewTimeOfDay = EEnvArt_TimeOfDay::Morning;
        else if (CurrentTimeNormalized < 0.6f)
            NewTimeOfDay = EEnvArt_TimeOfDay::Midday;
        else if (CurrentTimeNormalized < 0.8f)
            NewTimeOfDay = EEnvArt_TimeOfDay::Afternoon;
        else if (CurrentTimeNormalized < 0.9f)
            NewTimeOfDay = EEnvArt_TimeOfDay::Dusk;
        else
            NewTimeOfDay = EEnvArt_TimeOfDay::Night;
        
        if (NewTimeOfDay != CurrentTimeOfDay)
        {
            SetTimeOfDay(NewTimeOfDay, false);
        }
    }
    
    UpdateLighting(DeltaTime);
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay, bool bInstant)
{
    if (NewTimeOfDay == CurrentTimeOfDay)
        return;
    
    CurrentTimeOfDay = NewTimeOfDay;
    
    if (LightingPresets.Contains(CurrentTimeOfDay))
    {
        ApplyLightingSettings(LightingPresets[CurrentTimeOfDay], bInstant);
    }
    
    OnTimeOfDayChanged(CurrentTimeOfDay);
}

void AEnvArt_AtmosphereManager::SetTimeNormalized(float TimeValue, bool bInstant)
{
    CurrentTimeNormalized = FMath::Clamp(TimeValue, 0.0f, 1.0f);
    
    // Convert to time of day
    EEnvArt_TimeOfDay NewTimeOfDay = EEnvArt_TimeOfDay::Midday;
    if (TimeValue < 0.1f)
        NewTimeOfDay = EEnvArt_TimeOfDay::Night;
    else if (TimeValue < 0.2f)
        NewTimeOfDay = EEnvArt_TimeOfDay::Dawn;
    else if (TimeValue < 0.4f)
        NewTimeOfDay = EEnvArt_TimeOfDay::Morning;
    else if (TimeValue < 0.6f)
        NewTimeOfDay = EEnvArt_TimeOfDay::Midday;
    else if (TimeValue < 0.8f)
        NewTimeOfDay = EEnvArt_TimeOfDay::Afternoon;
    else if (TimeValue < 0.9f)
        NewTimeOfDay = EEnvArt_TimeOfDay::Dusk;
    else
        NewTimeOfDay = EEnvArt_TimeOfDay::Night;
    
    SetTimeOfDay(NewTimeOfDay, bInstant);
}

void AEnvArt_AtmosphereManager::ApplyLightingSettings(const FEnvArt_LightingSettings& Settings, bool bInstant)
{
    if (bInstant || !bIsTransitioning)
    {
        if (bInstant)
        {
            // Apply settings immediately
            if (SunActor && SunActor->GetLightComponent())
            {
                SunActor->GetLightComponent()->SetLightColor(Settings.SunColor);
                SunActor->GetLightComponent()->SetIntensity(Settings.SunIntensity);
                SunActor->SetActorRotation(Settings.SunRotation);
            }
            
            if (SkyLightActor && SkyLightActor->GetLightComponent())
            {
                SkyLightActor->GetLightComponent()->SetLightColor(Settings.SkyLightColor);
                SkyLightActor->GetLightComponent()->SetIntensity(Settings.SkyLightIntensity);
            }
        }
        else
        {
            // Start transition
            StartSettings = GetCurrentLightingSettings();
            TargetSettings = Settings;
            bIsTransitioning = true;
            TransitionTimer = 0.0f;
        }
    }
}

void AEnvArt_AtmosphereManager::SetWeatherIntensity(float Intensity)
{
    // Adjust fog and particle effects based on weather intensity
    if (FogActor)
    {
        // Increase fog density with weather intensity
        float FogDensity = FMath::Lerp(0.01f, 0.1f, Intensity);
        // Apply fog settings via component if available
    }
    
    OnWeatherChanged(Intensity);
}

void AEnvArt_AtmosphereManager::CreateAtmosphericParticles(const TArray<FVector>& Locations)
{
    for (int32 i = 0; i < Locations.Num(); i++)
    {
        // Spawn particle emitters at specified locations
        AActor* ParticleActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Locations[i], FRotator::ZeroRotator);
        if (ParticleActor)
        {
            ParticleActor->SetActorLabel(FString::Printf(TEXT("AtmosphericParticles_%d"), i + 1));
        }
    }
}

void AEnvArt_AtmosphereManager::InitializeLightingPresets()
{
    // Dawn
    FEnvArt_LightingSettings DawnSettings;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnSettings.SunIntensity = 4.0f;
    DawnSettings.SunRotation = FRotator(-5.0f, 80.0f, 0.0f);
    DawnSettings.SkyLightColor = FLinearColor(0.6f, 0.7f, 1.0f, 1.0f);
    DawnSettings.SkyLightIntensity = 1.0f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Dawn, DawnSettings);
    
    // Morning
    FEnvArt_LightingSettings MorningSettings;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningSettings.SunIntensity = 6.0f;
    MorningSettings.SunRotation = FRotator(-20.0f, 60.0f, 0.0f);
    MorningSettings.SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    MorningSettings.SkyLightIntensity = 1.5f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Morning, MorningSettings);
    
    // Midday
    FEnvArt_LightingSettings MiddaySettings;
    MiddaySettings.SunColor = FLinearColor(1.0f, 1.0f, 0.9f, 1.0f);
    MiddaySettings.SunIntensity = 10.0f;
    MiddaySettings.SunRotation = FRotator(-60.0f, 0.0f, 0.0f);
    MiddaySettings.SkyLightColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    MiddaySettings.SkyLightIntensity = 2.0f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Midday, MiddaySettings);
    
    // Afternoon
    FEnvArt_LightingSettings AfternoonSettings;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    AfternoonSettings.SunIntensity = 8.0f;
    AfternoonSettings.SunRotation = FRotator(-30.0f, -45.0f, 0.0f);
    AfternoonSettings.SkyLightColor = FLinearColor(0.5f, 0.6f, 0.9f, 1.0f);
    AfternoonSettings.SkyLightIntensity = 1.8f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Afternoon, AfternoonSettings);
    
    // Dusk
    FEnvArt_LightingSettings DuskSettings;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f);
    DuskSettings.SunIntensity = 3.0f;
    DuskSettings.SunRotation = FRotator(-2.0f, -80.0f, 0.0f);
    DuskSettings.SkyLightColor = FLinearColor(0.7f, 0.5f, 0.8f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.8f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Dusk, DuskSettings);
    
    // Night
    FEnvArt_LightingSettings NightSettings;
    NightSettings.SunColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunRotation = FRotator(30.0f, 0.0f, 0.0f);
    NightSettings.SkyLightColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    NightSettings.SkyLightIntensity = 0.3f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Night, NightSettings);
}

void AEnvArt_AtmosphereManager::FindSceneLightingActors()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        SunActor = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
    }
}

void AEnvArt_AtmosphereManager::UpdateLighting(float DeltaTime)
{
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        float Alpha = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
        
        InterpolateLightingSettings(StartSettings, TargetSettings, Alpha);
        
        if (Alpha >= 1.0f)
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }
}

void AEnvArt_AtmosphereManager::InterpolateLightingSettings(const FEnvArt_LightingSettings& From, const FEnvArt_LightingSettings& To, float Alpha)
{
    if (SunActor && SunActor->GetLightComponent())
    {
        FLinearColor InterpolatedColor = FMath::Lerp(From.SunColor, To.SunColor, Alpha);
        float InterpolatedIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
        FRotator InterpolatedRotation = FMath::Lerp(From.SunRotation, To.SunRotation, Alpha);
        
        SunActor->GetLightComponent()->SetLightColor(InterpolatedColor);
        SunActor->GetLightComponent()->SetIntensity(InterpolatedIntensity);
        SunActor->SetActorRotation(InterpolatedRotation);
    }
    
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        FLinearColor InterpolatedSkyColor = FMath::Lerp(From.SkyLightColor, To.SkyLightColor, Alpha);
        float InterpolatedSkyIntensity = FMath::Lerp(From.SkyLightIntensity, To.SkyLightIntensity, Alpha);
        
        SkyLightActor->GetLightComponent()->SetLightColor(InterpolatedSkyColor);
        SkyLightActor->GetLightComponent()->SetIntensity(InterpolatedSkyIntensity);
    }
}

FEnvArt_LightingSettings AEnvArt_AtmosphereManager::GetCurrentLightingSettings() const
{
    FEnvArt_LightingSettings CurrentSettings;
    
    if (SunActor && SunActor->GetLightComponent())
    {
        CurrentSettings.SunColor = SunActor->GetLightComponent()->GetLightColor();
        CurrentSettings.SunIntensity = SunActor->GetLightComponent()->Intensity;
        CurrentSettings.SunRotation = SunActor->GetActorRotation();
    }
    
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        CurrentSettings.SkyLightColor = SkyLightActor->GetLightComponent()->GetLightColor();
        CurrentSettings.SkyLightIntensity = SkyLightActor->GetLightComponent()->Intensity;
    }
    
    return CurrentSettings;
}