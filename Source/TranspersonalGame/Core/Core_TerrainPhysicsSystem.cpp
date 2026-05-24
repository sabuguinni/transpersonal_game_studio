#include "Core_TerrainPhysicsSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape/LandscapeComponent.h"
#include "Landscape/LandscapeInfo.h"
#include "BiomeManager.h"
#include "DrawDebugHelpers.h"

UCore_TerrainPhysicsSystem::UCore_TerrainPhysicsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for terrain physics
    
    // Initialize default terrain properties
    BiomeFrictionValues.Add(ECore_BiomeType::Savana, 0.7f);
    BiomeFrictionValues.Add(ECore_BiomeType::Forest, 0.6f);
    BiomeFrictionValues.Add(ECore_BiomeType::Swamp, 0.3f);
    BiomeFrictionValues.Add(ECore_BiomeType::Desert, 0.4f);
    BiomeFrictionValues.Add(ECore_BiomeType::Mountain, 0.8f);
    
    BiomeHardnessValues.Add(ECore_BiomeType::Savana, 0.6f);
    BiomeHardnessValues.Add(ECore_BiomeType::Forest, 0.5f);
    BiomeHardnessValues.Add(ECore_BiomeType::Swamp, 0.2f);
    BiomeHardnessValues.Add(ECore_BiomeType::Desert, 0.4f);
    BiomeHardnessValues.Add(ECore_BiomeType::Mountain, 0.9f);
    
    BiomeMoistureValues.Add(ECore_BiomeType::Savana, 0.3f);
    BiomeMoistureValues.Add(ECore_BiomeType::Forest, 0.7f);
    BiomeMoistureValues.Add(ECore_BiomeType::Swamp, 1.0f);
    BiomeMoistureValues.Add(ECore_BiomeType::Desert, 0.1f);
    BiomeMoistureValues.Add(ECore_BiomeType::Mountain, 0.4f);
    
    // Footprint settings
    bEnableFootprints = true;
    MaxFootprints = 500;
    FootprintFadeTime = 300.0f; // 5 minutes
    MinFootprintWeight = 50.0f; // kg
    
    // Erosion settings
    bEnableErosion = true;
    RainIntensity = 0.0f;
    WindStrength = 0.0f;
    ErosionUpdateFrequency = 60.0f; // 1 minute
    
    // Slope physics
    MaxStableSlope = 45.0f; // degrees
    LandslideThreshold = 60.0f; // degrees
    bEnableSlopePhysics = true;
    
    // Mud physics
    MudViscosity = 0.5f;
    QuicksandSinkRate = 0.1f; // m/s
    bEnableMudPhysics = true;
    
    // Performance settings
    PhysicsLODDistances = {1000.0f, 5000.0f, 10000.0f};
    DistantPhysicsUpdateRate = 1.0f; // 1 second
    MaxPhysicsCalculationsPerFrame = 10;
    
    // Internal state
    ErosionTimer = 0.0f;
    CurrentFrameCalculations = 0;
    CachedLandscape = nullptr;
    BiomeManager = nullptr;
}

void UCore_TerrainPhysicsSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeTerrainPhysics();
}

void UCore_TerrainPhysicsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Reset frame calculation counter
    CurrentFrameCalculations = 0;
    
    // Update footprint system
    if (bEnableFootprints)
    {
        UpdateFootprints(DeltaTime);
    }
    
    // Process erosion
    if (bEnableErosion)
    {
        ProcessErosion(DeltaTime);
    }
    
    // Check slope stability
    if (bEnableSlopePhysics)
    {
        CheckSlopeStability();
    }
    
    // Update mud physics
    if (bEnableMudPhysics)
    {
        UpdateMudPhysics(DeltaTime);
    }
}

void UCore_TerrainPhysicsSystem::InitializeTerrainPhysics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find landscape in the world
    for (TActorIterator<ALandscape> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        CachedLandscape = *ActorIterator;
        break;
    }
    
    // Find biome manager
    BiomeManager = World->GetGameInstance()->GetSubsystem<UBiomeManager>();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysicsSystem initialized - Landscape: %s, BiomeManager: %s"),
        CachedLandscape ? TEXT("Found") : TEXT("Not Found"),
        BiomeManager ? TEXT("Found") : TEXT("Not Found"));
}

