#include "World_ResourceDistributionManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"

UWorld_ResourceDistributionManager::UWorld_ResourceDistributionManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = ResourceUpdateInterval;
    
    // Initialize default settings
    MaxResourceNodes = 500;
    MinResourceDistance = 200.0f;
    ResourceClusterRadius = 1000.0f;
    MaxWaterSources = 50;
    MaxShelterLocations = 30;
    ResourceUpdateInterval = 5.0f;
    MaxResourceDistance = 10000.0f;
    MaxActiveResourceNodes = 100;
    
    LastResourceUpdate = 0.0f;
    ActiveResourceNodes = 0;
    bResourceSystemInitialized = false;
}

void UWorld_ResourceDistributionManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize biome resource profiles
    InitializeDefaultBiomeProfiles();
    
    // Generate initial resource distribution
    GenerateResourceNodes();
    GenerateWaterSources();
    GenerateShelterLocations();
    
    bResourceSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Resource Distribution Manager initialized with %d resource nodes"), ResourceNodes.Num());
}

void UWorld_ResourceDistributionManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bResourceSystemInitialized)
        return;
    
    LastResourceUpdate += DeltaTime;
    
    // Update resource regeneration
    if (LastResourceUpdate >= ResourceUpdateInterval)
    {
        RegenerateResources(LastResourceUpdate);
        LastResourceUpdate = 0.0f;
    }
    
    // Update resource LOD based on player location
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        UpdateResourceLOD(PlayerPawn->GetActorLocation());
    }
}

