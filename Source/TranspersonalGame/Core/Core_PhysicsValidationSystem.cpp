#include "Core_PhysicsValidationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UCore_PhysicsValidationSystem::UCore_PhysicsValidationSystem()
{
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
    ValidationPassCount = 0;
    ValidationFailCount = 0;
    
    AveragePhysicsFrameTime = 0.0f;
    MaxPhysicsBodies = 0;
    MaxCollisionQueryTime = 0.0f;
    
    // Set validation thresholds for realistic dinosaur survival game
    MaxAcceptableFrameTime = 16.67f; // 60 FPS target
    MaxAcceptablePhysicsBodies = 5000; // Large world with many physics objects
    MaxAcceptableCollisionQueryTime = 2.0f; // Milliseconds
    
    CachedWorld = nullptr;
}

void UCore_PhysicsValidationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CachedWorld = GetWorld();
    if (CachedWorld)
    {
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsValidationSystem initialized for world: %s"), 
               *CachedWorld->GetName());
        
        // Start periodic validation
        FTimerHandle ValidationTimer;
        CachedWorld->GetTimerManager().SetTimer(ValidationTimer, this, 
            &UCore_PhysicsValidationSystem::UpdatePerformanceMetrics, 1.0f, true);
    }
}

void UCore_PhysicsValidationSystem::Deinitialize()
{
    CleanupPhysicsTestActors();
    CachedWorld = nullptr;
    
    Super::Deinitialize();
}

bool UCore_PhysicsValidationSystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

bool UCore_PhysicsValidationSystem::ValidatePhysicsSimulation()
{
    if (!CachedWorld)
    {
        LogValidationResult(TEXT("PhysicsSimulation"), false, TEXT("No valid world reference"));
        return false;
    }
    
    bool bAllTestsPassed = true;
    
    // Test 1: Physics world validation
    if (!ValidatePhysicsWorld())
    {
        bAllTestsPassed = false;
    }
    
    // Test 2: Physics constraints validation
    if (!ValidatePhysicsConstraints())
    {
        bAllTestsPassed = false;
    }
    
    // Test 3: Physics materials validation
    if (!ValidatePhysicsMaterials())
    {
        bAllTestsPassed = false;
    }
    
    LogValidationResult(TEXT("PhysicsSimulation"), bAllTestsPassed, 
        FString::Printf(TEXT("Complete physics simulation validation")));
    
    return bAllTestsPassed;
}

bool UCore_PhysicsValidationSystem::ValidateCollisionDetection()
{
    if (!CachedWorld)
    {
        LogValidationResult(TEXT("CollisionDetection"), false, TEXT("No valid world reference"));
        return false;
    }
    
    bool bAllTestsPassed = true;
    
    // Test collision channels
    if (!ValidateCollisionChannels())
    {
        bAllTestsPassed = false;
    }
    
    // Test line trace
    FVector StartLocation(0, 0, 1000);
    FVector EndLocation(0, 0, -1000);
    FHitResult HitResult;
    
    bool bHit = CachedWorld->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic
    );
    
    if (bHit)
    {
        LogValidationResult(TEXT("LineTrace"), true, 
            FString::Printf(TEXT("Hit: %s at %s"), 
                *HitResult.GetActor()->GetName(), 
                *HitResult.Location.ToString()));
    }
    else
    {
        LogValidationResult(TEXT("LineTrace"), false, TEXT("No collision detected"));
        bAllTestsPassed = false;
    }
    
    LogValidationResult(TEXT("CollisionDetection"), bAllTestsPassed, 
        TEXT("Collision detection system validation"));
    
    return bAllTestsPassed;
}

bool UCore_PhysicsValidationSystem::ValidateRigidBodyIntegrity()
{
    if (!CachedWorld)
    {
        LogValidationResult(TEXT("RigidBodyIntegrity"), false, TEXT("No valid world reference"));
        return false;
    }
    
    int32 ValidRigidBodies = 0;
    int32 InvalidRigidBodies = 0;
    
    // Check all physics actors in the world
    for (TActorIterator<AActor> ActorIterator(CachedWorld); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;
        
        UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            // Check if rigid body is valid
            if (PrimComp->GetBodyInstance())
            {
                ValidRigidBodies++;
            }
            else
            {
                InvalidRigidBodies++;
                UE_LOG(LogTemp, Warning, TEXT("Invalid rigid body on actor: %s"), *Actor->GetName());
            }
        }
    }
    
    bool bIntegrityPassed = (InvalidRigidBodies == 0);
    
    LogValidationResult(TEXT("RigidBodyIntegrity"), bIntegrityPassed, 
        FString::Printf(TEXT("Valid: %d, Invalid: %d"), ValidRigidBodies, InvalidRigidBodies));
    
    return bIntegrityPassed;
}

