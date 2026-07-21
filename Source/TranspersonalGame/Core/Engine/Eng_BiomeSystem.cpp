#include "Eng_BiomeSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"

UEng_BiomeSystem::UEng_BiomeSystem()
{
    bIsInitialized = false;
}

void UEng_BiomeSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Initializing Biome System"));
    
    SetupDefaultBiomes();
    InitializeBiomes();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Biome System initialized with %d biomes"), BiomeConfigurations.Num());
}

void UEng_BiomeSystem::Deinitialize()
{
    BiomeConfigurations.Empty();
    BiomeActors.Empty();
    bIsInitialized = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Biome System deinitialized"));
    
    Super::Deinitialize();
}

void UEng_BiomeSystem::SetupDefaultBiomes()
{
    // Savanna - Central biome (0,0)
    FEng_BiomeData SavannaData;
    SavannaData.BiomeType = EEng_BiomeType::Savanna;
    SavannaData.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavannaData.Radius = 30000.0f;
    SavannaData.Temperature = 28.0f;
    SavannaData.Humidity = 0.4f;
    SavannaData.Fertility = 0.6f;
    SavannaData.MaxActorsPerBiome = 4000;
    BiomeConfigurations.Add(EEng_BiomeType::Savanna, SavannaData);

    // Swamp - Southwest (-50000, -45000)
    FEng_BiomeData SwampData;
    SwampData.BiomeType = EEng_BiomeType::Swamp;
    SwampData.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    SwampData.Radius = 25000.0f;
    SwampData.Temperature = 24.0f;
    SwampData.Humidity = 0.9f;
    SwampData.Fertility = 0.8f;
    SwampData.MaxActorsPerBiome = 4000;
    BiomeConfigurations.Add(EEng_BiomeType::Swamp, SwampData);

    // Forest - Northwest (-45000, 40000)
    FEng_BiomeData ForestData;
    ForestData.BiomeType = EEng_BiomeType::Forest;
    ForestData.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    ForestData.Radius = 28000.0f;
    ForestData.Temperature = 22.0f;
    ForestData.Humidity = 0.7f;
    ForestData.Fertility = 0.9f;
    ForestData.MaxActorsPerBiome = 4000;
    BiomeConfigurations.Add(EEng_BiomeType::Forest, ForestData);

    // Desert - East (55000, 0)
    FEng_BiomeData DesertData;
    DesertData.BiomeType = EEng_BiomeType::Desert;
    DesertData.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertData.Radius = 32000.0f;
    DesertData.Temperature = 35.0f;
    DesertData.Humidity = 0.1f;
    DesertData.Fertility = 0.2f;
    DesertData.MaxActorsPerBiome = 4000;
    BiomeConfigurations.Add(EEng_BiomeType::Desert, DesertData);

    // Mountain - Northeast (40000, 50000)
    FEng_BiomeData MountainData;
    MountainData.BiomeType = EEng_BiomeType::Mountain;
    MountainData.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    MountainData.Radius = 26000.0f;
    MountainData.Temperature = 15.0f;
    MountainData.Humidity = 0.5f;
    MountainData.Fertility = 0.4f;
    MountainData.MaxActorsPerBiome = 4000;
    BiomeConfigurations.Add(EEng_BiomeType::Mountain, MountainData);
}

void UEng_BiomeSystem::InitializeBiomes()
{
    if (!bIsInitialized)
    {
        // Initialize actor tracking arrays
        for (const auto& BiomePair : BiomeConfigurations)
        {
            BiomeActors.Add(BiomePair.Key, TArray<TWeakObjectPtr<AActor>>());
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Biome tracking initialized for %d biomes"), BiomeActors.Num());
    }
}

EEng_BiomeType UEng_BiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float MinDistance = MAX_FLT;
    EEng_BiomeType ClosestBiome = EEng_BiomeType::Savanna;

    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FEng_BiomeData& BiomeData = BiomePair.Value;
        float Distance = FVector::Dist2D(WorldLocation, BiomeData.CenterLocation);
        
        // If within biome radius, return this biome
        if (Distance <= BiomeData.Radius)
        {
            return BiomeData.BiomeType;
        }
        
        // Track closest biome for fallback
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = BiomeData.BiomeType;
        }
    }

    // Return closest biome if not within any radius
    return ClosestBiome;
}

FEng_BiomeData UEng_BiomeSystem::GetBiomeData(EEng_BiomeType BiomeType) const
{
    if (const FEng_BiomeData* FoundData = BiomeConfigurations.Find(BiomeType))
    {
        return *FoundData;
    }
    
    // Return default savanna data if not found
    FEng_BiomeData DefaultData;
    return DefaultData;
}

