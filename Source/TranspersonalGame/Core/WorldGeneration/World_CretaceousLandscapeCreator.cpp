#include "World_CretaceousLandscapeCreator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "LandscapeEditorObject.h"
#include "EditorLevelLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"

DEFINE_LOG_CATEGORY_STATIC(LogCretaceousLandscape, Log, All);

AWorld_CretaceousLandscapeCreator::AWorld_CretaceousLandscapeCreator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    LandscapeSize = FVector2D(10000.0f, 10000.0f); // 10km x 10km
    HeightmapResolution = 2048;
    QuadsPerSection = 63;
    SectionsPerComponent = 2;
    ComponentCountX = 32;
    ComponentCountY = 32;
    LandscapeScale = FVector(100.0f, 100.0f, 100.0f);
    
    bAutoCreateOnBeginPlay = true;
    bCreateBiomeZones = true;
    
    // Initialize biome zone data
    InitializeBiomeZones();
}

void AWorld_CretaceousLandscapeCreator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoCreateOnBeginPlay)
    {
        CreateCretaceousLandscape();
    }
}

void AWorld_CretaceousLandscapeCreator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWorld_CretaceousLandscapeCreator::InitializeBiomeZones()
{
    BiomeZones.Empty();
    
    // Define 5 biome zones for Cretaceous period
    FWorld_BiomeZoneData SwampZone;
    SwampZone.ZoneName = TEXT("Pantano_SW");
    SwampZone.BiomeType = EWorld_BiomeType::Swamp;
    SwampZone.Location = FVector(-2500.0f, -2500.0f, 100.0f);
    SwampZone.Radius = 1000.0f;
    SwampZone.Humidity = 0.9f;
    SwampZone.Temperature = 0.8f;
    BiomeZones.Add(SwampZone);
    
    FWorld_BiomeZoneData ForestZone;
    ForestZone.ZoneName = TEXT("Floresta_NW");
    ForestZone.BiomeType = EWorld_BiomeType::Forest;
    ForestZone.Location = FVector(-2500.0f, 2500.0f, 100.0f);
    ForestZone.Radius = 1200.0f;
    ForestZone.Humidity = 0.7f;
    ForestZone.Temperature = 0.7f;
    BiomeZones.Add(ForestZone);
    
    FWorld_BiomeZoneData SavannaZone;
    SavannaZone.ZoneName = TEXT("Savana_Centro");
    SavannaZone.BiomeType = EWorld_BiomeType::Savanna;
    SavannaZone.Location = FVector(0.0f, 0.0f, 100.0f);
    SavannaZone.Radius = 1500.0f;
    SavannaZone.Humidity = 0.5f;
    SavannaZone.Temperature = 0.8f;
    BiomeZones.Add(SavannaZone);
    
    FWorld_BiomeZoneData DesertZone;
    DesertZone.ZoneName = TEXT("Deserto_E");
    DesertZone.BiomeType = EWorld_BiomeType::Desert;
    DesertZone.Location = FVector(2500.0f, 0.0f, 100.0f);
    DesertZone.Radius = 1000.0f;
    DesertZone.Humidity = 0.2f;
    DesertZone.Temperature = 0.9f;
    BiomeZones.Add(DesertZone);
    
    FWorld_BiomeZoneData MountainZone;
    MountainZone.ZoneName = TEXT("Montanha_NE");
    MountainZone.BiomeType = EWorld_BiomeType::Mountain;
    MountainZone.Location = FVector(2500.0f, 2500.0f, 300.0f);
    MountainZone.Radius = 800.0f;
    MountainZone.Humidity = 0.4f;
    MountainZone.Temperature = 0.4f;
    BiomeZones.Add(MountainZone);
    
    UE_LOG(LogCretaceousLandscape, Log, TEXT("Initialized %d biome zones"), BiomeZones.Num());
}

bool AWorld_CretaceousLandscapeCreator::CreateCretaceousLandscape()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogCretaceousLandscape, Error, TEXT("No valid world found"));
        return false;
    }
    
    // Check if landscape already exists
    TArray<AActor*> ExistingLandscapes;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), ExistingLandscapes);
    
    if (ExistingLandscapes.Num() > 0)
    {
        MainLandscape = Cast<ALandscape>(ExistingLandscapes[0]);
        if (MainLandscape)
        {
            MainLandscape->SetActorLabel(TEXT("MainLandscape_Cretaceous"));
            UE_LOG(LogCretaceousLandscape, Log, TEXT("Using existing landscape: %s"), *MainLandscape->GetActorLabel());
        }
    }
    else
    {
        // Create new landscape
        MainLandscape = CreateNewLandscape();
    }
    
    if (MainLandscape && bCreateBiomeZones)
    {
        CreateBiomeZoneActors();
    }
    
    return MainLandscape != nullptr;
}

ALandscape* AWorld_CretaceousLandscapeCreator::CreateNewLandscape()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Create landscape actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName(TEXT("MainLandscape_Cretaceous"));
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    ALandscape* NewLandscape = World->SpawnActor<ALandscape>(
        ALandscape::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams
    );
    
    if (NewLandscape)
    {
        NewLandscape->SetActorLabel(TEXT("MainLandscape_Cretaceous"));
        NewLandscape->SetActorScale3D(LandscapeScale);
        
        UE_LOG(LogCretaceousLandscape, Log, TEXT("Created new landscape: %s"), *NewLandscape->GetActorLabel());
    }
    else
    {
        UE_LOG(LogCretaceousLandscape, Error, TEXT("Failed to create landscape actor"));
    }
    
    return NewLandscape;
}

