#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TranspersonalGameCore.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalCore, Log, All);

/**
 * Core module for Transpersonal Game
 * Handles initialization of all major systems and enforces architecture rules
 */
class TRANSPERSONALGAME_API FTranspersonalGameCoreModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    /** Get the singleton instance */
    static FTranspersonalGameCoreModule& Get()
    {
        return FModuleManager::LoadModuleChecked<FTranspersonalGameCoreModule>("TranspersonalGameCore");
    }

    /** Check if the module is loaded */
    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("TranspersonalGameCore");
    }

private:
    void InitializeCoreSystems();
    void ValidateRequiredPlugins();
    void SetupPerformanceBudgets();
};

/**
 * Core Game Instance Subsystem
 * Manages global game state and system coordination
 */
UCLASS()
class TRANSPERSONALGAME_API UTranspersonalCoreSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Get the core subsystem instance */
    UFUNCTION(BlueprintPure, Category = "Transpersonal Core")
    static UTranspersonalCoreSubsystem* Get(const UObject* WorldContext);

    /** Register a module with the core system */
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Core")
    void RegisterModule(const FString& ModuleName, UObject* ModuleInstance);

    /** Check if required systems are initialized */
    UFUNCTION(BlueprintPure, Category = "Transpersonal Core")
    bool AreCorSystemsReady() const { return bCoreSystemsInitialized; }

    /** Get current performance metrics */
    UFUNCTION(BlueprintPure, Category = "Transpersonal Core")
    void GetPerformanceMetrics(float& FrameTime, int32& TriangleCount, float& MemoryUsageMB) const;

protected:
    /** Registered modules */
    UPROPERTY()
    TMap<FString, TObjectPtr<UObject>> RegisteredModules;

    /** Core systems initialization status */
    UPROPERTY()
    bool bCoreSystemsInitialized = false;

    /** Performance tracking */
    UPROPERTY()
    float LastFrameTime = 0.0f;

    UPROPERTY()
    int32 LastTriangleCount = 0;

    UPROPERTY()
    float LastMemoryUsage = 0.0f;

private:
    void InitializeNanite();
    void InitializeLumen();
    void InitializeWorldPartition();
    void InitializeVirtualShadowMaps();
    void InitializeMassEntity();
    void UpdatePerformanceMetrics();

    FTimerHandle PerformanceUpdateTimer;
};

/**
 * Core Game Mode Base
 * Provides foundation for all game modes in the project
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATranspersonalGameModeBase();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Initialize game-specific systems */
    UFUNCTION(BlueprintImplementableEvent, Category = "Transpersonal Game")
    void OnGameSystemsInitialized();

    /** Called when all core systems are ready */
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Game")
    virtual void OnCoreSystemsReady();

    /** Target performance metrics */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MaxMemoryUsageMB = 8192.0f; // 8GB console target

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxSimultaneousAgents = 50000;
};

/**
 * Core Player Controller
 * Base controller with integrated systems support
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ATranspersonalPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    /** Core systems access */
    UFUNCTION(BlueprintPure, Category = "Transpersonal Core")
    UTranspersonalCoreSubsystem* GetCoreSubsystem() const;

    /** Debug performance display */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void TogglePerformanceDisplay();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
    bool bShowPerformanceMetrics = false;

private:
    void DisplayPerformanceMetrics();
    FTimerHandle PerformanceDisplayTimer;
};