#include "Core_TerrainPhysicsSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Landscape/LandscapeComponent.h"
#include "Landscape/LandscapeInfo.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UCore_TerrainPhysicsSystem::UCore_TerrainPhysicsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // === DEFAULT TERRAIN PROPERTIES ===
    bEnableTerrainDeformation = true;
    DeformationThreshold = 500.0f;
    MaxDeformationDepth = 50.0f;
    DeformationRecoveryRate = 2.0f;
    
    // === ENVIRONMENTAL DEFAULTS ===
    WindForce = FVector(0.0f, 100.0f, 0.0f);
    TerrainGravityModifier = 1.0f;
    bEnableMudPhysics = true;
    MudViscosity = 0.5f;
    
    // === PERFORMANCE DEFAULTS ===
    bEnableTerrainLOD = true;
    HighDetailDistance = 1000.0f;
    MediumDetailDistance = 5000.0f;
    LowDetailDistance = 10000.0f;
    
    // === INTERNAL INITIALIZATION ===
    MainLandscape = nullptr;
    LastPerformanceCheck = 0.0f;
    PhysicsCalculationsPerSecond = 0;
    
    InitializeDefaultSurfaces();
}

void UCore_TerrainPhysicsSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsSystem: Initializing terrain physics..."));
    
    // Find the main landscape in the world
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
        {
            ALandscape* Landscape = *ActorItr;
            if (Landscape && !MainLandscape)
            {
                MainLandscape = Landscape;
                InitializeTerrainPhysics(Landscape);
                break;
            }
        }
    }
    
    if (!ValidateTerrainConfiguration())
    {
        UE_LOG(LogTemp, Error, TEXT("Core_TerrainPhysicsSystem: Invalid terrain configuration detected!"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsSystem: Terrain physics system initialized successfully"));
}

void UCore_TerrainPhysicsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!MainLandscape)
        return;
    
    // Update core terrain physics systems
    UpdateTerrainPhysics(DeltaTime);
    
    // Process terrain deformation if enabled
    if (bEnableTerrainDeformation)
    {
        ProcessTerrainDeformation(DeltaTime);
    }
    
    // Update collision LOD system
    if (bEnableTerrainLOD)
    {
        UpdateTerrainLOD();
    }
    
    // Apply environmental forces
    ApplyEnvironmentalForces(DeltaTime);
    
    // Performance monitoring
    PhysicsCalculationsPerSecond++;
    if (GetWorld()->GetTimeSeconds() - LastPerformanceCheck > 1.0f)
    {
        LastPerformanceCheck = GetWorld()->GetTimeSeconds();
        PhysicsCalculationsPerSecond = 0;
    }
}

void UCore_TerrainPhysicsSystem::InitializeDefaultSurfaces()
{
    // === SURFACE FRICTION VALUES ===
    SurfaceFrictionMap.Add(TEXT("Grass"), 0.7f);
    SurfaceFrictionMap.Add(TEXT("Dirt"), 0.6f);
    SurfaceFrictionMap.Add(TEXT("Rock"), 0.9f);
    SurfaceFrictionMap.Add(TEXT("Sand"), 0.4f);
    SurfaceFrictionMap.Add(TEXT("Mud"), 0.2f);
    SurfaceFrictionMap.Add(TEXT("Ice"), 0.1f);
    SurfaceFrictionMap.Add(TEXT("Water"), 0.0f);
    
    // === SURFACE BOUNCINESS VALUES ===
    SurfaceBounciness.Add(TEXT("Grass"), 0.1f);
    SurfaceBounciness.Add(TEXT("Dirt"), 0.05f);
    SurfaceBounciness.Add(TEXT("Rock"), 0.3f);
    SurfaceBounciness.Add(TEXT("Sand"), 0.02f);
    SurfaceBounciness.Add(TEXT("Mud"), 0.0f);
    SurfaceBounciness.Add(TEXT("Ice"), 0.2f);
    SurfaceBounciness.Add(TEXT("Water"), 0.0f);
    
    // === SURFACE DENSITY VALUES ===
    SurfaceDensity.Add(TEXT("Grass"), 1.0f);
    SurfaceDensity.Add(TEXT("Dirt"), 1.2f);
    SurfaceDensity.Add(TEXT("Rock"), 2.5f);
    SurfaceDensity.Add(TEXT("Sand"), 0.8f);
    SurfaceDensity.Add(TEXT("Mud"), 0.6f);
    SurfaceDensity.Add(TEXT("Ice"), 0.9f);
    SurfaceDensity.Add(TEXT("Water"), 0.3f);
}

