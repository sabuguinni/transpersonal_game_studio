#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitecturalFramework.generated.h"

/**
 * ENGINE ARCHITECT CYCLE 003 - ARCHITECTURAL FRAMEWORK
 * 
 * This is the foundational architectural framework that defines:
 * 1. System registration and lifecycle management
 * 2. Cross-system communication protocols
 * 3. Performance monitoring and validation
 * 4. Module dependency resolution
 * 5. Compilation safety and type management
 * 
 * ALL OTHER AGENTS MUST FOLLOW THESE ARCHITECTURAL RULES
 */

// Architecture Validation Levels
UENUM(BlueprintType)
enum class EEng_ArchValidationLevel : uint8
{
    None            UMETA(DisplayName = "None"),
    Basic           UMETA(DisplayName = "Basic"),
    Standard        UMETA(DisplayName = "Standard"),
    Strict          UMETA(DisplayName = "Strict"),
    Maximum         UMETA(DisplayName = "Maximum")
};

// System Registration Status
UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Unregistered    UMETA(DisplayName = "Unregistered"),
    Registered      UMETA(DisplayName = "Registered"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Active          UMETA(DisplayName = "Active"),
    Error           UMETA(DisplayName = "Error"),
    Disabled        UMETA(DisplayName = "Disabled")
};

// Module Dependencies
UENUM(BlueprintType)
enum class EEng_ModuleDependency : uint8
{
    Core            UMETA(DisplayName = "Core"),
    Physics         UMETA(DisplayName = "Physics"),
    World           UMETA(DisplayName = "World"),
    AI              UMETA(DisplayName = "AI"),
    Character       UMETA(DisplayName = "Character"),
    Combat          UMETA(DisplayName = "Combat"),
    Quest           UMETA(DisplayName = "Quest"),
    Audio           UMETA(DisplayName = "Audio"),
    VFX             UMETA(DisplayName = "VFX"),
    UI              UMETA(DisplayName = "UI")
};

// System Registration Data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemRegistration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    TArray<EEng_ModuleDependency> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float InitializationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FDateTime LastValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    int32 ValidationErrors;

    FEng_SystemRegistration()
    {
        SystemName = "Unknown System";
        Status = EEng_SystemStatus::Unregistered;
        InitializationTime = 0.0f;
        LastValidation = FDateTime::Now();
        ValidationErrors = 0;
    }
};

// Performance Metrics
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 PhysicsBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    FEng_PerformanceMetrics()
    {
        FrameTime = 16.67f; // Target 60fps
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        ActiveActors = 0;
        PhysicsBodies = 0;
        MemoryUsageMB = 0.0f;
    }
};

/**
 * ARCHITECTURAL FRAMEWORK SUBSYSTEM
 * 
 * This is the central authority for all architectural decisions.
 * All systems must register here and follow the established patterns.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_ArchitecturalFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RegisterSystem(const FString& SystemName, const TArray<EEng_ModuleDependency>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EEng_SystemStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FEng_SystemRegistration> GetAllSystems();

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetValidationErrorCount();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsPerformanceWithinLimits();

    // Compilation Safety
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateTypeRegistry();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetCompilationErrors();

protected:
    // Internal system registry
    UPROPERTY()
    TMap<FString, FEng_SystemRegistration> RegisteredSystems;

    // Performance tracking
    UPROPERTY()
    FEng_PerformanceMetrics CurrentMetrics;

    // Validation settings
    UPROPERTY(EditAnywhere, Category = "Architecture")
    EEng_ArchValidationLevel ValidationLevel;

    UPROPERTY(EditAnywhere, Category = "Architecture")
    float PerformanceValidationInterval;

    UPROPERTY(EditAnywhere, Category = "Architecture")
    bool bEnableRuntimeValidation;

    // Internal methods
    void UpdatePerformanceMetrics();
    void ValidateSystemDependencies();
    void LogArchitecturalEvent(const FString& Event, const FString& Details);

private:
    // Validation timer
    FTimerHandle ValidationTimer;
    
    // Error tracking
    TArray<FString> CompilationErrors;
    int32 TotalValidationErrors;
};