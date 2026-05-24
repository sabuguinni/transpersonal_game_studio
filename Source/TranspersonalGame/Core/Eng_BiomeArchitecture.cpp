#include "Eng_BiomeArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

// UEng_BiomeZoneComponent Implementation
UEng_BiomeZoneComponent::UEng_BiomeZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    BiomeType = EEng_BiomeType::Savanna;
    ZoneRadius = 5000.0f;
    ZonePriority = 0;
}

void UEng_BiomeZoneComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with biome manager
    if (UEng_BiomeManager* BiomeManager = GetWorld()->GetGameInstance()->GetSubsystem<UEng_BiomeManager>())
    {
        BiomeManager->RegisterBiomeZone(this);
    }
}

void UEng_BiomeZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    CheckActorProximity();
}

bool UEng_BiomeZoneComponent::IsLocationInZone(const FVector& Location) const
{
    if (!GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Location);
    return Distance <= ZoneRadius;
}

float UEng_BiomeZoneComponent::GetDistanceToZoneCenter(const FVector& Location) const
{
    if (!GetOwner())
    {
        return MAX_FLT;
    }
    
    return FVector::Dist(GetOwner()->GetActorLocation(), Location);
}

void UEng_BiomeZoneComponent::CheckActorProximity()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    // Get player character
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    bool bPlayerInZone = IsLocationInZone(PlayerPawn->GetActorLocation());
    bool bWasInZone = ActorsInZone.Contains(PlayerPawn);
    
    if (bPlayerInZone && !bWasInZone)
    {
        // Player entered zone
        ActorsInZone.Add(PlayerPawn);
        OnBiomeEntered.Broadcast(EEng_BiomeType::Savanna, BiomeType); // From current to this biome
        
        // Notify biome manager
        if (UEng_BiomeManager* BiomeManager = GetWorld()->GetGameInstance()->GetSubsystem<UEng_BiomeManager>())
        {
            BiomeManager->SetCurrentBiome(BiomeType);
        }
    }
    else if (!bPlayerInZone && bWasInZone)
    {
        // Player exited zone
        ActorsInZone.Remove(PlayerPawn);
        OnBiomeExited.Broadcast(BiomeType, EEng_BiomeType::Savanna); // From this biome to default
    }
}

// UEng_BiomeManager Implementation
void UEng_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeDefaultBiomes();
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d default biomes"), RegisteredBiomes.Num());
}

void UEng_BiomeManager::Deinitialize()
{
    RegisteredBiomes.Empty();
    ActiveZones.Empty();
    Super::Deinitialize();
}

void UEng_BiomeManager::RegisterBiome(const FEng_BiomeDefinition& BiomeDefinition)
{
    RegisteredBiomes.Add(BiomeDefinition.BiomeType, BiomeDefinition);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered biome %s"), *BiomeDefinition.BiomeName);
}

EEng_BiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Find the highest priority zone that contains this location
    UEng_BiomeZoneComponent* BestZone = nullptr;
    int32 HighestPriority = -1;
    
    for (UEng_BiomeZoneComponent* Zone : ActiveZones)
    {
        if (Zone && Zone->IsLocationInZone(WorldLocation))
        {
            if (Zone->ZonePriority > HighestPriority)
            {
                BestZone = Zone;
                HighestPriority = Zone->ZonePriority;
            }
        }
    }
    
    return BestZone ? BestZone->BiomeType : EEng_BiomeType::Savanna;
}

FEng_BiomeDefinition UEng_BiomeManager::GetBiomeDefinition(EEng_BiomeType BiomeType) const
{
    if (const FEng_BiomeDefinition* Found = RegisteredBiomes.Find(BiomeType))
    {
        return *Found;
    }
    
    // Return default savanna if not found
    FEng_BiomeDefinition Default;
    Default.BiomeType = EEng_BiomeType::Savanna;
    Default.BiomeName = TEXT("Default Savanna");
    return Default;
}

TArray<EEng_BiomeType> UEng_BiomeManager::GetAllRegisteredBiomes() const
{
    TArray<EEng_BiomeType> BiomeTypes;
    RegisteredBiomes.GetKeys(BiomeTypes);
    return BiomeTypes;
}

void UEng_BiomeManager::SetCurrentBiome(EEng_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        EEng_BiomeType OldBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        OnBiomeChanged.Broadcast(OldBiome, NewBiome);
        
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Biome changed from %d to %d"), (int32)OldBiome, (int32)NewBiome);
    }
}

float UEng_BiomeManager::GetTemperatureAtLocation(const FVector& Location) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeDefinition BiomeDef = GetBiomeDefinition(BiomeType);
    return BiomeDef.Temperature;
}

float UEng_BiomeManager::GetHumidityAtLocation(const FVector& Location) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeDefinition BiomeDef = GetBiomeDefinition(BiomeType);
    return BiomeDef.Humidity;
}

