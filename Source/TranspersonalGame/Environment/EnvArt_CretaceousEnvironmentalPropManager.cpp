#include "EnvArt_CretaceousEnvironmentalPropManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Engine/CollisionProfile.h"

UEnvArt_CretaceousEnvironmentalPropManager::UEnvArt_CretaceousEnvironmentalPropManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    MaxActiveProps = 500;
    CullingDistance = 10000.0f;
    
    InitializePropLibrary();
}

void UEnvArt_CretaceousEnvironmentalPropManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize biome prop mapping
    BiomePropMap.Empty();
    
    for (auto& PropData : FallenLogProps)
    {
        for (EBiomeType BiomeType : PropData.AllowedBiomes)
        {
            if (!BiomePropMap.Contains(BiomeType))
            {
                BiomePropMap.Add(BiomeType, TArray<FEnvArt_CretaceousPropData*>());
            }
            BiomePropMap[BiomeType].Add(&PropData);
        }
    }
    
    for (auto& PropData : RockFormationProps)
    {
        for (EBiomeType BiomeType : PropData.AllowedBiomes)
        {
            if (!BiomePropMap.Contains(BiomeType))
            {
                BiomePropMap.Add(BiomeType, TArray<FEnvArt_CretaceousPropData*>());
            }
            BiomePropMap[BiomeType].Add(&PropData);
        }
    }
    
    for (auto& PropData : AncientTreeStumpProps)
    {
        for (EBiomeType BiomeType : PropData.AllowedBiomes)
        {
            if (!BiomePropMap.Contains(BiomeType))
            {
                BiomePropMap.Add(BiomeType, TArray<FEnvArt_CretaceousPropData*>());
            }
            BiomePropMap[BiomeType].Add(&PropData);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CretaceousEnvironmentalPropManager initialized with %d prop types"), 
           FallenLogProps.Num() + RockFormationProps.Num() + AncientTreeStumpProps.Num());
}

void UEnvArt_CretaceousEnvironmentalPropManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    OptimizePropPerformance();
}

