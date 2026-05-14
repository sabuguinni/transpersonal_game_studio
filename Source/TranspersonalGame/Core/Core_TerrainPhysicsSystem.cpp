#include "Core_TerrainPhysicsSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_TerrainPhysicsSystem::UCore_TerrainPhysicsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for performance
    
    // Initialize default settings
    bEnableTerrainDeformation = true;
    GlobalDeformationThreshold = 2000.0f;
    MaxDeformationRadius = 500.0f;
    MaxDeformationDepth = 50.0f;
    MaxActiveDeformations = 100;
    DeformationUpdateFrequency = 0.1f;
    
    LastDeformationUpdate = 0.0f;
    DeformationsProcessedThisFrame = 0;
    FrameStartTime = 0.0f;
    
    // Initialize default surface properties
    InitializeDefaultSurfaceProperties();
}

void UCore_TerrainPhysicsSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache landscape reference for performance
    if (UWorld* World = GetWorld())
    {
        CachedLandscape = FindLandscapeAtLocation(GetOwner()->GetActorLocation());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsSystem: Initialized with %d surface types"), SurfaceProperties.Num());
}

void UCore_TerrainPhysicsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    FrameStartTime = GetWorld()->GetTimeSeconds();
    DeformationsProcessedThisFrame = 0;
    
    // Update deformations at controlled frequency
    if (bEnableTerrainDeformation && (FrameStartTime - LastDeformationUpdate) >= DeformationUpdateFrequency)
    {
        UpdateDeformations(DeltaTime);
        LastDeformationUpdate = FrameStartTime;
    }
    
    // Cleanup expired deformations
    CleanupExpiredDeformations();
}

void UCore_TerrainPhysicsSystem::InitializeDefaultSurfaceProperties()
{
    // Rock surface
    FCore_TerrainSurfaceProperties RockProperties;
    RockProperties.SurfaceType = ECore_TerrainSurfaceType::Rock;
    RockProperties.Friction = 0.9f;
    RockProperties.Restitution = 0.1f;
    RockProperties.Density = 2500.0f;
    RockProperties.Hardness = 0.9f;
    RockProperties.bCanDeform = false;
    RockProperties.DeformationThreshold = 5000.0f;
    RockProperties.RecoveryRate = 0.1f;
    SurfaceProperties.Add(ECore_TerrainSurfaceType::Rock, RockProperties);
    
    // Dirt surface
    FCore_TerrainSurfaceProperties DirtProperties;
    DirtProperties.SurfaceType = ECore_TerrainSurfaceType::Dirt;
    DirtProperties.Friction = 0.7f;
    DirtProperties.Restitution = 0.3f;
    DirtProperties.Density = 1500.0f;
    DirtProperties.Hardness = 0.4f;
    DirtProperties.bCanDeform = true;
    DirtProperties.DeformationThreshold = 800.0f;
    DirtProperties.RecoveryRate = 2.0f;
    SurfaceProperties.Add(ECore_TerrainSurfaceType::Dirt, DirtProperties);
    
    // Grass surface
    FCore_TerrainSurfaceProperties GrassProperties;
    GrassProperties.SurfaceType = ECore_TerrainSurfaceType::Grass;
    GrassProperties.Friction = 0.6f;
    GrassProperties.Restitution = 0.4f;
    GrassProperties.Density = 1200.0f;
    GrassProperties.Hardness = 0.3f;
    GrassProperties.bCanDeform = true;
    GrassProperties.DeformationThreshold = 600.0f;
    GrassProperties.RecoveryRate = 3.0f;
    SurfaceProperties.Add(ECore_TerrainSurfaceType::Grass, GrassProperties);
    
    // Sand surface
    FCore_TerrainSurfaceProperties SandProperties;
    SandProperties.SurfaceType = ECore_TerrainSurfaceType::Sand;
    SandProperties.Friction = 0.4f;
    SandProperties.Restitution = 0.2f;
    SandProperties.Density = 1600.0f;
    SandProperties.Hardness = 0.2f;
    SandProperties.bCanDeform = true;
    SandProperties.DeformationThreshold = 400.0f;
    SandProperties.RecoveryRate = 1.5f;
    SurfaceProperties.Add(ECore_TerrainSurfaceType::Sand, SandProperties);
    
    // Mud surface
    FCore_TerrainSurfaceProperties MudProperties;
    MudProperties.SurfaceType = ECore_TerrainSurfaceType::Mud;
    MudProperties.Friction = 0.3f;
    MudProperties.Restitution = 0.1f;
    MudProperties.Density = 1800.0f;
    MudProperties.Hardness = 0.1f;
    MudProperties.bCanDeform = true;
    MudProperties.DeformationThreshold = 200.0f;
    MudProperties.RecoveryRate = 0.5f;
    SurfaceProperties.Add(ECore_TerrainSurfaceType::Mud, MudProperties);
    
    // Add more surface types...
    // Snow, Ice, Water, Lava, Wood with appropriate properties
}

