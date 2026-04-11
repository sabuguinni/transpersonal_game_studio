#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerformanceTarget : uint8
{
    PC_High = 0     UMETA(DisplayName = "PC High-End (60 FPS)"),
    PC_Medium = 1   UMETA(DisplayName = "PC Medium (45 FPS)"),
    Console = 2     UMETA(DisplayName = "Console (30 FPS)"),
    Mobile = 3      UMETA(DisplayName = "Mobile (30 FPS)")
};

UENUM(BlueprintType)
enum class EPerformanceLevel : uint8
{
    Ultra = 0       UMETA(DisplayName = "Ultra Quality"),
    High = 1        UMETA(DisplayName = "High Quality"),
    Medium = 2      UMETA(DisplayName = "Medium Quality"),
    Low = 3         UMETA(DisplayName = "Low Quality"),
    Potato = 4      UMETA(DisplayName = "Potato Mode")
};

USTRUCT(BlueprintType)
struct FPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 ShadowMapResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    float ViewDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 MaxLights = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bEnableRayTracing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bEnableLumen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    int32 AntiAliasingMethod = 2; // TAA

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    int32 PostProcessQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    int32 TexturePoolSize = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODBias = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullingDistance = 10000.0f;

    FPerformanceSettings()
    {
        // Default constructor with safe defaults
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    APerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsageMB() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetDrawCalls() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetTriangleCount() const;

    // Performance settings
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(EPerformanceTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerformanceLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceSettings(const FPerformanceSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceSettings GetCurrentSettings() const { return CurrentSettings; }

    // Optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeShadows();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLighting();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTextures();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMode();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void DisablePerformanceMode();

    // Auto-optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartAutoOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopAutoOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsAutoOptimizationEnabled() const { return bAutoOptimizationEnabled; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerformanceTarget CurrentTarget = EPerformanceTarget::PC_High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerformanceLevel CurrentLevel = EPerformanceLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerformanceSettings CurrentSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinAcceptableFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoOptimizationEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationCheckInterval = 5.0f;

private:
    // Performance tracking
    TArray<float> FPSHistory;
    float LastOptimizationCheck = 0.0f;
    int32 MaxFPSHistorySize = 60; // 1 second at 60fps

    // Internal functions
    void UpdateFPSHistory();
    void CheckPerformanceAndOptimize();
    void ApplyTargetSettings();
    void ExecuteConsoleCommand(const FString& Command);
    FPerformanceSettings GetSettingsForLevel(EPerformanceLevel Level) const;
    FPerformanceSettings GetSettingsForTarget(EPerformanceTarget Target) const;
};