FLinearColor UEng_BiomeManager::GetSkyColorForBiome(EEng_BiomeType BiomeType) const
{
    FEng_BiomeDefinition BiomeDef = GetBiomeDefinition(BiomeType);
    return BiomeDef.SkyColor;
}

void UEng_BiomeManager::InitializeDefaultBiomes()
{
    // Savanna Biome
    FEng_BiomeDefinition Savanna;
    Savanna.BiomeType = EEng_BiomeType::Savanna;
    Savanna.BiomeName = TEXT("African Savanna");
    Savanna.Temperature = 28.0f;
    Savanna.Humidity = 0.4f;
    Savanna.Precipitation = 0.3f;
    Savanna.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);
    Savanna.FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
    Savanna.FogDensity = 0.01f;
    RegisterBiome(Savanna);
    
    // Forest Biome
    FEng_BiomeDefinition Forest;
    Forest.BiomeType = EEng_BiomeType::Forest;
    Forest.BiomeName = TEXT("Prehistoric Forest");
    Forest.Temperature = 22.0f;
    Forest.Humidity = 0.8f;
    Forest.Precipitation = 0.7f;
    Forest.SkyColor = FLinearColor(0.3f, 0.6f, 0.9f, 1.0f);
    Forest.FogColor = FLinearColor(0.7f, 0.9f, 0.7f, 1.0f);
    Forest.FogDensity = 0.03f;
    RegisterBiome(Forest);
    
    // Desert Biome
    FEng_BiomeDefinition Desert;
    Desert.BiomeType = EEng_BiomeType::Desert;
    Desert.BiomeName = TEXT("Cretaceous Desert");
    Desert.Temperature = 35.0f;
    Desert.Humidity = 0.1f;
    Desert.Precipitation = 0.05f;
    Desert.SkyColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
    Desert.FogColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    Desert.FogDensity = 0.005f;
    RegisterBiome(Desert);
    
    // Swamp Biome
    FEng_BiomeDefinition Swamp;
    Swamp.BiomeType = EEng_BiomeType::Swamp;
    Swamp.BiomeName = TEXT("Mesozoic Swamp");
    Swamp.Temperature = 26.0f;
    Swamp.Humidity = 0.9f;
    Swamp.Precipitation = 0.8f;
    Swamp.SkyColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
    Swamp.FogColor = FLinearColor(0.6f, 0.7f, 0.6f, 1.0f);
    Swamp.FogDensity = 0.05f;
    RegisterBiome(Swamp);
    
    // Mountain Biome
    FEng_BiomeDefinition Mountain;
    Mountain.BiomeType = EEng_BiomeType::Mountain;
    Mountain.BiomeName = TEXT("Volcanic Highlands");
    Mountain.Temperature = 15.0f;
    Mountain.Humidity = 0.6f;
    Mountain.Precipitation = 0.5f;
    Mountain.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    Mountain.FogColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
    Mountain.FogDensity = 0.02f;
    RegisterBiome(Mountain);
}

void UEng_BiomeManager::RegisterBiomeZone(UEng_BiomeZoneComponent* ZoneComponent)
{
    if (ZoneComponent && !ActiveZones.Contains(ZoneComponent))
    {
        ActiveZones.Add(ZoneComponent);
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered biome zone for %d"), (int32)ZoneComponent->BiomeType);
    }
}

void UEng_BiomeManager::UnregisterBiomeZone(UEng_BiomeZoneComponent* ZoneComponent)
{
    if (ZoneComponent)
    {
        ActiveZones.Remove(ZoneComponent);
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Unregistered biome zone for %d"), (int32)ZoneComponent->BiomeType);
    }
}

// AEng_BiomeMarker Implementation
AEng_BiomeMarker::AEng_BiomeMarker()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create marker mesh
    MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
    MarkerMesh->SetupAttachment(RootComponent);
    MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Create biome zone component
    BiomeZone = CreateDefaultSubobject<UEng_BiomeZoneComponent>(TEXT("BiomeZone"));
    
    // Set default values
    BiomeType = EEng_BiomeType::Savanna;
    BiomeRadius = 5000.0f;
}

void AEng_BiomeMarker::BeginPlay()
{
    Super::BeginPlay();
    
    // Sync component values with actor values
    if (BiomeZone)
    {
        BiomeZone->BiomeType = BiomeType;
        BiomeZone->ZoneRadius = BiomeRadius;
    }
}

void AEng_BiomeMarker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Unregister from biome manager
    if (BiomeZone && GetWorld())
    {
        if (UEng_BiomeManager* BiomeManager = GetWorld()->GetGameInstance()->GetSubsystem<UEng_BiomeManager>())
        {
            BiomeManager->UnregisterBiomeZone(BiomeZone);
        }
    }
    
    Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void AEng_BiomeMarker::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    // Sync changes to component
    if (BiomeZone)
    {
        BiomeZone->BiomeType = BiomeType;
        BiomeZone->ZoneRadius = BiomeRadius;
    }
}
#endif