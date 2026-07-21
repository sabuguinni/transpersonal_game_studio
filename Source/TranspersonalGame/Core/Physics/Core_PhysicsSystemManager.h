#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Core/SharedTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GlobalGravityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DefaultLinearDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DefaultAngularDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxPhysicsSubsteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableAsyncPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float PhysicsTimeStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float CollisionTolerance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 MaxPhysicsObjects;

    FCore_PhysicsSettings()
    {
        GlobalGravityScale = 1.0f;
        DefaultLinearDamping = 0.01f;
        DefaultAngularDamping = 0.0f;
        MaxPhysicsSubsteps = 6.0f;
        bEnableAsyncPhysics = true;
        PhysicsTimeStep = 0.016667f; // 60fps
        CollisionTolerance = 0.1f;
        MaxPhysicsObjects = 10000;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsObjectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    AActor* PhysicsActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FVector AngularVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bIsSimulating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_PhysicsType PhysicsType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LastUpdateTime;

    FCore_PhysicsObjectData()
    {
        PhysicsActor = nullptr;
        Mass = 1.0f;
        Velocity = FVector::ZeroVector;
        AngularVelocity = FVector::ZeroVector;
        bIsSimulating = true;
        PhysicsType = ECore_PhysicsType::Dynamic;
        LastUpdateTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Physics Management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UpdatePhysicsSettings(const FCore_PhysicsSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void RegisterPhysicsObject(AActor* Actor, ECore_PhysicsType PhysicsType = ECore_PhysicsType::Dynamic);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UnregisterPhysicsObject(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetGlobalGravity(float NewGravityScale);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnablePhysicsSimulation(AActor* Actor, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyImpulse(AActor* Actor, const FVector& Impulse, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyForce(AActor* Actor, const FVector& Force);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetObjectMass(AActor* Actor, float NewMass);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetLinearDamping(AActor* Actor, float Damping);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetAngularDamping(AActor* Actor, float Damping);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void FreezeObject(AActor* Actor, bool bFreezePosition = true, bool bFreezeRotation = true);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void WakeUpObject(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void PutObjectToSleep(AActor* Actor);

    // Physics Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics System")
    bool IsObjectSimulating(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics System")
    FVector GetObjectVelocity(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics System")
    float GetObjectMass(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics System")
    int32 GetActivePhysicsObjectCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics System")
    TArray<AActor*> GetPhysicsObjects() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsLOD(AActor* Actor, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnablePhysicsMultithreading(bool bEnable);

    // Debug Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics System")
    void DebugDrawPhysicsObjects();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics System")
    void LogPhysicsSystemStats();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (AllowPrivateAccess = "true"))
    FCore_PhysicsSettings PhysicsSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Objects", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_PhysicsObjectData> RegisteredObjects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 ActivePhysicsObjects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float PhysicsUpdateTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsSystemInitialized;

private:
    void UpdatePhysicsObjectData(float DeltaTime);
    void CleanupInvalidObjects();
    UPrimitiveComponent* GetPhysicsComponent(AActor* Actor) const;
    void ApplyPhysicsLOD(AActor* Actor, int32 LODLevel);
    void ValidatePhysicsSettings();

    float LastOptimizationTime;
    int32 PhysicsFrameCounter;
    TArray<AActor*> PendingRegistration;
    TArray<AActor*> PendingUnregistration;
};