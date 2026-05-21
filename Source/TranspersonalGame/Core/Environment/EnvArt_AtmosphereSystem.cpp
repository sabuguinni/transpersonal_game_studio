#include "EnvArt_AtmosphereSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UEnvArt_AtmosphereSystem::UEnvArt_AtmosphereSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    SunLight = nullptr;
    HeightFog = nullptr;
    SkyAtmosphere = nullptr;
    CurrentDayTime = 0.0f;
    
    InitializeTimeOfDaySettings();
}

void UEnvArt_AtmosphereSystem::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingComponents();
    SetGoldenHourLighting();
}

void UEnvArt_AtmosphereSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableDynamicLighting && DayDuration > 0.0f)
    {
        CurrentDayTime += DeltaTime;
        if (CurrentDayTime >= DayDuration)
        {
            CurrentDayTime = 0.0f;
        }
        
        // Calculate time of day based on current day time
        float TimeRatio = CurrentDayTime / DayDuration;
        int32 TimeIndex = FMath::FloorToInt(TimeRatio * 6.0f); // 6 time periods
        EEnvArt_TimeOfDay NewTimeOfDay = static_cast<EEnvArt_TimeOfDay>(FMath::Clamp(TimeIndex, 0, 5));
        
        if (NewTimeOfDay != CurrentTimeOfDay)
        {
            CurrentTimeOfDay = NewTimeOfDay;
            UpdateLighting();
        }
    }
}

void UEnvArt_AtmosphereSystem::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    UpdateLighting();
}

void UEnvArt_AtmosphereSystem::UpdateLighting()
{
    if (TimeOfDaySettings.Contains(CurrentTimeOfDay))
    {
        const FEnvArt_LightingSettings& Settings = TimeOfDaySettings[CurrentTimeOfDay];
        ApplyLightingSettings(Settings);
    }
}

void UEnvArt_AtmosphereSystem::SetGoldenHourLighting()
{
    FEnvArt_LightingSettings GoldenHour;
    GoldenHour.SunElevation = -30.0f;
    GoldenHour.SunAzimuth = 45.0f;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    GoldenHour.SunIntensity = 2.5f;
    GoldenHour.FogColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    GoldenHour.FogDensity = 0.015f;
    
    ApplyLightingSettings(GoldenHour);
}

void UEnvArt_AtmosphereSystem::AddVolumetricFog(const FVector& Location, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create a fog volume actor at the specified location
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AActor* FogVolume = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (FogVolume)
    {
        UExponentialHeightFogComponent* FogComponent = NewObject<UExponentialHeightFogComponent>(FogVolume);
        if (FogComponent)
        {
            FogComponent->SetFogDensity(0.05f);
            FogComponent->SetFogHeightFalloff(0.2f);
            FogComponent->SetFogMaxOpacity(0.6f);
            FogComponent->SetInscatteringColor(FLinearColor(0.7f, 0.8f, 1.0f, 1.0f));
            FogComponent->AttachToComponent(FogVolume->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
            FogComponent->RegisterComponent();
            
            FogVolume->SetActorLabel(FString::Printf(TEXT("VolumetricFog_%d"), FMath::RandRange(1000, 9999)));
        }
    }
}

void UEnvArt_AtmosphereSystem::FindLightingComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find directional light (sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    // Find height fog component in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            HeightFog = Actor->FindComponentByClass<UExponentialHeightFogComponent>();
            if (HeightFog)
            {
                break;
            }
        }
    }
    
    // Find sky atmosphere component
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            SkyAtmosphere = Actor->FindComponentByClass<USkyAtmosphereComponent>();
            if (SkyAtmosphere)
            {
                break;
            }
        }
    }
}

void UEnvArt_AtmosphereSystem::ApplyLightingSettings(const FEnvArt_LightingSettings& Settings)
{
    // Apply sun settings
    if (SunLight && SunLight->GetLightComponent())
    {
        FRotator SunRotation = FRotator(Settings.SunElevation, Settings.SunAzimuth, 0.0f);
        SunLight->SetActorRotation(SunRotation);
        
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetLightColor(Settings.SunColor);
        LightComp->SetIntensity(Settings.SunIntensity);
    }
    
    // Apply fog settings
    if (HeightFog)
    {
        HeightFog->SetFogDensity(Settings.FogDensity);
        HeightFog->SetInscatteringColor(Settings.FogColor);
    }
}

void UEnvArt_AtmosphereSystem::InitializeTimeOfDaySettings()
{
    // Dawn
    FEnvArt_LightingSettings Dawn;
    Dawn.SunElevation = -15.0f;
    Dawn.SunAzimuth = 90.0f;
    Dawn.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    Dawn.SunIntensity = 1.5f;
    Dawn.FogColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    Dawn.FogDensity = 0.025f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Dawn, Dawn);
    
    // Morning
    FEnvArt_LightingSettings Morning;
    Morning.SunElevation = 15.0f;
    Morning.SunAzimuth = 120.0f;
    Morning.SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    Morning.SunIntensity = 3.0f;
    Morning.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    Morning.FogDensity = 0.015f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Morning, Morning);
    
    // Noon
    FEnvArt_LightingSettings Noon;
    Noon.SunElevation = 75.0f;
    Noon.SunAzimuth = 180.0f;
    Noon.SunColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    Noon.SunIntensity = 5.0f;
    Noon.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    Noon.FogDensity = 0.01f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Noon, Noon);
    
    // Afternoon
    FEnvArt_LightingSettings Afternoon;
    Afternoon.SunElevation = 45.0f;
    Afternoon.SunAzimuth = 240.0f;
    Afternoon.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    Afternoon.SunIntensity = 3.5f;
    Afternoon.FogColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
    Afternoon.FogDensity = 0.012f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Afternoon, Afternoon);
    
    // Dusk
    FEnvArt_LightingSettings Dusk;
    Dusk.SunElevation = -10.0f;
    Dusk.SunAzimuth = 270.0f;
    Dusk.SunColor = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f);
    Dusk.SunIntensity = 2.0f;
    Dusk.FogColor = FLinearColor(0.9f, 0.5f, 0.3f, 1.0f);
    Dusk.FogDensity = 0.02f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Dusk, Dusk);
    
    // Night
    FEnvArt_LightingSettings Night;
    Night.SunElevation = -45.0f;
    Night.SunAzimuth = 0.0f;
    Night.SunColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
    Night.SunIntensity = 0.1f;
    Night.FogColor = FLinearColor(0.2f, 0.2f, 0.4f, 1.0f);
    Night.FogDensity = 0.03f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Night, Night);
}