#include "EnvArt_CretaceousTreeSystem.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

UEnvArt_CretaceousTreeSystem::UEnvArt_CretaceousTreeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    // Initialize default tree species data
    FEnvArt_CretaceousTreeData AraucariaData;
    AraucariaData.BiomeType = EBiomeType::Forest;
    AraucariaData.TreeHeight = 25.0f;
    AraucariaData.TrunkDiameter = 3.0f;
    AraucariaData.CanopyRadius = 12.0f;
    AraucariaData.Age = 80.0f;
    TreeSpeciesData.Add(EEnvArt_CretaceousTreeSpecies::Araucaria, AraucariaData);
    
    FEnvArt_CretaceousTreeData GinkgoData;
    GinkgoData.BiomeType = EBiomeType::Forest;
    GinkgoData.TreeHeight = 18.0f;
    GinkgoData.TrunkDiameter = 2.5f;
    GinkgoData.CanopyRadius = 10.0f;
    GinkgoData.Age = 60.0f;
    TreeSpeciesData.Add(EEnvArt_CretaceousTreeSpecies::Ginkgo, GinkgoData);
    
    FEnvArt_CretaceousTreeData CycadData;
    CycadData.BiomeType = EBiomeType::Tropical;
    CycadData.TreeHeight = 8.0f;
    CycadData.TrunkDiameter = 1.5f;
    CycadData.CanopyRadius = 6.0f;
    CycadData.Age = 30.0f;
    TreeSpeciesData.Add(EEnvArt_CretaceousTreeSpecies::Cycad, CycadData);
    
    FEnvArt_CretaceousTreeData FernData;
    FernData.BiomeType = EBiomeType::Swamp;
    FernData.TreeHeight = 12.0f;
    FernData.TrunkDiameter = 1.0f;
    FernData.CanopyRadius = 8.0f;
    FernData.Age = 25.0f;
    TreeSpeciesData.Add(EEnvArt_CretaceousTreeSpecies::Fern, FernData);
    
    FEnvArt_CretaceousTreeData ConiferData;
    ConiferData.BiomeType = EBiomeType::Forest;
    ConiferData.TreeHeight = 30.0f;
    ConiferData.TrunkDiameter = 4.0f;
    ConiferData.CanopyRadius = 15.0f;
    ConiferData.Age = 100.0f;
    TreeSpeciesData.Add(EEnvArt_CretaceousTreeSpecies::Conifer, ConiferData);
    
    FEnvArt_CretaceousTreeData RedwoodData;
    RedwoodData.BiomeType = EBiomeType::Forest;
    RedwoodData.TreeHeight = 50.0f;
    RedwoodData.TrunkDiameter = 6.0f;
    RedwoodData.CanopyRadius = 20.0f;
    RedwoodData.Age = 200.0f;
    TreeSpeciesData.Add(EEnvArt_CretaceousTreeSpecies::Redwood, RedwoodData);
    
    FEnvArt_CretaceousTreeData FallenLogData;
    FallenLogData.BiomeType = EBiomeType::Forest;
    FallenLogData.TreeHeight = 2.0f;
    FallenLogData.TrunkDiameter = 3.0f;
    FallenLogData.CanopyRadius = 0.0f;
    FallenLogData.Age = 150.0f;
    FallenLogData.bIsFallenLog = true;
    FallenLogData.MossGrowth = 0.8f;
    FallenLogData.WeatheringLevel = 0.7f;
    TreeSpeciesData.Add(EEnvArt_CretaceousTreeSpecies::FallenLog, FallenLogData);
}

void UEnvArt_CretaceousTreeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("CretaceousTreeSystem: Initialized for prehistoric forest environments"));
    
    // Initialize default environmental state
    CurrentSeasonProgress = 0.5f; // Mid-season
    CurrentRainIntensity = 0.2f;  // Light rain
    CurrentWindStrength = 0.3f;   // Gentle breeze
}

