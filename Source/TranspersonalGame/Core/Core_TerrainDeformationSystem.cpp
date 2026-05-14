#include "Core_TerrainDeformationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Landscape/LandscapeProxy.h"
#include "Landscape/LandscapeInfo.h"
#include "Landscape/LandscapeDataAccess.h"
#include "Landscape/LandscapeEdit.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

UCore_TerrainDeformationSystem::UCore_TerrainDeformationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for terrain processing
    
    // Initialize default settings
    MaxDeformationRadius = 500.0f;
    MinDeformationForce = 100.0f;
    DeformationFalloffExponent = 2.0f;
    MaxConcurrentDeformations = 50;
    
    bEnableTerrainRecovery = true;
    GlobalRecoveryRate = 0.1f;
    RecoveryTickInterval = 1.0f;
    
    MaxHeightmapUpdatesPerFrame = 10;
    MinDeformationThreshold = 1.0f;
    
    CachedLandscape = nullptr;
    LandscapeInfo = nullptr;
    LastRecoveryTime = 0.0f;
}

void UCore_TerrainDeformationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find landscape in the world
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<ALandscapeProxy> ActorItr(World); ActorItr; ++ActorItr)
        {
            CachedLandscape = *ActorItr;
            if (CachedLandscape)
            {
                LandscapeInfo = CachedLandscape->GetLandscapeInfo();
                UE_LOG(LogTemp, Warning, TEXT("Core_TerrainDeformationSystem: Found landscape %s"), *CachedLandscape->GetName());
                break;
            }
        }
    }
    
    // Initialize terrain materials
    InitializeTerrainMaterials();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainDeformationSystem: Initialized with %d terrain materials"), TerrainMaterials.Num());
}

void UCore_TerrainDeformationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Process pending deformations
    int32 ProcessedThisFrame = 0;
    for (int32 i = PendingDeformations.Num() - 1; i >= 0 && ProcessedThisFrame < MaxHeightmapUpdatesPerFrame; i--)
    {
        if (ShouldProcessDeformation(PendingDeformations[i]))
        {
            ModifyLandscapeHeightmap(PendingDeformations[i]);
            ActiveDeformations.Add(PendingDeformations[i]);
            PendingDeformations.RemoveAt(i);
            ProcessedThisFrame++;
        }
    }
    
    // Process terrain recovery
    if (bEnableTerrainRecovery)
    {
        ProcessTerrainRecovery(DeltaTime);
    }
    
    // Optimize deformation queue
    OptimizeDeformationQueue();
}

void UCore_TerrainDeformationSystem::ApplyDeformation(const FCore_DeformationEvent& DeformationEvent)
{
    if (!CachedLandscape || !LandscapeInfo)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainDeformationSystem: No landscape found for deformation"));
        return;
    }
    
    // Validate deformation parameters
    if (DeformationEvent.Force < MinDeformationForce)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainDeformationSystem: Deformation force too low: %f"), DeformationEvent.Force);
        return;
    }
    
    if (DeformationEvent.Radius > MaxDeformationRadius)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainDeformationSystem: Deformation radius too large: %f"), DeformationEvent.Radius);
        return;
    }
    
    // Check if we can deform at this location
    if (!CanDeformAtLocation(DeformationEvent.ImpactLocation, DeformationEvent.Force))
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainDeformationSystem: Cannot deform at location due to terrain hardness"));
        return;
    }
    
    // Add to pending deformations queue
    FCore_DeformationEvent ProcessedEvent = DeformationEvent;
    ProcessedEvent.TimeStamp = GetWorld()->GetTimeSeconds();
    
    PendingDeformations.Add(ProcessedEvent);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationSystem: Queued deformation at %s with force %f"), 
           *DeformationEvent.ImpactLocation.ToString(), DeformationEvent.Force);
}

void UCore_TerrainDeformationSystem::ApplyExplosionDeformation(FVector Location, float ExplosionForce, float BlastRadius)
{
    FCore_DeformationEvent ExplosionEvent;
    ExplosionEvent.ImpactLocation = Location;
    ExplosionEvent.Force = ExplosionForce;
    ExplosionEvent.Radius = BlastRadius;
    ExplosionEvent.Depth = FMath::Clamp(ExplosionForce * 0.1f, 10.0f, 300.0f);
    ExplosionEvent.TerrainType = GetTerrainTypeAtLocation(Location);
    
    ApplyDeformation(ExplosionEvent);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationSystem: Applied explosion deformation - Force: %f, Radius: %f"), 
           ExplosionForce, BlastRadius);
}