ECore_TerrainSurfaceType UCore_TerrainPhysicsSystem::GetSurfaceTypeAtLocation(const FVector& Location)
{
    UPhysicalMaterial* PhysMat = GetPhysicalMaterialAtLocation(Location);
    if (PhysMat && PhysicalMaterialMapping.Contains(PhysMat))
    {
        return PhysicalMaterialMapping[PhysMat];
    }
    
    // Default fallback based on height and slope analysis
    if (CachedLandscape.IsValid())
    {
        FVector LandscapeLocation = CachedLandscape->GetActorLocation();
        float RelativeHeight = Location.Z - LandscapeLocation.Z;
        
        if (RelativeHeight > 1000.0f)
        {
            return ECore_TerrainSurfaceType::Rock; // High altitude = rock
        }
        else if (RelativeHeight < 100.0f)
        {
            return ECore_TerrainSurfaceType::Mud; // Low altitude = mud/water
        }
    }
    
    return ECore_TerrainSurfaceType::Grass; // Default surface type
}

FCore_TerrainSurfaceProperties UCore_TerrainPhysicsSystem::GetSurfacePropertiesAtLocation(const FVector& Location)
{
    ECore_TerrainSurfaceType SurfaceType = GetSurfaceTypeAtLocation(Location);
    
    if (SurfaceProperties.Contains(SurfaceType))
    {
        return SurfaceProperties[SurfaceType];
    }
    
    // Return default properties if not found
    return FCore_TerrainSurfaceProperties();
}

bool UCore_TerrainPhysicsSystem::ApplyTerrainDeformation(const FVector& Location, float Force, float Radius)
{
    if (!bEnableTerrainDeformation || Force < GlobalDeformationThreshold)
    {
        return false;
    }
    
    // Check if we're at max deformations
    if (ActiveDeformations.Num() >= MaxActiveDeformations)
    {
        // Remove oldest deformation
        ActiveDeformations.RemoveAt(0);
    }
    
    // Get surface properties at location
    FCore_TerrainSurfaceProperties SurfaceProps = GetSurfacePropertiesAtLocation(Location);
    
    if (!SurfaceProps.bCanDeform || Force < SurfaceProps.DeformationThreshold)
    {
        return false;
    }
    
    // Calculate deformation parameters
    float ClampedRadius = FMath::Clamp(Radius, 10.0f, MaxDeformationRadius);
    float DeformationDepth = FMath::Clamp((Force / SurfaceProps.DeformationThreshold) * 10.0f, 1.0f, MaxDeformationDepth);
    
    // Create deformation data
    FCore_TerrainDeformationData NewDeformation;
    NewDeformation.Location = Location;
    NewDeformation.Radius = ClampedRadius;
    NewDeformation.Depth = DeformationDepth;
    NewDeformation.Force = Force;
    NewDeformation.Timestamp = GetWorld()->GetTimeSeconds();
    NewDeformation.bIsRecovering = false;
    
    ActiveDeformations.Add(NewDeformation);
    
    // Broadcast deformation event
    OnTerrainDeformation.Broadcast(Location, Force);
    
    UE_LOG(LogTemp, Log, TEXT("Terrain deformation applied at %s with force %.2f, depth %.2f"), 
           *Location.ToString(), Force, DeformationDepth);
    
    return true;
}

void UCore_TerrainPhysicsSystem::ProcessSurfaceImpact(const FVector& Location, float ImpactForce, UPrimitiveComponent* HitComponent)
{
    ECore_TerrainSurfaceType SurfaceType = GetSurfaceTypeAtLocation(Location);
    FCore_TerrainSurfaceProperties SurfaceProps = GetSurfacePropertiesAtLocation(Location);
    
    // Apply physical material properties to the hit component
    if (HitComponent)
    {
        ApplyPhysicalMaterialProperties(HitComponent, SurfaceProps);
    }
    
    // Check for deformation
    if (ImpactForce > SurfaceProps.DeformationThreshold)
    {
        ApplyTerrainDeformation(Location, ImpactForce, 100.0f);
    }
    
    // Broadcast surface impact event
    OnSurfaceImpact.Broadcast(Location, SurfaceType, ImpactForce);
    
    UE_LOG(LogTemp, Log, TEXT("Surface impact processed: Type=%d, Force=%.2f at %s"), 
           (int32)SurfaceType, ImpactForce, *Location.ToString());
}

