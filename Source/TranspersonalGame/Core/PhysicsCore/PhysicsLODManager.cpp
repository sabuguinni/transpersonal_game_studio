#include "PhysicsLODManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsLOD, Log, All);

UPhysicsLODManager::UPhysicsLODManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update LOD every 100ms
    
    // Default LOD distances (in Unreal Units - 1 UU = 1 cm)
    HighDetailDistance = 1000.0f;    // 10 meters
    MediumDetailDistance = 3000.0f;  // 30 meters
    LowDetailDistance = 8000.0f;     // 80 meters
    
    // Performance thresholds
    TargetFrameTime = 16.67f; // 60 FPS target (16.67ms per frame)
    PhysicsObjectLimit = 500;
    
    bEnableDebugVisualization = false;
    bAdaptiveQuality = true;
    
    CurrentLODLevel = ECore_PhysicsLODLevel::High;
    FrameTimeHistory.SetNum(60); // Store 1 second of frame times at 60fps
    FrameTimeIndex = 0;
}

void UPhysicsLODManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to world
    World = GetWorld();
    if (!World)
    {
        UE_LOG(LogPhysicsLOD, Error, TEXT("PhysicsLODManager: No valid world found"));
        return;
    }
    
    // Initialize frame time history
    for (int32 i = 0; i < FrameTimeHistory.Num(); i++)
    {
        FrameTimeHistory[i] = TargetFrameTime;
    }
    
    UE_LOG(LogPhysicsLOD, Log, TEXT("PhysicsLODManager initialized successfully"));
}

void UPhysicsLODManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!World)
    {
        return;
    }
    
    // Update frame time history for adaptive quality
    UpdateFrameTimeHistory(DeltaTime * 1000.0f); // Convert to milliseconds
    
    // Get player camera location for distance calculations
    FVector CameraLocation = GetPlayerCameraLocation();
    if (CameraLocation.IsZero())
    {
        return; // No valid camera found
    }
    
    // Update LOD for all physics objects
    UpdatePhysicsLOD(CameraLocation);
    
    // Adaptive quality adjustment based on performance
    if (bAdaptiveQuality)
    {
        UpdateAdaptiveQuality();
    }
    
    // Debug visualization
    if (bEnableDebugVisualization)
    {
        DrawDebugInfo(CameraLocation);
    }
}

FVector UPhysicsLODManager::GetPlayerCameraLocation() const
{
    if (!World)
    {
        return FVector::ZeroVector;
    }
    
    // Try to get the first player controller
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return FVector::ZeroVector;
    }
    
    // Get camera location from possessed pawn
    APawn* PlayerPawn = PlayerController->GetPawn();
    if (PlayerPawn)
    {
        // Try to find a camera component
        UCameraComponent* CameraComp = PlayerPawn->FindComponentByClass<UCameraComponent>();
        if (CameraComp)
        {
            return CameraComp->GetComponentLocation();
        }
        
        // Fall back to pawn location + eye height
        return PlayerPawn->GetActorLocation() + FVector(0, 0, 180.0f); // Approximate eye height
    }
    
    // Last resort: use player controller location
    FVector Location;
    FRotator Rotation;
    PlayerController->GetPlayerViewPoint(Location, Rotation);
    return Location;
}

