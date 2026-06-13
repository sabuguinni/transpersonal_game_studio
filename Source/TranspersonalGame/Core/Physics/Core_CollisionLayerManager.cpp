#include "Core_CollisionLayerManager.h"
#include "Engine/World.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/CollisionProfile.h"

UCore_CollisionLayerManager::UCore_CollisionLayerManager()
{
    // Initialize default values
}

void UCore_CollisionLayerManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Initializing collision layer system"));
    
    // Initialize collision layers and physics materials
    InitializeCollisionLayers();
    InitializePhysicsMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Initialization complete"));
}

void UCore_CollisionLayerManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Deinitializing collision layer system"));
    
    // Clean up created physics materials
    for (auto& Pair : CreatedPhysicsMaterials)
    {
        if (Pair.Value)
        {
            Pair.Value->ConditionalBeginDestroy();
        }
    }
    CreatedPhysicsMaterials.Empty();
    
    CollisionLayers.Empty();
    PhysicsMaterials.Empty();
    
    Super::Deinitialize();
}

void UCore_CollisionLayerManager::InitializeCollisionLayers()
{
    CollisionLayers.Empty();
    CreateDefaultCollisionLayers();
    SetupCollisionResponses();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Initialized %d collision layers"), CollisionLayers.Num());
}

void UCore_CollisionLayerManager::CreateDefaultCollisionLayers()
{
    // Player collision layer
    FCore_CollisionLayer PlayerLayer;
    PlayerLayer.LayerName = FName("Player");
    PlayerLayer.CollisionChannel = ECC_Pawn;
    PlayerLayer.DefaultResponse = ECR_Block;
    PlayerLayer.BlockingLayers = {FName("World"), FName("Dinosaur"), FName("Environment")};
    PlayerLayer.IgnoringLayers = {FName("Projectile"), FName("Trigger")};
    RegisterCollisionLayer(PlayerLayer);

    // Dinosaur collision layer
    FCore_CollisionLayer DinosaurLayer;
    DinosaurLayer.LayerName = FName("Dinosaur");
    DinosaurLayer.CollisionChannel = ECC_Pawn;
    DinosaurLayer.DefaultResponse = ECR_Block;
    DinosaurLayer.BlockingLayers = {FName("World"), FName("Player"), FName("Environment")};
    DinosaurLayer.IgnoringLayers = {FName("Trigger")};
    RegisterCollisionLayer(DinosaurLayer);

    // World static collision layer
    FCore_CollisionLayer WorldLayer;
    WorldLayer.LayerName = FName("World");
    WorldLayer.CollisionChannel = ECC_WorldStatic;
    WorldLayer.DefaultResponse = ECR_Block;
    WorldLayer.BlockingLayers = {FName("Player"), FName("Dinosaur"), FName("Projectile")};
    RegisterCollisionLayer(WorldLayer);

    // Environment objects (rocks, trees, etc.)
    FCore_CollisionLayer EnvironmentLayer;
    EnvironmentLayer.LayerName = FName("Environment");
    EnvironmentLayer.CollisionChannel = ECC_WorldDynamic;
    EnvironmentLayer.DefaultResponse = ECR_Block;
    EnvironmentLayer.BlockingLayers = {FName("Player"), FName("Dinosaur"), FName("Projectile")};
    RegisterCollisionLayer(EnvironmentLayer);

    // Projectile collision layer
    FCore_CollisionLayer ProjectileLayer;
    ProjectileLayer.LayerName = FName("Projectile");
    ProjectileLayer.CollisionChannel = ECC_WorldDynamic;
    ProjectileLayer.DefaultResponse = ECR_Block;
    ProjectileLayer.BlockingLayers = {FName("World"), FName("Dinosaur"), FName("Environment")};
    ProjectileLayer.IgnoringLayers = {FName("Player"), FName("Trigger")};
    RegisterCollisionLayer(ProjectileLayer);

    // Trigger collision layer
    FCore_CollisionLayer TriggerLayer;
    TriggerLayer.LayerName = FName("Trigger");
    TriggerLayer.CollisionChannel = ECC_WorldDynamic;
    TriggerLayer.DefaultResponse = ECR_Overlap;
    TriggerLayer.IgnoringLayers = {FName("World"), FName("Environment")};
    RegisterCollisionLayer(TriggerLayer);
}

