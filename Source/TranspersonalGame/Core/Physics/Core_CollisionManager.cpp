#include "Core_CollisionManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_CollisionManager::UCore_CollisionManager()
{
    // Set default collision channels
    TerrainChannel = ECC_WorldStatic;
    DinosaurChannel = ECC_Pawn;
    CharacterChannel = ECC_Pawn;
}

void UCore_CollisionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_CollisionManager: Initializing collision system"));
    
    InitializePhysicsMaterials();
    SetupCollisionChannels();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_CollisionManager: Collision system ready"));
}

void UCore_CollisionManager::Deinitialize()
{
    CollisionRegistry.Empty();
    TerrainMaterials.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_CollisionManager: Collision system shutdown"));
    
    Super::Deinitialize();
}

void UCore_CollisionManager::RegisterCollisionObject(UPrimitiveComponent* Component, const FCore_CollisionData& CollisionData)
{
    if (!Component)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_CollisionManager: Cannot register null component"));
        return;
    }

    CollisionRegistry.Add(Component, CollisionData);
    
    // Apply collision settings based on type
    switch (CollisionData.CollisionType)
    {
        case ECore_CollisionType::Terrain:
            Component->SetCollisionObjectType(TerrainChannel);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        case ECore_CollisionType::Dinosaur:
            Component->SetCollisionObjectType(DinosaurChannel);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetMassOverrideInKg(NAME_None, CollisionData.Mass, true);
            break;
            
        case ECore_CollisionType::Character:
            Component->SetCollisionObjectType(CharacterChannel);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
            
        default:
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Registered collision object of type %d"), (int32)CollisionData.CollisionType);
}

void UCore_CollisionManager::UnregisterCollisionObject(UPrimitiveComponent* Component)
{
    if (Component && CollisionRegistry.Contains(Component))
    {
        CollisionRegistry.Remove(Component);
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Unregistered collision object"));
    }
}

bool UCore_CollisionManager::LineTraceForTerrain(const FVector& Start, const FVector& End, FHitResult& OutHit)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = true;

    bool bHit = World->LineTraceSingleByChannel(
        OutHit,
        Start,
        End,
        TerrainChannel,
        QueryParams
    );

    if (bHit)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_CollisionManager: Terrain hit at %s"), *OutHit.Location.ToString());
    }

    return bHit;
}

bool UCore_CollisionManager::SphereTraceForDinosaurs(const FVector& Center, float Radius, TArray<FHitResult>& OutHits)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    bool bHit = World->SweepMultiByChannel(
        OutHits,
        Center,
        Center,
        FQuat::Identity,
        DinosaurChannel,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );

    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_CollisionManager: Sphere trace found %d dinosaurs"), OutHits.Num());

    return bHit;
}

bool UCore_CollisionManager::CheckGroundStability(const FVector& Location, float& OutStabilityFactor)
{
    FHitResult Hit;
    FVector TraceStart = Location + FVector(0, 0, 50);
    FVector TraceEnd = Location - FVector(0, 0, 200);
    
    if (LineTraceForTerrain(TraceStart, TraceEnd, Hit))
    {
        // Calculate stability based on surface normal
        FVector UpVector = FVector::UpVector;
        float DotProduct = FVector::DotProduct(Hit.Normal, UpVector);
        
        // Stability ranges from 0 (vertical cliff) to 1 (flat ground)
        OutStabilityFactor = FMath::Clamp(DotProduct, 0.0f, 1.0f);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Core_CollisionManager: Ground stability at %s is %f"), *Location.ToString(), OutStabilityFactor);
        
        return true;
    }
    
    OutStabilityFactor = 0.0f;
    return false;
}

void UCore_CollisionManager::ApplyTerrainPhysics(UPrimitiveComponent* Component, ECore_CollisionType TerrainType)
{
    if (!Component)
    {
        return;
    }

    UPhysicalMaterial** FoundMaterial = TerrainMaterials.Find(TerrainType);
    if (FoundMaterial && *FoundMaterial)
    {
        Component->SetPhysMaterialOverride(*FoundMaterial);
        UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Applied terrain physics for type %d"), (int32)TerrainType);
    }
}

