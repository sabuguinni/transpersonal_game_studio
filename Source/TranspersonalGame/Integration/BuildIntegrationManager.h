#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildValidationComplete, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModuleStatusChanged, FString, ModuleName, EBuild_ModuleStatus, Status);

/**
 * Build Integration Manager - Agente #19
 * Gerencia integração contínua, validação de builds e estado dos módulos
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

    // Build Validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateProjectBuild();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateModuleIntegrity(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckHeaderCppPairs();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CleanOrphanedHeaders();

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RegisterModule(const FString& ModuleName, EBuild_ModuleStatus Status);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_ModuleStatus GetModuleStatus(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetFailedModules() const;

    // Actor Validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateMapActors(const FString& MapPath);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    int32 GetActorCountByType(const FString& ActorType) const;

    // Compilation Testing
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void TestCompilation();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsProjectCompiling() const;

    // Integration Reports
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SaveBuildReport(const FBuild_IntegrationReport& Report);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnBuildValidationComplete OnBuildValidationComplete;

    UPROPERTY(BlueprintAssignable, Category = "Build Integration")
    FOnModuleStatusChanged OnModuleStatusChanged;

protected:
    // Module tracking
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TMap<FString, EBuild_ModuleStatus> ModuleStatusMap;

    // Actor tracking
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TMap<FString, int32> ActorCountByType;

    // Build state
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIsCompiling;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bLastBuildSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FDateTime LastBuildTime;

    // Critical modules that must exist
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> CriticalModules;

    // Biome data for validation
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TMap<FString, FBuild_BiomeData> BiomeMap;

private:
    void InitializeCriticalModules();
    void InitializeBiomeMap();
    void ValidateActorPlacement(AActor* Actor);
    FString GetBiomeForLocation(const FVector& Location) const;
    void LogBuildError(const FString& Error);
    void LogBuildWarning(const FString& Warning);
};