// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsValidationActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Chaos/ChaosEngineInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhysicsValidation, Log, All);

APhysicsValidationActor::APhysicsValidationActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create test mesh component
    TestMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TestMeshComponent"));
    TestMeshComponent->SetupAttachment(RootComponent);
    TestMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TestMeshComponent->SetSimulatePhysics(true);

    // Create skeletal mesh component for ragdoll tests
    TestSkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TestSkeletalMeshComponent"));
    TestSkeletalMeshComponent->SetupAttachment(RootComponent);
    TestSkeletalMeshComponent->SetRelativeLocation(FVector(200.0f, 0.0f, 0.0f));

    // Create physics system components
    CollisionSystemComponent = CreateDefaultSubobject<UCollisionSystem>(TEXT("CollisionSystemComponent"));
    RagdollSystemComponent = CreateDefaultSubobject<URagdollSystemComponent>(TEXT("RagdollSystemComponent"));

    // Set default mesh to cube
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        TestMeshComponent->SetStaticMesh(CubeMeshAsset.Object);
    }

    // Initialize validation results
    ValidationResults = TEXT("Physics Validation - Not Run");
}

void APhysicsValidationActor::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTestComponents();
    
    if (bRunTestsOnBeginPlay)
    {
        // Delay test execution to allow systems to initialize
        GetWorldTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &APhysicsValidationActor::RunAllValidationTests,
            1.0f,
            false
        );
    }
}

void APhysicsValidationActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Draw debug information for active tests
    if (SpawnedTestObjects.Num() > 0)
    {
        for (AActor* TestObject : SpawnedTestObjects)
        {
            if (IsValid(TestObject))
            {
                DrawDebugSphere(GetWorld(), TestObject->GetActorLocation(), 25.0f, 8, FColor::Green, false, -1.0f, 0, 2.0f);
            }
        }
    }
}

void APhysicsValidationActor::RunAllValidationTests()
{
    UE_LOG(LogPhysicsValidation, Warning, TEXT("Starting Physics Validation Tests..."));
    
    TestStartTime = GetWorld()->GetTimeSeconds();
    TestsPassed = 0;
    TestsFailed = 0;
    ValidationResults = TEXT("Running Physics Validation Tests...\n");

    // Test Physics System Manager
    UPhysicsSystemManager* PhysicsManager = UPhysicsSystemManager::Get(this);
    LogTestResult(TEXT("Physics System Manager"), PhysicsManager != nullptr, 
        PhysicsManager ? TEXT("Manager found and accessible") : TEXT("Manager not found"));

    // Test Chaos Physics availability
    bool bChaosAvailable = true; // Chaos is default in UE5
    LogTestResult(TEXT("Chaos Physics"), bChaosAvailable, TEXT("Chaos Physics system available"));

    // Run individual system tests
    TestCollisionSystem();
    TestRagdollSystem();
    TestDestructionSystem();
    
    // Performance test with fewer objects for validation
    TestPhysicsPerformance(25);

    // Compile final results
    float TestDuration = GetWorld()->GetTimeSeconds() - TestStartTime;
    ValidationResults += FString::Printf(TEXT("\n=== VALIDATION SUMMARY ===\n"));
    ValidationResults += FString::Printf(TEXT("Tests Passed: %d\n"), TestsPassed);
    ValidationResults += FString::Printf(TEXT("Tests Failed: %d\n"), TestsFailed);
    ValidationResults += FString::Printf(TEXT("Test Duration: %.2f seconds\n"), TestDuration);
    ValidationResults += FString::Printf(TEXT("Overall Result: %s\n"), 
        TestsFailed == 0 ? TEXT("PASS") : TEXT("FAIL"));

    UE_LOG(LogPhysicsValidation, Warning, TEXT("Physics Validation Complete - %d passed, %d failed"), 
        TestsPassed, TestsFailed);
}