void UEnvArt_CretaceousTreeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up destroyed actors from tracking arrays
    SpawnedTrees.RemoveAll([](const TWeakObjectPtr<AActor>& Actor) {
        return !Actor.IsValid();
    });
    
    SpawnedLogs.RemoveAll([](const TWeakObjectPtr<AActor>& Actor) {
        return !Actor.IsValid();
    });
    
    ActiveParticleEffects.RemoveAll([](const TWeakObjectPtr<UNiagaraComponent>& Component) {
        return !Component.IsValid();
    });
}

void UEnvArt_CretaceousTreeSystem::SpawnTreeCluster(const FVector& Location, EEnvArt_CretaceousTreeSpecies Species, int32 Count)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("CretaceousTreeSystem: No valid world for spawning trees"));
        return;
    }
    
    Count = FMath::Clamp(Count, 1, MaxTreesPerCluster);
    
    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation = GetRandomSpawnLocation(Location, MaxTreeSpacing * 2.0f);
        
        if (!IsValidTreeLocation(SpawnLocation, MinTreeSpacing))
        {
            continue;
        }
        
        FRotator SpawnRotation = GetTerrainAlignedRotation(SpawnLocation);
        
        // Spawn tree actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AStaticMeshActor* TreeActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
        
        if (TreeActor)
        {
            ApplySpeciesCharacteristics(TreeActor, Species);
            SpawnedTrees.Add(TreeActor);
            
            // Add environmental effects
            if (Species == EEnvArt_CretaceousTreeSpecies::Araucaria || Species == EEnvArt_CretaceousTreeSpecies::Conifer)
            {
                // Spawn pollen particles for conifers
                if (PollenParticles.LoadSynchronous())
                {
                    UNiagaraComponent* PollenComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                        GetWorld(), PollenParticles.Get(), SpawnLocation + FVector(0, 0, 500), FRotator::ZeroRotator);
                    if (PollenComponent)
                    {
                        ActiveParticleEffects.Add(PollenComponent);
                    }
                }
            }
            
            UE_LOG(LogTemp, Log, TEXT("CretaceousTreeSystem: Spawned %s tree at %s"), 
                *UEnum::GetValueAsString(Species), *SpawnLocation.ToString());
        }
    }
}

