#include "Core_TerrainPhysicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Engine/CollisionProfile.h"

UCore_TerrainPhysicsManager::UCore_TerrainPhysicsManager()
{
    // Initialize default terrain physics settings
    TerrainDensity = 2500.0f; // kg/m³ - typical rock density
    TerrainFriction = 0.7f;
    TerrainRestitution = 0.1f;
    TerrainLinearDamping = 0.5f;
    TerrainAngularDamping = 0.8f;
    
    // Slope physics
    MaxWalkableSlope = 45.0f;
    SlopeSlipThreshold = 60.0f;
    SlopeSlipForce = 980.0f; // cm/s² gravity equivalent
    
    // Erosion simulation
    bEnableErosionSimulation = false;
    ErosionRate = 0.01f;
    ErosionThreshold = 30.0f;
    
    // Stability settings
    StabilityCheckRadius = 500.0f;
    MinSupportAngle = 30.0f;
    
    // Initialize terrain type data
    InitializeTerrainTypeData();
}

void UCore_TerrainPhysicsManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Initializing terrain physics system"));
    
    // Setup physics delegates
    if (UWorld* World = GetWorld())
    {
        // Register for physics tick
        World->GetTimerManager().SetTimer(
            PhysicsTickHandle,
            this,
            &UCore_TerrainPhysicsManager::TickTerrainPhysics,
            0.1f, // 10Hz update rate
            true
        );
        
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Physics tick registered"));
    }
    
    // Initialize terrain collision profiles
    SetupTerrainCollisionProfiles();
}

void UCore_TerrainPhysicsManager::Deinitialize()
{
    // Clear physics timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PhysicsTickHandle);
    }
    
    // Clear tracked objects
    TrackedTerrainObjects.Empty();
    
    Super::Deinitialize();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsManager: Deinitialized"));
}

void UCore_TerrainPhysicsManager::ApplyTerrainPhysicsToActor(AActor* Actor, ECore_TerrainType TerrainType)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysicsManager: Null actor passed to ApplyTerrainPhysicsToActor"));
        return;
    }
    
    // Get terrain properties
    FCore_TerrainPhysicsProperties TerrainProps = GetTerrainProperties(TerrainType);
    
    // Apply to all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PrimComp->GetBodyInstance())
        {
            FBodyInstance* BodyInstance = PrimComp->GetBodyInstance();
            
            // Apply physics properties
            BodyInstance->SetMassOverride(TerrainProps.Density * 1000.0f); // Convert to grams
            BodyInstance->SetLinearDamping(TerrainProps.LinearDamping);
            BodyInstance->SetAngularDamping(TerrainProps.AngularDamping);
            
            // Set surface properties
            PrimComp->SetPhysMaterialOverride(nullptr); // Use default for now
            
            // Set collision profile based on terrain type
            FName CollisionProfile = GetTerrainCollisionProfile(TerrainType);
            PrimComp->SetCollisionProfileName(CollisionProfile);
            
            UE_LOG(LogTemp, Log, TEXT("Applied terrain physics to %s: Type=%d, Density=%.2f"), 
                   *Actor->GetName(), (int32)TerrainType, TerrainProps.Density);
        }
    }
    
    // Track this actor for ongoing physics updates
    TrackedTerrainObjects.AddUnique(Actor);
}

FCore_TerrainPhysicsProperties UCore_TerrainPhysicsManager::GetTerrainProperties(ECore_TerrainType TerrainType) const
{
    if (const FCore_TerrainPhysicsProperties* Props = TerrainTypeProperties.Find(TerrainType))
    {
        return *Props;
    }
    
    // Return default properties if not found
    FCore_TerrainPhysicsProperties DefaultProps;
    DefaultProps.Density = TerrainDensity;
    DefaultProps.Friction = TerrainFriction;
    DefaultProps.Restitution = TerrainRestitution;
    DefaultProps.LinearDamping = TerrainLinearDamping;
    DefaultProps.AngularDamping = TerrainAngularDamping;
    DefaultProps.Hardness = 0.5f;
    DefaultProps.Porosity = 0.3f;
    
    return DefaultProps;
}

bool UCore_TerrainPhysicsManager::IsWalkableSlope(const FVector& SurfaceNormal) const
{
    // Calculate slope angle from surface normal
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector)));
    
    return SlopeAngle <= MaxWalkableSlope;
}

