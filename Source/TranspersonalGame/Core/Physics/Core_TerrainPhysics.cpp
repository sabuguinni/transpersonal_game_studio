#include "Core_TerrainPhysics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"
#include "CollisionQueryParams.h"

ACore_TerrainPhysics::ACore_TerrainPhysics()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default terrain properties
    InitializeTerrainProperties();
    
    // Set default values
    TerrainUpdateRate = 0.1f;
    MaxImpactForce = 10000.0f;
    bEnableTerrainDeformation = true;
    bEnableFootprints = true;
    bEnableTemperatureEffects = true;
    LastUpdateTime = 0.0f;
}

void ACore_TerrainPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Find and track all landscapes in the world
    UpdateTrackedLandscapes();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysics: System initialized with %d tracked landscapes"), TrackedLandscapes.Num());
}

void ACore_TerrainPhysics::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update terrain systems at specified rate
    if (GetWorld()->GetTimeSeconds() - LastUpdateTime >= TerrainUpdateRate)
    {
        ProcessTerrainImpacts(DeltaTime);
        
        if (bEnableTemperatureEffects)
        {
            UpdateTerrainTemperature(DeltaTime);
        }
        
        ProcessEnvironmentalEffects(DeltaTime);
        CleanupOldImpacts();
        
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void ACore_TerrainPhysics::InitializeTerrainProperties()
{
    // Grass terrain
    FCore_TerrainPhysicsProperties GrassProps;
    GrassProps.Friction = 0.8f;
    GrassProps.Restitution = 0.2f;
    GrassProps.Density = 0.5f;
    GrassProps.Hardness = 0.3f;
    GrassProps.Temperature = 20.0f;
    GrassProps.bCanDeform = true;
    GrassProps.bCausesFootprints = true;
    TerrainProperties.Add(ECore_TerrainType::Grass, GrassProps);
    
    // Rock terrain
    FCore_TerrainPhysicsProperties RockProps;
    RockProps.Friction = 0.9f;
    RockProps.Restitution = 0.1f;
    RockProps.Density = 2.5f;
    RockProps.Hardness = 1.0f;
    RockProps.Temperature = 15.0f;
    RockProps.bCanDeform = false;
    RockProps.bCausesFootprints = false;
    TerrainProperties.Add(ECore_TerrainType::Rock, RockProps);
    
    // Sand terrain
    FCore_TerrainPhysicsProperties SandProps;
    SandProps.Friction = 0.6f;
    SandProps.Restitution = 0.1f;
    SandProps.Density = 1.5f;
    SandProps.Hardness = 0.2f;
    SandProps.Temperature = 25.0f;
    SandProps.bCanDeform = true;
    SandProps.bCausesFootprints = true;
    TerrainProperties.Add(ECore_TerrainType::Sand, SandProps);
    
    // Mud terrain
    FCore_TerrainPhysicsProperties MudProps;
    MudProps.Friction = 0.4f;
    MudProps.Restitution = 0.05f;
    MudProps.Density = 1.8f;
    MudProps.Hardness = 0.1f;
    MudProps.Temperature = 18.0f;
    MudProps.bCanDeform = true;
    MudProps.bIsSlippery = true;
    MudProps.bCausesFootprints = true;
    TerrainProperties.Add(ECore_TerrainType::Mud, MudProps);
    
    // Snow terrain
    FCore_TerrainPhysicsProperties SnowProps;
    SnowProps.Friction = 0.3f;
    SnowProps.Restitution = 0.1f;
    SnowProps.Density = 0.3f;
    SnowProps.Hardness = 0.1f;
    SnowProps.Temperature = -5.0f;
    SnowProps.bCanDeform = true;
    SnowProps.bIsSlippery = true;
    SnowProps.bCausesFootprints = true;
    TerrainProperties.Add(ECore_TerrainType::Snow, SnowProps);
    
    // Water terrain
    FCore_TerrainPhysicsProperties WaterProps;
    WaterProps.Friction = 0.1f;
    WaterProps.Restitution = 0.0f;
    WaterProps.Density = 1.0f;
    WaterProps.Hardness = 0.0f;
    WaterProps.Temperature = 10.0f;
    WaterProps.bCanDeform = false;
    WaterProps.bIsSlippery = true;
    TerrainProperties.Add(ECore_TerrainType::Water, WaterProps);
    
    // Ice terrain
    FCore_TerrainPhysicsProperties IceProps;
    IceProps.Friction = 0.05f;
    IceProps.Restitution = 0.2f;
    IceProps.Density = 0.9f;
    IceProps.Hardness = 0.8f;
    IceProps.Temperature = -10.0f;
    IceProps.bCanDeform = false;
    IceProps.bIsSlippery = true;
    TerrainProperties.Add(ECore_TerrainType::Ice, IceProps);
    
    // Lava terrain
    FCore_TerrainPhysicsProperties LavaProps;
    LavaProps.Friction = 0.2f;
    LavaProps.Restitution = 0.0f;
    LavaProps.Density = 3.0f;
    LavaProps.Hardness = 0.0f;
    LavaProps.Temperature = 1000.0f;
    LavaProps.bCanDeform = false;
    LavaProps.bIsSlippery = false;
    TerrainProperties.Add(ECore_TerrainType::Lava, LavaProps);
}

ECore_TerrainType ACore_TerrainPhysics::GetTerrainTypeAtLocation(const FVector& Location)
{
    // Perform line trace to hit the terrain
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 1000);
    FVector TraceEnd = Location - FVector(0, 0, 1000);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        // Check if we hit a landscape
        if (ALandscape* Landscape = Cast<ALandscape>(HitResult.GetActor()))
        {
            return SampleTerrainTypeFromLandscape(HitResult.Location);
        }
        
        // Check physical material
        if (HitResult.PhysMaterial.IsValid())
        {
            FString MaterialName = HitResult.PhysMaterial->GetName();
            if (MaterialName.Contains(TEXT("Rock"))) return ECore_TerrainType::Rock;
            if (MaterialName.Contains(TEXT("Sand"))) return ECore_TerrainType::Sand;
            if (MaterialName.Contains(TEXT("Mud"))) return ECore_TerrainType::Mud;
            if (MaterialName.Contains(TEXT("Snow"))) return ECore_TerrainType::Snow;
            if (MaterialName.Contains(TEXT("Water"))) return ECore_TerrainType::Water;
            if (MaterialName.Contains(TEXT("Ice"))) return ECore_TerrainType::Ice;
            if (MaterialName.Contains(TEXT("Lava"))) return ECore_TerrainType::Lava;
        }
    }
    
    // Default to grass
    return ECore_TerrainType::Grass;
}