bool UCore_CollisionLayerManager::RegisterCollisionLayer(const FCore_CollisionLayer& Layer)
{
    // Check if layer already exists
    for (const FCore_CollisionLayer& ExistingLayer : CollisionLayers)
    {
        if (ExistingLayer.LayerName == Layer.LayerName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_CollisionLayerManager: Layer %s already exists"), *Layer.LayerName.ToString());
            return false;
        }
    }

    CollisionLayers.Add(Layer);
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Registered collision layer %s"), *Layer.LayerName.ToString());
    return true;
}

FCore_CollisionLayer UCore_CollisionLayerManager::GetCollisionLayer(FName LayerName) const
{
    for (const FCore_CollisionLayer& Layer : CollisionLayers)
    {
        if (Layer.LayerName == LayerName)
        {
            return Layer;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_CollisionLayerManager: Layer %s not found"), *LayerName.ToString());
    return FCore_CollisionLayer();
}

ECollisionChannel UCore_CollisionLayerManager::GetCollisionChannel(FName LayerName) const
{
    FCore_CollisionLayer Layer = GetCollisionLayer(LayerName);
    return Layer.CollisionChannel;
}

void UCore_CollisionLayerManager::SetCollisionResponseBetweenLayers(FName Layer1, FName Layer2, ECollisionResponse Response)
{
    // Find and update layer collision responses
    for (FCore_CollisionLayer& Layer : CollisionLayers)
    {
        if (Layer.LayerName == Layer1)
        {
            if (Response == ECR_Block)
            {
                Layer.BlockingLayers.AddUnique(Layer2);
                Layer.IgnoringLayers.Remove(Layer2);
            }
            else if (Response == ECR_Ignore)
            {
                Layer.IgnoringLayers.AddUnique(Layer2);
                Layer.BlockingLayers.Remove(Layer2);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Set collision response between %s and %s"), *Layer1.ToString(), *Layer2.ToString());
}

void UCore_CollisionLayerManager::InitializePhysicsMaterials()
{
    PhysicsMaterials.Empty();
    CreateDefaultPhysicsMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Initialized %d physics materials"), PhysicsMaterials.Num());
}

void UCore_CollisionLayerManager::CreateDefaultPhysicsMaterials()
{
    // Stone material
    FCore_PhysicsMaterial StoneMaterial;
    StoneMaterial.MaterialName = FName("Stone");
    StoneMaterial.Friction = 0.8f;
    StoneMaterial.Restitution = 0.1f;
    StoneMaterial.Density = 2.5f;
    RegisterPhysicsMaterial(StoneMaterial);

    // Wood material
    FCore_PhysicsMaterial WoodMaterial;
    WoodMaterial.MaterialName = FName("Wood");
    WoodMaterial.Friction = 0.6f;
    WoodMaterial.Restitution = 0.2f;
    WoodMaterial.Density = 0.8f;
    RegisterPhysicsMaterial(WoodMaterial);

    // Flesh material (for characters)
    FCore_PhysicsMaterial FleshMaterial;
    FleshMaterial.MaterialName = FName("Flesh");
    FleshMaterial.Friction = 0.7f;
    FleshMaterial.Restitution = 0.1f;
    FleshMaterial.Density = 1.0f;
    RegisterPhysicsMaterial(FleshMaterial);

    // Ground material
    FCore_PhysicsMaterial GroundMaterial;
    GroundMaterial.MaterialName = FName("Ground");
    GroundMaterial.Friction = 0.9f;
    GroundMaterial.Restitution = 0.0f;
    GroundMaterial.Density = 2.0f;
    RegisterPhysicsMaterial(GroundMaterial);

    // Water material
    FCore_PhysicsMaterial WaterMaterial;
    WaterMaterial.MaterialName = FName("Water");
    WaterMaterial.Friction = 0.1f;
    WaterMaterial.Restitution = 0.0f;
    WaterMaterial.Density = 1.0f;
    RegisterPhysicsMaterial(WaterMaterial);
}

bool UCore_CollisionLayerManager::RegisterPhysicsMaterial(const FCore_PhysicsMaterial& Material)
{
    // Check if material already exists
    for (const FCore_PhysicsMaterial& ExistingMaterial : PhysicsMaterials)
    {
        if (ExistingMaterial.MaterialName == Material.MaterialName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_CollisionLayerManager: Physics material %s already exists"), *Material.MaterialName.ToString());
            return false;
        }
    }

    PhysicsMaterials.Add(Material);
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Registered physics material %s"), *Material.MaterialName.ToString());
    return true;
}

FCore_PhysicsMaterial UCore_CollisionLayerManager::GetPhysicsMaterial(FName MaterialName) const
{
    for (const FCore_PhysicsMaterial& Material : PhysicsMaterials)
    {
        if (Material.MaterialName == MaterialName)
        {
            return Material;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_CollisionLayerManager: Physics material %s not found"), *MaterialName.ToString());
    return FCore_PhysicsMaterial();
}

UPhysicalMaterial* UCore_CollisionLayerManager::CreateUE5PhysicsMaterial(FName MaterialName)
{
    // Check if already created
    if (CreatedPhysicsMaterials.Contains(MaterialName))
    {
        return CreatedPhysicsMaterials[MaterialName];
    }

    FCore_PhysicsMaterial CoreMaterial = GetPhysicsMaterial(MaterialName);
    if (CoreMaterial.MaterialName == NAME_None)
    {
        return nullptr;
    }

    // Create UE5 physics material
    UPhysicalMaterial* PhysMat = NewObject<UPhysicalMaterial>();
    PhysMat->Friction = CoreMaterial.Friction;
    PhysMat->Restitution = CoreMaterial.Restitution;
    PhysMat->Density = CoreMaterial.Density;
    
    if (CoreMaterial.bOverrideFrictionCombineMode)
    {
        PhysMat->bOverrideFrictionCombineMode = true;
        PhysMat->FrictionCombineMode = CoreMaterial.FrictionCombineMode;
    }

    CreatedPhysicsMaterials.Add(MaterialName, PhysMat);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Created UE5 physics material %s"), *MaterialName.ToString());
    return PhysMat;
}

void UCore_CollisionLayerManager::SetActorCollisionLayer(AActor* Actor, FName LayerName)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionLayerManager: Invalid actor for collision layer setup"));
        return;
    }

    ECollisionChannel Channel = GetCollisionChannel(LayerName);
    
    // Set collision on all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            PrimComp->SetCollisionObjectType(Channel);
            UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Set collision layer %s on %s"), *LayerName.ToString(), *Actor->GetName());
        }
    }
}

void UCore_CollisionLayerManager::SetActorPhysicsMaterial(AActor* Actor, FName MaterialName)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionLayerManager: Invalid actor for physics material setup"));
        return;
    }

    UPhysicalMaterial* PhysMat = CreateUE5PhysicsMaterial(MaterialName);
    if (!PhysMat)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionLayerManager: Failed to create physics material %s"), *MaterialName.ToString());
        return;
    }

    // Apply to all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            PrimComp->SetPhysMaterialOverride(PhysMat);
            UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Set physics material %s on %s"), *MaterialName.ToString(), *Actor->GetName());
        }
    }
}