FVector UCore_TerrainPhysicsManager::CalculateSlopeForce(const FVector& SurfaceNormal, float ObjectMass) const
{
    // Calculate slope angle
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector)));
    
    if (SlopeAngle > SlopeSlipThreshold)
    {
        // Calculate slip direction (down the slope)
        FVector SlipDirection = FVector::UpVector - (FVector::DotProduct(FVector::UpVector, SurfaceNormal) * SurfaceNormal);
        SlipDirection.Normalize();
        
        // Calculate slip force magnitude
        float SlipForceMagnitude = ObjectMass * SlopeSlipForce * FMath::Sin(FMath::DegreesToRadians(SlopeAngle));
        
        return SlipDirection * SlipForceMagnitude;
    }
    
    return FVector::ZeroVector;
}

void UCore_TerrainPhysicsManager::SimulateTerrainErosion(const FVector& Location, float Intensity, float Radius)
{
    if (!bEnableErosionSimulation)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Simulating terrain erosion at %s, Intensity=%.2f, Radius=%.2f"), 
           *Location.ToString(), Intensity, Radius);
    
    // Find landscape actors in the area
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (ALandscape* Landscape = Cast<ALandscape>(Actor))
            {
                // Apply erosion to landscape (simplified simulation)
                ApplyErosionToLandscape(Landscape, Location, Intensity, Radius);
            }
        }
    }
}

bool UCore_TerrainPhysicsManager::CheckTerrainStability(const FVector& Location, float CheckRadius) const
{
    // Perform stability analysis around the given location
    if (UWorld* World = GetWorld())
    {
        // Check slope angles in the area
        TArray<FVector> CheckPoints;
        int32 NumCheckPoints = 8;
        
        for (int32 i = 0; i < NumCheckPoints; i++)
        {
            float Angle = (2.0f * PI * i) / NumCheckPoints;
            FVector CheckPoint = Location + FVector(
                FMath::Cos(Angle) * CheckRadius,
                FMath::Sin(Angle) * CheckRadius,
                0.0f
            );
            CheckPoints.Add(CheckPoint);
        }
        
        // Perform line traces to get surface normals
        int32 StablePoints = 0;
        for (const FVector& CheckPoint : CheckPoints)
        {
            FHitResult HitResult;
            FVector TraceStart = CheckPoint + FVector(0, 0, 1000.0f);
            FVector TraceEnd = CheckPoint - FVector(0, 0, 1000.0f);
            
            if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
            {
                if (IsWalkableSlope(HitResult.Normal))
                {
                    StablePoints++;
                }
            }
        }
        
        // Consider stable if majority of check points are stable
        return (float)StablePoints / NumCheckPoints >= 0.6f;
    }
    
    return true; // Default to stable if we can't check
}

