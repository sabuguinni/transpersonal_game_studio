#include "World_VolcanicLandscapeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PointLight.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

AWorld_VolcanicLandscapeManager::AWorld_VolcanicLandscapeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default volcanic features
    MaxVolcanicFeatures = 10;
    VolcanicActivityUpdateInterval = 5.0f;
    GeothermalEffectRadius = 2000.0f;
    bEnableVolcanicSounds = true;
    bEnableVolcanicEffects = true;
    
    LastVolcanicUpdateTime = 0.0f;
    ActiveVolcanicFeatures = 0;
}

void AWorld_VolcanicLandscapeManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_VolcanicLandscapeManager: BeginPlay started"));
    
    // Generate initial volcanic landscape
    GenerateVolcanicLandscape();
}

void AWorld_VolcanicLandscapeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update volcanic activity periodically
    if (GetWorld()->GetTimeSeconds() - LastVolcanicUpdateTime > VolcanicActivityUpdateInterval)
    {
        UpdateVolcanicActivity(DeltaTime);
        LastVolcanicUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void AWorld_VolcanicLandscapeManager::GenerateVolcanicLandscape()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating volcanic landscape..."));
    
    // Clear existing features
    VolcanicFeatures.Empty();
    GeothermalZones.Empty();
    
    // Create main volcanic cone
    FWorld_VolcanicFeature MainVolcano;
    MainVolcano.Location = FVector(2000, 2000, 100);
    MainVolcano.Scale = 50.0f;
    MainVolcano.VolcanicType = EWorld_VolcanicType::Active;
    MainVolcano.bIsActive = true;
    MainVolcano.Temperature = 800.0f;
    MainVolcano.DangerRadius = 1500.0f;
    VolcanicFeatures.Add(MainVolcano);
    
    CreateVolcanicCone(MainVolcano.Location, MainVolcano.Scale, MainVolcano.VolcanicType);
    
    // Create smaller volcanic vents
    for (int32 i = 0; i < 3; i++)
    {
        float Angle = i * 120.0f; // 120 degrees apart
        float Distance = 1500.0f;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector VentLocation = FVector(
            2000 + Distance * FMath::Cos(RadianAngle),
            2000 + Distance * FMath::Sin(RadianAngle),
            50
        );
        
        FWorld_VolcanicFeature Vent;
        Vent.Location = VentLocation;
        Vent.Scale = 15.0f;
        Vent.VolcanicType = EWorld_VolcanicType::Dormant;
        Vent.bIsActive = false;
        Vent.Temperature = 200.0f;
        Vent.DangerRadius = 500.0f;
        VolcanicFeatures.Add(Vent);
        
        CreateVolcanicCone(Vent.Location, Vent.Scale, Vent.VolcanicType);
    }
    
    // Create geothermal zones
    FWorld_GeothermalZone MainGeothermalZone;
    MainGeothermalZone.CenterLocation = FVector(2000, 2000, 0);
    MainGeothermalZone.Radius = 3000.0f;
    MainGeothermalZone.AverageTemperature = 60.0f;
    MainGeothermalZone.NumHotSprings = 8;
    GeothermalZones.Add(MainGeothermalZone);
    
    CreateGeothermalZone(MainGeothermalZone.CenterLocation, MainGeothermalZone.Radius);
    CreateHotSprings(MainGeothermalZone);
    
    // Create volcanic rock formations
    CreateVolcanicRocks(FVector(2000, 2000, 0), 4000.0f, 20);
    
    UE_LOG(LogTemp, Warning, TEXT("Volcanic landscape generation complete. Features: %d, Zones: %d"), 
           VolcanicFeatures.Num(), GeothermalZones.Num());
}