void UPhysicsLODManager::UpdatePhysicsLOD(const FVector& CameraLocation)
{
    if (!World)
    {
        return;
    }
    
    int32 ProcessedObjects = 0;
    int32 HighLODObjects = 0;
    int32 MediumLODObjects = 0;
    int32 LowLODObjects = 0;
    int32 DisabledObjects = 0;
    
    // Iterate through all actors with physics components
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        // Skip player pawns and essential actors
        if (Actor->IsA<APawn>())
        {
            continue;
        }
        
        // Process static mesh components
        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
        
        for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
        {
            if (MeshComp && MeshComp->IsSimulatingPhysics())
            {
                float Distance = FVector::Dist(CameraLocation, Actor->GetActorLocation());
                ECore_PhysicsLODLevel LODLevel = CalculateLODLevel(Distance);
                
                ApplyLODToComponent(MeshComp, LODLevel);
                ProcessedObjects++;
                
                // Count objects by LOD level
                switch (LODLevel)
                {
                    case ECore_PhysicsLODLevel::High:
                        HighLODObjects++;
                        break;
                    case ECore_PhysicsLODLevel::Medium:
                        MediumLODObjects++;
                        break;
                    case ECore_PhysicsLODLevel::Low:
                        LowLODObjects++;
                        break;
                    case ECore_PhysicsLODLevel::Disabled:
                        DisabledObjects++;
                        break;
                }
            }
        }
        
        // Limit processing to avoid frame drops
        if (ProcessedObjects >= PhysicsObjectLimit)
        {
            break;
        }
    }
    
    // Store statistics
    LODStatistics.HighLODObjects = HighLODObjects;
    LODStatistics.MediumLODObjects = MediumLODObjects;
    LODStatistics.LowLODObjects = LowLODObjects;
    LODStatistics.DisabledObjects = DisabledObjects;
    LODStatistics.TotalProcessedObjects = ProcessedObjects;
}

ECore_PhysicsLODLevel UPhysicsLODManager::CalculateLODLevel(float Distance) const
{
    // Apply global LOD level modifier
    float HighDistance = HighDetailDistance;
    float MediumDistance = MediumDetailDistance;
    float LowDistance = LowDetailDistance;
    
    // Adjust distances based on current global LOD level
    switch (CurrentLODLevel)
    {
        case ECore_PhysicsLODLevel::High:
            // No adjustment needed
            break;
        case ECore_PhysicsLODLevel::Medium:
            HighDistance *= 0.7f;
            MediumDistance *= 0.7f;
            LowDistance *= 0.7f;
            break;
        case ECore_PhysicsLODLevel::Low:
            HighDistance *= 0.5f;
            MediumDistance *= 0.5f;
            LowDistance *= 0.5f;
            break;
        case ECore_PhysicsLODLevel::Disabled:
            return ECore_PhysicsLODLevel::Disabled;
    }
    
    if (Distance <= HighDistance)
    {
        return ECore_PhysicsLODLevel::High;
    }
    else if (Distance <= MediumDistance)
    {
        return ECore_PhysicsLODLevel::Medium;
    }
    else if (Distance <= LowDistance)
    {
        return ECore_PhysicsLODLevel::Low;
    }
    else
    {
        return ECore_PhysicsLODLevel::Disabled;
    }
}

void UPhysicsLODManager::ApplyLODToComponent(UPrimitiveComponent* Component, ECore_PhysicsLODLevel LODLevel)
{
    if (!Component)
    {
        return;
    }
    
    switch (LODLevel)
    {
        case ECore_PhysicsLODLevel::High:
            // Full physics simulation
            Component->SetSimulatePhysics(true);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetNotifyRigidBodyCollision(true);
            break;
            
        case ECore_PhysicsLODLevel::Medium:
            // Reduced physics simulation
            Component->SetSimulatePhysics(true);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetNotifyRigidBodyCollision(false); // Disable collision events
            break;
            
        case ECore_PhysicsLODLevel::Low:
            // Kinematic only (no physics simulation but collision detection)
            Component->SetSimulatePhysics(false);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            Component->SetNotifyRigidBodyCollision(false);
            break;
            
        case ECore_PhysicsLODLevel::Disabled:
            // No physics or collision
            Component->SetSimulatePhysics(false);
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Component->SetNotifyRigidBodyCollision(false);
            break;
    }
}

void UPhysicsLODManager::UpdateFrameTimeHistory(float FrameTimeMs)
{
    FrameTimeHistory[FrameTimeIndex] = FrameTimeMs;
    FrameTimeIndex = (FrameTimeIndex + 1) % FrameTimeHistory.Num();
}

float UPhysicsLODManager::GetAverageFrameTime() const
{
    float Total = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Total += FrameTime;
    }
    return Total / FrameTimeHistory.Num();
}