void UEnvArt_CretaceousTreeSystem::SpawnFallenLog(const FVector& Location, const FRotator& Rotation)
{
    if (!GetWorld())
    {
        return;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* LogActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
    
    if (LogActor)
    {
        ApplySpeciesCharacteristics(LogActor, EEnvArt_CretaceousTreeSpecies::FallenLog);
        SpawnedLogs.Add(LogActor);
        
        // Add moss growth and weathering effects
        AddMossGrowth(LogActor, 0.8f);
        
        UE_LOG(LogTemp, Log, TEXT("CretaceousTreeSystem: Spawned fallen log at %s"), *Location.ToString());
    }
}

void UEnvArt_CretaceousTreeSystem::UpdateTreeSeason(float SeasonProgress)
{
    CurrentSeasonProgress = FMath::Clamp(SeasonProgress, 0.0f, 1.0f);
    
    // Update all spawned trees based on seasonal changes
    for (const TWeakObjectPtr<AActor>& TreePtr : SpawnedTrees)
    {
        if (AActor* TreeActor = TreePtr.Get())
        {
            if (UStaticMeshComponent* MeshComp = TreeActor->FindComponentByClass<UStaticMeshComponent>())
            {
                // Create dynamic material instance for seasonal changes
                if (UMaterialInterface* CurrentMaterial = MeshComp->GetMaterial(0))
                {
                    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(CurrentMaterial, TreeActor);
                    if (DynamicMaterial)
                    {
                        // Adjust foliage color based on season
                        float SeasonalHue = FMath::Lerp(0.3f, 0.1f, CurrentSeasonProgress); // Green to brown
                        DynamicMaterial->SetScalarParameterValue(TEXT("SeasonProgress"), CurrentSeasonProgress);
                        DynamicMaterial->SetScalarParameterValue(TEXT("FoliageHue"), SeasonalHue);
                        MeshComp->SetMaterial(0, DynamicMaterial);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("CretaceousTreeSystem: Updated trees for season progress: %f"), CurrentSeasonProgress);
}

void UEnvArt_CretaceousTreeSystem::ApplyWeatherEffects(float RainIntensity, float WindStrength)
{
    CurrentRainIntensity = FMath::Clamp(RainIntensity, 0.0f, 1.0f);
    CurrentWindStrength = FMath::Clamp(WindStrength, 0.0f, 1.0f);
    
    // Apply weather effects to particle systems
    for (const TWeakObjectPtr<UNiagaraComponent>& EffectPtr : ActiveParticleEffects)
    {
        if (UNiagaraComponent* Effect = EffectPtr.Get())
        {
            Effect->SetFloatParameter(TEXT("WindStrength"), CurrentWindStrength);
            Effect->SetFloatParameter(TEXT("RainIntensity"), CurrentRainIntensity);
        }
    }
    
    // Spawn falling leaves during strong wind
    if (CurrentWindStrength > 0.6f && FallingLeavesParticles.LoadSynchronous())
    {
        for (const TWeakObjectPtr<AActor>& TreePtr : SpawnedTrees)
        {
            if (AActor* TreeActor = TreePtr.Get())
            {
                FVector TreeLocation = TreeActor->GetActorLocation();
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    GetWorld(), FallingLeavesParticles.Get(), TreeLocation + FVector(0, 0, 800), FRotator::ZeroRotator);
            }
        }
    }
}

void UEnvArt_CretaceousTreeSystem::AddMossGrowth(AActor* TreeActor, float GrowthAmount)
{
    if (!TreeActor)
    {
        return;
    }
    
    if (UStaticMeshComponent* MeshComp = TreeActor->FindComponentByClass<UStaticMeshComponent>())
    {
        // Apply mossy bark material if growth is significant
        if (GrowthAmount > 0.5f && MossyBarkMaterial.LoadSynchronous())
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(MossyBarkMaterial.Get(), TreeActor);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetScalarParameterValue(TEXT("MossGrowth"), GrowthAmount);
                DynamicMaterial->SetScalarParameterValue(TEXT("Wetness"), CurrentRainIntensity);
                MeshComp->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

void UEnvArt_CretaceousTreeSystem::CreateBirdNest(AActor* TreeActor, const FVector& NestLocation)
{
    if (!TreeActor || !GetWorld())
    {
        return;
    }
    
    // Spawn a simple nest prop (placeholder for now)
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* NestActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), NestLocation, FRotator::ZeroRotator, SpawnParams);
    
    if (NestActor)
    {
        // Attach nest to tree
        NestActor->AttachToActor(TreeActor, FAttachmentTransformRules::KeepWorldTransform);
        UE_LOG(LogTemp, Log, TEXT("CretaceousTreeSystem: Created bird nest on tree at %s"), *NestLocation.ToString());
    }
}

void UEnvArt_CretaceousTreeSystem::SpawnInsectSwarm(const FVector& Location)
{
    if (!GetWorld() || !InsectSwarmParticles.LoadSynchronous())
    {
        return;
    }
    
    UNiagaraComponent* SwarmComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), InsectSwarmParticles.Get(), Location, FRotator::ZeroRotator);
    
    if (SwarmComponent)
    {
        SwarmComponent->SetFloatParameter(TEXT("SwarmSize"), FMath::RandRange(50.0f, 200.0f));
        SwarmComponent->SetFloatParameter(TEXT("SwarmSpeed"), FMath::RandRange(100.0f, 300.0f));
        ActiveParticleEffects.Add(SwarmComponent);
        
        UE_LOG(LogTemp, Log, TEXT("CretaceousTreeSystem: Spawned insect swarm at %s"), *Location.ToString());
    }
}

FVector UEnvArt_CretaceousTreeSystem::GetRandomSpawnLocation(const FVector& CenterLocation, float Radius)
{
    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    float RandomDistance = FMath::RandRange(0.0f, Radius);
    
    FVector Offset = FVector(
        FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
        FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
        0.0f
    );
    
    return CenterLocation + Offset;
}

FRotator UEnvArt_CretaceousTreeSystem::GetTerrainAlignedRotation(const FVector& Location)
{
    if (!GetWorld())
    {
        return FRotator::ZeroRotator;
    }
    
    // Perform line trace to get terrain normal
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 1000);
    FVector TraceEnd = Location - FVector(0, 0, 1000);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        FVector UpVector = HitResult.Normal;
        FVector ForwardVector = FVector::ForwardVector;
        FVector RightVector = FVector::CrossProduct(UpVector, ForwardVector).GetSafeNormal();
        ForwardVector = FVector::CrossProduct(RightVector, UpVector).GetSafeNormal();
        
        return FRotationMatrix::MakeFromXZ(ForwardVector, UpVector).Rotator();
    }
    
    return FRotator::ZeroRotator;
}

