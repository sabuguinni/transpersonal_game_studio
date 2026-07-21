#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_DependencyManager.generated.h"

UENUM(BlueprintType)
enum class EEng_DependencyType : uint8
{
    Module,
    Class,
    Asset,
    Plugin,
    System
};

UENUM(BlueprintType)
enum class EEng_DependencyStatus : uint8
{
    Unknown,
    Available,
    Missing,
    Loading,
    Failed
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_DependencyInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    FString Name;

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    EEng_DependencyType Type;

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    EEng_DependencyStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    FString Path;

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    TArray<FString> RequiredBy;

    UPROPERTY(BlueprintReadOnly, Category = "Dependency")
    TArray<FString> Requires;

    FEng_DependencyInfo()
    {
        Name = TEXT("");
        Type = EEng_DependencyType::Module;
        Status = EEng_DependencyStatus::Unknown;
        Path = TEXT("");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_DependencyManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    void RegisterDependency(const FString& Name, EEng_DependencyType Type, const FString& Path);

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    void AddDependencyRelation(const FString& Dependent, const FString& Dependency);

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    bool CheckDependency(const FString& Name);

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    TArray<FEng_DependencyInfo> GetAllDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    TArray<FString> GetMissingDependencies() const;

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    bool ValidateAllDependencies();

    UFUNCTION(BlueprintCallable, Category = "Dependencies")
    TArray<FString> GetDependencyChain(const FString& Name) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    TMap<FString, FEng_DependencyInfo> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Dependencies")
    bool bValidationComplete;

private:
    void RegisterCoreDependencies();
    void RegisterModuleDependencies();
    void RegisterAssetDependencies();
    EEng_DependencyStatus CheckDependencyStatus(const FEng_DependencyInfo& Dependency);
    void BuildDependencyGraph();
    bool DetectCircularDependencies();
};

#include "Eng_DependencyManager.generated.h"