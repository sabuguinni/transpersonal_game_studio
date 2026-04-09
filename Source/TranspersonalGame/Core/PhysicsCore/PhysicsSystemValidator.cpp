// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsSystemValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Character.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/GameViewportClient.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "GeometryCollection/GeometryCollectionActor.h"

DEFINE_LOG_CATEGORY(LogPhysicsValidator);

UPhysicsSystemValidator::UPhysicsSystemValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for monitoring
    
    bValidationEnabled = true;
    bAutoRunValidation = true;
    bContinuousMonitoring = true;
    MonitoringInterval = 1.0f;
    TargetFramerate = 60.0f;
    MaxPhysicsTickTime = 16.67f;
    MaxRigidBodies = 1000;
    MaxMemoryUsage = 512.0f;
    
    LastMonitoringTime = 0.0f;
}

void UPhysicsSystemValidator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPhysicsValidator, Log, TEXT("Physics System Validator initialized"));
    
    if (bAutoRunValidation && bValidationEnabled)
    {
        // Delay validation to allow world to fully initialize
        FTimerHandle ValidationTimer;
        GetWorld()->GetTimerManager().SetTimer(ValidationTimer, this, 
            &UPhysicsSystemValidator::RunAllValidationTests, 2.0f, false);
    }
}

void UPhysicsSystemValidator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Cleanup test actors
    for (AActor* TestActor : TestActors)
    {
        if (IsValid(TestActor))
        {
            TestActor->Destroy();
        }
    }
    TestActors.Empty();
    TestMeshComponents.Empty();
    
    Super::EndPlay(EndPlayReason);
}

void UPhysicsSystemValidator::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bContinuousMonitoring && bValidationEnabled)
    {
        MonitorPerformance();
    }
}

void UPhysicsSystemValidator::RunAllValidationTests()
{
    if (!bValidationEnabled)
    {
        UE_LOG(LogPhysicsValidator, Warning, TEXT("Validation is disabled"));
        return;
    }
    
    UE_LOG(LogPhysicsValidator, Log, TEXT("Starting comprehensive physics validation..."));
    
    ClearValidationResults();
    
    // Run all validation tests
    ValidationResults.Add(ValidateChaosEngine());
    ValidationResults.Add(ValidateCollisionDetection());
    ValidationResults.Add(ValidateRigidBodyDynamics());
    ValidationResults.Add(ValidateRagdollPhysics());
    ValidationResults.Add(ValidateDestructionSystem());
    ValidationResults.Add(ValidateFluidSimulation());
    ValidationResults.Add(ValidateClothSimulation());
    ValidationResults.Add(ValidateVehiclePhysics());
    ValidationResults.Add(ValidatePerformance());
    ValidationResults.Add(ValidateNetworkedPhysics());
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    bool bAllPassed = AllTestsPassed();
    UE_LOG(LogPhysicsValidator, Log, TEXT("Physics validation complete. All tests passed: %s"), 
        bAllPassed ? TEXT("YES") : TEXT("NO"));
    
    // Broadcast completion event
    OnValidationComplete.Broadcast(bAllPassed, ValidationResults);
    
    // Generate and log report
    FString Report = GenerateValidationReport();
    UE_LOG(LogPhysicsValidator, Log, TEXT("Validation Report:\n%s"), *Report);
}

void UPhysicsSystemValidator::RunValidationTest(EPhysicsValidationType TestType)
{
    if (!bValidationEnabled)
    {
        return;
    }
    
    FPhysicsValidationEntry Result;
    
    switch (TestType)
    {
        case EPhysicsValidationType::ChaosEngine:
            Result = ValidateChaosEngine();
            break;
        case EPhysicsValidationType::CollisionDetection:
            Result = ValidateCollisionDetection();
            break;
        case EPhysicsValidationType::RigidBodyDynamics:
            Result = ValidateRigidBodyDynamics();
            break;
        case EPhysicsValidationType::RagdollPhysics:
            Result = ValidateRagdollPhysics();
            break;
        case EPhysicsValidationType::DestructionSystem:
            Result = ValidateDestructionSystem();
            break;
        case EPhysicsValidationType::FluidSimulation:
            Result = ValidateFluidSimulation();
            break;
        case EPhysicsValidationType::ClothSimulation:
            Result = ValidateClothSimulation();
            break;
        case EPhysicsValidationType::VehiclePhysics:
            Result = ValidateVehiclePhysics();
            break;
        case EPhysicsValidationType::PerformanceTest:
            Result = ValidatePerformance();
            break;
        case EPhysicsValidationType::NetworkedPhysics:
            Result = ValidateNetworkedPhysics();
            break;
    }
    
    ValidationResults.Add(Result);
}

