#include "Eng_SystemIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UEng_SystemIntegrator::UEng_SystemIntegrator()
{
    bSystemsInitialized = false;
}

void UEng_SystemIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Register core system layers
    RegisterSystemLayer(TEXT("Foundation"), 100);
    RegisterSystemLayer(TEXT("Core"), 200);
    RegisterSystemLayer(TEXT("Gameplay"), 300);
    RegisterSystemLayer(TEXT("Presentation"), 400);
    
    // Register core systems to layers
    RegisterSystemToLayer(TEXT("BiomeManager"), TEXT("Foundation"));
    RegisterSystemToLayer(TEXT("WorldGenerator"), TEXT("Foundation"));
    RegisterSystemToLayer(TEXT("PhysicsCore"), TEXT("Core"));
    RegisterSystemToLayer(TEXT("CharacterSystem"), TEXT("Gameplay"));
    RegisterSystemToLayer(TEXT("DinosaurAI"), TEXT("Gameplay"));
    RegisterSystemToLayer(TEXT("AudioSystem"), TEXT("Presentation"));
    RegisterSystemToLayer(TEXT("VFXSystem"), TEXT("Presentation"));
    
    // Set up dependencies
    AddSystemDependency(TEXT("WorldGenerator"), TEXT("BiomeManager"));
    AddSystemDependency(TEXT("CharacterSystem"), TEXT("PhysicsCore"));
    AddSystemDependency(TEXT("DinosaurAI"), TEXT("PhysicsCore"));
    AddSystemDependency(TEXT("AudioSystem"), TEXT("CharacterSystem"), true);
    AddSystemDependency(TEXT("VFXSystem"), TEXT("DinosaurAI"), true);
    
    UE_LOG(LogTemp, Warning, TEXT("SystemIntegrator initialized with %d layers"), SystemLayers.Num());
}

void UEng_SystemIntegrator::Deinitialize()
{
    ShutdownSystemsInReverseOrder();
    Super::Deinitialize();
}

void UEng_SystemIntegrator::RegisterSystemLayer(const FString& LayerName, int32 Priority)
{
    FEng_SystemLayer NewLayer;
    NewLayer.LayerName = LayerName;
    NewLayer.Priority = Priority;
    NewLayer.bIsActive = true;
    
    SystemLayers.Add(NewLayer);
    SortLayersByPriority();
    
    UE_LOG(LogTemp, Log, TEXT("Registered system layer: %s (Priority: %d)"), *LayerName, Priority);
}