void AWorld_VolcanicLandscapeManager::CreateVolcanicCone(const FVector& Location, float Scale, EWorld_VolcanicType VolcanicType)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Spawn a static mesh actor for the volcanic cone
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    
    AStaticMeshActor* VolcanicCone = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (VolcanicCone)
    {
        // Set scale based on volcanic type
        FVector ConeScale = FVector(Scale, Scale, Scale * 0.4f); // Cones are wider than tall
        if (VolcanicType == EWorld_VolcanicType::Active)
        {
            ConeScale *= 1.5f; // Active volcanoes are larger
        }
        
        VolcanicCone->SetActorScale3D(ConeScale);
        VolcanicCone->SetActorLabel(FString::Printf(TEXT("VolcanicCone_%s"), 
                                   VolcanicType == EWorld_VolcanicType::Active ? TEXT("Active") : TEXT("Dormant")));
        
        VolcanicCones.Add(VolcanicCone);
        
        // Add smoke effects for active volcanoes
        if (VolcanicType == EWorld_VolcanicType::Active && bEnableVolcanicEffects)
        {
            CreateVolcanicSmoke(Location + FVector(0, 0, Scale * 20));
        }
        
        UE_LOG(LogTemp, Log, TEXT("Created volcanic cone at %s with scale %f"), *Location.ToString(), Scale);
    }
}

void AWorld_VolcanicLandscapeManager::CreateGeothermalZone(const FVector& CenterLocation, float Radius)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Create ambient lighting for the geothermal zone
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    
    APointLight* GeothermalLight = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), 
                                                                      CenterLocation + FVector(0, 0, 200), 
                                                                      FRotator::ZeroRotator, 
                                                                      SpawnParams);
    
    if (GeothermalLight)
    {
        GeothermalLight->SetActorLabel(TEXT("GeothermalZone_AmbientLight"));
        
        // Configure light properties
        UPointLightComponent* LightComponent = GeothermalLight->GetPointLightComponent();
        if (LightComponent)
        {
            LightComponent->SetIntensity(500.0f);
            LightComponent->SetAttenuationRadius(Radius);
            LightComponent->SetLightColor(FLinearColor(1.0f, 0.6f, 0.2f)); // Warm orange glow
        }
        
        GeothermalLights.Add(GeothermalLight);
        
        UE_LOG(LogTemp, Log, TEXT("Created geothermal zone at %s with radius %f"), *CenterLocation.ToString(), Radius);
    }
}