void UCore_TerrainDeformationSystem::ApplyFootstepDeformation(FVector Location, float Weight, float FootSize)
{
    FCore_DeformationEvent FootstepEvent;
    FootstepEvent.ImpactLocation = Location;
    FootstepEvent.Force = Weight * 0.5f; // Reduce force for footsteps
    FootstepEvent.Radius = FootSize;
    FootstepEvent.Depth = FMath::Clamp(Weight * 0.01f, 1.0f, 10.0f);
    FootstepEvent.TerrainType = GetTerrainTypeAtLocation(Location);
    
    ApplyDeformation(FootstepEvent);
}

void UCore_TerrainDeformationSystem::ApplyVehicleDeformation(FVector Location, float VehicleWeight, float TireWidth)
{
    FCore_DeformationEvent VehicleEvent;
    VehicleEvent.ImpactLocation = Location;
    VehicleEvent.Force = VehicleWeight * 2.0f; // Vehicles create more pressure
    VehicleEvent.Radius = TireWidth * 2.0f;
    VehicleEvent.Depth = FMath::Clamp(VehicleWeight * 0.05f, 5.0f, 50.0f);
    VehicleEvent.TerrainType = GetTerrainTypeAtLocation(Location);
    
    ApplyDeformation(VehicleEvent);
}

ECore_TerrainType UCore_TerrainDeformationSystem::GetTerrainTypeAtLocation(FVector Location)
{
    if (!CachedLandscape || !LandscapeInfo)
    {
        return ECore_TerrainType::Dirt;
    }
    
    // Sample landscape material at location
    // For now, return based on height - this could be enhanced with actual material sampling
    float Height = Location.Z;
    
    if (Height > 1000.0f)
    {
        return ECore_TerrainType::Rock;
    }
    else if (Height < -100.0f)
    {
        return ECore_TerrainType::Mud;
    }
    else if (Height < 100.0f)
    {
        return ECore_TerrainType::Sand;
    }
    else
    {
        return ECore_TerrainType::Dirt;
    }
}

float UCore_TerrainDeformationSystem::GetTerrainHardnessAtLocation(FVector Location)
{
    ECore_TerrainType TerrainType = GetTerrainTypeAtLocation(Location);
    FCore_TerrainMaterial* Material = TerrainMaterials.Find(TerrainType);
    
    if (Material)
    {
        return Material->Hardness;
    }
    
    return 1.0f; // Default hardness
}

bool UCore_TerrainDeformationSystem::CanDeformAtLocation(FVector Location, float RequiredForce)
{
    float TerrainHardness = GetTerrainHardnessAtLocation(Location);
    FCore_TerrainMaterial Material = GetMaterialProperties(GetTerrainTypeAtLocation(Location));
    
    float RequiredForceForDeformation = Material.DeformationResistance * TerrainHardness;
    
    return RequiredForce >= RequiredForceForDeformation;
}

void UCore_TerrainDeformationSystem::ProcessTerrainRecovery(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastRecoveryTime < RecoveryTickInterval)
    {
        return;
    }
    
    LastRecoveryTime = CurrentTime;
    
    // Process recovery for active deformations
    for (int32 i = ActiveDeformations.Num() - 1; i >= 0; i--)
    {
        ProcessSingleDeformationRecovery(ActiveDeformations[i], DeltaTime);
        
        // Remove fully recovered deformations
        if (ActiveDeformations[i].Depth <= MinDeformationThreshold)
        {
            ActiveDeformations.RemoveAt(i);
        }
    }
}

