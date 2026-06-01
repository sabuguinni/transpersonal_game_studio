#include "EnvArt_AtmosphereManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentBiome = EBiomeType::Savanna;
    CurrentTimeOfDay = 12.0f; // Noon
    MainSun = nullptr;
    
    InitializeDefaultSettings();
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the main directional light in the scene
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        MainSun = Cast<ADirectionalLight>(FoundActors[0]);
        if (MainSun)
        {
            UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Found main directional light"));
        }
    }
    
    // Create fog actors for each biome if they don't exist
    CreateBiomeFogActors();
    
    // Apply default atmosphere
    SetBiomeAtmosphere(CurrentBiome);
    SetTimeOfDay(CurrentTimeOfDay);
}

void AEnvArt_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update atmosphere based on player location if needed
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        UpdateAtmosphereForLocation(PlayerLocation);
    }
}

void AEnvArt_AtmosphereManager::InitializeDefaultSettings()
{
    // Savanna atmosphere - clear, warm
    FEnvArt_BiomeAtmosphere SavannaAtmo;
    SavannaAtmo.FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
    SavannaAtmo.FogDensity = 0.01f;
    SavannaAtmo.FogHeightFalloff = 0.1f;
    SavannaAtmo.VolumetricFogScatteringDistribution = 0.3f;
    SavannaAtmo.VolumetricFogAlbedo = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    BiomeAtmosphereSettings.Add(EBiomeType::Savanna, SavannaAtmo);
    
    // Swamp atmosphere - thick, humid
    FEnvArt_BiomeAtmosphere SwampAtmo;
    SwampAtmo.FogColor = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f);
    SwampAtmo.FogDensity = 0.05f;
    SwampAtmo.FogHeightFalloff = 0.3f;
    SwampAtmo.VolumetricFogScatteringDistribution = 0.5f;
    SwampAtmo.VolumetricFogAlbedo = FLinearColor(0.8f, 1.0f, 0.9f, 1.0f);
    BiomeAtmosphereSettings.Add(EBiomeType::Swamp, SwampAtmo);
    
    // Forest atmosphere - dappled, mysterious
    FEnvArt_BiomeAtmosphere ForestAtmo;
    ForestAtmo.FogColor = FLinearColor(0.5f, 0.7f, 0.5f, 1.0f);
    ForestAtmo.FogDensity = 0.03f;
    ForestAtmo.FogHeightFalloff = 0.2f;
    ForestAtmo.VolumetricFogScatteringDistribution = 0.4f;
    ForestAtmo.VolumetricFogAlbedo = FLinearColor(0.7f, 1.0f, 0.7f, 1.0f);
    BiomeAtmosphereSettings.Add(EBiomeType::Forest, ForestAtmo);
    
    // Desert atmosphere - harsh, clear
    FEnvArt_BiomeAtmosphere DesertAtmo;
    DesertAtmo.FogColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    DesertAtmo.FogDensity = 0.005f;
    DesertAtmo.FogHeightFalloff = 0.05f;
    DesertAtmo.VolumetricFogScatteringDistribution = 0.2f;
    DesertAtmo.VolumetricFogAlbedo = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    BiomeAtmosphereSettings.Add(EBiomeType::Desert, DesertAtmo);
    
    // Mountain atmosphere - thin, crisp
    FEnvArt_BiomeAtmosphere MountainAtmo;
    MountainAtmo.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    MountainAtmo.FogDensity = 0.02f;
    MountainAtmo.FogHeightFalloff = 0.15f;
    MountainAtmo.VolumetricFogScatteringDistribution = 0.1f;
    MountainAtmo.VolumetricFogAlbedo = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
    BiomeAtmosphereSettings.Add(EBiomeType::Mountain, MountainAtmo);
    
    // Time of day presets
    FEnvArt_TimeOfDaySettings GoldenHour;
    GoldenHour.SunAngleElevation = -15.0f;
    GoldenHour.SunAngleAzimuth = 45.0f;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    GoldenHour.SunIntensity = 4.0f;
    TimeOfDayPresets.Add(TEXT("GoldenHour"), GoldenHour);
    
    FEnvArt_TimeOfDaySettings Noon;
    Noon.SunAngleElevation = 60.0f;
    Noon.SunAngleAzimuth = 180.0f;
    Noon.SunColor = FLinearColor(1.0f, 1.0f, 0.9f, 1.0f);
    Noon.SunIntensity = 5.0f;
    TimeOfDayPresets.Add(TEXT("Noon"), Noon);
    
    FEnvArt_TimeOfDaySettings Dusk;
    Dusk.SunAngleElevation = -30.0f;
    Dusk.SunAngleAzimuth = 270.0f;
    Dusk.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    Dusk.SunIntensity = 2.0f;
    TimeOfDayPresets.Add(TEXT("Dusk"), Dusk);
}

