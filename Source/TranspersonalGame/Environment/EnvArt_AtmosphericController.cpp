#include "EnvArt_AtmosphericController.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

AEnvArt_AtmosphericController::AEnvArt_AtmosphericController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize atmospheric settings
    TimeOfDay = 0.4f; // Golden hour
    FogDensity = 0.02f;
    SunIntensity = 3.0f;
    SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    
    bAutoUpdateAtmosphere = true;
    AtmosphereUpdateInterval = 1.0f;
    LastUpdateTime = 0.0f;
}

void AEnvArt_AtmosphericController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find existing directional light
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundLights);
    
    if (FoundLights.Num() > 0)
    {
        DirectionalLightActor = Cast<ADirectionalLight>(FoundLights[0]);
        if (DirectionalLightActor)
        {
            DirectionalLightComponent = DirectionalLightActor->GetLightComponent();
        }
    }
    
    // Find existing fog
    TArray<AActor*> FoundFog;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundFog);
    
    if (FoundFog.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FoundFog[0]);
        if (FogActor)
        {
            FogComponent = FogActor->GetComponent();
        }
    }
    
    // Apply initial atmospheric settings
    UpdateAtmosphere();
}

void AEnvArt_AtmosphericController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoUpdateAtmosphere)
    {
        LastUpdateTime += DeltaTime;
        if (LastUpdateTime >= AtmosphereUpdateInterval)
        {
            UpdateAtmosphere();
            LastUpdateTime = 0.0f;
        }
    }
}

void AEnvArt_AtmosphericController::UpdateAtmosphere()
{
    UpdateSunLighting();
    UpdateFogSettings();
    UpdateBiomeSpecificAtmosphere();
}

void AEnvArt_AtmosphericController::UpdateSunLighting()
{
    if (!DirectionalLightComponent)
        return;
    
    // Calculate sun angle based on time of day
    float SunElevation = FMath::Lerp(-90.0f, 90.0f, TimeOfDay);
    float SunAzimuth = FMath::Lerp(0.0f, 360.0f, TimeOfDay);
    
    // Golden hour adjustment
    if (TimeOfDay >= 0.3f && TimeOfDay <= 0.5f)
    {
        SunElevation = FMath::Clamp(SunElevation, -30.0f, 30.0f);
    }
    
    FRotator SunRotation(SunElevation, SunAzimuth, 0.0f);
    DirectionalLightActor->SetActorRotation(SunRotation);
    
    // Update light properties
    DirectionalLightComponent->SetLightColor(SunColor);
    DirectionalLightComponent->SetIntensity(SunIntensity);
    
    // Adjust color temperature based on time
    float ColorTemp = FMath::Lerp(2000.0f, 6500.0f, TimeOfDay);
    DirectionalLightComponent->SetTemperature(ColorTemp);
}

void AEnvArt_AtmosphericController::UpdateFogSettings()
{
    if (!FogComponent)
        return;
    
    FogComponent->SetFogDensity(FogDensity);
    FogComponent->SetFogInscatteringColor(FogColor);
    FogComponent->SetFogHeightFalloff(0.2f);
    FogComponent->SetFogMaxOpacity(0.8f);
}

void AEnvArt_AtmosphericController::UpdateBiomeSpecificAtmosphere()
{
    // Get player location to determine current biome
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    EEnvArt_BiomeType CurrentBiome = GetBiomeAtLocation(PlayerLocation);
    
    // Apply biome-specific atmospheric effects
    switch (CurrentBiome)
    {
        case EEnvArt_BiomeType::Forest:
            ApplyForestAtmosphere();
            break;
        case EEnvArt_BiomeType::Swamp:
            ApplySwampAtmosphere();
            break;
        case EEnvArt_BiomeType::Desert:
            ApplyDesertAtmosphere();
            break;
        case EEnvArt_BiomeType::Mountain:
            ApplyMountainAtmosphere();
            break;
        case EEnvArt_BiomeType::Savanna:
        default:
            ApplySavannaAtmosphere();
            break;
    }
}

EEnvArt_BiomeType AEnvArt_AtmosphericController::GetBiomeAtLocation(const FVector& Location)
{
    // Simple biome detection based on coordinates
    if (Location.X < -20000 && Location.Y > 20000)
        return EEnvArt_BiomeType::Forest;
    else if (Location.X < -20000 && Location.Y < -20000)
        return EEnvArt_BiomeType::Swamp;
    else if (Location.X > 30000 && FMath::Abs(Location.Y) < 20000)
        return EEnvArt_BiomeType::Desert;
    else if (Location.X > 20000 && Location.Y > 30000)
        return EEnvArt_BiomeType::Mountain;
    else
        return EEnvArt_BiomeType::Savanna;
}

void AEnvArt_AtmosphericController::ApplyForestAtmosphere()
{
    FogDensity = 0.03f;
    FogColor = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f); // Green tint
    SunIntensity = 2.5f; // Filtered through canopy
}

void AEnvArt_AtmosphericController::ApplySwampAtmosphere()
{
    FogDensity = 0.05f;
    FogColor = FLinearColor(0.5f, 0.6f, 0.5f, 1.0f); // Murky green
    SunIntensity = 2.0f; // Dim and moody
}

void AEnvArt_AtmosphericController::ApplyDesertAtmosphere()
{
    FogDensity = 0.01f;
    FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f); // Sandy haze
    SunIntensity = 4.0f; // Harsh sun
}

void AEnvArt_AtmosphericController::ApplyMountainAtmosphere()
{
    FogDensity = 0.025f;
    FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f); // Cool blue
    SunIntensity = 3.5f; // Clear mountain air
}

void AEnvArt_AtmosphericController::ApplySavannaAtmosphere()
{
    FogDensity = 0.015f;
    FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f); // Warm dust
    SunIntensity = 3.5f; // Bright savanna sun
}

void AEnvArt_AtmosphericController::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);
    UpdateAtmosphere();
}

void AEnvArt_AtmosphericController::SetFogDensity(float NewFogDensity)
{
    FogDensity = FMath::Clamp(NewFogDensity, 0.0f, 1.0f);
    UpdateFogSettings();
}

void AEnvArt_AtmosphericController::SetSunIntensity(float NewSunIntensity)
{
    SunIntensity = FMath::Clamp(NewSunIntensity, 0.0f, 10.0f);
    UpdateSunLighting();
}