FCore_TerrainPhysicsProperties ACore_TerrainPhysics::GetTerrainPropertiesAtLocation(const FVector& Location)
{
    ECore_TerrainType TerrainType = GetTerrainTypeAtLocation(Location);
    
    if (TerrainProperties.Contains(TerrainType))
    {
        FCore_TerrainPhysicsProperties Props = TerrainProperties[TerrainType];
        
        // Apply temperature modifications if enabled
        if (bEnableTemperatureEffects)
        {
            float CurrentTemp = GetTemperatureAtLocation(Location);
            Props.Hardness = CalculateTerrainHardness(TerrainType, CurrentTemp);
        }
        
        return Props;
    }
    
    return FCore_TerrainPhysicsProperties();
}

float ACore_TerrainPhysics::GetTerrainSlopeAtLocation(const FVector& Location)
{
    FVector Normal = GetTerrainNormalAtLocation(Location);
    float Slope = FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector));
    return FMath::RadiansToDegrees(Slope);
}

FVector ACore_TerrainPhysics::GetTerrainNormalAtLocation(const FVector& Location)
{
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 1000);
    FVector TraceEnd = Location - FVector(0, 0, 1000);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Normal;
    }
    
    return FVector::UpVector;
}

void ACore_TerrainPhysics::ApplyTerrainImpact(const FCore_TerrainImpactData& ImpactData)
{
    if (ImpactData.ImpactForce > MaxImpactForce)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysics: Impact force %f exceeds maximum %f"), ImpactData.ImpactForce, MaxImpactForce);
        return;
    }
    
    // Store impact for processing
    FCore_TerrainImpactData ProcessedImpact = ImpactData;
    ProcessedImpact.Timestamp = GetWorld()->GetTimeSeconds();
    RecentImpacts.Add(ProcessedImpact);
    
    // Get terrain properties at impact location
    FCore_TerrainPhysicsProperties TerrainProps = GetTerrainPropertiesAtLocation(ImpactData.ImpactLocation);
    
    // Create footprint if enabled and terrain supports it
    if (bEnableFootprints && TerrainProps.bCausesFootprints)
    {
        float FootprintSize = FMath::Clamp(ImpactData.ImpactForce / 1000.0f, 0.1f, 2.0f);
        float FootprintDepth = FMath::Clamp(ImpactData.ImpactForce / 5000.0f, 0.01f, 0.5f);
        CreateFootprint(ImpactData.ImpactLocation, FootprintSize, FootprintDepth);
    }
    
    // Deform terrain if enabled and terrain can deform
    if (bEnableTerrainDeformation && TerrainProps.bCanDeform)
    {
        float DeformRadius = FMath::Clamp(ImpactData.ImpactForce / 2000.0f, 0.5f, 5.0f);
        float DeformStrength = FMath::Clamp(ImpactData.ImpactForce / 10000.0f, 0.1f, 1.0f);
        DeformTerrain(ImpactData.ImpactLocation, DeformRadius, DeformStrength);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Applied impact at %s with force %f"), *ImpactData.ImpactLocation.ToString(), ImpactData.ImpactForce);
}

