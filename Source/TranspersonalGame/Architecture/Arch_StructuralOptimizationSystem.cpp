#include "Arch_StructuralOptimizationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UArch_StructuralOptimizationComponent::UArch_StructuralOptimizationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default structural metrics
    StructuralMetrics.LoadCapacity = 1000.0f;
    StructuralMetrics.CurrentStress = 0.0f;
    StructuralMetrics.WeatheringFactor = 0.0f;
    StructuralMetrics.StructuralIntegrity = 100.0f;
    StructuralMetrics.bRequiresReinforcement = false;

    // Initialize optimization parameters
    OptimizationParams.PerformanceBudget = 16.67f;
    OptimizationParams.MaxVisibleStructures = 50;
    OptimizationParams.CullingDistance = 5000.0f;
    OptimizationParams.bEnableLODOptimization = true;
    OptimizationParams.LODDistanceMultiplier = 1.5f;
}

void UArch_StructuralOptimizationComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize managed structures array
    ManagedStructures.Empty();

    // Find nearby structures to manage
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), FoundActors);

        for (AActor* Actor : FoundActors)
        {
            if (Actor && FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation()) <= OptimizationParams.CullingDistance)
            {
                ManagedStructures.Add(Actor);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("StructuralOptimizationComponent initialized with %d managed structures"), ManagedStructures.Num());
}

void UArch_StructuralOptimizationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AccumulatedTime += DeltaTime;

    if (AccumulatedTime >= OptimizationInterval)
    {
        UpdateStructuralMetrics();
        
        if (OptimizationParams.bEnableLODOptimization)
        {
            OptimizePerformance();
        }

        ApplyWeatheringEffects(AccumulatedTime);
        AccumulatedTime = 0.0f;
    }
}

void UArch_StructuralOptimizationComponent::AnalyzeStructuralIntegrity()
{
    float TotalStress = 0.0f;
    int32 ValidStructures = 0;

    for (AActor* Structure : ManagedStructures)
    {
        if (IsValid(Structure))
        {
            float StructureStress = CalculateStressDistribution(Structure);
            TotalStress += StructureStress;
            ValidStructures++;
        }
    }

    if (ValidStructures > 0)
    {
        StructuralMetrics.CurrentStress = TotalStress / ValidStructures;
        StructuralMetrics.bRequiresReinforcement = StructuralMetrics.CurrentStress > (StructuralMetrics.LoadCapacity * 0.8f);
    }

    UE_LOG(LogTemp, Log, TEXT("Structural analysis complete. Average stress: %f"), StructuralMetrics.CurrentStress);
}

void UArch_StructuralOptimizationComponent::OptimizePerformance()
{
    CullDistantStructures();
    AdjustLODLevels();
    UpdateLODSettings();
}

void UArch_StructuralOptimizationComponent::UpdateLODSettings()
{
    if (!GetOwner()) return;

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    for (AActor* Structure : ManagedStructures)
    {
        if (!IsValid(Structure)) continue;

        float Distance = FVector::Dist(OwnerLocation, Structure->GetActorLocation());
        
        UStaticMeshComponent* MeshComp = Structure->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp)
        {
            // Calculate LOD level based on distance
            int32 LODLevel = 0;
            if (Distance > 1000.0f * OptimizationParams.LODDistanceMultiplier)
            {
                LODLevel = 2;
            }
            else if (Distance > 500.0f * OptimizationParams.LODDistanceMultiplier)
            {
                LODLevel = 1;
            }

            MeshComp->SetForcedLodModel(LODLevel + 1);
        }
    }
}

float UArch_StructuralOptimizationComponent::CalculateStressDistribution(AActor* Structure)
{
    if (!IsValid(Structure)) return 0.0f;

    // Simplified stress calculation based on structure properties
    float BaseStress = 100.0f;
    float WeatherMultiplier = 1.0f + (StructuralMetrics.WeatheringFactor * 0.1f);
    float HeightFactor = Structure->GetActorLocation().Z / 1000.0f;

    return BaseStress * WeatherMultiplier * (1.0f + HeightFactor);
}

bool UArch_StructuralOptimizationComponent::ValidateStructuralStability(AActor* Structure)
{
    if (!IsValid(Structure)) return false;

    float StructureStress = CalculateStressDistribution(Structure);
    return StructureStress <= StructuralMetrics.LoadCapacity;
}