void UEnvArt_CretaceousEnvironmentalPropManager::SpawnPropsInArea(FVector CenterLocation, float Radius, EBiomeType BiomeType)
{
    if (!BiomePropMap.Contains(BiomeType))
    {
        UE_LOG(LogTemp, Warning, TEXT("No props available for biome type"));
        return;
    }
    
    TArray<FEnvArt_CretaceousPropData*>& AvailableProps = BiomePropMap[BiomeType];
    if (AvailableProps.Num() == 0)
    {
        return;
    }
    
    int32 PropsToSpawn = FMath::Min(SpawnSettings.MaxPropsPerArea, MaxActiveProps - SpawnedProps.Num());
    
    for (int32 i = 0; i < PropsToSpawn; i++)
    {
        // Generate random location within radius
        float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
        float RandomDistance = FMath::RandRange(0.0f, Radius);
        
        FVector SpawnLocation = CenterLocation + FVector(
            FMath::Cos(RandomAngle) * RandomDistance,
            FMath::Sin(RandomAngle) * RandomDistance,
            0.0f
        );
        
        // Check if location is valid
        if (!IsValidSpawnLocation(SpawnLocation, SpawnSettings.MinDistanceBetweenProps))
        {
            continue;
        }
        
        // Get ground location
        if (SpawnSettings.bSnapToGround)
        {
            SpawnLocation = GetGroundLocation(SpawnLocation);
        }
        
        // Select random prop
        FEnvArt_CretaceousPropData* SelectedProp = AvailableProps[FMath::RandRange(0, AvailableProps.Num() - 1)];
        
        // Check spawn probability
        if (FMath::RandRange(0.0f, 1.0f) > SelectedProp->SpawnProbability)
        {
            continue;
        }
        
        // Generate rotation
        FRotator SpawnRotation = FRotator::ZeroRotator;
        if (SpawnSettings.bUseRandomRotation)
        {
            SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        }
        
        // Spawn the prop
        AActor* SpawnedProp = SpawnSingleProp(*SelectedProp, SpawnLocation, SpawnRotation);
        if (SpawnedProp)
        {
            SpawnedProps.Add(SpawnedProp);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned %d props in area at %s"), PropsToSpawn, *CenterLocation.ToString());
}

void UEnvArt_CretaceousEnvironmentalPropManager::ClearPropsInArea(FVector CenterLocation, float Radius)
{
    TArray<AActor*> PropsToRemove;
    
    for (AActor* Prop : SpawnedProps)
    {
        if (IsValid(Prop))
        {
            float Distance = FVector::Dist(Prop->GetActorLocation(), CenterLocation);
            if (Distance <= Radius)
            {
                PropsToRemove.Add(Prop);
            }
        }
    }
    
    for (AActor* Prop : PropsToRemove)
    {
        SpawnedProps.Remove(Prop);
        Prop->Destroy();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Cleared %d props in area"), PropsToRemove.Num());
}

void UEnvArt_CretaceousEnvironmentalPropManager::UpdatePropLOD(float PlayerDistance)
{
    for (AActor* Prop : SpawnedProps)
    {
        if (IsValid(Prop))
        {
            UStaticMeshComponent* MeshComp = Prop->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp)
            {
                if (PlayerDistance > CullingDistance)
                {
                    MeshComp->SetVisibility(false);
                }
                else
                {
                    MeshComp->SetVisibility(true);
                    
                    // Simple LOD based on distance
                    if (PlayerDistance > CullingDistance * 0.5f)
                    {
                        MeshComp->SetForcedLodModel(2);
                    }
                    else if (PlayerDistance > CullingDistance * 0.25f)
                    {
                        MeshComp->SetForcedLodModel(1);
                    }
                    else
                    {
                        MeshComp->SetForcedLodModel(0);
                    }
                }
            }
        }
    }
}

TArray<AActor*> UEnvArt_CretaceousEnvironmentalPropManager::GetActiveProps() const
{
    TArray<AActor*> ValidProps;
    
    for (AActor* Prop : SpawnedProps)
    {
        if (IsValid(Prop))
        {
            ValidProps.Add(Prop);
        }
    }
    
    return ValidProps;
}

void UEnvArt_CretaceousEnvironmentalPropManager::GenerateDefaultPropLibrary()
{
    // Clear existing props
    FallenLogProps.Empty();
    RockFormationProps.Empty();
    AncientTreeStumpProps.Empty();
    CretaceousFernProps.Empty();
    
    // Generate fallen log props
    FEnvArt_CretaceousPropData FallenLog1;
    FallenLog1.PropName = TEXT("Large Fallen Cretaceous Log");
    FallenLog1.MinScale = FVector(0.8f, 0.8f, 0.8f);
    FallenLog1.MaxScale = FVector(1.5f, 1.5f, 1.2f);
    FallenLog1.SpawnProbability = 0.3f;
    FallenLog1.AllowedBiomes = {EBiomeType::Forest, EBiomeType::Swamp};
    FallenLogProps.Add(FallenLog1);
    
    FEnvArt_CretaceousPropData FallenLog2;
    FallenLog2.PropName = TEXT("Mossy Fallen Branch");
    FallenLog2.MinScale = FVector(0.5f, 0.5f, 0.5f);
    FallenLog2.MaxScale = FVector(1.0f, 1.0f, 1.0f);
    FallenLog2.SpawnProbability = 0.6f;
    FallenLog2.AllowedBiomes = {EBiomeType::Forest, EBiomeType::Swamp};
    FallenLogProps.Add(FallenLog2);
    
    // Generate rock formation props
    FEnvArt_CretaceousPropData Rock1;
    Rock1.PropName = TEXT("Weathered Sandstone Boulder");
    Rock1.MinScale = FVector(0.7f, 0.7f, 0.7f);
    Rock1.MaxScale = FVector(1.8f, 1.8f, 1.5f);
    Rock1.SpawnProbability = 0.4f;
    Rock1.AllowedBiomes = {EBiomeType::Desert, EBiomeType::Plains, EBiomeType::Mountain};
    RockFormationProps.Add(Rock1);
    
    FEnvArt_CretaceousPropData Rock2;
    Rock2.PropName = TEXT("Limestone Outcrop");
    Rock2.MinScale = FVector(1.0f, 1.0f, 0.8f);
    Rock2.MaxScale = FVector(2.5f, 2.5f, 1.8f);
    Rock2.SpawnProbability = 0.2f;
    Rock2.AllowedBiomes = {EBiomeType::Desert, EBiomeType::Mountain};
    RockFormationProps.Add(Rock2);
    
    // Generate ancient tree stump props
    FEnvArt_CretaceousPropData Stump1;
    Stump1.PropName = TEXT("Ancient Conifer Stump");
    Stump1.MinScale = FVector(0.8f, 0.8f, 0.6f);
    Stump1.MaxScale = FVector(1.5f, 1.5f, 1.2f);
    Stump1.SpawnProbability = 0.25f;
    Stump1.AllowedBiomes = {EBiomeType::Forest};
    AncientTreeStumpProps.Add(Stump1);
    
    // Generate fern props
    FEnvArt_CretaceousPropData Fern1;
    Fern1.PropName = TEXT("Large Cretaceous Fern Cluster");
    Fern1.MinScale = FVector(0.8f, 0.8f, 0.8f);
    Fern1.MaxScale = FVector(1.3f, 1.3f, 1.3f);
    Fern1.SpawnProbability = 0.7f;
    Fern1.AllowedBiomes = {EBiomeType::Forest, EBiomeType::Swamp};
    CretaceousFernProps.Add(Fern1);
    
    UE_LOG(LogTemp, Warning, TEXT("Generated default prop library with %d total props"), 
           FallenLogProps.Num() + RockFormationProps.Num() + AncientTreeStumpProps.Num() + CretaceousFernProps.Num());
}

void UEnvArt_CretaceousEnvironmentalPropManager::InitializePropLibrary()
{
    // Initialize with default props if empty
    if (FallenLogProps.Num() == 0 && RockFormationProps.Num() == 0 && AncientTreeStumpProps.Num() == 0)
    {
        GenerateDefaultPropLibrary();
    }
}

AActor* UEnvArt_CretaceousEnvironmentalPropManager::SpawnSingleProp(const FEnvArt_CretaceousPropData& PropData, FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Create static mesh actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* PropActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation, SpawnParams);
    if (!PropActor)
    {
        return nullptr;
    }
    
    // Set random scale
    FVector RandomScale = FVector(
        FMath::RandRange(PropData.MinScale.X, PropData.MaxScale.X),
        FMath::RandRange(PropData.MinScale.Y, PropData.MaxScale.Y),
        FMath::RandRange(PropData.MinScale.Z, PropData.MaxScale.Z)
    );
    PropActor->SetActorScale3D(RandomScale);
    
    // Set actor label for debugging
    PropActor->SetActorLabel(PropData.PropName);
    
    // Configure static mesh component
    UStaticMeshComponent* MeshComp = PropActor->GetStaticMeshComponent();
    if (MeshComp)
    {
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        MeshComp->SetCanEverAffectNavigation(true);
    }
    
    return PropActor;
}

bool UEnvArt_CretaceousEnvironmentalPropManager::IsValidSpawnLocation(FVector Location, float MinDistance)
{
    // Check distance from existing props
    for (AActor* ExistingProp : SpawnedProps)
    {
        if (IsValid(ExistingProp))
        {
            float Distance = FVector::Dist(ExistingProp->GetActorLocation(), Location);
            if (Distance < MinDistance)
            {
                return false;
            }
        }
    }
    
    return true;
}

FVector UEnvArt_CretaceousEnvironmentalPropManager::GetGroundLocation(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return Location;
    }
    
    FVector StartTrace = Location + FVector(0.0f, 0.0f, 1000.0f);
    FVector EndTrace = Location - FVector(0.0f, 0.0f, 1000.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Location;
    }
    
    return Location;
}

void UEnvArt_CretaceousEnvironmentalPropManager::OptimizePropPerformance()
{
    // Remove invalid props
    SpawnedProps.RemoveAll([](AActor* Prop) {
        return !IsValid(Prop);
    });
    
    // Enforce max prop limit
    if (SpawnedProps.Num() > MaxActiveProps)
    {
        int32 PropsToRemove = SpawnedProps.Num() - MaxActiveProps;
        for (int32 i = 0; i < PropsToRemove; i++)
        {
            if (SpawnedProps.Num() > 0)
            {
                AActor* PropToRemove = SpawnedProps[0];
                SpawnedProps.RemoveAt(0);
                if (IsValid(PropToRemove))
                {
                    PropToRemove->Destroy();
                }
            }
        }
    }
}