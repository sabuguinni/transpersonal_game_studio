#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Containers/Map.h"
#include "SharedTypes.h"
#include "Eng_SystemsRegistry.generated.h"

// Forward declarations
class UEng_ArchitecturalValidator;
class UCore_PhysicsManager;
class UBiomeManager;
class AStudioDirector;

/**
 * Engine Systems Registry - Central hub for all architectural systems
 * Manages initialization order, dependencies, and system validation
 * This is the master coordinator that ensures all systems start in the correct order
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemsRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemsRegistry();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystem(const FString& SystemName, UObject* SystemInstance, int32 Priority = 100);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterSystem(const FString& SystemName);

    // System Access
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UObject* GetSystem(const FString& SystemName);

    template<typename T>
    T* GetSystem(const FString& SystemName)
    {
        return Cast<T>(GetSystem(SystemName));
    }

    // System Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystem(const FString& SystemName);

    // Initialization Control
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ShutdownAllSystems();

    // System Status
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetRegisteredSystems() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemRegistered(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemInitialized(const FString& SystemName) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetSystemInitializationTime(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void LogSystemsStatus() const;

protected:
    // System registration data
    USTRUCT(BlueprintType)
    struct FEng_SystemRegistration
    {
        GENERATED_BODY()

        UPROPERTY(BlueprintReadOnly)
        FString SystemName;

        UPROPERTY(BlueprintReadOnly)
        UObject* SystemInstance;

        UPROPERTY(BlueprintReadOnly)
        int32 Priority;

        UPROPERTY(BlueprintReadOnly)
        bool bIsInitialized;

        UPROPERTY(BlueprintReadOnly)
        float InitializationTime;

        UPROPERTY(BlueprintReadOnly)
        FDateTime RegistrationTime;

        FEng_SystemRegistration()
            : SystemInstance(nullptr)
            , Priority(100)
            , bIsInitialized(false)
            , InitializationTime(0.0f)
        {
            RegistrationTime = FDateTime::Now();
        }
    };

    // Registered systems map
    UPROPERTY()
    TMap<FString, FEng_SystemRegistration> RegisteredSystems;

    // Core system references for fast access
    UPROPERTY()
    UCore_PhysicsManager* PhysicsManager;

    UPROPERTY()
    UBiomeManager* BiomeManager;

    UPROPERTY()
    AStudioDirector* StudioDirector;

    // Initialization state
    UPROPERTY()
    bool bAllSystemsInitialized;

    UPROPERTY()
    float TotalInitializationTime;

private:
    // Internal initialization helpers
    void InitializeCorePhysics();
    void InitializeWorldSystems();
    void InitializeGameplaySystems();
    void InitializeAudioSystems();
    void InitializeVFXSystems();
    
    // Validation helpers
    bool ValidatePhysicsIntegration();
    bool ValidateWorldGeneration();
    bool ValidateGameplayIntegration();
    
    // Performance tracking
    void StartSystemTimer(const FString& SystemName);
    void EndSystemTimer(const FString& SystemName);
    
    // System dependency resolution
    TArray<FString> GetSystemDependencies(const FString& SystemName);
    bool AreDependenciesSatisfied(const FString& SystemName);
    void SortSystemsByPriority(TArray<FEng_SystemRegistration>& Systems);
};