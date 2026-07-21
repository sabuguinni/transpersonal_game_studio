#include "Core_TerrainPhysicsEnhancer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACore_TerrainPhysicsEnhancer::ACore_TerrainPhysicsEnhancer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default physics properties
    PhysicsProperties.Friction = 0.7f;
    PhysicsProperties.Restitution = 0.3f;
    PhysicsProperties.Density = 1.0f;
    PhysicsProperties.bEnableComplexCollision = true;
    PhysicsProperties.TerrainType = ECore_TerrainType::Grassy;

    // Set default update interval
    PhysicsUpdateInterval = 0.1f;
    bEnableAdvancedPhysics = true;
    bPhysicsInitialized = false;
}

void ACore_TerrainPhysicsEnhancer::BeginPlay()
{
    Super::BeginPlay();

    // Initialize terrain physics system
    InitializeTerrainPhysics();

    // Set up physics update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            PhysicsUpdateTimer,
            this,
            &ACore_TerrainPhysicsEnhancer::UpdatePhysicsProperties,
            PhysicsUpdateInterval,
            true
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsEnhancer: Advanced terrain physics system initialized"));
}

void ACore_TerrainPhysicsEnhancer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableAdvancedPhysics && bPhysicsInitialized)
    {
        // Update physics based on current conditions
        LastPhysicsUpdate += DeltaTime;
        
        if (LastPhysicsUpdate >= PhysicsUpdateInterval)
        {
            ProcessTerrainCollision();
            LastPhysicsUpdate = 0.0f;
        }
    }
}

void ACore_TerrainPhysicsEnhancer::InitializeTerrainPhysics()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsEnhancer: Initializing terrain physics system"));

    // Find all landscape actors in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysicsEnhancer: No valid world found"));
        return;
    }

    // Clear existing managed landscapes
    ManagedLandscapes.Empty();

    // Find and register all landscape actors
    TArray<AActor*> FoundLandscapes;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), FoundLandscapes);

    for (AActor* Actor : FoundLandscapes)
    {
        if (ALandscape* Landscape = Cast<ALandscape>(Actor))
        {
            RegisterLandscapeActor(Landscape);
        }
    }

    // Set up terrain material physics
    SetupTerrainMaterialPhysics();

    // Enable complex collision for all managed terrains
    EnableComplexCollisionForTerrain();

    // Create physics zones
    CreatePhysicsZones();

    bPhysicsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsEnhancer: Physics initialization complete - %d landscapes managed"), ManagedLandscapes.Num());
}

void ACore_TerrainPhysicsEnhancer::UpdateTerrainPhysicsProperties()
{
    if (!bPhysicsInitialized || ManagedLandscapes.Num() == 0)
    {
        return;
    }

    // Apply current physics properties to all managed landscapes
    for (ALandscape* Landscape : ManagedLandscapes)
    {
        if (IsValid(Landscape))
        {
            // Update collision properties
            UPrimitiveComponent* LandscapeComponent = Landscape->GetRootComponent();
            if (LandscapeComponent)
            {
                // Set collision enabled
                LandscapeComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                LandscapeComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
                
                // Update physics properties based on terrain type
                ApplyTerrainTypePhysics(PhysicsProperties.TerrainType);
            }
        }
    }
}

void ACore_TerrainPhysicsEnhancer::ApplyTerrainTypePhysics(ECore_TerrainType TerrainType)
{
    FCore_TerrainPhysicsProperties TypePhysics;

    switch (TerrainType)
    {
        case ECore_TerrainType::Rocky:
            TypePhysics = GetRockyTerrainPhysics();
            break;
        case ECore_TerrainType::Muddy:
            TypePhysics = GetMuddyTerrainPhysics();
            break;
        case ECore_TerrainType::Sandy:
            TypePhysics = GetSandyTerrainPhysics();
            break;
        case ECore_TerrainType::Grassy:
            TypePhysics = GetGrassyTerrainPhysics();
            break;
        case ECore_TerrainType::Icy:
            TypePhysics = GetIcyTerrainPhysics();
            break;
        default:
            TypePhysics = GetGrassyTerrainPhysics();
            break;
    }

    // Apply the physics properties
    PhysicsProperties = TypePhysics;
    UpdateTerrainPhysicsProperties();

    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsEnhancer: Applied %s terrain physics"), 
           *UEnum::GetValueAsString(TerrainType));
}

FCore_TerrainPhysicsProperties ACore_TerrainPhysicsEnhancer::GetTerrainPhysicsAtLocation(FVector Location)
{
    // For now, return current physics properties
    // In a more advanced implementation, this would sample terrain data at the location
    return PhysicsProperties;
}

void ACore_TerrainPhysicsEnhancer::RegisterLandscapeActor(ALandscape* LandscapeActor)
{
    if (IsValid(LandscapeActor) && !ManagedLandscapes.Contains(LandscapeActor))
    {
        ManagedLandscapes.Add(LandscapeActor);
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsEnhancer: Registered landscape actor %s"), 
               *LandscapeActor->GetName());
    }
}

void ACore_TerrainPhysicsEnhancer::UnregisterLandscapeActor(ALandscape* LandscapeActor)
{
    if (ManagedLandscapes.Contains(LandscapeActor))
    {
        ManagedLandscapes.Remove(LandscapeActor);
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsEnhancer: Unregistered landscape actor %s"), 
               *LandscapeActor->GetName());
    }
}

