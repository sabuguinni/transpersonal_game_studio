#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Eng_ArchitectureManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemModule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FString> Dependencies;

    FEng_SystemModule()
    {
        ModuleName = TEXT("Unknown");
        bIsLoaded = false;
        bIsActive = false;
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TriangleCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsage;

    FEng_PerformanceMetrics()
    {
        FrameTime = 0.0f;
        RenderTime = 0.0f;
        GameThreadTime = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        MemoryUsage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitectureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeSystemModules();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterSystemModule(const FString& ModuleName, int32 Priority, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool LoadSystemModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool UnloadSystemModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsSystemModuleLoaded(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetLoadedModules() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTargets(float TargetFrameTime, int32 MaxDrawCalls, float MaxMemoryMB);

    // System Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetSystemErrors() const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void RunDiagnostics();

    // Actor Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetTotalActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsActorCountWithinLimits() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CleanupExcessActors();

    // Module Dependencies
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckModuleDependencies(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetModuleDependencies(const FString& ModuleName) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_SystemModule> SystemModules;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxDrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    int32 MaxActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> SystemErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bSystemsInitialized;

private:
    void InitializeCoreModules();
    void ValidateModuleLoad(const FString& ModuleName);
    void UpdateSystemMetrics();
    FEng_SystemModule* FindSystemModule(const FString& ModuleName);
    bool ResolveDependencies(const FString& ModuleName);
};