void UWorld_ResourceDistributionManager::GenerateResourceNodes()
{
    ResourceNodes.Empty();
    
    // Define resource types and their base spawn parameters
    TArray<EWorld_ResourceType> ResourceTypes = {
        EWorld_ResourceType::Iron,
        EWorld_ResourceType::Stone,
        EWorld_ResourceType::Flint,
        EWorld_ResourceType::Clay,
        EWorld_ResourceType::Salt,
        EWorld_ResourceType::Obsidian,
        EWorld_ResourceType::Wood,
        EWorld_ResourceType::Plant_Fiber,
        EWorld_ResourceType::Food_Berries,
        EWorld_ResourceType::Food_Nuts,
        EWorld_ResourceType::Food_Roots
    };
    
    // Generate resource nodes across the world
    for (EWorld_ResourceType ResourceType : ResourceTypes)
    {
        int32 NodeCount = FMath::RandRange(20, 50); // Vary node count per resource type
        
        for (int32 i = 0; i < NodeCount; i++)
        {
            FWorld_ResourceNode NewNode;
            NewNode.ResourceType = ResourceType;
            
            // Generate random location within world bounds
            FVector RandomLocation = FVector(
                FMath::RandRange(-5000.0f, 5000.0f),
                FMath::RandRange(-5000.0f, 5000.0f),
                FMath::RandRange(0.0f, 1000.0f)
            );
            
            // Find valid location for this resource type
            NewNode.Location = FindValidResourceLocation(ResourceType, RandomLocation, 1000.0f);
            
            // Set resource properties based on type
            switch (ResourceType)
            {
                case EWorld_ResourceType::Iron:
                    NewNode.Quality = static_cast<EWorld_ResourceQuality>(FMath::RandRange(0, 2));
                    NewNode.MaxYield = FMath::RandRange(50, 200);
                    NewNode.bRequiresTools = true;
                    NewNode.bIsRenewable = false;
                    break;
                    
                case EWorld_ResourceType::Stone:
                    NewNode.Quality = EWorld_ResourceQuality::Common;
                    NewNode.MaxYield = FMath::RandRange(100, 300);
                    NewNode.bRequiresTools = true;
                    NewNode.bIsRenewable = false;
                    break;
                    
                case EWorld_ResourceType::Wood:
                    NewNode.Quality = EWorld_ResourceQuality::Good;
                    NewNode.MaxYield = FMath::RandRange(20, 80);
                    NewNode.bRequiresTools = true;
                    NewNode.bIsRenewable = true;
                    NewNode.RegenerationRate = 0.1f; // Slow regeneration
                    break;
                    
                case EWorld_ResourceType::Food_Berries:
                    NewNode.Quality = static_cast<EWorld_ResourceQuality>(FMath::RandRange(1, 3));
                    NewNode.MaxYield = FMath::RandRange(10, 30);
                    NewNode.bRequiresTools = false;
                    NewNode.bIsRenewable = true;
                    NewNode.RegenerationRate = 2.0f; // Fast regeneration
                    break;
                    
                default:
                    NewNode.Quality = EWorld_ResourceQuality::Common;
                    NewNode.MaxYield = FMath::RandRange(30, 100);
                    NewNode.bRequiresTools = false;
                    NewNode.bIsRenewable = true;
                    break;
            }
            
            NewNode.CurrentYield = NewNode.MaxYield;
            NewNode.Radius = FMath::RandRange(200.0f, 800.0f);
            
            ResourceNodes.Add(NewNode);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Generated %d resource nodes"), ResourceNodes.Num());
}

void UWorld_ResourceDistributionManager::GenerateBiomeResources(EBiomeType BiomeType, FVector CenterLocation, float Radius)
{
    FWorld_BiomeResourceProfile BiomeProfile = GetBiomeResourceProfile(BiomeType);
    
    // Generate primary resources (higher density)
    for (EWorld_ResourceType ResourceType : BiomeProfile.PrimaryResources)
    {
        int32 NodeCount = static_cast<int32>(static_cast<float>(BiomeProfile.ResourceDensity) * 3.0f + 2.0f);
        CreateResourceCluster(ResourceType, CenterLocation, NodeCount);
    }
    
    // Generate secondary resources (medium density)
    for (EWorld_ResourceType ResourceType : BiomeProfile.SecondaryResources)
    {
        int32 NodeCount = static_cast<int32>(static_cast<float>(BiomeProfile.ResourceDensity) * 2.0f + 1.0f);
        CreateResourceCluster(ResourceType, CenterLocation, NodeCount);
    }
    
    // Generate rare resources (low density)
    for (EWorld_ResourceType ResourceType : BiomeProfile.RareResources)
    {
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f) // 30% chance for rare resources
        {
            CreateResourceCluster(ResourceType, CenterLocation, 1);
        }
    }
}

TArray<FWorld_ResourceNode> UWorld_ResourceDistributionManager::GetResourcesInRadius(FVector Location, float Radius)
{
    TArray<FWorld_ResourceNode> NearbyResources;
    
    for (const FWorld_ResourceNode& Node : ResourceNodes)
    {
        float Distance = FVector::Dist(Location, Node.Location);
        if (Distance <= Radius)
        {
            NearbyResources.Add(Node);
        }
    }
    
    return NearbyResources;
}

TArray<FWorld_ResourceNode> UWorld_ResourceDistributionManager::GetResourcesByType(EWorld_ResourceType ResourceType)
{
    TArray<FWorld_ResourceNode> TypedResources;
    
    for (const FWorld_ResourceNode& Node : ResourceNodes)
    {
        if (Node.ResourceType == ResourceType)
        {
            TypedResources.Add(Node);
        }
    }
    
    return TypedResources;
}

bool UWorld_ResourceDistributionManager::HarvestResource(int32 NodeIndex, int32 Amount)
{
    if (!ResourceNodes.IsValidIndex(NodeIndex))
        return false;
    
    FWorld_ResourceNode& Node = ResourceNodes[NodeIndex];
    
    if (Node.CurrentYield < Amount)
        return false;
    
    Node.CurrentYield -= Amount;
    
    UE_LOG(LogTemp, Log, TEXT("Harvested %d resources from node %d. Remaining: %d"), Amount, NodeIndex, Node.CurrentYield);
    
    return true;
}

void UWorld_ResourceDistributionManager::RegenerateResources(float DeltaTime)
{
    for (FWorld_ResourceNode& Node : ResourceNodes)
    {
        if (Node.bIsRenewable && Node.CurrentYield < Node.MaxYield)
        {
            float RegenAmount = Node.RegenerationRate * DeltaTime;
            Node.CurrentYield = FMath::Min(Node.CurrentYield + static_cast<int32>(RegenAmount), Node.MaxYield);
        }
    }
}

int32 UWorld_ResourceDistributionManager::GetNearestResourceNode(FVector Location, EWorld_ResourceType ResourceType)
{
    int32 NearestIndex = -1;
    float NearestDistance = MAX_FLT;
    
    for (int32 i = 0; i < ResourceNodes.Num(); i++)
    {
        const FWorld_ResourceNode& Node = ResourceNodes[i];
        
        if (Node.ResourceType == ResourceType && Node.CurrentYield > 0)
        {
            float Distance = FVector::Dist(Location, Node.Location);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestIndex = i;
            }
        }
    }
    
    return NearestIndex;
}