void UCore_TerrainPhysicsSystem::UpdateDeformations(float DeltaTime)
{
    for (int32 i = ActiveDeformations.Num() - 1; i >= 0; --i)
    {
        if (DeformationsProcessedThisFrame >= 10) // Limit processing per frame
        {
            break;
        }
        
        FCore_TerrainDeformationData& Deformation = ActiveDeformations[i];
        
        // Start recovery after 5 seconds
        float Age = GetWorld()->GetTimeSeconds() - Deformation.Timestamp;
        if (Age > 5.0f && !Deformation.bIsRecovering)
        {
            Deformation.bIsRecovering = true;
        }
        
        if (Deformation.bIsRecovering)
        {
            ProcessDeformationRecovery(Deformation, DeltaTime);
        }
        
        DeformationsProcessedThisFrame++;
    }
}

void UCore_TerrainPhysicsSystem::ProcessDeformationRecovery(FCore_TerrainDeformationData& Deformation, float DeltaTime)
{
    FCore_TerrainSurfaceProperties SurfaceProps = GetSurfacePropertiesAtLocation(Deformation.Location);
    
    // Reduce deformation depth over time
    float RecoveryAmount = SurfaceProps.RecoveryRate * DeltaTime;
    Deformation.Depth = FMath::Max(0.0f, Deformation.Depth - RecoveryAmount);
    
    // Remove if fully recovered
    if (Deformation.Depth <= 0.1f)
    {
        for (int32 i = ActiveDeformations.Num() - 1; i >= 0; --i)
        {
            if (ActiveDeformations[i].Location.Equals(Deformation.Location, 1.0f))
            {
                ActiveDeformations.RemoveAt(i);
                break;
            }
        }
    }
}

UPhysicalMaterial* UCore_TerrainPhysicsSystem::GetPhysicalMaterialAtLocation(const FVector& Location)
{
    // Perform line trace to get physical material
    FHitResult HitResult;
    FVector Start = Location + FVector(0, 0, 100);
    FVector End = Location - FVector(0, 0, 100);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bReturnPhysicalMaterial = true;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams))
    {
        return HitResult.PhysMaterial.Get();
    }
    
    return nullptr;
}

ALandscape* UCore_TerrainPhysicsSystem::FindLandscapeAtLocation(const FVector& Location)
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> LandscapeActors;
        UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
        
        for (AActor* Actor : LandscapeActors)
        {
            if (ALandscape* Landscape = Cast<ALandscape>(Actor))
            {
                FBox LandscapeBounds = Landscape->GetComponentsBoundingBox();
                if (LandscapeBounds.IsInsideXY(Location))
                {
                    return Landscape;
                }
            }
        }
    }
    
    return nullptr;
}

void UCore_TerrainPhysicsSystem::ApplyPhysicalMaterialProperties(UPrimitiveComponent* Component, const FCore_TerrainSurfaceProperties& Properties)
{
    if (!Component || !Component->GetBodyInstance())
    {
        return;
    }
    
    FBodyInstance* BodyInstance = Component->GetBodyInstance();
    
    // Apply friction and restitution
    BodyInstance->SetPhysMaterialOverride(nullptr); // Clear override first
    
    // Note: In a full implementation, you would create and assign a UPhysicalMaterial
    // with the desired properties. This is a simplified version.
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Applied surface properties: Friction=%.2f, Restitution=%.2f"), 
           Properties.Friction, Properties.Restitution);
}

void UCore_TerrainPhysicsSystem::CleanupExpiredDeformations()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveDeformations.Num() - 1; i >= 0; --i)
    {
        const FCore_TerrainDeformationData& Deformation = ActiveDeformations[i];
        float Age = CurrentTime - Deformation.Timestamp;
        
        // Remove deformations older than 60 seconds
        if (Age > 60.0f)
        {
            ActiveDeformations.RemoveAt(i);
        }
    }
}

void UCore_TerrainPhysicsSystem::ClearAllDeformations()
{
    ActiveDeformations.Empty();
    UE_LOG(LogTemp, Warning, TEXT("All terrain deformations cleared"));
}

int32 UCore_TerrainPhysicsSystem::GetActiveDeformationCount() const
{
    return ActiveDeformations.Num();
}

void UCore_TerrainPhysicsSystem::SetSurfaceProperties(ECore_TerrainSurfaceType SurfaceType, const FCore_TerrainSurfaceProperties& Properties)
{
    SurfaceProperties.Add(SurfaceType, Properties);
    UE_LOG(LogTemp, Log, TEXT("Surface properties updated for type %d"), (int32)SurfaceType);
}