void UCore_TerrainPhysicsSystem::UpdateFootprints(float DeltaTime)
{
    // Age and remove old footprints
    for (int32 i = ActiveFootprints.Num() - 1; i >= 0; i--)
    {
        ActiveFootprints[i].Age += DeltaTime;
        
        if (ActiveFootprints[i].Age >= FootprintFadeTime)
        {
            ActiveFootprints.RemoveAt(i);
        }
    }
    
    // Limit total footprints
    while (ActiveFootprints.Num() > MaxFootprints)
    {
        ActiveFootprints.RemoveAt(0);
    }
}

void UCore_TerrainPhysicsSystem::ProcessErosion(float DeltaTime)
{
    ErosionTimer += DeltaTime;
    
    if (ErosionTimer >= ErosionUpdateFrequency)
    {
        ErosionTimer = 0.0f;
        
        // Process erosion calculations
        if (CurrentFrameCalculations < MaxPhysicsCalculationsPerFrame)
        {
            // Sample random locations for erosion
            UWorld* World = GetWorld();
            if (World && CachedLandscape)
            {
                FVector LandscapeLocation = CachedLandscape->GetActorLocation();
                FVector RandomLocation = LandscapeLocation + FVector(
                    FMath::RandRange(-50000.0f, 50000.0f),
                    FMath::RandRange(-50000.0f, 50000.0f),
                    0.0f
                );
                
                float ErosionFactor = CalculateErosionFactor(RandomLocation, RainIntensity, WindStrength);
                
                if (ErosionFactor > 0.1f)
                {
                    ApplyTerrainDeformation(RandomLocation, ErosionFactor * 0.01f, 500.0f);
                }
                
                CurrentFrameCalculations++;
            }
        }
    }
}

void UCore_TerrainPhysicsSystem::CheckSlopeStability()
{
    if (CurrentFrameCalculations >= MaxPhysicsCalculationsPerFrame) return;
    
    // Sample random locations for slope stability
    UWorld* World = GetWorld();
    if (World && CachedLandscape)
    {
        FVector LandscapeLocation = CachedLandscape->GetActorLocation();
        FVector RandomLocation = LandscapeLocation + FVector(
            FMath::RandRange(-50000.0f, 50000.0f),
            FMath::RandRange(-50000.0f, 50000.0f),
            0.0f
        );
        
        FVector TerrainNormal = GetTerrainNormalAtLocation(RandomLocation);
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(TerrainNormal, FVector::UpVector)));
        
        if (SlopeAngle > LandslideThreshold)
        {
            // Trigger landslide effect
            UE_LOG(LogTemp, Warning, TEXT("Landslide triggered at location: %s, Slope: %.1f degrees"), 
                *RandomLocation.ToString(), SlopeAngle);
            
            // Apply terrain deformation to simulate landslide
            ApplyTerrainDeformation(RandomLocation, 0.05f, 1000.0f);
        }
        
        CurrentFrameCalculations++;
    }
}

void UCore_TerrainPhysicsSystem::UpdateMudPhysics(float DeltaTime)
{
    // Check for actors in mud areas
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find all pawns and check if they're in mud
    for (TActorIterator<APawn> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        APawn* Pawn = *ActorIterator;
        if (!Pawn) continue;
        
        FVector PawnLocation = Pawn->GetActorLocation();
        ECore_BiomeType BiomeType = GetBiomeTypeAtLocation(PawnLocation);
        
        if (BiomeType == ECore_BiomeType::Swamp)
        {
            float MudDepth = GetMudDepthAtLocation(PawnLocation);
            
            if (MudDepth > 10.0f) // 10cm of mud
            {
                // Apply mud physics to the pawn
                UPrimitiveComponent* PawnRoot = Cast<UPrimitiveComponent>(Pawn->GetRootComponent());
                if (PawnRoot)
                {
                    FVector MudForce = FVector(0, 0, -MudDepth * QuicksandSinkRate * 100.0f);
                    PawnRoot->AddForce(MudForce);
                    
                    // Reduce movement speed
                    if (UCharacterMovementComponent* MovementComp = Pawn->FindComponentByClass<UCharacterMovementComponent>())
                    {
                        float MudSlowdown = FMath::Clamp(1.0f - (MudDepth / 100.0f), 0.1f, 1.0f);
                        MovementComp->MaxWalkSpeed *= MudSlowdown;
                    }
                }
            }
        }
        
        CurrentFrameCalculations++;
        if (CurrentFrameCalculations >= MaxPhysicsCalculationsPerFrame) break;
    }
}

