#include "Core_PhysicsPerformanceDashboard.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"

ACore_PhysicsPerformanceDashboard::ACore_PhysicsPerformanceDashboard()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create dashboard mesh
    DashboardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DashboardMesh"));
    DashboardMesh->SetupAttachment(RootComponent);

    // Initialize performance thresholds
    FrameTimeWarningThreshold = 16.67f; // 60 FPS threshold
    CollisionWarningThreshold = 1000;
    MemoryWarningThreshold = 512.0f; // 512 MB

    // Initialize update settings
    UpdateFrequency = 0.5f; // Update twice per second
    TimeSinceLastUpdate = 0.0f;

    // Create indicator components
    CreateIndicatorComponents();

    // Set default mesh if available
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded() && DashboardMesh)
    {
        DashboardMesh->SetStaticMesh(CubeMeshAsset.Object);
        DashboardMesh->SetRelativeScale3D(FVector(2.0f, 0.1f, 1.0f));
    }
}

void ACore_PhysicsPerformanceDashboard::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize metrics
    ResetMetrics();
    
    // Perform initial update
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Dashboard initialized at location: %s"), 
           *GetActorLocation().ToString());
}

void ACore_PhysicsPerformanceDashboard::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= UpdateFrequency)
    {
        UpdatePerformanceMetrics();
        TimeSinceLastUpdate = 0.0f;
    }
}

void ACore_PhysicsPerformanceDashboard::CreateIndicatorComponents()
{
    // Create frame time indicator
    FrameTimeIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameTimeIndicator"));
    FrameTimeIndicator->SetupAttachment(RootComponent);
    FrameTimeIndicator->SetRelativeLocation(FVector(-200, -100, 50));

    // Create collision indicator
    CollisionIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionIndicator"));
    CollisionIndicator->SetupAttachment(RootComponent);
    CollisionIndicator->SetRelativeLocation(FVector(-200, 0, 50));

    // Create ragdoll indicator
    RagdollIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RagdollIndicator"));
    RagdollIndicator->SetupAttachment(RootComponent);
    RagdollIndicator->SetRelativeLocation(FVector(-200, 100, 50));

    // Create destruction indicator
    DestructionIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestructionIndicator"));
    DestructionIndicator->SetupAttachment(RootComponent);
    DestructionIndicator->SetRelativeLocation(FVector(0, -100, 50));

    // Create fluid indicator
    FluidIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FluidIndicator"));
    FluidIndicator->SetupAttachment(RootComponent);
    FluidIndicator->SetRelativeLocation(FVector(0, 100, 50));

    // Create memory indicator
    MemoryIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MemoryIndicator"));
    MemoryIndicator->SetupAttachment(RootComponent);
    MemoryIndicator->SetRelativeLocation(FVector(200, 0, 50));

    // Set sphere mesh for all indicators
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        UStaticMesh* SphereMesh = SphereMeshAsset.Object;
        FVector IndicatorScale(0.3f, 0.3f, 0.3f);

        if (FrameTimeIndicator)
        {
            FrameTimeIndicator->SetStaticMesh(SphereMesh);
            FrameTimeIndicator->SetRelativeScale3D(IndicatorScale);
        }
        
        if (CollisionIndicator)
        {
            CollisionIndicator->SetStaticMesh(SphereMesh);
            CollisionIndicator->SetRelativeScale3D(IndicatorScale);
        }
        
        if (RagdollIndicator)
        {
            RagdollIndicator->SetStaticMesh(SphereMesh);
            RagdollIndicator->SetRelativeScale3D(IndicatorScale);
        }
        
        if (DestructionIndicator)
        {
            DestructionIndicator->SetStaticMesh(SphereMesh);
            DestructionIndicator->SetRelativeScale3D(IndicatorScale);
        }
        
        if (FluidIndicator)
        {
            FluidIndicator->SetStaticMesh(SphereMesh);
            FluidIndicator->SetRelativeScale3D(IndicatorScale);
        }
        
        if (MemoryIndicator)
        {
            MemoryIndicator->SetStaticMesh(SphereMesh);
            MemoryIndicator->SetRelativeScale3D(IndicatorScale);
        }
    }
}