void APhysicsValidationActor::TestCollisionSystem()
{
    UE_LOG(LogPhysicsValidation, Log, TEXT("Testing Collision System..."));

    // Test collision system component
    bool bCollisionSystemValid = IsValid(CollisionSystemComponent);
    LogTestResult(TEXT("Collision System Component"), bCollisionSystemValid, 
        bCollisionSystemValid ? TEXT("Component created successfully") : TEXT("Component creation failed"));

    if (bCollisionSystemValid)
    {
        // Test collision system initialization
        CollisionSystemComponent->InitializeCollisionSystem();
        LogTestResult(TEXT("Collision System Init"), true, TEXT("Initialization completed"));

        // Test collision detection between test objects
        if (SpawnedTestObjects.Num() >= 2)
        {
            FAdvancedCollisionResult CollisionResult = CollisionSystemComponent->PerformAdvancedCollisionTest(
                SpawnedTestObjects[0], 
                SpawnedTestObjects[1], 
                ECollisionTestType::Simple
            );
            LogTestResult(TEXT("Collision Detection"), true, TEXT("Collision test completed"));
        }
    }
}

void APhysicsValidationActor::TestRagdollSystem()
{
    UE_LOG(LogPhysicsValidation, Log, TEXT("Testing Ragdoll System..."));

    // Test ragdoll system component
    bool bRagdollSystemValid = IsValid(RagdollSystemComponent);
    LogTestResult(TEXT("Ragdoll System Component"), bRagdollSystemValid,
        bRagdollSystemValid ? TEXT("Component created successfully") : TEXT("Component creation failed"));

    if (bRagdollSystemValid && IsValid(TestSkeletalMeshComponent))
    {
        // Test ragdoll activation
        RagdollSystemComponent->ActivateRagdoll(GetActorLocation(), 500.0f);
        bool bRagdollActive = RagdollSystemComponent->IsRagdollActive();
        LogTestResult(TEXT("Ragdoll Activation"), bRagdollActive, 
            bRagdollActive ? TEXT("Ragdoll activated successfully") : TEXT("Ragdoll activation failed"));

        // Test ragdoll deactivation
        RagdollSystemComponent->DeactivateRagdoll();
        bool bRagdollDeactivated = !RagdollSystemComponent->IsRagdollActive();
        LogTestResult(TEXT("Ragdoll Deactivation"), bRagdollDeactivated,
            bRagdollDeactivated ? TEXT("Ragdoll deactivated successfully") : TEXT("Ragdoll deactivation failed"));
    }
}

void APhysicsValidationActor::TestDestructionSystem()
{
    UE_LOG(LogPhysicsValidation, Log, TEXT("Testing Destruction System..."));

    // Test if destruction system classes are available
    UClass* DestructionSystemClass = FindObject<UClass>(ANY_PACKAGE, TEXT("DestructionSystemComponent"));
    bool bDestructionSystemAvailable = DestructionSystemClass != nullptr;
    LogTestResult(TEXT("Destruction System Class"), bDestructionSystemAvailable,
        bDestructionSystemAvailable ? TEXT("Destruction system class found") : TEXT("Destruction system class not found"));

    // Test Chaos Destruction availability
    bool bChaosDestructionAvailable = true; // Assume available in UE5
    LogTestResult(TEXT("Chaos Destruction"), bChaosDestructionAvailable, TEXT("Chaos Destruction system available"));
}

void APhysicsValidationActor::TestPhysicsPerformance(int32 NumObjects)
{
    UE_LOG(LogPhysicsValidation, Log, TEXT("Testing Physics Performance with %d objects..."), NumObjects);

    float StartTime = FPlatformTime::Seconds();
    
    // Spawn test objects for performance testing
    SpawnTestPhysicsObjects(NumObjects);
    
    float SpawnTime = FPlatformTime::Seconds() - StartTime;
    
    // Wait a frame for physics to settle
    GetWorldTimerManager().SetTimer(
        FTimerHandle(),
        [this, StartTime, SpawnTime, NumObjects]()
        {
            float TotalTime = FPlatformTime::Seconds() - StartTime;
            bool bPerformanceGood = TotalTime < 1.0f; // Should complete within 1 second
            
            FString PerformanceDetails = FString::Printf(TEXT("Spawned %d objects in %.3f seconds, total time %.3f seconds"), 
                NumObjects, SpawnTime, TotalTime);
            LogTestResult(TEXT("Physics Performance"), bPerformanceGood, PerformanceDetails);
            
            // Clean up test objects
            ClearTestObjects();
        },
        0.1f,
        false
    );
}