void UCore_TerrainPhysicsSystem::ApplyTerrainPhysicsAtLocation(const FVector& Location, float Radius, float Force)
{
    if (!CachedLandscape) return;
    
    // Apply physics force to terrain
    ApplyTerrainDeformation(Location, Force * 0.01f, Radius);
    
    UE_LOG(LogTemp, Log, TEXT("Applied terrain physics at %s with force %.2f and radius %.2f"), 
        *Location.ToString(), Force, Radius);
}

FCore_TerrainProperties UCore_TerrainPhysicsSystem::GetTerrainPropertiesAtLocation(const FVector& Location)
{
    FCore_TerrainProperties Properties;
    
    ECore_BiomeType BiomeType = GetBiomeTypeAtLocation(Location);
    
    Properties.BiomeType = BiomeType;
    Properties.Friction = BiomeFrictionValues.FindRef(BiomeType);
    Properties.Hardness = BiomeHardnessValues.FindRef(BiomeType);
    Properties.Moisture = BiomeMoistureValues.FindRef(BiomeType);
    Properties.Height = GetTerrainHeightAtLocation(Location);
    Properties.Normal = GetTerrainNormalAtLocation(Location);
    
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Properties.Normal, FVector::UpVector)));
    Properties.SlopeAngle = SlopeAngle;
    Properties.bIsStable = SlopeAngle <= MaxStableSlope;
    
    return Properties;
}

void UCore_TerrainPhysicsSystem::CreateFootprint(const FVector& Location, float Weight, float Size, ECore_CreatureType CreatureType)
{
    if (!bEnableFootprints || Weight < MinFootprintWeight) return;
    
    ECore_BiomeType BiomeType = GetBiomeTypeAtLocation(Location);
    float Hardness = BiomeHardnessValues.FindRef(BiomeType);
    
    // Calculate footprint depth based on weight and terrain hardness
    float FootprintDepth = (Weight / 1000.0f) * (1.0f - Hardness) * Size;
    
    if (FootprintDepth > 0.5f) // Minimum 0.5cm depth
    {
        FCore_FootprintData Footprint;
        Footprint.Location = Location;
        Footprint.Depth = FootprintDepth;
        Footprint.Size = Size;
        Footprint.CreatureType = CreatureType;
        Footprint.Age = 0.0f;
        Footprint.BiomeType = BiomeType;
        
        ActiveFootprints.Add(Footprint);
        
        UE_LOG(LogTemp, Log, TEXT("Created footprint at %s - Depth: %.2fcm, Size: %.2f"), 
            *Location.ToString(), FootprintDepth, Size);
    }
}

void UCore_TerrainPhysicsSystem::TriggerErosionInArea(const FVector& Center, float Radius, float Intensity)
{
    ApplyTerrainDeformation(Center, Intensity * 0.02f, Radius);
    
    UE_LOG(LogTemp, Log, TEXT("Triggered erosion at %s - Radius: %.2f, Intensity: %.2f"), 
        *Center.ToString(), Radius, Intensity);
}

bool UCore_TerrainPhysicsSystem::IsLocationStableForBuilding(const FVector& Location, float StructureWeight)
{
    FCore_TerrainProperties Properties = GetTerrainPropertiesAtLocation(Location);
    
    // Check slope stability
    if (Properties.SlopeAngle > MaxStableSlope * 0.8f) // 80% of max slope for safety
    {
        return false;
    }
    
    // Check terrain hardness for foundation
    if (Properties.Hardness < 0.3f) // Too soft for heavy structures
    {
        return false;
    }
    
    // Check if in swamp (unstable ground)
    if (Properties.BiomeType == ECore_BiomeType::Swamp && StructureWeight > 1000.0f)
    {
        return false;
    }
    
    return true;
}