void UPhysicsLODManager::UpdateAdaptiveQuality()
{
    float AverageFrameTime = GetAverageFrameTime();
    
    // If we're consistently above target frame time, reduce quality
    if (AverageFrameTime > TargetFrameTime * 1.2f) // 20% tolerance
    {
        if (CurrentLODLevel == ECore_PhysicsLODLevel::High)
        {
            CurrentLODLevel = ECore_PhysicsLODLevel::Medium;
            UE_LOG(LogPhysicsLOD, Warning, TEXT("Reducing physics LOD to Medium due to performance"));
        }
        else if (CurrentLODLevel == ECore_PhysicsLODLevel::Medium)
        {
            CurrentLODLevel = ECore_PhysicsLODLevel::Low;
            UE_LOG(LogPhysicsLOD, Warning, TEXT("Reducing physics LOD to Low due to performance"));
        }
    }
    // If we're consistently below target frame time, increase quality
    else if (AverageFrameTime < TargetFrameTime * 0.8f) // 20% headroom
    {
        if (CurrentLODLevel == ECore_PhysicsLODLevel::Low)
        {
            CurrentLODLevel = ECore_PhysicsLODLevel::Medium;
            UE_LOG(LogPhysicsLOD, Log, TEXT("Increasing physics LOD to Medium due to good performance"));
        }
        else if (CurrentLODLevel == ECore_PhysicsLODLevel::Medium)
        {
            CurrentLODLevel = ECore_PhysicsLODLevel::High;
            UE_LOG(LogPhysicsLOD, Log, TEXT("Increasing physics LOD to High due to good performance"));
        }
    }
}

void UPhysicsLODManager::DrawDebugInfo(const FVector& CameraLocation)
{
    if (!World)
    {
        return;
    }
    
    // Draw LOD distance spheres
    DrawDebugSphere(World, CameraLocation, HighDetailDistance, 32, FColor::Green, false, -1.0f, 0, 10.0f);
    DrawDebugSphere(World, CameraLocation, MediumDetailDistance, 32, FColor::Yellow, false, -1.0f, 0, 10.0f);
    DrawDebugSphere(World, CameraLocation, LowDetailDistance, 32, FColor::Red, false, -1.0f, 0, 10.0f);
    
    // Draw text info
    FString DebugText = FString::Printf(TEXT("Physics LOD Manager\nGlobal LOD: %s\nAvg Frame Time: %.2fms\nObjects: H:%d M:%d L:%d D:%d"),
        *UEnum::GetValueAsString(CurrentLODLevel),
        GetAverageFrameTime(),
        LODStatistics.HighLODObjects,
        LODStatistics.MediumLODObjects,
        LODStatistics.LowLODObjects,
        LODStatistics.DisabledObjects
    );
    
    DrawDebugString(World, CameraLocation + FVector(0, 0, 200), DebugText, nullptr, FColor::White, 0.0f);
}

void UPhysicsLODManager::SetGlobalLODLevel(ECore_PhysicsLODLevel NewLODLevel)
{
    if (CurrentLODLevel != NewLODLevel)
    {
        CurrentLODLevel = NewLODLevel;
        UE_LOG(LogPhysicsLOD, Log, TEXT("Global physics LOD level set to: %s"), *UEnum::GetValueAsString(NewLODLevel));
    }
}

void UPhysicsLODManager::SetLODDistances(float High, float Medium, float Low)
{
    HighDetailDistance = High;
    MediumDetailDistance = Medium;
    LowDetailDistance = Low;
    
    UE_LOG(LogPhysicsLOD, Log, TEXT("LOD distances updated: High=%.0f, Medium=%.0f, Low=%.0f"), High, Medium, Low);
}

FCore_PhysicsLODStatistics UPhysicsLODManager::GetLODStatistics() const
{
    return LODStatistics;
}

void UPhysicsLODManager::EnableDebugVisualization(bool bEnable)
{
    bEnableDebugVisualization = bEnable;
}

void UPhysicsLODManager::SetAdaptiveQuality(bool bEnable)
{
    bAdaptiveQuality = bEnable;
}

void UPhysicsLODManager::SetTargetFrameTime(float TargetMs)
{
    TargetFrameTime = TargetMs;
}

void UPhysicsLODManager::SetPhysicsObjectLimit(int32 Limit)
{
    PhysicsObjectLimit = FMath::Max(1, Limit);
}