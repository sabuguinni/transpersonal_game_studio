#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Core_PhysicsTestManager.generated.h"

/**
 * Physics Test Manager - Creates and manages physics test scenarios
 * Spawns test objects, applies forces, and validates physics behavior
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_PhysicsTestManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsTestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Physics test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Testing")
    int32 TestCubeCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Testing")
    float TestObjectMass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Testing")
    float ForceApplicationInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Testing")
    float TestForceStrength = 50000.0f;

    // Test objects tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Testing")
    TArray<AActor*> TestCubes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Testing")
    AActor* TestSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Testing")
    AActor* TestRamp;

    // Physics test functions
    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void SpawnPhysicsTestObjects();

    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void ApplyRandomForcesToTestObjects();

    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void ResetTestObjectPositions();

    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void ValidatePhysicsBehavior();

    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void CleanupTestObjects();

    // Material physics testing
    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void TestMaterialPhysicsInteractions();

    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void TestCollisionResponses();

    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void TestDestructionPhysics();

private:
    float LastForceApplicationTime = 0.0f;
    int32 TestObjectIndex = 0;

    // Helper functions
    void ConfigurePhysicsForActor(AActor* Actor, float Mass, bool bSimulatePhysics = true);
    FVector GetRandomForceVector() const;
    void LogPhysicsTestResults(const FString& TestName, bool bSuccess) const;
};