float UCore_TerrainPhysicsSystem::GetMudDepthAtLocation(const FVector& Location)
{
    ECore_BiomeType BiomeType = GetBiomeTypeAtLocation(Location);
    
    if (BiomeType != ECore_BiomeType::Swamp)
    {
        return 0.0f;
    }
    
    float Moisture = BiomeMoistureValues.FindRef(BiomeType);
    float BaseDepth = 20.0f; // 20cm base mud depth in swamps
    
    // Add variation based on location
    float NoiseValue = FMath::PerlinNoise2D(FVector2D(Location.X / 1000.0f, Location.Y / 1000.0f));
    float VariationDepth = NoiseValue * 30.0f; // +/- 30cm variation
    
    return FMath::Max(0.0f, BaseDepth + VariationDepth);
}

ECore_BiomeType UCore_TerrainPhysicsSystem::GetBiomeTypeAtLocation(const FVector& Location)
{
    if (BiomeManager)
    {
        return BiomeManager->GetBiomeTypeAtLocation(Location);
    }
    
    // Fallback: simple distance-based biome detection
    if (Location.X < -25000.0f && Location.Y < -25000.0f)
        return ECore_BiomeType::Swamp;
    else if (Location.X < -25000.0f && Location.Y > 25000.0f)
        return ECore_BiomeType::Forest;
    else if (Location.X > 25000.0f)
        return ECore_BiomeType::Desert;
    else if (Location.Y > 25000.0f)
        return ECore_BiomeType::Mountain;
    else
        return ECore_BiomeType::Savana;
}

FVector UCore_TerrainPhysicsSystem::GetTerrainNormalAtLocation(const FVector& Location)
{
    if (!CachedLandscape) return FVector::UpVector;
    
    // Sample nearby points to calculate normal
    float SampleDistance = 100.0f;
    float HeightCenter = GetTerrainHeightAtLocation(Location);
    float HeightX = GetTerrainHeightAtLocation(Location + FVector(SampleDistance, 0, 0));
    float HeightY = GetTerrainHeightAtLocation(Location + FVector(0, SampleDistance, 0));
    
    FVector TangentX = FVector(SampleDistance, 0, HeightX - HeightCenter).GetSafeNormal();
    FVector TangentY = FVector(0, SampleDistance, HeightY - HeightCenter).GetSafeNormal();
    
    return FVector::CrossProduct(TangentY, TangentX).GetSafeNormal();
}

float UCore_TerrainPhysicsSystem::GetTerrainHeightAtLocation(const FVector& Location)
{
    if (!CachedLandscape) return 0.0f;
    
    UWorld* World = GetWorld();
    if (!World) return 0.0f;
    
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 10000.0f);
    FVector EndLocation = Location + FVector(0, 0, -10000.0f);
    
    if (World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic))
    {
        return HitResult.Location.Z;
    }
    
    return 0.0f;
}

bool UCore_TerrainPhysicsSystem::ShouldUpdatePhysicsAtDistance(float Distance)
{
    for (int32 i = 0; i < PhysicsLODDistances.Num(); i++)
    {
        if (Distance <= PhysicsLODDistances[i])
        {
            return true;
        }
    }
    
    return false;
}

void UCore_TerrainPhysicsSystem::CleanupOldFootprints()
{
    ActiveFootprints.RemoveAll([this](const FCore_FootprintData& Footprint)
    {
        return Footprint.Age >= FootprintFadeTime;
    });
}

float UCore_TerrainPhysicsSystem::CalculateErosionFactor(const FVector& Location, float RainAmount, float WindAmount)
{
    FVector TerrainNormal = GetTerrainNormalAtLocation(Location);
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(TerrainNormal, FVector::UpVector)));
    
    // Steeper slopes erode faster
    float SlopeFactor = SlopeAngle / 90.0f;
    
    // Rain and wind contribute to erosion
    float WeatherFactor = (RainAmount + WindAmount) * 0.5f;
    
    return SlopeFactor * WeatherFactor;
}

void UCore_TerrainPhysicsSystem::ApplyTerrainDeformation(const FVector& Location, float Intensity, float Radius)
{
    // This would require direct landscape editing which is complex in runtime
    // For now, we log the deformation for debugging
    UE_LOG(LogTemp, Log, TEXT("Terrain deformation applied at %s - Intensity: %.4f, Radius: %.2f"), 
        *Location.ToString(), Intensity, Radius);
    
    // In a full implementation, this would modify the landscape heightmap
    // using ULandscapeInfo::ModifyLandscape or similar functions
}