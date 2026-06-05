#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Eng_GameplayArchitect.generated.h"

// Forward declarations
class UEng_BiomeSystem;
class UEng_SurvivalSystem;
class UEng_DinosaurSystem;

UENUM(BlueprintType)
enum class EEng_GameplaySystemType : uint8
{
    None            UMETA(DisplayName = "None"),
    Biome           UMETA(DisplayName = "Biome System"),
    Survival        UMETA(DisplayName = "Survival System"),
    Dinosaur        UMETA(DisplayName = "Dinosaur System"),
    Combat          UMETA(DisplayName = "Combat System"),
    Quest           UMETA(DisplayName = "Quest System"),
    NPC             UMETA(DisplayName = "NPC System"),
    World           UMETA(DisplayName = "World System"),
    Audio           UMETA(DisplayName = "Audio System"),
    VFX             UMETA(DisplayName = "VFX System"),
    Performance     UMETA(DisplayName = "Performance System")
};

UENUM(BlueprintType)
enum class EEng_GameplayPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    High            UMETA(DisplayName = "High"),
    Medium          UMETA(DisplayName = "Medium"),
    Low             UMETA(DisplayName = "Low"),
    Background      UMETA(DisplayName = "Background")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_GameplaySystemInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    EEng_GameplaySystemType SystemType = EEng_GameplaySystemType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    EEng_GameplayPriority Priority = EEng_GameplayPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    bool bIsInitialized = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    float InitializationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    float LastUpdateTime = 0.0f;

    FEng_GameplaySystemInfo()
    {
        SystemType = EEng_GameplaySystemType::None;
        SystemName = TEXT("Unknown System");
        Priority = EEng_GameplayPriority::Medium;
        bIsActive = false;
        bIsInitialized = false;
        InitializationTime = 0.0f;
        LastUpdateTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_GameplayArchitectureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnableBiomeSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnableSurvivalSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnableDinosaurSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnablePerformanceMonitoring = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float SystemUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxConcurrentSystems = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoInitializeSystems = true;

    FEng_GameplayArchitectureConfig()
    {
        bEnableBiomeSystem = true;
        bEnableSurvivalSystem = true;
        bEnableDinosaurSystem = true;
        bEnablePerformanceMonitoring = true;
        SystemUpdateInterval = 0.1f;
        MaxConcurrentSystems = 10;
        bAutoInitializeSystems = true;
    }
};

/**
 * Engine Architect Gameplay Architecture System
 * Defines and manages the core gameplay architecture for the prehistoric survival game
 * Coordinates between biome, survival, dinosaur, and other gameplay systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_GameplayArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_GameplayArchitect();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Methods
    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void InitializeGameplayArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    void ShutdownGameplayArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    bool RegisterGameplaySystem(EEng_GameplaySystemType SystemType, const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    bool UnregisterGameplaySystem(EEng_GameplaySystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    bool IsSystemRegistered(EEng_GameplaySystemType SystemType) const;

    UFUNCTION(BlueprintCallable, Category = "Gameplay Architecture")
    bool IsSystemActive(EEng_GameplaySystemType SystemType) const;

    // System Management
    UFUNCTION(BlueprintCallable, Category = "System Management")
    void ActivateSystem(EEng_GameplaySystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void DeactivateSystem(EEng_GameplaySystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void UpdateAllSystems(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "System Management")
    void UpdateSystem(EEng_GameplaySystemType SystemType, float DeltaTime);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetArchitectureConfig(const FEng_GameplayArchitectureConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    FEng_GameplayArchitectureConfig GetArchitectureConfig() const;

    // System Information
    UFUNCTION(BlueprintCallable, Category = "System Info")
    TArray<FEng_GameplaySystemInfo> GetAllSystemInfo() const;

    UFUNCTION(BlueprintCallable, Category = "System Info")
    FEng_GameplaySystemInfo GetSystemInfo(EEng_GameplaySystemType SystemType) const;

    UFUNCTION(BlueprintCallable, Category = "System Info")
    int32 GetActiveSystemCount() const;

    UFUNCTION(BlueprintCallable, Category = "System Info")
    float GetTotalSystemUpdateTime() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMonitoring(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceMonitoringEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSystemPerformanceMetric(EEng_GameplaySystemType SystemType) const;

    // Debug and Validation
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void ValidateArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void PrintSystemStatus();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    bool RunArchitectureDiagnostics();

protected:
    // Core system registry
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TMap<EEng_GameplaySystemType, FEng_GameplaySystemInfo> RegisteredSystems;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_GameplayArchitectureConfig ArchitectureConfig;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    TMap<EEng_GameplaySystemType, float> SystemPerformanceMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bPerformanceMonitoringEnabled;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float TotalUpdateTime;

    // Internal methods
    void InitializeDefaultSystems();
    void CleanupSystems();
    void UpdatePerformanceMetrics(EEng_GameplaySystemType SystemType, float UpdateTime);
    bool ValidateSystemDependencies(EEng_GameplaySystemType SystemType) const;
    void LogSystemEvent(EEng_GameplaySystemType SystemType, const FString& Event) const;

private:
    // Timer handle for system updates
    FTimerHandle SystemUpdateTimerHandle;

    // Initialization state
    bool bIsArchitectureInitialized;
    
    // System update tracking
    double LastSystemUpdateTime;
    int32 SystemUpdateCounter;
};