void UCore_TerrainPhysicsManager::DebugDrawTerrainPhysics()
{
    if (UWorld* World = GetWorld())
    {
        // Draw debug info for tracked terrain objects
        for (TWeakObjectPtr<AActor> ActorPtr : TrackedTerrainObjects)
        {
            if (AActor* Actor = ActorPtr.Get())
            {
                FVector ActorLocation = Actor->GetActorLocation();
                
                // Draw actor bounds
                FBox ActorBounds = Actor->GetComponentsBoundingBox();
                DrawDebugBox(World, ActorBounds.GetCenter(), ActorBounds.GetExtent(), 
                           FColor::Green, false, 1.0f, 0, 2.0f);
                
                // Draw stability info
                bool bIsStable = CheckTerrainStability(ActorLocation, StabilityCheckRadius);
                FColor StabilityColor = bIsStable ? FColor::Green : FColor::Red;
                
                DrawDebugSphere(World, ActorLocation, 50.0f, 12, StabilityColor, false, 1.0f, 0, 3.0f);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Debug draw complete for %d terrain objects"), TrackedTerrainObjects.Num());
    }
}

void UCore_TerrainPhysicsManager::ValidateTerrainPhysics()
{
    int32 ValidObjects = 0;
    int32 InvalidObjects = 0;
    
    for (int32 i = TrackedTerrainObjects.Num() - 1; i >= 0; i--)
    {
        if (TrackedTerrainObjects[i].IsValid())
        {
            ValidObjects++;
        }
        else
        {
            TrackedTerrainObjects.RemoveAt(i);
            InvalidObjects++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Terrain Physics Validation: %d valid, %d invalid objects removed"), 
           ValidObjects, InvalidObjects);
}

void UCore_TerrainPhysicsManager::TickTerrainPhysics()
{
    // Update physics for all tracked terrain objects
    for (TWeakObjectPtr<AActor> ActorPtr : TrackedTerrainObjects)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            UpdateActorTerrainPhysics(Actor);
        }
    }
}

void UCore_TerrainPhysicsManager::InitializeTerrainTypeData()
{
    // Rock terrain
    FCore_TerrainPhysicsProperties RockProps;
    RockProps.Density = 2700.0f;
    RockProps.Friction = 0.8f;
    RockProps.Restitution = 0.1f;
    RockProps.LinearDamping = 0.3f;
    RockProps.AngularDamping = 0.5f;
    RockProps.Hardness = 0.9f;
    RockProps.Porosity = 0.1f;
    TerrainTypeProperties.Add(ECore_TerrainType::Rock, RockProps);
    
    // Soil terrain
    FCore_TerrainPhysicsProperties SoilProps;
    SoilProps.Density = 1500.0f;
    SoilProps.Friction = 0.6f;
    SoilProps.Restitution = 0.05f;
    SoilProps.LinearDamping = 0.8f;
    SoilProps.AngularDamping = 0.9f;
    SoilProps.Hardness = 0.3f;
    SoilProps.Porosity = 0.5f;
    TerrainTypeProperties.Add(ECore_TerrainType::Soil, SoilProps);
    
    // Sand terrain
    FCore_TerrainPhysicsProperties SandProps;
    SandProps.Density = 1600.0f;
    SandProps.Friction = 0.4f;
    SandProps.Restitution = 0.02f;
    SandProps.LinearDamping = 1.2f;
    SandProps.AngularDamping = 1.5f;
    SandProps.Hardness = 0.2f;
    SandProps.Porosity = 0.4f;
    TerrainTypeProperties.Add(ECore_TerrainType::Sand, SandProps);
    
    // Mud terrain
    FCore_TerrainPhysicsProperties MudProps;
    MudProps.Density = 1800.0f;
    MudProps.Friction = 0.3f;
    MudProps.Restitution = 0.01f;
    MudProps.LinearDamping = 2.0f;
    MudProps.AngularDamping = 2.5f;
    MudProps.Hardness = 0.1f;
    MudProps.Porosity = 0.7f;
    TerrainTypeProperties.Add(ECore_TerrainType::Mud, MudProps);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d terrain type configurations"), TerrainTypeProperties.Num());
}

void UCore_TerrainPhysicsManager::SetupTerrainCollisionProfiles()
{
    // Setup collision profiles for different terrain types
    // This would typically be done through project settings, but we can log the requirements
    
    UE_LOG(LogTemp, Warning, TEXT("Terrain collision profiles should be configured in Project Settings:"));
    UE_LOG(LogTemp, Warning, TEXT("- TerrainRock: BlockAll, high friction"));
    UE_LOG(LogTemp, Warning, TEXT("- TerrainSoil: BlockAll, medium friction"));
    UE_LOG(LogTemp, Warning, TEXT("- TerrainSand: BlockAll, low friction"));
    UE_LOG(LogTemp, Warning, TEXT("- TerrainMud: BlockAll, very low friction"));
}

FName UCore_TerrainPhysicsManager::GetTerrainCollisionProfile(ECore_TerrainType TerrainType) const
{
    switch (TerrainType)
    {
        case ECore_TerrainType::Rock:
            return FName("TerrainRock");
        case ECore_TerrainType::Soil:
            return FName("TerrainSoil");
        case ECore_TerrainType::Sand:
            return FName("TerrainSand");
        case ECore_TerrainType::Mud:
            return FName("TerrainMud");
        default:
            return FName("BlockAll");
    }
}

void UCore_TerrainPhysicsManager::UpdateActorTerrainPhysics(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    // Check if actor is on unstable terrain
    FVector ActorLocation = Actor->GetActorLocation();
    bool bIsStable = CheckTerrainStability(ActorLocation, StabilityCheckRadius);
    
    if (!bIsStable)
    {
        // Apply instability effects
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                // Add small random forces to simulate instability
                FVector InstabilityForce = FVector(
                    FMath::RandRange(-100.0f, 100.0f),
                    FMath::RandRange(-100.0f, 100.0f),
                    0.0f
                );
                
                PrimComp->AddForce(InstabilityForce);
            }
        }
    }
}

void UCore_TerrainPhysicsManager::ApplyErosionToLandscape(ALandscape* Landscape, const FVector& Location, float Intensity, float Radius)
{
    if (!Landscape)
    {
        return;
    }
    
    // This is a simplified erosion simulation
    // In a full implementation, this would modify the landscape heightmap
    
    UE_LOG(LogTemp, Log, TEXT("Applied erosion to landscape %s at %s (Intensity=%.2f, Radius=%.2f)"), 
           *Landscape->GetName(), *Location.ToString(), Intensity, Radius);
    
    // For now, just log the erosion event
    // Real implementation would use Landscape->GetLandscapeInfo() and modify heightmaps
}