bool UCore_TerrainPhysicsSystem::IsLocationDeformed(const FVector& Location, float Tolerance) const
{
    for (const FCore_TerrainDeformationData& Deformation : ActiveDeformations)
    {
        float Distance = FVector::Dist(Location, Deformation.Location);
        if (Distance <= (Deformation.Radius + Tolerance))
        {
            return true;
        }
    }
    
    return false;
}

void UCore_TerrainPhysicsSystem::ValidatePhysicalMaterialMappings()
{
    int32 ValidMappings = 0;
    
    for (const auto& Mapping : PhysicalMaterialMapping)
    {
        if (Mapping.Key && SurfaceProperties.Contains(Mapping.Value))
        {
            ValidMappings++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Physical Material Mappings Validated: %d/%d valid"), 
           ValidMappings, PhysicalMaterialMapping.Num());
}

// ACore_TerrainPhysicsManager Implementation

ACore_TerrainPhysicsManager::ACore_TerrainPhysicsManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
    
    // Create terrain physics system component
    TerrainPhysicsSystem = CreateDefaultSubobject<UCore_TerrainPhysicsSystem>(TEXT("TerrainPhysicsSystem"));
    
    bAutoRegisterWithGameMode = true;
    bProcessGlobalTerrainEvents = true;
}

void ACore_TerrainPhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind to terrain physics events
    if (TerrainPhysicsSystem)
    {
        TerrainPhysicsSystem->OnTerrainDeformation.AddDynamic(this, &ACore_TerrainPhysicsManager::OnTerrainDeformationReceived);
        TerrainPhysicsSystem->OnSurfaceImpact.AddDynamic(this, &ACore_TerrainPhysicsManager::OnSurfaceImpactReceived);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsManager initialized and ready"));
}

void ACore_TerrainPhysicsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Process global terrain events if enabled
    if (bProcessGlobalTerrainEvents)
    {
        // Cleanup invalid registered actors
        for (int32 i = RegisteredTerrainActors.Num() - 1; i >= 0; --i)
        {
            if (!RegisteredTerrainActors[i].IsValid())
            {
                RegisteredTerrainActors.RemoveAt(i);
            }
        }
    }
}

ACore_TerrainPhysicsManager* ACore_TerrainPhysicsManager::GetTerrainPhysicsManager(const UObject* WorldContext)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        TArray<AActor*> FoundManagers;
        UGameplayStatics::GetAllActorsOfClass(World, ACore_TerrainPhysicsManager::StaticClass(), FoundManagers);
        
        if (FoundManagers.Num() > 0)
        {
            return Cast<ACore_TerrainPhysicsManager>(FoundManagers[0]);
        }
    }
    
    return nullptr;
}

void ACore_TerrainPhysicsManager::RegisterTerrainActor(AActor* TerrainActor)
{
    if (TerrainActor && !RegisteredTerrainActors.Contains(TerrainActor))
    {
        RegisteredTerrainActors.Add(TerrainActor);
        UE_LOG(LogTemp, Log, TEXT("Terrain actor registered: %s"), *TerrainActor->GetName());
    }
}

void ACore_TerrainPhysicsManager::UnregisterTerrainActor(AActor* TerrainActor)
{
    if (TerrainActor)
    {
        RegisteredTerrainActors.Remove(TerrainActor);
        UE_LOG(LogTemp, Log, TEXT("Terrain actor unregistered: %s"), *TerrainActor->GetName());
    }
}

void ACore_TerrainPhysicsManager::ProcessGlobalTerrainImpact(const FVector& Location, float Force, AActor* Instigator)
{
    if (TerrainPhysicsSystem)
    {
        TerrainPhysicsSystem->ProcessSurfaceImpact(Location, Force, nullptr);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Global terrain impact processed at %s with force %.2f"), 
           *Location.ToString(), Force);
}

void ACore_TerrainPhysicsManager::OnTerrainDeformationReceived(FVector Location, float Force)
{
    // Handle global deformation events
    UE_LOG(LogTemp, VeryVerbose, TEXT("Global deformation event: %s, Force: %.2f"), 
           *Location.ToString(), Force);
}

void ACore_TerrainPhysicsManager::OnSurfaceImpactReceived(FVector Location, ECore_TerrainSurfaceType SurfaceType, float ImpactForce)
{
    // Handle global surface impact events
    UE_LOG(LogTemp, VeryVerbose, TEXT("Global surface impact: Type=%d, Force=%.2f at %s"), 
           (int32)SurfaceType, ImpactForce, *Location.ToString());
}