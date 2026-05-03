#include "Core_TerrainInteractionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_TerrainInteractionSystem::UCore_TerrainInteractionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for terrain checks
    
    // Initialize default values
    MaxInteractionDistance = 200.0f;
    TerrainCheckFrequency = 10.0f;
    bEnableTerrainDeformation = true;
    bEnableDetailedSurfaceDetection = true;
    
    TerrainCheckTimer = 0.0f;
}

void UCore_TerrainInteractionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSurfaceProperties();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainInteractionSystem: Initialized for actor %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_TerrainInteractionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TerrainCheckTimer += DeltaTime;
    
    // Perform terrain checks at specified frequency
    if (TerrainCheckTimer >= (1.0f / TerrainCheckFrequency))
    {
        TerrainCheckTimer = 0.0f;
        
        AActor* Owner = GetOwner();
        if (Owner)
        {
            FVector ActorLocation = Owner->GetActorLocation();
            ECore_TerrainType CurrentTerrain = GetTerrainTypeAtLocation(ActorLocation);
            
            // Apply environmental effects
            ApplyEnvironmentalEffects(Owner, CurrentTerrain, DeltaTime);
            
            // Broadcast terrain interaction event
            OnTerrainInteraction.Broadcast(Owner, ActorLocation, CurrentTerrain);
        }
        
        // Clean cache periodically
        CleanCache();
    }
}

ECore_TerrainType UCore_TerrainInteractionSystem::GetTerrainTypeAtLocation(const FVector& WorldLocation) const
{
    // Check cache first
    FVector CacheKey = FVector(
        FMath::RoundToFloat(WorldLocation.X / 100.0f) * 100.0f,
        FMath::RoundToFloat(WorldLocation.Y / 100.0f) * 100.0f,
        0.0f
    );
    
    if (const ECore_TerrainType* CachedType = TerrainTypeCache.Find(CacheKey))
    {
        return *CachedType;
    }
    
    // Perform trace to determine terrain type
    FHitResult HitResult;
    FVector TraceStart = WorldLocation + FVector(0, 0, 100);
    FVector TraceEnd = WorldLocation - FVector(0, 0, MaxInteractionDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());
    
    ECore_TerrainType DetectedType = ECore_TerrainType::Plains;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        if (HitResult.GetActor())
        {
            // Check if hit a landscape
            if (ALandscape* Landscape = Cast<ALandscape>(HitResult.GetActor()))
            {
                DetectedType = CalculateTerrainTypeFromLandscape(HitResult.Location);
            }
            // Check for water bodies
            else if (HitResult.GetActor()->GetName().Contains(TEXT("Water")))
            {
                DetectedType = ECore_TerrainType::Swamp;
            }
            // Check for rocky terrain
            else if (HitResult.GetActor()->GetName().Contains(TEXT("Rock")) || 
                     HitResult.GetActor()->GetName().Contains(TEXT("Mountain")))
            {
                DetectedType = ECore_TerrainType::Mountains;
            }
            // Check for desert terrain
            else if (HitResult.GetActor()->GetName().Contains(TEXT("Sand")) || 
                     HitResult.GetActor()->GetName().Contains(TEXT("Desert")))
            {
                DetectedType = ECore_TerrainType::Desert;
            }
            // Check for forest terrain
            else if (HitResult.GetActor()->GetName().Contains(TEXT("Tree")) || 
                     HitResult.GetActor()->GetName().Contains(TEXT("Forest")))
            {
                DetectedType = ECore_TerrainType::Forest;
            }
        }
    }
    
    // Cache the result
    if (TerrainTypeCache.Num() < MaxCacheSize)
    {
        const_cast<UCore_TerrainInteractionSystem*>(this)->TerrainTypeCache.Add(CacheKey, DetectedType);
    }
    
    return DetectedType;
}

