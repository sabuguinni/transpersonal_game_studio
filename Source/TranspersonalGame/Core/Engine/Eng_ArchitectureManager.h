#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "../../SharedTypes.h"
#include "Eng_ArchitectureManager.generated.h"

/**
 * Engine Architecture Manager
 * Central system that manages the technical architecture of the game.
 * Coordinates module loading, system initialization, and performance monitoring.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_ArchitectureManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_ArchitectureManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core architecture components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    UStaticMeshComponent* ArchitectureMesh;

    // System status tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    bool bIsSystemInitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    float SystemLoadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    int32 ActiveModuleCount;

    // Performance metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CurrentFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EEng_SystemState SystemState;

public:
    // Architecture management functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UnregisterModule(const FString& ModuleName);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSystemLoadPercentage() const;

    // System state management
    UFUNCTION(BlueprintCallable, Category = "System")
    void SetSystemState(EEng_SystemState NewState);

    UFUNCTION(BlueprintCallable, Category = "System")
    EEng_SystemState GetSystemState() const;

    // Debug and validation
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void LogSystemStatus();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void RunArchitectureValidation();

private:
    // Internal system tracking
    TArray<FString> RegisteredModules;
    float LastPerformanceUpdate;
    bool bPerformanceMonitoringEnabled;

    // Internal helper functions
    void SetupArchitectureVisualization();
    void InitializePerformanceMonitoring();
    void ValidateModuleDependencies();
};