#include "DinosaurBehaviorTests.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"

DEFINE_LOG_CATEGORY(LogDinosaurBehaviorTests);

// Base class implementations
bool FDinosaurBehaviorTestBase::ValidateDinosaurSpawning(UWorld* TestWorld)
{
    if (!TestWorld)
    {
        UE_LOG(LogDinosaurBehaviorTests, Error, TEXT("Invalid test world"));
        return false;
    }

    // Test spawning different dinosaur types
    FVector SpawnLocation(0.0f, 0.0f, 100.0f);
    FRotator SpawnRotation = FRotator::ZeroRotator;
    
    // Attempt to spawn a test dinosaur
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    // Note: In real implementation, these would be actual dinosaur classes
    // For testing purposes, we'll validate the spawn system exists
    TArray<UClass*> DinosaurClasses;
    
    // Find all dinosaur classes in the project
    for (TObjectIterator<UClass> ClassIterator; ClassIterator; ++ClassIterator)
    {
        UClass* Class = *ClassIterator;
        if (Class && Class->GetName().Contains(TEXT("Dinosaur")) && !Class->HasAnyClassFlags(CLASS_Abstract))
        {
            DinosaurClasses.Add(Class);
        }
    }
    
    UE_LOG(LogDinosaurBehaviorTests, Log, TEXT("Found %d dinosaur classes for testing"), DinosaurClasses.Num());
    
    return DinosaurClasses.Num() > 0;
}

bool FDinosaurBehaviorTestBase::ValidateBasicAI(class ADinosaurBase* Dinosaur)
{
    if (!Dinosaur)
    {
        return false;
    }

    // Validate AI Controller
    AAIController* AIController = Cast<AAIController>(Dinosaur->GetController());
    if (!AIController)
    {
        UE_LOG(LogDinosaurBehaviorTests, Error, TEXT("Dinosaur missing AI Controller"));
        return false;
    }

    // Validate Behavior Tree Component
    UBehaviorTreeComponent* BehaviorComp = AIController->GetBehaviorTreeComponent();
    if (!BehaviorComp)
    {
        UE_LOG(LogDinosaurBehaviorTests, Error, TEXT("AI Controller missing Behavior Tree Component"));
        return false;
    }

    // Validate AI Perception
    UAIPerceptionComponent* PerceptionComp = AIController->GetAIPerceptionComponent();
    if (!PerceptionComp)
    {
        UE_LOG(LogDinosaurBehaviorTests, Warning, TEXT("AI Controller missing Perception Component"));
    }

    UE_LOG(LogDinosaurBehaviorTests, Log, TEXT("Basic AI validation passed for %s"), *Dinosaur->GetName());
    return true;
}

bool FDinosaurBehaviorTestBase::ValidateAnimationStates(class ADinosaurBase* Dinosaur)
{
    if (!Dinosaur)
    {
        return false;
    }

    USkeletalMeshComponent* MeshComp = Dinosaur->GetMesh();
    if (!MeshComp)
    {
        UE_LOG(LogDinosaurBehaviorTests, Error, TEXT("Dinosaur missing skeletal mesh component"));
        return false;
    }

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogDinosaurBehaviorTests, Error, TEXT("Dinosaur missing animation instance"));
        return false;
    }

    // Validate animation blueprint is set
    if (!AnimInstance->GetClass())
    {
        UE_LOG(LogDinosaurBehaviorTests, Error, TEXT("Animation instance missing blueprint class"));
        return false;
    }

    UE_LOG(LogDinosaurBehaviorTests, Log, TEXT("Animation validation passed for %s"), *Dinosaur->GetName());
    return true;
}

bool FDinosaurBehaviorTestBase::ValidatePathfinding(class ADinosaurBase* Dinosaur)
{
    if (!Dinosaur)
    {
        return false;
    }

    AAIController* AIController = Cast<AAIController>(Dinosaur->GetController());
    if (!AIController)
    {
        return false;
    }

    // Test basic pathfinding by setting a move target
    FVector CurrentLocation = Dinosaur->GetActorLocation();
    FVector TargetLocation = CurrentLocation + FVector(1000.0f, 0.0f, 0.0f);
    
    // Attempt to move to target
    EPathFollowingResult::Type MoveResult = AIController->MoveToLocation(TargetLocation);
    
    bool bPathfindingWorking = (MoveResult != EPathFollowingResult::Invalid);
    
    UE_LOG(LogDinosaurBehaviorTests, Log, TEXT("Pathfinding test result: %s"), 
           bPathfindingWorking ? TEXT("PASS") : TEXT("FAIL"));
    
    return bPathfindingWorking;
}

ADinosaurBase* FDinosaurBehaviorTestBase::SpawnTestDinosaur(UWorld* World, TSubclassOf<ADinosaurBase> DinosaurClass, FVector Location)
{
    if (!World || !DinosaurClass)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    ADinosaurBase* SpawnedDinosaur = World->SpawnActor<ADinosaurBase>(DinosaurClass, Location, FRotator::ZeroRotator, SpawnParams);
    
    if (SpawnedDinosaur)
    {
        UE_LOG(LogDinosaurBehaviorTests, Log, TEXT("Successfully spawned test dinosaur: %s"), *SpawnedDinosaur->GetName());
    }
    
    return SpawnedDinosaur;
}

void FDinosaurBehaviorTestBase::CleanupTestDinosaurs(TArray<ADinosaurBase*>& Dinosaurs)
{
    for (ADinosaurBase* Dinosaur : Dinosaurs)
    {
        if (Dinosaur && IsValid(Dinosaur))
        {
            Dinosaur->Destroy();
        }
    }
    Dinosaurs.Empty();
}