float UCore_TerrainPhysicsSystem::GetSurfaceFriction(const FVector& WorldLocation)
{
    FString MaterialType = GetSurfaceMaterial(WorldLocation);
    
    if (SurfaceFrictionMap.Contains(MaterialType))
    {
        return SurfaceFrictionMap[MaterialType];
    }
    
    return 0.5f; // Default friction
}

FString UCore_TerrainPhysicsSystem::GetSurfaceMaterial(const FVector& WorldLocation)
{
    if (!MainLandscape)
        return TEXT("Rock");
    
    // Simplified material detection based on height and slope
    float Height = WorldLocation.Z;
    float Slope = GetTerrainSlope(WorldLocation);
    
    // Basic material classification
    if (Height < 0.0f)
        return TEXT("Water");
    else if (Slope > 45.0f)
        return TEXT("Rock");
    else if (Height < 100.0f && Slope < 10.0f)
        return TEXT("Mud");
    else if (Height < 500.0f)
        return TEXT("Grass");
    else if (Height < 1000.0f)
        return TEXT("Dirt");
    else
        return TEXT("Rock");
}

bool UCore_TerrainPhysicsSystem::ApplyTerrainForce(const FVector& WorldLocation, const FVector& Force, float Radius)
{
    if (!bEnableTerrainDeformation || !MainLandscape)
        return false;
    
    float ForceStrength = Force.Size();
    if (ForceStrength < DeformationThreshold)
        return false;
    
    // Convert world location to landscape coordinates
    FIntPoint LandscapeCoord = FIntPoint(
        FMath::RoundToInt(WorldLocation.X / 100.0f),
        FMath::RoundToInt(WorldLocation.Y / 100.0f)
    );
    
    // Calculate deformation amount
    float DeformationAmount = FMath::Clamp(
        ForceStrength / DeformationThreshold * MaxDeformationDepth,
        0.0f,
        MaxDeformationDepth
    );
    
    // Apply deformation to the map
    if (DeformationMap.Contains(LandscapeCoord))
    {
        DeformationMap[LandscapeCoord] = FMath::Max(DeformationMap[LandscapeCoord], DeformationAmount);
    }
    else
    {
        DeformationMap.Add(LandscapeCoord, DeformationAmount);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Terrain deformation applied at (%f, %f) with strength %f"), 
           WorldLocation.X, WorldLocation.Y, DeformationAmount);
    
    return true;
}

bool UCore_TerrainPhysicsSystem::IsLocationInMud(const FVector& WorldLocation)
{
    if (!bEnableMudPhysics)
        return false;
    
    FString MaterialType = GetSurfaceMaterial(WorldLocation);
    return MaterialType == TEXT("Mud") || MaterialType == TEXT("Water");
}

float UCore_TerrainPhysicsSystem::GetTerrainSlope(const FVector& WorldLocation)
{
    if (!MainLandscape)
        return 0.0f;
    
    // Sample height at multiple points to calculate slope
    float SampleDistance = 100.0f;
    FVector ForwardLocation = WorldLocation + FVector(SampleDistance, 0, 0);
    FVector RightLocation = WorldLocation + FVector(0, SampleDistance, 0);
    
    float CenterHeight = WorldLocation.Z;
    float ForwardHeight = MainLandscape->GetActorLocation().Z; // Simplified
    float RightHeight = MainLandscape->GetActorLocation().Z;   // Simplified
    
    // Calculate slope using cross product
    FVector Forward = FVector(SampleDistance, 0, ForwardHeight - CenterHeight);
    FVector Right = FVector(0, SampleDistance, RightHeight - CenterHeight);
    FVector Normal = FVector::CrossProduct(Forward, Right).GetSafeNormal();
    
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));
    return SlopeAngle;
}

void UCore_TerrainPhysicsSystem::InitializeTerrainPhysics(ALandscape* TargetLandscape)
{
    if (!TargetLandscape)
        return;
    
    MainLandscape = TargetLandscape;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysicsSystem: Initializing physics for landscape: %s"), 
           *TargetLandscape->GetName());
    
    // Configure landscape collision
    ULandscapeInfo* LandscapeInfo = TargetLandscape->GetLandscapeInfo();
    if (LandscapeInfo)
    {
        // Set up collision properties
        for (auto& Component : LandscapeInfo->XYtoComponentMap)
        {
            ULandscapeComponent* LandComp = Component.Value;
            if (LandComp)
            {
                // Configure collision settings
                LandComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                LandComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
            }
        }
    }
}

