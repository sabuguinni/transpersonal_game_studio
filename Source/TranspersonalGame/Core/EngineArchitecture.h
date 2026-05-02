#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "EngineArchitecture.generated.h"

/**
 * Engine Architecture Subsystem
 * Defines and enforces the technical architecture rules for the entire project
 * All other agents must follow the patterns established here
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngineArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleStructure();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidatePerformanceRequirements();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateMemoryUsage();

    // Module registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterCoreModule(const FString& ModuleName, const FString& ModulePath);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterGameplayModule(const FString& ModuleName, const FString& ModulePath);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetCurrentActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetCurrentMemoryUsageMB() const;

    // Architecture enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool EnforceNamingConventions();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool EnforceModuleDependencies();

protected:
    // Registered modules
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, FString> CoreModules;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, FString> GameplayModules;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTimeMs = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorCount = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB = 8192.0f; // 8GB

    // Architecture state
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bIsArchitectureValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FDateTime LastValidationTime;

private:
    // Internal validation methods
    bool ValidateFileStructure();
    bool ValidateIncludePaths();
    bool ValidateNamingConventions();
    void LogArchitectureStatus();
};