void AWorld_VolcanicLandscapeManager::CreateHotSprings(const FWorld_GeothermalZone& Zone)
{
    if (!GetWorld())
    {
        return;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    
    for (int32 i = 0; i < Zone.NumHotSprings; i++)
    {
        // Generate random position within the geothermal zone
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(100.0f, Zone.Radius * 0.8f);
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector HotSpringLocation = Zone.CenterLocation + FVector(
            Distance * FMath::Cos(RadianAngle),
            Distance * FMath::Sin(RadianAngle),
            0
        );
        
        // Spawn hot spring as a flattened sphere
        AStaticMeshActor* HotSpring = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), 
                                                                              HotSpringLocation, 
                                                                              FRotator::ZeroRotator, 
                                                                              SpawnParams);
        
        if (HotSpring)
        {
            HotSpring->SetActorScale3D(FVector(8.0f, 8.0f, 1.0f)); // Flat like a pool
            HotSpring->SetActorLabel(FString::Printf(TEXT("HotSpring_%d"), i + 1));
            
            HotSprings.Add(HotSpring);
            
            // Add steam effects if enabled
            if (bEnableVolcanicEffects)
            {
                CreateVolcanicSmoke(HotSpringLocation + FVector(0, 0, 50));
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created %d hot springs in geothermal zone"), Zone.NumHotSprings);
}

void AWorld_VolcanicLandscapeManager::CreateVolcanicRocks(const FVector& CenterLocation, float Radius, int32 NumRocks)
{
    if (!GetWorld())
    {
        return;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    
    for (int32 i = 0; i < NumRocks; i++)
    {
        // Generate random position within radius
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(200.0f, Radius);
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector RockLocation = CenterLocation + FVector(
            Distance * FMath::Cos(RadianAngle),
            Distance * FMath::Sin(RadianAngle),
            FMath::RandRange(0.0f, 100.0f)
        );
        
        // Spawn rock as a cube with random scale
        AStaticMeshActor* VolcanicRock = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), 
                                                                                 RockLocation, 
                                                                                 FRotator(FMath::RandRange(0.0f, 360.0f), 
                                                                                         FMath::RandRange(0.0f, 360.0f), 
                                                                                         FMath::RandRange(0.0f, 360.0f)), 
                                                                                 SpawnParams);
        
        if (VolcanicRock)
        {
            float RockScale = FMath::RandRange(2.0f, 8.0f);
            VolcanicRock->SetActorScale3D(FVector(RockScale, RockScale * 0.8f, RockScale * 1.2f));
            VolcanicRock->SetActorLabel(FString::Printf(TEXT("VolcanicRock_%d"), i + 1));
            
            VolcanicRocks.Add(VolcanicRock);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created %d volcanic rocks around %s"), NumRocks, *CenterLocation.ToString());
}

void AWorld_VolcanicLandscapeManager::UpdateVolcanicActivity(float DeltaTime)
{
    ActiveVolcanicFeatures = 0;
    
    for (int32 i = 0; i < VolcanicFeatures.Num(); i++)
    {
        FWorld_VolcanicFeature& Feature = VolcanicFeatures[i];
        
        if (Feature.bIsActive)
        {
            ActiveVolcanicFeatures++;
            
            // Randomly trigger eruption for active volcanoes
            if (Feature.VolcanicType == EWorld_VolcanicType::Active && FMath::RandRange(0.0f, 1.0f) < 0.01f)
            {
                TriggerVolcanicEruption(Feature.Location);
            }
        }
        
        // Temperature fluctuation
        float TemperatureVariation = FMath::RandRange(-10.0f, 10.0f);
        Feature.Temperature = FMath::Clamp(Feature.Temperature + TemperatureVariation, 20.0f, 1000.0f);
    }
    
    if (ActiveVolcanicFeatures > 0)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Updated %d active volcanic features"), ActiveVolcanicFeatures);
    }
}

void AWorld_VolcanicLandscapeManager::TriggerVolcanicEruption(const FVector& Location)
{
    UE_LOG(LogTemp, Warning, TEXT("VOLCANIC ERUPTION triggered at %s!"), *Location.ToString());
    
    // Create eruption effects
    if (bEnableVolcanicEffects)
    {
        CreateVolcanicSmoke(Location + FVector(0, 0, 500));
        CreateLavaFlow(Location, Location + FVector(FMath::RandRange(-1000, 1000), FMath::RandRange(-1000, 1000), -200));
    }
}

void AWorld_VolcanicLandscapeManager::CreateVolcanicSmoke(const FVector& Location)
{
    // Placeholder for particle system - would normally create a particle system component
    UE_LOG(LogTemp, Log, TEXT("Creating volcanic smoke effect at %s"), *Location.ToString());
    
    // For now, create a light to represent the smoke/glow
    if (GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        
        APointLight* SmokeLight = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), 
                                                                     Location, 
                                                                     FRotator::ZeroRotator, 
                                                                     SpawnParams);
        
        if (SmokeLight)
        {
            SmokeLight->SetActorLabel(TEXT("VolcanicSmoke_Effect"));
            
            UPointLightComponent* LightComponent = SmokeLight->GetPointLightComponent();
            if (LightComponent)
            {
                LightComponent->SetIntensity(200.0f);
                LightComponent->SetAttenuationRadius(300.0f);
                LightComponent->SetLightColor(FLinearColor(0.8f, 0.4f, 0.1f)); // Orange smoke glow
            }
            
            GeothermalLights.Add(SmokeLight);
        }
    }
}

