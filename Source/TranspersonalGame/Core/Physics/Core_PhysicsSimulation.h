#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsSimulation.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsQuality : uint8
{
    Low UMETA(DisplayName = "Low Quality"),
    Medium UMETA(DisplayName = "Medium Quality"),
    High UMETA(DisplayName = "High Quality"),
    Ultra UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float TimeStep = 0.016667f; // 60fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 MaxSubSteps = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableAsyncPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_PhysicsQuality QualityLevel = ECore_PhysicsQuality::High;

    FCore_PhysicsSettings()
    {
        TimeStep = 0.016667f;
        MaxSubSteps = 6;
        GravityScale = 1.0f;
        bEnableAsyncPhysics = true;
        QualityLevel = ECore_PhysicsQuality::High;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsSimulation : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSimulation();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    FCore_PhysicsSettings PhysicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bUseCustomGravity = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (EditCondition = "bUseCustomGravity"))
    FVector CustomGravityDirection = FVector(0.0f, 0.0f, -980.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsUpdateRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsObjects = 1000;

    UPROPERTY(BlueprintReadOnly, Category = "Debug")
    int32 CurrentPhysicsObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Debug")
    float AverageFrameTime = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyPhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsQuality(ECore_PhysicsQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnableAsyncPhysics(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetCustomGravity(FVector NewGravity);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void GetPhysicsStats(int32& OutActiveObjects, float& OutFrameTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsPerformance();

private:
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCounter = 0;
    
    void UpdatePhysicsStats();
    void ApplyQualitySettings();
};