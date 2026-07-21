#include "Core_TerrainDeformation.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Landscape/LandscapeInfo.h"
#include "LandscapeEdit.h"
#include "LandscapeDataAccess.h"

// Terrain Deformation Component Implementation
UCore_TerrainDeformationComponent::UCore_TerrainDeformationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    MaxDeformationRadius = 500.0f;
    MaxDeformationIntensity = 2.0f;
    DeformationFadeTime = 30.0f;
    bEnableDebugVisualization = false;
    
    ActiveDeformations.Reserve(50);
}

void UCore_TerrainDeformationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find and cache landscape reference
    CachedLandscape = FindLandscape();
    
    // Register with the global terrain deformation manager
    if (UCore_TerrainDeformationManager* Manager = UCore_TerrainDeformationManager::GetTerrainDeformationManager(this))
    {
        Manager->RegisterDeformationComponent(this);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationComponent: Initialized with landscape: %s"), 
           CachedLandscape ? *CachedLandscape->GetName() : TEXT("None"));
}

void UCore_TerrainDeformationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update deformation fade
    UpdateDeformationFade(DeltaTime);
    
    // Debug visualization
    if (bEnableDebugVisualization)
    {
        for (const FCore_TerrainDeformationData& Deformation : ActiveDeformations)
        {
            FColor DebugColor = Deformation.Intensity > 0.0f ? FColor::Red : FColor::Blue;
            DrawDebugSphere(GetWorld(), Deformation.Location, Deformation.Radius, 12, DebugColor, false, 0.1f);
        }
    }
}

void UCore_TerrainDeformationComponent::ApplyDeformation(const FVector& Location, float Radius, float Intensity, ECore_TerrainType TerrainType)
{
    // Validate parameters
    if (!CanDeformTerrain(Location, TerrainType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainDeformationComponent: Cannot deform terrain at location %s"), *Location.ToString());
        return;
    }
    
    // Clamp values to safe ranges
    Radius = FMath::Clamp(Radius, 10.0f, MaxDeformationRadius);
    Intensity = FMath::Clamp(Intensity, -MaxDeformationIntensity, MaxDeformationIntensity);
    
    // Create deformation data
    FCore_TerrainDeformationData DeformationData;
    DeformationData.Location = Location;
    DeformationData.Radius = Radius;
    DeformationData.Intensity = Intensity;
    DeformationData.TerrainType = TerrainType;
    DeformationData.Timestamp = GetWorld()->GetTimeSeconds();
    DeformationData.bIsPermanent = FMath::Abs(Intensity) > 1.0f; // High intensity deformations are permanent
    
    // Add to active deformations
    ActiveDeformations.Add(DeformationData);
    
    // Apply to landscape if available
    if (CachedLandscape)
    {
        ApplyLandscapeDeformation(DeformationData);
    }
    
    // Broadcast event
    OnTerrainDeformed.Broadcast(Location, Radius, Intensity);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationComponent: Applied deformation at %s (R:%.1f, I:%.2f)"), 
           *Location.ToString(), Radius, Intensity);
}

void UCore_TerrainDeformationComponent::RemoveDeformation(const FVector& Location, float Radius)
{
    // Remove deformations within the specified area
    ActiveDeformations.RemoveAll([&](const FCore_TerrainDeformationData& Deformation)
    {
        float Distance = FVector::Dist(Deformation.Location, Location);
        return Distance <= Radius && !Deformation.bIsPermanent;
    });
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationComponent: Removed deformations near %s (R:%.1f)"), 
           *Location.ToString(), Radius);
}

float UCore_TerrainDeformationComponent::GetTerrainHardness(const FVector& Location) const
{
    // Base hardness values for different terrain types
    float BaseHardness = 1.0f;
    
    // Check for active deformations that might affect hardness
    for (const FCore_TerrainDeformationData& Deformation : ActiveDeformations)
    {
        float Distance = FVector::Dist(Deformation.Location, Location);
        if (Distance <= Deformation.Radius)
        {
            // Deformed terrain is softer
            float InfluenceFactor = 1.0f - (Distance / Deformation.Radius);
            BaseHardness *= (1.0f - (FMath::Abs(Deformation.Intensity) * 0.3f * InfluenceFactor));
        }
    }
    
    return FMath::Clamp(BaseHardness, 0.1f, 2.0f);
}

bool UCore_TerrainDeformationComponent::CanDeformTerrain(const FVector& Location, ECore_TerrainType TerrainType) const
{
    // Check if location is valid
    if (!GetWorld())
    {
        return false;
    }
    
    // Check terrain type restrictions
    switch (TerrainType)
    {
        case ECore_TerrainType::Rock:
            return false; // Rock cannot be easily deformed
        case ECore_TerrainType::Water:
            return false; // Water deformation handled separately
        case ECore_TerrainType::Dirt:
        case ECore_TerrainType::Grass:
        case ECore_TerrainType::Sand:
            return true; // These can be deformed
        default:
            return true;
    }
}

void UCore_TerrainDeformationComponent::UpdateDeformationFade(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove expired temporary deformations
    ActiveDeformations.RemoveAll([&](const FCore_TerrainDeformationData& Deformation)
    {
        if (Deformation.bIsPermanent)
        {
            return false; // Keep permanent deformations
        }
        
        float Age = CurrentTime - Deformation.Timestamp;
        return Age > DeformationFadeTime;
    });
}