void UCore_TerrainPhysicsSystem::UpdateTerrainPhysics(float DeltaTime)
{
    // Update physics calculations and apply forces to nearby objects
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Get player character for proximity checks
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerCharacter)
        return;
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    
    // Apply surface-specific physics to the player
    FString SurfaceMaterial = GetSurfaceMaterial(PlayerLocation);
    float SurfaceFriction = GetSurfaceFriction(PlayerLocation);
    
    // Modify player movement based on surface
    if (IsLocationInMud(PlayerLocation))
    {
        // Apply mud physics - reduce movement speed
        UCharacterMovementComponent* MovementComp = PlayerCharacter->GetCharacterMovement();
        if (MovementComp)
        {
            float MudSpeedModifier = 1.0f - (MudViscosity * 0.5f);
            MovementComp->MaxWalkSpeed = MovementComp->GetMaxSpeed() * MudSpeedModifier;
        }
    }
}

void UCore_TerrainPhysicsSystem::ProcessTerrainDeformation(float DeltaTime)
{
    // Process deformation recovery
    TArray<FIntPoint> KeysToRemove;
    
    for (auto& DeformationPair : DeformationMap)
    {
        FIntPoint& Coord = const_cast<FIntPoint&>(DeformationPair.Key);
        float& DeformationValue = DeformationPair.Value;
        
        // Recover deformation over time
        DeformationValue -= DeformationRecoveryRate * DeltaTime;
        
        if (DeformationValue <= 0.0f)
        {
            KeysToRemove.Add(Coord);
        }
    }
    
    // Remove fully recovered deformations
    for (const FIntPoint& Key : KeysToRemove)
    {
        DeformationMap.Remove(Key);
    }
}

void UCore_TerrainPhysicsSystem::UpdateTerrainLOD()
{
    if (!MainLandscape)
        return;
    
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerCharacter)
        return;
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    
    // Simplified LOD system - adjust collision complexity based on distance
    ULandscapeInfo* LandscapeInfo = MainLandscape->GetLandscapeInfo();
    if (LandscapeInfo)
    {
        for (auto& Component : LandscapeInfo->XYtoComponentMap)
        {
            ULandscapeComponent* LandComp = Component.Value;
            if (LandComp)
            {
                float Distance = FVector::Dist(PlayerLocation, LandComp->GetComponentLocation());
                
                // Adjust collision based on distance
                if (Distance < HighDetailDistance)
                {
                    LandComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                }
                else if (Distance < MediumDetailDistance)
                {
                    LandComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                }
                else if (Distance < LowDetailDistance)
                {
                    LandComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                }
                else
                {
                    LandComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                }
            }
        }
    }
}

void UCore_TerrainPhysicsSystem::ApplyEnvironmentalForces(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Apply wind force to physics objects near terrain
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* MeshActor = *ActorItr;
        if (!MeshActor)
            continue;
        
        UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
        if (!MeshComp || !MeshComp->IsSimulatingPhysics())
            continue;
        
        // Apply wind force
        FVector ScaledWindForce = WindForce * DeltaTime;
        MeshComp->AddForce(ScaledWindForce);
    }
}

void UCore_TerrainPhysicsSystem::DrawTerrainPhysicsDebug()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Draw deformation points
    for (const auto& DeformationPair : DeformationMap)
    {
        FVector WorldPos = FVector(
            DeformationPair.Key.X * 100.0f,
            DeformationPair.Key.Y * 100.0f,
            0.0f
        );
        
        float DeformationValue = DeformationPair.Value;
        FColor DebugColor = FColor::Lerp(FColor::Green, FColor::Red, DeformationValue / MaxDeformationDepth);
        
        DrawDebugSphere(World, WorldPos, 50.0f, 8, DebugColor, false, 1.0f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Terrain Physics Debug: %d deformation points active"), DeformationMap.Num());
}

void UCore_TerrainPhysicsSystem::LogTerrainPhysicsStats()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TERRAIN PHYSICS STATS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Active Deformations: %d"), DeformationMap.Num());
    UE_LOG(LogTemp, Warning, TEXT("Physics Calculations/sec: %d"), PhysicsCalculationsPerSecond);
    UE_LOG(LogTemp, Warning, TEXT("Terrain Deformation: %s"), bEnableTerrainDeformation ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("Mud Physics: %s"), bEnableMudPhysics ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("Terrain LOD: %s"), bEnableTerrainLOD ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("Surface Materials: %d"), SurfaceFrictionMap.Num());
}

bool UCore_TerrainPhysicsSystem::ValidateTerrainConfiguration()
{
    bool bIsValid = true;
    
    if (DeformationThreshold <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid DeformationThreshold: %f"), DeformationThreshold);
        bIsValid = false;
    }
    
    if (MaxDeformationDepth <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid MaxDeformationDepth: %f"), MaxDeformationDepth);
        bIsValid = false;
    }
    
    if (SurfaceFrictionMap.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No surface materials configured"));
        bIsValid = false;
    }
    
    return bIsValid;
}