void AWorld_VolcanicLandscapeManager::CreateLavaFlow(const FVector& StartLocation, const FVector& EndLocation)
{
    UE_LOG(LogTemp, Warning, TEXT("Creating lava flow from %s to %s"), *StartLocation.ToString(), *EndLocation.ToString());
    
    // Create a series of glowing spheres to represent lava flow
    if (GetWorld())
    {
        FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
        float Distance = FVector::Dist(StartLocation, EndLocation);
        int32 NumLavaChunks = FMath::Max(1, FMath::FloorToInt(Distance / 100.0f));
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        
        for (int32 i = 0; i < NumLavaChunks; i++)
        {
            float Alpha = (float)i / (float)NumLavaChunks;
            FVector LavaLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
            
            APointLight* LavaLight = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), 
                                                                        LavaLocation, 
                                                                        FRotator::ZeroRotator, 
                                                                        SpawnParams);
            
            if (LavaLight)
            {
                LavaLight->SetActorLabel(FString::Printf(TEXT("LavaFlow_%d"), i));
                
                UPointLightComponent* LightComponent = LavaLight->GetPointLightComponent();
                if (LightComponent)
                {
                    LightComponent->SetIntensity(800.0f);
                    LightComponent->SetAttenuationRadius(150.0f);
                    LightComponent->SetLightColor(FLinearColor(1.0f, 0.2f, 0.0f)); // Bright red lava glow
                }
                
                GeothermalLights.Add(LavaLight);
            }
        }
    }
}

bool AWorld_VolcanicLandscapeManager::IsLocationInVolcanicZone(const FVector& Location) const
{
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        float Distance = FVector::Dist(Location, Feature.Location);
        if (Distance <= Feature.DangerRadius)
        {
            return true;
        }
    }
    
    return false;
}

float AWorld_VolcanicLandscapeManager::GetVolcanicTemperatureAtLocation(const FVector& Location) const
{
    float MaxTemperature = 25.0f; // Base environmental temperature
    
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        float Distance = FVector::Dist(Location, Feature.Location);
        if (Distance <= Feature.DangerRadius)
        {
            // Temperature decreases with distance from volcanic feature
            float TemperatureInfluence = FMath::Clamp(1.0f - (Distance / Feature.DangerRadius), 0.0f, 1.0f);
            float LocalTemperature = 25.0f + (Feature.Temperature - 25.0f) * TemperatureInfluence;
            MaxTemperature = FMath::Max(MaxTemperature, LocalTemperature);
        }
    }
    
    return MaxTemperature;
}

EWorld_BiomeType AWorld_VolcanicLandscapeManager::GetVolcanicBiomeType(const FVector& Location) const
{
    if (IsLocationInVolcanicZone(Location))
    {
        float Temperature = GetVolcanicTemperatureAtLocation(Location);
        
        if (Temperature > 100.0f)
        {
            return EWorld_BiomeType::Volcanic;
        }
        else if (Temperature > 60.0f)
        {
            return EWorld_BiomeType::Geothermal;
        }
    }
    
    return EWorld_BiomeType::Temperate; // Default biome
}

void AWorld_VolcanicLandscapeManager::OptimizeVolcanicFeatures(const FVector& PlayerLocation, float ViewDistance)
{
    for (int32 i = 0; i < VolcanicCones.Num(); i++)
    {
        if (VolcanicCones[i])
        {
            float Distance = FVector::Dist(PlayerLocation, VolcanicCones[i]->GetActorLocation());
            bool bShouldBeVisible = Distance <= ViewDistance;
            
            VolcanicCones[i]->SetActorHiddenInGame(!bShouldBeVisible);
        }
    }
    
    for (int32 i = 0; i < HotSprings.Num(); i++)
    {
        if (HotSprings[i])
        {
            float Distance = FVector::Dist(PlayerLocation, HotSprings[i]->GetActorLocation());
            bool bShouldBeVisible = Distance <= ViewDistance * 0.5f; // Hot springs visible at shorter distance
            
            HotSprings[i]->SetActorHiddenInGame(!bShouldBeVisible);
        }
    }
}

void AWorld_VolcanicLandscapeManager::EnableVolcanicFeature(int32 FeatureIndex, bool bEnable)
{
    if (VolcanicFeatures.IsValidIndex(FeatureIndex))
    {
        VolcanicFeatures[FeatureIndex].bIsActive = bEnable;
        
        UE_LOG(LogTemp, Log, TEXT("Volcanic feature %d %s"), FeatureIndex, bEnable ? TEXT("enabled") : TEXT("disabled"));
    }
}