void UEng_SystemIntegrator::RegisterSystemToLayer(const FString& SystemName, const FString& LayerName)
{
    for (FEng_SystemLayer& Layer : SystemLayers)
    {
        if (Layer.LayerName == LayerName)
        {
            Layer.SystemNames.AddUnique(SystemName);
            UE_LOG(LogTemp, Log, TEXT("Registered system %s to layer %s"), *SystemName, *LayerName);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Layer %s not found for system %s"), *LayerName, *SystemName);
}

void UEng_SystemIntegrator::AddSystemDependency(const FString& SystemName, const FString& DependencyName, bool bOptional)
{
    FEng_SystemDependency* ExistingDep = SystemDependencies.FindByPredicate([&SystemName](const FEng_SystemDependency& Dep)
    {
        return Dep.SystemName == SystemName;
    });
    
    if (ExistingDep)
    {
        ExistingDep->Dependencies.AddUnique(DependencyName);
    }
    else
    {
        FEng_SystemDependency NewDep;
        NewDep.SystemName = SystemName;
        NewDep.Dependencies.Add(DependencyName);
        NewDep.bIsOptional = bOptional;
        SystemDependencies.Add(NewDep);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Added dependency: %s depends on %s (Optional: %s)"), 
           *SystemName, *DependencyName, bOptional ? TEXT("Yes") : TEXT("No"));
}

bool UEng_SystemIntegrator::ValidateSystemDependencies()
{
    TArray<FString> VisitedSystems;
    
    for (const FEng_SystemDependency& Dependency : SystemDependencies)
    {
        VisitedSystems.Empty();
        if (!ValidateDependencyChain(Dependency.SystemName, VisitedSystems))
        {
            UE_LOG(LogTemp, Error, TEXT("Circular dependency detected for system: %s"), *Dependency.SystemName);
            return false;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("All system dependencies validated successfully"));
    return true;
}

void UEng_SystemIntegrator::InitializeSystemsInOrder()
{
    if (bSystemsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Systems already initialized"));
        return;
    }
    
    if (!ValidateSystemDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize systems - dependency validation failed"));
        return;
    }
    
    BuildInitializationOrder();
    
    for (const FString& SystemName : InitializationOrder)
    {
        UE_LOG(LogTemp, Log, TEXT("Initializing system: %s"), *SystemName);
        // Here we would actually initialize the system
        // For now, just log the initialization
    }
    
    bSystemsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("All systems initialized in order"));
}

void UEng_SystemIntegrator::ShutdownSystemsInReverseOrder()
{
    if (!bSystemsInitialized)
    {
        return;
    }
    
    for (int32 i = InitializationOrder.Num() - 1; i >= 0; i--)
    {
        const FString& SystemName = InitializationOrder[i];
        UE_LOG(LogTemp, Log, TEXT("Shutting down system: %s"), *SystemName);
        // Here we would actually shutdown the system
    }
    
    bSystemsInitialized = false;
    UE_LOG(LogTemp, Warning, TEXT("All systems shut down"));
}

TArray<FString> UEng_SystemIntegrator::GetSystemInitializationOrder()
{
    if (InitializationOrder.Num() == 0)
    {
        BuildInitializationOrder();
    }
    return InitializationOrder;
}

bool UEng_SystemIntegrator::IsSystemActive(const FString& SystemName)
{
    for (const FEng_SystemLayer& Layer : SystemLayers)
    {
        if (Layer.SystemNames.Contains(SystemName))
        {
            return Layer.bIsActive;
        }
    }
    return false;
}

void UEng_SystemIntegrator::SetSystemActive(const FString& SystemName, bool bActive)
{
    for (FEng_SystemLayer& Layer : SystemLayers)
    {
        if (Layer.SystemNames.Contains(SystemName))
        {
            Layer.bIsActive = bActive;
            UE_LOG(LogTemp, Log, TEXT("System %s set to %s"), *SystemName, bActive ? TEXT("Active") : TEXT("Inactive"));
            return;
        }
    }
}

FString UEng_SystemIntegrator::GetSystemStatus()
{
    FString Status = TEXT("=== SYSTEM STATUS ===\n");
    
    for (const FEng_SystemLayer& Layer : SystemLayers)
    {
        Status += FString::Printf(TEXT("Layer: %s (Priority: %d, Active: %s)\n"), 
                                  *Layer.LayerName, Layer.Priority, Layer.bIsActive ? TEXT("Yes") : TEXT("No"));
        
        for (const FString& SystemName : Layer.SystemNames)
        {
            Status += FString::Printf(TEXT("  - %s\n"), *SystemName);
        }
    }
    
    Status += FString::Printf(TEXT("\nInitialization Order: %s\n"), 
                              *FString::Join(InitializationOrder, TEXT(" -> ")));
    
    return Status;
}

void UEng_SystemIntegrator::SortLayersByPriority()
{
    SystemLayers.Sort([](const FEng_SystemLayer& A, const FEng_SystemLayer& B)
    {
        return A.Priority < B.Priority;
    });
}

void UEng_SystemIntegrator::BuildInitializationOrder()
{
    InitializationOrder.Empty();
    
    // Add systems in layer priority order
    for (const FEng_SystemLayer& Layer : SystemLayers)
    {
        if (Layer.bIsActive)
        {
            for (const FString& SystemName : Layer.SystemNames)
            {
                InitializationOrder.AddUnique(SystemName);
            }
        }
    }
    
    // Sort within each layer based on dependencies
    // Simple topological sort - dependencies come first
    for (int32 i = 0; i < InitializationOrder.Num(); i++)
    {
        const FString& SystemName = InitializationOrder[i];
        
        const FEng_SystemDependency* Dependency = SystemDependencies.FindByPredicate([&SystemName](const FEng_SystemDependency& Dep)
        {
            return Dep.SystemName == SystemName;
        });
        
        if (Dependency)
        {
            for (const FString& DepName : Dependency->Dependencies)
            {
                int32 DepIndex = InitializationOrder.Find(DepName);
                if (DepIndex > i)
                {
                    // Move dependency before current system
                    InitializationOrder.RemoveAt(DepIndex);
                    InitializationOrder.Insert(DepName, i);
                }
            }
        }
    }
}

bool UEng_SystemIntegrator::ValidateDependencyChain(const FString& SystemName, TArray<FString>& VisitedSystems)
{
    if (VisitedSystems.Contains(SystemName))
    {
        return false; // Circular dependency
    }
    
    VisitedSystems.Add(SystemName);
    
    const FEng_SystemDependency* Dependency = SystemDependencies.FindByPredicate([&SystemName](const FEng_SystemDependency& Dep)
    {
        return Dep.SystemName == SystemName;
    });
    
    if (Dependency)
    {
        for (const FString& DepName : Dependency->Dependencies)
        {
            if (!ValidateDependencyChain(DepName, VisitedSystems))
            {
                return false;
            }
        }
    }
    
    VisitedSystems.Remove(SystemName);
    return true;
}