ECore_SurfaceMaterial UCore_TerrainInteractionSystem::GetSurfaceMaterialAtLocation(const FVector& WorldLocation) const
{
    if (!bEnableDetailedSurfaceDetection)
    {
        return ECore_SurfaceMaterial::Dirt;
    }
    
    // Check cache first
    FVector CacheKey = FVector(
        FMath::RoundToFloat(WorldLocation.X / 50.0f) * 50.0f,
        FMath::RoundToFloat(WorldLocation.Y / 50.0f) * 50.0f,
        0.0f
    );
    
    if (const ECore_SurfaceMaterial* CachedMaterial = SurfaceMaterialCache.Find(CacheKey))
    {
        return *CachedMaterial;
    }
    
    FHitResult HitResult;
    if (TraceSurfaceAtLocation(WorldLocation + FVector(0, 0, 50), WorldLocation - FVector(0, 0, 100), HitResult))
    {
        ECore_SurfaceMaterial DetectedMaterial = ECore_SurfaceMaterial::Dirt;
        
        if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(HitResult.GetComponent()))
        {
            DetectedMaterial = AnalyzeStaticMeshSurface(MeshComp);
        }
        else
        {
            // Analyze by actor name/type
            FString ActorName = HitResult.GetActor()->GetName().ToLower();
            
            if (ActorName.Contains(TEXT("grass")) || ActorName.Contains(TEXT("vegetation")))
            {
                DetectedMaterial = ECore_SurfaceMaterial::Grass;
            }
            else if (ActorName.Contains(TEXT("rock")) || ActorName.Contains(TEXT("stone")))
            {
                DetectedMaterial = ECore_SurfaceMaterial::Rock;
            }
            else if (ActorName.Contains(TEXT("mud")) || ActorName.Contains(TEXT("swamp")))
            {
                DetectedMaterial = ECore_SurfaceMaterial::Mud;
            }
            else if (ActorName.Contains(TEXT("sand")))
            {
                DetectedMaterial = ECore_SurfaceMaterial::Sand;
            }
            else if (ActorName.Contains(TEXT("snow")) || ActorName.Contains(TEXT("ice")))
            {
                DetectedMaterial = ECore_SurfaceMaterial::Snow;
            }
            else if (ActorName.Contains(TEXT("water")))
            {
                DetectedMaterial = ECore_SurfaceMaterial::Water;
            }
        }
        
        // Cache the result
        if (SurfaceMaterialCache.Num() < MaxCacheSize)
        {
            const_cast<UCore_TerrainInteractionSystem*>(this)->SurfaceMaterialCache.Add(CacheKey, DetectedMaterial);
        }
        
        return DetectedMaterial;
    }
    
    return ECore_SurfaceMaterial::Dirt;
}

void UCore_TerrainInteractionSystem::GenerateFootstepInteraction(const FVector& FootLocation, float ImpactForce)
{
    ECore_SurfaceMaterial SurfaceType = GetSurfaceMaterialAtLocation(FootLocation);
    
    // Broadcast footstep event
    OnFootstepGenerated.Broadcast(FootLocation, SurfaceType);
    
    // Apply terrain deformation for heavy impacts
    if (bEnableTerrainDeformation && ImpactForce > 500.0f)
    {
        float DeformationRadius = FMath::Clamp(ImpactForce / 1000.0f, 5.0f, 25.0f);
        float DeformationDepth = FMath::Clamp(ImpactForce / 2000.0f, 1.0f, 10.0f);
        
        ApplyTerrainDeformation(FootLocation, DeformationRadius, DeformationDepth);
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Footstep generated at %s on surface %d with force %f"), 
           *FootLocation.ToString(), (int32)SurfaceType, ImpactForce);
}

bool UCore_TerrainInteractionSystem::CanDeformTerrainAtLocation(const FVector& WorldLocation) const
{
    if (!bEnableTerrainDeformation)
    {
        return false;
    }
    
    ECore_SurfaceMaterial SurfaceType = GetSurfaceMaterialAtLocation(WorldLocation);
    
    // Only soft surfaces can be deformed
    switch (SurfaceType)
    {
        case ECore_SurfaceMaterial::Dirt:
        case ECore_SurfaceMaterial::Mud:
        case ECore_SurfaceMaterial::Sand:
        case ECore_SurfaceMaterial::Snow:
            return true;
        default:
            return false;
    }
}