void ACore_TerrainPhysics::CreateFootprint(const FVector& Location, float Size, float Depth)
{
    // This would integrate with a footprint system or decal system
    // For now, we'll just log the footprint creation
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Created footprint at %s, Size: %f, Depth: %f"), *Location.ToString(), Size, Depth);
    
    // In a full implementation, this would:
    // 1. Create a decal or modify landscape heightmap
    // 2. Apply visual footprint texture
    // 3. Store footprint data for persistence
}

void ACore_TerrainPhysics::DeformTerrain(const FVector& Location, float Radius, float Strength)
{
    // This would integrate with landscape modification system
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Deformed terrain at %s, Radius: %f, Strength: %f"), *Location.ToString(), Radius, Strength);
    
    // In a full implementation, this would:
    // 1. Modify landscape heightmap data
    // 2. Update collision mesh
    // 3. Trigger landscape component updates
}

bool ACore_TerrainPhysics::IsLocationWalkable(const FVector& Location)
{
    float Slope = GetTerrainSlopeAtLocation(Location);
    FCore_TerrainPhysicsProperties Props = GetTerrainPropertiesAtLocation(Location);
    
    // Check slope threshold (45 degrees is typically unwalkable)
    if (Slope > 45.0f)
    {
        return false;
    }
    
    // Check terrain type specific walkability
    ECore_TerrainType TerrainType = GetTerrainTypeAtLocation(Location);
    if (TerrainType == ECore_TerrainType::Lava || TerrainType == ECore_TerrainType::Water)
    {
        return false;
    }
    
    return true;
}

void ACore_TerrainPhysics::UpdatePhysicsMaterialForTerrain(ECore_TerrainType TerrainType)
{
    if (!TerrainProperties.Contains(TerrainType))
    {
        return;
    }
    
    FCore_TerrainPhysicsProperties Props = TerrainProperties[TerrainType];
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Updated physics material for terrain type %d"), (int32)TerrainType);
}

