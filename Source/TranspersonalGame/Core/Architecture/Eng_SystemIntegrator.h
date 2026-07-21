#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_SystemIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemLayer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString LayerName;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> SystemNames;

    UPROPERTY(BlueprintReadOnly)
    int32 Priority;

    UPROPERTY(BlueprintReadOnly)
    bool bIsActive;

    FEng_SystemLayer()
    {
        LayerName = TEXT("Default");
        Priority = 0;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemDependency
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly)
    bool bIsOptional;

    FEng_SystemDependency()
    {
        bIsOptional = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_SystemIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemIntegrator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterSystemLayer(const FString& LayerName, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterSystemToLayer(const FString& SystemName, const FString& LayerName);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void AddSystemDependency(const FString& SystemName, const FString& DependencyName, bool bOptional = false);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void InitializeSystemsInOrder();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void ShutdownSystemsInReverseOrder();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    TArray<FString> GetSystemInitializationOrder();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool IsSystemActive(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void SetSystemActive(const FString& SystemName, bool bActive);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    FString GetSystemStatus();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    TArray<FEng_SystemLayer> SystemLayers;

    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    TArray<FEng_SystemDependency> SystemDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    TArray<FString> InitializationOrder;

    UPROPERTY(BlueprintReadOnly, Category = "System Integration")
    bool bSystemsInitialized;

private:
    void SortLayersByPriority();
    void BuildInitializationOrder();
    bool ValidateDependencyChain(const FString& SystemName, TArray<FString>& VisitedSystems);
};