void UWorld_ResourceDistributionManager::GenerateWaterSources()
{
    WaterSources.Empty();
    
    for (int32 i = 0; i < MaxWaterSources; i++)
    {
        FVector WaterLocation = FVector(
            FMath::RandRange(-4000.0f, 4000.0f),
            FMath::RandRange(-4000.0f, 4000.0f),
            FMath::RandRange(0.0f, 500.0f)
        );
        
        WaterSources.Add(WaterLocation);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Generated %d water sources"), WaterSources.Num());
}

TArray<FVector> UWorld_ResourceDistributionManager::GetNearbyWaterSources(FVector Location, float SearchRadius)
{
    TArray<FVector> NearbyWater;
    
    for (const FVector& WaterSource : WaterSources)
    {
        float Distance = FVector::Dist(Location, WaterSource);
        if (Distance <= SearchRadius)
        {
            NearbyWater.Add(WaterSource);
        }
    }
    
    return NearbyWater;
}

void UWorld_ResourceDistributionManager::GenerateShelterLocations()
{
    ShelterLocations.Empty();
    
    for (int32 i = 0; i < MaxShelterLocations; i++)
    {
        FVector ShelterLocation = FVector(
            FMath::RandRange(-3000.0f, 3000.0f),
            FMath::RandRange(-3000.0f, 3000.0f),
            FMath::RandRange(100.0f, 800.0f)
        );
        
        ShelterLocations.Add(ShelterLocation);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Generated %d shelter locations"), ShelterLocations.Num());
}

TArray<FVector> UWorld_ResourceDistributionManager::GetNearbyShelters(FVector Location, float SearchRadius)
{
    TArray<FVector> NearbyShelters;
    
    for (const FVector& Shelter : ShelterLocations)
    {
        float Distance = FVector::Dist(Location, Shelter);
        if (Distance <= SearchRadius)
        {
            NearbyShelters.Add(Shelter);
        }
    }
    
    return NearbyShelters;
}

void UWorld_ResourceDistributionManager::SetupBiomeResourceProfiles()
{
    InitializeDefaultBiomeProfiles();
}

FWorld_BiomeResourceProfile UWorld_ResourceDistributionManager::GetBiomeResourceProfile(EBiomeType BiomeType)
{
    for (const FWorld_BiomeResourceProfile& Profile : BiomeResourceProfiles)
    {
        if (Profile.BiomeType == BiomeType)
        {
            return Profile;
        }
    }
    
    // Return default profile if not found
    FWorld_BiomeResourceProfile DefaultProfile;
    DefaultProfile.BiomeType = BiomeType;
    return DefaultProfile;
}

void UWorld_ResourceDistributionManager::OptimizeResourceDistribution(int32 MaxActiveNodes)
{
    MaxActiveResourceNodes = MaxActiveNodes;
    
    // Disable distant resource nodes to improve performance
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        UpdateResourceLOD(PlayerPawn->GetActorLocation());
    }
}

void UWorld_ResourceDistributionManager::UpdateResourceLOD(FVector PlayerLocation)
{
    ActiveResourceNodes = 0;
    
    // Sort resources by distance and activate only the nearest ones
    TArray<TPair<float, int32>> ResourceDistances;
    
    for (int32 i = 0; i < ResourceNodes.Num(); i++)
    {
        float Distance = FVector::Dist(PlayerLocation, ResourceNodes[i].Location);
        ResourceDistances.Add(TPair<float, int32>(Distance, i));
    }
    
    // Sort by distance
    ResourceDistances.Sort([](const TPair<float, int32>& A, const TPair<float, int32>& B) {
        return A.Key < B.Key;
    });
    
    // Activate nearest resources up to the limit
    for (int32 i = 0; i < FMath::Min(ResourceDistances.Num(), MaxActiveResourceNodes); i++)
    {
        int32 ResourceIndex = ResourceDistances[i].Value;
        if (ResourceDistances[i].Key <= MaxResourceDistance)
        {
            ActiveResourceNodes++;
        }
    }
}

void UWorld_ResourceDistributionManager::InitializeDefaultBiomeProfiles()
{
    BiomeResourceProfiles.Empty();
    
    // Temperate Forest Profile
    FWorld_BiomeResourceProfile ForestProfile;
    ForestProfile.BiomeType = EBiomeType::Temperate_Forest;
    ForestProfile.PrimaryResources = {EWorld_ResourceType::Wood, EWorld_ResourceType::Food_Berries, EWorld_ResourceType::Plant_Fiber};
    ForestProfile.SecondaryResources = {EWorld_ResourceType::Stone, EWorld_ResourceType::Food_Nuts};
    ForestProfile.RareResources = {EWorld_ResourceType::Iron};
    ForestProfile.ResourceDensity = EWorld_ResourceDensity::High;
    ForestProfile.WaterAvailability = 0.8f;
    ForestProfile.ShelterAvailability = 0.7f;
    BiomeResourceProfiles.Add(ForestProfile);
    
    // Desert Profile
    FWorld_BiomeResourceProfile DesertProfile;
    DesertProfile.BiomeType = EBiomeType::Desert;
    DesertProfile.PrimaryResources = {EWorld_ResourceType::Stone, EWorld_ResourceType::Salt};
    DesertProfile.SecondaryResources = {EWorld_ResourceType::Clay, EWorld_ResourceType::Flint};
    DesertProfile.RareResources = {EWorld_ResourceType::Obsidian};
    DesertProfile.ResourceDensity = EWorld_ResourceDensity::Low;
    DesertProfile.WaterAvailability = 0.1f;
    DesertProfile.ShelterAvailability = 0.2f;
    BiomeResourceProfiles.Add(DesertProfile);
    
    // Mountain Profile
    FWorld_BiomeResourceProfile MountainProfile;
    MountainProfile.BiomeType = EBiomeType::Mountain;
    MountainProfile.PrimaryResources = {EWorld_ResourceType::Stone, EWorld_ResourceType::Iron};
    MountainProfile.SecondaryResources = {EWorld_ResourceType::Flint, EWorld_ResourceType::Obsidian};
    MountainProfile.RareResources = {EWorld_ResourceType::Salt};
    MountainProfile.ResourceDensity = EWorld_ResourceDensity::Medium;
    MountainProfile.WaterAvailability = 0.4f;
    MountainProfile.ShelterAvailability = 0.6f;
    BiomeResourceProfiles.Add(MountainProfile);
}

FVector UWorld_ResourceDistributionManager::FindValidResourceLocation(EWorld_ResourceType ResourceType, FVector PreferredLocation, float SearchRadius)
{
    // Try the preferred location first
    if (IsLocationValidForResource(PreferredLocation, ResourceType))
    {
        return PreferredLocation;
    }
    
    // Search for valid location in expanding radius
    for (int32 Attempts = 0; Attempts < 10; Attempts++)
    {
        FVector TestLocation = PreferredLocation + FVector(
            FMath::RandRange(-SearchRadius, SearchRadius),
            FMath::RandRange(-SearchRadius, SearchRadius),
            0.0f
        );
        
        if (IsLocationValidForResource(TestLocation, ResourceType))
        {
            return TestLocation;
        }
    }
    
    // Return preferred location if no valid location found
    return PreferredLocation;
}

bool UWorld_ResourceDistributionManager::IsLocationValidForResource(FVector Location, EWorld_ResourceType ResourceType)
{
    // Check minimum distance to other resources
    for (const FWorld_ResourceNode& ExistingNode : ResourceNodes)
    {
        float Distance = FVector::Dist(Location, ExistingNode.Location);
        if (Distance < MinResourceDistance)
        {
            return false;
        }
    }
    
    // Additional validation based on resource type could be added here
    return true;
}

void UWorld_ResourceDistributionManager::CreateResourceCluster(EWorld_ResourceType ResourceType, FVector CenterLocation, int32 NodeCount)
{
    for (int32 i = 0; i < NodeCount; i++)
    {
        FWorld_ResourceNode ClusterNode;
        ClusterNode.ResourceType = ResourceType;
        
        // Generate location within cluster radius
        FVector ClusterOffset = FVector(
            FMath::RandRange(-ResourceClusterRadius, ResourceClusterRadius),
            FMath::RandRange(-ResourceClusterRadius, ResourceClusterRadius),
            FMath::RandRange(-100.0f, 100.0f)
        );
        
        ClusterNode.Location = CenterLocation + ClusterOffset;
        ClusterNode.Quality = static_cast<EWorld_ResourceQuality>(FMath::RandRange(0, 4));
        ClusterNode.MaxYield = FMath::RandRange(20, 150);
        ClusterNode.CurrentYield = ClusterNode.MaxYield;
        ClusterNode.Radius = FMath::RandRange(100.0f, 500.0f);
        
        ResourceNodes.Add(ClusterNode);
    }
}

float UWorld_ResourceDistributionManager::CalculateResourceQualityMultiplier(EWorld_ResourceQuality Quality)
{
    switch (Quality)
    {
        case EWorld_ResourceQuality::Poor:
            return 0.5f;
        case EWorld_ResourceQuality::Common:
            return 1.0f;
        case EWorld_ResourceQuality::Good:
            return 1.5f;
        case EWorld_ResourceQuality::Excellent:
            return 2.0f;
        case EWorld_ResourceQuality::Rare:
            return 3.0f;
        default:
            return 1.0f;
    }
}

void UWorld_ResourceDistributionManager::UpdateResourceVisibility(FVector PlayerLocation)
{
    // Update resource visibility based on distance and performance settings
    for (FWorld_ResourceNode& Node : ResourceNodes)
    {
        float Distance = FVector::Dist(PlayerLocation, Node.Location);
        
        // Resources beyond max distance are considered inactive
        if (Distance > MaxResourceDistance)
        {
            // Mark as inactive for performance
            continue;
        }
    }
}