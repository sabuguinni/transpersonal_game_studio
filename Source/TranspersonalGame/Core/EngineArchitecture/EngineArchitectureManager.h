#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../../SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

/**
 * Engine Architecture Manager - Central coordinator for all game systems
 * Manages system lifecycle, dependencies, and performance monitoring
 */

UENUM(BlueprintType)
enum class EEng_SystemType : uint8
{
    Core            UMETA(DisplayName = "Core Systems"),
    WorldGeneration UMETA(DisplayName = "World Generation"),
    Character       UMETA(DisplayName = "Character Systems"),
    AI              UMETA(DisplayName = "AI Systems"),
    Combat          UMETA(DisplayName = "Combat Systems"),
    Audio           UMETA(DisplayName = "Audio Systems"),
    VFX             UMETA(DisplayName = "VFX Systems"),
    Performance     UMETA(DisplayName = "Performance Systems"),
    Network         UMETA(DisplayName = "Network Systems")
};

UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Active          UMETA(DisplayName = "Active"),
    Error           UMETA(DisplayName = "Error"),
    Shutdown        UMETA(DisplayName = "Shutdown")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    EEng_SystemType SystemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    EEng_SystemStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    float InitializationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    float LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    TArray<FString> Dependencies;

    FEng_SystemInfo()
    {
        SystemName = TEXT("");
        SystemType = EEng_SystemType::Core;
        Status = EEng_SystemStatus::Inactive;
        InitializationTime = 0.0f;
        LastUpdateTime = 0.0f;
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 VisibleActorCount;

    FEng_PerformanceMetrics()
    {
        FrameRate = 60.0f;
        MemoryUsageMB = 0.0f;
        CPUUsagePercent = 0.0f;
        GPUUsagePercent = 0.0f;
        ActiveActorCount = 0;
        VisibleActorCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterSystem(const FEng_SystemInfo& SystemInfo);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool InitializeSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ShutdownSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EEng_SystemStatus GetSystemStatus(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FEng_SystemInfo> GetAllSystems() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FEng_SystemInfo> GetSystemsByType(EEng_SystemType SystemType) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits() const;

    // System Dependencies
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckSystemDependencies(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetSystemDependencies(const FString& SystemName) const;

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintSystemStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ValidateAllSystems() const;

    UFUNCTION(CallInEditor, Category = "Debug")
    void EditorValidateArchitecture();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Systems")
    TArray<FEng_SystemInfo> RegisteredSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB;

private:
    FTimerHandle PerformanceTimerHandle;

    void InitializeCoreArchitecture();
    void SetupSystemDependencies();
    void StartPerformanceMonitoring();
    void OnPerformanceTimer();
    
    FEng_SystemInfo* FindSystemInfo(const FString& SystemName);
    const FEng_SystemInfo* FindSystemInfo(const FString& SystemName) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEngineArchitectureActor : public AActor
{
    GENERATED_BODY()

public:
    AEngineArchitectureActor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    UEngineArchitectureManager* GetArchitectureManager() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ShutdownArchitecture();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoInitialize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bShowDebugInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float DebugUpdateInterval;

private:
    FTimerHandle DebugTimerHandle;
    void OnDebugTimer();
};