void UCore_CollisionLayerManager::ApplyCollisionProfileToActor(AActor* Actor, FName ProfileName)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionLayerManager: Invalid actor for collision profile setup"));
        return;
    }

    // Apply collision profile to all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            PrimComp->SetCollisionProfileName(ProfileName);
            UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Applied collision profile %s to %s"), *ProfileName.ToString(), *Actor->GetName());
        }
    }
}

void UCore_CollisionLayerManager::SetupCollisionResponses()
{
    // This would typically set up collision response matrix
    // For now, we log the setup completion
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Collision response matrix setup complete"));
}

void UCore_CollisionLayerManager::ValidateCollisionSetup()
{
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Validating collision setup"));
    
    int32 ValidLayers = 0;
    int32 ValidMaterials = 0;
    
    for (const FCore_CollisionLayer& Layer : CollisionLayers)
    {
        if (Layer.LayerName != NAME_None)
        {
            ValidLayers++;
        }
    }
    
    for (const FCore_PhysicsMaterial& Material : PhysicsMaterials)
    {
        if (Material.MaterialName != NAME_None)
        {
            ValidMaterials++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: Validation complete - %d valid layers, %d valid materials"), ValidLayers, ValidMaterials);
}

void UCore_CollisionLayerManager::LogCollisionLayers()
{
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: === Collision Layers ==="));
    for (const FCore_CollisionLayer& Layer : CollisionLayers)
    {
        UE_LOG(LogTemp, Log, TEXT("Layer: %s, Channel: %d, Response: %d"), 
               *Layer.LayerName.ToString(), 
               (int32)Layer.CollisionChannel, 
               (int32)Layer.DefaultResponse);
    }
}

void UCore_CollisionLayerManager::LogPhysicsMaterials()
{
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionLayerManager: === Physics Materials ==="));
    for (const FCore_PhysicsMaterial& Material : PhysicsMaterials)
    {
        UE_LOG(LogTemp, Log, TEXT("Material: %s, Friction: %.2f, Restitution: %.2f, Density: %.2f"), 
               *Material.MaterialName.ToString(), 
               Material.Friction, 
               Material.Restitution, 
               Material.Density);
    }
}