#include "EnvArt_AtmosphereManager.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    CurrentTimeOfDay = EEnvArt_TimeOfDay::Noon;
    DayDurationMinutes = 20.0f;
    bAutoAdvanceTime = true;
    TimeAccumulator = 0.0f;

    InitializeTimeSettings();
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    SetTimeOfDay(CurrentTimeOfDay);
}

void AEnvArt_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoAdvanceTime)
    {
        UpdateTimeOfDay(DeltaTime);
    }
}

void AEnvArt_AtmosphereManager::InitializeTimeSettings()
{
    // Dawn
    FEnvArt_AtmosphereSettings DawnSettings;
    DawnSettings.SunIntensity = 1.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.SunAngle = 10.0f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(0.8f, 0.7f, 0.9f, 1.0f);
    TimeSettings.Add(EEnvArt_TimeOfDay::Dawn, DawnSettings);

    // Morning
    FEnvArt_AtmosphereSettings MorningSettings;
    MorningSettings.SunIntensity = 2.5f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    MorningSettings.SunAngle = 30.0f;
    MorningSettings.FogDensity = 0.03f;
    MorningSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    TimeSettings.Add(EEnvArt_TimeOfDay::Morning, MorningSettings);

    // Noon
    FEnvArt_AtmosphereSettings NoonSettings;
    NoonSettings.SunIntensity = 4.0f;
    NoonSettings.SunColor = FLinearColor::White;
    NoonSettings.SunAngle = 80.0f;
    NoonSettings.FogDensity = 0.01f;
    NoonSettings.FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    TimeSettings.Add(EEnvArt_TimeOfDay::Noon, NoonSettings);

    // Afternoon
    FEnvArt_AtmosphereSettings AfternoonSettings;
    AfternoonSettings.SunIntensity = 3.0f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AfternoonSettings.SunAngle = 50.0f;
    AfternoonSettings.FogDensity = 0.02f;
    AfternoonSettings.FogColor = FLinearColor(0.7f, 0.6f, 0.5f, 1.0f);
    TimeSettings.Add(EEnvArt_TimeOfDay::Afternoon, AfternoonSettings);

    // Dusk (Golden Hour)
    FEnvArt_AtmosphereSettings DuskSettings;
    DuskSettings.SunIntensity = 2.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskSettings.SunAngle = 15.0f;
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(0.9f, 0.5f, 0.3f, 1.0f);
    TimeSettings.Add(EEnvArt_TimeOfDay::Dusk, DuskSettings);

    // Night
    FEnvArt_AtmosphereSettings NightSettings;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SunAngle = -20.0f;
    NightSettings.FogDensity = 0.06f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    TimeSettings.Add(EEnvArt_TimeOfDay::Night, NightSettings);
}

void AEnvArt_AtmosphereManager::UpdateTimeOfDay(float DeltaTime)
{
    TimeAccumulator += DeltaTime;
    
    float TimePerPhase = (DayDurationMinutes * 60.0f) / 6.0f; // 6 time phases
    
    if (TimeAccumulator >= TimePerPhase)
    {
        TimeAccumulator = 0.0f;
        
        int32 CurrentIndex = static_cast<int32>(CurrentTimeOfDay);
        CurrentIndex = (CurrentIndex + 1) % 6;
        CurrentTimeOfDay = static_cast<EEnvArt_TimeOfDay>(CurrentIndex);
        
        SetTimeOfDay(CurrentTimeOfDay);
    }
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(EEnvArt_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;
    
    if (TimeSettings.Contains(NewTime))
    {
        ApplyAtmosphereSettings(TimeSettings[NewTime]);
    }
}

void AEnvArt_AtmosphereManager::ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* DirLight = SunLight->GetLightComponent();
        DirLight->SetIntensity(Settings.SunIntensity);
        DirLight->SetLightColor(Settings.SunColor);
        
        FRotator SunRotation = FRotator(-Settings.SunAngle, 45.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
    }

    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        USkyLightComponent* SkyLight = SkyLightActor->GetLightComponent();
        SkyLight->SetIntensity(Settings.SunIntensity * 0.3f);
        SkyLight->RecaptureSky();
    }

    if (FogActor && FogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
        FogComp->SetFogHeightFalloff(0.2f);
        FogComp->SetFogMaxOpacity(0.8f);
    }
}

void AEnvArt_AtmosphereManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find SkyLight
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
    }

    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}

void AEnvArt_AtmosphereManager::SetGoldenHourLighting()
{
    FEnvArt_AtmosphereSettings GoldenHour;
    GoldenHour.SunIntensity = 2.5f;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    GoldenHour.SunAngle = 20.0f;
    GoldenHour.FogDensity = 0.03f;
    GoldenHour.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    
    ApplyAtmosphereSettings(GoldenHour);
    CurrentTimeOfDay = EEnvArt_TimeOfDay::Dusk;
}

void AEnvArt_AtmosphereManager::SetForestFogEffect()
{
    if (FogActor && FogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        FogComp->SetFogDensity(0.08f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.6f, 0.8f, 0.7f, 1.0f));
        FogComp->SetFogHeightFalloff(0.5f);
        FogComp->SetStartDistance(500.0f);
        FogComp->SetFogMaxOpacity(0.6f);
    }
}