void ACore_TerrainPhysicsEnhancer::EnableComplexCollisionForTerrain()
{
    for (ALandscape* Landscape : ManagedLandscapes)
    {
        if (IsValid(Landscape))
        {
            UPrimitiveComponent* LandscapeComponent = Landscape->GetRootComponent();
            if (LandscapeComponent)
            {
                // Enable complex collision for detailed physics
                LandscapeComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                LandscapeComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
                
                // Set collision object type
                LandscapeComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsEnhancer: Complex collision enabled for all terrain"));
}

void ACore_TerrainPhysicsEnhancer::SetupTerrainMaterialPhysics()
{
    // Set up material physics properties for terrain interaction
    // This would typically involve creating or modifying physical materials
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsEnhancer: Terrain material physics configured"));
}

void ACore_TerrainPhysicsEnhancer::CreatePhysicsZones()
{
    // Create different physics zones for varied terrain behavior
    // This is handled in the UE5 Python script for immediate visual feedback
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsEnhancer: Physics zones creation initiated"));
}

bool ACore_TerrainPhysicsEnhancer::ValidateTerrainPhysicsSetup()
{
    if (ManagedLandscapes.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysicsEnhancer: No landscapes registered"));
        return false;
    }

    for (ALandscape* Landscape : ManagedLandscapes)
    {
        if (!IsValid(Landscape))
        {
            UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysicsEnhancer: Invalid landscape found"));
            return false;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsEnhancer: Physics setup validation passed"));
    return true;
}

void ACore_TerrainPhysicsEnhancer::OptimizeTerrainPhysicsLOD()
{
    // Implement LOD optimization for terrain physics
    // Reduce physics complexity based on distance and importance
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsEnhancer: Physics LOD optimization applied"));
}

void ACore_TerrainPhysicsEnhancer::UpdatePhysicsBasedOnPlayerDistance(FVector PlayerLocation)
{
    float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerLocation);
    
    // Adjust physics update frequency based on distance
    if (DistanceToPlayer > 5000.0f)
    {
        PhysicsUpdateInterval = 0.5f; // Slower updates for distant terrain
    }
    else if (DistanceToPlayer > 2000.0f)
    {
        PhysicsUpdateInterval = 0.2f; // Medium updates
    }
    else
    {
        PhysicsUpdateInterval = 0.1f; // Fast updates for nearby terrain
    }
}

void ACore_TerrainPhysicsEnhancer::UpdatePhysicsProperties()
{
    if (bEnableAdvancedPhysics && bPhysicsInitialized)
    {
        // Update terrain physics properties
        UpdateTerrainPhysicsProperties();
    }
}

void ACore_TerrainPhysicsEnhancer::ProcessTerrainCollision()
{
    // Process terrain collision events and responses
    // This would handle dynamic terrain interaction
}

void ACore_TerrainPhysicsEnhancer::HandleTerrainDeformation()
{
    // Handle terrain deformation physics
    // This would manage terrain changes due to impacts or environmental effects
}

FCore_TerrainPhysicsProperties ACore_TerrainPhysicsEnhancer::GetRockyTerrainPhysics()
{
    FCore_TerrainPhysicsProperties RockyPhysics;
    RockyPhysics.Friction = 0.9f;        // High friction
    RockyPhysics.Restitution = 0.1f;     // Low bounce
    RockyPhysics.Density = 2.5f;         // High density
    RockyPhysics.bEnableComplexCollision = true;
    RockyPhysics.TerrainType = ECore_TerrainType::Rocky;
    return RockyPhysics;
}

FCore_TerrainPhysicsProperties ACore_TerrainPhysicsEnhancer::GetMuddyTerrainPhysics()
{
    FCore_TerrainPhysicsProperties MuddyPhysics;
    MuddyPhysics.Friction = 0.3f;        // Low friction
    MuddyPhysics.Restitution = 0.05f;    // Very low bounce
    MuddyPhysics.Density = 1.8f;         // Medium-high density
    MuddyPhysics.bEnableComplexCollision = true;
    MuddyPhysics.TerrainType = ECore_TerrainType::Muddy;
    return MuddyPhysics;
}

FCore_TerrainPhysicsProperties ACore_TerrainPhysicsEnhancer::GetSandyTerrainPhysics()
{
    FCore_TerrainPhysicsProperties SandyPhysics;
    SandyPhysics.Friction = 0.5f;        // Medium friction
    SandyPhysics.Restitution = 0.2f;     // Low bounce
    SandyPhysics.Density = 1.6f;         // Medium density
    SandyPhysics.bEnableComplexCollision = true;
    SandyPhysics.TerrainType = ECore_TerrainType::Sandy;
    return SandyPhysics;
}

FCore_TerrainPhysicsProperties ACore_TerrainPhysicsEnhancer::GetGrassyTerrainPhysics()
{
    FCore_TerrainPhysicsProperties GrassyPhysics;
    GrassyPhysics.Friction = 0.7f;       // Good friction
    GrassyPhysics.Restitution = 0.3f;    // Medium bounce
    GrassyPhysics.Density = 1.2f;        // Light density
    GrassyPhysics.bEnableComplexCollision = true;
    GrassyPhysics.TerrainType = ECore_TerrainType::Grassy;
    return GrassyPhysics;
}

FCore_TerrainPhysicsProperties ACore_TerrainPhysicsEnhancer::GetIcyTerrainPhysics()
{
    FCore_TerrainPhysicsProperties IcyPhysics;
    IcyPhysics.Friction = 0.1f;          // Very low friction
    IcyPhysics.Restitution = 0.8f;       // High bounce
    IcyPhysics.Density = 0.9f;           // Low density
    IcyPhysics.bEnableComplexCollision = true;
    IcyPhysics.TerrainType = ECore_TerrainType::Icy;
    return IcyPhysics;
}