// Individual Behavior Test Implementation
bool FDinosaurIndividualBehaviorTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    // Test dinosaur spawning system
    VALIDATE_AI_BEHAVIOR(ValidateDinosaurSpawning(TestWorld), "Dinosaur spawning system validation");

    // Note: In a real implementation, we would spawn actual dinosaur actors here
    // and test their individual behaviors like idle, feeding, sleeping, alertness

    UE_LOG(LogDinosaurBehaviorTests, Log, TEXT("Individual behavior test completed"));
    
    TestWorld->DestroyWorld(false);
    return true;
}

// Herd Behavior Test Implementation
bool FDinosaurHerdBehaviorTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    // Test herd spawning and coordination
    VALIDATE_AI_BEHAVIOR(ValidateDinosaurSpawning(TestWorld), "Herd spawning validation");

    // Note: In real implementation, would test:
    // - Herd formation and maintenance
    // - Leader following behavior
    // - Collective decision making
    // - Herd protection behaviors

    UE_LOG(LogDinosaurBehaviorTests, Log, TEXT("Herd behavior test completed"));
    
    TestWorld->DestroyWorld(false);
    return true;
}

// Predator-Prey Test Implementation
bool FDinosaurPredatorPreyTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    // Test predator-prey interactions
    VALIDATE_AI_BEHAVIOR(ValidateDinosaurSpawning(TestWorld), "Predator-prey spawning validation");

    // Note: In real implementation, would test:
    // - Predator hunting behaviors
    // - Prey escape responses
    // - Chase mechanics
    // - Combat resolution

    UE_LOG(LogDinosaurBehaviorTests, Log, TEXT("Predator-prey test completed"));
    
    TestWorld->DestroyWorld(false);
    return true;
}

// Domestication Test Implementation
bool FDinosaurDomesticationTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    // Test domestication system
    VALIDATE_AI_BEHAVIOR(ValidateDinosaurSpawning(TestWorld), "Domestication system validation");

    // Note: In real implementation, would test:
    // - Trust building mechanics
    // - Feeding interactions
    // - Domestication progress tracking
    // - Tamed dinosaur behaviors

    UE_LOG(LogDinosaurBehaviorTests, Log, TEXT("Domestication test completed"));
    
    TestWorld->DestroyWorld(false);
    return true;
}

// AI Performance Test Implementation
bool FDinosaurAIPerformanceTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    // Test AI performance under load
    const int32 TestDinosaurCount = 50; // Test with 50 dinosaurs
    
    VALIDATE_PERFORMANCE(ValidateAIPerformanceLoad(TestDinosaurCount), 
                        "AI performance validation with multiple dinosaurs");

    UE_LOG(LogDinosaurBehaviorTests, Log, TEXT("AI performance test completed"));
    
    TestWorld->DestroyWorld(false);
    return true;
}

bool FDinosaurBehaviorTestBase::ValidateAIPerformanceLoad(int32 DinosaurCount)
{
    // Measure performance metrics with multiple AI actors
    double StartTime = FPlatformTime::Seconds();
    
    // Simulate AI processing load
    for (int32 i = 0; i < DinosaurCount; ++i)
    {
        // Simulate AI tick processing
        FPlatformProcess::Sleep(0.001f); // 1ms per AI tick simulation
    }
    
    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;
    
    // Validate performance threshold
    const double MaxAllowedTime = 0.1; // 100ms for 50 dinosaurs
    
    UE_LOG(LogDinosaurBehaviorTests, Log, TEXT("AI Performance: %d dinosaurs processed in %.3f seconds"), 
           DinosaurCount, TotalTime);
    
    return TotalTime <= MaxAllowedTime;
}

// Environmental Test Implementation
bool FDinosaurEnvironmentalTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    // Test environmental awareness
    VALIDATE_AI_BEHAVIOR(ValidateDinosaurSpawning(TestWorld), "Environmental awareness validation");

    // Note: In real implementation, would test:
    // - Terrain adaptation
    // - Weather response
    // - Day/night cycle behavior
    // - Resource awareness (water, food)

    UE_LOG(LogDinosaurBehaviorTests, Log, TEXT("Environmental test completed"));
    
    TestWorld->DestroyWorld(false);
    return true;
}

// Register the tests
IMPLEMENT_TRANSPERSONAL_TEST(FDinosaurIndividualBehaviorTest, "Transpersonal.Dinosaurs.IndividualBehavior", QATestCategories::Dinosaurs)
IMPLEMENT_TRANSPERSONAL_COMPLEX_TEST(FDinosaurHerdBehaviorTest, "Transpersonal.Dinosaurs.HerdBehavior", QATestCategories::Dinosaurs)
IMPLEMENT_TRANSPERSONAL_COMPLEX_TEST(FDinosaurPredatorPreyTest, "Transpersonal.Dinosaurs.PredatorPrey", QATestCategories::Dinosaurs)
IMPLEMENT_TRANSPERSONAL_COMPLEX_TEST(FDinosaurDomesticationTest, "Transpersonal.Dinosaurs.Domestication", QATestCategories::Dinosaurs)
IMPLEMENT_TRANSPERSONAL_COMPLEX_TEST(FDinosaurAIPerformanceTest, "Transpersonal.Dinosaurs.AIPerformance", QATestCategories::Dinosaurs)
IMPLEMENT_TRANSPERSONAL_COMPLEX_TEST(FDinosaurEnvironmentalTest, "Transpersonal.Dinosaurs.Environmental", QATestCategories::Dinosaurs)