void UCore_CollisionManager::SetCollisionResponseForDinosaur(UPrimitiveComponent* Component, bool bIsLargeDinosaur)
{
    if (!Component)
    {
        return;
    }

    if (bIsLargeDinosaur)
    {
        // Large dinosaurs can push through vegetation but are blocked by terrain
        Component->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
        Component->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
        Component->SetMassOverrideInKg(NAME_None, 5000.0f, true);
    }
    else
    {
        // Small dinosaurs are blocked by most objects
        Component->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
        Component->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
        Component->SetMassOverrideInKg(NAME_None, 500.0f, true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Set collision response for %s dinosaur"), bIsLargeDinosaur ? TEXT("large") : TEXT("small"));
}

void UCore_CollisionManager::HandleImpactEvent(const FHitResult& Hit, float ImpactForce)
{
    if (!Hit.GetComponent())
    {
        return;
    }

    float Damage = CalculateImpactDamage(Hit, ImpactForce);
    
    // Check if object can be destroyed
    FCore_CollisionData* CollisionData = CollisionRegistry.Find(Hit.GetComponent());
    if (CollisionData && CollisionData->bCanBeDestroyed && Damage > 50.0f)
    {
        ProcessDestructibleCollision(Hit.GetComponent(), Hit.Location, ImpactForce);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Impact event - Force: %f, Damage: %f"), ImpactForce, Damage);
}

void UCore_CollisionManager::ProcessDestructibleCollision(UPrimitiveComponent* Component, const FVector& ImpactPoint, float Force)
{
    if (!Component)
    {
        return;
    }

    AActor* Owner = Component->GetOwner();
    if (Owner)
    {
        // Create destruction effect (placeholder for now)
        UE_LOG(LogTemp, Warning, TEXT("Core_CollisionManager: Destroying object %s at impact point %s"), *Owner->GetName(), *ImpactPoint.ToString());
        
        // In a full implementation, this would trigger particle effects, sound, and debris
        Owner->Destroy();
    }
}

void UCore_CollisionManager::InitializePhysicsMaterials()
{
    // Create physics materials for different terrain types
    UPhysicalMaterial* GrassMaterial = NewObject<UPhysicalMaterial>();
    GrassMaterial->Friction = 0.8f;
    GrassMaterial->Restitution = 0.1f;
    GrassMaterial->Density = 1000.0f;
    TerrainMaterials.Add(ECore_CollisionType::Terrain, GrassMaterial);
    
    UPhysicalMaterial* WaterMaterial = NewObject<UPhysicalMaterial>();
    WaterMaterial->Friction = 0.1f;
    WaterMaterial->Restitution = 0.0f;
    WaterMaterial->Density = 1000.0f;
    TerrainMaterials.Add(ECore_CollisionType::Water, WaterMaterial);
    
    UPhysicalMaterial* LavaMaterial = NewObject<UPhysicalMaterial>();
    LavaMaterial->Friction = 0.3f;
    LavaMaterial->Restitution = 0.0f;
    LavaMaterial->Density = 2000.0f;
    TerrainMaterials.Add(ECore_CollisionType::Lava, LavaMaterial);
    
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Initialized %d physics materials"), TerrainMaterials.Num());
}

void UCore_CollisionManager::SetupCollisionChannels()
{
    // Collision channels are set up in the project settings
    // This function can be extended to programmatically configure collision responses
    UE_LOG(LogTemp, Log, TEXT("Core_CollisionManager: Collision channels configured"));
}

float UCore_CollisionManager::CalculateImpactDamage(const FHitResult& Hit, float Force)
{
    // Simple damage calculation based on impact force and material properties
    float BaseDamage = Force * 0.1f;
    
    FCore_CollisionData* CollisionData = CollisionRegistry.Find(Hit.GetComponent());
    if (CollisionData)
    {
        // Modify damage based on material properties
        BaseDamage *= (2.0f - CollisionData->Restitution);
    }
    
    return FMath::Clamp(BaseDamage, 0.0f, 1000.0f);
}