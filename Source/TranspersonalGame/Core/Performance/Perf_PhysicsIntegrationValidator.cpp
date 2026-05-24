#include "Perf_PhysicsIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

UPerf_PhysicsIntegrationValidator::UPerf_PhysicsIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    ValidationInterval = 5.0f;
    bAutoValidateOnBeginPlay = true;
    bLogValidationResults = true;
    MaxAcceptablePhysicsTickTime = 16.67f; // 60 FPS target
    MaxPhysicsActorsWarningThreshold = 500;
    MaxPhysicsActorsCriticalThreshold = 1000;
    
    LastValidationTime = 0.0f;
    LastValidationResult = EPerf_PhysicsValidationResult::Passed;
    LastValidationMessage = TEXT("No validation performed yet");
}

void UPerf_PhysicsIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoValidateOnBeginPlay)
    {
        // Delay initial validation to allow world to fully initialize
        FTimerHandle ValidationTimer;
        GetWorld()->GetTimerManager().SetTimer(ValidationTimer, [this]()
        {
            ValidatePhysicsSystem();
        }, 2.0f, false);
    }
}

void UPerf_PhysicsIntegrationValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastValidationTime >= ValidationInterval)
    {
        ValidatePhysicsSystem();
        LastValidationTime = CurrentTime;
    }
}

EPerf_PhysicsValidationResult UPerf_PhysicsIntegrationValidator::ValidatePhysicsSystem()
{
    FPerf_PhysicsValidationMetrics Metrics;
    CollectPhysicsMetrics(Metrics);
    
    EPerf_PhysicsValidationResult Result = EPerf_PhysicsValidationResult::Passed;
    ValidatePhysicsPerformance(Metrics, Result);
    
    LastValidationMetrics = Metrics;
    LastValidationResult = Result;
    
    if (bLogValidationResults)
    {
        LogValidationResult(Metrics, Result);
    }
    
    return Result;
}

FPerf_PhysicsValidationMetrics UPerf_PhysicsIntegrationValidator::GetPhysicsMetrics()
{
    FPerf_PhysicsValidationMetrics Metrics;
    CollectPhysicsMetrics(Metrics);
    return Metrics;
}

bool UPerf_PhysicsIntegrationValidator::ValidatePhysicsActorIntegrity(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }
    
    // Check if actor has physics components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    bool bHasValidPhysics = false;
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component && Component->IsSimulatingPhysics())
        {
            // Validate physics body
            FBodyInstance* BodyInstance = Component->GetBodyInstance();
            if (BodyInstance && BodyInstance->IsValidBodyInstance())
            {
                bHasValidPhysics = true;
                
                // Check for common physics issues
                if (BodyInstance->GetBodyMass() <= 0.0f)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Physics Actor %s has invalid mass"), *Actor->GetName());
                    return false;
                }
                
                // Validate collision settings
                if (Component->GetCollisionEnabled() == ECollisionEnabled::NoCollision && 
                    Component->IsSimulatingPhysics())
                {
                    UE_LOG(LogTemp, Warning, TEXT("Physics Actor %s has physics simulation but no collision"), *Actor->GetName());
                    return false;
                }
            }
        }
    }
    
    return bHasValidPhysics;
}

