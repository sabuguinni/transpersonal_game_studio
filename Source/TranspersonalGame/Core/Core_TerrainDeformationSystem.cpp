#include "Core_TerrainDeformationSystem.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeComponent.h"
#include "Landscape/LandscapeInfo.h"
#include "Landscape/LandscapeDataAccess.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

UCore_TerrainDeformationSystem::UCore_TerrainDeformationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize default values
    MaxActiveDeformations = 100;
    MaxDeformationRadius = 1000.0f;
    MaxDeformationDepth = 200.0f;
    CurrentLODLevel = 1;
    bDeformationEnabled = true;
    bShowDebugVisualization = false;
    
    MainLandscape = nullptr;
    LandscapeInfo = nullptr;
    
    CurrentFrameDeformations = 0;
    TotalDeformationsProcessed = 0;
    LastPerformanceCheckTime = 0.0f;
}

void UCore_TerrainDeformationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize landscape references
    InitializeLandscapeReferences();
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainDeformationSystem: Initialized with %d max deformations"), MaxActiveDeformations);
}

void UCore_TerrainDeformationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bDeformationEnabled)
    {
        return;
    }
    
    // Update recovery operations
    UpdateRecoveryOperations(DeltaTime);
    
    // Cleanup expired deformations
    CleanupExpiredDeformations();
    
    // Performance monitoring
    LastPerformanceCheckTime += DeltaTime;
    if (LastPerformanceCheckTime >= 5.0f) // Check every 5 seconds
    {
        UE_LOG(LogTemp, Log, TEXT("TerrainDeformation: %d active deformations, %d total processed"), 
               ActiveDeformations.Num(), TotalDeformationsProcessed);
        LastPerformanceCheckTime = 0.0f;
    }
    
    // Debug visualization
    if (bShowDebugVisualization)
    {
        for (const FCore_TerrainDeformation& Deformation : ActiveDeformations)
        {
            DrawDebugSphere(GetWorld(), Deformation.Location, Deformation.Radius, 12, FColor::Red, false, -1.0f, 0, 2.0f);
        }
        
        for (const FCore_TerrainRecovery& Recovery : ActiveRecoveries)
        {
            DrawDebugSphere(GetWorld(), Recovery.Location, Recovery.Radius, 12, FColor::Green, false, -1.0f, 0, 1.0f);
        }
    }
    
    CurrentFrameDeformations = 0;
}

void UCore_TerrainDeformationSystem::CreateTerrainDeformation(
    const FVector& WorldLocation,
    float DeformationRadius,
    float DeformationDepth,
    ECore_TerrainDeformationType DeformationType,
    bool bPermanent)
{
    if (!bDeformationEnabled || !MainLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainDeformation: Cannot create deformation - system disabled or no landscape"));
        return;
    }
    
    // Clamp values to limits
    DeformationRadius = FMath::Clamp(DeformationRadius, 10.0f, MaxDeformationRadius);
    DeformationDepth = FMath::Clamp(DeformationDepth, -MaxDeformationDepth, MaxDeformationDepth);
    
    // Check if we've reached maximum deformations
    if (ActiveDeformations.Num() >= MaxActiveDeformations)
    {
        // Remove oldest non-permanent deformation
        for (int32 i = 0; i < ActiveDeformations.Num(); ++i)
        {
            if (!ActiveDeformations[i].bPermanent)
            {
                ActiveDeformations.RemoveAt(i);
                break;
            }
        }
    }
    
    // Create new deformation
    FCore_TerrainDeformation NewDeformation;
    NewDeformation.Location = WorldLocation;
    NewDeformation.Radius = DeformationRadius;
    NewDeformation.Depth = DeformationDepth;
    NewDeformation.Type = DeformationType;
    NewDeformation.bPermanent = bPermanent;
    NewDeformation.CreationTime = GetWorld()->GetTimeSeconds();
    NewDeformation.DecayTime = bPermanent ? 0.0f : 300.0f; // 5 minutes default decay
    
    // Apply LOD scaling
    float LODScale = 1.0f;
    switch (CurrentLODLevel)
    {
        case 0: LODScale = 1.0f; break;   // Full detail
        case 1: LODScale = 0.75f; break; // High detail
        case 2: LODScale = 0.5f; break;  // Medium detail
        case 3: LODScale = 0.25f; break; // Low detail
    }
    
    NewDeformation.Radius *= LODScale;
    NewDeformation.Depth *= LODScale;
    
    // Process the deformation
    ProcessLandscapeDeformation(NewDeformation);
    
    // Add to active deformations
    ActiveDeformations.Add(NewDeformation);
    
    TotalDeformationsProcessed++;
    CurrentFrameDeformations++;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformation: Created deformation at %s, radius %.1f, depth %.1f"), 
           *WorldLocation.ToString(), DeformationRadius, DeformationDepth);
}

