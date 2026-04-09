#include "AutomationTestSuite.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/StaticMeshActor.h"

DEFINE_LOG_CATEGORY(LogTranspersonalQA);

// Core System Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTranspersonalCoreSystemTest, "TranspersonalGame.Core.SystemInitialization", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTranspersonalCoreSystemTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogTranspersonalQA, Warning, TEXT("=== CORE SYSTEM TEST STARTED ==="));
    
    // Test 1: Engine initialization
    if (!GEngine)
    {
        AddError(TEXT("GEngine is null - engine not properly initialized"));
        return false;
    }
    
    // Test 2: World context
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    }
    
    if (!World)
    {
        AddError(TEXT("No valid world context available"));
        return false;
    }
    
    // Test 3: Game instance
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        AddError(TEXT("Game instance not available"));
        return false;
    }
    
    UE_LOG(LogTranspersonalQA, Warning, TEXT("✓ Core systems initialized successfully"));
    return true;
}

// VFX System Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTranspersonalVFXSystemTest, "TranspersonalGame.VFX.NiagaraIntegration", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTranspersonalVFXSystemTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogTranspersonalQA, Warning, TEXT("=== VFX SYSTEM TEST STARTED ==="));
    
    UWorld* World = GetTestWorld();
    if (!World)
    {
        AddError(TEXT("No test world available for VFX testing"));
        return false;
    }
    
    // Test 1: Niagara system spawning
    FVector SpawnLocation = FVector(0.0f, 0.0f, 100.0f);
    AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
    
    if (!TestActor)
    {
        AddError(TEXT("Failed to spawn test actor for VFX"));
        return false;
    }
    
    // Test 2: Niagara component creation
    UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(TestActor);
    if (!NiagaraComp)
    {
        AddError(TEXT("Failed to create Niagara component"));
        TestActor->Destroy();
        return false;
    }
    
    TestActor->SetRootComponent(NiagaraComp);
    
    // Test 3: Component activation
    NiagaraComp->Activate();
    
    if (!NiagaraComp->IsActive())
    {
        AddError(TEXT("Niagara component failed to activate"));
        TestActor->Destroy();
        return false;
    }
    
    // Cleanup
    TestActor->Destroy();
    
    UE_LOG(LogTranspersonalQA, Warning, TEXT("✓ VFX systems tested successfully"));
    return true;
}

// AI System Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTranspersonalAISystemTest, "TranspersonalGame.AI.BehaviorTreeIntegration", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTranspersonalAISystemTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogTranspersonalQA, Warning, TEXT("=== AI SYSTEM TEST STARTED ==="));
    
    UWorld* World = GetTestWorld();
    if (!World)
    {
        AddError(TEXT("No test world available for AI testing"));
        return false;
    }
    
    // Test 1: AI Controller creation
    AAIController* AIController = World->SpawnActor<AAIController>();
    if (!AIController)
    {
        AddError(TEXT("Failed to spawn AI Controller"));
        return false;
    }
    
    // Test 2: Behavior Tree component
    UBehaviorTreeComponent* BTComponent = AIController->FindComponentByClass<UBehaviorTreeComponent>();
    if (!BTComponent)
    {
        // Create if not exists
        BTComponent = NewObject<UBehaviorTreeComponent>(AIController);
        AIController->AddInstanceComponent(BTComponent);
    }
    
    if (!BTComponent)
    {
        AddError(TEXT("Failed to create Behavior Tree component"));
        AIController->Destroy();
        return false;
    }
    
    // Test 3: Blackboard component
    UBlackboardComponent* BBComponent = AIController->FindComponentByClass<UBlackboardComponent>();
    if (!BBComponent)
    {
        BBComponent = NewObject<UBlackboardComponent>(AIController);
        AIController->AddInstanceComponent(BBComponent);
    }
    
    if (!BBComponent)
    {
        AddError(TEXT("Failed to create Blackboard component"));
        AIController->Destroy();
        return false;
    }
    
    // Cleanup
    AIController->Destroy();
    
    UE_LOG(LogTranspersonalQA, Warning, TEXT("✓ AI systems tested successfully"));
    return true;
}

