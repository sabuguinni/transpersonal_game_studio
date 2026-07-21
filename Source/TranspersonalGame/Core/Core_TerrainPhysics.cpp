#include "Core_TerrainPhysics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Landscape/LandscapeInfo.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCore_TerrainPhysics::UCore_TerrainPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    bEnableTerrainDeformation = true;
    MaxDeformationDepth = 15.0f;
    DeformationRecoveryRate = 2.0f;
    
    bEnableGroundStability = true;
    LandslideAngleThreshold = 45.0f;
    CollapseWeightThreshold = 5000.0f;
    
    bEnableWaterErosion = true;
    ErosionRate = 1.0f;
    MinWaterFlowForErosion = 10.0f;
    
    LandscapeRef = nullptr;
    LastErosionUpdate = 0.0f;
}

void UCore_TerrainPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLandscapeReference();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysics: System initialized"));
    UE_LOG(LogTemp, Warning, TEXT("  - Deformation: %s"), bEnableTerrainDeformation ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("  - Ground Stability: %s"), bEnableGroundStability ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("  - Water Erosion: %s"), bEnableWaterErosion ? TEXT("Enabled") : TEXT("Disabled"));
}

void UCore_TerrainPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!LandscapeRef)
    {
        InitializeLandscapeReference();
        return;
    }
    
    // Update deformation recovery
    if (bEnableTerrainDeformation)
    {
        UpdateDeformationRecovery(DeltaTime);
    }
    
    // Update erosion system
    if (bEnableWaterErosion)
    {
        LastErosionUpdate += DeltaTime;
        if (LastErosionUpdate >= 1.0f) // Update erosion every second
        {
            // Process erosion for areas with water flow
            // This would integrate with a water system in a full implementation
            LastErosionUpdate = 0.0f;
        }
    }
}

