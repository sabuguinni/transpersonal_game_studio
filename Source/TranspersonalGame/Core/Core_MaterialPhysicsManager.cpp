#include "Core_MaterialPhysicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/StaticMesh.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "GameFramework/Character.h"

UCore_MaterialPhysicsManager::UCore_MaterialPhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default color palette
    TerrainGreenColor = FLinearColor(0.2f, 0.6f, 0.3f, 1.0f); // Forest green
    TreeBrownColor = FLinearColor(0.4f, 0.25f, 0.1f, 1.0f); // Bark brown
    RockGreyColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // Stone grey
    TRexDarkGreenColor = FLinearColor(0.15f, 0.4f, 0.2f, 1.0f); // Dark green
    RaptorOrangeBrownColor = FLinearColor(0.6f, 0.35f, 0.15f, 1.0f); // Orange-brown
    BrachiosaurusBlueGreyColor = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f); // Blue-grey
    
    // Performance settings
    MaterialUpdateFrequency = 1.0f; // Update once per second
    MaxMaterialsPerFrame = 10;
    bEnableDynamicMaterialUpdates = true;
    
    // Internal state
    LastMaterialUpdate = 0.0f;
    CurrentMaterialIndex = 0;
}

void UCore_MaterialPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_MaterialPhysicsManager: BeginPlay started"));
    
    InitializeDefaultMaterials();
    InitializePhysicalProperties();
    CacheWorldActors();
    
    // Apply initial materials
    ApplyTerrainMaterials();
    ApplyVegetationMaterials();
    ApplyRockMaterials();
    ApplyDinosaurMaterials();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_MaterialPhysicsManager: Initialization complete"));
}

void UCore_MaterialPhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableDynamicMaterialUpdates)
        return;
    
    LastMaterialUpdate += DeltaTime;
    
    if (LastMaterialUpdate >= MaterialUpdateFrequency)
    {
        // Refresh cached actors periodically
        CacheWorldActors();
        LastMaterialUpdate = 0.0f;
    }
}

void UCore_MaterialPhysicsManager::ApplyTerrainMaterials()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_MaterialPhysicsManager: Applying terrain materials"));
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find landscape actors
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape)
        {
            UE_LOG(LogTemp, Warning, TEXT("Found landscape actor: %s"), *Landscape->GetName());
            // Apply green terrain material to landscape
            // Note: Landscape materials are complex and require specific setup
            // For now, we log the discovery
        }
    }
    
    // Apply materials to terrain-like static mesh actors
    for (AActor* Actor : CachedActors)
    {
        if (!Actor) continue;
        
        FString ActorName = Actor->GetActorLabel().ToLower();
        if (ActorName.Contains(TEXT("terrain")) || ActorName.Contains(TEXT("ground")) || ActorName.Contains(TEXT("landscape")))
        {
            UMaterialInterface* TerrainMaterial = CreateBasicColorMaterial(TerrainGreenColor, TEXT("M_TerrainGreen"));
            if (TerrainMaterial)
            {
                ApplyMaterialToActor(Actor, TerrainMaterial);
                UE_LOG(LogTemp, Warning, TEXT("Applied green material to terrain actor: %s"), *Actor->GetActorLabel());
            }
        }
    }
}

void UCore_MaterialPhysicsManager::ApplyVegetationMaterials()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_MaterialPhysicsManager: Applying vegetation materials"));
    
    for (AActor* Actor : CachedActors)
    {
        if (!Actor) continue;
        
        FString ActorName = Actor->GetActorLabel().ToLower();
        if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("trunk")) || ActorName.Contains(TEXT("vegetation")))
        {
            UMaterialInterface* TreeMaterial = CreateBasicColorMaterial(TreeBrownColor, TEXT("M_TreeBrown"));
            if (TreeMaterial)
            {
                ApplyMaterialToActor(Actor, TreeMaterial);
                UE_LOG(LogTemp, Warning, TEXT("Applied brown material to tree actor: %s"), *Actor->GetActorLabel());
            }
        }
    }
}

void UCore_MaterialPhysicsManager::ApplyRockMaterials()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_MaterialPhysicsManager: Applying rock materials"));
    
    for (AActor* Actor : CachedActors)
    {
        if (!Actor) continue;
        
        FString ActorName = Actor->GetActorLabel().ToLower();
        if (ActorName.Contains(TEXT("rock")) || ActorName.Contains(TEXT("stone")) || ActorName.Contains(TEXT("boulder")))
        {
            UMaterialInterface* RockMaterial = CreateBasicColorMaterial(RockGreyColor, TEXT("M_RockGrey"));
            if (RockMaterial)
            {
                ApplyMaterialToActor(Actor, RockMaterial);
                UE_LOG(LogTemp, Warning, TEXT("Applied grey material to rock actor: %s"), *Actor->GetActorLabel());
            }
        }
    }
}