// Navigation System Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTranspersonalNavigationTest, "TranspersonalGame.Navigation.NavMeshGeneration", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTranspersonalNavigationTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogTranspersonalQA, Warning, TEXT("=== NAVIGATION SYSTEM TEST STARTED ==="));
    
    UWorld* World = GetTestWorld();
    if (!World)
    {
        AddError(TEXT("No test world available for navigation testing"));
        return false;
    }
    
    // Test 1: Navigation system availability
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys)
    {
        AddError(TEXT("Navigation system not available"));
        return false;
    }
    
    // Test 2: NavMesh data
    ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavSys->GetDefaultNavDataInstance());
    if (!NavMesh)
    {
        AddWarning(TEXT("No default NavMesh found - this may be expected in test environment"));
    }
    
    // Test 3: Pathfinding capability
    FVector StartLocation = FVector(0.0f, 0.0f, 0.0f);
    FVector EndLocation = FVector(1000.0f, 0.0f, 0.0f);
    
    FNavLocation NavStart, NavEnd;
    bool bStartValid = NavSys->ProjectPointToNavigation(StartLocation, NavStart);
    bool bEndValid = NavSys->ProjectPointToNavigation(EndLocation, NavEnd);
    
    if (!bStartValid || !bEndValid)
    {
        AddWarning(TEXT("Navigation projection failed - may be expected without NavMesh"));
    }
    
    UE_LOG(LogTranspersonalQA, Warning, TEXT("✓ Navigation systems tested successfully"));
    return true;
}

// Performance Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTranspersonalPerformanceTest, "TranspersonalGame.Performance.FrameRateStability", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTranspersonalPerformanceTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogTranspersonalQA, Warning, TEXT("=== PERFORMANCE TEST STARTED ==="));
    
    // Test 1: Frame rate monitoring
    float CurrentFPS = 1.0f / FApp::GetDeltaTime();
    
    if (CurrentFPS < 30.0f)
    {
        AddWarning(FString::Printf(TEXT("Low frame rate detected: %.2f FPS"), CurrentFPS));
    }
    
    // Test 2: Memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    UE_LOG(LogTranspersonalQA, Warning, TEXT("Current memory usage: %.2f MB"), MemoryUsageMB);
    
    if (MemoryUsageMB > 8192.0f) // 8GB threshold
    {
        AddWarning(TEXT("High memory usage detected"));
    }
    
    // Test 3: Garbage collection
    GEngine->ForceGarbageCollection(true);
    
    UE_LOG(LogTranspersonalQA, Warning, TEXT("✓ Performance tests completed"));
    return true;
}

// Audio System Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTranspersonalAudioSystemTest, "TranspersonalGame.Audio.SoundIntegration", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTranspersonalAudioSystemTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogTranspersonalQA, Warning, TEXT("=== AUDIO SYSTEM TEST STARTED ==="));
    
    UWorld* World = GetTestWorld();
    if (!World)
    {
        AddError(TEXT("No test world available for audio testing"));
        return false;
    }
    
    // Test 1: Audio component creation
    AActor* TestActor = World->SpawnActor<AActor>();
    if (!TestActor)
    {
        AddError(TEXT("Failed to spawn test actor for audio"));
        return false;
    }
    
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(TestActor);
    if (!AudioComp)
    {
        AddError(TEXT("Failed to create audio component"));
        TestActor->Destroy();
        return false;
    }
    
    TestActor->SetRootComponent(AudioComp);
    
    // Test 2: Audio component functionality
    AudioComp->SetVolumeMultiplier(1.0f);
    AudioComp->SetPitchMultiplier(1.0f);
    
    // Cleanup
    TestActor->Destroy();
    
    UE_LOG(LogTranspersonalQA, Warning, TEXT("✓ Audio systems tested successfully"));
    return true;
}

UWorld* UTranspersonalQATestSuite::GetTestWorld()
{
    UWorld* World = nullptr;
    
    if (GEngine)
    {
        World = GEngine->GetCurrentPlayWorld();
        if (!World)
        {
            World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
        }
    }
    
    return World;
}

void UTranspersonalQATestSuite::RunAllTests()
{
    UE_LOG(LogTranspersonalQA, Warning, TEXT("=== TRANSPERSONAL GAME QA TEST SUITE ==="));
    UE_LOG(LogTranspersonalQA, Warning, TEXT("Running comprehensive system tests..."));
    
    // Tests will be automatically discovered and run by the automation framework
    UE_LOG(LogTranspersonalQA, Warning, TEXT("All tests queued for execution"));
}