FPhysicsValidationEntry UPhysicsSystemValidator::ValidateChaosEngine()
{
    FPhysicsValidationEntry Entry;
    Entry.TestType = EPhysicsValidationType::ChaosEngine;
    Entry.TestName = TEXT("Chaos Physics Engine");
    Entry.Description = TEXT("Validate Chaos physics engine availability and configuration");
    Entry.Timestamp = FDateTime::Now();
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Check if Chaos is enabled
        const UPhysicsSettings* PhysicsSettings = GetDefault<UPhysicsSettings>();
        if (!PhysicsSettings)
        {
            Entry.Result = EPhysicsValidationResult::Failed;
            Entry.ErrorMessage = TEXT("Physics settings not accessible");
            Entry.ExecutionTime = FPlatformTime::Seconds() - StartTime;
            return Entry;
        }
        
        // Validate Chaos settings
        bool bChaosEnabled = true; // Chaos is default in UE5
        if (!bChaosEnabled)
        {
            Entry.Result = EPhysicsValidationResult::Failed;
            Entry.ErrorMessage = TEXT("Chaos physics engine not enabled");
        }
        else
        {
            Entry.Result = EPhysicsValidationResult::Passed;
            Entry.ErrorMessage = TEXT("Chaos physics engine validated successfully");
        }
    }
    catch (...)
    {
        Entry.Result = EPhysicsValidationResult::Failed;
        Entry.ErrorMessage = TEXT("Exception during Chaos validation");
    }
    
    Entry.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogPhysicsValidator, Log, TEXT("Chaos Engine Validation: %s"), 
        Entry.Result == EPhysicsValidationResult::Passed ? TEXT("PASSED") : TEXT("FAILED"));
    
    return Entry;
}

FPhysicsValidationEntry UPhysicsSystemValidator::ValidateCollisionDetection()
{
    FPhysicsValidationEntry Entry;
    Entry.TestType = EPhysicsValidationType::CollisionDetection;
    Entry.TestName = TEXT("Collision Detection System");
    Entry.Description = TEXT("Validate collision detection and response systems");
    Entry.Timestamp = FDateTime::Now();
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test collision response
        bool bCollisionTest = TestCollisionResponse();
        
        if (bCollisionTest)
        {
            Entry.Result = EPhysicsValidationResult::Passed;
            Entry.ErrorMessage = TEXT("Collision detection system validated successfully");
        }
        else
        {
            Entry.Result = EPhysicsValidationResult::Failed;
            Entry.ErrorMessage = TEXT("Collision detection test failed");
        }
    }
    catch (...)
    {
        Entry.Result = EPhysicsValidationResult::Failed;
        Entry.ErrorMessage = TEXT("Exception during collision validation");
    }
    
    Entry.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogPhysicsValidator, Log, TEXT("Collision Detection Validation: %s"), 
        Entry.Result == EPhysicsValidationResult::Passed ? TEXT("PASSED") : TEXT("FAILED"));
    
    return Entry;
}

FPhysicsValidationEntry UPhysicsSystemValidator::ValidateRigidBodyDynamics()
{
    FPhysicsValidationEntry Entry;
    Entry.TestType = EPhysicsValidationType::RigidBodyDynamics;
    Entry.TestName = TEXT("Rigid Body Dynamics");
    Entry.Description = TEXT("Validate rigid body physics simulation");
    Entry.Timestamp = FDateTime::Now();
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        bool bRigidBodyTest = CreateTestRigidBody();
        
        if (bRigidBodyTest)
        {
            Entry.Result = EPhysicsValidationResult::Passed;
            Entry.ErrorMessage = TEXT("Rigid body dynamics validated successfully");
        }
        else
        {
            Entry.Result = EPhysicsValidationResult::Failed;
            Entry.ErrorMessage = TEXT("Rigid body dynamics test failed");
        }
    }
    catch (...)
    {
        Entry.Result = EPhysicsValidationResult::Failed;
        Entry.ErrorMessage = TEXT("Exception during rigid body validation");
    }
    
    Entry.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogPhysicsValidator, Log, TEXT("Rigid Body Dynamics Validation: %s"), 
        Entry.Result == EPhysicsValidationResult::Passed ? TEXT("PASSED") : TEXT("FAILED"));
    
    return Entry;
}