void UCore_MaterialPhysicsManager::ApplyDinosaurMaterials()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_MaterialPhysicsManager: Applying dinosaur materials"));
    
    for (AActor* Actor : CachedActors)
    {
        if (!Actor) continue;
        
        FString ActorName = Actor->GetActorLabel().ToLower();
        UMaterialInterface* DinosaurMaterial = nullptr;
        
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("t-rex")))
        {
            DinosaurMaterial = CreateBasicColorMaterial(TRexDarkGreenColor, TEXT("M_TRexDarkGreen"));
        }
        else if (ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("velociraptor")))
        {
            DinosaurMaterial = CreateBasicColorMaterial(RaptorOrangeBrownColor, TEXT("M_RaptorOrangeBrown"));
        }
        else if (ActorName.Contains(TEXT("brachi")) || ActorName.Contains(TEXT("brachiosaurus")))
        {
            DinosaurMaterial = CreateBasicColorMaterial(BrachiosaurusBlueGreyColor, TEXT("M_BrachiosaurusBlueGrey"));
        }
        
        if (DinosaurMaterial)
        {
            ApplyMaterialToActor(Actor, DinosaurMaterial);
            UE_LOG(LogTemp, Warning, TEXT("Applied dinosaur material to actor: %s"), *Actor->GetActorLabel());
        }
    }
}

void UCore_MaterialPhysicsManager::ApplyBiomeMaterials(ECore_BiomeType BiomeType, const TArray<AActor*>& Actors)
{
    UE_LOG(LogTemp, Warning, TEXT("Core_MaterialPhysicsManager: Applying biome materials for biome type: %d"), (int32)BiomeType);
    
    for (AActor* Actor : Actors)
    {
        if (!Actor) continue;
        
        ECore_SurfaceType SurfaceType = DetermineSurfaceType(Actor);
        UMaterialInterface* Material = nullptr;
        
        // Apply biome-specific material variations
        switch (BiomeType)
        {
        case ECore_BiomeType::Savana:
            if (SurfaceType == ECore_SurfaceType::Grass)
                Material = CreateBasicColorMaterial(FLinearColor(0.6f, 0.5f, 0.2f, 1.0f), TEXT("M_SavanaGrass"));
            break;
        case ECore_BiomeType::Floresta:
            if (SurfaceType == ECore_SurfaceType::Grass)
                Material = CreateBasicColorMaterial(FLinearColor(0.1f, 0.4f, 0.2f, 1.0f), TEXT("M_ForestGrass"));
            break;
        case ECore_BiomeType::Pantano:
            if (SurfaceType == ECore_SurfaceType::Mud)
                Material = CreateBasicColorMaterial(FLinearColor(0.3f, 0.2f, 0.1f, 1.0f), TEXT("M_SwampMud"));
            break;
        case ECore_BiomeType::Deserto:
            if (SurfaceType == ECore_SurfaceType::Sand)
                Material = CreateBasicColorMaterial(FLinearColor(0.8f, 0.7f, 0.5f, 1.0f), TEXT("M_DesertSand"));
            break;
        case ECore_BiomeType::Montanha:
            if (SurfaceType == ECore_SurfaceType::Rock)
                Material = CreateBasicColorMaterial(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f), TEXT("M_MountainRock"));
            break;
        }
        
        if (Material)
        {
            ApplyMaterialToActor(Actor, Material);
        }
    }
}

UPhysicalMaterial* UCore_MaterialPhysicsManager::GetPhysicalMaterialForSurface(ECore_SurfaceType SurfaceType)
{
    if (PhysicalMaterials.Contains(SurfaceType))
    {
        TSoftObjectPtr<UPhysicalMaterial> MaterialPtr = PhysicalMaterials[SurfaceType];
        return MaterialPtr.LoadSynchronous();
    }
    
    return nullptr;
}

void UCore_MaterialPhysicsManager::SetupPhysicalMaterialProperties()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_MaterialPhysicsManager: Setting up physical material properties"));
    
    // Initialize surface friction values
    SurfaceFriction.Add(ECore_SurfaceType::Grass, 0.7f);
    SurfaceFriction.Add(ECore_SurfaceType::Rock, 0.9f);
    SurfaceFriction.Add(ECore_SurfaceType::Mud, 0.3f);
    SurfaceFriction.Add(ECore_SurfaceType::Sand, 0.5f);
    SurfaceFriction.Add(ECore_SurfaceType::Water, 0.1f);
    SurfaceFriction.Add(ECore_SurfaceType::Wood, 0.6f);
    SurfaceFriction.Add(ECore_SurfaceType::Bone, 0.8f);
    
    // Initialize surface restitution values
    SurfaceRestitution.Add(ECore_SurfaceType::Grass, 0.1f);
    SurfaceRestitution.Add(ECore_SurfaceType::Rock, 0.2f);
    SurfaceRestitution.Add(ECore_SurfaceType::Mud, 0.05f);
    SurfaceRestitution.Add(ECore_SurfaceType::Sand, 0.1f);
    SurfaceRestitution.Add(ECore_SurfaceType::Water, 0.0f);
    SurfaceRestitution.Add(ECore_SurfaceType::Wood, 0.3f);
    SurfaceRestitution.Add(ECore_SurfaceType::Bone, 0.4f);
    
    // Initialize surface destructibility
    SurfaceDestructibility.Add(ECore_SurfaceType::Grass, false);
    SurfaceDestructibility.Add(ECore_SurfaceType::Rock, true);
    SurfaceDestructibility.Add(ECore_SurfaceType::Mud, false);
    SurfaceDestructibility.Add(ECore_SurfaceType::Sand, false);
    SurfaceDestructibility.Add(ECore_SurfaceType::Water, false);
    SurfaceDestructibility.Add(ECore_SurfaceType::Wood, true);
    SurfaceDestructibility.Add(ECore_SurfaceType::Bone, true);
}