void UPerf_PhysicsIntegrationValidator::RunPhysicsPerformanceTest(float TestDuration)
{
    UE_LOG(LogTemp, Log, TEXT("Starting Physics Performance Test for %.1f seconds"), TestDuration);
    
    PhysicsTickSamples.Empty();
    
    FTimerHandle TestTimer;
    GetWorld()->GetTimerManager().SetTimer(TestTimer, [this]()
    {
        // Sample physics tick time
        float PhysicsTickTime = GetWorld()->GetPhysicsScene()->GetLastDeltaTime() * 1000.0f; // Convert to ms
        PhysicsTickSamples.Add(PhysicsTickTime);
        
    }, 0.1f, true); // Sample every 100ms
    
    // Stop test after duration
    FTimerHandle StopTimer;
    GetWorld()->GetTimerManager().SetTimer(StopTimer, [this, TestTimer]()
    {
        GetWorld()->GetTimerManager().ClearTimer(TestTimer);
        
        // Analyze results
        if (PhysicsTickSamples.Num() > 0)
        {
            float TotalTime = 0.0f;
            float MaxTime = 0.0f;
            for (float Sample : PhysicsTickSamples)
            {
                TotalTime += Sample;
                MaxTime = FMath::Max(MaxTime, Sample);
            }
            
            float AverageTime = TotalTime / PhysicsTickSamples.Num();
            
            UE_LOG(LogTemp, Log, TEXT("Physics Performance Test Results:"));
            UE_LOG(LogTemp, Log, TEXT("- Average Tick Time: %.2f ms"), AverageTime);
            UE_LOG(LogTemp, Log, TEXT("- Max Tick Time: %.2f ms"), MaxTime);
            UE_LOG(LogTemp, Log, TEXT("- Samples Collected: %d"), PhysicsTickSamples.Num());
            
            LastValidationMetrics.AveragePhysicsTickTime = AverageTime;
            LastValidationMetrics.MaxPhysicsTickTime = MaxTime;
        }
        
    }, TestDuration, false);
}

void UPerf_PhysicsIntegrationValidator::ValidatePhysicsArchitectureIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Validating Physics Architecture Integration"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        LastValidationMessage = TEXT("No valid world found");
        LastValidationResult = EPerf_PhysicsValidationResult::Failed;
        return;
    }
    
    // Check for Core_PhysicsArchitectureAdapter
    bool bFoundAdapter = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Core_PhysicsArchitectureAdapter")))
        {
            bFoundAdapter = true;
            UE_LOG(LogTemp, Log, TEXT("Found Physics Architecture Adapter: %s"), *Actor->GetName());
            break;
        }
    }
    
    if (!bFoundAdapter)
    {
        LastValidationMessage = TEXT("Physics Architecture Adapter not found in world");
        LastValidationResult = EPerf_PhysicsValidationResult::Warning;
        UE_LOG(LogTemp, Warning, TEXT("Physics Architecture Adapter not found - integration may be incomplete"));
    }
    else
    {
        LastValidationMessage = TEXT("Physics Architecture Integration validated successfully");
        LastValidationResult = EPerf_PhysicsValidationResult::Passed;
    }
}

void UPerf_PhysicsIntegrationValidator::CollectPhysicsMetrics(FPerf_PhysicsValidationMetrics& OutMetrics)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    OutMetrics.TotalPhysicsActors = 0;
    OutMetrics.SimulatingActors = 0;
    OutMetrics.StaticActors = 0;
    OutMetrics.bPhysicsSystemHealthy = true;
    
    // Iterate through all actors and collect physics data
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        bool bHasPhysics = false;
        bool bIsSimulating = false;
        
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component && Component->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                bHasPhysics = true;
                if (Component->IsSimulatingPhysics())
                {
                    bIsSimulating = true;
                }
            }
        }
        
        if (bHasPhysics)
        {
            OutMetrics.TotalPhysicsActors++;
            if (bIsSimulating)
            {
                OutMetrics.SimulatingActors++;
            }
            else
            {
                OutMetrics.StaticActors++;
            }
        }
    }
    
    // Calculate average physics tick time from recent samples
    if (PhysicsTickSamples.Num() > 0)
    {
        float TotalTime = 0.0f;
        float MaxTime = 0.0f;
        int32 SampleCount = FMath::Min(PhysicsTickSamples.Num(), 10); // Use last 10 samples
        
        for (int32 i = PhysicsTickSamples.Num() - SampleCount; i < PhysicsTickSamples.Num(); i++)
        {
            TotalTime += PhysicsTickSamples[i];
            MaxTime = FMath::Max(MaxTime, PhysicsTickSamples[i]);
        }
        
        OutMetrics.AveragePhysicsTickTime = TotalTime / SampleCount;
        OutMetrics.MaxPhysicsTickTime = MaxTime;
    }
    
    // Estimate collision pairs (simplified calculation)
    OutMetrics.CollisionPairsCount = (OutMetrics.SimulatingActors * (OutMetrics.TotalPhysicsActors - 1)) / 2;
}