void ACore_TerrainPhysics::ApplyTerrainEffectsToActor(AActor* Actor, const FVector& Location)
{
    if (!Actor)
    {
        return;
    }
    
    FCore_TerrainPhysicsProperties Props = GetTerrainPropertiesAtLocation(Location);
    
    // Apply slippery effects
    if (Props.bIsSlippery)
    {
        if (UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>())
        {
            // Reduce friction temporarily
            // This would be implemented with custom physics materials
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Applied terrain effects to actor %s at location %s"), *Actor->GetName(), *Location.ToString());
}

float ACore_TerrainPhysics::GetTemperatureAtLocation(const FVector& Location)
{
    // Check if we have cached temperature data
    FVector GridLocation = FVector(
        FMath::RoundToFloat(Location.X / 100.0f) * 100.0f,
        FMath::RoundToFloat(Location.Y / 100.0f) * 100.0f,
        0.0f
    );
    
    if (TemperatureMap.Contains(GridLocation))
    {
        return TemperatureMap[GridLocation];
    }
    
    // Calculate base temperature based on terrain type
    ECore_TerrainType TerrainType = GetTerrainTypeAtLocation(Location);
    if (TerrainProperties.Contains(TerrainType))
    {
        float BaseTemp = TerrainProperties[TerrainType].Temperature;
        
        // Add some variation based on location
        float HeightModifier = Location.Z * -0.01f; // Colder at higher altitudes
        float RandomVariation = FMath::RandRange(-5.0f, 5.0f);
        
        float FinalTemp = BaseTemp + HeightModifier + RandomVariation;
        TemperatureMap.Add(GridLocation, FinalTemp);
        
        return FinalTemp;
    }
    
    return 20.0f; // Default temperature
}

void ACore_TerrainPhysics::UpdateTerrainTemperature(float DeltaTime)
{
    // Update temperature map based on environmental factors
    // This is a simplified implementation
    for (auto& TempPair : TemperatureMap)
    {
        float& Temperature = TempPair.Value;
        
        // Apply time-based temperature changes (day/night cycle, seasons)
        float TimeOfDay = FMath::Fmod(GetWorld()->GetTimeSeconds() / 3600.0f, 24.0f); // Hours
        float DayNightModifier = FMath::Sin((TimeOfDay / 24.0f) * 2.0f * PI) * 10.0f;
        
        Temperature += DayNightModifier * DeltaTime * 0.1f;
    }
}

void ACore_TerrainPhysics::ProcessEnvironmentalEffects(float DeltaTime)
{
    // Process environmental effects like erosion, freezing, melting
    // This is where complex terrain evolution would happen
    
    for (const FCore_TerrainImpactData& Impact : RecentImpacts)
    {
        // Process impact-based environmental changes
        float TimeSinceImpact = GetWorld()->GetTimeSeconds() - Impact.Timestamp;
        
        if (TimeSinceImpact < 60.0f) // Effects last for 1 minute
        {
            // Apply ongoing effects from impacts
            FCore_TerrainPhysicsProperties Props = GetTerrainPropertiesAtLocation(Impact.ImpactLocation);
            
            if (Props.bCanDeform && Impact.ImpactForce > 1000.0f)
            {
                // Continue deformation effects
            }
        }
    }
}

void ACore_TerrainPhysics::ProcessTerrainImpacts(float DeltaTime)
{
    // Process all recent impacts and their ongoing effects
    for (int32 i = RecentImpacts.Num() - 1; i >= 0; i--)
    {
        const FCore_TerrainImpactData& Impact = RecentImpacts[i];
        float TimeSinceImpact = GetWorld()->GetTimeSeconds() - Impact.Timestamp;
        
        // Apply ongoing effects
        if (TimeSinceImpact < 30.0f) // Effects last for 30 seconds
        {
            // Continue processing impact effects
            ProcessEnvironmentalEffects(DeltaTime);
        }
    }
}

void ACore_TerrainPhysics::CleanupOldImpacts()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove impacts older than 5 minutes
    RecentImpacts.RemoveAll([CurrentTime](const FCore_TerrainImpactData& Impact)
    {
        return (CurrentTime - Impact.Timestamp) > 300.0f;
    });
}

void ACore_TerrainPhysics::UpdateTrackedLandscapes()
{
    TrackedLandscapes.Empty();
    
    // Find all landscape actors in the world
    for (TActorIterator<ALandscape> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape)
        {
            TrackedLandscapes.Add(Landscape);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Found %d landscapes to track"), TrackedLandscapes.Num());
}

ECore_TerrainType ACore_TerrainPhysics::SampleTerrainTypeFromLandscape(const FVector& Location)
{
    // This would sample landscape layer data to determine terrain type
    // For now, return a simple height-based determination
    
    if (Location.Z < -100.0f)
    {
        return ECore_TerrainType::Water;
    }
    else if (Location.Z > 1000.0f)
    {
        return ECore_TerrainType::Snow;
    }
    else if (Location.Z > 500.0f)
    {
        return ECore_TerrainType::Rock;
    }
    else
    {
        return ECore_TerrainType::Grass;
    }
}

float ACore_TerrainPhysics::CalculateTerrainHardness(ECore_TerrainType TerrainType, float Temperature)
{
    if (!TerrainProperties.Contains(TerrainType))
    {
        return 1.0f;
    }
    
    float BaseHardness = TerrainProperties[TerrainType].Hardness;
    
    // Temperature affects hardness
    if (Temperature < 0.0f) // Freezing makes things harder
    {
        BaseHardness *= 1.5f;
    }
    else if (Temperature > 50.0f) // Heat can soften some materials
    {
        BaseHardness *= 0.8f;
    }
    
    return FMath::Clamp(BaseHardness, 0.1f, 2.0f);
}

void ACore_TerrainPhysics::ValidateTerrainPhysics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Core_TerrainPhysics Validation ==="));
    UE_LOG(LogTemp, Warning, TEXT("Terrain Properties Count: %d"), TerrainProperties.Num());
    UE_LOG(LogTemp, Warning, TEXT("Tracked Landscapes: %d"), TrackedLandscapes.Num());
    UE_LOG(LogTemp, Warning, TEXT("Recent Impacts: %d"), RecentImpacts.Num());
    UE_LOG(LogTemp, Warning, TEXT("Temperature Map Size: %d"), TemperatureMap.Num());
    UE_LOG(LogTemp, Warning, TEXT("Terrain Deformation: %s"), bEnableTerrainDeformation ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("Footprints: %s"), bEnableFootprints ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("Temperature Effects: %s"), bEnableTemperatureEffects ? TEXT("Enabled") : TEXT("Disabled"));
    
    // Test terrain sampling at actor location
    FVector TestLocation = GetActorLocation();
    ECore_TerrainType TerrainType = GetTerrainTypeAtLocation(TestLocation);
    FCore_TerrainPhysicsProperties Props = GetTerrainPropertiesAtLocation(TestLocation);
    float Slope = GetTerrainSlopeAtLocation(TestLocation);
    
    UE_LOG(LogTemp, Warning, TEXT("Test Location: %s"), *TestLocation.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Terrain Type: %d"), (int32)TerrainType);
    UE_LOG(LogTemp, Warning, TEXT("Friction: %f, Hardness: %f"), Props.Friction, Props.Hardness);
    UE_LOG(LogTemp, Warning, TEXT("Slope: %f degrees"), Slope);
    UE_LOG(LogTemp, Warning, TEXT("Walkable: %s"), IsLocationWalkable(TestLocation) ? TEXT("Yes") : TEXT("No"));
}

void ACore_TerrainPhysics::TestTerrainInteraction()
{
    FVector TestLocation = GetActorLocation();
    
    // Create a test impact
    FCore_TerrainImpactData TestImpact;
    TestImpact.ImpactLocation = TestLocation;
    TestImpact.ImpactNormal = FVector::UpVector;
    TestImpact.ImpactForce = 5000.0f;
    TestImpact.TerrainType = GetTerrainTypeAtLocation(TestLocation);
    TestImpact.ImpactingActor = this;
    
    ApplyTerrainImpact(TestImpact);
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysics: Test impact applied at %s"), *TestLocation.ToString());
}

void ACore_TerrainPhysics::DebugDrawTerrainInfo()
{
    if (!GetWorld())
    {
        return;
    }
    
    FVector ActorLocation = GetActorLocation();
    
    // Draw terrain type info
    ECore_TerrainType TerrainType = GetTerrainTypeAtLocation(ActorLocation);
    FString TerrainTypeString = UEnum::GetValueAsString(TerrainType);
    
    // Draw debug text
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 200), 
                   FString::Printf(TEXT("Terrain: %s"), *TerrainTypeString), 
                   nullptr, FColor::White, 0.0f);
    
    // Draw slope indicator
    float Slope = GetTerrainSlopeAtLocation(ActorLocation);
    FColor SlopeColor = Slope > 30.0f ? FColor::Red : (Slope > 15.0f ? FColor::Yellow : FColor::Green);
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 150), 
                   FString::Printf(TEXT("Slope: %.1f°"), Slope), 
                   nullptr, SlopeColor, 0.0f);
    
    // Draw temperature info
    float Temperature = GetTemperatureAtLocation(ActorLocation);
    FColor TempColor = Temperature > 30.0f ? FColor::Red : (Temperature < 0.0f ? FColor::Cyan : FColor::Green);
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 100), 
                   FString::Printf(TEXT("Temp: %.1f°C"), Temperature), 
                   nullptr, TempColor, 0.0f);
    
    // Draw terrain normal
    FVector TerrainNormal = GetTerrainNormalAtLocation(ActorLocation);
    DrawDebugLine(GetWorld(), ActorLocation, ActorLocation + (TerrainNormal * 100.0f), FColor::Blue, false, 0.0f, 0, 2.0f);
}