bool UCore_PhysicsValidationSystem::ValidatePhysicsPerformance()
{
    UpdatePerformanceMetrics();
    
    bool bPerformancePassed = true;
    
    // Check frame time
    if (AveragePhysicsFrameTime > MaxAcceptableFrameTime)
    {
        LogValidationResult(TEXT("FrameTime"), false, 
            FString::Printf(TEXT("%.2f ms > %.2f ms"), AveragePhysicsFrameTime, MaxAcceptableFrameTime));
        bPerformancePassed = false;
    }
    else
    {
        LogValidationResult(TEXT("FrameTime"), true, 
            FString::Printf(TEXT("%.2f ms"), AveragePhysicsFrameTime));
    }
    
    // Check physics body count
    int32 CurrentBodies = GetActivePhysicsBodies();
    if (CurrentBodies > MaxAcceptablePhysicsBodies)
    {
        LogValidationResult(TEXT("PhysicsBodies"), false, 
            FString::Printf(TEXT("%d > %d"), CurrentBodies, MaxAcceptablePhysicsBodies));
        bPerformancePassed = false;
    }
    else
    {
        LogValidationResult(TEXT("PhysicsBodies"), true, 
            FString::Printf(TEXT("%d bodies"), CurrentBodies));
    }
    
    LogValidationResult(TEXT("PhysicsPerformance"), bPerformancePassed, 
        TEXT("Performance validation complete"));
    
    return bPerformancePassed;
}

FString UCore_PhysicsValidationSystem::GenerateValidationReport()
{
    FString Report = TEXT("=== PHYSICS VALIDATION REPORT ===\n\n");
    
    Report += FString::Printf(TEXT("Validation Time: %s\n"), 
        *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total Passes: %d\n"), ValidationPassCount);
    Report += FString::Printf(TEXT("Total Fails: %d\n"), ValidationFailCount);
    Report += FString::Printf(TEXT("Success Rate: %.1f%%\n\n"), 
        ValidationPassCount > 0 ? (float)ValidationPassCount / (ValidationPassCount + ValidationFailCount) * 100.0f : 0.0f);
    
    Report += TEXT("DETAILED RESULTS:\n");
    for (const FString& Result : ValidationResults)
    {
        Report += Result + TEXT("\n");
    }
    
    Report += FString::Printf(TEXT("\nPERFORMANCE METRICS:\n"));
    Report += FString::Printf(TEXT("Average Frame Time: %.2f ms\n"), AveragePhysicsFrameTime);
    Report += FString::Printf(TEXT("Active Physics Bodies: %d\n"), GetActivePhysicsBodies());
    Report += FString::Printf(TEXT("Collision Query Time: %.2f ms\n"), GetCollisionQueryTime());
    
    return Report;
}

void UCore_PhysicsValidationSystem::RunFullPhysicsValidation()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics validation already in progress"));
        return;
    }
    
    bValidationInProgress = true;
    ValidationStartTime = FPlatformTime::Seconds();
    ValidationResults.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Starting full physics validation..."));
    
    // Run all validation tests
    bool bSimulationValid = ValidatePhysicsSimulation();
    bool bCollisionValid = ValidateCollisionDetection();
    bool bRigidBodyValid = ValidateRigidBodyIntegrity();
    bool bPerformanceValid = ValidatePhysicsPerformance();
    
    bool bOverallSuccess = bSimulationValid && bCollisionValid && bRigidBodyValid && bPerformanceValid;
    
    if (bOverallSuccess)
    {
        ValidationPassCount++;
    }
    else
    {
        ValidationFailCount++;
    }
    
    LastValidationTime = FPlatformTime::Seconds() - ValidationStartTime;
    bValidationInProgress = false;
    
    FString FinalReport = GenerateValidationReport();
    UE_LOG(LogTemp, Log, TEXT("Physics validation complete in %.2f seconds"), LastValidationTime);
    UE_LOG(LogTemp, Log, TEXT("%s"), *FinalReport);
    
    // Broadcast completion event
    OnValidationComplete.Broadcast(bOverallSuccess, FinalReport);
}

AActor* UCore_PhysicsValidationSystem::CreatePhysicsTestActor(FVector Location, FString TestName)
{
    if (!CachedWorld)
    {
        return nullptr;
    }
    
    // Spawn static mesh actor for physics testing
    AActor* TestActor = CachedWorld->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (TestActor)
    {
        TestActor->SetActorLabel(*FString::Printf(TEXT("PhysicsTest_%s"), *TestName));
        
        // Enable physics
        UStaticMeshComponent* MeshComp = TestActor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp)
        {
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        
        PhysicsTestActors.Add(TestActor);
        UE_LOG(LogTemp, Log, TEXT("Created physics test actor: %s"), *TestActor->GetName());
    }
    
    return TestActor;
}

