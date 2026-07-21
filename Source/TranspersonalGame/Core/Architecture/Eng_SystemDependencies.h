#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_SystemDependencies.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemDependency
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependency")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependency")
    TArray<FString> RequiredSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependency")
    int32 InitializationPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependency")
    bool bIsInitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependency")
    bool bIsCritical;

    FEng_SystemDependency()
    {
        SystemName = TEXT("Unknown");
        InitializationPriority = 100;
        bIsInitialized = false;
        bIsCritical = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString ModuleVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    float LoadTime;

    FEng_ModuleInfo()
    {
        ModuleName = TEXT("Unknown");
        ModuleVersion = TEXT("1.0.0");
        bIsLoaded = false;
        LoadTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemDependencies : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemDependencies();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "System Dependencies")
    void RegisterSystemDependency(const FEng_SystemDependency& Dependency);

    UFUNCTION(BlueprintCallable, Category = "System Dependencies")
    bool ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "System Dependencies")
    TArray<FString> GetInitializationOrder();

    UFUNCTION(BlueprintCallable, Category = "System Dependencies")
    bool IsSystemInitialized(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Dependencies")
    void MarkSystemInitialized(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Dependencies")
    void RegisterModule(const FEng_ModuleInfo& ModuleInfo);

    UFUNCTION(BlueprintCallable, Category = "System Dependencies")
    TArray<FEng_ModuleInfo> GetLoadedModules() const;

    UFUNCTION(BlueprintCallable, Category = "System Dependencies")
    bool CheckCircularDependencies();

    UFUNCTION(BlueprintCallable, Category = "System Dependencies")
    void GenerateDependencyReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependencies")
    TArray<FEng_SystemDependency> SystemDependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependencies")
    TArray<FEng_ModuleInfo> LoadedModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependencies")
    bool bDependenciesValidated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependencies")
    float LastValidationTime;

private:
    void InitializeCoreDependencies();
    bool ResolveDependencyOrder(TArray<FString>& OutOrder);
    bool HasCircularDependency(const FString& SystemName, TArray<FString>& VisitedSystems);
    void LogDependencyChain(const FString& SystemName, int32 Depth = 0);
};