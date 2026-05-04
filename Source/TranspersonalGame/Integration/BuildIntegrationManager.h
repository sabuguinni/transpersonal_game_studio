#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBuildIntegration, Log, All);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsCompiled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString LastBuildTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> ErrorMessages;

    FBuild_CompilationStatus()
    {
        bIsCompiled = false;
        ErrorCount = 0;
        WarningCount = 0;
        LastBuildTime = TEXT("Never");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ClassCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> LoadedClasses;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("Unknown");
        bIsLoaded = false;
        ClassCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ActorInventory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Actors")
    FString ActorClass;

    UPROPERTY(BlueprintReadOnly, Category = "Actors")
    int32 Count = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Actors")
    bool bHasDuplicates = false;

    UPROPERTY(BlueprintReadOnly, Category = "Actors")
    TArray<FString> ActorNames;

    FBuild_ActorInventory()
    {
        ActorClass = TEXT("Unknown");
        Count = 0;
        bHasDuplicates = false;
    }
};

/**
 * Build Integration Manager - Coordena integração entre todos os sistemas
 * Responsável por validação de compilação, limpeza de duplicados, e orquestração
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Compilation Status
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_CompilationStatus GetCompilationStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RefreshCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor = true)
    bool ValidateAllModules();

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> GetModuleStatuses() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsModuleLoaded(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetLoadedClasses(const FString& ModuleName) const;

    // Actor Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_ActorInventory> GetActorInventory() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor = true)
    int32 CleanupDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool HasDuplicateLightingActors() const;

    // Integration Validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor = true)
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetIntegrationErrors() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CanSystemsIntegrate(const FString& SystemA, const FString& SystemB) const;

    // Build Orchestration
    UFUNCTION(BlueprintCallable, Category = "Build Integration", CallInEditor = true)
    void TriggerFullBuildValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float GetBuildProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FString GetCurrentBuildPhase() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_CompilationStatus CurrentCompilationStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_ActorInventory> ActorInventories;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> IntegrationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float BuildProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString CurrentBuildPhase = TEXT("Idle");

private:
    void ScanForModules();
    void ScanForActors();
    void ValidateClassLoading();
    void CheckForDuplicates();
    void UpdateBuildProgress(float Progress, const FString& Phase);

    // Critical system dependencies
    TArray<FString> CriticalClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager")
    };

    TArray<FString> LightingActorTypes = {
        TEXT("DirectionalLight"),
        TEXT("SkyAtmosphere"),
        TEXT("SkyLight"),
        TEXT("ExponentialHeightFog")
    };
};