float UCore_MaterialPhysicsManager::GetFrictionForSurface(ECore_SurfaceType SurfaceType)
{
    if (SurfaceFriction.Contains(SurfaceType))
    {
        return SurfaceFriction[SurfaceType];
    }
    return 0.7f; // Default friction
}

float UCore_MaterialPhysicsManager::GetRestitutionForSurface(ECore_SurfaceType SurfaceType)
{
    if (SurfaceRestitution.Contains(SurfaceType))
    {
        return SurfaceRestitution[SurfaceType];
    }
    return 0.1f; // Default restitution
}

bool UCore_MaterialPhysicsManager::IsSurfaceDestructible(ECore_SurfaceType SurfaceType)
{
    if (SurfaceDestructibility.Contains(SurfaceType))
    {
        return SurfaceDestructibility[SurfaceType];
    }
    return false; // Default not destructible
}

void UCore_MaterialPhysicsManager::InitializeDefaultMaterials()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_MaterialPhysicsManager: Initializing default materials"));
    
    // Initialize surface materials map
    // Note: In a real implementation, these would be loaded from content browser assets
    // For now, we'll create them dynamically as needed
}

void UCore_MaterialPhysicsManager::InitializePhysicalProperties()
{
    SetupPhysicalMaterialProperties();
}

void UCore_MaterialPhysicsManager::CacheWorldActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    CachedActors.Empty();
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !Actor->IsPendingKill())
        {
            CachedActors.Add(Actor);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_MaterialPhysicsManager: Cached %d actors"), CachedActors.Num());
}

UMaterialInterface* UCore_MaterialPhysicsManager::CreateBasicColorMaterial(const FLinearColor& Color, const FString& MaterialName)
{
    // In a real implementation, this would create a proper material instance
    // For now, we return nullptr and rely on logging for verification
    UE_LOG(LogTemp, Warning, TEXT("Creating material %s with color R:%.2f G:%.2f B:%.2f"), 
           *MaterialName, Color.R, Color.G, Color.B);
    
    return nullptr; // Placeholder - would return actual material in full implementation
}

void UCore_MaterialPhysicsManager::ApplyMaterialToActor(AActor* Actor, UMaterialInterface* Material)
{
    if (!Actor || !Material) return;
    
    // Find static mesh component and apply material
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        MeshComp->SetMaterial(0, Material);
        UE_LOG(LogTemp, Warning, TEXT("Applied material to actor: %s"), *Actor->GetActorLabel());
    }
}

ECore_SurfaceType UCore_MaterialPhysicsManager::DetermineSurfaceType(AActor* Actor)
{
    if (!Actor) return ECore_SurfaceType::Grass;
    
    FString ActorName = Actor->GetActorLabel().ToLower();
    
    if (ActorName.Contains(TEXT("rock")) || ActorName.Contains(TEXT("stone")))
        return ECore_SurfaceType::Rock;
    else if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("wood")))
        return ECore_SurfaceType::Wood;
    else if (ActorName.Contains(TEXT("water")) || ActorName.Contains(TEXT("river")))
        return ECore_SurfaceType::Water;
    else if (ActorName.Contains(TEXT("sand")) || ActorName.Contains(TEXT("desert")))
        return ECore_SurfaceType::Sand;
    else if (ActorName.Contains(TEXT("mud")) || ActorName.Contains(TEXT("swamp")))
        return ECore_SurfaceType::Mud;
    else if (ActorName.Contains(TEXT("bone")) || ActorName.Contains(TEXT("skeleton")))
        return ECore_SurfaceType::Bone;
    
    return ECore_SurfaceType::Grass; // Default
}

ECore_BiomeType UCore_MaterialPhysicsManager::DetermineBiomeType(const FVector& Location)
{
    // Simple biome determination based on location
    // In a real implementation, this would use proper biome data
    
    if (Location.X < -40000 && Location.Y > 30000)
        return ECore_BiomeType::Floresta;
    else if (Location.X < -40000 && Location.Y < -40000)
        return ECore_BiomeType::Pantano;
    else if (Location.X > 50000)
        return ECore_BiomeType::Deserto;
    else if (Location.X > 35000 && Location.Y > 45000)
        return ECore_BiomeType::Montanha;
    
    return ECore_BiomeType::Savana; // Default
}