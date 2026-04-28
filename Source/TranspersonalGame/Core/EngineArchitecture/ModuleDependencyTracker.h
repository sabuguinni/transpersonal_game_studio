#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SharedTypes.h"
#include "ModuleDependencyTracker.generated.h"

/**
 * Module Dependency Tracker
 * Tracks dependencies between game modules and ensures proper initialization order
 * Prevents circular dependencies and enforces architectural rules
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleDependency
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependency")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependency")
    TArray<FString> RequiredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependency")
    TArray<FString> OptionalModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependency")
    int32 InitializationPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependency")
    bool bIsInitialized;

    FEng_ModuleDependency()
    {
        InitializationPriority = 0;
        bIsInitialized = false;
    }
};

UENUM(BlueprintType)
enum class EEng_DependencyStatus : uint8
{
    Unresolved      UMETA(DisplayName = "Unresolved"),
    Resolving       UMETA(DisplayName = "Resolving"),
    Resolved        UMETA(DisplayName = "Resolved"),
    CircularRef     UMETA(DisplayName = "Circular Reference"),
    Failed          UMETA(DisplayName = "Failed")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UModuleDependencyTracker : public UObject
{
    GENERATED_BODY()

public:
    UModuleDependencyTracker();

    // Module Registration
    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    void RegisterModule(const FString& ModuleName, const TArray<FString>& RequiredModules, int32 Priority = 0);

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    void RegisterOptionalDependency(const FString& ModuleName, const FString& OptionalModule);

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    bool IsModuleRegistered(const FString& ModuleName) const;

    // Dependency Resolution
    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    TArray<FString> GetInitializationOrder();

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    bool ValidateAllDependencies();

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    EEng_DependencyStatus GetModuleStatus(const FString& ModuleName) const;

    // Circular Dependency Detection
    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    bool HasCircularDependencies();

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    TArray<FString> GetCircularDependencyChain();

    // Module State Management
    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    void MarkModuleInitialized(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    void MarkModuleFailed(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    TArray<FString> GetInitializedModules() const;

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    TArray<FString> GetFailedModules() const;

    // Dependency Analysis
    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    TArray<FString> GetModuleDependents(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    TArray<FString> GetModuleDependencies(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    FString GenerateDependencyReport() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    TMap<FString, FEng_ModuleDependency> ModuleRegistry;

    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    TMap<FString, EEng_DependencyStatus> ModuleStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    TArray<FString> FailedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    TArray<FString> CircularDependencyChain;

private:
    // Internal dependency resolution
    bool ResolveDependenciesRecursive(const FString& ModuleName, TArray<FString>& ResolvedOrder, TSet<FString>& Visiting);
    void DetectCircularDependencies();
    bool CanModuleInitialize(const FString& ModuleName) const;
    void UpdateModuleStatus(const FString& ModuleName, EEng_DependencyStatus Status);
};