void UCore_TerrainDeformationSystem::CreateDinosaurFootprint(
    const FVector& FootprintLocation,
    ECore_DinosaurSpecies DinosaurSpecies,
    float FootSize,
    bool bDeepMud)
{
    float BaseRadius = 50.0f;
    float BaseDepth = 10.0f;
    
    // Species-specific footprint characteristics
    switch (DinosaurSpecies)
    {
        case ECore_DinosaurSpecies::TRex:
            BaseRadius = 80.0f;
            BaseDepth = 25.0f;
            break;
        case ECore_DinosaurSpecies::Velociraptor:
            BaseRadius = 30.0f;
            BaseDepth = 8.0f;
            break;
        case ECore_DinosaurSpecies::Brachiosaurus:
            BaseRadius = 120.0f;
            BaseDepth = 40.0f;
            break;
        case ECore_DinosaurSpecies::Triceratops:
            BaseRadius = 70.0f;
            BaseDepth = 20.0f;
            break;
        case ECore_DinosaurSpecies::Stegosaurus:
            BaseRadius = 60.0f;
            BaseDepth = 15.0f;
            break;
    }
    
    // Apply size scaling
    BaseRadius *= FootSize;
    BaseDepth *= FootSize;
    
    // Deep mud increases depth
    if (bDeepMud)
    {
        BaseDepth *= 2.0f;
    }
    
    // Create the footprint deformation
    CreateTerrainDeformation(
        FootprintLocation,
        BaseRadius,
        -BaseDepth, // Negative for depression
        ECore_TerrainDeformationType::Footprint,
        false // Footprints are not permanent
    );
    
    // Modify terrain material to show disturbed earth
    ModifyTerrainMaterial(
        FootprintLocation,
        BaseRadius * 1.2f,
        bDeepMud ? ECore_TerrainMaterial::Mud : ECore_TerrainMaterial::DisturbedEarth,
        0.7f
    );
}

void UCore_TerrainDeformationSystem::CreateImpactCrater(
    const FVector& ImpactLocation,
    float ImpactForce,
    ECore_ImpactType ImpactType)
{
    float CraterRadius = 100.0f;
    float CraterDepth = 30.0f;
    
    // Impact type affects crater characteristics
    switch (ImpactType)
    {
        case ECore_ImpactType::Meteorite:
            CraterRadius = FMath::Sqrt(ImpactForce) * 2.0f;
            CraterDepth = FMath::Sqrt(ImpactForce) * 0.8f;
            break;
        case ECore_ImpactType::TreeFall:
            CraterRadius = ImpactForce * 0.5f;
            CraterDepth = ImpactForce * 0.2f;
            break;
        case ECore_ImpactType::RockSlide:
            CraterRadius = ImpactForce * 0.8f;
            CraterDepth = ImpactForce * 0.3f;
            break;
        case ECore_ImpactType::Explosion:
            CraterRadius = ImpactForce * 1.5f;
            CraterDepth = ImpactForce * 0.6f;
            break;
    }
    
    // Create the crater
    CreateTerrainDeformation(
        ImpactLocation,
        CraterRadius,
        -CraterDepth,
        ECore_TerrainDeformationType::Impact,
        ImpactType == ECore_ImpactType::Meteorite // Meteorite craters are permanent
    );
    
    // Modify terrain material based on impact type
    ECore_TerrainMaterial ImpactMaterial = ECore_TerrainMaterial::DisturbedEarth;
    if (ImpactType == ECore_ImpactType::Meteorite)
    {
        ImpactMaterial = ECore_TerrainMaterial::ScorchedEarth;
    }
    
    ModifyTerrainMaterial(ImpactLocation, CraterRadius, ImpactMaterial, 0.9f);
}

void UCore_TerrainDeformationSystem::ModifyTerrainMaterial(
    const FVector& Location,
    float Radius,
    ECore_TerrainMaterial NewMaterial,
    float BlendStrength)
{
    if (!MainLandscape || !LandscapeInfo)
    {
        return;
    }
    
    // Material modification would be implemented here
    // This requires access to landscape layer data which is complex
    // For now, we log the operation
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformation: Material modification at %s, radius %.1f, material %d"), 
           *Location.ToString(), Radius, (int32)NewMaterial);
}

