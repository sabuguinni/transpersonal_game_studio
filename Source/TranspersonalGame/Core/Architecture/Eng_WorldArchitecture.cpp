#include "Eng_WorldArchitecture.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UEng_WorldArchitecture::UEng_WorldArchitecture()
{
    WorldBounds = 20000.0f;
    MaxTotalActors = 8000;
    bWorldInitialized = false;
}

void UEng_WorldArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitecture: Initializing world architecture system"));
    
    InitializeWorldZones();
    bWorldInitialized = true;
}

void UEng_WorldArchitecture::Deinitialize()
{
    WorldZones.Empty();
    bWorldInitialized = false;
    
    Super::Deinitialize();
}

void UEng_WorldArchitecture::InitializeWorldZones()
{
    WorldZones.Empty();
    CreateDefaultZones();
    ValidateZoneOverlaps();
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitecture: Created %d world zones"), WorldZones.Num());
}

void UEng_WorldArchitecture::CreateDefaultZones()
{
    // Central grassland zone
    FEng_WorldZone CentralZone;
    CentralZone.ZoneName = TEXT("Central Grassland");
    CentralZone.ZoneCenter = FVector(0.0f, 0.0f, 0.0f);
    CentralZone.ZoneRadius = 8000.0f;
    CentralZone.BiomeType = EBiomeType::Grassland;
    CentralZone.MaxDinosaurs = 50;
    CentralZone.AllowedSpecies.Add(EDinosaurSpecies::Triceratops);
    CentralZone.AllowedSpecies.Add(EDinosaurSpecies::Parasaurolophus);
    CentralZone.AllowedSpecies.Add(EDinosaurSpecies::Brachiosaurus);
    WorldZones.Add(CentralZone);

    // Northern forest zone
    FEng_WorldZone ForestZone;
    ForestZone.ZoneName = TEXT("Northern Forest");
    ForestZone.ZoneCenter = FVector(0.0f, 15000.0f, 0.0f);
    ForestZone.ZoneRadius = 6000.0f;
    ForestZone.BiomeType = EBiomeType::Forest;
    ForestZone.MaxDinosaurs = 30;
    ForestZone.AllowedSpecies.Add(EDinosaurSpecies::Velociraptor);
    ForestZone.AllowedSpecies.Add(EDinosaurSpecies::Compsognathus);
    WorldZones.Add(ForestZone);

    // Southern desert zone
    FEng_WorldZone DesertZone;
    DesertZone.ZoneName = TEXT("Southern Desert");
    DesertZone.ZoneCenter = FVector(0.0f, -15000.0f, 0.0f);
    DesertZone.ZoneRadius = 7000.0f;
    DesertZone.BiomeType = EBiomeType::Desert;
    DesertZone.MaxDinosaurs = 25;
    DesertZone.AllowedSpecies.Add(EDinosaurSpecies::TRex);
    DesertZone.AllowedSpecies.Add(EDinosaurSpecies::Ankylosaurus);
    WorldZones.Add(DesertZone);

    // Eastern mountain zone
    FEng_WorldZone MountainZone;
    MountainZone.ZoneName = TEXT("Eastern Mountains");
    MountainZone.ZoneCenter = FVector(15000.0f, 0.0f, 2000.0f);
    MountainZone.ZoneRadius = 5000.0f;
    MountainZone.BiomeType = EBiomeType::Mountain;
    MountainZone.MaxDinosaurs = 15;
    MountainZone.AllowedSpecies.Add(EDinosaurSpecies::Pteranodon);
    WorldZones.Add(MountainZone);
}

void UEng_WorldArchitecture::RegisterWorldZone(const FEng_WorldZone& Zone)
{
    WorldZones.Add(Zone);
    ValidateZoneOverlaps();
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitecture: Registered zone %s at %s"), 
           *Zone.ZoneName, *Zone.ZoneCenter.ToString());
}

FEng_WorldZone UEng_WorldArchitecture::GetZoneAtLocation(const FVector& Location)
{
    for (const FEng_WorldZone& Zone : WorldZones)
    {
        float Distance = FVector::Dist(Location, Zone.ZoneCenter);
        if (Distance <= Zone.ZoneRadius)
        {
            return Zone;
        }
    }
    
    // Return default zone if no match found
    FEng_WorldZone DefaultZone;
    DefaultZone.ZoneName = TEXT("Unknown Zone");
    DefaultZone.BiomeType = EBiomeType::Grassland;
    return DefaultZone;
}

TArray<FEng_WorldZone> UEng_WorldArchitecture::GetAllZones() const
{
    return WorldZones;
}

void UEng_WorldArchitecture::SetTerrainSettings(const FEng_TerrainSettings& Settings)
{
    TerrainSettings = Settings;
    UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitecture: Updated terrain settings - Height: %f, Noise: %f"), 
           Settings.HeightScale, Settings.NoiseScale);
}

FEng_TerrainSettings UEng_WorldArchitecture::GetTerrainSettings() const
{
    return TerrainSettings;
}

bool UEng_WorldArchitecture::IsLocationInValidZone(const FVector& Location)
{
    FEng_WorldZone Zone = GetZoneAtLocation(Location);
    return Zone.ZoneName != TEXT("Unknown Zone");
}

void UEng_WorldArchitecture::ValidateWorldIntegrity()
{
    ValidateZoneOverlaps();
    
    int32 TotalMaxDinosaurs = 0;
    for (const FEng_WorldZone& Zone : WorldZones)
    {
        TotalMaxDinosaurs += Zone.MaxDinosaurs;
    }
    
    if (TotalMaxDinosaurs > 150)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitecture: WARNING - Total max dinosaurs (%d) exceeds cap of 150"), 
               TotalMaxDinosaurs);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitecture: World integrity check complete - %d zones, max %d dinosaurs"), 
           WorldZones.Num(), TotalMaxDinosaurs);
}

void UEng_WorldArchitecture::ValidateZoneOverlaps()
{
    for (int32 i = 0; i < WorldZones.Num(); i++)
    {
        for (int32 j = i + 1; j < WorldZones.Num(); j++)
        {
            float Distance = CalculateZoneDistance(WorldZones[i], WorldZones[j]);
            float MinDistance = WorldZones[i].ZoneRadius + WorldZones[j].ZoneRadius;
            
            if (Distance < MinDistance)
            {
                UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitecture: Zone overlap detected between %s and %s"), 
                       *WorldZones[i].ZoneName, *WorldZones[j].ZoneName);
            }
        }
    }
}

float UEng_WorldArchitecture::CalculateZoneDistance(const FEng_WorldZone& ZoneA, const FEng_WorldZone& ZoneB)
{
    return FVector::Dist(ZoneA.ZoneCenter, ZoneB.ZoneCenter);
}