void UCore_TerrainPhysics::ApplyTerrainDeformation(const FVector& WorldLocation, float Force, float Radius)
{
    if (!bEnableTerrainDeformation || !LandscapeRef)
    {
        return;
    }
    
    // Calculate deformation depth based on force and terrain hardness
    float TerrainHardness = GetTerrainHardness(WorldLocation);
    float DeformationDepth = FMath::Clamp(Force / TerrainHardness, 0.0f, MaxDeformationDepth);
    
    // Convert world location to landscape coordinates
    FVector LandscapeLocation = LandscapeRef->GetTransform().InverseTransformPosition(WorldLocation);
    FIntPoint LandscapeCoord = FIntPoint(
        FMath::RoundToInt(LandscapeLocation.X / 100.0f), // Convert to landscape units
        FMath::RoundToInt(LandscapeLocation.Y / 100.0f)
    );
    
    // Apply deformation in radius
    int32 RadiusInUnits = FMath::RoundToInt(Radius / 100.0f);
    for (int32 X = -RadiusInUnits; X <= RadiusInUnits; X++)
    {
        for (int32 Y = -RadiusInUnits; Y <= RadiusInUnits; Y++)
        {
            FIntPoint CurrentCoord = LandscapeCoord + FIntPoint(X, Y);
            float Distance = FVector2D(X, Y).Size();
            
            if (Distance <= RadiusInUnits)
            {
                // Calculate falloff
                float Falloff = 1.0f - (Distance / RadiusInUnits);
                float LocalDeformation = DeformationDepth * Falloff;
                
                // Store deformation data
                float& ExistingDeformation = DeformationMap.FindOrAdd(CurrentCoord);
                ExistingDeformation = FMath::Max(ExistingDeformation, LocalDeformation);
                
                // Apply to landscape (simplified - would need proper landscape editing API)
                ModifyLandscapeHeight(WorldLocation + FVector(X * 100, Y * 100, 0), -LocalDeformation, 50.0f);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied terrain deformation: Force=%.2f, Depth=%.2f, Radius=%.2f"), 
           Force, DeformationDepth, Radius);
}

bool UCore_TerrainPhysics::IsGroundStable(const FVector& WorldLocation, float CheckRadius)
{
    if (!bEnableGroundStability || !LandscapeRef)
    {
        return true; // Default to stable if system disabled
    }
    
    // Calculate average slope in the area
    float AverageSlope = CalculateSlope(WorldLocation);
    
    // Check if slope exceeds threshold
    if (AverageSlope > LandslideAngleThreshold)
    {
        return false;
    }
    
    // Check weight load (simplified - would integrate with physics system)
    // For now, assume stable unless extreme conditions
    return true;
}

void UCore_TerrainPhysics::TriggerLandslide(const FVector& WorldLocation, float AffectedRadius)
{
    if (!bEnableGroundStability || !LandscapeRef)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Landslide triggered at location: %s, Radius: %.2f"), 
           *WorldLocation.ToString(), AffectedRadius);
    
    // Apply deformation to simulate landslide
    ApplyTerrainDeformation(WorldLocation, 1000.0f, AffectedRadius);
    
    // Create debris (simplified implementation)
    // In full implementation, would spawn debris actors and apply physics
    for (int32 i = 0; i < 5; i++)
    {
        FVector DebrisLocation = WorldLocation + FVector(
            FMath::RandRange(-AffectedRadius, AffectedRadius),
            FMath::RandRange(-AffectedRadius, AffectedRadius),
            FMath::RandRange(0, 200)
        );
        
        // Spawn debris actor (placeholder)
        UE_LOG(LogTemp, Log, TEXT("Debris spawned at: %s"), *DebrisLocation.ToString());
    }
}

float UCore_TerrainPhysics::GetTerrainHardness(const FVector& WorldLocation)
{
    if (!LandscapeRef)
    {
        return 100.0f; // Default hardness
    }
    
    // Get landscape material at location (simplified)
    // In full implementation, would sample landscape material layers
    float BaseHardness = 100.0f;
    
    // Modify based on height (higher = rockier = harder)
    float Height = GetHeightAtLocation(WorldLocation);
    float HeightModifier = FMath::Clamp(Height / 1000.0f, 0.5f, 2.0f);
    
    return BaseHardness * HeightModifier;
}

void UCore_TerrainPhysics::ApplyWaterErosion(const FVector& WorldLocation, float WaterFlow, float DeltaTime)
{
    if (!bEnableWaterErosion || !LandscapeRef || WaterFlow < MinWaterFlowForErosion)
    {
        return;
    }
    
    // Calculate erosion amount
    float ErosionAmount = ErosionRate * WaterFlow * DeltaTime * 0.1f; // Scale factor
    
    // Apply erosion as negative deformation
    ApplyTerrainDeformation(WorldLocation, ErosionAmount * 100.0f, 200.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Water erosion applied: Flow=%.2f, Erosion=%.2f"), 
           WaterFlow, ErosionAmount);
}

void UCore_TerrainPhysics::ValidateTerrainPhysics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Terrain Physics Validation ==="));
    
    // Check landscape reference
    if (LandscapeRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ Landscape reference valid: %s"), *LandscapeRef->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ Landscape reference invalid"));
        InitializeLandscapeReference();
    }
    
    // Test deformation system
    if (bEnableTerrainDeformation)
    {
        FVector TestLocation = GetOwner()->GetActorLocation() + FVector(500, 0, 0);
        ApplyTerrainDeformation(TestLocation, 500.0f, 100.0f);
        UE_LOG(LogTemp, Warning, TEXT("✓ Deformation test applied at %s"), *TestLocation.ToString());
    }
    
    // Test stability system
    if (bEnableGroundStability)
    {
        FVector TestLocation = GetOwner()->GetActorLocation();
        bool bStable = IsGroundStable(TestLocation);
        UE_LOG(LogTemp, Warning, TEXT("✓ Ground stability test: %s"), bStable ? TEXT("Stable") : TEXT("Unstable"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Deformation map entries: %d"), DeformationMap.Num());
    UE_LOG(LogTemp, Warning, TEXT("=== Validation Complete ==="));
}

void UCore_TerrainPhysics::ResetTerrainDeformations()
{
    DeformationMap.Empty();
    StabilityCache.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Terrain deformations reset"));
}

void UCore_TerrainPhysics::InitializeLandscapeReference()
{
    if (GetWorld())
    {
        // Find landscape actor in the world
        TArray<AActor*> LandscapeActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscape::StaticClass(), LandscapeActors);
        
        if (LandscapeActors.Num() > 0)
        {
            LandscapeRef = Cast<ALandscape>(LandscapeActors[0]);
            UE_LOG(LogTemp, Warning, TEXT("Landscape reference initialized: %s"), *LandscapeRef->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No landscape found in world"));
        }
    }
}

void UCore_TerrainPhysics::UpdateDeformationRecovery(float DeltaTime)
{
    if (DeformationMap.Num() == 0)
    {
        return;
    }
    
    float RecoveryAmount = DeformationRecoveryRate * DeltaTime;
    TArray<FIntPoint> KeysToRemove;
    
    for (auto& DeformationPair : DeformationMap)
    {
        DeformationPair.Value -= RecoveryAmount;
        
        if (DeformationPair.Value <= 0.0f)
        {
            KeysToRemove.Add(DeformationPair.Key);
        }
    }
    
    // Remove fully recovered deformations
    for (const FIntPoint& Key : KeysToRemove)
    {
        DeformationMap.Remove(Key);
    }
}

float UCore_TerrainPhysics::CalculateSlope(const FVector& WorldLocation)
{
    if (!LandscapeRef)
    {
        return 0.0f;
    }
    
    // Sample heights at multiple points to calculate slope
    float SampleDistance = 100.0f;
    float CenterHeight = GetHeightAtLocation(WorldLocation);
    float NorthHeight = GetHeightAtLocation(WorldLocation + FVector(0, SampleDistance, 0));
    float EastHeight = GetHeightAtLocation(WorldLocation + FVector(SampleDistance, 0, 0));
    
    // Calculate slope vectors
    FVector SlopeX = FVector(SampleDistance, 0, EastHeight - CenterHeight);
    FVector SlopeY = FVector(0, SampleDistance, NorthHeight - CenterHeight);
    
    // Calculate normal and slope angle
    FVector Normal = FVector::CrossProduct(SlopeX, SlopeY).GetSafeNormal();
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));
    
    return SlopeAngle;
}

float UCore_TerrainPhysics::GetHeightAtLocation(const FVector& WorldLocation)
{
    if (!LandscapeRef)
    {
        return 0.0f;
    }
    
    // Use landscape's GetHeightAtLocation if available
    // Simplified implementation - would use proper landscape sampling
    FVector LandscapeLocation = LandscapeRef->GetTransform().InverseTransformPosition(WorldLocation);
    
    // Return approximate height (in full implementation would sample landscape heightmap)
    return WorldLocation.Z;
}

void UCore_TerrainPhysics::ModifyLandscapeHeight(const FVector& WorldLocation, float HeightDelta, float Radius)
{
    // Placeholder for landscape height modification
    // In full implementation would use LandscapeEditorUtils or runtime landscape modification
    UE_LOG(LogTemp, VeryVerbose, TEXT("Height modification: Location=%s, Delta=%.2f, Radius=%.2f"), 
           *WorldLocation.ToString(), HeightDelta, Radius);
}