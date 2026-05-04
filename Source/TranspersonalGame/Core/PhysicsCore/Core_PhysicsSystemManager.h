#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsMode : uint8
{
    Realistic       UMETA(DisplayName = "Realistic Physics"),
    Arcade         UMETA(DisplayName = "Arcade Physics"),
    Survival       UMETA(DisplayName = "Survival Physics"),
    Cinematic      UMETA(DisplayName = "Cinematic Physics")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxAngularVelocity = 3600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bSimulatePhysics = true;

    FCore_PhysicsSettings()
    {
        GravityScale = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        MaxAngularVelocity = 3600.0f;
        bEnableGravity = true;
        bSimulatePhysics = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_PhysicsSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Physics mode control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics System")
    ECore_PhysicsMode CurrentPhysicsMode = ECore_PhysicsMode::Realistic;

    // Physics settings for different modes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics System")
    FCore_PhysicsSettings RealisticSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics System")
    FCore_PhysicsSettings ArcadeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics System")
    FCore_PhysicsSettings SurvivalSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics System")
    FCore_PhysicsSettings CinematicSettings;

    // Performance monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsObjects = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsObjects = 1000;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsMode(ECore_PhysicsMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyPhysicsSettingsToActor(AActor* Actor, const FCore_PhysicsSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    FCore_PhysicsSettings GetCurrentPhysicsSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UpdatePhysicsStatistics();

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Physics System")
    void TestPhysicsSystem();

    UFUNCTION(CallInEditor, Category = "Physics System")
    void ResetPhysicsSettings();

private:
    void InitializePhysicsSettings();
    void ApplyGlobalPhysicsSettings(const FCore_PhysicsSettings& Settings);
    void MonitorPhysicsPerformance();

    // Internal state
    float LastPerformanceCheck = 0.0f;
    float PerformanceCheckInterval = 1.0f;
};