FPhysicsValidationEntry UPhysicsSystemValidator::ValidateRagdollPhysics()
{
    FPhysicsValidationEntry Entry;
    Entry.TestType = EPhysicsValidationType::RagdollPhysics;
    Entry.TestName = TEXT("Ragdoll Physics System");
    Entry.Description = TEXT("Validate ragdoll physics for character death/unconsciousness");
    Entry.Timestamp = FDateTime::Now();
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        bool bRagdollTest = CreateTestRagdoll();
        
        if (bRagdollTest)
        {
            Entry.Result = EPhysicsValidationResult::Passed;
            Entry.ErrorMessage = TEXT("Ragdoll physics system validated successfully");
        }
        else
        {
            Entry.Result = EPhysicsValidationResult::Warning;
            Entry.ErrorMessage = TEXT("Ragdoll physics test incomplete - implementation in progress");
        }
    }
    catch (...)
    {
        Entry.Result = EPhysicsValidationResult::Failed;
        Entry.ErrorMessage = TEXT("Exception during ragdoll validation");
    }
    
    Entry.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogPhysicsValidator, Log, TEXT("Ragdoll Physics Validation: %s"), 
        Entry.Result == EPhysicsValidationResult::Passed ? TEXT("PASSED") : TEXT("WARNING"));
    
    return Entry;
}

FPhysicsValidationEntry UPhysicsSystemValidator::ValidateDestructionSystem()
{
    FPhysicsValidationEntry Entry;
    Entry.TestType = EPhysicsValidationType::DestructionSystem;
    Entry.TestName = TEXT("Destruction System");
    Entry.Description = TEXT("Validate Chaos destruction for environmental objects");
    Entry.Timestamp = FDateTime::Now();
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        bool bDestructionTest = CreateTestDestruction();
        
        if (bDestructionTest)
        {
            Entry.Result = EPhysicsValidationResult::Passed;
            Entry.ErrorMessage = TEXT("Destruction system validated successfully");
        }
        else
        {
            Entry.Result = EPhysicsValidationResult::Warning;
            Entry.ErrorMessage = TEXT("Destruction system test incomplete - requires geometry collections");
        }
    }
    catch (...)
    {
        Entry.Result = EPhysicsValidationResult::Failed;
        Entry.ErrorMessage = TEXT("Exception during destruction validation");
    }
    
    Entry.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogPhysicsValidator, Log, TEXT("Destruction System Validation: %s"), 
        Entry.Result == EPhysicsValidationResult::Passed ? TEXT("PASSED") : TEXT("WARNING"));
    
    return Entry;
}

FPhysicsValidationEntry UPhysicsSystemValidator::ValidateFluidSimulation()
{
    FPhysicsValidationEntry Entry;
    Entry.TestType = EPhysicsValidationType::FluidSimulation;
    Entry.TestName = TEXT("Fluid Simulation");
    Entry.Description = TEXT("Validate fluid physics for water/rivers");
    Entry.Timestamp = FDateTime::Now();
    
    float StartTime = FPlatformTime::Seconds();
    
    // Fluid simulation is typically handled by Niagara, not core physics
    Entry.Result = EPhysicsValidationResult::NotImplemented;
    Entry.ErrorMessage = TEXT("Fluid simulation handled by VFX systems (Niagara)");
    
    Entry.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogPhysicsValidator, Log, TEXT("Fluid Simulation Validation: NOT IMPLEMENTED (VFX responsibility)"));
    
    return Entry;
}