void UArch_StructuralOptimizationComponent::ApplyWeatheringEffects(float DeltaTime)
{
    // Gradual weathering over time
    StructuralMetrics.WeatheringFactor += DeltaTime * 0.001f; // Very slow weathering
    StructuralMetrics.StructuralIntegrity = FMath::Max(0.0f, 100.0f - (StructuralMetrics.WeatheringFactor * 10.0f));

    // Apply visual effects based on weathering
    for (AActor* Structure : ManagedStructures)
    {
        if (!IsValid(Structure)) continue;

        UStaticMeshComponent* MeshComp = Structure->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp && StructuralMetrics.WeatheringFactor > 5.0f)
        {
            // Could apply weathered materials here
            // MeshComp->SetMaterial(0, WeatheredMaterial);
        }
    }
}

void UArch_StructuralOptimizationComponent::UpdateStructuralMetrics()
{
    AnalyzeStructuralIntegrity();

    // Remove invalid structures
    ManagedStructures.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
}

void UArch_StructuralOptimizationComponent::CullDistantStructures()
{
    if (!GetOwner()) return;

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    for (AActor* Structure : ManagedStructures)
    {
        if (!IsValid(Structure)) continue;

        float Distance = FVector::Dist(OwnerLocation, Structure->GetActorLocation());
        bool bShouldBeVisible = Distance <= OptimizationParams.CullingDistance;

        Structure->SetActorHiddenInGame(!bShouldBeVisible);
    }
}

void UArch_StructuralOptimizationComponent::AdjustLODLevels()
{
    // LOD adjustment is handled in UpdateLODSettings
    UpdateLODSettings();
}

// Manager Implementation
AArch_StructuralOptimizationManager::AArch_StructuralOptimizationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f;

    OptimizationComponent = CreateDefaultSubobject<UArch_StructuralOptimizationComponent>(TEXT("OptimizationComponent"));

    GlobalPerformanceBudget = 16.67f;
    MaxActiveStructures = 100;
    bEnableGlobalOptimization = true;
}

void AArch_StructuralOptimizationManager::BeginPlay()
{
    Super::BeginPlay();

    RegisteredStructures.Empty();
    LastOptimizationTime = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("StructuralOptimizationManager initialized"));
}

void AArch_StructuralOptimizationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastOptimizationTime += DeltaTime;

    if (bEnableGlobalOptimization && LastOptimizationTime >= OptimizationFrequency)
    {
        PerformGlobalOptimization();
        LastOptimizationTime = 0.0f;
    }
}

void AArch_StructuralOptimizationManager::RegisterStructure(AActor* Structure)
{
    if (IsValid(Structure) && !RegisteredStructures.Contains(Structure))
    {
        RegisteredStructures.Add(Structure);
        UE_LOG(LogTemp, Log, TEXT("Registered structure: %s"), *Structure->GetName());
    }
}

void AArch_StructuralOptimizationManager::UnregisterStructure(AActor* Structure)
{
    if (RegisteredStructures.Contains(Structure))
    {
        RegisteredStructures.Remove(Structure);
        UE_LOG(LogTemp, Log, TEXT("Unregistered structure: %s"), *Structure->GetName());
    }
}

void AArch_StructuralOptimizationManager::OptimizeAllStructures()
{
    PerformGlobalOptimization();
}

FArch_StructuralMetrics AArch_StructuralOptimizationManager::GetGlobalStructuralHealth()
{
    FArch_StructuralMetrics GlobalMetrics;
    
    if (OptimizationComponent)
    {
        GlobalMetrics = OptimizationComponent->StructuralMetrics;
    }

    return GlobalMetrics;
}

void AArch_StructuralOptimizationManager::PerformGlobalOptimization()
{
    UpdateGlobalMetrics();
    ManageStructureVisibility();

    if (OptimizationComponent)
    {
        OptimizationComponent->OptimizePerformance();
    }

    UE_LOG(LogTemp, Log, TEXT("Global optimization performed on %d structures"), RegisteredStructures.Num());
}

void AArch_StructuralOptimizationManager::UpdateGlobalMetrics()
{
    // Clean up invalid structures
    RegisteredStructures.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });

    // Update optimization component with current structures
    if (OptimizationComponent)
    {
        OptimizationComponent->ManagedStructures = RegisteredStructures;
    }
}

void AArch_StructuralOptimizationManager::ManageStructureVisibility()
{
    // Ensure we don't exceed the maximum active structures
    int32 VisibleCount = 0;
    
    for (AActor* Structure : RegisteredStructures)
    {
        if (!IsValid(Structure)) continue;

        if (VisibleCount < MaxActiveStructures)
        {
            Structure->SetActorHiddenInGame(false);
            VisibleCount++;
        }
        else
        {
            Structure->SetActorHiddenInGame(true);
        }
    }
}