void UCore_TerrainDeformationSystem::SetRecoveryEnabled(bool bEnabled)
{
    bEnableTerrainRecovery = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationSystem: Terrain recovery %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_TerrainDeformationSystem::DebugDrawDeformationZones()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Draw active deformations
    for (const FCore_DeformationEvent& Event : ActiveDeformations)
    {
        DrawDebugSphere(GetWorld(), Event.ImpactLocation, Event.Radius, 12, FColor::Red, false, 5.0f);
        DrawDebugString(GetWorld(), Event.ImpactLocation + FVector(0, 0, 100), 
                       FString::Printf(TEXT("Force: %.1f\nDepth: %.1f"), Event.Force, Event.Depth), 
                       nullptr, FColor::White, 5.0f);
    }
    
    // Draw pending deformations
    for (const FCore_DeformationEvent& Event : PendingDeformations)
    {
        DrawDebugSphere(GetWorld(), Event.ImpactLocation, Event.Radius, 12, FColor::Yellow, false, 5.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationSystem: Drew %d active and %d pending deformations"), 
           ActiveDeformations.Num(), PendingDeformations.Num());
}

void UCore_TerrainDeformationSystem::ValidateTerrainSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TERRAIN DEFORMATION SYSTEM VALIDATION ==="));
    
    if (CachedLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ Landscape found: %s"), *CachedLandscape->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ No landscape found"));
    }
    
    if (LandscapeInfo)
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ LandscapeInfo available"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ No LandscapeInfo"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Active deformations: %d"), ActiveDeformations.Num());
    UE_LOG(LogTemp, Warning, TEXT("Pending deformations: %d"), PendingDeformations.Num());
    UE_LOG(LogTemp, Warning, TEXT("Terrain materials: %d"), TerrainMaterials.Num());
    UE_LOG(LogTemp, Warning, TEXT("Recovery enabled: %s"), bEnableTerrainRecovery ? TEXT("Yes") : TEXT("No"));
}

void UCore_TerrainDeformationSystem::ModifyLandscapeHeightmap(const FCore_DeformationEvent& Event)
{
    if (!CachedLandscape || !LandscapeInfo)
    {
        return;
    }
    
    // Calculate affected heightmap region
    FVector2D LandscapeLocation = FVector2D(Event.ImpactLocation.X, Event.ImpactLocation.Y);
    float RadiusInUnits = Event.Radius;
    
    // For now, log the deformation - actual heightmap modification requires more complex landscape API usage
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationSystem: Processing heightmap deformation at %s"), 
           *Event.ImpactLocation.ToString());
    
    // TODO: Implement actual heightmap modification using FLandscapeEditDataInterface
    // This would require more complex integration with UE5's landscape editing system
}

void UCore_TerrainDeformationSystem::CalculateDeformationShape(const FCore_DeformationEvent& Event, TArray<FVector2D>& OutPoints, TArray<float>& OutHeights)
{
    OutPoints.Empty();
    OutHeights.Empty();
    
    // Generate circular deformation pattern
    int32 NumPoints = 16;
    float AngleStep = 2.0f * PI / NumPoints;
    
    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = i * AngleStep;
        float X = Event.ImpactLocation.X + FMath::Cos(Angle) * Event.Radius;
        float Y = Event.ImpactLocation.Y + FMath::Sin(Angle) * Event.Radius;
        
        OutPoints.Add(FVector2D(X, Y));
        
        // Calculate height based on distance from center with falloff
        float Distance = FMath::Sqrt(X * X + Y * Y);
        float NormalizedDistance = Distance / Event.Radius;
        float Falloff = FMath::Pow(1.0f - NormalizedDistance, DeformationFalloffExponent);
        float Height = Event.Depth * Falloff;
        
        OutHeights.Add(Height);
    }
}

void UCore_TerrainDeformationSystem::InitializeTerrainMaterials()
{
    TerrainMaterials.Empty();
    
    // Initialize dirt material
    FCore_TerrainMaterial DirtMaterial;
    DirtMaterial.MaterialType = ECore_TerrainType::Dirt;
    DirtMaterial.Hardness = 1.0f;
    DirtMaterial.DeformationResistance = 100.0f;
    DirtMaterial.RecoveryRate = 0.2f;
    DirtMaterial.MaxDeformation = 150.0f;
    TerrainMaterials.Add(ECore_TerrainType::Dirt, DirtMaterial);
    
    // Initialize sand material
    FCore_TerrainMaterial SandMaterial;
    SandMaterial.MaterialType = ECore_TerrainType::Sand;
    SandMaterial.Hardness = 0.5f;
    SandMaterial.DeformationResistance = 50.0f;
    SandMaterial.RecoveryRate = 0.1f;
    SandMaterial.MaxDeformation = 200.0f;
    TerrainMaterials.Add(ECore_TerrainType::Sand, SandMaterial);
    
    // Initialize rock material
    FCore_TerrainMaterial RockMaterial;
    RockMaterial.MaterialType = ECore_TerrainType::Rock;
    RockMaterial.Hardness = 5.0f;
    RockMaterial.DeformationResistance = 500.0f;
    RockMaterial.RecoveryRate = 0.0f; // Rocks don't recover
    RockMaterial.MaxDeformation = 50.0f;
    TerrainMaterials.Add(ECore_TerrainType::Rock, RockMaterial);
    
    // Initialize mud material
    FCore_TerrainMaterial MudMaterial;
    MudMaterial.MaterialType = ECore_TerrainType::Mud;
    MudMaterial.Hardness = 0.3f;
    MudMaterial.DeformationResistance = 30.0f;
    MudMaterial.RecoveryRate = 0.5f;
    MudMaterial.MaxDeformation = 300.0f;
    TerrainMaterials.Add(ECore_TerrainType::Mud, MudMaterial);
}