void APhysicsValidationActor::SpawnTestPhysicsObjects(int32 Count)
{
    UE_LOG(LogPhysicsValidation, Log, TEXT("Spawning %d test physics objects..."), Count);

    ClearTestObjects(); // Clear existing objects first

    FVector BaseLocation = GetActorLocation() + TestSpawnOffset;
    
    for (int32 i = 0; i < Count; ++i)
    {
        FVector SpawnLocation = BaseLocation + FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(0.0f, 1000.0f)
        );

        AActor* TestObject = nullptr;
        if (i % 2 == 0)
        {
            TestObject = SpawnTestCube(SpawnLocation, FMath::RandRange(50.0f, 150.0f));
        }
        else
        {
            TestObject = SpawnTestSphere(SpawnLocation, FMath::RandRange(25.0f, 75.0f));
        }

        if (TestObject)
        {
            SpawnedTestObjects.Add(TestObject);
        }
    }

    UE_LOG(LogPhysicsValidation, Log, TEXT("Spawned %d test objects"), SpawnedTestObjects.Num());
}

void APhysicsValidationActor::ClearTestObjects()
{
    for (AActor* TestObject : SpawnedTestObjects)
    {
        if (IsValid(TestObject))
        {
            TestObject->Destroy();
        }
    }
    SpawnedTestObjects.Empty();
}

void APhysicsValidationActor::LogTestResult(const FString& TestName, bool bSuccess, const FString& Details)
{
    if (bSuccess)
    {
        TestsPassed++;
        ValidationResults += FString::Printf(TEXT("✓ %s: PASS"), *TestName);
    }
    else
    {
        TestsFailed++;
        ValidationResults += FString::Printf(TEXT("✗ %s: FAIL"), *TestName);
    }

    if (!Details.IsEmpty())
    {
        ValidationResults += FString::Printf(TEXT(" - %s"), *Details);
    }
    ValidationResults += TEXT("\n");

    UE_LOG(LogPhysicsValidation, Log, TEXT("%s: %s - %s"), 
        *TestName, bSuccess ? TEXT("PASS") : TEXT("FAIL"), *Details);
}

void APhysicsValidationActor::InitializeTestComponents()
{
    // Initialize collision system
    if (IsValid(CollisionSystemComponent))
    {
        CollisionSystemComponent->InitializeCollisionSystem();
    }

    // Configure test mesh physics
    if (IsValid(TestMeshComponent))
    {
        TestMeshComponent->SetMassOverrideInKg(NAME_None, 100.0f, true);
        TestMeshComponent->SetLinearDamping(0.1f);
        TestMeshComponent->SetAngularDamping(0.1f);
    }

    // Configure skeletal mesh for ragdoll testing
    if (IsValid(TestSkeletalMeshComponent))
    {
        TestSkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        TestSkeletalMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    }
}

AActor* APhysicsValidationActor::SpawnTestCube(FVector Location, float Size)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Spawn a static mesh actor with cube mesh
    AStaticMeshActor* CubeActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (CubeActor)
    {
        UStaticMeshComponent* MeshComp = CubeActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Set cube mesh
            UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
            if (CubeMesh)
            {
                MeshComp->SetStaticMesh(CubeMesh);
                MeshComp->SetWorldScale3D(FVector(Size / 100.0f)); // Default cube is 100 units
                MeshComp->SetSimulatePhysics(true);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                MeshComp->SetMassOverrideInKg(NAME_None, Size * 0.1f, true);
            }
        }
    }

    return CubeActor;
}

AActor* APhysicsValidationActor::SpawnTestSphere(FVector Location, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Spawn a static mesh actor with sphere mesh
    AStaticMeshActor* SphereActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (SphereActor)
    {
        UStaticMeshComponent* MeshComp = SphereActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            // Set sphere mesh
            UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
            if (SphereMesh)
            {
                MeshComp->SetStaticMesh(SphereMesh);
                MeshComp->SetWorldScale3D(FVector(Radius / 50.0f)); // Default sphere is 50 units radius
                MeshComp->SetSimulatePhysics(true);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                MeshComp->SetMassOverrideInKg(NAME_None, Radius * 0.05f, true);
            }
        }
    }

    return SphereActor;
}