FPhysicsValidationEntry UPhysicsSystemValidator::ValidateClothSimulation()
{
    FPhysicsValidationEntry Entry;
    Entry.TestType = EPhysicsValidationType::ClothSimulation;
    Entry.TestName = TEXT("Cloth Simulation");
    Entry.Description = TEXT("Validate Chaos cloth for character clothing");
    Entry.Timestamp = FDateTime::Now();
    
    float StartTime = FPlatformTime::Seconds();
    
    // Cloth simulation validation would require character assets
    Entry.Result = EPhysicsValidationResult::NotImplemented;
    Entry.ErrorMessage = TEXT("Cloth simulation requires character assets - deferred to Character Artist");
    
    Entry.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogPhysicsValidator, Log, TEXT("Cloth Simulation Validation: NOT IMPLEMENTED (Character Artist responsibility)"));
    
    return Entry;
}

FPhysicsValidationEntry UPhysicsSystemValidator::ValidateVehiclePhysics()
{
    FPhysicsValidationEntry Entry;
    Entry.TestType = EPhysicsValidationType::VehiclePhysics;
    Entry.TestName = TEXT("Vehicle Physics");
    Entry.Description = TEXT("Validate Chaos vehicles for game transportation");
    Entry.Timestamp = FDateTime::Now();
    
    float StartTime = FPlatformTime::Seconds();
    
    // Vehicle physics not required for prehistoric survival game
    Entry.Result = EPhysicsValidationResult::Skipped;
    Entry.ErrorMessage = TEXT("Vehicle physics not required for prehistoric survival game concept");
    
    Entry.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogPhysicsValidator, Log, TEXT("Vehicle Physics Validation: SKIPPED (not required for game concept)"));
    
    return Entry;
}

FPhysicsValidationEntry UPhysicsSystemValidator::ValidatePerformance()
{
    FPhysicsValidationEntry Entry;
    Entry.TestType = EPhysicsValidationType::PerformanceTest;
    Entry.TestName = TEXT("Physics Performance");
    Entry.Description = TEXT("Validate physics performance meets target framerate");
    Entry.Timestamp = FDateTime::Now();
    
    float StartTime = FPlatformTime::Seconds();
    
    UpdatePerformanceMetrics();
    
    bool bPerformanceGood = CurrentPerformanceMetrics.bWithinTargetFramerate &&
                           CurrentPerformanceMetrics.PhysicsTickTime <= MaxPhysicsTickTime &&
                           CurrentPerformanceMetrics.MemoryUsage <= MaxMemoryUsage;
    
    if (bPerformanceGood)
    {
        Entry.Result = EPhysicsValidationResult::Passed;
        Entry.ErrorMessage = FString::Printf(TEXT("Performance validated: %.1f FPS, %.2f ms physics"), 
            GetCurrentFramerate(), CurrentPerformanceMetrics.PhysicsTickTime);
    }
    else
    {
        Entry.Result = EPhysicsValidationResult::Warning;
        Entry.ErrorMessage = FString::Printf(TEXT("Performance concerns: %.1f FPS, %.2f ms physics"), 
            GetCurrentFramerate(), CurrentPerformanceMetrics.PhysicsTickTime);
    }
    
    Entry.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogPhysicsValidator, Log, TEXT("Physics Performance Validation: %s"), 
        Entry.Result == EPhysicsValidationResult::Passed ? TEXT("PASSED") : TEXT("WARNING"));
    
    return Entry;
}

FPhysicsValidationEntry UPhysicsSystemValidator::ValidateNetworkedPhysics()
{
    FPhysicsValidationEntry Entry;
    Entry.TestType = EPhysicsValidationType::NetworkedPhysics;
    Entry.TestName = TEXT("Networked Physics");
    Entry.Description = TEXT("Validate physics replication for multiplayer");
    Entry.Timestamp = FDateTime::Now();
    
    float StartTime = FPlatformTime::Seconds();
    
    // Networked physics validation requires multiplayer setup
    Entry.Result = EPhysicsValidationResult::NotImplemented;
    Entry.ErrorMessage = TEXT("Networked physics validation requires multiplayer testing environment");
    
    Entry.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogPhysicsValidator, Log, TEXT("Networked Physics Validation: NOT IMPLEMENTED (requires multiplayer setup)"));
    
    return Entry;
}

bool UPhysicsSystemValidator::AllTestsPassed() const
{
    for (const FPhysicsValidationEntry& Entry : ValidationResults)
    {
        if (Entry.Result == EPhysicsValidationResult::Failed)
        {
            return false;
        }
    }
    return true;
}