FCore_TerrainMaterial UCore_TerrainDeformationSystem::GetMaterialProperties(ECore_TerrainType TerrainType)
{
    FCore_TerrainMaterial* Material = TerrainMaterials.Find(TerrainType);
    
    if (Material)
    {
        return *Material;
    }
    
    // Return default material if not found
    FCore_TerrainMaterial DefaultMaterial;
    return DefaultMaterial;
}

void UCore_TerrainDeformationSystem::OptimizeDeformationQueue()
{
    // Remove old pending deformations that are too old
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float MaxPendingTime = 5.0f;
    
    for (int32 i = PendingDeformations.Num() - 1; i >= 0; i--)
    {
        if (CurrentTime - PendingDeformations[i].TimeStamp > MaxPendingTime)
        {
            PendingDeformations.RemoveAt(i);
        }
    }
    
    // Limit total deformations
    if (ActiveDeformations.Num() > MaxConcurrentDeformations)
    {
        int32 ToRemove = ActiveDeformations.Num() - MaxConcurrentDeformations;
        ActiveDeformations.RemoveAt(0, ToRemove);
    }
}

bool UCore_TerrainDeformationSystem::ShouldProcessDeformation(const FCore_DeformationEvent& Event)
{
    // Check if deformation is significant enough
    if (Event.Depth < MinDeformationThreshold)
    {
        return false;
    }
    
    // Check if we're not over the concurrent limit
    if (ActiveDeformations.Num() >= MaxConcurrentDeformations)
    {
        return false;
    }
    
    return true;
}

void UCore_TerrainDeformationSystem::ProcessSingleDeformationRecovery(FCore_DeformationEvent& Event, float DeltaTime)
{
    FCore_TerrainMaterial Material = GetMaterialProperties(Event.TerrainType);
    
    if (Material.RecoveryRate <= 0.0f)
    {
        return; // No recovery for this material type
    }
    
    float RecoveryAmount = Material.RecoveryRate * GlobalRecoveryRate * DeltaTime;
    Event.Depth = FMath::Max(0.0f, Event.Depth - RecoveryAmount);
    
    // Update heightmap if significant recovery occurred
    if (RecoveryAmount > MinDeformationThreshold)
    {
        ModifyLandscapeHeightmap(Event);
    }
}

void UCore_TerrainDeformationSystem::DrawDeformationDebugInfo(const FCore_DeformationEvent& Event)
{
    if (!GetWorld())
    {
        return;
    }
    
    FColor DebugColor = FColor::Red;
    switch (Event.TerrainType)
    {
        case ECore_TerrainType::Dirt: DebugColor = FColor::Brown; break;
        case ECore_TerrainType::Sand: DebugColor = FColor::Yellow; break;
        case ECore_TerrainType::Rock: DebugColor = FColor::Black; break;
        case ECore_TerrainType::Mud: DebugColor = FColor::Orange; break;
        default: DebugColor = FColor::Red; break;
    }
    
    DrawDebugSphere(GetWorld(), Event.ImpactLocation, Event.Radius, 12, DebugColor, false, 1.0f);
}

void UCore_TerrainDeformationSystem::LogDeformationStats()
{
    UE_LOG(LogTemp, Log, TEXT("=== TERRAIN DEFORMATION STATS ==="));
    UE_LOG(LogTemp, Log, TEXT("Active deformations: %d"), ActiveDeformations.Num());
    UE_LOG(LogTemp, Log, TEXT("Pending deformations: %d"), PendingDeformations.Num());
    UE_LOG(LogTemp, Log, TEXT("Recovery enabled: %s"), bEnableTerrainRecovery ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Max concurrent: %d"), MaxConcurrentDeformations);
}