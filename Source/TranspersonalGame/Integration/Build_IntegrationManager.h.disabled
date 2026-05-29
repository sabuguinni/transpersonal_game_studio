#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_IntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bCompilationSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    float CompilationTimeSeconds = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString CompilationOutput;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorsInMap = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FunctionalCppClasses = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 OrphanHeaders = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bMapPlayable = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 IntegrationScore = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> NextPriorities;
};

/**
 * Integration Manager - Coordena a integração de todos os sistemas do jogo
 * Responsável por validar compilação, limpar duplicados, e garantir que o jogo é jogável
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_IntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Compilation Management
    UFUNCTION(BlueprintCallable, Category = "Integration|Compilation")
    FBuild_CompilationResult TestCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration|Compilation")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration|Compilation")
    TArray<FString> FindOrphanHeaders();

    // Map Validation
    UFUNCTION(BlueprintCallable, Category = "Integration|Map")
    FBuild_IntegrationStatus ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Integration|Map")
    int32 CleanDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Integration|Map")
    bool IsMapPlayable();

    // Class Validation
    UFUNCTION(BlueprintCallable, Category = "Integration|Classes")
    TArray<FString> GetFunctionalCppClasses();

    UFUNCTION(BlueprintCallable, Category = "Integration|Classes")
    bool ValidateClassIntegrity(const FString& ClassName);

    // Integration Reporting
    UFUNCTION(BlueprintCallable, Category = "Integration|Reporting")
    FBuild_IntegrationStatus GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration|Reporting")
    void LogIntegrationStatus();

    // Critical System Checks
    UFUNCTION(BlueprintCallable, Category = "Integration|Critical")
    bool ValidateCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration|Critical")
    TArray<FString> GetCriticalIssues();

protected:
    // Internal validation methods
    bool ValidatePlayerStart();
    bool ValidateLighting();
    bool ValidateTerrain();
    bool ValidateGameMode();
    
    int32 CountActorsByType(UClass* ActorClass);
    void CleanActorDuplicates(UClass* ActorClass, int32 MaxCount = 1);

private:
    UPROPERTY()
    FBuild_IntegrationStatus CurrentStatus;

    UPROPERTY()
    TArray<FString> KnownCppClasses;

    UPROPERTY()
    bool bInitialized = false;

    // Critical actor counts
    int32 PlayerStartCount = 0;
    int32 DirectionalLightCount = 0;
    int32 LandscapeCount = 0;
    int32 SkyAtmosphereCount = 0;
};