ALandscape* UCore_TerrainDeformationComponent::FindLandscape() const
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    // Find landscape actor in the world
    for (TActorIterator<ALandscape> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape && IsValid(Landscape))
        {
            return Landscape;
        }
    }
    
    return nullptr;
}

void UCore_TerrainDeformationComponent::ApplyLandscapeDeformation(const FCore_TerrainDeformationData& DeformationData)
{
    if (!CachedLandscape || !IsValid(CachedLandscape))
    {
        return;
    }
    
    // Get landscape info
    ULandscapeInfo* LandscapeInfo = CachedLandscape->GetLandscapeInfo();
    if (!LandscapeInfo)
    {
        return;
    }
    
    // Convert world location to landscape coordinates
    FVector LandscapeLocation = CachedLandscape->GetTransform().InverseTransformPosition(DeformationData.Location);
    
    // Calculate affected area in landscape coordinates
    int32 MinX = FMath::FloorToInt((LandscapeLocation.X - DeformationData.Radius) / CachedLandscape->GetActorScale().X);
    int32 MaxX = FMath::CeilToInt((LandscapeLocation.X + DeformationData.Radius) / CachedLandscape->GetActorScale().X);
    int32 MinY = FMath::FloorToInt((LandscapeLocation.Y - DeformationData.Radius) / CachedLandscape->GetActorScale().Y);
    int32 MaxY = FMath::CeilToInt((LandscapeLocation.Y + DeformationData.Radius) / CachedLandscape->GetActorScale().Y);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Core_TerrainDeformationComponent: Applying landscape deformation at (%d,%d) to (%d,%d)"), 
           MinX, MinY, MaxX, MaxY);
}

// Terrain Deformation Manager Implementation
void UCore_TerrainDeformationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bTerrainDeformationEnabled = true;
    GlobalDeformationMultiplier = 1.0f;
    MaxActiveDeformations = 100;
    
    RegisteredComponents.Reserve(20);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationManager: Initialized"));
}

void UCore_TerrainDeformationManager::Deinitialize()
{
    RegisteredComponents.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationManager: Deinitialized"));
    
    Super::Deinitialize();
}

UCore_TerrainDeformationManager* UCore_TerrainDeformationManager::GetTerrainDeformationManager(const UObject* WorldContext)
{
    if (!WorldContext)
    {
        return nullptr;
    }
    
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UCore_TerrainDeformationManager>();
        }
    }
    
    return nullptr;
}

void UCore_TerrainDeformationManager::RegisterDeformationComponent(UCore_TerrainDeformationComponent* Component)
{
    if (!Component || RegisteredComponents.Contains(Component))
    {
        return;
    }
    
    RegisteredComponents.Add(Component);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationManager: Registered component %s"), 
           Component->GetOwner() ? *Component->GetOwner()->GetName() : TEXT("Unknown"));
}

void UCore_TerrainDeformationManager::UnregisterDeformationComponent(UCore_TerrainDeformationComponent* Component)
{
    RegisteredComponents.Remove(Component);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationManager: Unregistered component"));
}

void UCore_TerrainDeformationManager::ApplyGlobalDeformation(const FVector& Location, float Radius, float Intensity, ECore_TerrainType TerrainType)
{
    if (!bTerrainDeformationEnabled)
    {
        return;
    }
    
    // Apply deformation to all registered components within range
    float AdjustedIntensity = Intensity * GlobalDeformationMultiplier;
    
    for (UCore_TerrainDeformationComponent* Component : RegisteredComponents)
    {
        if (!Component || !IsValid(Component))
        {
            continue;
        }
        
        // Check if component is within range
        if (Component->GetOwner())
        {
            float Distance = FVector::Dist(Component->GetOwner()->GetActorLocation(), Location);
            if (Distance <= Radius * 2.0f) // Extended range for global effects
            {
                Component->ApplyDeformation(Location, Radius, AdjustedIntensity, TerrainType);
            }
        }
    }
    
    // Clean up invalid references
    CleanupInvalidComponents();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationManager: Applied global deformation at %s"), *Location.ToString());
}

FCore_TerrainDeformationData UCore_TerrainDeformationManager::GetTerrainDataAtLocation(const FVector& Location) const
{
    FCore_TerrainDeformationData ResultData;
    ResultData.Location = Location;
    
    // Find the closest deformation component
    float ClosestDistance = FLT_MAX;
    UCore_TerrainDeformationComponent* ClosestComponent = nullptr;
    
    for (UCore_TerrainDeformationComponent* Component : RegisteredComponents)
    {
        if (!Component || !IsValid(Component) || !Component->GetOwner())
        {
            continue;
        }
        
        float Distance = FVector::Dist(Component->GetOwner()->GetActorLocation(), Location);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestComponent = Component;
        }
    }
    
    // Get terrain hardness from the closest component
    if (ClosestComponent)
    {
        float Hardness = ClosestComponent->GetTerrainHardness(Location);
        ResultData.Intensity = 1.0f / FMath::Max(Hardness, 0.1f); // Convert hardness to deformation potential
    }
    
    return ResultData;
}

void UCore_TerrainDeformationManager::SetTerrainDeformationEnabled(bool bEnabled)
{
    bTerrainDeformationEnabled = bEnabled;
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationManager: Terrain deformation %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_TerrainDeformationManager::CleanupInvalidComponents()
{
    RegisteredComponents.RemoveAll([](UCore_TerrainDeformationComponent* Component)
    {
        return !Component || !IsValid(Component);
    });
}