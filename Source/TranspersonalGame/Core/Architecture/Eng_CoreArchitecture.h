#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_CoreArchitecture.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CoreArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    void RegisterCoreSystem(const FString& SystemName, const FString& SystemVersion);

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    bool IsSystemRegistered(const FString& SystemName) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    void SetPerformanceTarget(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    float GetCurrentFPS() const;

    // Module Dependencies
    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    void RegisterModuleDependency(const FString& ModuleName, const FString& DependsOn);

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    TArray<FString> GetModuleDependencies(const FString& ModuleName) const;

    // System Health
    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    void ReportSystemHealth(const FString& SystemName, bool bIsHealthy);

    UFUNCTION(BlueprintCallable, Category = "Core Architecture")
    bool GetSystemHealth(const FString& SystemName) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Core Architecture")
    TMap<FString, FString> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Core Architecture")
    TMap<FString, TArray<FString>> ModuleDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Core Architecture")
    TMap<FString, bool> SystemHealthStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Core Architecture")
    float TargetFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Core Architecture")
    float CurrentFPS;

private:
    void UpdatePerformanceMetrics();
    void ValidateSystemDependencies();
};