void UEnvArt_CretaceousTreeSystem::ApplySpeciesCharacteristics(AActor* TreeActor, EEnvArt_CretaceousTreeSpecies Species)
{
    if (!TreeActor)
    {
        return;
    }
    
    const FEnvArt_CretaceousTreeData* SpeciesData = TreeSpeciesData.Find(Species);
    if (!SpeciesData)
    {
        return;
    }
    
    // Set actor scale based on species characteristics
    float ScaleVariation = FMath::RandRange(0.8f, 1.2f);
    FVector Scale = FVector(
        SpeciesData->TrunkDiameter * ScaleVariation,
        SpeciesData->TrunkDiameter * ScaleVariation,
        SpeciesData->TreeHeight * ScaleVariation
    );
    TreeActor->SetActorScale3D(Scale);
    
    // Apply appropriate material
    if (UStaticMeshComponent* MeshComp = TreeActor->FindComponentByClass<UStaticMeshComponent>())
    {
        if (SpeciesData->bIsFallenLog)
        {
            UpdateTreeMaterial(TreeActor, SpeciesData->MossGrowth, SpeciesData->WeatheringLevel);
        }
        else if (BarkMaterial.LoadSynchronous())
        {
            MeshComp->SetMaterial(0, BarkMaterial.Get());
        }
    }
    
    // Set actor name for identification
    FString ActorName = FString::Printf(TEXT("%s_%s"), 
        *UEnum::GetValueAsString(Species), 
        *FDateTime::Now().ToString(TEXT("%H%M%S")));
    TreeActor->SetActorLabel(ActorName);
}

void UEnvArt_CretaceousTreeSystem::UpdateTreeMaterial(AActor* TreeActor, float MossGrowth, float Weathering)
{
    if (!TreeActor)
    {
        return;
    }
    
    if (UStaticMeshComponent* MeshComp = TreeActor->FindComponentByClass<UStaticMeshComponent>())
    {
        UMaterialInterface* BaseMaterial = (MossGrowth > 0.3f) ? MossyBarkMaterial.Get() : BarkMaterial.Get();
        
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, TreeActor);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetScalarParameterValue(TEXT("MossGrowth"), MossGrowth);
                DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), Weathering);
                DynamicMaterial->SetScalarParameterValue(TEXT("Age"), FMath::RandRange(0.3f, 1.0f));
                MeshComp->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

bool UEnvArt_CretaceousTreeSystem::IsValidTreeLocation(const FVector& Location, float MinDistance)
{
    // Check distance from existing trees
    for (const TWeakObjectPtr<AActor>& TreePtr : SpawnedTrees)
    {
        if (AActor* ExistingTree = TreePtr.Get())
        {
            float Distance = FVector::Dist(Location, ExistingTree->GetActorLocation());
            if (Distance < MinDistance)
            {
                return false;
            }
        }
    }
    
    // Check distance from existing logs
    for (const TWeakObjectPtr<AActor>& LogPtr : SpawnedLogs)
    {
        if (AActor* ExistingLog = LogPtr.Get())
        {
            float Distance = FVector::Dist(Location, ExistingLog->GetActorLocation());
            if (Distance < MinDistance * 0.5f)
            {
                return false;
            }
        }
    }
    
    return true;
}