void UCore_TerrainDeformationSystem::SetDeformationLOD(int32 LODLevel)
{
    CurrentLODLevel = FMath::Clamp(LODLevel, 0, 3);
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformation: LOD level set to %d"), CurrentLODLevel);
}

void UCore_TerrainDeformationSystem::SetDeformationEnabled(bool bEnabled)
{
    bDeformationEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformation: System %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCore_TerrainDeformationSystem::StartTerrainRecovery(
    const FVector& RecoveryLocation,
    float RecoveryRadius,
    float RecoveryTime)
{
    FCore_TerrainRecovery NewRecovery;
    NewRecovery.Location = RecoveryLocation;
    NewRecovery.Radius = RecoveryRadius;
    NewRecovery.TotalRecoveryTime = RecoveryTime;
    NewRecovery.ElapsedTime = 0.0f;
    NewRecovery.StartTime = GetWorld()->GetTimeSeconds();
    
    ActiveRecoveries.Add(NewRecovery);
    
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformation: Started recovery at %s, radius %.1f, time %.1fs"), 
           *RecoveryLocation.ToString(), RecoveryRadius, RecoveryTime);
}

void UCore_TerrainDeformationSystem::SetDebugVisualization(bool bShowDebug)
{
    bShowDebugVisualization = bShowDebug;
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformation: Debug visualization %s"), bShowDebug ? TEXT("enabled") : TEXT("disabled"));
}

FCore_TerrainDeformationStats UCore_TerrainDeformationSystem::GetDeformationStats() const
{
    FCore_TerrainDeformationStats Stats;
    Stats.ActiveDeformations = ActiveDeformations.Num();
    Stats.ActiveRecoveries = ActiveRecoveries.Num();
    Stats.TotalDeformationsProcessed = TotalDeformationsProcessed;
    Stats.CurrentLODLevel = CurrentLODLevel;
    Stats.bSystemEnabled = bDeformationEnabled;
    
    return Stats;
}

void UCore_TerrainDeformationSystem::InitializeLandscapeReferences()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find landscape actor in the world
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        MainLandscape = *ActorItr;
        break; // Use first landscape found
    }
    
    if (MainLandscape)
    {
        LandscapeInfo = MainLandscape->GetLandscapeInfo();
        UE_LOG(LogTemp, Log, TEXT("TerrainDeformation: Found landscape %s"), *MainLandscape->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainDeformation: No landscape found in world"));
    }
}

void UCore_TerrainDeformationSystem::ProcessLandscapeDeformation(const FCore_TerrainDeformation& Deformation)
{
    if (!MainLandscape || !LandscapeInfo)
    {
        return;
    }
    
    // Get landscape heightmap data
    TArray<uint16> HeightData;
    FIntRect DataRect;
    
    if (GetLandscapeHeightData(Deformation.Location, Deformation.Radius, HeightData, DataRect))
    {
        // Modify height data based on deformation
        int32 CenterX = DataRect.Width() / 2;
        int32 CenterY = DataRect.Height() / 2;
        float RadiusSquared = Deformation.Radius * Deformation.Radius;
        
        for (int32 Y = 0; Y < DataRect.Height(); ++Y)
        {
            for (int32 X = 0; X < DataRect.Width(); ++X)
            {
                float DistanceSquared = FMath::Square(X - CenterX) + FMath::Square(Y - CenterY);
                if (DistanceSquared <= RadiusSquared)
                {
                    float Distance = FMath::Sqrt(DistanceSquared);
                    float Falloff = 1.0f - (Distance / Deformation.Radius);
                    Falloff = FMath::Pow(Falloff, 2.0f); // Smooth falloff
                    
                    int32 Index = Y * DataRect.Width() + X;
                    if (HeightData.IsValidIndex(Index))
                    {
                        float HeightChange = Deformation.Depth * Falloff;
                        int32 NewHeight = FMath::Clamp(
                            (int32)HeightData[Index] + (int32)(HeightChange * 256.0f), 
                            0, 
                            65535
                        );
                        HeightData[Index] = (uint16)NewHeight;
                    }
                }
            }
        }
        
        // Apply modified height data back to landscape
        SetLandscapeHeightData(DataRect, HeightData);
    }
}