TArray<FEng_BiomeData> UEng_BiomeSystem::GetAllBiomes() const
{
    TArray<FEng_BiomeData> AllBiomes;
    for (const auto& BiomePair : BiomeConfigurations)
    {
        AllBiomes.Add(BiomePair.Value);
    }
    return AllBiomes;
}

bool UEng_BiomeSystem::IsLocationInBiome(const FVector& WorldLocation, EEng_BiomeType BiomeType) const
{
    if (const FEng_BiomeData* BiomeData = BiomeConfigurations.Find(BiomeType))
    {
        float Distance = FVector::Dist2D(WorldLocation, BiomeData->CenterLocation);
        return Distance <= BiomeData->Radius;
    }
    return false;
}

float UEng_BiomeSystem::GetDistanceToBiomeCenter(const FVector& WorldLocation, EEng_BiomeType BiomeType) const
{
    if (const FEng_BiomeData* BiomeData = BiomeConfigurations.Find(BiomeType))
    {
        return FVector::Dist2D(WorldLocation, BiomeData->CenterLocation);
    }
    return MAX_FLT;
}

float UEng_BiomeSystem::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(CurrentBiome);
    
    // Add some variation based on height and time of day
    float HeightVariation = -WorldLocation.Z * 0.01f; // Cooler at higher altitudes
    float BaseTemperature = BiomeData.Temperature + HeightVariation;
    
    return FMath::Clamp(BaseTemperature, -10.0f, 50.0f);
}

float UEng_BiomeSystem::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(CurrentBiome);
    
    return FMath::Clamp(BiomeData.Humidity, 0.0f, 1.0f);
}

float UEng_BiomeSystem::GetFertilityAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData BiomeData = GetBiomeData(CurrentBiome);
    
    return FMath::Clamp(BiomeData.Fertility, 0.0f, 1.0f);
}

float UEng_BiomeSystem::GenerateHeightAtLocation(const FVector& WorldLocation) const
{
    float X = WorldLocation.X * TerrainSettings.NoiseScale;
    float Y = WorldLocation.Y * TerrainSettings.NoiseScale;
    
    float Height = GenerateNoise(X, Y, TerrainSettings.Octaves, TerrainSettings.Persistence, 1.0f);
    
    // Apply biome-specific height modifications
    EEng_BiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    switch (CurrentBiome)
    {
        case EEng_BiomeType::Mountain:
            Height *= 2.0f; // Mountains are higher
            break;
        case EEng_BiomeType::Swamp:
            Height *= 0.3f; // Swamps are lower
            break;
        case EEng_BiomeType::Desert:
            Height *= 0.7f; // Moderate elevation
            break;
        default:
            break;
    }
    
    return Height * TerrainSettings.HeightScale;
}

FVector UEng_BiomeSystem::GetTerrainNormalAtLocation(const FVector& WorldLocation) const
{
    const float SampleDistance = 100.0f;
    
    float HeightCenter = GenerateHeightAtLocation(WorldLocation);
    float HeightRight = GenerateHeightAtLocation(WorldLocation + FVector(SampleDistance, 0, 0));
    float HeightUp = GenerateHeightAtLocation(WorldLocation + FVector(0, SampleDistance, 0));
    
    FVector TangentX = FVector(SampleDistance, 0, HeightRight - HeightCenter).GetSafeNormal();
    FVector TangentY = FVector(0, SampleDistance, HeightUp - HeightCenter).GetSafeNormal();
    
    return FVector::CrossProduct(TangentY, TangentX).GetSafeNormal();
}

bool UEng_BiomeSystem::CanSpawnActorInBiome(EEng_BiomeType BiomeType) const
{
    if (const TArray<TWeakObjectPtr<AActor>>* ActorArray = BiomeActors.Find(BiomeType))
    {
        // Clean up invalid weak pointers
        int32 ValidActorCount = 0;
        for (const TWeakObjectPtr<AActor>& ActorPtr : *ActorArray)
        {
            if (ActorPtr.IsValid())
            {
                ValidActorCount++;
            }
        }
        
        const FEng_BiomeData* BiomeData = BiomeConfigurations.Find(BiomeType);
        if (BiomeData)
        {
            return ValidActorCount < BiomeData->MaxActorsPerBiome;
        }
    }
    return true;
}

int32 UEng_BiomeSystem::GetActorCountInBiome(EEng_BiomeType BiomeType) const
{
    if (const TArray<TWeakObjectPtr<AActor>>* ActorArray = BiomeActors.Find(BiomeType))
    {
        int32 ValidCount = 0;
        for (const TWeakObjectPtr<AActor>& ActorPtr : *ActorArray)
        {
            if (ActorPtr.IsValid())
            {
                ValidCount++;
            }
        }
        return ValidCount;
    }
    return 0;
}