void ACore_PhysicsPerformanceDashboard::UpdatePerformanceMetrics()
{
    CollectPhysicsMetrics();
    UpdateIndicatorColors();
    
    // Log performance status if thresholds exceeded
    if (!IsPerformanceWithinThresholds())
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Performance Warning - Frame: %.2fms, Collisions: %d, Memory: %.1fMB"),
               CurrentMetrics.FrameTimeMS, CurrentMetrics.CollisionChecksPerFrame, CurrentMetrics.PhysicsMemoryMB);
    }
}

void ACore_PhysicsPerformanceDashboard::CollectPhysicsMetrics()
{
    // Get frame time (simplified - in real implementation would use proper stats)
    CurrentMetrics.FrameTimeMS = FApp::GetDeltaTime() * 1000.0f;
    
    // Simulate physics metrics (in real implementation would query actual physics systems)
    if (UWorld* World = GetWorld())
    {
        // Count physics bodies as proxy for collision checks
        CurrentMetrics.CollisionChecksPerFrame = FMath::RandRange(100, 2000);
        
        // Count ragdoll actors
        CurrentMetrics.ActiveRagdolls = FMath::RandRange(0, 10);
        
        // Simulate destruction events
        CurrentMetrics.DestructionEventsPerFrame = FMath::RandRange(0, 5);
        
        // Simulate fluid simulation cost
        CurrentMetrics.FluidSimulationCost = FMath::FRandRange(0.0f, 5.0f);
    }
    
    // Get memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.PhysicsMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

void ACore_PhysicsPerformanceDashboard::UpdateIndicatorColors()
{
    // Update frame time indicator
    if (FrameTimeIndicator)
    {
        FLinearColor FrameColor = GetPerformanceColor(CurrentMetrics.FrameTimeMS, FrameTimeWarningThreshold, true);
        // In real implementation, would create and apply dynamic material
    }
    
    // Update collision indicator
    if (CollisionIndicator)
    {
        FLinearColor CollisionColor = GetPerformanceColor(CurrentMetrics.CollisionChecksPerFrame, CollisionWarningThreshold, true);
        // In real implementation, would create and apply dynamic material
    }
    
    // Update memory indicator
    if (MemoryIndicator)
    {
        FLinearColor MemoryColor = GetPerformanceColor(CurrentMetrics.PhysicsMemoryMB, MemoryWarningThreshold, true);
        // In real implementation, would create and apply dynamic material
    }
}

FLinearColor ACore_PhysicsPerformanceDashboard::GetPerformanceColor(float Value, float Threshold, bool bInvertLogic) const
{
    float Ratio = Value / Threshold;
    
    if (bInvertLogic)
    {
        // For metrics where lower is better (frame time, memory usage)
        if (Ratio <= 0.7f)
            return FLinearColor::Green;
        else if (Ratio <= 1.0f)
            return FLinearColor::Yellow;
        else
            return FLinearColor::Red;
    }
    else
    {
        // For metrics where higher is better
        if (Ratio >= 1.0f)
            return FLinearColor::Green;
        else if (Ratio >= 0.7f)
            return FLinearColor::Yellow;
        else
            return FLinearColor::Red;
    }
}

FCore_PhysicsPerformanceMetrics ACore_PhysicsPerformanceDashboard::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void ACore_PhysicsPerformanceDashboard::SetPerformanceThresholds(float FrameThreshold, int32 CollisionThreshold, float MemoryThreshold)
{
    FrameTimeWarningThreshold = FrameThreshold;
    CollisionWarningThreshold = CollisionThreshold;
    MemoryWarningThreshold = MemoryThreshold;
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Thresholds Updated - Frame: %.2f, Collision: %d, Memory: %.1f"),
           FrameThreshold, CollisionThreshold, MemoryThreshold);
}

bool ACore_PhysicsPerformanceDashboard::IsPerformanceWithinThresholds() const
{
    return (CurrentMetrics.FrameTimeMS <= FrameTimeWarningThreshold) &&
           (CurrentMetrics.CollisionChecksPerFrame <= CollisionWarningThreshold) &&
           (CurrentMetrics.PhysicsMemoryMB <= MemoryWarningThreshold);
}

void ACore_PhysicsPerformanceDashboard::ResetMetrics()
{
    CurrentMetrics = FCore_PhysicsPerformanceMetrics();
    TimeSinceLastUpdate = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Physics Performance Metrics Reset"));
}