void AWorld_CretaceousLandscapeCreator::CreateBiomeZoneActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    CreatedBiomeActors.Empty();
    
    for (const FWorld_BiomeZoneData& ZoneData : BiomeZones)
    {
        // Create biome zone actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*FString::Printf(TEXT("BiomeZone_%s"), *ZoneData.ZoneName));
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AActor* BiomeActor = World->SpawnActor<AActor>(
            AActor::StaticClass(),
            ZoneData.Location,
            FRotator::ZeroRotator,
            SpawnParams
        );
        
        if (BiomeActor)
        {
            BiomeActor->SetActorLabel(FString::Printf(TEXT("BiomeZone_%s"), *ZoneData.ZoneName));
            
            // Add sphere component for visualization
            UStaticMeshComponent* SphereComp = NewObject<UStaticMeshComponent>(BiomeActor);
            if (SphereComp)
            {
                BiomeActor->SetRootComponent(SphereComp);
                
                // Load sphere mesh
                UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
                if (SphereMesh)
                {
                    SphereComp->SetStaticMesh(SphereMesh);
                    
                    // Scale based on biome radius
                    float ScaleFactor = ZoneData.Radius / 50.0f; // Sphere is 100 units, we want radius in cm
                    SphereComp->SetWorldScale3D(FVector(ScaleFactor, ScaleFactor, ScaleFactor));
                }
            }
            
            CreatedBiomeActors.Add(BiomeActor);
            UE_LOG(LogCretaceousLandscape, Log, TEXT("Created biome zone: %s at %s"), 
                   *ZoneData.ZoneName, *ZoneData.Location.ToString());
        }
    }
    
    UE_LOG(LogCretaceousLandscape, Log, TEXT("Created %d biome zone actors"), CreatedBiomeActors.Num());
}

bool AWorld_CretaceousLandscapeCreator::ExpandLandscapeSize(FVector2D NewSize)
{
    if (!MainLandscape)
    {
        UE_LOG(LogCretaceousLandscape, Warning, TEXT("No landscape to expand"));
        return false;
    }
    
    LandscapeSize = NewSize;
    
    // Calculate new scale
    FVector NewScale = FVector(
        NewSize.X / 10000.0f * 100.0f,
        NewSize.Y / 10000.0f * 100.0f,
        LandscapeScale.Z
    );
    
    MainLandscape->SetActorScale3D(NewScale);
    LandscapeScale = NewScale;
    
    UE_LOG(LogCretaceousLandscape, Log, TEXT("Expanded landscape to %s with scale %s"), 
           *NewSize.ToString(), *NewScale.ToString());
    
    return true;
}

FWorld_BiomeZoneData AWorld_CretaceousLandscapeCreator::GetBiomeDataAtLocation(FVector Location) const
{
    FWorld_BiomeZoneData DefaultBiome;
    DefaultBiome.BiomeType = EWorld_BiomeType::Savanna;
    DefaultBiome.ZoneName = TEXT("Default");
    DefaultBiome.Humidity = 0.5f;
    DefaultBiome.Temperature = 0.6f;
    
    float ClosestDistance = FLT_MAX;
    FWorld_BiomeZoneData ClosestBiome = DefaultBiome;
    
    for (const FWorld_BiomeZoneData& ZoneData : BiomeZones)
    {
        float Distance = FVector::Dist2D(Location, ZoneData.Location);
        if (Distance <= ZoneData.Radius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = ZoneData;
        }
    }
    
    return ClosestBiome;
}

TArray<FWorld_BiomeZoneData> AWorld_CretaceousLandscapeCreator::GetAllBiomeZones() const
{
    return BiomeZones;
}

void AWorld_CretaceousLandscapeCreator::RegenerateBiomeZones()
{
    // Remove existing biome actors
    for (AActor* Actor : CreatedBiomeActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    CreatedBiomeActors.Empty();
    
    // Recreate biome zones
    InitializeBiomeZones();
    CreateBiomeZoneActors();
    
    UE_LOG(LogCretaceousLandscape, Log, TEXT("Regenerated biome zones"));
}

bool AWorld_CretaceousLandscapeCreator::ValidateLandscapeSetup() const
{
    if (!MainLandscape)
    {
        UE_LOG(LogCretaceousLandscape, Warning, TEXT("No main landscape found"));
        return false;
    }
    
    if (BiomeZones.Num() != 5)
    {
        UE_LOG(LogCretaceousLandscape, Warning, TEXT("Expected 5 biome zones, found %d"), BiomeZones.Num());
        return false;
    }
    
    if (CreatedBiomeActors.Num() != BiomeZones.Num())
    {
        UE_LOG(LogCretaceousLandscape, Warning, TEXT("Biome actor count mismatch"));
        return false;
    }
    
    // Check landscape size meets minimum 10km requirement
    FVector CurrentScale = MainLandscape->GetActorScale3D();
    float EffectiveSize = FMath::Max(CurrentScale.X, CurrentScale.Y) * 100.0f; // Convert to meters
    
    if (EffectiveSize < 10000.0f)
    {
        UE_LOG(LogCretaceousLandscape, Warning, TEXT("Landscape size %f is below 10km requirement"), EffectiveSize);
        return false;
    }
    
    UE_LOG(LogCretaceousLandscape, Log, TEXT("Landscape setup validation passed"));
    return true;
}