void UEng_BiomeSystem::RegisterActorInBiome(AActor* Actor, EEng_BiomeType BiomeType)
{
    if (Actor && BiomeActors.Contains(BiomeType))
    {
        BiomeActors[BiomeType].Add(Actor);
    }
}

void UEng_BiomeSystem::UnregisterActorFromBiome(AActor* Actor, EEng_BiomeType BiomeType)
{
    if (Actor && BiomeActors.Contains(BiomeType))
    {
        BiomeActors[BiomeType].RemoveAll([Actor](const TWeakObjectPtr<AActor>& WeakPtr)
        {
            return WeakPtr.Get() == Actor;
        });
    }
}

bool UEng_BiomeSystem::ValidateBiomeConfiguration() const
{
    if (BiomeConfigurations.Num() != 5)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Expected 5 biomes, found %d"), BiomeConfigurations.Num());
        return false;
    }
    
    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FEng_BiomeData& Data = BiomePair.Value;
        if (Data.Radius <= 0.0f || Data.MaxActorsPerBiome <= 0)
        {
            UE_LOG(LogTemp, Error, TEXT("Engine Architect: Invalid biome configuration for %s"), 
                   *UEnum::GetValueAsString(Data.BiomeType));
            return false;
        }
    }
    
    return true;
}

void UEng_BiomeSystem::LogBiomeStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== Engine Architect: Biome System Status ==="));
    for (const auto& BiomePair : BiomeConfigurations)
    {
        const FEng_BiomeData& Data = BiomePair.Value;
        int32 ActorCount = GetActorCountInBiome(BiomePair.Key);
        UE_LOG(LogTemp, Warning, TEXT("Biome %s: Center(%s) Radius=%.0f Actors=%d/%d"), 
               *UEnum::GetValueAsString(Data.BiomeType),
               *Data.CenterLocation.ToString(),
               Data.Radius,
               ActorCount,
               Data.MaxActorsPerBiome);
    }
}

float UEng_BiomeSystem::GenerateNoise(float X, float Y, int32 Octaves, float Persistence, float Scale) const
{
    float Value = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Scale;
    float MaxValue = 0.0f;

    for (int32 i = 0; i < Octaves; i++)
    {
        Value += FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)) * Amplitude;
        MaxValue += Amplitude;
        Amplitude *= Persistence;
        Frequency *= TerrainSettings.Lacunarity;
    }

    return Value / MaxValue;
}

// UEng_BiomeAwareComponent Implementation

UEng_BiomeAwareComponent::UEng_BiomeAwareComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    SetTickGroup(TG_PostPhysics);
    
    UpdateFrequency = 1.0f;
    bAutoRegisterWithBiome = true;
    CurrentBiome = EEng_BiomeType::Savanna;
    LastUpdateTime = 0.0f;
}

void UEng_BiomeAwareComponent::BeginPlay()
{
    Super::BeginPlay();
    
    BiomeSystem = GetWorld()->GetGameInstance()->GetSubsystem<UEng_BiomeSystem>();
    if (BiomeSystem)
    {
        UpdateBiomeAwareness();
        
        if (bAutoRegisterWithBiome)
        {
            BiomeSystem->RegisterActorInBiome(GetOwner(), CurrentBiome);
        }
    }
}

void UEng_BiomeAwareComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= (1.0f / UpdateFrequency))
    {
        UpdateBiomeAwareness();
        LastUpdateTime = 0.0f;
    }
}

FEng_BiomeData UEng_BiomeAwareComponent::GetCurrentBiomeData() const
{
    if (BiomeSystem)
    {
        return BiomeSystem->GetBiomeData(CurrentBiome);
    }
    
    return FEng_BiomeData();
}

void UEng_BiomeAwareComponent::UpdateBiomeAwareness()
{
    if (!BiomeSystem || !GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    EEng_BiomeType NewBiome = BiomeSystem->GetBiomeAtLocation(OwnerLocation);
    
    if (NewBiome != CurrentBiome)
    {
        EEng_BiomeType OldBiome = CurrentBiome;
        
        // Unregister from old biome
        if (bAutoRegisterWithBiome)
        {
            BiomeSystem->UnregisterActorFromBiome(GetOwner(), OldBiome);
        }
        
        CurrentBiome = NewBiome;
        
        // Register with new biome
        if (bAutoRegisterWithBiome)
        {
            BiomeSystem->RegisterActorInBiome(GetOwner(), CurrentBiome);
        }
        
        // Broadcast change event
        OnBiomeChanged.Broadcast(OldBiome, NewBiome);
        
        UE_LOG(LogTemp, Log, TEXT("Actor %s moved from %s to %s biome"), 
               *GetOwner()->GetName(),
               *UEnum::GetValueAsString(OldBiome),
               *UEnum::GetValueAsString(NewBiome));
    }
}