void UCore_TerrainInteractionSystem::ApplyTerrainDeformation(const FVector& ImpactLocation, float DeformationRadius, float DeformationDepth)
{
    if (!CanDeformTerrainAtLocation(ImpactLocation))
    {
        return;
    }
    
    // For now, just log the deformation - actual landscape modification would require more complex implementation
    UE_LOG(LogTemp, Log, TEXT("Terrain deformation applied at %s: Radius=%f, Depth=%f"), 
           *ImpactLocation.ToString(), DeformationRadius, DeformationDepth);
    
    // In a full implementation, this would:
    // 1. Find the landscape component at the location
    // 2. Modify the heightmap data
    // 3. Update the landscape rendering
    // 4. Potentially spawn debris or particle effects
}

bool UCore_TerrainInteractionSystem::TraceSurfaceAtLocation(const FVector& StartLocation, const FVector& EndLocation, FHitResult& OutHit) const
{
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());
    
    return GetWorld()->LineTraceSingleByChannel(OutHit, StartLocation, EndLocation, ECC_WorldStatic, QueryParams);
}

float UCore_TerrainInteractionSystem::GetMovementModifierForSurface(ECore_SurfaceMaterial SurfaceType) const
{
    if (const float* Modifier = SurfaceMovementModifiers.Find(SurfaceType))
    {
        return *Modifier;
    }
    
    return 1.0f; // Default modifier
}

bool UCore_TerrainInteractionSystem::IsSurfaceSlippery(ECore_SurfaceMaterial SurfaceType) const
{
    switch (SurfaceType)
    {
        case ECore_SurfaceMaterial::Mud:
        case ECore_SurfaceMaterial::Water:
        case ECore_SurfaceMaterial::Snow:
            return true;
        default:
            return false;
    }
}

void UCore_TerrainInteractionSystem::ApplyEnvironmentalEffects(AActor* TargetActor, ECore_TerrainType TerrainType, float DeltaTime)
{
    if (!TargetActor)
    {
        return;
    }
    
    // Apply movement modifiers based on terrain
    if (ACharacter* Character = Cast<ACharacter>(TargetActor))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            float StabilityFactor = CalculateTerrainStability(TargetActor);
            
            // Adjust movement based on terrain stability
            float BaseMaxWalkSpeed = 600.0f; // Default UE5 character speed
            float ModifiedSpeed = BaseMaxWalkSpeed * StabilityFactor;
            
            MovementComp->MaxWalkSpeed = FMath::Lerp(MovementComp->MaxWalkSpeed, ModifiedSpeed, DeltaTime * 2.0f);
        }
    }
}

float UCore_TerrainInteractionSystem::CalculateTerrainStability(AActor* Actor) const
{
    if (!Actor)
    {
        return 1.0f;
    }
    
    ECore_TerrainType TerrainType = GetTerrainTypeAtLocation(Actor->GetActorLocation());
    
    if (const float* StabilityFactor = TerrainStabilityFactors.Find(TerrainType))
    {
        return *StabilityFactor;
    }
    
    return 1.0f; // Default stability
}

