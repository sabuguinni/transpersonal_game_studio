#include "Core_PhysicsSimulation.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"

UCore_PhysicsSimulation::UCore_PhysicsSimulation()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    PhysicsSettings = FCore_PhysicsSettings();
    bUseCustomGravity = false;
    CustomGravityDirection = FVector(0.0f, 0.0f, -980.0f);
    PhysicsUpdateRate = 60.0f;
    MaxPhysicsObjects = 1000;
    CurrentPhysicsObjects = 0;
    AverageFrameTime = 0.0f;
}

void UCore_PhysicsSimulation::BeginPlay()
{
    Super::BeginPlay();
    
    ApplyPhysicsSettings();
    
    if (bUseCustomGravity)
    {
        SetCustomGravity(CustomGravityDirection);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSimulation: Initialized with quality level %d"), 
           (int32)PhysicsSettings.QualityLevel);
}

void UCore_PhysicsSimulation::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePhysicsStats();
    
    // Performance monitoring
    FrameTimeAccumulator += DeltaTime;
    FrameCounter++;
    
    if (FrameCounter >= 60) // Update average every 60 frames
    {
        AverageFrameTime = FrameTimeAccumulator / FrameCounter;
        FrameTimeAccumulator = 0.0f;
        FrameCounter = 0;
        
        // Auto-optimize if performance is poor
        if (AverageFrameTime > 0.020f) // Above 20ms (below 50fps)
        {
            OptimizePhysicsPerformance();
        }
    }
}

void UCore_PhysicsSimulation::ApplyPhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsSimulation: No valid world found"));
        return;
    }

    // Apply physics settings to world
    if (UPhysicsSettings* Settings = UPhysicsSettings::Get())
    {
        Settings->DefaultGravityZ = CustomGravityDirection.Z * PhysicsSettings.GravityScale;
        Settings->bEnableAsyncScene = PhysicsSettings.bEnableAsyncPhysics;
        
        ApplyQualitySettings();
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSimulation: Applied physics settings - Gravity: %f, Async: %s"), 
               Settings->DefaultGravityZ, 
               PhysicsSettings.bEnableAsyncPhysics ? TEXT("True") : TEXT("False"));
    }
}

void UCore_PhysicsSimulation::SetPhysicsQuality(ECore_PhysicsQuality NewQuality)
{
    PhysicsSettings.QualityLevel = NewQuality;
    ApplyQualitySettings();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSimulation: Physics quality set to %d"), (int32)NewQuality);
}

void UCore_PhysicsSimulation::EnableAsyncPhysics(bool bEnable)
{
    PhysicsSettings.bEnableAsyncPhysics = bEnable;
    ApplyPhysicsSettings();
}

void UCore_PhysicsSimulation::SetCustomGravity(FVector NewGravity)
{
    CustomGravityDirection = NewGravity;
    bUseCustomGravity = true;
    
    UWorld* World = GetWorld();
    if (World && UPhysicsSettings::Get())
    {
        UPhysicsSettings::Get()->DefaultGravityZ = NewGravity.Z;
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSimulation: Custom gravity set to %s"), *NewGravity.ToString());
    }
}

void UCore_PhysicsSimulation::GetPhysicsStats(int32& OutActiveObjects, float& OutFrameTime)
{
    OutActiveObjects = CurrentPhysicsObjects;
    OutFrameTime = AverageFrameTime;
}

void UCore_PhysicsSimulation::OptimizePhysicsPerformance()
{
    // Reduce physics quality if performance is poor
    if (PhysicsSettings.QualityLevel == ECore_PhysicsQuality::Ultra)
    {
        SetPhysicsQuality(ECore_PhysicsQuality::High);
    }
    else if (PhysicsSettings.QualityLevel == ECore_PhysicsQuality::High)
    {
        SetPhysicsQuality(ECore_PhysicsQuality::Medium);
    }
    else if (PhysicsSettings.QualityLevel == ECore_PhysicsQuality::Medium)
    {
        SetPhysicsQuality(ECore_PhysicsQuality::Low);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSimulation: Auto-optimized physics quality due to poor performance"));
}

void UCore_PhysicsSimulation::UpdatePhysicsStats()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count physics objects in the world
    CurrentPhysicsObjects = 0;
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                CurrentPhysicsObjects++;
            }
        }
    }
    
    // Warn if approaching limits
    if (CurrentPhysicsObjects > MaxPhysicsObjects * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSimulation: Approaching physics object limit - %d/%d"), 
               CurrentPhysicsObjects, MaxPhysicsObjects);
    }
}

void UCore_PhysicsSimulation::ApplyQualitySettings()
{
    switch (PhysicsSettings.QualityLevel)
    {
        case ECore_PhysicsQuality::Low:
            PhysicsSettings.TimeStep = 0.025f; // 40fps
            PhysicsSettings.MaxSubSteps = 2;
            MaxPhysicsObjects = 500;
            break;
            
        case ECore_PhysicsQuality::Medium:
            PhysicsSettings.TimeStep = 0.020f; // 50fps
            PhysicsSettings.MaxSubSteps = 4;
            MaxPhysicsObjects = 750;
            break;
            
        case ECore_PhysicsQuality::High:
            PhysicsSettings.TimeStep = 0.016667f; // 60fps
            PhysicsSettings.MaxSubSteps = 6;
            MaxPhysicsObjects = 1000;
            break;
            
        case ECore_PhysicsQuality::Ultra:
            PhysicsSettings.TimeStep = 0.0125f; // 80fps
            PhysicsSettings.MaxSubSteps = 8;
            MaxPhysicsObjects = 1500;
            break;
    }
}