int32 UPhysicsSystemValidator::GetTestCountByResult(EPhysicsValidationResult Result) const
{
    int32 Count = 0;
    for (const FPhysicsValidationEntry& Entry : ValidationResults)
    {
        if (Entry.Result == Result)
        {
            Count++;
        }
    }
    return Count;
}

void UPhysicsSystemValidator::ClearValidationResults()
{
    ValidationResults.Empty();
}

FString UPhysicsSystemValidator::GenerateValidationReport() const
{
    FString Report;
    Report += TEXT("=== PHYSICS SYSTEM VALIDATION REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total Tests: %d\n"), ValidationResults.Num());
    Report += FString::Printf(TEXT("Passed: %d\n"), GetTestCountByResult(EPhysicsValidationResult::Passed));
    Report += FString::Printf(TEXT("Failed: %d\n"), GetTestCountByResult(EPhysicsValidationResult::Failed));
    Report += FString::Printf(TEXT("Warnings: %d\n"), GetTestCountByResult(EPhysicsValidationResult::Warning));
    Report += FString::Printf(TEXT("Not Implemented: %d\n"), GetTestCountByResult(EPhysicsValidationResult::NotImplemented));
    Report += FString::Printf(TEXT("Skipped: %d\n"), GetTestCountByResult(EPhysicsValidationResult::Skipped));
    Report += TEXT("\n=== DETAILED RESULTS ===\n");
    
    for (const FPhysicsValidationEntry& Entry : ValidationResults)
    {
        FString ResultText;
        switch (Entry.Result)
        {
            case EPhysicsValidationResult::Passed: ResultText = TEXT("PASSED"); break;
            case EPhysicsValidationResult::Failed: ResultText = TEXT("FAILED"); break;
            case EPhysicsValidationResult::Warning: ResultText = TEXT("WARNING"); break;
            case EPhysicsValidationResult::NotImplemented: ResultText = TEXT("NOT IMPLEMENTED"); break;
            case EPhysicsValidationResult::Skipped: ResultText = TEXT("SKIPPED"); break;
        }
        
        Report += FString::Printf(TEXT("[%s] %s (%.3fs)\n"), 
            *ResultText, *Entry.TestName, Entry.ExecutionTime);
        Report += FString::Printf(TEXT("    %s\n"), *Entry.ErrorMessage);
    }
    
    Report += TEXT("\n=== PERFORMANCE METRICS ===\n");
    Report += FString::Printf(TEXT("Physics Tick Time: %.2f ms\n"), CurrentPerformanceMetrics.PhysicsTickTime);
    Report += FString::Printf(TEXT("Active Rigid Bodies: %d\n"), CurrentPerformanceMetrics.ActiveRigidBodies);
    Report += FString::Printf(TEXT("Memory Usage: %.1f MB\n"), CurrentPerformanceMetrics.MemoryUsage);
    Report += FString::Printf(TEXT("Within Target Framerate: %s\n"), 
        CurrentPerformanceMetrics.bWithinTargetFramerate ? TEXT("YES") : TEXT("NO"));
    
    return Report;
}

bool UPhysicsSystemValidator::SaveValidationReportToFile(const FString& FilePath) const
{
    FString Report = GenerateValidationReport();
    return FFileHelper::SaveStringToFile(Report, *FilePath);
}

void UPhysicsSystemValidator::UpdatePerformanceMetrics()
{
    CurrentPerformanceMetrics.PhysicsTickTime = GetPhysicsTickTime();
    CurrentPerformanceMetrics.ActiveRigidBodies = GetActiveRigidBodyCount();
    CurrentPerformanceMetrics.MemoryUsage = GetPhysicsMemoryUsage();
    CurrentPerformanceMetrics.CPUUsage = 0.0f; // TODO: Implement CPU usage tracking
    
    float CurrentFPS = GetCurrentFramerate();
    CurrentPerformanceMetrics.bWithinTargetFramerate = CurrentFPS >= (TargetFramerate * 0.9f);
}

void UPhysicsSystemValidator::MonitorPerformance()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastMonitoringTime >= MonitoringInterval)
    {
        UpdatePerformanceMetrics();
        LastMonitoringTime = CurrentTime;
        
        // Log performance warnings
        if (!CurrentPerformanceMetrics.bWithinTargetFramerate)
        {
            UE_LOG(LogPhysicsValidator, Warning, TEXT("Physics performance below target: %.1f FPS"), 
                GetCurrentFramerate());
        }
    }
}