void AEnvArt_AtmosphereManager::SetBiomeAtmosphere(EBiomeType BiomeType)
{
    CurrentBiome = BiomeType;
    
    if (BiomeAtmosphereSettings.Contains(BiomeType))
    {
        const FEnvArt_BiomeAtmosphere& Settings = BiomeAtmosphereSettings[BiomeType];
        
        // Update fog actor for this biome
        if (BiomeFogActors.Contains(BiomeType))
        {
            AExponentialHeightFog* FogActor = BiomeFogActors[BiomeType];
            if (FogActor)
            {
                UpdateFogSettings(FogActor, Settings);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Set atmosphere for biome %d"), (int32)BiomeType);
    }
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(float TimeHour)
{
    CurrentTimeOfDay = FMath::Clamp(TimeHour, 0.0f, 24.0f);
    
    // Apply golden hour preset for atmospheric effect
    if (TimeOfDayPresets.Contains(TEXT("GoldenHour")))
    {
        UpdateSunSettings(TimeOfDayPresets[TEXT("GoldenHour")]);
    }
}

void AEnvArt_AtmosphereManager::ApplyTimeOfDayPreset(const FString& PresetName)
{
    if (TimeOfDayPresets.Contains(PresetName))
    {
        UpdateSunSettings(TimeOfDayPresets[PresetName]);
        UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Applied time preset %s"), *PresetName);
    }
}

void AEnvArt_AtmosphereManager::CreateBiomeFogActors()
{
    // Biome center coordinates
    TMap<EBiomeType, FVector> BiomeCenters;
    BiomeCenters.Add(EBiomeType::Savanna, FVector(0, 0, 500));
    BiomeCenters.Add(EBiomeType::Swamp, FVector(-50000, -45000, 500));
    BiomeCenters.Add(EBiomeType::Forest, FVector(-45000, 40000, 500));
    BiomeCenters.Add(EBiomeType::Desert, FVector(55000, 0, 500));
    BiomeCenters.Add(EBiomeType::Mountain, FVector(40000, 50000, 500));
    
    for (auto& BiomePair : BiomeCenters)
    {
        EBiomeType BiomeType = BiomePair.Key;
        FVector Location = BiomePair.Value;
        
        if (!BiomeFogActors.Contains(BiomeType))
        {
            AExponentialHeightFog* FogActor = GetWorld()->SpawnActor<AExponentialHeightFog>(Location, FRotator::ZeroRotator);
            if (FogActor)
            {
                FString BiomeName = UEnum::GetValueAsString(BiomeType);
                FogActor->SetActorLabel(FString::Printf(TEXT("AtmosphericFog_%s"), *BiomeName));
                BiomeFogActors.Add(BiomeType, FogActor);
                
                UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Created fog actor for %s"), *BiomeName);
            }
        }
    }
}

void AEnvArt_AtmosphereManager::UpdateAtmosphereForLocation(const FVector& Location)
{
    EBiomeType LocationBiome = GetBiomeAtLocation(Location);
    if (LocationBiome != CurrentBiome)
    {
        SetBiomeAtmosphere(LocationBiome);
    }
}

EBiomeType AEnvArt_AtmosphereManager::GetBiomeAtLocation(const FVector& Location)
{
    // Simple distance-based biome detection
    float MinDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Savanna;
    
    TMap<EBiomeType, FVector> BiomeCenters;
    BiomeCenters.Add(EBiomeType::Savanna, FVector(0, 0, 0));
    BiomeCenters.Add(EBiomeType::Swamp, FVector(-50000, -45000, 0));
    BiomeCenters.Add(EBiomeType::Forest, FVector(-45000, 40000, 0));
    BiomeCenters.Add(EBiomeType::Desert, FVector(55000, 0, 0));
    BiomeCenters.Add(EBiomeType::Mountain, FVector(40000, 50000, 0));
    
    for (auto& BiomePair : BiomeCenters)
    {
        float Distance = FVector::Dist2D(Location, BiomePair.Value);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = BiomePair.Key;
        }
    }
    
    return ClosestBiome;
}

void AEnvArt_AtmosphereManager::UpdateFogSettings(AExponentialHeightFog* FogActor, const FEnvArt_BiomeAtmosphere& Settings)
{
    if (!FogActor || !FogActor->GetComponent())
    {
        return;
    }
    
    UExponentialHeightFogComponent* FogComponent = FogActor->GetComponent();
    
    FogComponent->SetFogInscatteringColor(Settings.FogColor);
    FogComponent->SetFogDensity(Settings.FogDensity);
    FogComponent->SetFogHeightFalloff(Settings.FogHeightFalloff);
    FogComponent->SetVolumetricFogScatteringDistribution(Settings.VolumetricFogScatteringDistribution);
    FogComponent->SetVolumetricFogAlbedo(Settings.VolumetricFogAlbedo);
    FogComponent->SetVolumetricFogEmissive(Settings.VolumetricFogEmissive);
    
    // Enable volumetric fog
    FogComponent->SetVolumetricFog(true);
}

void AEnvArt_AtmosphereManager::UpdateSunSettings(const FEnvArt_TimeOfDaySettings& Settings)
{
    if (!MainSun || !MainSun->GetLightComponent())
    {
        return;
    }
    
    // Set sun rotation
    FRotator SunRotation(Settings.SunAngleElevation, Settings.SunAngleAzimuth, 0.0f);
    MainSun->SetActorRotation(SunRotation);
    
    // Set sun properties
    UDirectionalLightComponent* LightComponent = MainSun->GetLightComponent();
    LightComponent->SetLightColor(Settings.SunColor);
    LightComponent->SetIntensity(Settings.SunIntensity);
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Updated sun settings - Elevation: %f, Azimuth: %f"), 
           Settings.SunAngleElevation, Settings.SunAngleAzimuth);
}

FEnvArt_BiomeAtmosphere AEnvArt_AtmosphereManager::InterpolateBiomeAtmosphere(const FEnvArt_BiomeAtmosphere& A, const FEnvArt_BiomeAtmosphere& B, float Alpha)
{
    FEnvArt_BiomeAtmosphere Result;
    
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.VolumetricFogScatteringDistribution = FMath::Lerp(A.VolumetricFogScatteringDistribution, B.VolumetricFogScatteringDistribution, Alpha);
    Result.VolumetricFogAlbedo = FMath::Lerp(A.VolumetricFogAlbedo, B.VolumetricFogAlbedo, Alpha);
    Result.VolumetricFogEmissive = FMath::Lerp(A.VolumetricFogEmissive, B.VolumetricFogEmissive, Alpha);
    
    return Result;
}