void UCore_TerrainDeformationSystem::UpdateRecoveryOperations(float DeltaTime)
{
    for (int32 i = ActiveRecoveries.Num() - 1; i >= 0; --i)
    {
        FCore_TerrainRecovery& Recovery = ActiveRecoveries[i];
        Recovery.ElapsedTime += DeltaTime;
        
        float RecoveryProgress = Recovery.ElapsedTime / Recovery.TotalRecoveryTime;
        
        if (RecoveryProgress >= 1.0f)
        {
            // Recovery complete
            ActiveRecoveries.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("TerrainDeformation: Recovery completed at %s"), *Recovery.Location.ToString());
        }
        else
        {
            // Apply gradual recovery (simplified implementation)
            // In a full implementation, this would gradually restore the original heightmap
        }
    }
}

void UCore_TerrainDeformationSystem::CleanupExpiredDeformations()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveDeformations.Num() - 1; i >= 0; --i)
    {
        const FCore_TerrainDeformation& Deformation = ActiveDeformations[i];
        
        if (!Deformation.bPermanent && Deformation.DecayTime > 0.0f)
        {
            float Age = CurrentTime - Deformation.CreationTime;
            if (Age >= Deformation.DecayTime)
            {
                ActiveDeformations.RemoveAt(i);
            }
        }
    }
}

TArray<FVector2D> UCore_TerrainDeformationSystem::CalculateDeformationShape(ECore_TerrainDeformationType Type, float Radius)
{
    TArray<FVector2D> ShapePoints;
    
    // Generate shape points based on deformation type
    int32 NumPoints = 16;
    
    for (int32 i = 0; i < NumPoints; ++i)
    {
        float Angle = (2.0f * PI * i) / NumPoints;
        float X = FMath::Cos(Angle) * Radius;
        float Y = FMath::Sin(Angle) * Radius;
        
        // Modify shape based on type
        switch (Type)
        {
            case ECore_TerrainDeformationType::Footprint:
                // Oval shape for footprints
                Y *= 0.7f;
                break;
            case ECore_TerrainDeformationType::Impact:
                // Circular shape for impacts
                break;
            case ECore_TerrainDeformationType::Erosion:
                // Irregular shape for erosion
                X += FMath::RandRange(-Radius * 0.1f, Radius * 0.1f);
                Y += FMath::RandRange(-Radius * 0.1f, Radius * 0.1f);
                break;
        }
        
        ShapePoints.Add(FVector2D(X, Y));
    }
    
    return ShapePoints;
}

bool UCore_TerrainDeformationSystem::GetLandscapeHeightData(const FVector& WorldLocation, float Radius, TArray<uint16>& HeightData, FIntRect& DataRect)
{
    if (!LandscapeInfo)
    {
        return false;
    }
    
    // Convert world location to landscape coordinates
    FVector LandscapeLocation = MainLandscape->GetTransform().InverseTransformPosition(WorldLocation);
    
    // Calculate data rectangle
    int32 RadiusInQuads = FMath::CeilToInt(Radius / MainLandscape->GetActorScale().X);
    DataRect = FIntRect(
        FMath::FloorToInt(LandscapeLocation.X) - RadiusInQuads,
        FMath::FloorToInt(LandscapeLocation.Y) - RadiusInQuads,
        FMath::CeilToInt(LandscapeLocation.X) + RadiusInQuads,
        FMath::CeilToInt(LandscapeLocation.Y) + RadiusInQuads
    );
    
    // Simplified height data access (in real implementation, would use FLandscapeEditDataInterface)
    int32 DataSize = DataRect.Width() * DataRect.Height();
    HeightData.SetNum(DataSize);
    
    // Fill with dummy data for now
    for (int32 i = 0; i < DataSize; ++i)
    {
        HeightData[i] = 32768; // Mid-range height value
    }
    
    return true;
}

bool UCore_TerrainDeformationSystem::SetLandscapeHeightData(const FIntRect& DataRect, const TArray<uint16>& HeightData)
{
    if (!LandscapeInfo)
    {
        return false;
    }
    
    // Simplified height data setting (in real implementation, would use FLandscapeEditDataInterface)
    // This is a complex operation that requires proper landscape editing interface
    
    UE_LOG(LogTemp, Log, TEXT("TerrainDeformation: Height data applied to rect (%d,%d) to (%d,%d)"), 
           DataRect.Min.X, DataRect.Min.Y, DataRect.Max.X, DataRect.Max.Y);
    
    return true;
}