void UCore_TerrainInteractionSystem::InitializeSurfaceProperties()
{
    // Initialize movement modifiers
    SurfaceMovementModifiers.Empty();
    SurfaceMovementModifiers.Add(ECore_SurfaceMaterial::Grass, 1.0f);
    SurfaceMovementModifiers.Add(ECore_SurfaceMaterial::Dirt, 0.9f);
    SurfaceMovementModifiers.Add(ECore_SurfaceMaterial::Rock, 0.8f);
    SurfaceMovementModifiers.Add(ECore_SurfaceMaterial::Mud, 0.6f);
    SurfaceMovementModifiers.Add(ECore_SurfaceMaterial::Sand, 0.7f);
    SurfaceMovementModifiers.Add(ECore_SurfaceMaterial::Snow, 0.5f);
    SurfaceMovementModifiers.Add(ECore_SurfaceMaterial::Water, 0.3f);
    
    // Initialize friction values
    SurfaceFrictionValues.Empty();
    SurfaceFrictionValues.Add(ECore_SurfaceMaterial::Grass, 0.8f);
    SurfaceFrictionValues.Add(ECore_SurfaceMaterial::Dirt, 0.7f);
    SurfaceFrictionValues.Add(ECore_SurfaceMaterial::Rock, 0.9f);
    SurfaceFrictionValues.Add(ECore_SurfaceMaterial::Mud, 0.3f);
    SurfaceFrictionValues.Add(ECore_SurfaceMaterial::Sand, 0.5f);
    SurfaceFrictionValues.Add(ECore_SurfaceMaterial::Snow, 0.2f);
    SurfaceFrictionValues.Add(ECore_SurfaceMaterial::Water, 0.1f);
    
    // Initialize terrain stability factors
    TerrainStabilityFactors.Empty();
    TerrainStabilityFactors.Add(ECore_TerrainType::Plains, 1.0f);
    TerrainStabilityFactors.Add(ECore_TerrainType::Forest, 0.9f);
    TerrainStabilityFactors.Add(ECore_TerrainType::Mountains, 0.7f);
    TerrainStabilityFactors.Add(ECore_TerrainType::Swamp, 0.5f);
    TerrainStabilityFactors.Add(ECore_TerrainType::Desert, 0.8f);
}

void UCore_TerrainInteractionSystem::CleanCache()
{
    // Clean terrain type cache if it gets too large
    if (TerrainTypeCache.Num() > MaxCacheSize)
    {
        TerrainTypeCache.Empty();
    }
    
    // Clean surface material cache if it gets too large
    if (SurfaceMaterialCache.Num() > MaxCacheSize)
    {
        SurfaceMaterialCache.Empty();
    }
}

ULandscapeComponent* UCore_TerrainInteractionSystem::GetLandscapeComponentAtLocation(const FVector& WorldLocation) const
{
    // Find landscape actors in the world
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscape::StaticClass(), LandscapeActors);
    
    for (AActor* Actor : LandscapeActors)
    {
        if (ALandscape* Landscape = Cast<ALandscape>(Actor))
        {
            // Get the landscape component at the specified location
            ULandscapeComponent* Component = Landscape->GetLandscapeComponentAtLocation(WorldLocation);
            if (Component)
            {
                return Component;
            }
        }
    }
    
    return nullptr;
}

ECore_SurfaceMaterial UCore_TerrainInteractionSystem::AnalyzeStaticMeshSurface(UStaticMeshComponent* MeshComponent) const
{
    if (!MeshComponent)
    {
        return ECore_SurfaceMaterial::Dirt;
    }
    
    // Analyze material names to determine surface type
    TArray<UMaterialInterface*> Materials = MeshComponent->GetMaterials();
    
    for (UMaterialInterface* Material : Materials)
    {
        if (Material)
        {
            FString MaterialName = Material->GetName().ToLower();
            
            if (MaterialName.Contains(TEXT("grass")) || MaterialName.Contains(TEXT("vegetation")))
            {
                return ECore_SurfaceMaterial::Grass;
            }
            else if (MaterialName.Contains(TEXT("rock")) || MaterialName.Contains(TEXT("stone")))
            {
                return ECore_SurfaceMaterial::Rock;
            }
            else if (MaterialName.Contains(TEXT("mud")))
            {
                return ECore_SurfaceMaterial::Mud;
            }
            else if (MaterialName.Contains(TEXT("sand")))
            {
                return ECore_SurfaceMaterial::Sand;
            }
            else if (MaterialName.Contains(TEXT("snow")) || MaterialName.Contains(TEXT("ice")))
            {
                return ECore_SurfaceMaterial::Snow;
            }
            else if (MaterialName.Contains(TEXT("water")))
            {
                return ECore_SurfaceMaterial::Water;
            }
        }
    }
    
    return ECore_SurfaceMaterial::Dirt;
}

ECore_TerrainType UCore_TerrainInteractionSystem::CalculateTerrainTypeFromLandscape(const FVector& WorldLocation) const
{
    // For now, return Plains as default
    // In a full implementation, this would analyze landscape layer weights
    // to determine the dominant terrain type at the location
    
    return ECore_TerrainType::Plains;
}