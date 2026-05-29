#include "CretaceousAtmosphere.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

ACretaceousAtmosphere::ACretaceousAtmosphere()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default Cretaceous atmospheric settings
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.7f, 1.0f);  // Warm orange
    AtmosphereSettings.SunIntensity = 5.0f;
    AtmosphereSettings.SunElevation = -45.0f;
    AtmosphereSettings.SunAzimuth = 135.0f;
    AtmosphereSettings.RayleighScattering = 0.8f;
    AtmosphereSettings.MieScattering = 0.6f;
    AtmosphereSettings.AtmosphereHeight = 80.0f;
    AtmosphereSettings.FogDensity = 0.02f;
    AtmosphereSettings.FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    SunLight = nullptr;
    SkyAtmosphereActor = nullptr;
    HeightFogActor = nullptr;
    bAtmosphereConfigured = false;
}

void ACretaceousAtmosphere::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply Cretaceous atmosphere settings on start
    ApplyCretaceousAtmosphere();
}

void ACretaceousAtmosphere::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Ensure atmosphere stays configured
    if (!bAtmosphereConfigured)
    {
        ApplyCretaceousAtmosphere();
    }
}

void ACretaceousAtmosphere::ApplyCretaceousAtmosphere()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    ConfigureDirectionalLight();
    ConfigureSkyAtmosphere();
    ConfigureHeightFog();

    bAtmosphereConfigured = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Cretaceous Atmosphere applied successfully"));
}

void ACretaceousAtmosphere::ConfigureDirectionalLight()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find existing directional light or create new one
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    if (!SunLight)
    {
        // Spawn new directional light
        FVector SpawnLocation = FVector(0.0f, 0.0f, 1000.0f);
        FRotator SpawnRotation = FRotator(AtmosphereSettings.SunElevation, AtmosphereSettings.SunAzimuth, 0.0f);
        
        SunLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), SpawnLocation, SpawnRotation);
        if (SunLight)
        {
            SunLight->SetActorLabel(TEXT("Cretaceous_Sun"));
        }
    }

    if (SunLight)
    {
        // Configure sun for Cretaceous period
        FRotator SunRotation = FRotator(AtmosphereSettings.SunElevation, AtmosphereSettings.SunAzimuth, 0.0f);
        SunLight->SetActorRotation(SunRotation);

        UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
        if (LightComponent)
        {
            LightComponent->SetLightColor(AtmosphereSettings.SunColor);
            LightComponent->SetIntensity(AtmosphereSettings.SunIntensity);
            LightComponent->SetCastShadows(true);
            LightComponent->SetCastVolumetricShadow(true);
        }
    }
}

void ACretaceousAtmosphere::ConfigureSkyAtmosphere()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find existing sky atmosphere
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        SkyAtmosphereActor = Cast<ASkyAtmosphere>(FoundActors[0]);
    }

    if (!SkyAtmosphereActor)
    {
        // Spawn new sky atmosphere
        SkyAtmosphereActor = World->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass());
        if (SkyAtmosphereActor)
        {
            SkyAtmosphereActor->SetActorLabel(TEXT("Cretaceous_Atmosphere"));
        }
    }

    if (SkyAtmosphereActor)
    {
        USkyAtmosphereComponent* AtmosphereComponent = SkyAtmosphereActor->GetAtmosphereComponent();
        if (AtmosphereComponent)
        {
            // Configure for Cretaceous atmospheric composition
            AtmosphereComponent->RayleighScatteringScale = AtmosphereSettings.RayleighScattering;
            AtmosphereComponent->MieScatteringScale = AtmosphereSettings.MieScattering;
            AtmosphereComponent->AtmosphereHeight = AtmosphereSettings.AtmosphereHeight;
        }
    }
}

void ACretaceousAtmosphere::ConfigureHeightFog()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find existing height fog
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        HeightFogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    if (!HeightFogActor)
    {
        // Spawn new height fog
        HeightFogActor = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass());
        if (HeightFogActor)
        {
            HeightFogActor->SetActorLabel(TEXT("Cretaceous_Fog"));
        }
    }

    if (HeightFogActor)
    {
        UExponentialHeightFogComponent* FogComponent = HeightFogActor->GetComponent();
        if (FogComponent)
        {
            // Configure for humid Cretaceous climate
            FogComponent->FogDensity = AtmosphereSettings.FogDensity;
            FogComponent->FogHeightFalloff = 0.2f;
            FogComponent->FogInscatteringColor = AtmosphereSettings.FogColor;
        }
    }
}

void ACretaceousAtmosphere::SetTimeOfDay(float TimeHours)
{
    // Clamp time to 24-hour format
    TimeHours = FMath::Fmod(TimeHours, 24.0f);
    if (TimeHours < 0.0f)
    {
        TimeHours += 24.0f;
    }

    // Calculate sun position based on time
    float SunAngle = (TimeHours - 12.0f) * 15.0f; // 15 degrees per hour
    AtmosphereSettings.SunAzimuth = SunAngle;

    // Calculate sun elevation (higher at noon, lower at dawn/dusk)
    float NoonOffset = FMath::Abs(TimeHours - 12.0f);
    AtmosphereSettings.SunElevation = -45.0f + (45.0f * FMath::Cos(NoonOffset * PI / 12.0f));

    // Update sun color based on time of day
    AtmosphereSettings.SunColor = GetCretaceousSunColor(TimeHours);

    // Apply changes
    ConfigureDirectionalLight();
}

FLinearColor ACretaceousAtmosphere::GetCretaceousSunColor(float TimeHours) const
{
    // Cretaceous sun colors throughout the day
    if (TimeHours >= 6.0f && TimeHours <= 18.0f) // Daytime
    {
        // Warm orange during the day due to higher CO2
        float DayProgress = (TimeHours - 6.0f) / 12.0f; // 0 to 1
        float Warmth = 0.7f + (0.3f * FMath::Sin(DayProgress * PI)); // Peak warmth at noon
        return FLinearColor(1.0f, 0.85f, Warmth, 1.0f);
    }
    else // Night time
    {
        // Cooler colors at night
        return FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    }
}