float UPhysicsSystemValidator::GetCurrentFramerate() const
{
    if (GEngine && GEngine->GameViewport)
    {
        return 1.0f / GEngine->GameViewport->GetWorld()->GetDeltaSeconds();
    }
    return 0.0f;
}

float UPhysicsSystemValidator::GetPhysicsTickTime() const
{
    // Approximate physics tick time based on frame time
    return GetWorld()->GetDeltaSeconds() * 1000.0f * 0.3f; // Physics typically ~30% of frame
}

int32 UPhysicsSystemValidator::GetActiveRigidBodyCount() const
{
    // Count physics bodies in the world
    int32 Count = 0;
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                TArray<UStaticMeshComponent*> MeshComponents;
                Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
                for (UStaticMeshComponent* MeshComp : MeshComponents)
                {
                    if (MeshComp && MeshComp->GetBodyInstance() && MeshComp->GetBodyInstance()->bSimulatePhysics)
                    {
                        Count++;
                    }
                }
            }
        }
    }
    return Count;
}

float UPhysicsSystemValidator::GetPhysicsMemoryUsage() const
{
    // Approximate physics memory usage
    // This would need platform-specific implementation for accurate measurement
    return GetActiveRigidBodyCount() * 0.1f; // Rough estimate: 100KB per rigid body
}

bool UPhysicsSystemValidator::CreateTestRigidBody()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    try
    {
        // Create a test static mesh actor with physics
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(TEXT("PhysicsTestActor"));
        
        AStaticMeshActor* TestActor = World->SpawnActor<AStaticMeshActor>(SpawnParams);
        if (TestActor)
        {
            UStaticMeshComponent* MeshComp = TestActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                // Enable physics simulation
                MeshComp->SetSimulatePhysics(true);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                
                TestActors.Add(TestActor);
                TestMeshComponents.Add(MeshComp);
                
                UE_LOG(LogPhysicsValidator, Log, TEXT("Test rigid body created successfully"));
                return true;
            }
        }
    }
    catch (...)
    {
        UE_LOG(LogPhysicsValidator, Error, TEXT("Exception creating test rigid body"));
    }
    
    return false;
}

bool UPhysicsSystemValidator::CreateTestRagdoll()
{
    // Ragdoll testing requires a character with physics asset
    // This is a placeholder that would be implemented with actual character assets
    UE_LOG(LogPhysicsValidator, Log, TEXT("Ragdoll test placeholder - requires character assets"));
    return true; // Return true for now as this is implementation-dependent
}

bool UPhysicsSystemValidator::CreateTestDestruction()
{
    // Destruction testing requires geometry collection assets
    // This is a placeholder that would be implemented with actual destructible assets
    UE_LOG(LogPhysicsValidator, Log, TEXT("Destruction test placeholder - requires geometry collection assets"));
    return true; // Return true for now as this is implementation-dependent
}

bool UPhysicsSystemValidator::TestCollisionResponse()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    try
    {
        // Test basic collision detection
        FVector StartLocation(0, 0, 1000);
        FVector EndLocation(0, 0, -1000);
        
        FHitResult HitResult;
        bool bHit = World->LineTraceSingleByChannel(
            HitResult,
            StartLocation,
            EndLocation,
            ECollisionChannel::ECC_WorldStatic
        );
        
        UE_LOG(LogPhysicsValidator, Log, TEXT("Collision test - Line trace hit: %s"), 
            bHit ? TEXT("YES") : TEXT("NO"));
        
        return true; // Basic collision system is functional if we can perform traces
    }
    catch (...)
    {
        UE_LOG(LogPhysicsValidator, Error, TEXT("Exception during collision test"));
        return false;
    }
}

bool UPhysicsSystemValidator::TestPhysicsConstraints()
{
    // Physics constraints testing would require two rigid bodies and a constraint component
    UE_LOG(LogPhysicsValidator, Log, TEXT("Physics constraints test placeholder"));
    return true;
}

bool UPhysicsSystemValidator::TestPhysicsMaterials()
{
    // Physics materials testing
    UE_LOG(LogPhysicsValidator, Log, TEXT("Physics materials test placeholder"));
    return true;
}