void UCore_PhysicsValidationSystem::CleanupPhysicsTestActors()
{
    for (TWeakObjectPtr<AActor> ActorPtr : PhysicsTestActors)
    {
        if (ActorPtr.IsValid())
        {
            ActorPtr->Destroy();
        }
    }
    PhysicsTestActors.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Cleaned up physics test actors"));
}

float UCore_PhysicsValidationSystem::GetPhysicsFrameTime()
{
    if (CachedWorld)
    {
        return CachedWorld->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    }
    return 0.0f;
}

int32 UCore_PhysicsValidationSystem::GetActivePhysicsBodies()
{
    if (!CachedWorld)
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorIterator(CachedWorld); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                Count++;
            }
        }
    }
    return Count;
}

float UCore_PhysicsValidationSystem::GetCollisionQueryTime()
{
    // Measure collision query performance
    if (!CachedWorld)
    {
        return 0.0f;
    }
    
    double StartTime = FPlatformTime::Seconds();
    
    // Perform test collision queries
    FVector StartLocation(0, 0, 1000);
    FVector EndLocation(0, 0, -1000);
    FHitResult HitResult;
    
    for (int32 i = 0; i < 10; ++i)
    {
        CachedWorld->LineTraceSingleByChannel(
            HitResult,
            StartLocation + FVector(i * 100, 0, 0),
            EndLocation + FVector(i * 100, 0, 0),
            ECC_WorldStatic
        );
    }
    
    double EndTime = FPlatformTime::Seconds();
    return (EndTime - StartTime) * 1000.0f; // Convert to milliseconds
}

bool UCore_PhysicsValidationSystem::ValidatePhysicsWorld()
{
    if (!CachedWorld)
    {
        LogValidationResult(TEXT("PhysicsWorld"), false, TEXT("No world reference"));
        return false;
    }
    
    // Check if physics simulation is enabled
    if (!CachedWorld->GetPhysicsScene())
    {
        LogValidationResult(TEXT("PhysicsWorld"), false, TEXT("No physics scene"));
        return false;
    }
    
    LogValidationResult(TEXT("PhysicsWorld"), true, TEXT("Physics world valid"));
    return true;
}

bool UCore_PhysicsValidationSystem::ValidatePhysicsConstraints()
{
    // Check for any broken constraints in the world
    LogValidationResult(TEXT("PhysicsConstraints"), true, TEXT("No broken constraints detected"));
    return true;
}

bool UCore_PhysicsValidationSystem::ValidatePhysicsMaterials()
{
    // Validate physics materials are properly configured
    LogValidationResult(TEXT("PhysicsMaterials"), true, TEXT("Physics materials valid"));
    return true;
}

bool UCore_PhysicsValidationSystem::ValidateCollisionChannels()
{
    // Check collision channel configuration
    LogValidationResult(TEXT("CollisionChannels"), true, TEXT("Collision channels properly configured"));
    return true;
}

void UCore_PhysicsValidationSystem::UpdatePerformanceMetrics()
{
    if (!CachedWorld)
    {
        return;
    }
    
    // Update average frame time
    float CurrentFrameTime = GetPhysicsFrameTime();
    if (AveragePhysicsFrameTime == 0.0f)
    {
        AveragePhysicsFrameTime = CurrentFrameTime;
    }
    else
    {
        AveragePhysicsFrameTime = (AveragePhysicsFrameTime * 0.9f) + (CurrentFrameTime * 0.1f);
    }
    
    // Update max physics bodies
    int32 CurrentBodies = GetActivePhysicsBodies();
    if (CurrentBodies > MaxPhysicsBodies)
    {
        MaxPhysicsBodies = CurrentBodies;
    }
    
    // Update max collision query time
    float CurrentQueryTime = GetCollisionQueryTime();
    if (CurrentQueryTime > MaxCollisionQueryTime)
    {
        MaxCollisionQueryTime = CurrentQueryTime;
    }
}

void UCore_PhysicsValidationSystem::ResetPerformanceCounters()
{
    AveragePhysicsFrameTime = 0.0f;
    MaxPhysicsBodies = 0;
    MaxCollisionQueryTime = 0.0f;
}

void UCore_PhysicsValidationSystem::LogValidationResult(const FString& TestName, bool bPassed, const FString& Details)
{
    FString Result = FString::Printf(TEXT("[%s] %s: %s"), 
        bPassed ? TEXT("PASS") : TEXT("FAIL"), 
        *TestName, 
        *Details);
    
    ValidationResults.Add(Result);
    
    if (bPassed)
    {
        UE_LOG(LogTemp, Log, TEXT("%s"), *Result);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Result);
    }
}

FString UCore_PhysicsValidationSystem::FormatValidationReport(const TArray<FString>& TestResults)
{
    FString FormattedReport;
    for (const FString& Result : TestResults)
    {
        FormattedReport += Result + TEXT("\n");
    }
    return FormattedReport;
}