void UPerf_PhysicsIntegrationValidator::ValidatePhysicsPerformance(FPerf_PhysicsValidationMetrics& Metrics, EPerf_PhysicsValidationResult& Result)
{
    Result = EPerf_PhysicsValidationResult::Passed;
    FString ValidationMessage = TEXT("Physics validation passed");
    
    // Check physics actor count thresholds
    if (Metrics.TotalPhysicsActors >= MaxPhysicsActorsCriticalThreshold)
    {
        Result = EPerf_PhysicsValidationResult::Critical;
        ValidationMessage = FString::Printf(TEXT("Critical: Too many physics actors (%d >= %d)"), 
            Metrics.TotalPhysicsActors, MaxPhysicsActorsCriticalThreshold);
        Metrics.bPhysicsSystemHealthy = false;
    }
    else if (Metrics.TotalPhysicsActors >= MaxPhysicsActorsWarningThreshold)
    {
        Result = EPerf_PhysicsValidationResult::Warning;
        ValidationMessage = FString::Printf(TEXT("Warning: High physics actor count (%d >= %d)"), 
            Metrics.TotalPhysicsActors, MaxPhysicsActorsWarningThreshold);
    }
    
    // Check physics tick time
    if (Metrics.MaxPhysicsTickTime > MaxAcceptablePhysicsTickTime * 2.0f)
    {
        Result = EPerf_PhysicsValidationResult::Critical;
        ValidationMessage = FString::Printf(TEXT("Critical: Physics tick time too high (%.2f ms > %.2f ms)"), 
            Metrics.MaxPhysicsTickTime, MaxAcceptablePhysicsTickTime * 2.0f);
        Metrics.bPhysicsSystemHealthy = false;
    }
    else if (Metrics.MaxPhysicsTickTime > MaxAcceptablePhysicsTickTime)
    {
        if (Result == EPerf_PhysicsValidationResult::Passed)
        {
            Result = EPerf_PhysicsValidationResult::Warning;
            ValidationMessage = FString::Printf(TEXT("Warning: Physics tick time elevated (%.2f ms > %.2f ms)"), 
                Metrics.MaxPhysicsTickTime, MaxAcceptablePhysicsTickTime);
        }
    }
    
    LastValidationMessage = ValidationMessage;
}

void UPerf_PhysicsIntegrationValidator::LogValidationResult(const FPerf_PhysicsValidationMetrics& Metrics, EPerf_PhysicsValidationResult Result)
{
    FString ResultString;
    switch (Result)
    {
        case EPerf_PhysicsValidationResult::Passed:
            ResultString = TEXT("PASSED");
            break;
        case EPerf_PhysicsValidationResult::Warning:
            ResultString = TEXT("WARNING");
            break;
        case EPerf_PhysicsValidationResult::Failed:
            ResultString = TEXT("FAILED");
            break;
        case EPerf_PhysicsValidationResult::Critical:
            ResultString = TEXT("CRITICAL");
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS VALIDATION RESULT: %s ==="), *ResultString);
    UE_LOG(LogTemp, Log, TEXT("Total Physics Actors: %d"), Metrics.TotalPhysicsActors);
    UE_LOG(LogTemp, Log, TEXT("Simulating Actors: %d"), Metrics.SimulatingActors);
    UE_LOG(LogTemp, Log, TEXT("Static Actors: %d"), Metrics.StaticActors);
    UE_LOG(LogTemp, Log, TEXT("Average Physics Tick: %.2f ms"), Metrics.AveragePhysicsTickTime);
    UE_LOG(LogTemp, Log, TEXT("Max Physics Tick: %.2f ms"), Metrics.MaxPhysicsTickTime);
    UE_LOG(LogTemp, Log, TEXT("Collision Pairs: %d"), Metrics.CollisionPairsCount);
    UE_LOG(LogTemp, Log, TEXT("System Healthy: %s"), Metrics.bPhysicsSystemHealthy ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Message: %s"), *LastValidationMessage);
}